#ifndef VOLUME_ALSA_H
#define VOLUME_ALSA_H

#include <alsa/asoundlib.h>
#include <poll.h>
#include <QSocketNotifier>
//==================================================================================

#define NUM_VOLMON   3
#define TIMER_VAL    1000

class VolumeApplet;

struct AlsaVolume {
    Plugin   *plug;

    GtkWidget *main;
    GtkWidget *iwin;

	bool iwin_showing;
	bool scroll_lock;

    char *action;
    int timer;
    int itest;

    int vol_level;
	bool is_muted;
	bool has_mute;
	bool snd_ctrl_on;

    cairo_surface_t *surf_alsa;
    CbRgba *rgb_non,  *rgb_snd;

    int pad_x, pad_y;
    int width, height;	// main size
    int x, y, w, h;		    // for cairo_set_source_surface

	AlsaVolume(VolumeApplet *applet);
	~AlsaVolume();

	int getVolume();
	void setVolume(int volume);
	void volumeChanged(int socket);

	bool initialize();
	void finalize();

private:
	bool findElement(const char *ename);
	bool hasMute() const;
	bool isMuted() const;
	void updateVolumeData();

	VolumeApplet         *m_applet;
	bool                  m_initialized;
	snd_mixer_t          *m_mixer;			/* The mixer */
	snd_mixer_selem_id_t *m_sid;			/* The element ID */
	snd_mixer_elem_t     *m_masterElement;	/* The Master element */
};
//==================================================================================

//int getVolume();
//bool setVolume(long level);
//==================================================================================
#endif
