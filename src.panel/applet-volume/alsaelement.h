#ifndef ALSAELEMENT_H
#define ALSAELEMENT_H

#include <QVector>
#include <alsa/asoundlib.h>
//==================================================================================

class AlsaChannel;

class AlsaElement
{
public:
	AlsaElement(snd_mixer_t *mixer, const char *elementName);
	~AlsaElement();

	void setVolumePercent(double percent);
	double volumePercent() const;
	bool isMuted() const;
	void toggleMute() const;

private:
	snd_mixer_elem_t *m_mixer;
	long m_hi, m_lo;
	QVector<AlsaChannel *> m_channels;
};

#endif
