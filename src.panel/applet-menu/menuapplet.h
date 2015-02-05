#ifndef MENUAPPLET_H
#define MENUAPPLET_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtGui/QAction>
#include <QtGui/QPlastiqueStyle>
#include "applet.h"
//==================================================================================

class AppMenu;
class AppItem;
class SepItem;

class MenuApplet: public Applet
{
	Q_OBJECT
public:
	MenuApplet(PanelWindow *panelWindow);
	~MenuApplet();

	bool start();
	bool stop();

	QSize desiredSize();
	void clicked();

	void createAction(AppItem *appItem);
	void createMenuUi(AppMenu *appMenu);
	void addSeparator(SepItem *setItem);

protected:
	void layoutChanged();
	bool isHighlighted();

private slots:
	void actionTriggered();
	void menuAboutToShow();

private:
	friend class AppMenu;

	bool m_menuOpened;
	QMenu *m_menu;
	QGraphicsPixmapItem *m_rootItem;
};

#endif
