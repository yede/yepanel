#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QProcess>
#include <QtGui/QPixmap>
#include <QDebug>

#include "appitem.h"
#include "iconloader.h"
#include "menuapplet.h"
#include "global.h"
#include "panel.h"
//==================================================================================

namespace NodeRole {
	enum { System, Custom };
}

//==================================================================================
// AppNode
//==================================================================================

AppNode::AppNode(int nodeType)
	: m_parent(0)
	, m_role(NodeRole::System)
	, m_nodeType(nodeType)
{
}

AppNode::~AppNode()
{
	if (m_parent)
		m_parent->remove(this);
}

void AppNode::setIconFile(const QString &iconFile)
{
	m_iconImage = IconLoader::loadFromFile(iconFile, Applet::cfg().menuIconSize);
}

void AppNode::setIconName(const QString &iconName)
{
	m_iconImage = IconLoader::loadIcon(iconName, Applet::cfg().menuIconSize);
}

bool AppNode::needSort() const
{
	return (m_role == NodeRole::System);
}

QMenu *AppNode::getParentMenu() const
{
	if (m_parent) return m_parent->m_menu;
	if (!AppMenu::m_mainAppMenu) return 0;
	return AppMenu::m_mainAppMenu->m_menu;
}

//==================================================================================
// SepItem
//==================================================================================

SepItem::SepItem()
	: AppNode(NodeType::SepItem)
	, m_action(0)
{
}

SepItem::~SepItem()
{
	if (m_action) delete m_action;
}

bool SepItem::hasUi() const
{
	return (m_action != 0);
}

void SepItem::setAction(QAction *action)
{
	if (m_action) delete m_action;
	m_action = action;
}

//==================================================================================
// AppItem
//==================================================================================

AppItem::AppItem()
	: AppNode(NodeType::AppItem)
	, m_action(0)
{
}

AppItem::~AppItem()
{
	if (m_action) delete m_action;
}

bool AppItem::exists() const
{
	return QFile(m_exec).exists();
}

bool AppItem::hasUi() const
{
	return (m_action != 0);
}

void AppItem::setAction(QAction *action)
{
	if (m_action) delete m_action;
	m_action = action;
}

void AppItem::launch() const
{
	QString exec = m_exec;
	bool flag = true;

	// Handle special arguments.
	for (;;)
	{
		int argPos = exec.indexOf('%');
		if (argPos == -1)
			break;
		if (flag) {
			flag = false;
			qDebug() << "AppItem::launch()" << m_title << m_exec;
		}
		// For now, just remove them.
		int spacePos = exec.indexOf(' ', argPos);
		if (spacePos == -1)
			exec.resize(argPos);
		else
			exec.remove(argPos, spacePos - argPos);
	}

	exec = exec.trimmed();
	QStringList args = exec.split(' ');
	QString process = args[0];
	args.removeAt(0);
	QProcess::startDetached(process, args, getenv("HOME"));
}

//==================================================================================
// AppMenu
//==================================================================================

AppMenu::AppMenu()
	: AppNode(NodeType::AppMenu)
	, m_menu(0)
	, m_hasNewChild(false)
{
}

AppMenu::~AppMenu()
{
	deleteChildren();
	if (m_menu) delete m_menu;
}
//==================================================================================

bool AppMenu::hasUi() const
{
	return (m_menu != 0);
}

void AppMenu::setMenu(QMenu *menu)
{
	if (m_menu) delete m_menu;
	m_menu = menu;
}

void AppMenu::remove(AppNode *node)
{
	m_children.removeOne(node);
}

void AppMenu::deleteChildren()
{
	int n = m_children.size() - 1;
	for (int i = n; i >= 0; i--) {
		AppNode *node = m_children.at(i);
		delete node;
	}
	m_hasNewChild = false;
}

void AppMenu::createUi()
{
	if (!m_hasNewChild) return;

	foreach (AppNode *node, m_children)
		createUi(node);

	m_hasNewChild = false;
}

void AppMenu::createUi(AppNode *node)
{
	switch (node->m_nodeType) {
		case NodeType::AppMenu:
			m_menuApplet->createMenuUi(static_cast<AppMenu *>(node));  break;
		case NodeType::AppItem:
			m_menuApplet->createAction(static_cast<AppItem *>(node));  break;
		case NodeType::SepItem:
			m_menuApplet->addSeparator(static_cast<SepItem *>(node));  break;
	}
}
//==================================================================================

AppMenu *AppMenu::addMenu(const QString &title, const QString &iconName)
{
	return addMenu(title, QString(), iconName);
}

AppMenu *AppMenu::addMenu(const QString &title, const QString &category,
						  const QString &iconName)
{
	AppMenu *appMenu = new AppMenu;
	appMenu->m_parent = this;
	appMenu->m_title = title;
	appMenu->m_category = category;
	appMenu->setIconName(iconName);

	m_children.append(appMenu);

	return appMenu;
}

AppItem *AppMenu::addItem(const QString &title, const QString &exec,
						  const QString &icon)
{
	AppItem *appItem = new AppItem;
	appItem->m_parent = this;
	appItem->m_title = title;
	appItem->m_exec = exec;
	appItem->setIconName(icon);

	m_children.append(appItem);

	return appItem;
}

SepItem *AppMenu::addSeparator()
{
	SepItem *sepItem = new SepItem;
	sepItem->m_parent = this;

	m_children.append(sepItem);

	return sepItem;
}
//==================================================================================

