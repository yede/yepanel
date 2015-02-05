#ifndef TASKITEM_H
#define TASKITEM_H

#include <QtCore/QVector>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsPixmapItem>
//==================================================================================

class TextGraphicsItem;
class TaskApplet;
class TaskClient;

// A TaskItem is a button in the task-bar.
// A TaskItem may own several clients.
// A client may be a single running program, or a stand-alone window opened
// by some multi-window applications, e.g. gimp, dia.

class TaskItem: public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	TaskItem(TaskApplet *taskApplet, TaskClient *client);
	~TaskItem();

	void updateButtonState(int state);
	void updateContent();

	void addClient(TaskClient *client);
	void removeClient(TaskClient *client);

	void setGeometry(const QPoint &targetPosition, const QSize &targetSize, bool instantly);
	void moveInstantly();
	void startAnimation();

	const QVector<TaskClient*> &clients() const { return m_clients; }
	int count() const;
	bool isActive() const;
	int pagerIndex() const;

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
	void updateClientsIconGeometry();
	bool isUrgent() const;
	void paintButton(QPainter *painter, const QPixmap &base, const QPixmap &left, const QPixmap &right);

public slots:
	void animate();
	void close();

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void wheelEvent(QGraphicsSceneWheelEvent *event);

private:
	TaskApplet *m_taskApplet;
	int  m_state;
	bool m_hovering;
	bool m_pressed;
	bool m_dragging;
	qreal m_highlightIntensity;
	qreal m_urgencyHighlightIntensity;

	QTimer *m_animationTimer;
//	TextGraphicsItem *m_textItem;
//	QGraphicsPixmapItem *m_iconItem;
	QVector<TaskClient*> m_clients;
	QPoint m_position;
	QPoint m_targetPosition;
	QSize m_size;
	QSize m_targetSize;
	QPointF m_mouseDownPosition;
	QPoint m_dragStartPosition;
};

#endif
