#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QDebug>

#include "trayapplet.h"
#include "trayitem.h"
#include "panelapplication.h"
#include "panelwindow.h"
#include "x11support.h"
//==================================================================================

#define ITEM_SIZE  24
#define SPACING    2
//==================================================================================

TrayApplet::TrayApplet(PanelWindow *panelWindow)
	: Applet(panelWindow)
	, m_initialized(false)
{
}

TrayApplet::~TrayApplet()
{
	if (m_initialized)
		X11Support::freeSystemTray();

	while (!m_trayItems.isEmpty())
	{
		delete m_trayItems[m_trayItems.size() - 1];
	}
}

bool TrayApplet::start()
{
	m_initialized = X11Support::makeSystemTray(m_panelWindow->winId());

	if (!m_initialized)
	{
		// Another tray is active.
		return false;
	}

	connect(X11Support::instance(), SIGNAL(windowClosed(ulong)), this, SLOT(windowClosed(ulong)));
	connect(X11Support::instance(), SIGNAL(windowReconfigured(ulong,int,int,int,int)), this, SLOT(windowReconfigured(ulong,int,int,int,int)));
	connect(X11Support::instance(), SIGNAL(windowDamaged(ulong)), this, SLOT(windowDamaged(ulong)));
	connect(X11Support::instance(), SIGNAL(clientMessageReceived(ulong,ulong,void*)), this, SLOT(clientMessageReceived(ulong,ulong,void*)));

	return true;
}

bool TrayApplet::stop()
{
	return true;
}

QSize TrayApplet::desiredSize()
{
	int w = (ITEM_SIZE + SPACING) * m_trayItems.size() - SPACING;
	if (w < 0) w = 0;
	return QSize(w, m_panelWindow->height());
}

void TrayApplet::registerTrayItem(TrayItem *trayItem)
{
	m_trayItems.append(trayItem);
	m_panelWindow->updateLayout();
}

void TrayApplet::unregisterTrayItem(TrayItem *trayItem)
{
	m_trayItems.remove(m_trayItems.indexOf(trayItem));
	m_panelWindow->updateLayout();
}

void TrayApplet::layoutChanged()
{
	updateLayout();
}

void TrayApplet::clientMessageReceived(unsigned long window,
									   unsigned long atom, void *data)
{
	Q_UNUSED(window);

	if (atom == X11Support::atom("_NET_SYSTEM_TRAY_OPCODE"))
	{
		unsigned long *l = reinterpret_cast<unsigned long*>(data);
		if (l[1] == 0) // TRAY_REQUEST_DOCK
		{
			for (int i = 0; i < m_trayItems.size(); i++)
			{
				if (m_trayItems[i]->window() == l[2])
					return; // Already added.
			}
			new TrayItem(this, l[2]);
		}
	}
}

void TrayApplet::windowClosed(unsigned long window)
{
	for (int i = 0; i < m_trayItems.size(); i++)
	{
		if (m_trayItems[i]->window() == window)
		{
			delete m_trayItems[i];
			break;
		}
	}
}

void TrayApplet::windowReconfigured(unsigned long window, int x, int y,
									int width, int height)
{
	Q_UNUSED(x);
	Q_UNUSED(y);
	Q_UNUSED(width);
	Q_UNUSED(height);

	for (int i = 0; i < m_trayItems.size(); i++)
	{
		if (m_trayItems[i]->window() == window)
		{
			X11Support::resizeWindow(window, ITEM_SIZE, ITEM_SIZE);
			break;
		}
	}
}

void TrayApplet::windowDamaged(unsigned long window)
{
	for (int i = 0; i < m_trayItems.size(); i++)
	{
		if (m_trayItems[i]->window() == window)
		{
			m_trayItems[i]->setPixmapDamaged();		// this prevent m_trayItems to read x11-pixmap during closing a window
			m_trayItems[i]->update();				// (some app cause panel crashed when it's closed)
			break;
		}
	}
}

void TrayApplet::updateLayout()
{
	int x = 0;
	for (int i = 0; i < m_trayItems.size(); i++)
	{
		m_trayItems[i]->setSize(QSize(ITEM_SIZE, m_size.height()));
		m_trayItems[i]->setPosition(QPoint(x, 0));
		x += ITEM_SIZE + SPACING;
	}
}
