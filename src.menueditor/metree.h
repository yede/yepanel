#ifndef METREE_H
#define METREE_H

#include <QTreeWidget>
#include "jsonfile.h"
//==================================================================================

class MeWindow;

class MeTree : public QTreeWidget
{
	Q_OBJECT
public:
	explicit MeTree(QWidget *parent = 0);

	void loadFile();
	void saveFile();

	QTreeWidgetItem *newItem(const QString &title, int type,
							 const QString &icon, const QString &exec,
							 QTreeWidgetItem *parent, int row = -1,
							 bool focus = false);

	void setWindow(MeWindow *window) { m_window = window; }

protected:
	void mousePressEvent(QMouseEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);

	QMimeData *mimeData(const QList<QTreeWidgetItem *> items) const;
	QStringList mimeTypes() const;
	bool dropMimeData(QTreeWidgetItem *parent, int index,
					  const QMimeData *data, Qt::DropAction action);

signals:
	
public slots:
	void onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
	static void  addSep(void *parent, void *userData);
	static void  addItem(void *parent, const QString &title, const QString &icon, const QString &exec, void *userData);
	static void *addMenu(void *parent, const QString &title, const QString &icon, void *userData);

	void saveNode(cJSON *node, QTreeWidgetItem *item);
	static void saveAll(cJSON *root, void *userData);

private:
	friend class MeDelegate;

	MeWindow *m_window;
	QTreeWidgetItem *m_dropItem;
	int m_dropArea;
	bool m_hovering;
	QModelIndex m_hoverIndex;
	QStringList m_mimeTypes;
	JsonFile m_json;
	QIcon m_iconFolder;
	QIcon m_iconGear;
};

#endif // METREE_H
