#include <QtGui/QApplication>
#include <QtGui/QX11Info>
#include <QtGui/QImage>
#include <QtCore/QDateTime>
#include <QDebug>

#include "x11support.h"

// Keep all the X11 stuff with scary defines below normal headers.
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
//==================================================================================

static XErrorHandler oldX11ErrorHandler = NULL;

static int x11errorHandler(Display *display, XErrorEvent *error)
{
	if (error->error_code == BadWindow)
		return 0; // This usually happens when querying property on a window that's already gone. That's OK.

	return (*oldX11ErrorHandler)(display, error);
}

X11Support *X11Support::m_instance = NULL;
//==================================================================================

X11Support::X11Support()
{
	m_instance = this;
	oldX11ErrorHandler = XSetErrorHandler(x11errorHandler);
	int damageErrorBase;
	XDamageQueryExtension(QX11Info::display(), &m_damageEventBase, &damageErrorBase);
}

X11Support::~X11Support()
{
	m_instance = NULL;
}

void X11Support::onX11Event(XEvent *event)
{
	if (event->type == m_damageEventBase + XDamageNotify)
	{
		// Repair damaged area.
		XDamageNotifyEvent *damageEvent = reinterpret_cast<XDamageNotifyEvent*>(event);
		XDamageSubtract(QX11Info::display(), damageEvent->damage, None, None);

		emit windowDamaged(event->xany.window);
	}
	if (event->type == DestroyNotify)
		emit windowClosed(event->xdestroywindow.window);
	if (event->type == ConfigureNotify)
		emit windowReconfigured(event->xconfigure.window, event->xconfigure.x, event->xconfigure.y, event->xconfigure.width, event->xconfigure.height);
	if (event->type == PropertyNotify)
		handleWindowPropertyChanged(event->xproperty.window, event->xproperty.atom);
	if (event->type == ClientMessage)
		emit clientMessageReceived(event->xclient.window, event->xclient.message_type, event->xclient.data.b);
}

void X11Support::handleWindowPropertyChanged(unsigned long window, unsigned long atom)
{
	if (window == rootWindow()) {
		if (atom == X11Support::atom("_NET_CLIENT_LIST"))        emit clientListChanged();
		if (atom == X11Support::atom("_NET_ACTIVE_WINDOW"))      emit activeWindowChanged();
		if (atom == X11Support::atom("_NET_CURRENT_DESKTOP"))    emit currentDesktopChanged(getCurrentDesktopNumber());
		if (atom == X11Support::atom("_NET_NUMBER_OF_DESKTOPS")) emit desktopCountChanged(getDesktopCount());
	} else {
		emit windowPropertyChanged(window, atom);
	}
}
//==================================================================================

unsigned long X11Support::rootWindow()
{
	return QX11Info::appRootWindow();
}

unsigned long X11Support::atom(const QString &name)
{
	if (!m_instance->m_cachedAtoms.contains(name))
		m_instance->m_cachedAtoms[name] = XInternAtom(QX11Info::display(), name.toLatin1().data(), False);
	return m_instance->m_cachedAtoms[name];
}
//==================================================================================

void X11Support::sendClientMessage(unsigned long win, long type, long l0, long l1, long l2, long l3, long l4)
{
	XClientMessageEvent xev;

	xev.type = ClientMessage;
	xev.window = win;
	xev.message_type = type;
	xev.format = 32;
	xev.data.l[0] = l0;
	xev.data.l[1] = l1;
	xev.data.l[2] = l2;
	xev.data.l[3] = l3;
	xev.data.l[4] = l4;
	XSendEvent (QX11Info::display(), rootWindow(), False,
				(SubstructureNotifyMask | SubstructureRedirectMask),
				(XEvent *) &xev);
}
/*
void sendClientMessageWm(unsigned long win, Atom type, Atom arg)
{
	XClientMessageEvent xev;

	xev.type = ClientMessage;
	xev.window = win;
	xev.message_type = type;
	xev.format = 32;
	xev.data.l[0] = arg;
	xev.data.l[1] = QDateTime::currentDateTime().toLocalTime();	// GDK_CURRENT_TIME
	XSendEvent(QX11Info::display(), win, False, 0L, (XEvent *) &xev);
}*/
//==================================================================================

