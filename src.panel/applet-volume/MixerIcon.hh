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

#ifndef MIXER_ICON_HH
#define MIXER_ICON_HH

#include <QSystemTrayIcon>

class QAction;
class AlsaMixer;

class MixerIcon : public QSystemTrayIcon {
    Q_OBJECT

    public:
        MixerIcon(const QStringList &args, const QString &cmd);
        ~MixerIcon();

    protected:
        // Reimplementation of the virtual method
        // QSystemTrayIcon::event(QEvent*)
        bool event(QEvent*);

    private:
        void changeVolume(const int delta);
        void changeIcon(const double volume);

        unsigned int m_index;
        AlsaMixer *m_mixer;
        QStringList m_controls;
        QVector<QAction *> m_actions;
        QString m_command;

    public slots:
        void iconActivated(QSystemTrayIcon::ActivationReason);
        void actionSelected(QAction*);
};

#endif  // MIXER_ICON_HH
