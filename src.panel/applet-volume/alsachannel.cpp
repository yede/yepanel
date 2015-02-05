#include "alsachannel.h"
#include "alsaelement.h"
//==================================================================================

AlsaChannel::AlsaChannel(snd_mixer_elem_t *elementId,
						 const snd_mixer_selem_channel_id_t channelId)
	: m_parentId(elementId)
	, m_id(channelId)
{
}

AlsaChannel::~AlsaChannel()
{
	m_parentId = NULL;
}

long AlsaChannel::volume() const
{
	long vol = 0;
	snd_mixer_selem_get_playback_volume(m_parentId, m_id, &vol);

	return vol;
}

bool AlsaChannel::isMuted() const
{
	int val;
	if (snd_mixer_selem_get_playback_switch(m_parentId, m_id, &val) < 0)
		return false;

	return !static_cast<bool>(val);
}
