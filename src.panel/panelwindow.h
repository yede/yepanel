#ifndef PANELWINDOW_H
#define PANELWINDOW_H

#include <QtCore/QVector>
#include <QtGui/QWidget>
#include <QtGui/QGraphicsItem>
#include "panel.h"
//==================================================================================

class QFont;
class QGraphicsScene;
class QGraphicsView;
class Applet;
class PanelWindowBg;

class PanelWindow: public QWidget
{
	Q_OBJECT
public:
	PanelWindow();
	~PanelWindow();

	bool start();
	bool stop();

	void updatePosition();
	void updateLayout();
	void showPanelContextMenu(const QPoint &point);
	void resizeEvent(QResizeEvent *event);

	void setScreen(int screen);
	void setDockMode(bool dockMode);
	void setHorizontalAnchor(Panel::Anchor horizontalAnchor);
	void setVerticalAnchor(Panel::Anchor verticalAnchor);
	void setOrientation(Panel::Orientation orientation);
	void setLayoutPolicy(Panel::LayoutPolicy layoutPolicy);

	int                 screen()           const { return m_screen; }
	bool                dockMode()         const { return m_dockMode; }
	Panel::Anchor       horizontalAnchor() const { return m_horizontalAnchor; }
	Panel::Anchor       verticalAnchor()   const { return m_verticalAnchor; }
	Panel::Orientation  orientation()      const { return m_orientation; }
	Panel::LayoutPolicy layoutPolicy()     const { return m_layoutPolicy; }

	PanelWindowBg *baseItem()       const { return m_baseItem; }
	int            textBaseLine()   const;

	static const Panel &cfg();
	static const QFont &font();

	static const int defaultWidth = 512;
	static const int defaultHeight = 48;

private:
	bool m_dockMode;
	int m_screen;
	Panel::Anchor m_horizontalAnchor;
	Panel::Anchor m_verticalAnchor;
	Panel::Orientation m_orientation;
	Panel::LayoutPolicy m_layoutPolicy;

	QGraphicsScene *m_scene;
	QGraphicsView *m_view;
	PanelWindowBg *m_baseItem;
	QVector<Applet*> m_applets;
};

#endif
