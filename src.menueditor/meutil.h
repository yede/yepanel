#ifndef MEUTIL_H
#define MEUTIL_H

#include <QTreeWidget>
//==================================================================================

class MeUtil
{
public:
	MeUtil();

public:
	static void    setItemText(QTreeWidgetItem *item, const QString &text);
	static QString getItemText(QTreeWidgetItem *item);
	static void    setItemType(QTreeWidgetItem *item, int type);
	static int     getItemType(QTreeWidgetItem *item);
	static void    setItemMenu(QTreeWidgetItem *item, const QString &menu);
	static QString getItemMenu(QTreeWidgetItem *item);
	static void    setItemExec(QTreeWidgetItem *item, const QString &exec);
	static QString getItemExec(QTreeWidgetItem *item);
	static void    setItemIcon(QTreeWidgetItem *item, const QString &icon);
	static QString getItemIcon(QTreeWidgetItem *item);

	static QByteArray getItemTextByteArray(QTreeWidgetItem *item);
	static QByteArray getItemIconByteArray(QTreeWidgetItem *item);
	static QByteArray getItemExecByteArray(QTreeWidgetItem *item);

	static int getItemPos(QTreeWidget *tree, QTreeWidgetItem *item);
	static QTreeWidgetItem *getInsertPos(QTreeWidget *tree, int &pos);
	static QTreeWidgetItem *getMenuItem(QTreeWidget *tree, const QString &menu);
	static void setItemFlags(QTreeWidgetItem *item, bool editable = false);
	static void focusItem(QTreeWidget *tree, QTreeWidgetItem *item);
	static void takeItem(QTreeWidget *tree, QTreeWidgetItem *item);
	static void insertItem(QTreeWidget *tree, QTreeWidgetItem *item,
						   QTreeWidgetItem *parent, int row);
};

#endif // MEUTIL_H
