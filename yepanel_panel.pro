include(common.pri)
include(install_panel.pri)
# ================================================================================

TEMPLATE	= app
TARGET		= yepanel

unix:!macx {
	CONFIG += link_pkgconfig
	PKGCONFIG += x11 xdamage xcomposite alsa
}
# ================================================================================
INCLUDEPATH += \
	src.cJSON \
	src.common \
	src.panel \
	src.panel/applet-menu \
	src.panel/applet-pager \
	src.panel/applet-taskbar \
	src.panel/applet-network \
	src.panel/applet-acpi \
	src.panel/applet-volume \
	src.panel/applet-clock \
	src.panel/applet-demo \
	src.panel/applet-tray \
	src.panel/applet-spacer

HEADERS += \
	src.panel/applet-menu/menuapplet.h \
	src.panel/applet-menu/menuloader.h \
	src.panel/applet-menu/appitem.h \
	src.panel/applet-pager/pagerapplet.h \
	src.panel/applet-taskbar/taskapplet.h \
	src.panel/applet-taskbar/taskitem.h \
	src.panel/applet-taskbar/taskclient.h \
	src.panel/applet-network/networkapplet.h \
	src.panel/applet-acpi/acpiapplet.h \
	src.panel/applet-acpi/acpiacpower.h \
	src.panel/applet-acpi/acpibattery.h \
	src.panel/applet-acpi/acpithermal.h \
	src.panel/applet-acpi/acpifan.h \
	src.panel/applet-acpi/acpiutils.h \
	src.panel/applet-volume/volumeapplet.h \
	src.panel/applet-volume/alsamixer.h \
	src.panel/applet-volume/alsachannel.h \
	src.panel/applet-volume/alsaelement.h \
	src.panel/applet-clock/clockapplet.h \
	src.panel/applet-demo/demoapplet.h \
	src.panel/applet-tray/trayapplet.h \
	src.panel/applet-tray/trayitem.h \
	src.panel/applet-spacer/spacerapplet.h \
	src.panel/applet.h \
	src.panel/helpwidget.h \
	src.panel/iconloader.h \
	src.panel/x11support.h \
	src.panel/textgraphicsitem.h \
	src.panel/panelsignal.h \
	src.panel/panelapplication.h \
	src.panel/panelsettingsdlg.h \
	src.panel/panelwindow.h \
	src.panel/panelwindowbg.h \
	src.panel/panel.h \
	src.panel/animationutils.h

SOURCES += \
	src.panel/applet-menu/menuapplet.cpp \
	src.panel/applet-menu/menuloader.cpp \
	src.panel/applet-menu/appitem.cpp \
	src.panel/applet-pager/pagerapplet.cpp \
	src.panel/applet-taskbar/taskapplet.cpp \
	src.panel/applet-taskbar/taskitem.cpp \
	src.panel/applet-taskbar/taskclient.cpp \
	src.panel/applet-network/networkapplet.cpp \
	src.panel/applet-acpi/acpiapplet.cpp \
	src.panel/applet-acpi/acpiacpower.cpp \
	src.panel/applet-acpi/acpibattery.cpp \
	src.panel/applet-acpi/acpithermal.cpp \
	src.panel/applet-acpi/acpifan.cpp \
	src.panel/applet-acpi/acpiutils.cpp \
	src.panel/applet-volume/volumeapplet.cpp \
	src.panel/applet-volume/alsamixer.cpp \
	src.panel/applet-volume/alsachannel.cpp \
	src.panel/applet-volume/alsaelement.cpp \
	src.panel/applet-clock/clockapplet.cpp \
	src.panel/applet-demo/demoapplet.cpp \
	src.panel/applet-tray/trayapplet.cpp \
	src.panel/applet-tray/trayitem.cpp \
	src.panel/applet-spacer/spacerapplet.cpp \
	src.panel/applet.cpp \
	src.panel/helpwidget.cpp \
	src.panel/iconloader.cpp \
	src.panel/x11support.cpp \
	src.panel/textgraphicsitem.cpp \
	src.panel/panelsignal.cpp \
	src.panel/panelapplication.cpp \
	src.panel/panelsettingsdlg.cpp \
	src.panel/panelwindow.cpp \
	src.panel/panelwindowbg.cpp \
	src.panel/panel.cpp \
	src.panel/main.cpp

FORMS	+= \
	src.panel/panelsettingsdlg.ui

# ================================================================================

HEADERS += \
	src.cJSON/cJSON.h

SOURCES += \
	src.cJSON/cJSON.c

LIBS	+= -lm
# ================================================================================

HEADERS += \
	src.common/global.h \
	src.common/util.h \
	src.common/jsonfile.h

SOURCES += \
	src.common/global.cpp \
	src.common/util.cpp \
	src.common/jsonfile.cpp

RESOURCES		+= data/imgs.qrc
TRANSLATIONS	+= data/translations/yepanel_zh_CN.ts
# ================================================================================
