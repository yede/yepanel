#ifndef CLOCKAPPLET_H
#define CLOCKAPPLET_H

#include "applet.h"
//==================================================================================

class QTimer;

class ClockApplet: public Applet
{
	Q_OBJECT
public:
	ClockApplet(PanelWindow *panelWindow);
	~ClockApplet();

	bool start();
	bool stop();
	QSize desiredSize();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
	void layoutChanged();

private slots:
	void updateContent();

private:
	void scheduleUpdate();

	QTimer *m_timer;
	QString m_text;
	int m_x, m_y;
};

#endif
