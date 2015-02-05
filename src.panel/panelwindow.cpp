#include <QtGui/QResizeEvent>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QMenu>
#include <QDebug>

#include "panelwindow.h"
#include "panelwindowbg.h"
#include "panelapplication.h"
#include "x11support.h"
#include "menuapplet.h"
#include "pagerapplet.h"
#include "taskapplet.h"
#include "trayapplet.h"
#include "networkapplet.h"
#include "acpiapplet.h"
#include "volumeapplet.h"
#include "clockapplet.h"
#include "appitem.h"
//==================================================================================

PanelWindow::PanelWindow()
	: QWidget()
	, m_dockMode(false)
	, m_screen(0)
	, m_horizontalAnchor(Panel::Center)
	, m_verticalAnchor(Panel::Min)
	, m_orientation(Panel::Horizontal)
	, m_layoutPolicy(Panel::Normal)
{
//	setStyleSheet("background-color: transparent");
//	setAttribute(Qt::WA_TranslucentBackground);
//	setAttribute(Qt::WA_AlwaysShowToolTips);	// need this for a QWidget window

	m_scene = new QGraphicsScene();
	m_scene->setBackgroundBrush(QBrush(Qt::NoBrush));

	m_baseItem = new PanelWindowBg(this);
	m_scene->addItem(m_baseItem);
//	m_scene->setSceneRect(0, 0, 1024, 26);

	m_view = new QGraphicsView(m_scene, this);
	m_view->setStyleSheet("border-style: none;");
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setRenderHint(QPainter::Antialiasing);
	m_view->move(0, 0);
//	m_view->setAttribute(Qt::WA_AlwaysShowToolTips);
//	m_view->resize(1024, 26);
//	m_view->setSceneRect(0, 0, 1024, 26);

	m_applets.append(new MenuApplet(this));
	m_applets.append(new PagerApplet(this));
	m_applets.append(new TaskApplet(this));
	m_applets.append(new TrayApplet(this));
	m_applets.append(new NetWorkApplet(this));
	m_applets.append(new VolumeApplet(this));
	m_applets.append(new AcpiApplet(this));
	m_applets.append(new ClockApplet(this));

	resize(defaultWidth, defaultHeight);
}

PanelWindow::~PanelWindow()
{
	while (!m_applets.isEmpty())
	{
		delete m_applets[m_applets.size() - 1];
		m_applets.resize(m_applets.size() - 1);
	}
	delete m_view;
	delete m_baseItem;
	delete m_scene;
}

bool PanelWindow::start()
{
	for (int i = 0; i < m_applets.size();) {
		if (!m_applets[i]->start()) {
			m_applets.remove(i);
			qDebug() << "PanelWindow::start(): err, i=" << i;
		} else {
		//	qDebug() << "PanelWindow::start(): ok, i=" << i;
			i++;
		}
	}
	return true;
}

bool PanelWindow::stop()
{
	for (int i = 0; i < m_applets.size(); i++) {
		if (!m_applets[i]->stop()) return false;
	}
	return true;
}

void PanelWindow::setDockMode(bool dockMode)
{
	m_dockMode = dockMode;

	setAttribute(Qt::WA_X11NetWmWindowTypeDock, m_dockMode);

	if (!m_dockMode)
	{
		// No need to reserve space anymore.
		X11Support::removeWindowProperty(winId(), "_NET_WM_STRUT");
		X11Support::removeWindowProperty(winId(), "_NET_WM_STRUT_PARTIAL");
	}

	// When in dock mode, panel should appear on all desktops.
	unsigned long desktop = m_dockMode ? 0xFFFFFFFF : 0;
	X11Support::setWindowPropertyCardinal(winId(), "_NET_WM_DESKTOP", desktop);

	updateLayout();
	updatePosition();
}

void PanelWindow::setScreen(int screen)
{
	m_screen = screen;
	updateLayout();
	updatePosition();
}

void PanelWindow::setHorizontalAnchor(Panel::Anchor horizontalAnchor)
{
	m_horizontalAnchor = horizontalAnchor;
	updatePosition();
}

void PanelWindow::setVerticalAnchor(Panel::Anchor verticalAnchor)
{
	m_verticalAnchor = verticalAnchor;
	updatePosition();
}

void PanelWindow::setOrientation(Panel::Orientation orientation)
{
	m_orientation = orientation;
}

void PanelWindow::setLayoutPolicy(Panel::LayoutPolicy layoutPolicy)
{
	m_layoutPolicy = layoutPolicy;
	updateLayout();
}

