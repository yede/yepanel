#include <QApplication>
#include <QLocale>
#include <QDir>
#include <QDebug>
#include "global.h"
//==================================================================================

QString Global::m_appName;
QString Global::m_userDir;
QString Global::m_tempDir;
QString Global::m_dataDir;
QString Global::m_langDir;
QString Global::m_iconDir;
QString Global::m_cacheDir;
QString Global::m_userDataDir;
QString Global::m_cfgDir;
QString Global::m_cfgFile;
QString Global::m_menuFile;
QString Global::m_menuEditorFile;
QString Global::m_menuEditorName;

QTranslator Global::m_translator;
//==================================================================================

#define ORG_NAME	"CentSoft"
#define ORG_SITE	"centsoft.com"
#define USR_MENU	"custom-menus.json"

#define S_DATA_DIR   QUOTE_MACRO(D_DATA_DIR)
#define S_EXEC_DIR   QUOTE_MACRO(D_EXEC_DIR)
#define S_MAIN_APP   QUOTE_MACRO(D_MAIN_APP)
#define S_MENU_EDT   QUOTE_MACRO(D_MENU_EDT)
//==================================================================================

void Global::initPath()
{
	m_appName  = S_MAIN_APP;

	QCoreApplication::setOrganizationName(ORG_NAME);
	QCoreApplication::setOrganizationDomain(ORG_SITE);
	QCoreApplication::setApplicationName(m_appName);

	m_userDir     = QDir::homePath();
	m_tempDir     = QDir::tempPath();
	m_dataDir     = S_DATA_DIR;
	m_langDir     = S_DATA_DIR "/translations";
	m_iconDir     = S_DATA_DIR "/images";
	m_cacheDir    = QDir::homePath() + "/.cache/" + m_appName;
	m_userDataDir = QDir::homePath() + "/.local/share/" + m_appName;
	m_cfgDir      = QDir::homePath() + "/.config/" + m_appName;
	m_cfgFile     = m_cfgDir + QDir::separator() + m_appName + ".conf";
	m_menuFile    = m_cfgDir + QDir::separator() + USR_MENU;
	m_menuEditorFile = S_EXEC_DIR "/" S_MENU_EDT;
	m_menuEditorName = S_MENU_EDT;

//	qDebug() << "m_menuEditorFile" << m_menuEditorFile;
//	qDebug() << "m_menuEditorName" << m_menuEditorName;
//	qDebug() << "m_appName" << m_appName;
//	qDebug() << "m_dataDir" << m_dataDir;

	QDir dir(m_cfgDir);
	if (!dir.exists()) dir.mkpath(m_cfgDir);
	if (!dir.exists(m_cacheDir)) dir.mkpath(m_cacheDir);
}
//==================================================================================

static QString getSystemLanguage()
{
    QLocale locale = QLocale::system();
    QString lang = locale.name();

    if (lang.toLower() == "c") {
        lang = getenv("LANG");
        int pos = lang.indexOf('.');
        if (pos > 0) lang = lang.left(pos);
    }

    return lang;
}

void Global::initTranslator(const QString &appName)
{
    QString lang = getSystemLanguage();
//    qDebug() << "getSystemLanguage()" << lang;
    if (lang.isEmpty()) return;

    lang = m_langDir + QDir::separator() + appName + '_' + lang + ".qm";
//    qDebug() << "lang-file" << lang;
    if (!QFile(lang).exists()) return;

    m_translator.load(lang);
    qApp->installTranslator(&m_translator);
}
//==================================================================================
