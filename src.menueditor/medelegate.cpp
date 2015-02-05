#include <QPainter>
#include "medelegate.h"
#include "metree.h"
//==================================================================================

MeDelegate::MeDelegate(MeTree *parent)
	: QItemDelegate(parent)
	, m_tree(parent)
	, m_dropArea(parent->m_dropArea)
	, m_hovering(parent->m_hovering)
	, m_hoverIndex(parent->m_hoverIndex)
	, m_rowHeight(17)
{
}

QSize MeDelegate::sizeHint(const QStyleOptionViewItem &option,
						   const QModelIndex &index ) const
{
	QSize size = QItemDelegate::sizeHint(option, index);
	return QSize(size.width(), m_rowHeight);
}

void MeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
					   const QModelIndex &index) const
{
	QItemDelegate::paint(painter, option, index);

	if (!m_hovering || m_dropArea == DropArea::Unknown || index != m_hoverIndex)
		return;

	QRect r = option.rect;

	if (m_dropArea == DropArea::AboveItem) {
		r.setRect(r.x(), r.y(), r.width(), 1);
	}
	else if (m_dropArea == DropArea::BelowItem) {
		r.setRect(r.x(), r.bottom(), r.width(), 1);
	}
	else {	// DropArea::OnItem
		//
	}

	painter->setPen(QColor(0, 0, 255));
	painter->drawRect(r);

}
//==================================================================================
