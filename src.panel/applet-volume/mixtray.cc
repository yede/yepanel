// mixtray - tray icon for Alsa mixer control
// Copyright (c) 2011, Jonathan Debove
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <QtGui>
#include "MixerIcon.hh"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        std::cerr << qApp->arguments().at(0).toLocal8Bit().constData()
            << ": could not detect any system tray on this system."
            << std::endl;
    }

    QStringList controls(app.arguments());
    QString command("xterm -e alsamixer");

    for (int i = 1; i < controls.size(); i++) {
        QString ctl(controls.at(i));
        if (ctl == "-h") {
            std::cout << "Usage: "
                << controls.at(0).toLocal8Bit().constData()
                << " [-h|-e command] [controls]" << std::endl;
            exit(0);
        }
        if (ctl == "-e") {
            controls.removeAt(i);
            if (i < controls.size())
                command = controls.takeAt(i);
        }
    }

    controls.removeFirst();
    if (controls.isEmpty())
        controls << "Master";

    MixerIcon master(controls, command);
    master.show();

    return app.exec();
}
