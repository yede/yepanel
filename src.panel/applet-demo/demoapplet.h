#ifndef DEMOAPPLET_H
#define DEMOAPPLET_H

#include "applet.h"
//==================================================================================

class QGraphicsRectItem;

class DemoApplet: public Applet
{
	Q_OBJECT
public:
	DemoApplet(PanelWindow *panelWindow);
	~DemoApplet();

	bool start();
	bool stop();
	QSize desiredSize();

protected:
	void layoutChanged();

private:
	QGraphicsRectItem *m_rectItem;
};

#endif
