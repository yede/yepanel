#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsRectItem>

#include "demoapplet.h"
#include "panelwindow.h"
//==================================================================================

DemoApplet::DemoApplet(PanelWindow *panelWindow)
	: Applet(panelWindow)
{
	m_rectItem = new QGraphicsRectItem(this);
	m_rectItem->setPen(QPen(Qt::NoPen));
	m_rectItem->setBrush(QBrush(Qt::blue));
}

DemoApplet::~DemoApplet()
{
	delete m_rectItem;
}

bool DemoApplet::start()
{
	return true;
}

bool DemoApplet::stop()
{
	return true;
}

void DemoApplet::layoutChanged()
{
	static const int delta = 8;
	m_rectItem->setRect(delta, delta, m_size.width() - 2*delta, m_size.height() - 2*delta);
}

QSize DemoApplet::desiredSize()
{
	return QSize(64, 64);
}
