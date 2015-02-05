#include <QtGui/QWidget>
#include <QTimer>
#include <QDebug>

#include "taskclient.h"
#include "taskitem.h"
#include "taskapplet.h"
#include "pagerapplet.h"
#include "x11support.h"
#include "panelwindow.h"
#include "panel.h"
//==================================================================================

TaskApplet::TaskApplet(PanelWindow *panelWindow)
	: Applet(panelWindow)
	, m_activeWindow(0)
	, m_dragging(false)
	, m_pagerCount(4)
	, m_pagerIndex(0)
{
	m_instance = this;

	X11Support *xs = X11Support::instance();
	connect(xs, SIGNAL(desktopCountChanged(int)),
			this, SLOT(desktopCountChanged(int)));
	connect(xs, SIGNAL(currentDesktopChanged(int)),
			this, SLOT(currentDesktopChanged(int)));
	connect(xs, SIGNAL(clientListChanged()),
			this, SLOT(clientListChanged()));
	connect(xs, SIGNAL(activeWindowChanged()),
			this, SLOT(activeWindowChanged()));
	connect(xs, SIGNAL(windowPropertyChanged(ulong,ulong)),
			this, SLOT(windowPropertyChanged(ulong,ulong)));
}

TaskApplet::~TaskApplet()
{
	clearClients();
	clearButtons();
	m_instance = NULL;
}
//==================================================================================

void TaskApplet::clearButtons()
{
	int i = m_workButtons.size();
	while (i > 0) { i--; delete m_workButtons[i]; }

	i = m_idleButtons.size();
	while (i > 0) { i--; delete m_idleButtons[i]; }

	m_workButtons.clear();
	m_idleButtons.clear();
}

void TaskApplet::clearClients()
{
	foreach (TaskClient *client, m_clients) deleteClient(client);
	m_clients.clear();
}

void TaskApplet::deleteClient(TaskClient *client)
{
	client->removeFromButton();
	delete client;
}
//==================================================================================

bool TaskApplet::start()
{
	m_pagerIndex = X11Support::getCurrentDesktopNumber();
	m_pagerCount = X11Support::getDesktopCount();
//	desktopCountChanged(m_pagerCount);
//	currentDesktopChanged(m_pagerIndex);
	clientListChanged();
	activeWindowChanged();

	for (int i = 0; i < m_workButtons.size(); i++)
		if (m_workButtons[i]->pagerIndex() == m_pagerIndex)
			m_workButtons[i]->moveInstantly();

	return true;
}

bool TaskApplet::stop()
{
	return true;
}
//==================================================================================

void TaskApplet::updateLayout()
{
	// TODO: Vertical orientation support.

	int count = 0;		// item in current workspace (page)
	foreach (TaskItem *item, m_workButtons) {
		if (item->pagerIndex() == m_pagerIndex) count++;
	}

	if (count > 0) {
		int pos = 0;
		int spacing = 4;
		int spaceEach = m_size.width() / count;
		if (spaceEach > 200) spaceEach = 200; else if (spaceEach < 40) spaceEach = 40;

		for (int i = 0; i < m_workButtons.size(); i++) {
			TaskItem *item = m_workButtons[i];
			if (item->pagerIndex() == m_pagerIndex) {
				item->setGeometry(QPoint(pos, 0), QSize(spaceEach - spacing, m_size.height()), true);
			//	item->startAnimation();
				pos += spaceEach;
			}
		}
	}

	update();
}

void TaskApplet::draggingStarted()
{
	m_dragging = true;
}

void TaskApplet::draggingStopped()
{
	m_dragging = false;
	// Since we don't update it when dragging, we should do it now.
	clientListChanged();
}

void TaskApplet::moveItem(TaskItem *taskItem, bool right)
{
	int currentIndex = m_workButtons.indexOf(taskItem);
	if (right)
	{
		if (currentIndex != (m_workButtons.size() - 1))
		{
			m_workButtons.remove(currentIndex);
			m_workButtons.insert(currentIndex + 1, taskItem);
			updateLayout();
		}
	}
	else
	{
		if (currentIndex != 0)
		{
			m_workButtons.remove(currentIndex);
			m_workButtons.insert(currentIndex - 1, taskItem);
			updateLayout();
		}
	}
}

