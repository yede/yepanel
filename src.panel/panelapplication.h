#ifndef PANELAPPLICATION_H
#define PANELAPPLICATION_H

#include <QtGui/QApplication>
#include <QtGui/QFont>
#include "panel.h"
//==================================================================================

class X11Support;
class IconLoader;
class MenuLoader;
class PanelSettingsDlg;
class PanelWindow;

class PanelApplication: public QApplication
{
	Q_OBJECT
public:
	PanelApplication(int &argc, char **argv);
	~PanelApplication();

	static PanelApplication *instance()
	{
		return m_instance;
	}

	bool x11EventFilter(XEvent *event);

	void init();
	void start();
	void stop();
	void clear();

	const Panel &panel()     const { return m_panel; }
	const QFont &panelFont() const { return m_panel.fontNormal; }

signals:
	void restart();

public slots:
	void onRestart();
	void showMenuEditor();
	void showConfigurationDialog();

private:
	friend class PanelSettingsDlg;
	static PanelApplication *m_instance;
	PanelSettingsDlg *m_settingsDlg;

	X11Support *m_x11support;
	IconLoader *m_iconLoader;
	MenuLoader *m_menuLoader;

	Panel m_panel;
	QVector<PanelWindow*> m_panelWindows;
};

#endif
