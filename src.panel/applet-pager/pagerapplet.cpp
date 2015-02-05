#include <QGraphicsSceneWheelEvent>
#include <QPainter>
#include <QDebug>

#include "pagerapplet.h"
#include "taskapplet.h"
#include "panelwindow.h"
#include "panel.h"
#include "x11support.h"
//==================================================================================

PagerApplet::PagerApplet(PanelWindow *panelWindow)
	: Applet(panelWindow)
	, m_pageCount(4)
	, m_pageIndex(0)
	, m_deskW(10)
	, m_deskH(10)
	, m_cols(1)
	, m_rows(1)
{
	m_instance = this;
	X11Support *xs = X11Support::instance();
	connect(xs, SIGNAL(desktopCountChanged(int)),
			this, SLOT(desktopCountChanged(int)));
	connect(xs, SIGNAL(currentDesktopChanged(int)),
			this, SLOT(currentDesktopChanged(int)));
}

PagerApplet::~PagerApplet()
{
	m_instance = 0;
}
//==================================================================================

bool PagerApplet::start()
{
	m_pageIndex = X11Support::getCurrentDesktopNumber();
	m_pageCount = X11Support::getDesktopCount();
	m_pagerNames = X11Support::getDesktopNames();

	setInteractive(true);

	return true;
}

bool PagerApplet::stop()
{
	return true;
}

QSize PagerApplet::desiredSize()
{
	int margin  = cfg().meterMargin;
	int padding = cfg().meterPadding;
	int spacing = cfg().meterSpacing;
	setPaddings(padding, padding);
	int w, h, sp;

	if (m_panelWindow->orientation() == Panel::Horizontal) {
		setMargins(0, margin);
		m_rows = 2;
		m_cols = m_pageCount / m_rows;
		if (m_pageCount % m_rows) m_cols++;
		if (m_cols < 1) m_cols = 1;
		h = m_panelWindow->height();
		sp = h - (margin << 1) - (padding << 1);
		m_deskH = sp / m_rows;
		sp = (m_rows - 1) * spacing - sp % m_rows;
	//	qDebug("deskH: %d, h: %d, margin: %d, padding: %d, spacing: %d, sp: %d", m_deskH, h, margin, padding, spacing, sp);
		while (sp > 1) { sp -= m_rows; m_deskH--; }
		m_deskW = m_deskH * 16.0 / 10.0;
		w = m_deskW * m_cols + spacing * (m_cols - 1) + xAddons();
	} else {
		setMargins(margin, 0);
		w = m_panelWindow->width();
		h = 20 + yAddons();
	}

	m_outRect = QRect(m_margin.left, m_margin.top,
					  w - m_margin.left - m_margin.right,
					  h - m_margin.top - m_margin.bottom + sp);

	return QSize(w, h);
}

void PagerApplet::desktopCountChanged(int desktopCount)
{
	m_pageCount = desktopCount;
	m_pagerNames = X11Support::getDesktopNames();

	m_panelWindow->updateLayout();
	update();
	showHelp();
}

void PagerApplet::currentDesktopChanged(int currentIndex)
{
	m_pageIndex = currentIndex;

	update();
	showHelp();
}
//==================================================================================

void PagerApplet::paintUi(QPainter *painter, int X, int Y, int W, int H,
						  bool active, bool hasTask)
{
	const QColor &rgb = active ? cfg().pagerActiveColor : cfg().pagerNormalColor;
	painter->setBrush(QBrush(rgb));
	painter->drawRect(X, Y, W, H);

	if (hasTask) return;

	painter->setBrush(QBrush(cfg().meterColorBlack));
	painter->drawRect(X + 1, Y + 1, W - 2, H - 2);
}

void PagerApplet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
						QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	if (!TaskApplet::m_instance) return;

	painter->setPen(Qt::NoPen);
	painter->setBrush(QBrush(cfg().meterColorBlack));
	painter->drawRect(m_outRect);

	int spacing = cfg().meterSpacing;
	int x = m_outRect.left() + m_padding.left;
	int y, i = 0;

	for (int col = 0; col < m_cols; col++) {
		y = m_outRect.top() + m_padding.top;
		for (int row = 0; row < m_rows; row++) {
			bool isCurrent = (i == m_pageIndex);
			bool hasTask = TaskApplet::m_instance->hasClient(i);
			paintUi(painter, x, y, m_deskW, m_deskH, isCurrent, hasTask);
			i++;
			if (i >= m_pageCount) return;
			y += m_deskH + spacing;
		}
		x += m_deskW + spacing;
	}
}
//==================================================================================

void PagerApplet::wheelEvent(QGraphicsSceneWheelEvent *event)
{
	int index = m_pageIndex;
	if (event->delta() < 0) {
		index++;
		if (index >= m_pageCount) index = 0;
	} else {
		index--;
		if (index < 0) index = m_pageCount - 1;
	}
	X11Support::setCurrentDesktopNumber(index);
}

void PagerApplet::showHelp()
{
	if (!m_hovering) return;

	int i = m_pageIndex;
	QString name = (i < m_pagerNames.size()) ? m_pagerNames.at(i) : tr("Unknown");
	QString tips = QString("%1: %2 (%3/%4)")
				   .arg(tr("Current desktop"))
				   .arg(name)
				   .arg(m_pageIndex + 1)
				   .arg(m_pageCount);
	Applet::showHelp(tips);
}
