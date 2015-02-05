#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include "volume_alsa.h"
#include "volumeapplet.h"
//==================================================================================

AlsaVolume::AlsaVolume(VolumeApplet *applet)
	: m_applet(applet)
	, m_initialized(false)
	, m_mixer(NULL)
	, m_sid(NULL)
	, m_initialized(NULL)
{
}

AlsaVolume::~AlsaVolume()
{
}

void AlsaVolume::finalize()
{
	if (m_mixer != NULL) {
		snd_mixer_close(m_mixer);
		m_mixer = NULL;
	}
}

/* Initialize the ALSA interface. */
bool AlsaVolume::initialize()
{
	if (m_initialized) return true;

	/* Access the "default" device. */
	snd_mixer_selem_id_alloca(&m_sid);
	snd_mixer_open(&m_mixer, 0);
	snd_mixer_attach(m_mixer, "default");
	snd_mixer_selem_register(m_mixer, NULL, NULL);
	snd_mixer_load(m_mixer);

	/* Find Master element, or Front element, or PCM element, or LineOut element.
	 * If one of these succeeds, master_element is valid. */
	if (!findElement("Master") &&
		!findElement("Front") &&
		!findElement("PCM") &&
		!findElement("LineOut"))
	{
		return false;
	}

	/* Set the playback volume range as we wish it. */
	snd_mixer_selem_set_playback_volume_range(m_masterElement, 0, 100);

	/* Listen to events from ALSA. */
	int n_fds = snd_mixer_poll_descriptors_count(m_mixer);
	size_t size = sizeof(struct pollfd) * n_fds;
	struct pollfd *fds = malloc(size);
	memset(fds, 0, size);

	snd_mixer_poll_descriptors(m_mixer, fds, n_fds);
	QSocketNotifier *notifier;
	for (int i = 0; i < n_fds; ++i) {
		notifier = new QSocketNotifier(fds[i].fd, QSocketNotifier::Read, m_applet);
		QObject::connect(notifier, SIGNAL(activated(int)),
						 m_applet, SLOT(onVolumeChanged(int)));
	}
	free(fds);

	m_initialized = true;
	return true;
}
//==================================================================================

bool AlsaVolume::findElement(const char *ename)
{
	for (m_masterElement = snd_mixer_first_elem(m_mixer);
		 m_masterElement != NULL;
		 m_masterElement = snd_mixer_elem_next(m_masterElement))
	{
		snd_mixer_selem_get_id(m_masterElement,m_sid);
		if ((snd_mixer_selem_is_active(m_masterElement))
			&& (strcmp(ename, snd_mixer_selem_id_get_name(m_sid)) == 0))
			return true;
	}
	return false;
}

/* Get the presence of the mute control from the sound system. */
bool AlsaVolume::hasMute() const
{
	if (m_masterElement == NULL) return false;
	return snd_mixer_selem_has_playback_switch(m_masterElement);
}

/* Get the condition of the mute control from the sound system. */
bool AlsaVolume::isMuted() const
{
	/* The switch is on if sound is not muted, and off if the sound is muted.
	 * Initialize so that the sound appears unmuted if the control does not exist. */
	int value = 1;
	if (m_masterElement != NULL)
		snd_mixer_selem_get_playback_switch(m_masterElement, 0, &value);
	return (value == 0);
}

/* Get the volume from the sound system.
 * This implementation returns the average of the Front Left and Front Right channels. */
int AlsaVolume::getVolume()
{
	long aleft = 0;
	long aright = 0;
	if (m_masterElement != NULL) {
		snd_mixer_selem_get_playback_volume(m_masterElement, SND_MIXER_SCHN_FRONT_LEFT, &aleft);
		snd_mixer_selem_get_playback_volume(m_masterElement, SND_MIXER_SCHN_FRONT_RIGHT, &aright);
	}
	vol_level = ((aleft & 0xff) << 8) + (aright & 0xff);
	return (aleft + aright) >> 1;
}

