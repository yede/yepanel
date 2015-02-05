#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QVector>
#include <QDebug>

#include "volumeapplet.h"
#include "panelwindow.h"
#include "panel.h"
//==================================================================================

#define ICON_WIDTH    16
#define ICON_HEIGHT   16
#define SPACING        2
#define SPEAKER_X      0
#define SPEAKER_Y      0
#define SPEAKER_WIDTH  8
#define VOLUME_X      10
#define VOLUME_Y       0
#define VOLUME_WIDTH   5
#define VOLUME_HEIGHT 15
//==================================================================================

VolumeApplet::VolumeApplet(PanelWindow *panelWindow)
	: Applet(panelWindow)
	, m_pixmapVolume(":/ac/volume")
	, m_alsa(this)
	, m_vol(0)
	, m_muted(false)
{
	m_timer = new QTimer();
	m_timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateContent()));
}

VolumeApplet::~VolumeApplet()
{
}

bool VolumeApplet::start()
{
	QVector<const char *> elementNames;
	elementNames.append("Master");
	m_alsa.open(elementNames);

	setInteractive(true);

	return true;
}

bool VolumeApplet::stop()
{
	m_timer->stop();
	m_alsa.close();

	return true;
}

QSize VolumeApplet::desiredSize()
{
	int w, h;

	if (m_panelWindow->orientation() == Panel::Horizontal) {
		w = ICON_HEIGHT;
		h = m_panelWindow->height();
	} else {
		w = m_panelWindow->width();
		h = ICON_HEIGHT;
	}

	return QSize(w, h);
}

void VolumeApplet::updateContent()
{
	m_vol = m_alsa.getMasterVolume();
	m_muted = m_alsa.isMasterMuted();
	update();
	showHelp();
}

void VolumeApplet::onVolumeChanged(int socket)
{
	m_alsa.handleNotify(socket);

	m_timer->stop();
	m_timer->setInterval(20);
	m_timer->start();
}
//==================================================================================

void VolumeApplet::paintVal(QPainter *painter, int X, int Y, int W, int H, int val)
{
	const QColor &rgb = cfg().volumeColor;

	int L = H - 2;	// height of total value
	int v = 0;		// height of value

	if (val > 0) {
		v = L * val / 100;
		if (v == 0) v++;
	}

	painter->setBrush(QBrush(rgb));
	painter->drawRect(X, Y, W, H);	// draw outer frame

	if (v >= L) return;	// full value

	int h = L - v;
	int y = Y + 1;

	painter->setBrush(QBrush(cfg().meterColorBlack));
	painter->drawRect(X + 1, y, W - 2, h);
}

void VolumeApplet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
						 QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setPen(Qt::NoPen);
//	painter->setBrush(QBrush(cfg().colorGray));
//	painter->drawRect(m_outRect);

	int x = 0;
	int y = 1 + (m_size.height() - ICON_HEIGHT) / 2;
	int sx = SPEAKER_X;
	int sy = SPEAKER_Y;
	int sw = m_muted ? ICON_WIDTH : SPEAKER_WIDTH;
	int sh = ICON_HEIGHT;

	painter->drawPixmap(x, y, m_pixmapVolume, sx, sy, sw, sh);
	if (m_muted)
		return;

	x += VOLUME_X;
	y += VOLUME_Y;
	int w = VOLUME_WIDTH;
	int h = VOLUME_HEIGHT;

	paintVal(painter, x, y, w, h, m_vol);
}
//==================================================================================

void VolumeApplet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		event->accept();
		m_alsa.toggleMasterMute();
		updateContent();
	}
}

void VolumeApplet::wheelEvent(QGraphicsSceneWheelEvent *event)
{
	int vol = m_alsa.getMasterVolume();
	if (event->delta() > 0) {
		vol += 5;
		if (vol > 100) vol = 100;
	} else {
		vol -= 5;
		if (vol < 0) vol = 0;
	}
	m_alsa.setMasterVolume(vol);
	updateContent();
}

void VolumeApplet::showHelp()
{
	if (!m_hovering) return;

	QString tips = QString("%1: %2%  (%3: %4)")
				   .arg(tr("Master volume"))
				   .arg(m_vol)
				   .arg(tr("Muted"))
				   .arg(m_muted ? tr("Yes") : tr("No"));
	Applet::showHelp(tips);
}
