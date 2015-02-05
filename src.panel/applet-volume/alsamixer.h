#ifndef ALSAMIXER_H
#define ALSAMIXER_H

#include <QVector>
#include <alsa/asoundlib.h>
//==================================================================================

class VolumeApplet;
class AlsaElement;

class AlsaMixer
{
public:
	explicit AlsaMixer(VolumeApplet *applet);
	~AlsaMixer();

	bool open(const QVector<const char *> &elementNames);
	void close();
	void handleNotify(int socket);

	double getElementVolume(unsigned int index) const;
	void setElementVolume(unsigned int index, double volume);

	bool isElementMuted(unsigned int elementIndex) const;
	void toggleElementMute(unsigned int elementIndex);

	int getMasterVolume() const;
	void setMasterVolume(int volume);
	bool isMasterMuted() const;
	void toggleMasterMute();

private:
	void setupElements(const QVector<const char *> &elementNames);
	void setupNotifier();

private:
	VolumeApplet *m_applet;
	snd_mixer_t *m_mixer;
	int m_masterIndex;
	QVector<AlsaElement *> m_elements;
};

#endif
