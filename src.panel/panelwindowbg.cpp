#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>

#include "panelwindowbg.h"
#include "panelwindow.h"
#include "panel.h"
//==================================================================================

PanelWindowBg::PanelWindowBg(PanelWindow *panelWindow)
	: m_panelWindow(panelWindow)
{
	setZValue(-10.0); // Background.
	setAcceptedMouseButtons(Qt::RightButton);
}

PanelWindowBg::~PanelWindowBg()
{
}

QRectF PanelWindowBg::boundingRect() const
{
	return QRectF(0.0, 0.0, m_panelWindow->width(), m_panelWindow->height());
}

void PanelWindowBg::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
						  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	const Panel &c = m_panelWindow->cfg();
	painter->drawPixmap(0, 0, m_panelWindow->width(), m_panelWindow->height(), c.pxBase);

/*
	painter->setPen(Qt::NoPen);
	painter->setBrush(QColor(0, 0, 0, 128));
	painter->drawRect(boundingRect());

	static const int borderThickness = 2;
	if (m_panelWindow->verticalAnchor() == Panel::Min)
	{
		QLinearGradient gradient(0.0, m_panelWindow->height() - borderThickness,
								 0.0, m_panelWindow->height());
		gradient.setSpread(QGradient::RepeatSpread);
		gradient.setColorAt(0.0, QColor(255, 255, 255, 0));
		gradient.setColorAt(1.0, QColor(255, 255, 255, 128));
		painter->setBrush(QBrush(gradient));
		painter->drawRect(0.0, m_panelWindow->height() - borderThickness,
						  m_panelWindow->width(), borderThickness);
	}
	else
	{
		QLinearGradient gradient(0.0, 0.0, 0.0, borderThickness);
		gradient.setSpread(QGradient::RepeatSpread);
		gradient.setColorAt(0.0, QColor(255, 255, 255, 128));
		gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
		painter->setBrush(QBrush(gradient));
		painter->drawRect(0.0, 0.0, m_panelWindow->width(), borderThickness);
	}
*/
}

void PanelWindowBg::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
}

void PanelWindowBg::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if (isUnderMouse()) {
		int x = static_cast<int>(event->pos().x());
		int y = static_cast<int>(event->pos().y());
		m_panelWindow->showPanelContextMenu(QPoint(x, y));
	}
}
