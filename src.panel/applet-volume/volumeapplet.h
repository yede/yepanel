#ifndef VOLUMEAPPLET_H
#define VOLUMEAPPLET_H

#include "applet.h"
#include "alsamixer.h"
//==================================================================================

//class AlsaVolume;

class VolumeApplet: public Applet
{
	Q_OBJECT
public:
	VolumeApplet(PanelWindow *panelWindow);
	~VolumeApplet();

	bool start();
	bool stop();
	QSize desiredSize();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
			   QWidget *widget);
	void showHelp();

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void wheelEvent(QGraphicsSceneWheelEvent *event);

private:
	void paintVal(QPainter *painter, int X, int Y, int W, int H, int val);

private slots:
	void updateContent();
	void onVolumeChanged(int socket);

private:
	QPixmap m_pixmapVolume;
	AlsaMixer m_alsa;
	int m_vol;
	bool m_muted;
	QTimer *m_timer;
};

#endif
