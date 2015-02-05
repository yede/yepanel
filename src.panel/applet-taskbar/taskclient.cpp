#include <QDebug>

#include "taskclient.h"
#include "taskitem.h"
#include "taskapplet.h"
#include "pagerapplet.h"
#include "x11support.h"
//==================================================================================

TaskClient::TaskClient(TaskApplet *taskApplet, unsigned long handle)
	: m_taskApplet(taskApplet)
	, m_handle(handle)
	, m_taskItem(NULL)
{
	m_pagerIndex = X11Support::getWindowDesktopNumber(handle);
	X11Support::registerForWindowPropertyChanges(handle);

	updateVisibility();
	updateName();
	updateIcon();
	updateUrgency();
}

TaskClient::~TaskClient()
{
	Q_ASSERT(m_taskItem == NULL);
}

bool TaskClient::isActive() const
{
	return m_taskApplet->m_activeWindow == m_handle;
}

void TaskClient::removeFromButton()
{
	if (m_taskItem != NULL) {
		m_taskItem->removeClient(this);
		if (m_taskItem->count() < 1) {
			m_taskApplet->unRegTaskButton(m_taskItem);
			delete m_taskItem;
		}
		m_taskItem = NULL;
	}
}

void TaskClient::setVisibility()
{
	bool isCurrent = (m_pagerIndex == m_taskApplet->m_pagerIndex);
	bool visible = m_visible && isCurrent;

	if (visible) {
		if (m_taskItem != NULL) {
			m_taskItem->show();
		} else {
			m_taskItem = m_taskApplet->getTaskButtonForClient(this);
			m_taskItem->addClient(this);
		}
		return;
	}

	if (m_taskItem != NULL) {
		if (m_clientState == ClientStat::Active)
			m_clientState = ClientStat::Normal;
		m_taskItem->hide();
		m_taskApplet->updateLayout();
	}
}

void TaskClient::updateVisibility()
{
	quint32 windowTypes = X11Support::getWindowTypes(m_handle);
	quint32 windowStates = X11Support::getWindowStates(m_handle);
	quint32 normal = setBit(WindowTypeBit::Normal);

	// Show only regular windows in dock.
	// When no window type is set, assume it's normal window.
	m_visible = (windowTypes == 0 || windowTypes == normal);

	// Don't show window if requested explicitly in window states.
	if (getBit(windowStates, WindowStateBit::SkipTaskbar))
		m_visible = false;

	m_clientState = getBit(windowStates, WindowStateBit::Iconic) ? ClientStat::Iconic : ClientStat::Normal;

	setVisibility();
	updateClientState(m_clientState);
}

void TaskClient::updateClientState(int state)
{
	if (!m_taskItem) return;

//	qDebug() << "TaskClient::updateClientState" << state << m_clientState;
	m_clientState = state;
	m_taskItem->updateButtonState(state);
}

void TaskClient::unsetActiveState()
{
	if (!m_taskItem) return;

	quint32 windowStates = X11Support::getWindowStates(m_handle);
	int state = getBit(windowStates, WindowStateBit::Iconic) ? ClientStat::Iconic : ClientStat::Normal;
	m_taskItem->updateButtonState(state);
}

void TaskClient::updateName()
{
	m_name = X11Support::getWindowName(m_handle);
	if (m_taskItem != NULL)
		m_taskItem->updateContent();
}

void TaskClient::updateIcon()
{
	m_icon = X11Support::getWindowIcon(m_handle);
	if (m_taskItem != NULL)
		m_taskItem->updateContent();
}

void TaskClient::updateUrgency()
{
	m_isUrgent = X11Support::getWindowUrgency(m_handle);
	if (m_taskItem != NULL)
		m_taskItem->startAnimation();
}

void TaskClient::updatePagerIndex()
{
	int prevIndex = m_pagerIndex;
	m_pagerIndex = X11Support::getWindowDesktopNumber(m_handle);

	if (prevIndex != m_pagerIndex)
		setVisibility();

	if (PagerApplet::m_instance)
		PagerApplet::m_instance->update();
}

void TaskClient::setPagerIndex(int index)
{
	m_pagerIndex = index;
}

void TaskClient::windowPropertyChanged(unsigned long atom)
{
	if (atom == X11Support::atom("_NET_WM_STATE") ||
		atom == X11Support::atom("_NET_WM_WINDOW_TYPE"))  updateVisibility();

	if (atom == X11Support::atom("WM_NAME")       ||
		atom == X11Support::atom("_NET_WM_NAME")  ||
		atom == X11Support::atom("_NET_WM_VISIBLE_NAME")) updateName();

	if (atom == X11Support::atom("WM_HINTS"))        updateUrgency();
	if (atom == X11Support::atom("_NET_WM_ICON"))    updateIcon();
	if (atom == X11Support::atom("_NET_WM_DESKTOP")) updatePagerIndex();
}
