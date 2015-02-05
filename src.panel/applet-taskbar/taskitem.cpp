#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QMenu>
#include <QDebug>

#include "taskclient.h"
#include "taskitem.h"
#include "taskapplet.h"
#include "textgraphicsitem.h"
#include "panelwindow.h"
#include "x11support.h"
#include "animationutils.h"
#include "panel.h"
#include "helpwidget.h"
//==================================================================================

TaskItem::TaskItem(TaskApplet *taskApplet, TaskClient *client)
	: QObject()
	, m_taskApplet(taskApplet)
	, m_state(ClientStat::Normal)
	, m_hovering(false)
	, m_pressed(false)
	, m_dragging(false)
	, m_highlightIntensity(0.0)
	, m_urgencyHighlightIntensity(0.0)
{
	m_animationTimer = new QTimer();
	m_animationTimer->setInterval(20);
	m_animationTimer->setSingleShot(true);
	connect(m_animationTimer, SIGNAL(timeout()), this, SLOT(animate()));

	setParentItem(m_taskApplet);
	setAcceptsHoverEvents(true);
	setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);

//	m_iconItem = new QGraphicsPixmapItem(this);
//	m_textItem = new TextGraphicsItem(this);
	updateButtonState(client->clientState());

	addClient(client);
	m_taskApplet->doRegTaskButton(this);
}

TaskItem::~TaskItem()
{
	HelpWidget::closeHelp();

//	delete m_iconItem;
//	delete m_textItem;
	delete m_animationTimer;
}

void TaskItem::addClient(TaskClient *client)
{
	if (m_clients.contains(client)) return;

	m_clients.append(client);
	updateClientsIconGeometry();
	updateContent();
}

void TaskItem::removeClient(TaskClient *client)
{
	int i = m_clients.indexOf(client);
	if (i < 0) return;

	m_clients.remove(i);
	updateContent();
}

void TaskItem::updateButtonState(int state)
{
//	qDebug() << "TaskItem::updateClientState" << state << (m_clients.isEmpty() ? 0 : m_clients.at(0)->handle());
/*
	if (state == ClientStat::Normal) {
		m_textItem->setColor(Applet::cfg().taskNormalColor);
		m_textItem->setFont(Applet::cfg().fontBold);
	}
	else if (state == ClientStat::Active) {
		m_textItem->setColor(Applet::cfg().taskActiveColor);
		m_textItem->setFont(Applet::cfg().fontBold);
	}
	else if (state == ClientStat::Iconic) {
		m_textItem->setColor(Applet::cfg().taskIconicColor);
		m_textItem->setFont(Applet::cfg().fontNormal);
	}
*/
	m_state = state;
	update();
}

void TaskItem::updateContent()
{
	if (m_clients.isEmpty())
		return;
/*
	int taskIconSize = Applet::cfg().taskIconSize;
	int iconStaySize = taskIconSize + 6;
	QFontMetrics fm(Applet::cfg().fontBold);
	QString shortName = fm.elidedText(m_clients[0]->name(), Qt::ElideRight,
									  m_targetSize.width() - iconStaySize);
	m_textItem->setText(shortName);
	m_textItem->setPos(iconStaySize, m_taskApplet->panelWindow()->textBaseLine());

	QPixmap pixmap = m_clients[0]->icon().pixmap(taskIconSize);
	m_iconItem->setPixmap(pixmap);
	m_iconItem->setPos(3.0, (m_targetSize.height() - pixmap.height()) / 2);
*/
	update();
}

void TaskItem::setGeometry(const QPoint &targetPosition, const QSize &targetSize, bool instantly)
{
	m_targetPosition = targetPosition;
	m_targetSize = targetSize;
	updateClientsIconGeometry();

	if (instantly) {
	//	updateContent();
		moveInstantly();
	}
}

void TaskItem::moveInstantly()
{
	m_position = m_targetPosition;
	m_size = m_targetSize;
	setPos(m_position.x(), m_position.y());
	update();
}

void TaskItem::startAnimation()
{
	if (!m_animationTimer->isActive())
		m_animationTimer->start();
}

