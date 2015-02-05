#-------------------------------------------------
#
# Project created by Nat 2012-06-28
#
#-------------------------------------------------

TEMPLATE = subdirs
CONFIG  += ordered

SUBDIRS  = \
	panel \
	menueditor \
	netmanager

netmanager.file = yepanel_netmanager.pro

menueditor.file = yepanel_menueditor.pro

panel.file      = yepanel_panel.pro
panel.depend    =

# ================================================================================
