#include <QApplication>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDebug>

#include "mewindow.h"
#include "metree.h"
#include "medelegate.h"
#include "meutil.h"
#include "global.h"
//==================================================================================

MeTree::MeTree(QWidget *parent)
	: QTreeWidget(parent)
	, m_window(0)
	, m_dropItem(0)
	, m_hovering(false)
	, m_json(Global::menuFile(), this)	// "/home/nat/ydev/c/wm/centask/bin64.d/menu.json"
	, m_iconFolder(":/16/folder")
	, m_iconGear(":/16/gear")
{
	m_mimeTypes = QStringList(QString("MeTreeItemNode"));

	setHeaderHidden(true);
	setColumnCount(1);
	setDragDropMode(QAbstractItemView::DragDrop);
	setItemDelegate(new MeDelegate(this));

	connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
			this, SLOT(onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}
//==================================================================================

void MeTree::mousePressEvent(QMouseEvent *event)
{
	QTreeWidgetItem *item = itemAt(event->pos());
	setCurrentItem(item);
	QTreeWidget::mousePressEvent(event);
}

void MeTree::dragEnterEvent(QDragEnterEvent *event)
{
	m_dropItem = 0;
	m_dropArea = DropArea::Unknown;
	m_hovering = false;
//	event->acceptProposedAction();
	event->accept();
	QTreeView::dragEnterEvent(event);
}

void MeTree::dragLeaveEvent(QDragLeaveEvent *event)
{
	QTreeView::dragLeaveEvent(event);
	event->accept();

	m_hovering = false;
	if (m_hoverIndex.isValid()) update(m_hoverIndex);
}

void MeTree::dropEvent(QDropEvent *event)
{
	QTreeView::dropEvent(event);
	event->accept();

	m_hovering = false;
	if (m_hoverIndex.isValid()) update(m_hoverIndex);
}

void MeTree::dragMoveEvent(QDragMoveEvent *event)
{
	QTreeView::dragMoveEvent(event);
	m_dropItem = itemAt(event->pos());
	if (!m_dropItem) return;

	m_hoverIndex = indexAt(event->pos());
	m_hovering = true;
	QRect r = visualItemRect(m_dropItem);

	int y = event->pos().y();
	int y1 = r.y();
	int y2 = y1 + 2;
	int y4 = r.y() + r.height();
	int y3 = y4 - 2;

	if      (y < y1) m_dropArea = DropArea::Unknown;
	else if (y < y2) m_dropArea = DropArea::AboveItem;
	else if (y < y3) m_dropArea = DropArea::OnItem;
	else if (y < y4) m_dropArea = DropArea::BelowItem;
	else             m_dropArea = DropArea::Unknown;

	Qt::DropAction action = Qt::MoveAction;
	event->setDropAction(action);
	event->acceptProposedAction();
}
//==================================================================================

QMimeData *MeTree::mimeData(const QList<QTreeWidgetItem *> items) const
{
	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;
	QDataStream stream(&encodedData, QIODevice::WriteOnly);
	quint64 pointer;
	foreach (QTreeWidgetItem *item, items) {
		pointer = (quint64) item;
		stream << pointer;
		break;
	}
	mimeData->setData(m_mimeTypes.at(0), encodedData);
	return mimeData;
}

QStringList MeTree::mimeTypes() const
{
	return m_mimeTypes;
}

bool MeTree::dropMimeData(QTreeWidgetItem *parent, int index,
						  const QMimeData *data, Qt::DropAction action)
{
	Q_UNUSED(index);
	Q_UNUSED(action);
	if (!m_dropItem) return false;
	Q_ASSERT(m_dropItem->parent() == parent);

	QByteArray encodedData = data->data(m_mimeTypes.at(0));
	QDataStream stream(&encodedData, QIODevice::ReadOnly);
	QTreeWidgetItem *item;
	quint64 pointer;
	while (!stream.atEnd()) {
		stream >> pointer;
		item = (QTreeWidgetItem *) pointer;
		break;
	}

	MeUtil::takeItem(this, item);
	int pos = MeUtil::getItemPos(this, m_dropItem);	// after takeItem (maybe same parent)

	switch (m_dropArea) {
		case DropArea::AboveItem: break;
		case DropArea::BelowItem: pos++; break;
		default:
			if (MeUtil::getItemType(m_dropItem) == NodeType::AppMenu) {
				pos = -1;
				parent = m_dropItem;
			}
	}
	MeUtil::insertItem(this, item, parent, pos);

	return true;
}
//==================================================================================

void MeTree::onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
//	if (previous) m_window->readEditor(previous);
	m_window->showItem(current);
}

