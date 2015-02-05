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

//#include <iostream>

#include <QtGui>

#include "AlsaMixer.hh"
#include "MixerIcon.hh"

MixerIcon::MixerIcon(const QStringList &args, const QString &cmd)
: QSystemTrayIcon(), m_index(0), m_mixer(NULL), m_controls(args),
    m_actions(args.size()), m_command(cmd) {
    // Init menu
    QMenu *menu = new QMenu();

    unsigned int nc = args.size();
    std::vector<const char *> ctrls(nc);
    for (unsigned int i = 0; i < nc; ++i) {
        // Fill menu
        QAction *a = new QAction(m_controls[i], this);
        m_actions[i] = a;
        menu->addAction(a);

        // Convert QString to const char*
        QByteArray *b = new QByteArray(m_controls[i].toLocal8Bit());
        ctrls[i] = b->constData();
    }

    // Finalize menu
    QAction *quitAction = new QAction(tr("&Quit"), this);
    menu->addSeparator();
    menu->addAction(quitAction);
    connect(menu, SIGNAL(triggered(QAction*)),
            this, SLOT(actionSelected(QAction*)));
    setContextMenu(menu);

    // Init alsa mixer
    m_mixer = new AlsaMixer(ctrls);
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    changeVolume(0);
}

MixerIcon::~MixerIcon() {
    for (int i = 0; i < m_actions.size(); i++) {
        delete m_actions[i];
    }
    delete m_mixer;
}

bool MixerIcon::event(QEvent* e) {
    if (e->type() == QEvent::Wheel) {
        QWheelEvent *wEvent = static_cast<QWheelEvent *>(e);
        changeVolume(wEvent->delta());
        return true;
    }

    return QSystemTrayIcon::event(e);
}

void MixerIcon::changeVolume(const int delta) {
    double volume = m_mixer->getElementVolume(m_index) + delta * 5 / 72.0;

    if (volume < 0)
        volume = 0;
    if (volume > 100)
        volume = 100;

    if (delta != 0)
        m_mixer->setElementVolume(m_index, volume);

    setToolTip(m_controls.at(m_index) % ": " % QString::number(volume, 'f', 1) % "%");
    changeIcon(volume);
}

void MixerIcon::changeIcon(const double volume) {
    if (m_mixer->isElementMuted(m_index))
        setIcon(QIcon(":icons/audio-volume-muted.svg"));
    else {
        int roundVol = static_cast<int>(volume);
        if (volume - roundVol >= 0.5)
            roundVol++;
        switch (roundVol * 3 / 100) {
            case 0:
                setIcon(QIcon(":icons/audio-volume-low.svg"));
                break;
            case 1:
                setIcon(QIcon(":icons/audio-volume-medium.svg"));
                break;
            default:
                setIcon(QIcon(":icons/audio-volume-high.svg"));
        }
    }
}

void MixerIcon::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::DoubleClick:
            system(m_command.toLocal8Bit());
            break;
        case QSystemTrayIcon::MiddleClick:
            m_mixer->toggleElementMute(m_index);
            changeVolume(0);
            break;
        default:
            ;
    }
}

void MixerIcon::actionSelected(QAction *a) {
    for (int i = 0; i < m_actions.size(); ++i) {
        if (m_actions[i] == a) {
            m_index = i;
            changeVolume(0);
            return;
        }
    }
    qApp->quit();
}
