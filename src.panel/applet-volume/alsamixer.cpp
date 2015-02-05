#include <poll.h>
#include <QSocketNotifier>
#include <QDebug>

#include "alsaelement.h"
#include "alsamixer.h"
#include "volumeapplet.h"
//==================================================================================

AlsaMixer::AlsaMixer(VolumeApplet *applet)
	: m_applet(applet)
	, m_mixer(NULL)
	, m_masterIndex(-1)
{
}

AlsaMixer::~AlsaMixer()
{
	close();
}
//==================================================================================

void AlsaMixer::close()
{
	if (m_mixer == NULL) return;

	for (int i = 0; i < m_elements.size(); i++)
		delete m_elements[i];

	if (m_mixer != NULL)
		snd_mixer_close(m_mixer);

	m_mixer = NULL;
	m_masterIndex = -1;
}

bool AlsaMixer::open(const QVector<const char *> &elementNames)
{
	int err;
	snd_mixer_t *mixer = NULL;
	const char *card = "default";

	if ((err = snd_mixer_open(&mixer, 0)) < 0) {
		qDebug() << "ALSA mixer " << card << " open error: "
				 << snd_strerror(err);
		return false;
	}

	if ((err = snd_mixer_attach(mixer, card)) < 0) {
		qDebug() << "ALSA Attach to card " << card << " failed: "
				 << snd_strerror(err);
		snd_mixer_close(mixer);
		return false;
	}

	if ((err = snd_mixer_selem_register(mixer, NULL, NULL)) < 0) {
		qDebug() << "ALSA mixer register error: " << snd_strerror(err);
		snd_mixer_close(mixer);
		return false;
	}

	if ((err = snd_mixer_load(mixer)) < 0) {
		qDebug() << "ALSA mixer " << card << " load error: %s"
				 << snd_strerror(err);
		snd_mixer_close(mixer);
		return false;
	}

	m_mixer = mixer;
	setupElements(elementNames);

	return true;
}

void AlsaMixer::setupElements(const QVector<const char *> &elementNames)
{
	bool found = false;

	for (int i = 0; i < elementNames.size(); i++) {
		const char *name = elementNames.at(i);
		AlsaElement *elem = new AlsaElement(m_mixer, name);
		m_elements.push_back(elem);
		if (!found && memcmp(name, "Master", 7) == 0) {
			found = true;
			m_masterIndex = i;
		}
	}

	setupNotifier();
}

void AlsaMixer::setupNotifier()
{
//	snd_mixer_selem_set_playback_volume_range(elem, 0, 100);

	int n_fds = snd_mixer_poll_descriptors_count(m_mixer);
	size_t size = sizeof(pollfd) * n_fds;
	pollfd *fds = (pollfd *) malloc(size);
	memset(fds, 0, size);

	snd_mixer_poll_descriptors(m_mixer, fds, n_fds);
	QSocketNotifier *notifier;
	for (int i = 0; i < n_fds; ++i) {
		notifier = new QSocketNotifier(fds[i].fd, QSocketNotifier::Read, m_applet);
		QObject::connect(notifier, SIGNAL(activated(int)),
						 m_applet, SLOT(onVolumeChanged(int)));
	}

	free(fds);
}

void AlsaMixer::handleNotify(int socket)
{
	Q_UNUSED(socket);
	snd_mixer_handle_events(m_mixer);
//	update_volume_data(vol);
}
//==================================================================================

double AlsaMixer::getElementVolume(unsigned int index) const
{
	return m_elements.at(index)->volumePercent();
}

void AlsaMixer::setElementVolume(unsigned int index, double volume)
{
	m_elements.at(index)->setVolumePercent(volume);
}

bool AlsaMixer::isElementMuted(unsigned int index) const
{
	return m_elements.at(index)->isMuted();
}

void AlsaMixer::toggleElementMute(unsigned int index)
{
	m_elements.at(index)->toggleMute();
}
//==================================================================================

int AlsaMixer::getMasterVolume() const
{
	if (m_masterIndex < 0) return 0;

	return getElementVolume(m_masterIndex);
}

void AlsaMixer::setMasterVolume(int volume)
{
	if (m_masterIndex < 0) return;

	setElementVolume(m_masterIndex, volume);
}

bool AlsaMixer::isMasterMuted() const
{
	if (m_masterIndex < 0) return false;

	return isElementMuted(m_masterIndex);
}

void AlsaMixer::toggleMasterMute()
{
	if (m_masterIndex < 0) return;

	toggleElementMute(m_masterIndex);
}
