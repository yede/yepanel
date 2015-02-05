#include <QtCore/QMutexLocker>
#include <QtCore/QDir>
#include <QDebug>

#include "menuloader.h"
#include "iconloader.h"
#include "panelapplication.h"
#include "global.h"
//==================================================================================

namespace WatchType {
	enum { System = 0x01, Custom = 0x02 };
}

MenuLoader::MenuLoader()
	: m_abortWorker(false)
	, m_sleeping(false)
	, m_watchType(0)
	, m_customFile(Global::menuFile(), this)
	, m_customFileChanged(false)
{
	m_instance = this;

	m_updateTimer = new QTimer();
	m_updateTimer->setSingleShot(true);
	connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(onTimeOut()));

	m_watcher = new QFileSystemWatcher();
	connect(m_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChanged(QString)));
	connect(m_watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));
}

MenuLoader::~MenuLoader()
{
	delete m_watcher;
	delete m_updateTimer;

	m_instance = NULL;
}
//==================================================================================

void MenuLoader::addSep(void *parent, void *userData)
{
	Q_UNUSED(userData);
	AppMenu::addCustomNode(NodeType::SepItem, QString(), QString(), QString(), (AppMenu *) parent);
}

void MenuLoader::addItem(void *parent, const QString &title, const QString &icon, const QString &exec, void *userData)
{
	Q_UNUSED(userData);
	AppMenu::addCustomNode(NodeType::AppItem, title, icon, exec, (AppMenu *) parent);
}

void *MenuLoader::addMenu(void *parent, const QString &title, const QString &icon, void *userData)
{
	Q_UNUSED(userData);
	return AppMenu::addCustomNode(NodeType::AppMenu, title, icon, QString(), (AppMenu *) parent);
}
//==================================================================================

namespace TaskType {
	enum { CustomTask, SystemTask, AbortTask };
}

void MenuLoader::waitTask()
{
	QMutexLocker lock(&m_mutex);

	m_sleeping = true;
	m_waitCondition.wait(&m_mutex);
	m_sleeping = false;
}

void MenuLoader::parseSystemMenu()
{
	QMutexLocker lock(&m_mutex);

	QHash<QString, AppTime>::iterator i = m_systemHash.begin();
	while (i != m_systemHash.end()) {
		AppTime &appTime = i.value();
		if (appTime.isModified) {
			AppItem *item = AppMenu::addSystemItem(i.key());
			appTime.update(item);
		}
		i++;
	}
}

void MenuLoader::parseCustomMenu()
{
	QMutexLocker lock(&m_mutex);

	if (m_customFileChanged) {
		m_customFileChanged = false;
		m_customFile.loadFile(addMenu, addItem, addSep);
	}
}

void MenuLoader::run()
{
	for (;;) {
		parseCustomMenu();
		parseSystemMenu();
		waitTask();

		if (m_abortWorker) break;
	}
}
//==================================================================================

void MenuLoader::directoryChanged(const QString &path)
{
	Q_UNUSED(path);
	if (m_abortWorker) return;

	m_watchType |= WatchType::System;
	m_updateTimer->stop();				// m_watcher emit twice
	m_updateTimer->start(5000);
}

void MenuLoader::fileChanged(const QString &path)
{
	if (m_abortWorker) return;

	m_updateTimer->stop();				// m_watcher emit twice
	if (Global::menuFile() == path) {
		m_watchType |= WatchType::Custom;
		m_updateTimer->start(200);
	} else {
		m_watchType |= WatchType::System;
		m_updateTimer->start(5000);
	}
}

void MenuLoader::onTimeOut()
{
	if (m_abortWorker) return;

	int type = m_watchType;
	m_watchType = 0;

	reload(type, true);
}
//==================================================================================

void MenuLoader::reload(int type, bool checkClear)
{
	QStringList dirs, files;
	clearWatcher();

	while (!m_sleeping) {
		msleep(2);
	}

	m_mutex.lock();
	if (type & WatchType::Custom) readCustomList();
	if (type & WatchType::System) readSystemList(dirs, files);
	if (checkClear) {
		if (type & WatchType::Custom) clearCustomMenu();
		if (type & WatchType::System) clearSystemMenu();
	}
	m_mutex.unlock();

	m_waitCondition.wakeAll();

	setupWatcher(dirs, files);
}
//==================================================================================

