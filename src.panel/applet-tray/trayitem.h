#ifndef TRAYITEM_H
#define TRAYITEM_H

#include <QtCore/QVector>
#include <QtCore/QRectF>
#include <QtCore/QSize>
#include <QtGui/QGraphicsItem>
//==================================================================================

class TrayApplet;

class TrayItem: public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	TrayItem(TrayApplet *trayApplet, unsigned long window);
	~TrayItem();

	void setPosition(const QPoint &position);
	void setSize(const QSize &size);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
			   QWidget *widget);

	unsigned long window() const
	{
		return m_window;
	}

	void setPixmapDamaged() { m_pixmapDamaged = true; }

private:
	QSize m_size;
	TrayApplet *m_trayApplet;
	unsigned long m_window;
	QPixmap m_pixmap;
	bool    m_pixmapDamaged;
};

#endif
