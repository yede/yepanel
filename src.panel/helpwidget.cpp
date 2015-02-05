#include <QApplication>
#include <QDesktopWidget>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QTextOption>
#include <QHBoxLayout>
#include <QDebug>

#include "helpwidget.h"
#include "panelwindow.h"
#include "applet.h"
#include "panel.h"
//==================================================================================

HelpString::HelpString(int x, int y, int width, Qt::Alignment align, const QString &text)
	: x(x), y(y), width(width), alignment(align), text(text), tag(0)
{
}

HelpString::HelpString(int x, int y, const QString &text)
	: x(x), y(y), width(-1), alignment(Qt::AlignLeft), text(text), tag(0)
{
}

HelpString::HelpString()
	: x(0), y(0), width(-1), alignment(Qt::AlignLeft), tag(0)
{
}

void HelpString::setString(int x, int y, int width, Qt::Alignment align, const QString &text)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->alignment = align;
	this->text = text;
}

void HelpString::setString(int x, int y, const QString &text)
{
	this->x = x;
	this->y = y;
	this->width = -1;
	this->alignment = Qt::AlignLeft;
	this->text = text;
}

void HelpString::setString(const QString &text)
{
	this->text = text;
}

//==================================================================================
// HelpWidget
//==================================================================================

HelpInfo::HelpInfo(int width, int height)
	: width(width), height(height)
{}

HelpInfo::HelpInfo(int width)
	: width(width), height(-1)
{}

HelpInfo::HelpInfo()
	: width(-1), height(-1)
{}

void HelpInfo::addString(const HelpString *hs)
{
	strings.append(hs);
}

void HelpInfo::clear()
{
	width = -1;
	height = -1;
	strings.clear();
}

//==================================================================================
// HelpWidget
//==================================================================================

namespace HelpType {
	enum { String, StringList, HelpInfo };
}

HelpWidget::HelpWidget()
	: QWidget(0, Qt::X11BypassWindowManagerHint)
	, m_size(90, 20)
	, m_helpType(HelpType::String)
	, m_padX(6)
	, m_padY(4)
	, m_lineHeight(17)
{
	QPalette pa(PanelWindow::cfg().tooltipColor);
	setPalette(pa);
	setFont(PanelWindow::cfg().fontNormal);
/*
	QSpacerItem *spacer = new QSpacerItem(20, 1, QSizePolicy::Preferred,
										  QSizePolicy::Fixed);
	m_label = new QLabel(this);
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(8, 4, 0, 4);
	layout->setSpacing(0);
	layout->addWidget(m_label);
	layout->addSpacerItem(spacer);
*/
	m_desktopWidth = QApplication::desktop()->width();
	m_desktopHeight = QApplication::desktop()->height();
}

HelpWidget::~HelpWidget()
{
}

void HelpWidget::createInstance()
{
	if (!m_instance) m_instance = new HelpWidget();
}

void HelpWidget::destroyInstance()
{
	if (m_instance) {
		delete m_instance;
		m_instance = 0;
	}
}
//==================================================================================

void HelpWidget::closeHelp()
{
	if (m_instance) m_instance->hide();
}

void HelpWidget::showHelp(const QString &text)
{
	if (m_instance) m_instance->showHelp(text, m_position);
}

void HelpWidget::showHelp(const QStringList &strs)
{
	if (m_instance) m_instance->showHelp(strs, m_position);
}

void HelpWidget::showHelp(const HelpInfo &info)
{
	if (m_instance) m_instance->showHelp(info, m_position);
}
//==================================================================================

void HelpWidget::showHelp(const HelpInfo &info, const HelpPosition &pos)
{
	m_helpType = HelpType::HelpInfo;
	m_info = info;
	calcSize(info);
	showText(pos);
}

void HelpWidget::showHelp(const QStringList &strs, const HelpPosition &pos)
{
	m_helpType = HelpType::StringList;
	m_strs = strs;
	calcSize(strs);
	showText(pos);
}

