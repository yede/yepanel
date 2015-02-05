#-------------------------------------------------
# common.pri
#-------------------------------------------------

QT += core gui
CONFIG += qt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#===================================================================================

CONFIG(debug, debug|release) {
	DESTDIR_SUFFIX = .d
	DEFINES += DEBUG_BUILD
	CONFIG += warn_on
	QMAKE_CFLAGS   += -pg
	QMAKE_CXXFLAGS += -pg
	QMAKE_LFLAGS   += -pg
} else {
	DESTDIR_SUFFIX =
	CONFIG += release warn_off
}
# ================================================================================

exists(build-ide.pri) {
	include(build-ide.pri)
} else {
	include(build.pri)
}

DEFINES += D_DATA_DIR=$${D_DATA_DIR}
DEFINES += D_EXEC_DIR=$${DESTDIR}
DEFINES += D_MAIN_APP=yepanel
DEFINES += D_MENU_EDT=yepanel_menueditor
# ================================================================================

message("D_DATA_DIR="$${D_DATA_DIR})
