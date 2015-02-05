#ifndef MENULOADER_H
#define MENULOADER_H

#include <QtCore/QMetaType>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QThread>

#include "appitem.h"
#include "jsonfile.h"
//==================================================================================

namespace MenuType {
	enum { SystemMenu, CustomMenu };
}

struct AppTime
{
	AppTime(QDateTime time)
		: lastModified(time), appItem(0), isModified(true), isInuse(true) {}

	void checkUpdate(QDateTime time)
	{
		if (lastModified < time) {
			lastModified = time;
			isModified = true;
		}
		isInuse = true;
	}

	void update(AppItem *item)
	{
		appItem = item;
		isModified = false;
	}

	QDateTime lastModified;
	AppItem  *appItem;
	bool      isModified;
	bool      isInuse;
};

//Q_DECLARE_METATYPE(AppItem)
//==================================================================================

class QDir;
class AppItem;

class MenuLoader: public QThread
{
	Q_OBJECT
public:
	MenuLoader();
	~MenuLoader();

	static MenuLoader *instance() { return m_instance; }
	static void startLoader();
	static void stopLoader();

signals:
	void itemUpdated(const AppItem &app);
	void itemRemoved(const QString &path);
	void customMenuUpdated();

protected:
	void run();

private:
	void waitTask();
	void parseCustomMenu();
	void parseSystemMenu();

	void clearWatcher();
	void setupWatcher(const QStringList &dirs, const QStringList &files);
	void clearCustomMenu();
	void clearSystemMenu();
	void readCustomList();
	void readSystemList(QStringList &dirs, QStringList &files);
	void traverse(const QDir &dir, QStringList &dirs, QStringList &files);
	void reload(int type, bool checkClear = false);
	void ensureStart();
	void ensureStop();

	static void  addSep(void *parent, void *userData);
	static void  addItem(void *parent, const QString &title, const QString &icon, const QString &exec, void *userData);
	static void *addMenu(void *parent, const QString &title, const QString &icon, void *userData);

private slots:
	void directoryChanged(const QString &path);
	void fileChanged(const QString &path);
	void onTimeOut();

private:
	static MenuLoader *m_instance;

	QTimer *m_updateTimer;
	QFileSystemWatcher *m_watcher;

	bool           m_abortWorker;
	bool           m_sleeping;

	int            m_watchType;
	JsonFile       m_customFile;
	bool           m_customFileChanged;
	QHash<QString, AppTime> m_systemHash;
	QMutex         m_mutex;
	QWaitCondition m_waitCondition;
};

#endif