void HelpWidget::showHelp(const QString &text, const HelpPosition &pos)
{
	m_helpType = HelpType::String;
	m_text = text;
	calcSize(text);
	showText(pos);
}

void HelpWidget::calcSize(const HelpInfo &info)
{
	QFontMetrics fm(PanelWindow::cfg().fontNormal);
	int mainW = 0, mainH = 0;

	foreach (const HelpString *s, info.strings) {
		int w = s->width;
		if (w < 0) w = fm.width(s->text);
		int v = s->x + w;
		if (v > mainW) mainW = v;
		v = s->y + m_lineHeight;
		if (v > mainH) mainH = v;
	}
	m_size = QSize(mainW + (m_padX << 1), mainH + (m_padY << 1));
}

void HelpWidget::calcSize(const QStringList &strs)
{
	QFontMetrics fm(PanelWindow::cfg().fontNormal);
	int mainW = 0, mainH = 0;

	foreach (const QString &s, strs) {
		int w = fm.width(s);
		if (w > mainW) mainW = w;
		mainH += m_lineHeight;
	}
	m_size = QSize(mainW + (m_padX << 1), mainH + (m_padY << 1));
}

void HelpWidget::calcSize(const QString &text)
{
	QFontMetrics fm(PanelWindow::cfg().fontNormal);
	int w = fm.width(text) + 5;
	w += w / 50;
	m_size = QSize(w + (m_padX << 1), m_lineHeight + (m_padY << 1));
}

void HelpWidget::showText(const HelpPosition &pos)
{
	int x = pos.x, y = pos.y;

	if (m_position.orientation == Panel::Horizontal) {
		if (pos.limitTop > 0)
			y = pos.limitTop;
		else if (pos.limitBottom > 0)
			y = pos.limitBottom - m_size.height();
		int ax = m_size.width() + x + 4;
		if (ax > m_desktopWidth) x = m_desktopWidth - m_size.width() - 4;
	} else {
		if (pos.limitLeft > 0)
			x = pos.limitLeft;
		else if (pos.limitRight > 0)
			x = pos.limitRight - m_size.width();
		int ay = m_size.height() + y;
		if (ay > m_desktopHeight) y = m_desktopHeight - m_size.height();
	}

	if (this->y() != y || this->x() != x)
		move(x, y);

	if (!isVisible())
		show();
	else
		update();
}

void HelpWidget::calcPosition(const QPointF &pos, PanelWindow *panelWindow)
{
	m_position.orientation = panelWindow->orientation();
	int x = 0, y = 0, left = 0, right = 0, top = 0, bottom = 0;

	if (m_position.orientation == Panel::Horizontal) {
		x = pos.x() + panelWindow->x();
		y = panelWindow->y();
		if (panelWindow->verticalAnchor() == Panel::Min) {	// top
			top = y + panelWindow->height() + 4;
		} else {													// bottom
			bottom = y - 4;
		}
	}

	m_position.x = x;
	m_position.y = y;
	m_position.limitLeft = left;
	m_position.limitRight = right;
	m_position.limitTop = top;
	m_position.limitBottom = bottom;
}
//==================================================================================

void HelpWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	QTextOption o;
	QRectF r(0, 0, m_size.width() - 1, m_size.height() - 1);
	painter.drawRect(r);

	if (m_helpType == HelpType::String)
	{
		int w = m_size.width() - (m_padX << 1);
		int h = m_size.height() - (m_padY << 1);
		r.setRect(m_padX, m_padY, w, h);
		o.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		painter.drawText(r, m_text, o);
	}
	else if (m_helpType == HelpType::StringList)
	{
		int w = m_size.width() - (m_padX << 1);
		int y = 0;
		foreach (const QString &s, m_strs) {
			r.setRect(m_padX, m_padY + y, w, m_lineHeight);
			painter.drawText(r, s);
			y += m_lineHeight;
		}
	}
	else {
		foreach (const HelpString *s, m_info.strings) {
			r.setRect(s->x + m_padX, s->y + m_padY, s->width, m_lineHeight);
			o.setAlignment(s->alignment);
			painter.drawText(r, s->text, o);
		}
	}
}