void X11Support::removeWindowProperty(unsigned long window, const QString &name)
{
	XDeleteProperty(QX11Info::display(), window, atom(name));
}

void X11Support::setWindowPropertyCardinalArray(unsigned long window, const QString &name, const QVector<unsigned long> &values)
{
	XChangeProperty(QX11Info::display(), window, atom(name), XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char*>(values.data()), values.size());
}

void X11Support::setWindowPropertyCardinal(unsigned long window, const QString &name, unsigned long value)
{
	XChangeProperty(QX11Info::display(), window, atom(name), XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char*>(&value), 1);
}

void X11Support::setWindowPropertyVisualId(unsigned long window, const QString &name, unsigned long value)
{
	XChangeProperty(QX11Info::display(), window, atom(name), XA_VISUALID, 32, PropModeReplace, reinterpret_cast<const unsigned char*>(&value), 1);
}
//==================================================================================

template<class T>
static bool getWindowPropertyHelper(unsigned long window, unsigned long atom,
									unsigned long type, int &numItems, T *&data)
{
	Atom retType;
	int retFormat;
	unsigned long numItemsTemp;
	unsigned long bytesLeft;
	if (XGetWindowProperty(QX11Info::display(), window, atom, 0, 0x7FFFFFFF, False,
						   type, &retType, &retFormat, &numItemsTemp, &bytesLeft,
						   reinterpret_cast<unsigned char**>(&data)) != Success)
		return false;
	numItems = numItemsTemp;
	if (numItems == 0)
		return false;
	return true;
}

unsigned long X11Support::getWindowPropertyCardinal(unsigned long window, const QString &name)
{
	int numItems;
	unsigned long *data;
	unsigned long value = 0;
	if (!getWindowPropertyHelper(window, atom(name), XA_CARDINAL, numItems, data))
		return value;
	value = data[0];
	XFree(data);
	return value;
}

unsigned long X11Support::getWindowPropertyWindow(unsigned long window, const QString &name)
{
	int numItems;
	unsigned long *data;
	unsigned long value = 0;
	if (!getWindowPropertyHelper(window, atom(name), XA_WINDOW, numItems, data))
		return value;
	value = data[0];
	XFree(data);
	return value;
}

QVector<unsigned long> X11Support::getWindowPropertyWindowsArray(unsigned long window, const QString &name)
{
	int numItems;
	unsigned long *data;
	QVector<unsigned long> values;
	if (!getWindowPropertyHelper(window, atom(name), XA_WINDOW, numItems, data))
		return values;
	for (int i = 0; i < numItems; i++)
		values.append(data[i]);
	XFree(data);
	return values;
}

QVector<unsigned long> X11Support::getWindowPropertyAtomsArray(unsigned long window, const QString &name)
{
	int numItems;
	unsigned long *data;
	QVector<unsigned long> values;
	if (!getWindowPropertyHelper(window, atom(name), XA_ATOM, numItems, data))
		return values;
	for (int i = 0; i < numItems; i++)
		values.append(data[i]);
	XFree(data);
	return values;
}

QStringList X11Support::getWindowPropertyUTF8StringList(unsigned long window, const QString &name)
{
	QStringList values;
	int numItems;
	char *data;
	if (!getWindowPropertyHelper(window, atom(name), atom("UTF8_STRING"), numItems, data))
		return values;

	int i = 0;
	while (i < numItems) {
		values.append(QString::fromUtf8(&data[i]));
		while (i < numItems && data[i] != '\0') i++;
		while (i < numItems && data[i] == '\0') i++;
	}

	XFree(data);
	return values;
}

QString X11Support::getWindowPropertyUTF8String(unsigned long window, const QString &name)
{
	int numItems;
	char *data;
	QString value;
	if (!getWindowPropertyHelper(window, atom(name), atom("UTF8_STRING"), numItems, data))
		return value;
	value = QString::fromUtf8(data);
	XFree(data);
	return value;
}

QString X11Support::getWindowPropertyLatin1String(unsigned long window, const QString &name)
{
	int numItems;
	char *data;
	QString value;
	if (!getWindowPropertyHelper(window, atom(name), XA_STRING, numItems, data))
		return value;
	value = QString::fromLatin1(data);
	XFree(data);
	return value;
}
//==================================================================================

