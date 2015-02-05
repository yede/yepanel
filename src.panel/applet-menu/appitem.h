#ifndef APPITEM_H
#define APPITEM_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtGui/QImage>
#include <QtGui/QMenu>
//==================================================================================

class AppMenu;
class MenuApplet;

class AppNode
{
public:
	explicit AppNode(int nodeType);
	virtual ~AppNode();

	void setIconFile(const QString &iconFile);
	void setIconName(const QString &iconName);

	bool           needSort()      const;
	int            nodeType()      const { return m_nodeType; }
	const QString &title()         const { return m_title; }
	const QImage  &iconImage()     const { return m_iconImage; }
	QMenu         *getParentMenu() const;

private:
	friend class AppItem;
	friend class AppMenu;

	AppMenu *m_parent;
	int      m_role;
	int      m_nodeType;
	QString  m_title;
	QImage   m_iconImage;
};
//==================================================================================

class SepItem : public AppNode
{
public:
	explicit SepItem();
	virtual ~SepItem();

	bool hasUi() const;
	void setAction(QAction *action);

private:
	QAction *m_action;
};
//==================================================================================

class AppItem : public AppNode
{
public:
	explicit AppItem();
	virtual ~AppItem();

	bool hasUi() const;
	void setAction(QAction *action);
	void launch() const;

private:
	bool exists() const;

private:
	friend class AppNode;
	friend class AppMenu;

	QAction *m_action;
	QString m_exec;
};
//==================================================================================

class AppMenu : public AppNode
{
public:
	explicit AppMenu();
	virtual ~AppMenu();

	bool hasUi() const;
	void setMenu(QMenu *menu);
	void remove(AppNode *node);
	void deleteChildren();

	void createUi();
	static void createUi(AppNode *node);
	QAction *getNextAction(AppNode *node);

	AppMenu *addMenu(const QString &title, const QString &iconName);
	AppMenu *addMenu(const QString &title, const QString &category,
					 const QString &iconName);
	AppItem *addItem(const QString &title, const QString &exec,
					 const QString &icon);
	SepItem *addSeparator();

	static AppItem *addSystemItem(const QString &path);
	static AppMenu *addCustomNode(int type, const QString &name, const QString &icon, const QString &exec, AppMenu *parent);
//	static void addCustomNode(const QString &menu, int type, const QString &name, const QString &exec, const QString &icon);
	static void clearCustomMenu();
	static void clearSystemMenu(const QList<AppItem *> &clearList);

	static AppMenu *findByCategory(const QStringList &categories);
	static AppMenu *findParentMenu(const QString &title);

	static MenuApplet *menuApplet() { return m_menuApplet; }

private:
	void clearSystemItems(const QList<AppItem *> &clearList);
	AppMenu *doFindByCategory(const QStringList &categories);
	AppMenu *doFindParentMenu(const QString &title);

private:
	friend class AppNode;
	friend class AppItem;
	friend class MenuApplet;

	static MenuApplet *m_menuApplet;
	static AppMenu *m_mainAppMenu;

	QString m_category;
	QMenu *m_menu;
	bool m_hasNewChild;		// that's QAction or QMenu need to be created
	QList<AppNode *> m_children;
};
//==================================================================================

#endif