void MenuLoader::clearWatcher()
{
	if (!m_watcher->directories().isEmpty())
		m_watcher->removePaths(m_watcher->directories());

	if (!m_watcher->files().isEmpty())
		m_watcher->removePaths(m_watcher->files());
}

void MenuLoader::setupWatcher(const QStringList &dirs, const QStringList &files)
{
	Q_UNUSED(files);
	if (!dirs.isEmpty()) m_watcher->addPaths(dirs);
//	if (!files.isEmpty()) m_watcher->addPaths(files);
	m_watcher->addPath(Global::menuFile());
}
//==================================================================================

void MenuLoader::clearCustomMenu()
{
	AppMenu::clearCustomMenu();
}

void MenuLoader::clearSystemMenu()
{
	QList<AppItem *> deleteList;
	QHash<QString, AppTime>::iterator i = m_systemHash.begin();

	while (i != m_systemHash.end()) {
		AppTime &appTime = i.value();
		if (!appTime.isInuse) {
			if (appTime.appItem) deleteList.append(appTime.appItem);
			i = m_systemHash.erase(i);
		} else {
			if (appTime.isModified) {
				if (appTime.appItem) deleteList.append(appTime.appItem);
				appTime.appItem = 0;
			}
			i++;
		}
	}
	AppMenu::clearSystemMenu(deleteList);
}
//==================================================================================

void MenuLoader::readCustomList()
{
	m_customFileChanged = true;
}

void MenuLoader::readSystemList(QStringList &dirs, QStringList &files)
{
	QHash<QString, AppTime>::iterator i = m_systemHash.begin();
	while (i != m_systemHash.end()) {
		i.value().isInuse = false;
		i++;
	}

	char *env = getenv("XDG_DATA_DIRS");
	QString xdgDataDirs = env ? env : "/usr/local/share/:/usr/share/";
	QStringList paths = xdgDataDirs.split(':');

	foreach (const QString &path, paths) {
		QDir dir(path);
		if (!dir.exists()) continue;
		traverse(QDir(dir.absoluteFilePath("applications")), dirs, files);
	}
}

void MenuLoader::traverse(const QDir &dir, QStringList &dirs, QStringList &files)
{
	if (!dir.exists()) return;

	const Panel &cfg = PanelApplication::instance()->panel();
	dirs.append(dir.canonicalPath());

	QDir::Filters filters = QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files;
	QFileInfoList fileInfos = dir.entryInfoList(QStringList("*.desktop"), filters);

	foreach (const QFileInfo &fileInfo, fileInfos) {
		if (fileInfo.isDir()) {
			QString name = fileInfo.fileName();
			if (!cfg.menuSkipNames.contains(name)) {
				traverse(fileInfo.canonicalFilePath(), dirs, files);
			} else {
				qDebug() << "skip menu goup:" << name;
			}
		} else {
			QString path = fileInfo.canonicalFilePath();
		//	files.append(path);

			QDateTime lastModified = fileInfo.lastModified();
			QHash<QString, AppTime>::iterator i = m_systemHash.find(path);
			if (i == m_systemHash.end()) {
				m_systemHash.insert(path, AppTime(lastModified));
			} else {
				AppTime &appTime = i.value();
				appTime.checkUpdate(lastModified);
			}
		}
	}
}
//==================================================================================

void MenuLoader::ensureStart()
{
	m_sleeping = false;
	start(QThread::IdlePriority);

	reload(WatchType::System | WatchType::Custom);
}

void MenuLoader::ensureStop()
{
	m_abortWorker = true;
	m_updateTimer->stop();

	while (isRunning()) {
		m_waitCondition.wakeAll();
		msleep(2);
	}
}
//==================================================================================

void MenuLoader::startLoader()
{
	instance()->ensureStart();
}

void MenuLoader::stopLoader()
{
	instance()->ensureStop();
}
