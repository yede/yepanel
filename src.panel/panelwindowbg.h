#ifndef PANELWINDOWBG_H
#define PANELWINDOWBG_H

#include <QtGui/QWidget>
#include <QtGui/QGraphicsItem>
//==================================================================================

class QFont;
class QGraphicsScene;
class QGraphicsView;
class Applet;
class PanelWindow;

class PanelWindowBg: public QGraphicsItem
{
public:
	PanelWindowBg(PanelWindow *panelWindow);
	~PanelWindowBg();

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
			   QWidget *widget);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
	PanelWindow *m_panelWindow;
};

#endif
