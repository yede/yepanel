#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QPainter>
#include <QDebug>

#include "acpiapplet.h"
#include "panelwindow.h"
#include "panel.h"
#include "acpibattery.h"
#include "acpiacpower.h"
#include "acpiutils.h"
//==================================================================================

#define ICON_HEIGHT   16
#define SPACING        0

#define AC_WIDTH      11
#define AC_X           0
#define AC_Y           5
#define AC_OFFLINE_X   2
#define AC_OFFLINE_Y   0

#define BATTERY_WIDTH  9
#define VAL_HEIGHT    10
#define VAL_X          2
#define VAL_Y          4
//==================================================================================

AcpiApplet::AcpiApplet(PanelWindow *panelWindow)
	: Applet(panelWindow)
	, m_pixmapAc(":/ac/ac")
	, m_pixmapOffline(":/ac/deleted")
	, m_pixmapBattery(":/ac/battery")
	, m_batteryCount(1)
{
	m_timer = new QTimer();
	m_timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateContent()));
}

AcpiApplet::~AcpiApplet()
{
}

bool AcpiApplet::start()
{
	updateContent();
	setInteractive(true);

	return true;
}

bool AcpiApplet::stop()
{
	return true;
}

QSize AcpiApplet::desiredSize()
{
	int w, h, cnt = m_batteryCount;

	if (m_panelWindow->orientation() == Panel::Horizontal) {
		w = AC_WIDTH + BATTERY_WIDTH * cnt + SPACING * cnt;
		h = m_panelWindow->height();
	} else {
		w = m_panelWindow->width();
		h = AC_WIDTH + BATTERY_WIDTH * cnt + SPACING * cnt;
	}

	return QSize(w, h);
}

void AcpiApplet::updateContent()
{
	readDevices();

	if (m_batteries.size() == m_batteryCount) {
		update();
	} else {
		m_batteryCount = m_batteries.size();
		m_panelWindow->updateLayout();
	}

	m_timer->setInterval(1000);
	m_timer->start();
}
//==================================================================================

void AcpiApplet::paintUi(QPainter *painter, int X, int Y, int W, int H, int val)
{
	int v = 0;	// height of value

	if (val > 0) {
		v = H * val / 100;
		if (v == 0) v++;
	}

	int spH = H - v;	// height of space

	if (v > 0) {
		painter->setBrush(QBrush(cfg().batteryColor));
		painter->drawRect(X, Y + spH, W, v);
	}

	if (spH > 0) {
		painter->setBrush(QBrush(cfg().meterColorBlack));
		painter->drawRect(X, Y, W, spH);
	}
}

void AcpiApplet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
						  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setPen(Qt::NoPen);
//	painter->setBrush(QBrush(cfg().colorBlack));
//	painter->drawRect(m_outRect);

	int x0 = 0;
	int y0 = 1 + (m_size.height() - ICON_HEIGHT) / 2;

	int x = x0 + AC_X;
	int y = y0 + AC_Y;
	painter->drawPixmap(x, y, m_pixmapAc);	// draw ac icon
	//------------------------------------------------------------------------------

	bool online = false;
	if (!m_acpowers.isEmpty()) {
		AcpiAcpower *ac = m_acpowers.constBegin().value();
		online = (ac->online == 1);
	}
	if (!online) {
		x = x0 + AC_OFFLINE_X;
		y = y0 + AC_OFFLINE_Y;
		painter->drawPixmap(x, y, m_pixmapOffline);	// draw the "offline" icon
	}
	//------------------------------------------------------------------------------

	int w = BATTERY_WIDTH - VAL_X - VAL_X;
	int h = VAL_HEIGHT;
	x = x0 + AC_WIDTH;
	y = y0;

	QHash<QString, AcpiBattery*>::const_iterator i = m_batteries.constBegin();
	while (i != m_batteries.constEnd()) {
		AcpiBattery *battery = i.value();
		int val = battery->v_paint;
		paintUi(painter, x + VAL_X, y + VAL_Y, w, h, val);	// draw battery value
		painter->drawPixmap(x, y, m_pixmapBattery);			// draw battery icon
		x += BATTERY_WIDTH + SPACING;
		++i;
	}
}
//==================================================================================