void TaskItem::animate()
{
	bool needAnotherStep = false;

	static const qreal highlightAnimationSpeed = 0.15;
	qreal targetIntensity = isUnderMouse() ? 1.0 : 0.0;
	m_highlightIntensity = AnimationUtils::animate(m_highlightIntensity, targetIntensity, highlightAnimationSpeed, needAnotherStep);

	static const qreal urgencyHighlightAnimationSpeed = 0.015;
	qreal targetUrgencyIntensity = 0.0;
	if (isUrgent())
	{
		qint64 msecs = QDateTime::currentMSecsSinceEpoch() % 3000;
		if (msecs < 1500)
			targetUrgencyIntensity = 1.0;
		else
			targetUrgencyIntensity = 0.5;
		needAnotherStep = true;
	}
	m_urgencyHighlightIntensity = AnimationUtils::animate(m_urgencyHighlightIntensity, targetUrgencyIntensity, urgencyHighlightAnimationSpeed, needAnotherStep);

	if (!m_dragging)
	{
		static const int positionAnimationSpeed = 24;
		static const int sizeAnimationSpeed = 24;
		m_position.setX(AnimationUtils::animateExponentially(m_position.x(), m_targetPosition.x(), 0.2, positionAnimationSpeed, needAnotherStep));
		m_position.setY(AnimationUtils::animateExponentially(m_position.y(), m_targetPosition.y(), 0.2, positionAnimationSpeed, needAnotherStep));
		m_size.setWidth(AnimationUtils::animate(m_size.width(), m_targetSize.width(), sizeAnimationSpeed, needAnotherStep));
		m_size.setHeight(AnimationUtils::animate(m_size.height(), m_targetSize.height(), sizeAnimationSpeed, needAnotherStep));
		setPos(m_position.x(), m_position.y());
	}

	update();

	if (needAnotherStep)
		m_animationTimer->start();
}

void TaskItem::close()
{
	for (int i = 0; i < m_clients.size(); i++)
	{
		X11Support::closeWindow(m_clients[i]->handle());
	}
}

QRectF TaskItem::boundingRect() const
{
	return QRectF(0.0, 0.0, m_size.width() - 1, m_size.height());
}

void TaskItem::paintButton(QPainter *painter, const QPixmap &base, const QPixmap &left, const QPixmap &right)
{
	int x = left.width();
	int w = m_size.width() - x - right.width();
	int h = m_size.height();

	painter->drawPixmap(0, 0, left);
	painter->drawPixmap(x, 0, w, h, base);
	x += w;
	painter->drawPixmap(x, 0, right);
}

void TaskItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
					 QWidget *widget)
{
	Q_UNUSED(painter);
	Q_UNUSED(option);
	Q_UNUSED(widget);

	bool urgent = isUrgent();
	bool active = isActive();//m_state == ClientStat::Active;
//	if (!active && !urgent && !m_hovering) return;

	const Panel &c = Applet::cfg();

	if (active || m_pressed) {
		paintButton(painter, c.pxTaskActBase, c.pxTaskActLeft, c.pxTaskActRight);
	} else if (m_hovering) {
		paintButton(painter, c.pxTaskHovBase, c.pxTaskHovLeft, c.pxTaskHovRight);
	}

	if (urgent && m_urgencyHighlightIntensity > 0.001)
	{
		static const qreal roundRadius = 3.0;
		QPointF center(m_size.width() / 2.0, m_size.height() + 32.0);
		QRectF rect(0.0, 3.0, m_size.width(), m_size.height() - 5.0);
		QRadialGradient gradient(center, 200.0, center);
		int alpha = static_cast<int>(160 * m_urgencyHighlightIntensity);
		gradient.setColorAt(0.0, QColor(255, 100, 0, alpha));
		gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
		painter->setBrush(QBrush(gradient));
		painter->drawRoundedRect(rect, roundRadius, roundRadius);
	}

	int x, y, w, h, spacing = 2;
	int marginLeft = c.pxTaskActLeft.width();
	int marginRight = c.pxTaskActRight.width();
	int taskIconSize = c.taskIconSize;
	bool isHorz = m_taskApplet->panelWindow()->orientation() == Panel::Horizontal;

	QPixmap icon = m_clients[0]->icon().pixmap(taskIconSize);
	if (icon.isNull()) {
		icon = c.pxDefaultTaskIcon;
		if (icon.width() != taskIconSize)
			icon = icon.scaledToWidth(taskIconSize);
	}
	h = m_size.height();
	x = marginLeft;
	y = (h - icon.height()) / 2;
	if (isHorz) {
		y += c.taskIconOffset;
	} else {
		x += c.taskIconOffset;
	}
	painter->drawPixmap(x, y, icon);		// draw icon

	x = marginLeft + taskIconSize + spacing;
	y = 0;
	if (isHorz) {
		y += c.taskTextOffset;
	} else {
		x += c.taskTextOffset;
	}
	w = m_size.width() - x - marginRight;
	QFontMetrics fm(c.fontNormal);
	QString shortTx = fm.elidedText(m_clients[0]->name(), Qt::ElideRight, w, 0);
	QColor color = active ? c.taskActiveColor : c.taskNormalColor;
	painter->setPen(color);
	painter->drawText(x, y, w, h, Qt::AlignLeft | Qt::AlignVCenter, shortTx);	// draw text
//	qDebug() << "paint" << active << m_hovering << shortTx;
}

void TaskItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);
//	startAnimation();
	m_hovering = true;
	if (m_clients.size() > 0) {
		QPointF pos = QPointF(m_taskApplet->x() + this->x(), 0);
		HelpWidget::calcPosition(pos, m_taskApplet->panelWindow());
		HelpWidget::showHelp(m_clients.at(0)->name());
	}
	update();
}

void TaskItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);
//	startAnimation();
	m_hovering = false;
	HelpWidget::closeHelp();
	update();
}

void TaskItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_pressed = true;
		m_mouseDownPosition = event->scenePos();
		m_dragStartPosition = m_position;
		update();
	}
}

void TaskItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_pressed = false;
		if (m_dragging) {
			m_dragging = false;
			m_taskApplet->draggingStopped();
			setZValue(0.0); // No more on top.
			startAnimation(); // Item can be out of it's regular, start animation to bring it back.
			return;
		}
	}

	if (isUnderMouse())
	{
		if (m_clients.isEmpty())
			return;

		if (event->button() == Qt::LeftButton)
		{
			static const qreal clickMouseMoveTolerance = 10.0;

			if ((event->scenePos() - m_mouseDownPosition).manhattanLength() < clickMouseMoveTolerance)
			{
				if (m_taskApplet->activeWindow() == m_clients[0]->handle())
					X11Support::minimizeWindow(m_clients[0]->handle());
				else
					X11Support::activateWindow(m_clients[0]->handle());
			}
		}

		if (event->button() == Qt::RightButton)
		{
			QMenu menu;
			menu.addAction(QIcon::fromTheme("window-close"), "Close", this, SLOT(close()));
			menu.exec(event->screenPos());
		}
	}
}

void TaskItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	// Mouse events are sent only when mouse button is pressed.
	if (!m_pressed)
		return;

	QPointF pos = event->scenePos();

	if (!m_dragging) {
		if (qAbs(pos.x() - m_mouseDownPosition.x()) < 3 && qAbs(pos.y() - m_mouseDownPosition.y()) < 3) return;
		m_dragging = true;
		m_taskApplet->draggingStarted();
		setZValue(1.0); // Be on top when dragging.
	}

	// TODO: Vertical orientation support.

	QPointF delta = pos - m_mouseDownPosition;
	m_position.setX(m_dragStartPosition.x() + static_cast<int>(delta.x()));
	if (m_position.x() < 0)
		m_position.setX(0);
	if (m_position.x() >= m_taskApplet->size().width() - m_targetSize.width())
		m_position.setX(m_taskApplet->size().width() - m_targetSize.width());
	setPos(m_position.x(), m_position.y());

	int criticalShift = m_targetSize.width()*55/100;

	if (m_position.x() < m_targetPosition.x() - criticalShift)
		m_taskApplet->moveItem(this, false);

	if (m_position.x() > m_targetPosition.x() + criticalShift)
		m_taskApplet->moveItem(this, true);

	update();
}

void TaskItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
	if (m_clients.isEmpty()) return;

	if (event->delta() > 0) {
		X11Support::activateWindow(m_clients[0]->handle());
	} else {
		X11Support::minimizeWindow(m_clients[0]->handle());
	}
}

bool TaskItem::isUrgent() const
{
	for (int i = 0; i < m_clients.size(); i++) {
		if (m_clients[i]->isUrgent()) return true;
	}
	return false;
}

bool TaskItem::isActive() const
{
	foreach (TaskClient *client, m_clients) {
		if (client->isActive()) return true;
	}
	return false;
}

int TaskItem::pagerIndex() const
{
	return (m_clients.size() > 0) ? m_clients.at(0)->pagerIndex() : -1;
}

int TaskItem::count() const
{
	return m_clients.size();
}

/*
_NET_WM_ICON_GEOMETRY, x, y, width, height, CARDINAL[4]/32
This optional property MAY be set by stand alone tools like a taskbar or an iconbox.
It specifies the geometry of a possible icon in case the window is iconified.
Rationale: This makes it possible for a Window Manager to display a nice animation like morphing the window into its icon.
*/
void TaskItem::updateClientsIconGeometry()
{
	QPointF topLeft = m_taskApplet->mapToScene(m_targetPosition);
	QVector<unsigned long> values;
	values.resize(4);
	values[0] = static_cast<unsigned long>(topLeft.x()) + m_taskApplet->panelWindow()->pos().x();
	values[1] = static_cast<unsigned long>(topLeft.y()) + m_taskApplet->panelWindow()->pos().y();
	values[2] = m_targetSize.width();
	values[3] = m_targetSize.height();
	for (int i = 0; i < m_clients.size(); i++) {
		X11Support::setWindowPropertyCardinalArray(m_clients[i]->handle(), "_NET_WM_ICON_GEOMETRY", values);
	}
}
