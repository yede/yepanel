#ifndef TASKCLIENT_H
#define TASKCLIENT_H

#include <QtGui/QIcon>
//==================================================================================

namespace ClientStat {
	enum { Normal, Active, Iconic };
}

class TaskApplet;
class TaskItem;

// Used for tracking connected windows (X11 clients).
// TaskClient may have it's TaskItem, but not necessary (for example, special windows are not shown in dock).
class TaskClient
{
public:
	TaskClient(TaskApplet *taskApplet, unsigned long handle);
	~TaskClient();

	void removeFromButton();
	void windowPropertyChanged(unsigned long atom);
	void updateClientState(int state);
	void unsetActiveState();
	void setVisibility();
	void setPagerIndex(int index);

	unsigned long  handle()      const { return m_handle; }
	int            pagerIndex()  const { return m_pagerIndex; }
	const QString &name()        const { return m_name; }
	const QIcon   &icon()        const { return m_icon; }
	int            clientState() const { return m_clientState; }
	bool           isUrgent()    const { return m_isUrgent; }
	bool           isVisible()   const { return m_visible; }
	bool           isActive()    const;

private:
	void updateVisibility();
	void updateName();
	void updateIcon();
	void updateUrgency();
	void updatePagerIndex();

	TaskApplet *m_taskApplet;
	unsigned long m_handle;
	TaskItem *m_taskItem;
	int m_pagerIndex;
	QString m_name;
	QIcon m_icon;
	int m_clientState;
	bool m_isUrgent;
	bool m_visible;
};

#endif
