#ifndef X11SUPPORT_H
#define X11SUPPORT_H

#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>
//==================================================================================

namespace WindowStateBit {
	enum { SkipPager = 0, SkipTaskbar, Sticky, Iconic, Shaded };
}

namespace WindowTypeBit {
	enum { Normal = 0, Dialog, Splash, Utility, Menu, Toolbar, Dock, Desktop };
}

quint32 setBit(int bitIndex);
void setBit(quint32 &value, int bitIndex);
bool getBit(quint32 value, int bitIndex);
void dumpWindowStates(quint32 value);
void dumpWindowTypes(quint32 value);
//==================================================================================

class X11Support: public QObject
{
	Q_OBJECT
public:
	X11Support();
	~X11Support();

	void onX11Event(XEvent *event);
	void handleWindowPropertyChanged(unsigned long window, unsigned long atom);

	static X11Support *instance() { return m_instance; }

	static unsigned long rootWindow();
	static unsigned long atom(const QString &name);

	static void sendClientMessage(unsigned long win, long type, long l0, long l1, long l2, long l3, long l4);

	static void removeWindowProperty(unsigned long window, const QString &name);
	static void setWindowPropertyCardinalArray(unsigned long window, const QString &name, const QVector<unsigned long> &values);
	static void setWindowPropertyCardinal(unsigned long window, const QString &name, unsigned long value);
	static void setWindowPropertyVisualId(unsigned long window, const QString &name, unsigned long value);

	static unsigned long getWindowPropertyCardinal(unsigned long window, const QString &name);
	static unsigned long getWindowPropertyWindow(unsigned long window, const QString &name);
	static QVector<unsigned long> getWindowPropertyWindowsArray(unsigned long window, const QString &name);
	static QVector<unsigned long> getWindowPropertyAtomsArray(unsigned long window, const QString &name);
	static QStringList getWindowPropertyUTF8StringList(unsigned long window, const QString &name);
	static QString getWindowPropertyUTF8String(unsigned long window, const QString &name);
	static QString getWindowPropertyLatin1String(unsigned long window, const QString &name);

	static int getDesktopCount();
	static QStringList getDesktopNames();
	static int getCurrentDesktopNumber();
	static void setCurrentDesktopNumber(int index);
	static int getWindowDesktopNumber(unsigned long window);

	static quint32 getWindowStates(unsigned long window);
	static quint32 getWindowTypes(unsigned long window);
	static QString getWindowName(unsigned long window);
	static QIcon getWindowIcon(unsigned long window);
	static bool getWindowUrgency(unsigned long window);

	static void registerForWindowPropertyChanges(unsigned long window);
	static void registerForTrayIconUpdates(unsigned long window);
	static void activateWindow(unsigned long window);
	static void minimizeWindow(unsigned long window);
	static void closeWindow(unsigned long window);
	static void destroyWindow(unsigned long window);
	static void killClient(unsigned long window);
	static bool makeSystemTray(unsigned long window);
	static void freeSystemTray();
	static unsigned long getARGBVisualId();
	static void redirectWindow(unsigned long window);
	static void unredirectWindow(unsigned long window);
	static QPixmap getWindowPixmap(unsigned long window);
	static void resizeWindow(unsigned long window, int width, int height);
	static void moveWindow(unsigned long window, int x, int y);
	static void mapWindow(unsigned long window);
	static void reparentWindow(unsigned long window, unsigned long parent);
	static void setWindowBackgroundBlack(unsigned long window);

signals:
	void windowClosed(unsigned long window);
	void windowReconfigured(unsigned long window, int x, int y, int width, int height);
	void windowDamaged(unsigned long window);
	void clientMessageReceived(unsigned long window, unsigned long atom, void *data);

	void desktopCountChanged(int desktopCount);
	void currentDesktopChanged(int currentDesktopNumber);
	void clientListChanged();
	void activeWindowChanged();
	void windowPropertyChanged(unsigned long window, unsigned long atom);

private:
	static unsigned long systemTrayAtom();

	static X11Support *m_instance;
	int m_damageEventBase;
	QMap<QString, unsigned long> m_cachedAtoms;
};

#endif
