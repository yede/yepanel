#ifndef NETWORKAPPLET_H
#define NETWORKAPPLET_H

#include <QHash>
#include "applet.h"
#include "helpwidget.h"
//==================================================================================

struct NetSpeed
{
	NetSpeed(unsigned long send, unsigned long recv)
		: lastSend(send), sendOut(0)
		, lastRecv(recv), receive(0) {}

	void setValues(unsigned long send, unsigned long recv)
	{
		sendOut = send - lastSend;
		receive = recv - lastRecv;

		lastSend = send;
		lastRecv = recv;
	}

	unsigned long lastSend, sendOut;
	unsigned long lastRecv, receive;
	HelpString name, send, recv;
};
//==================================================================================

class NetWorkApplet: public Applet
{
	Q_OBJECT
public:
	NetWorkApplet(PanelWindow *panelWindow);
	~NetWorkApplet();

	bool start();
	bool stop();
	QSize desiredSize();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
	void layoutChanged();

private slots:
	void updateContent();

private:
	void paintUi(QPainter *painter, int X, int Y, int W, int H, int val, bool up);
	bool readDevices();
	void scheduleUpdate();
	void setHelpInfo(NetSpeed *net);
	void initHelpInfo(const QString &name, NetSpeed *net, int y);
	void initHelpInfo();

private:
	int m_count;	// device count
	QTimer *m_timer;
	QRect m_outRect;
	QHash<QString, NetSpeed*> m_items;

	HelpInfo m_info;
	HelpString m_hsSend, m_hsRecv;
};

#endif
