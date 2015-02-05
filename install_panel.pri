
target.path = /usr/bin

docs.path  += /usr/share/doc/yepanel
docs.files += README CHANGELOG LICENSE

trans.path  += /usr/share/yepanel/translations
trans.files += \
	data/translations/yepanel_zh_CN.qm

INSTALLS += target docs trans
