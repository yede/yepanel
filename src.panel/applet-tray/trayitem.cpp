#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QDebug>

#include "trayapplet.h"
#include "trayitem.h"
#include "panelapplication.h"
#include "panelwindow.h"
#include "x11support.h"
//==================================================================================

#define ITEM_SIZE  24
#define SPACING    2
//==================================================================================

TrayItem::TrayItem(TrayApplet *trayApplet, unsigned long window)
	: m_trayApplet(trayApplet), m_window(window)
	, m_pixmapDamaged(false)
{
	setParentItem(m_trayApplet);

	// This is needed for non-composited tray icons, otherwise we'll get a BadMatch on reparent attempt.
	// Doesn't affect composited icons.
	X11Support::setWindowBackgroundBlack(m_window);

	X11Support::registerForTrayIconUpdates(m_window);
	X11Support::reparentWindow(m_window, m_trayApplet->panelWindow()->winId());
	X11Support::resizeWindow(m_window, ITEM_SIZE, ITEM_SIZE);
	X11Support::redirectWindow(m_window);
	X11Support::mapWindow(m_window);

	m_trayApplet->registerTrayItem(this);
}

TrayItem::~TrayItem()
{
	X11Support::reparentWindow(m_window, X11Support::rootWindow());

	m_trayApplet->unregisterTrayItem(this);
}

void TrayItem::setPosition(const QPoint &position)
{
	int x = position.x();
	int y = position.y();
	setPos(x, y);

	x += static_cast<int>(m_trayApplet->pos().x()) +
		 (m_size.width() - ITEM_SIZE) / 2;
	y += static_cast<int>(m_trayApplet->pos().y()) +
		 (m_size.height() - ITEM_SIZE) / 2;
	X11Support::moveWindow(m_window, x, y);
}

void TrayItem::setSize(const QSize &size)
{
	m_size = size;
	update();
}

QRectF TrayItem::boundingRect() const
{
	return QRectF(0.0, 0.0, m_size.width() - 1, m_size.height() - 1);
}
//int g_count = 0;

void TrayItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
					 QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	if (m_pixmapDamaged || m_pixmap.isNull()) {
		m_pixmapDamaged = false;
		m_pixmap = X11Support::getWindowPixmap(m_window);
		if (m_pixmap.isNull()) {
			qDebug() << "TrayItem::paint: pixmap.isNull()" << m_window;
			return;
		}
	}

	// Background.
	painter->setPen(Qt::NoPen);
	QPointF center(m_size.width()/2.0, m_size.height()/2.0);
	QRadialGradient gradient(center, ITEM_SIZE / 2.0, center);
	gradient.setColorAt(0.0, QColor(255, 255, 255, 80));
	gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
	painter->setBrush(QBrush(gradient));
	painter->drawRect(boundingRect());

//	const Panel &c = PanelApplication::instance()->panel();
	int sz = 22;

	// Icon itself.
	int x = (m_size.width() - sz) / 2;
	int y = (m_size.height() - sz) / 2;

	painter->drawPixmap(x, y, sz, sz, m_pixmap);
}