/* Set the volume to the sound system.
 * This implementation sets the Front Left and Front Right channels to the specified value. */
void AlsaVolume::setVolume(int volume)
{
	if (m_masterElement != NULL) {
		long aleft = (volume & 0xff00) >> 8;
		long aright = volume & 0xff;
		snd_mixer_selem_set_playback_volume(m_masterElement, SND_MIXER_SCHN_FRONT_LEFT, aleft);
		snd_mixer_selem_set_playback_volume(m_masterElement, SND_MIXER_SCHN_FRONT_RIGHT, aright);
	}
}
//==================================================================================

/* Do a full redraw of the display. */
void AlsaVolume::updateVolumeData()
{
	/* Mute status. */
	is_muted =  isMuted();
	has_mute =  hasMute();

	/* Volume. */
	//int level = getVolume();
	getVolume();
}
//==================================================================================
#if 0
typedef enum {
	ALSA_SET_VOLUME,
	ALSA_GET_VOLUME
} AlsaAction;

enum { ALSA_FAILED = -1, ALSA_SUCCESS };
//==================================================================================

bool accessMasterVolume(long &result, snd_mixer_t *handle,
						AlsaAction action, int level)
{
	snd_mixer_selem_id_t *sid;
	snd_mixer_selem_channel_id_t channel;
	long min, max;
	const char *card = "default";
	const char *selem_name = "Master";

	if (snd_mixer_attach(handle, card) < 0) return false;;
	if (snd_mixer_selem_register(handle, NULL, NULL) < 0) return false;;
	if (snd_mixer_load(handle) < 0) return false;;

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);

	snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);
	if (!elem) return false;;

	snd_mixer_selem_set_playback_volume_range(elem, 0, 100);
	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	//snd_mixer_selem_set_playback_volume_all(elem, level * max / 100);

	if (action == ALSA_GET_VOLUME) {
		channel = (snd_mixer_selem_channel_id_t) 0;
		if (snd_mixer_selem_get_playback_volume(elem, channel, &result) < 0)
			return false;;
		/*
		long left = 0;
		long right = 0;
		snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &left);
		snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, &right);
		printf("%i, %i, %i, %i, %i\n", (int)min, (int)max, (int)result, (int)left, (int)right);
*/
		result -= min;
		max -= min;
		min = 0;
		result = 100 * result / max; // make the value bound from 0 to 100
	}
	else if (action == ALSA_SET_VOLUME) {
		if (level < 0 || level > 100) return false;;  // out of bounds

		level = (level * (max - min) / (100-1)) + min;

		channel = (snd_mixer_selem_channel_id_t) 0;
		if (snd_mixer_selem_set_playback_volume(elem, channel, level) < 0)
			return false;;

		channel = (snd_mixer_selem_channel_id_t) 0;
		if (snd_mixer_selem_set_playback_volume(elem, channel, level) < 0)
			return false;;
		//fprintf(stderr, "Set volume %i with status %i\n", (int) level, ret);
		result = ALSA_SUCCESS;
	}

	return true;
}

int handleMasterVolume(AlsaAction action, int level)
{
	long result = ALSA_FAILED;
	snd_mixer_t *handle;

	if (snd_mixer_open(&handle, 0) < 0) return result;

	accessMasterVolume(result, handle, action, level);
	snd_mixer_close(handle);

	return result;
}

int getVolume()
{
	long level = handleMasterVolume(ALSA_GET_VOLUME, 0);

	//level = 100;
	//printf("Ret %i\n", audio_volume(AUDIO_VOLUME_SET, &level));

	return level;
}

bool setVolume(long level)
{
	long result = handleMasterVolume(ALSA_SET_VOLUME, level);

	//level = 100;
	//printf("Ret %i\n", audio_volume(AUDIO_VOLUME_SET, &level));

	return (result == ALSA_SUCCESS);
}
#endif
