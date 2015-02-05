#A task-panel written in Qt4

With start-menu, workspace, task-bar, tray, and several monitor (network, volumn, acpi), and a clock.

Rewritten from qtpanel.

##Depends

xproto<br>
libx11<br>
libxdamage<br>
libxcomposite<br>
alsa-lib<br>
qt4

##Build

	qmake-qt4 yepanel.pro -r -spec linux-g++
	make
	make install

##Archlinux

...