AppItem *AppMenu::addSystemItem(const QString &path)
{
	QFile file(path);
	if (!file.exists()) return 0;
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return 0;

	bool isNoDisplay = false;
	QString title;
	QString exec;
	QString icon;
	QStringList categories;

	QTextStream in(&file);
	while (!in.atEnd()) {
		QString line = in.readLine();
		if (line[0] == '[') {
			if (line.contains("Desktop Entry")) continue;
			break; // We only process "Desktop Entry" here.
		}
		if (line[0] == '#') continue;
		QStringList list = line.split('=');
		if (list.size() < 2) continue;
		QString key = list[0];
		QString value = list[1];
		if (key == "NoDisplay" && value == "true") isNoDisplay = true;
		if (key == "Name") title = value;
		if (key == "Exec") exec = value;
		if (key == "Icon") icon = value;
		if (key == "Categories")
			categories = value.split(";", QString::SkipEmptyParts);
	}
	file.close();
	if (isNoDisplay) return 0;
	if (title.isEmpty() || exec.isEmpty() || categories.isEmpty()) return 0;

	AppMenu *appMenu = AppMenu::findByCategory(categories);
	if (!appMenu) return 0;

	appMenu->m_hasNewChild = true;
	return appMenu->addItem(title, exec, icon);
}

AppMenu *AppMenu::addCustomNode(int type, const QString &name, const QString &icon, const QString &exec, AppMenu *parent)
{
	if (!parent) parent = m_mainAppMenu;

	AppMenu *menu = 0;
	AppNode *node = 0;

	switch (type) {
		case NodeType::AppMenu: menu = parent->addMenu(name, icon); node = menu;  break;
		case NodeType::AppItem: node = parent->addItem(name, exec, icon); break;
		case NodeType::SepItem: node = parent->addSeparator();            break;
	}

	parent->m_hasNewChild = true;
	if (node) node->m_role = NodeRole::Custom;

	return menu;
}

/* void AppMenu::addCustomNode(const QString &menu, int type, const QString &name,
							const QString &exec, const QString &icon)
{
	AppMenu *appMenu = AppMenu::findParentMenu(menu);
	if (!appMenu) return;

	AppNode *node = 0;

	switch (type) {
		case NodeType::AppMenu: node = appMenu->addMenu(name, icon);       break;
		case NodeType::AppItem: node = appMenu->addItem(name, exec, icon); break;
		case NodeType::SepItem: node = appMenu->addSeparator();            break;
	}

	appMenu->m_hasNewChild = true;
	if (node) node->m_role = NodeRole::Custom;
} */

void AppMenu::clearCustomMenu()
{
	if (!m_mainAppMenu) return;

	int n = m_mainAppMenu->m_children.size() - 1;
	for (int i = n; i >= 0; i--) {
		AppNode *node = m_mainAppMenu->m_children.at(i);
		if (node->m_role != NodeRole::Custom) continue;
		delete node;
	}
}

void AppMenu::clearSystemMenu(const QList<AppItem *> &clearList)
{
	if (!m_mainAppMenu) return;

	int cnt = m_mainAppMenu->m_children.count();
	for (int i = 0; i < cnt; i++) {
		AppNode *node = m_mainAppMenu->m_children.at(i);
		if (node->m_role == NodeRole::System) {
			Q_ASSERT(node->m_nodeType == NodeType::AppMenu);
			static_cast<AppMenu *>(node)->clearSystemItems(clearList);
		}
	}
}

void AppMenu::clearSystemItems(const QList<AppItem *> &clearList)
{
	int n = m_children.size() - 1;
	for (int i = n; i >= 0; i--) {
		AppNode *node = m_children.at(i);
		Q_ASSERT(node->m_nodeType == NodeType::AppItem);
		AppItem *item = static_cast<AppItem *>(node);
		if (clearList.contains(item)) delete item;
	}
}

//==================================================================================

AppMenu *AppMenu::findByCategory(const QStringList &categories)
{
	if (!m_mainAppMenu) return 0;

	return m_mainAppMenu->doFindByCategory(categories);
}

AppMenu *AppMenu::findParentMenu(const QString &title)
{
	if (!m_mainAppMenu || title.isEmpty()) return m_mainAppMenu;

	return m_mainAppMenu->doFindParentMenu(title);
}

AppMenu *AppMenu::doFindByCategory(const QStringList &categories)
{
	int cnt = m_children.count();
	for (int i = 0; i < cnt; i++) {
		AppNode *node = m_children.at(i);
		if (node->m_nodeType != NodeType::AppMenu) continue;

		AppMenu *appMenu = static_cast<AppMenu *>(node);
		if (categories.contains(appMenu->m_category)) return appMenu;
	}

	if (cnt > 10) {
		AppNode *node = m_children.at(10);
		if (node->m_role == NodeRole::System &&
			node->m_nodeType == NodeType::AppMenu)
			return static_cast<AppMenu *>(node);
	}

	return 0;
}

AppMenu *AppMenu::doFindParentMenu(const QString &title)
{
	int cnt = m_children.count();
	for (int i = 0; i < cnt; i++) {
		AppNode *node = m_children.at(i);
		if (node->m_nodeType != NodeType::AppMenu) continue;

		AppMenu *appMenu = static_cast<AppMenu *>(node);
		if (!appMenu->m_category.isEmpty()) continue;

		if (appMenu->m_title == title) return appMenu;

		AppMenu *subMenu = appMenu->doFindParentMenu(title);
		if (subMenu) return subMenu;
	}
	return 0;
}
