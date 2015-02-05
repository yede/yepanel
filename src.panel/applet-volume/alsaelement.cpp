#include <QDebug>

#include "alsachannel.h"
#include "alsaelement.h"
//==================================================================================

typedef snd_mixer_selem_channel_id_t ChId;
//==================================================================================

AlsaElement::AlsaElement(snd_mixer_t *mixer, const char *elementName)
	: m_mixer(NULL)
	, m_hi(0)
	, m_lo(0)
	, m_channels(0)
{
	// Find element by name
	snd_mixer_selem_id_t *sid;
	if (!snd_mixer_selem_id_malloc(&sid)) {
		snd_mixer_selem_id_set_index(sid, 0);
		snd_mixer_selem_id_set_name(sid, elementName);
		m_mixer = snd_mixer_find_selem(mixer, sid);
	}
	if (!m_mixer) {
		qDebug() << "ALSA unable to find simple control"
				 << snd_mixer_selem_id_get_name(sid);
	}
	snd_mixer_selem_id_free(sid);

	// Store playback volume range
	if (m_mixer) {
		snd_mixer_selem_get_playback_volume_range(m_mixer, &m_lo, &m_hi);

		// Select playback channels
		for (int c = 0; c <= (int) SND_MIXER_SCHN_LAST; c++) {
			ChId cId = static_cast<ChId>(c);
			if (snd_mixer_selem_has_playback_channel(m_mixer, cId)) {
				AlsaChannel *chan = new AlsaChannel(m_mixer, cId);
				m_channels.push_back(chan);
			}
		}
	}
}

AlsaElement::~AlsaElement()
{
	m_mixer = NULL;
	for (int i = 0; i < m_channels.size(); i++)
		delete m_channels[i];
}

void AlsaElement::setVolumePercent(double percent)
{
	if (percent > 100.0)
		percent = 100.0;
	if (percent < 0.0)
		percent = 0.0;

	double vol = m_lo + (m_hi - m_lo) * percent / 100.0;
	long volume = static_cast<long>(vol);
	if (vol - volume >= 0.5)
		volume += 1;

	snd_mixer_selem_set_playback_volume_all(m_mixer, volume);
}

double AlsaElement::volumePercent() const
{
	if (m_lo == m_hi)
		return 0;

	long volume = m_lo;
	for (int i = 0; i < m_channels.size(); i++) {
		long tmpVolume = m_channels[i]->volume();
		if (tmpVolume > volume)
			volume = tmpVolume;
	}

	double den = static_cast<double>(m_hi - m_lo);

	return (volume - m_lo) * 100 / den;
}

bool AlsaElement::isMuted() const
{
	for (int i = 0; i < m_channels.size(); i++)
		if (!m_channels[i]->isMuted())
			return false;

	return true;  // true if all channels are muted
}

void AlsaElement::toggleMute() const
{
	snd_mixer_selem_set_playback_switch_all(m_mixer, isMuted());
}
