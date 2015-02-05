#ifndef ACPIAPPLET_H
#define ACPIAPPLET_H

#include <QHash>
#include "applet.h"
//==================================================================================

struct AcpiBattery;
struct AcpiAcpower;

class AcpiApplet: public Applet
{
	Q_OBJECT
public:
	AcpiApplet(PanelWindow *panelWindow);
	~AcpiApplet();

	bool start();
	bool stop();
	QSize desiredSize();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
			   QWidget *widget);

private:
	void paintUi(QPainter *painter, int X, int Y, int W, int H, int val);
	void readBatteries(const QStringList &names, const QString &path, QStringList &tips);
	void readAcpower(const QStringList &names, const QString &path, QStringList &tips);
	void readThermal(const QStringList &names, const QString &path, QStringList &tips);
	void readFans(const QStringList &names, const QString &path, QStringList &tips);
	bool readDevices();

protected:

private slots:
	void updateContent();

private:
	QPixmap m_pixmapAc;
	QPixmap m_pixmapOffline;
	QPixmap m_pixmapBattery;
	int m_batteryCount;
	QTimer *m_timer;
	QHash<QString, AcpiBattery*> m_batteries;
	QHash<QString, AcpiAcpower*> m_acpowers;
};

#endif
