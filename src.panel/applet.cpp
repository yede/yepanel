#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QDebug>

#include "applet.h"
#include "helpwidget.h"
#include "panelwindow.h"
#include "panelwindowbg.h"
#include "animationutils.h"
#include "panel.h"
//==================================================================================

Applet::Applet(PanelWindow *panelWindow)
	: m_panelWindow(panelWindow)
	, m_highlightIntensity(0.0)
	, m_interactive(false)
	, m_hovering(false)
{
	setZValue(-1.0);
	setAcceptedMouseButtons(Qt::RightButton);
	setParentItem(m_panelWindow->baseItem());
}

Applet::~Applet()
{
}

const Panel &Applet::cfg()
{
	return PanelWindow::cfg();
}

bool Applet::start()
{
//	m_panelWindow->updateLayout();
	return true;
}

bool Applet::stop()
{
	return true;
}

void Applet::setPaddings(int left, int top, int right, int bottom)
{
	m_padding.setValue(left, top, right, bottom);
}

void Applet::setPaddings(int left, int top)
{
	m_padding.setValue(left, top);
}

void Applet::setMargins(int left, int top)
{
	m_margin.setValue(left, top);
}

void Applet::setMargins(int left, int top, int right, int bottom)
{
	m_margin.setValue(left, top, right, bottom);
}

void Applet::setPosition(const QPoint &position)
{
	m_position = position;
	setPos(m_position);
}

void Applet::setSize(const QSize &size)
{
	m_size = size;
	layoutChanged();
}

void Applet::setBound(int x, int y, int width, int height)
{
	bool posChanged = (m_position.x() != x || m_position.y() != y);
	bool sizeChanged = (m_size.width() != width || m_size.height() != height);
	if (posChanged) {
		m_position.setX(x);
		m_position.setY(y);
	}
	if (sizeChanged) {
		m_size.setWidth(width);
		m_size.setHeight(height);
	}
//	if (posChanged)
		setPos(m_position);
//	if (sizeChanged)
		layoutChanged();
}

void Applet::setInteractive(bool interactive)
{
	m_interactive = interactive;

	if (m_interactive)
	{
		setAcceptsHoverEvents(true);
		setAcceptedMouseButtons(Qt::RightButton | Qt::LeftButton);
	}
	else
	{
		setAcceptsHoverEvents(false);
		setAcceptedMouseButtons(Qt::RightButton);
	}
}

QRectF Applet::boundingRect() const
{
	return QRectF(0.0, 0.0, m_size.width(), m_size.height());
}

QSize Applet::calculateMeterSize(QRect &outRect, int count, int unit)
{
	int margin  = cfg().meterMargin;
	int padding = cfg().meterPadding;
	int spacing = cfg().meterSpacing;
	setPaddings(padding, padding);
	int w, h;

	if (m_panelWindow->orientation() == Panel::Horizontal) {
		setMargins(0, margin);
		w = unit * count + spacing * (count - 1) + xAddons();
		h = m_panelWindow->height();
	} else {
		setMargins(margin, 0);
		w = m_panelWindow->width();
		h = unit * count + spacing * (count - 1) + yAddons();
	}

	outRect = QRect(m_margin.left, m_margin.top,
					w - m_margin.left - m_margin.right,
					h - m_margin.top - m_margin.bottom + 1);

	return QSize(w, h);
}

void Applet::showHelp(const QString &text)
{
	HelpWidget::showHelp(text);
}

void Applet::showHelp(const QStringList &strs)
{
	HelpWidget::showHelp(strs);
}

void Applet::showHelp(const HelpInfo &info)
{
	HelpWidget::showHelp(info);
}

void Applet::showHelp()
{
}
//==================================================================================

void Applet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
				   QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	if (m_size.width() < 32)
		return; // Too small to draw a background (don't want to deal with weird corner cases).

	if (!m_interactive)
		return; // Currently, background is only used for highlight on interactive applets.

	painter->setPen(Qt::NoPen);
	qreal radius = (m_size.width() * m_size.width() +
					m_size.height() * m_size.height()) / (4.0 * m_size.height());
	QPointF center(m_size.width() / 2.0,
				   m_size.height() + radius - m_size.height() / 2.0);
	static const qreal radiusInc = 10.0;
	QRadialGradient gradient(center, radius + radiusInc, center);
	QColor highlightColor(255, 255, 255,
						  static_cast<int>(150 * m_highlightIntensity));
	gradient.setColorAt(0.0, highlightColor);
	gradient.setColorAt((radius - m_size.height() / 2.0) / (radius + radiusInc),
						highlightColor);
	gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
	painter->setBrush(QBrush(gradient));
	painter->drawRect(boundingRect());
}

void Applet::animateHighlight()
{
	static const qreal highlightAnimationSpeed = 0.15;
	qreal targetIntensity = isHighlighted() ? 1.0 : 0.0;
	bool needAnotherStep = false;
	m_highlightIntensity = AnimationUtils::animate(m_highlightIntensity,
												   targetIntensity,
												   highlightAnimationSpeed,
												   needAnotherStep);
	if (needAnotherStep)
		QTimer::singleShot(20, this, SLOT(animateHighlight()));
	update();
}

void Applet::clicked()
{
}

void Applet::layoutChanged()
{
}

QPoint Applet::localToScreen(const QPoint &point)
{
	return m_panelWindow->pos() + m_position + point;
}

bool Applet::isHighlighted()
{
	return isUnderMouse();
}

void Applet::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
//	qDebug() << "hoverEnterEvent" << m_hovering;
	Q_UNUSED(event);
	m_hovering = true;
	animateHighlight();
	HelpWidget::calcPosition(pos(), m_panelWindow);
	showHelp();
}

void Applet::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
//	qDebug() << "hoverLeaveEvent" << m_hovering;
	Q_UNUSED(event);
	m_hovering = false;
	animateHighlight();
	HelpWidget::closeHelp();
}

void Applet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
}

void Applet::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if (isUnderMouse())
	{
		if (event->button() == Qt::LeftButton)
		{
			// FIXME: Workaround.
			// For some weird reason, if clicked() function is called directly, and menu is opened,
			// this item will receive hover enter event on menu close. But it shouldn't (mouse is outside).
			// Probably somehow related to taking a mouse grab when one is already active.
			QTimer::singleShot(1, this, SLOT(clicked()));
		}
		if (event->button() == Qt::RightButton)
		{
			m_panelWindow->showPanelContextMenu(m_position + QPoint(static_cast<int>(event->pos().x()), static_cast<int>(event->pos().y())));
		}
	}
}
