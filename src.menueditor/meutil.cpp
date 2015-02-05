#include "meutil.h"
#include "global.h"
//==================================================================================

MeUtil::MeUtil()
{
}

//==================================================================================
// treewidget
//==================================================================================

namespace PropRole {
	enum { Type = Qt::UserRole, Menu, Exec, Icon };
}
//==================================================================================

void MeUtil::setItemText(QTreeWidgetItem *item, const QString &text)
{
	if (item) item->setText(0, text);
}

QString MeUtil::getItemText(QTreeWidgetItem *item)
{
	return item ? item->text(0) : QString();
}

void MeUtil::setItemType(QTreeWidgetItem *item, int type)
{
	if (item) item->setData(0, PropRole::Type, type);
}

int MeUtil::getItemType(QTreeWidgetItem *item)
{
	return item ? item->data(0, PropRole::Type).toInt() : NodeType::AppMenu;
}

void MeUtil::setItemMenu(QTreeWidgetItem *item, const QString &menu)
{
	if (item) item->setData(0, PropRole::Menu, menu);
}

QString MeUtil::getItemMenu(QTreeWidgetItem *item)
{
	return item ? item->data(0, PropRole::Menu).toString() : QString();
}

void MeUtil::setItemExec(QTreeWidgetItem *item, const QString &exec)
{
	if (item) item->setData(0, PropRole::Exec, exec);
}

QString MeUtil::getItemExec(QTreeWidgetItem *item)
{
	return item ? item->data(0, PropRole::Exec).toString() : QString();
}

void MeUtil::setItemIcon(QTreeWidgetItem *item, const QString &icon)
{
	if (item) item->setData(0, PropRole::Icon, icon);
}

QString MeUtil::getItemIcon(QTreeWidgetItem *item)
{
	return item ? item->data(0, PropRole::Icon).toString() : QString();
}
//==================================================================================

QByteArray MeUtil::getItemTextByteArray(QTreeWidgetItem *item)
{
	return item ? item->text(0).toUtf8() : QByteArray();
}

QByteArray MeUtil::getItemIconByteArray(QTreeWidgetItem *item)
{
	return item ? item->data(0, PropRole::Icon).toByteArray() : QByteArray();
}

QByteArray MeUtil::getItemExecByteArray(QTreeWidgetItem *item)
{
	return item ? item->data(0, PropRole::Exec).toByteArray() : QByteArray();
}
//==================================================================================

int MeUtil::getItemPos(QTreeWidget *tree, QTreeWidgetItem *item)
{
	QTreeWidgetItem *parent = item->parent();
	return parent ? parent->indexOfChild(item)
				  : tree->indexOfTopLevelItem(item);
}

QTreeWidgetItem *MeUtil::getInsertPos(QTreeWidget *tree, int &pos)
{
	pos = -1;
	QTreeWidgetItem *item = tree->currentItem();
	if (!item) return 0;

	if (getItemType(item) == NodeType::AppMenu) return item;

	pos = getItemPos(tree, item);
	return item->parent();
}

//==================================================================================

static QTreeWidgetItem *getMenuNode(QTreeWidgetItem *parent, const QString &menu)
{
	int cnt = parent->childCount();
	for (int i = 0; i < cnt; i++) {
		QTreeWidgetItem *node = parent->child(i);
		if (MeUtil::getItemType(node) == NodeType::AppMenu &&
			MeUtil::getItemText(node) == menu) return node;
	}
	for (int i = 0; i < cnt; i++) {
		QTreeWidgetItem *child = parent->child(i);
		QTreeWidgetItem *node = getMenuNode(child, menu);
		if (node) return node;
	}
	return 0;
}

static QTreeWidgetItem *getMenuNode(QTreeWidget *tree, const QString &menu)
{
	if (menu.isEmpty()) return 0;

	int cnt = tree->topLevelItemCount();
	for (int i = 0; i < cnt; i++) {
		QTreeWidgetItem *node = tree->topLevelItem(i);
		if (MeUtil::getItemType(node) == NodeType::AppMenu &&
			MeUtil::getItemText(node) == menu) return node;
	}
	for (int i = 0; i < cnt; i++) {
		QTreeWidgetItem *parent = tree->topLevelItem(i);
		QTreeWidgetItem *node = getMenuNode(parent, menu);
		if (node) return node;
	}
	return 0;
}

QTreeWidgetItem *MeUtil::getMenuItem(QTreeWidget *tree, const QString &menu)
{
	return getMenuNode(tree, menu);
}
//==================================================================================

void MeUtil::setItemFlags(QTreeWidgetItem *item, bool editable)
{
	Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
	if (editable) flags |= Qt::ItemIsEditable;
	item->setFlags(flags);
}

void MeUtil::focusItem(QTreeWidget *tree, QTreeWidgetItem *item)
{
	item->setSelected(true);
	tree->setCurrentItem(item);
	tree->scrollToItem(item);
}
//==================================================================================

void MeUtil::takeItem(QTreeWidget *tree, QTreeWidgetItem *item)
{
	int itemPos = getItemPos(tree, item);
	QTreeWidgetItem *parent = item->parent();
	if (parent) {
		parent->takeChild(itemPos);
	} else {
		tree->takeTopLevelItem(itemPos);
	}
}

void MeUtil::insertItem(QTreeWidget *tree, QTreeWidgetItem *item,
						QTreeWidgetItem *parent, int row)
{
	if (parent) {
		if (row < 0) row = parent->childCount();
		parent->insertChild(row, item);
	} else {
		if (row < 0) row = tree->topLevelItemCount();
		tree->insertTopLevelItem(row, item);
	}
}
