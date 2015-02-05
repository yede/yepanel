#ifndef APPLET_H
#define APPLET_H

#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtGui/QGraphicsItem>
//==================================================================================

struct Margin
{
	Margin(int _left, int _top, int _right, int _bottom)
		: left(_left), top(_top), right(_right), bottom(_bottom) {}
	Margin(int horz, int vert)
		: left(horz), top(vert), right(horz), bottom(vert) {}
	Margin(): left(0), top(0), right(0), bottom(0) {}

	void setValue(int _left, int _top, int _right, int _bottom)
	{
		left = _left; top = _top; right = _right; bottom = _bottom;
	}

	void setValue(int horz, int vert)
	{
		left = horz; top = vert; right = horz; bottom = vert;
	}

	int left, top, right, bottom;
};

//==================================================================================

class QStyleOptionGraphicsItem;
class HelpInfo;
class HelpWidget;
class PanelWindow;
class Panel;

class Applet: public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	Applet(PanelWindow *panelWindow);
	~Applet();

	virtual bool start();
	virtual bool stop();

	virtual QSize desiredSize() = 0;
	QRectF boundingRect() const;
	QSize calculateMeterSize(QRect &outRect, int count, int unit);

	void setPaddings(int left, int top, int right, int bottom);
	void setPaddings(int left, int top);
	void setMargins(int left, int top, int right, int bottom);
	void setMargins(int left, int top);

	int xPaddings() const { return m_padding.left + m_padding.right; }
	int yPaddings() const { return m_padding.top + m_padding.bottom; }

	int xMargins() const { return m_margin.left + m_margin.right; }
	int yMargins() const { return m_margin.top + m_margin.bottom; }

	int xAddons() const { return xPaddings() + xMargins(); }
	int yAddons() const { return yPaddings() + yMargins(); }

	virtual void showHelp();
	static void showHelp(const QString &text);
	static void showHelp(const QStringList &strs);
	static void showHelp(const HelpInfo &info);

	static const Panel &cfg();
	PanelWindow *panelWindow() { return m_panelWindow; }
	const QSize &size() const { return m_size; }
	void setSize(const QSize &size);
	void setPosition(const QPoint &position);
	void setBound(int x, int y, int width, int height);
	void setInteractive(bool interactive);

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public slots:
	void animateHighlight();
	virtual void clicked();

protected:
	virtual void layoutChanged();
	QPoint localToScreen(const QPoint &point);

	virtual bool isHighlighted();

	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

	PanelWindow *m_panelWindow;
	QPoint m_position;
	QSize  m_size;
	Margin m_margin;
	Margin m_padding;
	qreal m_highlightIntensity;
	bool m_interactive;
	bool m_hovering;
};

#endif
