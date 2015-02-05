include(common.pri)
include(install_menueditor.pri)
# ================================================================================

TEMPLATE	= app
TARGET		= yepanel_menueditor

unix:!macx {
	CONFIG += link_pkgconfig
	PKGCONFIG +=
}
# ================================================================================
INCLUDEPATH += \
	src.cJSON \
	src.common \
	src.menueditor

HEADERS += \
	src.menueditor/mewindow.h \
	src.menueditor/metree.h \
	src.menueditor/meutil.h \
	src.menueditor/medelegate.h

SOURCES += \
	src.menueditor/memain.cpp \
	src.menueditor/mewindow.cpp \
	src.menueditor/metree.cpp \
	src.menueditor/meutil.cpp \
	src.menueditor/medelegate.cpp

FORMS	+= \
	src.menueditor/mewindow.ui
# ================================================================================

HEADERS += \
	src.cJSON/cJSON.h

SOURCES += \
	src.cJSON/cJSON.c

LIBS	+= -lm
# ================================================================================

HEADERS += \
	src.common/global.h \
	src.common/jsonfile.h

SOURCES += \
	src.common/global.cpp \
	src.common/jsonfile.cpp

RESOURCES		+= data/imgs.qrc
TRANSLATIONS	+= data/translations/yepanel_menueditor_zh_CN.ts
# ================================================================================