int X11Support::getDesktopCount()
{
	return getWindowPropertyCardinal(rootWindow(), "_NET_NUMBER_OF_DESKTOPS");
}

QStringList X11Support::getDesktopNames()
{
	int cnt = getDesktopCount();
	QStringList names = getWindowPropertyUTF8StringList(rootWindow(), "_NET_DESKTOP_NAMES");
	while (names.size() > cnt) names.removeLast();
	return names;
}

int X11Support::getCurrentDesktopNumber()
{
	return getWindowPropertyCardinal(rootWindow(), "_NET_CURRENT_DESKTOP");
}

void X11Support::setCurrentDesktopNumber(int index)
{
//	setWindowPropertyCardinal(rootWindow(), "_NET_CURRENT_DESKTOP", index);
	sendClientMessage(rootWindow(), atom("_NET_CURRENT_DESKTOP"), index, 0, 0, 0, 0);
}

int X11Support::getWindowDesktopNumber(unsigned long window)
{
	return getWindowPropertyCardinal(window, "_NET_WM_DESKTOP");
}
//==================================================================================

quint32 setBit(int bitIndex)
{
	quint32 bit = 1;
	while (bitIndex > 0) { bit = (bit << 1); bitIndex--; }
	return bit;
}

void setBit(quint32 &value, int bitIndex)
{
	quint32 bit = 1;
	while (bitIndex > 0) { bit = (bit << 1); bitIndex--; }
	value |= bit;
}

bool getBit(quint32 value, int bitIndex)
{
	quint32 bit = 1;
	while (bitIndex > 0) { bit = (bit << 1); bitIndex--; }
	return (value & bit);
}

void dumpWindowStates(quint32 value)
{
	qDebug() << "SkipPager"   << getBit(value, WindowStateBit::SkipPager);
	qDebug() << "SkipTaskbar" << getBit(value, WindowStateBit::SkipTaskbar);
	qDebug() << "Sticky"      << getBit(value, WindowStateBit::Sticky);
	qDebug() << "Iconic"      << getBit(value, WindowStateBit::Iconic);
	qDebug() << "Shaded"      << getBit(value, WindowStateBit::Shaded);
}

quint32 X11Support::getWindowStates(unsigned long window)
{
	QVector<unsigned long> states;
	states = getWindowPropertyAtomsArray(window, "_NET_WM_STATE");
	quint32 result = 0;

	foreach (unsigned long state, states) {
		if      (state == atom("_NET_WM_STATE_SKIP_PAGER"))   setBit(result, WindowStateBit::SkipPager);
		else if (state == atom("_NET_WM_STATE_SKIP_TASKBAR")) setBit(result, WindowStateBit::SkipTaskbar);
		else if (state == atom("_NET_WM_STATE_STICKY"))       setBit(result, WindowStateBit::Sticky);
		else if (state == atom("_NET_WM_STATE_HIDDEN"))       setBit(result, WindowStateBit::Iconic);
		else if (state == atom("_NET_WM_STATE_SHADED"))       setBit(result, WindowStateBit::Shaded);
	}

	return result;
}

void dumpWindowTypes(quint32 value)
{
	qDebug() << "Desktop" << getBit(value, WindowTypeBit::Desktop);
	qDebug() << "Dock   " << getBit(value, WindowTypeBit::Dock);
	qDebug() << "Toolbar" << getBit(value, WindowTypeBit::Toolbar);
	qDebug() << "Menu   " << getBit(value, WindowTypeBit::Menu);
	qDebug() << "Utility" << getBit(value, WindowTypeBit::Utility);
	qDebug() << "Splash " << getBit(value, WindowTypeBit::Splash);
	qDebug() << "Dialog " << getBit(value, WindowTypeBit::Dialog);
	qDebug() << "Normal " << getBit(value, WindowTypeBit::Normal);
}

