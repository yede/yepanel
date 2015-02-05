#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsScene>
#include <QDebug>

#include "clockapplet.h"
#include "panelwindow.h"
#include "panel.h"
//==================================================================================

ClockApplet::ClockApplet(PanelWindow *panelWindow)
	: Applet(panelWindow)
{
	setMargins(0, 0);
	setPaddings(0, 0, 4, 0);

	m_timer = new QTimer();
	m_timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateContent()));
}

ClockApplet::~ClockApplet()
{
	delete m_timer;
}

bool ClockApplet::start()
{
	updateContent();
	setInteractive(true);
	return true;
}

bool ClockApplet::stop()
{
	return true;
}

QSize ClockApplet::desiredSize()
{
	QString sample = QString("88:88");
	QFontMetrics fm(Applet::cfg().clockFont);
	int textWidth = fm.width(sample);
	int w, h;

	if (m_panelWindow->orientation() == Panel::Horizontal) {
		w = textWidth + xAddons();
		h = m_panelWindow->height();
		m_x = 0;
		m_y = (h - fm.height()) / 2 + fm.ascent() + 1;
	} else {
		w = m_panelWindow->width();
		h = textWidth + yAddons();
		m_x = 0;
		m_y = 0;
	}

	return QSize(w, h);
}

void ClockApplet::layoutChanged()
{
}

void ClockApplet::updateContent()
{
	QDateTime dateTimeNow = QDateTime::currentDateTime();
	QString text = dateTimeNow.toString("hh:mm");
	if (m_text != text) {
		m_text = text;
		update();
	}
	if (m_hovering) {
		text = dateTimeNow.toString("dddd yyyy-MM-dd hh:mm:ss");
		showHelp(text);
	}
	scheduleUpdate();
}

void ClockApplet::scheduleUpdate()
{
	m_timer->setInterval(1000 - QDateTime::currentDateTime().time().msec());
	m_timer->start();
}

void ClockApplet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setFont(Applet::cfg().clockFont);
	painter->setPen(QPen(Applet::cfg().clockShadowColor));
	painter->drawText(m_x + 1, m_y + 1, m_text);
	painter->setPen(QPen(Applet::cfg().clockTextColor));
	painter->drawText(m_x, m_y, m_text);
}

