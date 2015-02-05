#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QProcess>
#include <QDebug>

#include "panelapplication.h"
#include "panelsettingsdlg.h"
#include "panelsignal.h"
#include "panelwindow.h"
#include "helpwidget.h"
#include "iconloader.h"
#include "x11support.h"
#include "menuloader.h"
#include "menuapplet.h"
#include "pagerapplet.h"
#include "taskapplet.h"
#include "global.h"
#include "appitem.h"
//==================================================================================

// panelapplication.h:
PanelApplication *PanelApplication::m_instance = NULL;
// iconloader.h:
IconLoader *IconLoader::m_instance = NULL;
// menuloader.h:
MenuLoader *MenuLoader::m_instance = NULL;
// appitem.h:
MenuApplet *AppMenu::m_menuApplet  = 0;
AppMenu    *AppMenu::m_mainAppMenu = 0;
// pagerapplet.h:
PagerApplet *PagerApplet::m_instance = 0;
// taskapplet.h:
TaskApplet *TaskApplet::m_instance = 0;
// helpwidget.h:
HelpWidget *HelpWidget::m_instance = 0;
HelpPosition HelpWidget::m_position;
//==================================================================================

PanelApplication::PanelApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_settingsDlg(NULL)
{
    m_instance = this;
    init();
    PanelSignal::initSignalHandler();
    connect(this, SIGNAL(restart()), this, SLOT(onRestart()));
}

PanelApplication::~PanelApplication()
{
    PanelSignal::clearSignalHandler();

    stop();
    clear();

    delete m_menuLoader;
    delete m_iconLoader;
    delete m_x11support;

    if (m_settingsDlg != NULL) delete m_settingsDlg;
    HelpWidget::destroyInstance();
    m_instance = NULL;
}

bool PanelApplication::x11EventFilter(XEvent *event)
{
    m_x11support->onX11Event(event);
    return false;
}

void PanelApplication::init()
{
    Global::initPath();
    Global::initTranslator(Global::appName());

    m_panel.defaultSettings();
    HelpWidget::createInstance();

    m_x11support = new X11Support();
    m_iconLoader = new IconLoader();
    m_menuLoader = new MenuLoader();

    start();
}

void PanelApplication::start()
{
//	qDebug() << "PanelApplication::start()";
    m_panel.loadSettings();

    PanelWindow *panelWindow = new PanelWindow;
    panelWindow->resize(128, m_panel.panelHeight());
    panelWindow->setLayoutPolicy(Panel::FillSpace);
    panelWindow->setVerticalAnchor(m_panel.verticalAnchor);
    panelWindow->setDockMode(true);
    panelWindow->start();
    panelWindow->show();

    m_panelWindows.append(panelWindow);
}

void PanelApplication::stop()
{
    for (int i = 0; i < m_panelWindows.size(); i++) {
        m_panelWindows[i]->stop();
    }
}

void PanelApplication::clear()
{
//	qDebug() << "PanelApplication::clear()";
    for (int i = 0; i < m_panelWindows.size(); i++) {
        delete m_panelWindows[i];
    }
    m_panelWindows.clear();
}
//==================================================================================

void PanelApplication::onRestart()
{
//	stop();
//	clear();
//	start();

	m_panel.loadSettings();

//	int w = m_panel.panelWidth();
	int h = m_panel.panelHeight();

	for (int i = 0; i < m_panelWindows.size(); i++) {
		PanelWindow *p = m_panelWindows[i];
		p->resize(p->width(), h);
		p->updatePosition();
		p->updateLayout();
	}
}

void PanelApplication::showMenuEditor()
{
    QStringList args;
    QProcess::startDetached(Global::menuEditorFile(), args, getenv("HOME"));
}

void PanelApplication::showConfigurationDialog()
{
    if (m_settingsDlg == NULL) {
        m_settingsDlg = new PanelSettingsDlg(this);
    }
    m_settingsDlg->showDialog();
}