quint32 X11Support::getWindowTypes(unsigned long window)
{
	QVector<unsigned long> types;
	types = getWindowPropertyAtomsArray(window, "_NET_WM_WINDOW_TYPE");
	quint32 result = 0;

	foreach (unsigned long type, types) {
		if      (type == atom("_NET_WM_WINDOW_TYPE_DESKTOP")) setBit(result, WindowTypeBit::Desktop);
		else if (type == atom("_NET_WM_WINDOW_TYPE_DOCK"))    setBit(result, WindowTypeBit::Dock);
		else if (type == atom("_NET_WM_WINDOW_TYPE_TOOLBAR")) setBit(result, WindowTypeBit::Toolbar);
		else if (type == atom("_NET_WM_WINDOW_TYPE_MENU"))    setBit(result, WindowTypeBit::Menu);
		else if (type == atom("_NET_WM_WINDOW_TYPE_UTILITY")) setBit(result, WindowTypeBit::Utility);
		else if (type == atom("_NET_WM_WINDOW_TYPE_SPLASH"))  setBit(result, WindowTypeBit::Splash);
		else if (type == atom("_NET_WM_WINDOW_TYPE_DIALOG"))  setBit(result, WindowTypeBit::Dialog);
		else if (type == atom("_NET_WM_WINDOW_TYPE_NORMAL"))  setBit(result, WindowTypeBit::Normal);
	}

	return result;
}
//==================================================================================

QString X11Support::getWindowName(unsigned long window)
{
	QString result = getWindowPropertyUTF8String(window, "_NET_WM_VISIBLE_NAME");
	if (result.isEmpty())
		result = getWindowPropertyUTF8String(window, "_NET_WM_NAME");
	if (result.isEmpty())
		result = getWindowPropertyLatin1String(window, "WM_NAME");
	if (result.isEmpty())
		result = "<Unknown>";
	return result;
}

QIcon X11Support::getWindowIcon(unsigned long window)
{
	int numItems;
	unsigned long *rawData;
	QIcon icon;
	if (!getWindowPropertyHelper(window, atom("_NET_WM_ICON"), XA_CARDINAL, numItems, rawData))
		return icon;
	unsigned long *data = rawData;
	while (numItems > 0)
	{
		int width = static_cast<int>(data[0]);
		int height = static_cast<int>(data[1]);
		data += 2;
		numItems -= 2;
		QImage image(width, height, QImage::Format_ARGB32);
		for (int i = 0; i < height; i++)
		{
			for (int k = 0; k < width; k++)
			{
				image.setPixel(k, i, static_cast<unsigned int>(data[i*width + k]));
			}
		}
		data += width*height;
		numItems -= width*height;
		icon.addPixmap(QPixmap::fromImage(image));
	}
	XFree(rawData);
	return icon;
}

bool X11Support::getWindowUrgency(unsigned long window)
{
	XWMHints *hints = XGetWMHints(QX11Info::display(), window);
	if (hints == NULL)
		return false;
	bool isUrgent = (hints->flags & 256) != 0; // UrgencyHint
	XFree(hints);
	return isUrgent;
}
//==================================================================================

void X11Support::registerForWindowPropertyChanges(unsigned long window)
{
	XSelectInput(QX11Info::display(), window, PropertyChangeMask);
}

void X11Support::registerForTrayIconUpdates(unsigned long window)
{
	XSelectInput(QX11Info::display(), window, StructureNotifyMask);

	// Apparently, there is no need to destroy damage object, as it's gone automatically when window is destroyed.
	XDamageCreate(QX11Info::display(), window, XDamageReportNonEmpty);
}

static void sendNETWMMessage(unsigned long window, const QString &atomName, unsigned long l0 = 0, unsigned long l1 = 0, unsigned long l2 = 0, unsigned long l3 = 0, unsigned long l4 = 0)
{
	XClientMessageEvent event;
	event.type = ClientMessage;
	event.window = window;
	event.message_type = X11Support::atom(atomName);
	event.format = 32;
	event.data.l[0] = l0;
	event.data.l[1] = l1;
	event.data.l[2] = l2;
	event.data.l[3] = l3;
	event.data.l[4] = l4;
	XSendEvent(QX11Info::display(), X11Support::rootWindow(), False, SubstructureNotifyMask | SubstructureRedirectMask, reinterpret_cast<XEvent*>(&event));
}

