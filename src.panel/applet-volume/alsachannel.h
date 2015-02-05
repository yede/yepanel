#ifndef ALSACHANNEL_H
#define ALSACHANNEL_H

#include <alsa/asoundlib.h>
//==================================================================================

class AlsaChannel
{
public:
	AlsaChannel(snd_mixer_elem_t *elementId,
				const snd_mixer_selem_channel_id_t channelId);
	~AlsaChannel();

	long volume() const;
	bool isMuted() const;

private:
	snd_mixer_elem_t *m_parentId;
	const snd_mixer_selem_channel_id_t m_id;
};

#endif  // ALSA_CHANNEL_HH