void TaskApplet::layoutChanged()
{
	updateLayout();
}

QSize TaskApplet::desiredSize()
{
	return QSize(-1, -1); // Take all available space.
}

void TaskApplet::doRegTaskButton(TaskItem *taskItem)
{
	m_workButtons.append(taskItem);
	updateLayout();
//	taskItem->moveInstantly();
}

void TaskApplet::unRegTaskButton(TaskItem *taskItem)
{
	m_workButtons.remove(m_workButtons.indexOf(taskItem));
	updateLayout();
}

TaskItem *TaskApplet::getTaskButtonForClient(TaskClient *client)
{
	if (m_idleButtons.isEmpty())
		return new TaskItem(this, client);

	TaskItem *item = m_idleButtons.first();
	m_idleButtons.remove(0);
	item->updateButtonState(client->clientState());
	item->show();
	doRegTaskButton(item);
	return item;
}

void TaskApplet::addIdleTaskButton(TaskItem *taskItem)
{
	taskItem->hide();
	unRegTaskButton(taskItem);
	m_idleButtons.append(taskItem);
}

bool TaskApplet::hasClient(int pagerIndex) const
{
	foreach (TaskClient *client, m_clients) {
		if (client->pagerIndex() == pagerIndex) return true;
	}
	return false;
}

void TaskApplet::updateVisibilities()
{
	foreach (TaskClient *client, m_clients)
		client->setVisibility();
	updateLayout();
}
//==================================================================================

void TaskApplet::desktopCountChanged(int desktopCount)
{
	bool neetUpdate = (m_pagerCount > desktopCount);
	m_pagerCount = desktopCount;
	if (!neetUpdate) return;

	int maxIndex = desktopCount - 1;
	neetUpdate = false;
	foreach (TaskClient *client, m_clients) {
		if (client->pagerIndex() > maxIndex) {
			client->setPagerIndex(maxIndex);
			neetUpdate = true;
		}
	}

	if (neetUpdate)
		updateVisibilities();
}

void TaskApplet::currentDesktopChanged(int currentIndex)
{
//	qDebug() << "currentDesktopChanged" << currentIndex;
	m_pagerIndex = currentIndex;
	updateVisibilities();
}

void TaskApplet::clientListChanged()
{
//	qDebug() << "clientListChanged";
	if (m_dragging) {
		return; // Don't want new dock items to appear (or old to be removed) while rearranging them with drag and drop.
	}

	QVector<unsigned long> winList = X11Support::getWindowPropertyWindowsArray(X11Support::rootWindow(), "_NET_CLIENT_LIST");
	for (int i = 0; i < winList.size(); i++) {
		if (!m_clients.contains(winList[i])) {
			if (QWidget::find(winList[i]) == NULL) {
				m_clients[winList[i]] = new TaskClient(this, winList[i]);	// new client
			}
		}
	}

	QList<TaskClient *> removedList;
	foreach (TaskClient *client, m_clients) {
		if (!winList.contains(client->handle())) removedList.append(client);
	}

	foreach (TaskClient *client, removedList) {
		m_clients.remove(client->handle());
		deleteClient(client);
	}

	if (PagerApplet::m_instance)
		PagerApplet::m_instance->update();
}

void TaskApplet::activeWindowChanged()
{
	unsigned long lastWindow = m_activeWindow;
	m_activeWindow = X11Support::getWindowPropertyWindow(X11Support::rootWindow(),
														 "_NET_ACTIVE_WINDOW");
//	qDebug() << "activeWindowChanged" << lastWindow << m_activeWindow;
	if (lastWindow == m_activeWindow) return;

	if (m_activeWindow && m_clients.contains(m_activeWindow)) {
		m_clients[m_activeWindow]->updateClientState(ClientStat::Active);
	}

	if (lastWindow && m_clients.contains(lastWindow)) {
		m_clients[lastWindow]->unsetActiveState();
	}
}

void TaskApplet::windowPropertyChanged(unsigned long window, unsigned long atom)
{
//	qDebug() << "windowPropertyChanged" << window;
	if (m_clients.contains(window))
		m_clients[window]->windowPropertyChanged(atom);
}
