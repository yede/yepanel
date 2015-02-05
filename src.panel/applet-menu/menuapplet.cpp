#include <QtGui/QMenu>
#include <QtGui/QStyle>
#include <QtGui/QGraphicsScene>
#include <QDebug>

#include "menuapplet.h"
#include "menuloader.h"
#include "panelwindow.h"
#include "appitem.h"
//==================================================================================

MenuApplet::MenuApplet(PanelWindow *panelWindow)
	: Applet(panelWindow)
	, m_menuOpened(false)
{
	AppMenu::m_menuApplet = this;
	AppMenu *appMenu = AppMenu::m_mainAppMenu = new AppMenu;

	appMenu->addMenu(tr("Settings"),    "Settings",    "preferences-desktop");
	appMenu->addMenu(tr("System"),      "System",      "preferences-system");
	appMenu->addMenu(tr("Network"),     "Network",     "applications-internet");
	appMenu->addMenu(tr("Office"),      "Office",      "applications-office");
	appMenu->addMenu(tr("Graphics"),    "Graphics",    "applications-graphics");
	appMenu->addMenu(tr("Multimedia"),  "AudioVideo",  "applications-multimedia");
	appMenu->addMenu(tr("Education"),   "Education",   "applications-science");
//	appMenu->addMenu(tr("Qt"),          "Qt",          "applications-development");
	appMenu->addMenu(tr("Development"), "Development", "applications-development");
	appMenu->addMenu(tr("Accessories"), "Utility",     "applications-accessories");
	appMenu->addMenu(tr("Games"),       "Game",        "applications-games");
	appMenu->addMenu(tr("Other"),       "Other",       "applications-other");
	appMenu->m_hasNewChild = true;

	createMenuUi(appMenu);
	m_menu = appMenu->m_menu;	// after createMenuUi()

	m_rootItem = new QGraphicsPixmapItem(this);
	m_rootItem->setPixmap(QIcon(":/24/start").pixmap(24));
}

MenuApplet::~MenuApplet()
{
	AppMenu *appMenu = AppMenu::m_mainAppMenu;
	AppMenu::m_mainAppMenu = 0;

	delete m_rootItem;
	delete appMenu;
}
//==================================================================================

bool MenuApplet::start()
{
	setInteractive(true);
	MenuLoader::instance()->startLoader();

	return true;
}

bool MenuApplet::stop()
{
	m_menu = 0;
	MenuLoader::instance()->stopLoader();

	return true;
}
//==================================================================================

QSize MenuApplet::desiredSize()
{
	return QSize(m_rootItem->boundingRect().size().width() + 16,
				 m_rootItem->boundingRect().size().height());
}

void MenuApplet::clicked()
{
	if (!m_menu) return;

	m_menuOpened = true;
	animateHighlight();

	m_menu->move(localToScreen(QPoint(0, m_size.height())));
	m_menu->exec();

	m_menuOpened = false;
	animateHighlight();
}

void MenuApplet::layoutChanged()
{
//	m_rootItem->setPos(8, m_panelWindow->textBaseLine());
	m_rootItem->setPos(8, ((m_panelWindow->height() - 24) >> 1) + 1);
}

bool MenuApplet::isHighlighted()
{
	return m_menuOpened || Applet::isHighlighted();
}
//==================================================================================

static QAction *getBeforeAction(QMenu *menu, const QString &title)
{
	QList<QAction*> actions = menu->actions();

	for (int i = 0; i < actions.size(); i++) {
		QAction *action = actions.at(i);
		if (action->text().compare(title, Qt::CaseInsensitive) > 0) {
			return action;
		}
	}

	return 0;
}

void MenuApplet::createAction(AppItem *appItem)
{
	if (appItem->hasUi()) return;

	QMenu *menu = appItem->getParentMenu();
	if (!menu) {
		qDebug() << "MenuApplet::createAction(): getParentMenu()=0";
		return;
	}

	QIcon icon = QIcon(QPixmap::fromImage(appItem->iconImage()));
	QString title = appItem->title();
	QAction *before = appItem->needSort() ? getBeforeAction(menu, title) : 0;
	QAction *action = new QAction(icon, title, menu);
//	action->setIconVisibleInMenu(true);
	menu->insertAction(before, action);
	appItem->setAction(action);

	QVariant v = QVariant::fromValue<void*>(appItem);
	action->setData(v);
	connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));
}

void MenuApplet::actionTriggered()
{
	QAction *action = static_cast<QAction*>(sender());
	QVariant v = action->data();
	AppItem *appItem = reinterpret_cast<AppItem*>(v.value<void*>());
	appItem->launch();
}
//==================================================================================
#define APP_MENU_ID "appMenuId"

void MenuApplet::menuAboutToShow()
{
	QMenu *menu = static_cast<QMenu*>(sender());
	QVariant v = menu->property(APP_MENU_ID);
	AppMenu *appMenu = reinterpret_cast<AppMenu*>(v.value<void*>());

	appMenu->createUi();
}

void MenuApplet::createMenuUi(AppMenu *appMenu)
{
	if (appMenu->hasUi()) return;

	QMenu *parentMenu = appMenu->getParentMenu();
	QMenu *menu;

	if (parentMenu) {
		QIcon icon = QIcon(QPixmap::fromImage(appMenu->iconImage()));
		menu = parentMenu->addMenu(icon, appMenu->title());
	} else {
		menu = new QMenu(appMenu->title());
	}
	appMenu->setMenu(menu);

	QVariant v = QVariant::fromValue<void*>(appMenu);
	menu->setProperty(APP_MENU_ID, v);

	connect(menu, SIGNAL(aboutToShow()), this, SLOT(menuAboutToShow()));
}

void MenuApplet::addSeparator(SepItem *setItem)
{
	if (setItem->hasUi()) return;

	QMenu *menu = setItem->getParentMenu();
	if (!menu) {
		return;
	}

	QAction *action = menu->addSeparator();
	setItem->setAction(action);
}