void PanelWindow::updatePosition()
{
	if (!m_dockMode)
		return;

	QRect screenGeometry = QApplication::desktop()->screenGeometry(m_screen);

	int x;

	switch(m_horizontalAnchor)
	{
		case Panel::Min:
			x = screenGeometry.left();
			break;
		case Panel::Center:
			x = (screenGeometry.left() + screenGeometry.right() + 1 - width())/2;
			break;
		case Panel::Max:
			x = screenGeometry.right() - width() + 1;
			break;
		default:
			Q_ASSERT(false);
			break;
	}

	int y;

	switch(m_verticalAnchor)
	{
		case Panel::Min:
			y = screenGeometry.top();
			break;
		case Panel::Center:
			y = (screenGeometry.top() + screenGeometry.bottom() + 1 - height())/2;
			break;
		case Panel::Max:
			y = screenGeometry.bottom() - height() + 1;
			break;
		default:
			Q_ASSERT(false);
			break;
	}

	move(x, y);

	// Update reserved space.
	if (m_dockMode)
	{
		QVector<unsigned long> values; // Values for setting _NET_WM_STRUT_PARTIAL property.
		values.fill(0, 12);
		switch(m_horizontalAnchor)
		{
			case Panel::Min:
				values[0] = x + width();
				values[4] = y;
				values[5] = y + height();
				break;
			case Panel::Max:
				values[1] = QApplication::desktop()->width() - x;
				values[6] = y;
				values[7] = y + height();
				break;
			default:
				break;
		}

		switch(m_verticalAnchor)
		{
			case Panel::Min:
				values[2] = y + height();
				values[8] = x;
				values[9] = x + width();
				break;
			case Panel::Max:
				values[3] = QApplication::desktop()->height() - y;
				values[10] = x;
				values[11] = x + width();
				break;
			default:
				break;
		}

		X11Support::setWindowPropertyCardinalArray(winId(), "_NET_WM_STRUT_PARTIAL", values);
		values.resize(4);
		X11Support::setWindowPropertyCardinalArray(winId(), "_NET_WM_STRUT", values);
	}

	// Update "blur behind" hint.
	QVector<unsigned long> values;
	values.resize(4);
	values[0] = 0;
	values[1] = 0;
	values[2] = width();
	values[3] = height();
	X11Support::setWindowPropertyCardinalArray(winId(), "_KDE_NET_WM_BLUR_BEHIND_REGION", values);
}

const Panel &PanelWindow::cfg()
{
	return PanelApplication::instance()->panel();
}

const QFont &PanelWindow::font()
{
	return PanelApplication::instance()->panelFont();
}

int PanelWindow::textBaseLine() const
{
	QFontMetrics metrics(font());
	return (height() - metrics.height())/2 + metrics.ascent();
}

void PanelWindow::resizeEvent(QResizeEvent *event)
{
	QSize sz = event->size();
	m_view->resize(sz);
	m_view->setSceneRect(0, 0, sz.width(), sz.height());
	updateLayout();
	updatePosition();
}

void PanelWindow::updateLayout()
{
	m_baseItem->update();

	int spacing = cfg().appletSpacing;
	if (spacing < 4) spacing = 4;	// when < 4: cause next applet repaint

	bool isHorz = (m_orientation == Panel::Horizontal);
	int autoSizeCnt = 0;
	int appletTotal = 0;
	int appletCount = m_applets.size();
	QVector<int> lens(appletCount);
	//------------------------------------------------------------------------------

	for (int i = 0; i < appletCount; i++) {
		lens[i] = isHorz ? m_applets[i]->desiredSize().width()
						 : m_applets[i]->desiredSize().height();
		if (lens[i] > 0) appletTotal += lens[i];
		else if (lens[i] < 0) autoSizeCnt++;
	}
	//------------------------------------------------------------------------------

	if (m_layoutPolicy != Panel::Normal && !m_dockMode)
	{
		int desiredSize = 0;
		if (m_layoutPolicy == Panel::AutoSize)
		{
			for (int i = 0; i < appletCount; i++)
			{
				if (m_applets[i]->desiredSize().width() >= 0)
					desiredSize += m_applets[i]->desiredSize().width();
				else
					desiredSize += 64; // Spacer applets don't really make sense on auto-size panel.
			}
			desiredSize += spacing*(appletCount - 1);
			if (desiredSize < 0)
				desiredSize = 0;
		}
		if (m_layoutPolicy == Panel::FillSpace)
		{
			QRect screenGeometry = QApplication::desktop()->screenGeometry(m_screen);
			desiredSize = screenGeometry.width();
		}

		if (desiredSize != width())
			resize(desiredSize, height());
	}
	//------------------------------------------------------------------------------

	// Calculate size for auto sizing applet (width = -1)
	if (autoSizeCnt > 0) {
		int length = isHorz ? width() : height();
		int spacings = spacing * (appletCount - 1);
		int freeSpace = length - spacings - appletTotal;
		int len = freeSpace / autoSizeCnt;
		for (int i = 0; i < appletCount; i++) {
			if (lens[i] < 0) lens[i] = len;
		}
	}

	// Calculate rectangles for each applet.
	if (isHorz) {
		int x = 0, y = 0, h = height();
		for (int i = 0; i < appletCount; i++) {
			m_applets[i]->setBound(x, y, lens[i], h);
			x += lens[i] + spacing;
		}
	} else {
		int x = 0, y = 0, w = width();
		for (int i = 0; i < appletCount; i++) {
			m_applets[i]->setBound(x, y, w, lens[i]);
			y += lens[i] + spacing;
		}
	}

//	qDebug() << this->geometry();
}

void PanelWindow::showPanelContextMenu(const QPoint &point)
{
	QMenu menu;
	PanelApplication *app = PanelApplication::instance();

	menu.addAction(QIcon::fromTheme("preferences-desktop"),
				   tr("Configure panel..."),
				   app, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-desktop"),
				   tr("Edit custom menu..."),
				   app, SLOT(showMenuEditor()));
	menu.addAction(QIcon::fromTheme("application-exit"),
				   tr("Quit panel"),
				   QApplication::instance(), SLOT(quit()));

	menu.exec(pos() + point);
}