bool AcpiApplet::readDevices()
{
	QString path;
	QStringList devices;
	QStringList tips;

	if (AcpiUtil::find(devices, path, AcpiDeviceType::AcAdapter))
		readAcpower(devices, path, tips);

	if (AcpiUtil::find(devices, path, AcpiDeviceType::Battery))
		readBatteries(devices, path, tips);

	if (AcpiUtil::find(devices, path, AcpiDeviceType::CoolingDevice))
		readThermal(devices, path, tips);

	if (AcpiUtil::find(devices, path, AcpiDeviceType::ThermalZone))
		readFans(devices, path, tips);

	if (m_hovering)
		showHelp(tips);

	return true;
}

void AcpiApplet::readBatteries(const QStringList &names, const QString &path,
							   QStringList &tips)
{
	foreach (QString name, names) {
		if (m_batteries.contains(name)) {
			QHash<QString, AcpiBattery*>::iterator i = m_batteries.find(name);
			AcpiBattery *bi = i.value();
			bi->read();
			if (bi->charge_state == AcpiChargeState::Charging) {
				bi->n_paint ++;
				if (bi->n_paint > 4) bi->n_paint = 1;
				bi->v_paint = bi->n_paint * 25;
			} else {
				bi->v_paint = bi->remaining_capacity * 100 / bi->last_capacity;
			}
			if (m_hovering) {
				tips.append(QString("%1: %2% (%3: %4)")
							.arg(name).arg(bi->percentage)
							.arg(tr("State")).arg(bi->state_text));
				tips.append(QString("   %1 - %2 %3")
							.arg(tr("Design capacity"))
							.arg(bi->design_capacity).arg(bi->capacity_unit));
				tips.append(QString("   %1 - %2 %3")
							.arg(tr("Last capacity"))
							.arg(bi->last_capacity).arg(bi->capacity_unit));
				tips.append(QString("   %1 - %2 %3")
							.arg(tr("Remaining capacity"))
							.arg(bi->remaining_capacity).arg(bi->capacity_unit));
				tips.append(QString("   %1 - %2")
							.arg(bi->present_rate).arg(bi->poststr));
				tips.append(QString("   %1 - %2 %3  %4 %5  %6 %7")
							.arg(tr("eta"))
							.arg(bi->hours < 0 ? QString("??")
											   : QString::number(bi->hours))
							.arg(tr("Hours"))
							.arg(bi->minutes < 0 ? QString("??")
												 : QString::number(bi->minutes))
							.arg(tr("Minutes"))
							.arg(bi->seconds < 0 ? QString("??")
												 : QString::number(bi->seconds))
							.arg(tr("Seconds")));
			}
		} else {
			AcpiBattery *battery = new AcpiBattery(name, path);
			m_batteries.insert(name, battery);
		}
	}
}

void AcpiApplet::readAcpower(const QStringList &names, const QString &path,
							 QStringList &tips)
{
	foreach (QString name, names) {
		if (m_acpowers.contains(name)) {
			QHash<QString, AcpiAcpower*>::iterator i = m_acpowers.find(name);
			AcpiAcpower *ac = i.value();
			ac->read();
			if (m_hovering) {
				tips.append(QString("%1: (%2)")
							.arg(name).arg(tr("Ac power")));
				tips.append(QString("   %1 - %2")
							.arg(tr("State"))
							.arg(ac->online ? tr("Online") : tr("Offline")));
			}
		} else {
			AcpiAcpower *ac = new AcpiAcpower(name, path);
			m_acpowers.insert(name, ac);
		}
	}
}

void AcpiApplet::readThermal(const QStringList &names, const QString &path,
							 QStringList &tips)
{
	Q_UNUSED(names);
	Q_UNUSED(path);
	Q_UNUSED(tips);
}

void AcpiApplet::readFans(const QStringList &names, const QString &path,
						  QStringList &tips)
{
	Q_UNUSED(names);
	Q_UNUSED(path);
	Q_UNUSED(tips);
}