QTreeWidgetItem *MeTree::newItem(const QString &title, int type, const QString &icon, const QString &exec,
								 QTreeWidgetItem *parent, int row, bool focus)
{
	QStringList strs(title);
	QTreeWidgetItem *item = new QTreeWidgetItem(strs);
	switch (type) {
		case NodeType::AppMenu: item->setIcon(0, m_iconFolder); break;
		case NodeType::AppItem: item->setIcon(0, m_iconGear);   break;
		case NodeType::SepItem: break;
	}

	QFontMetrics fm(QApplication::font());
	QSize defSize;
	defSize.setHeight(fm.height() + 4);
	item->setData(0, Qt::SizeHintRole, defSize);
	item->setToolTip(0, title);

	MeUtil::insertItem(this, item, parent, row);
	if (parent)
		MeUtil::setItemMenu(item, MeUtil::getItemText(parent));
	MeUtil::setItemFlags(item);
	MeUtil::setItemType(item, type);
	MeUtil::setItemExec(item, exec);
	MeUtil::setItemIcon(item, icon);

	if (focus)
		MeUtil::focusItem(this, item);

	return item;
}

//==================================================================================
// load
//==================================================================================

void MeTree::addSep(void *parent, void *userData)
{
	MeTree *p = (MeTree *) userData;
	p->newItem(QString(), NodeType::SepItem, QString(), QString(), (QTreeWidgetItem *) parent);
}

void MeTree::addItem(void *parent, const QString &title, const QString &icon, const QString &exec, void *userData)
{
	MeTree *p = (MeTree *) userData;
	p->newItem(title, NodeType::AppItem, icon, exec, (QTreeWidgetItem *) parent);
}

void *MeTree::addMenu(void *parent, const QString &title, const QString &icon, void *userData)
{
	MeTree *p = (MeTree *) userData;
	return p->newItem(title, NodeType::AppMenu, icon, QString(), (QTreeWidgetItem *) parent);
}

void MeTree::loadFile()
{
	m_json.loadFile(addMenu, addItem, addSep);
}

//==================================================================================
// save
//==================================================================================

void MeTree::saveNode(cJSON *node, QTreeWidgetItem *item)
{
	int type = MeUtil::getItemType(item);

	if (type == NodeType::SepItem) {
		m_json.saveSep(node);
		return;
	}

	QByteArray baTitle = MeUtil::getItemText(item).toUtf8();
	QByteArray baIcon  = MeUtil::getItemIcon(item).toUtf8();
	QByteArray baExec  = MeUtil::getItemExec(item).toUtf8();

	const char *title = baTitle.constData();
	const char *icon  = baIcon.constData();
	const char *exec  = baExec.constData();

//	qDebug("saveNode: title=%s, icon=%s, exec=%s", title, icon, exec);

	if (type == NodeType::AppItem) {
		m_json.saveItem(node, title, icon, exec);
		return;
	}

	if (type == NodeType::AppMenu) {
		cJSON *subNode = m_json.saveMenu(node, title, icon);
		int cnt = item->childCount();
		for (int i = 0; i < cnt; i++) {
			QTreeWidgetItem *child = item->child(i);
			saveNode(subNode, child);
		}
	}
}

void MeTree::saveAll(cJSON *root, void *userData)
{
	MeTree *tree = (MeTree *) userData;
	int cnt = tree->topLevelItemCount();

	for (int i = 0; i < cnt; i++) {
		QTreeWidgetItem *item = tree->topLevelItem(i);
		tree->saveNode(root, item);
	}
}

void MeTree::saveFile()
{
	m_json.saveFile(saveAll);
}
//==================================================================================