void X11Support::activateWindow(unsigned long window)
{
	XWindowChanges wc;
	wc.stack_mode = Above;
	XConfigureWindow(QX11Info::display(), window, CWStackMode, &wc);

	// Apparently, KWin won't bring window to top with configure request,
	// so we also need to ask it politely by sending a message.
	sendNETWMMessage(window, "_NET_ACTIVE_WINDOW", 2, CurrentTime);
}

void X11Support::minimizeWindow(unsigned long window)
{
	XIconifyWindow(QX11Info::display(), window, QX11Info::appScreen());
}

void X11Support::closeWindow(unsigned long window)
{
	sendNETWMMessage(window, "_NET_CLOSE_WINDOW", CurrentTime, 2);
}

void X11Support::destroyWindow(unsigned long window)
{
	XDestroyWindow(QX11Info::display(), window);
}

void X11Support::killClient(unsigned long window)
{
	XKillClient(QX11Info::display(), window);
}

unsigned long X11Support::systemTrayAtom()
{
	return atom(QString("_NET_SYSTEM_TRAY_S") + QString::number(QX11Info::appScreen()));
}

bool X11Support::makeSystemTray(unsigned long window)
{
	if (XGetSelectionOwner(QX11Info::display(), systemTrayAtom()) != 0)
		return false;

	XSetSelectionOwner(QX11Info::display(), systemTrayAtom(), window, CurrentTime);
	setWindowPropertyVisualId(window, "_NET_SYSTEM_TRAY_VISUAL", getARGBVisualId());
	XSync(QX11Info::display(), False);

	// Inform other clients.
	XClientMessageEvent event;
	event.type = ClientMessage;
	event.window = rootWindow();
	event.message_type = atom("MANAGER");
	event.format = 32;
	event.data.l[0] = CurrentTime;
	event.data.l[1] = systemTrayAtom();
	event.data.l[2] = window;
	event.data.l[3] = 0;
	event.data.l[4] = 0;
	XSendEvent(QX11Info::display(), X11Support::rootWindow(), False, StructureNotifyMask, reinterpret_cast<XEvent*>(&event));

	return true;
}

void X11Support::freeSystemTray()
{
	XSetSelectionOwner(QX11Info::display(), systemTrayAtom(), None, CurrentTime);
}

unsigned long X11Support::getARGBVisualId()
{
	XVisualInfo visualInfoTemplate;
	visualInfoTemplate.screen = QX11Info::appScreen();
	visualInfoTemplate.depth = 32;
	visualInfoTemplate.red_mask = 0x00FF0000;
	visualInfoTemplate.green_mask = 0x0000FF00;
	visualInfoTemplate.blue_mask = 0x000000FF;

	int numVisuals;
	XVisualInfo *visualInfoList = XGetVisualInfo(QX11Info::display(), VisualScreenMask | VisualDepthMask | VisualRedMaskMask | VisualGreenMaskMask | VisualBlueMaskMask, &visualInfoTemplate, &numVisuals);
	unsigned long id = visualInfoList[0].visualid;
	XFree(visualInfoList);

	return id;
}

void X11Support::redirectWindow(unsigned long window)
{
	XCompositeRedirectWindow(QX11Info::display(), window, CompositeRedirectManual);
}

void X11Support::unredirectWindow(unsigned long window)
{
	XCompositeUnredirectWindow(QX11Info::display(), window, CompositeRedirectManual);
}

QPixmap X11Support::getWindowPixmap(unsigned long window)
{
	return QPixmap::fromX11Pixmap(XCompositeNameWindowPixmap(QX11Info::display(), window));
}

void X11Support::resizeWindow(unsigned long window, int width, int height)
{
	XResizeWindow(QX11Info::display(), window, width, height);
}

void X11Support::moveWindow(unsigned long window, int x, int y)
{
	XMoveWindow(QX11Info::display(), window, x, y);
}

void X11Support::mapWindow(unsigned long window)
{
	XMapWindow(QX11Info::display(), window);
}

void X11Support::reparentWindow(unsigned long window, unsigned long parent)
{
	XReparentWindow(QX11Info::display(), window, parent, 0, 0);
	XSync(QX11Info::display(), False);
}

void X11Support::setWindowBackgroundBlack(unsigned long window)
{
	XSetWindowBackground(QX11Info::display(), window, BlackPixel(QX11Info::display(), QX11Info::appScreen()));
}
