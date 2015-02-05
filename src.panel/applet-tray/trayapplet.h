#ifndef TRAYAPPLET_H
#define TRAYAPPLET_H

#include <QtCore/QVector>
#include <QtCore/QSize>
#include "applet.h"
//==================================================================================

class TrayItem;

class TrayApplet: public Applet
{
	Q_OBJECT
public:
	TrayApplet(PanelWindow *panelWindow);
	~TrayApplet();

	bool start();
	bool stop();
	QSize desiredSize();

	void registerTrayItem(TrayItem *trayItem);
	void unregisterTrayItem(TrayItem *trayItem);

protected:
	void layoutChanged();

private slots:
	void clientMessageReceived(unsigned long window, unsigned long atom, void *data);
	void windowClosed(unsigned long window);
	void windowReconfigured(unsigned long window, int x, int y, int width, int height);
	void windowDamaged(unsigned long window);

private:
	void updateLayout();

	bool m_initialized;
	QVector<TrayItem*> m_trayItems;
};

#endif
