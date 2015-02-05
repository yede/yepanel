#ifndef MEDELEGATE_H
#define MEDELEGATE_H

#include <QItemDelegate>
//==================================================================================

namespace DropArea {
	enum { Unknown = 0, NoItem, OnItem, AboveItem, BelowItem };
}
namespace NodeProp {
	enum { Type = 32, Menu, Exec, Icon };
}
//==================================================================================

class MeTree;

class MeDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	explicit MeDelegate(MeTree *parent);
	
	QSize sizeHint(const QStyleOptionViewItem &option,
				   const QModelIndex &index ) const;

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
			   const QModelIndex &index) const;

signals:
	
public slots:
	
private:
	MeTree      *m_tree;
	int         &m_dropArea;
	bool        &m_hovering;
	QModelIndex &m_hoverIndex;
	int          m_rowHeight;
};

#endif // MEDELEGATE_H
