#ifndef TASKAPPLET_H
#define TASKAPPLET_H

#include <QtCore/QVector>
#include <QtCore/QMap>
#include "applet.h"
//==================================================================================

class TaskItem;
class TaskClient;

class TaskApplet: public Applet
{
	Q_OBJECT
public:
	TaskApplet(PanelWindow *panelWindow);
	~TaskApplet();

	bool start();
	bool stop();
	QSize desiredSize();

	void doRegTaskButton(TaskItem *taskItem);
	void unRegTaskButton(TaskItem *taskItem);

	TaskItem *getTaskButtonForClient(TaskClient *client);
	void addIdleTaskButton(TaskItem *taskItem);
	bool hasClient(int pagerIndex) const;

	void updateLayout();
	void updateVisibilities();

	PanelWindow *panelWindow() const { return m_panelWindow; }
	unsigned long activeWindow() const { return m_activeWindow; }

	void draggingStarted();
	void draggingStopped();
	void moveItem(TaskItem *taskItem, bool right);

private:
	void clearClients();
	void clearButtons();
	void deleteClient(TaskClient *client);

protected:
	void layoutChanged();

private slots:
	void desktopCountChanged(int desktopCount);
	void currentDesktopChanged(int currentIndex);
	void clientListChanged();
	void activeWindowChanged();
	void windowPropertyChanged(unsigned long window, unsigned long atom);

private:
	friend class PagerApplet;
	friend class TaskClient;

	static TaskApplet *m_instance;
	QMap<unsigned long, TaskClient*> m_clients;
	QVector<TaskItem*> m_workButtons;
	QVector<TaskItem*> m_idleButtons;
	unsigned long m_activeWindow;
	bool m_dragging;
	int m_pagerCount;
	int m_pagerIndex;
};

#endif
