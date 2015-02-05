#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QTranslator>
//==================================================================================

#define QUOTE(x)		#x
#define QUOTE_MACRO(x)	QUOTE(x)

// for menu item:
namespace NodeType {
    enum { AppMenu, AppItem, SepItem };
}
#define MENU_ITEMS_KEY "menuitems"
//==================================================================================

class Global
{
public:
    static void initPath();
    static void initTranslator(const QString &appName);

public:
	static const QString &appName()     { return m_appName; }
	static const QString &userDir()     { return m_userDir; }
	static const QString &tempDir()     { return m_tempDir; }
	static const QString &dataDir()     { return m_dataDir; }
	static const QString &langDir()     { return m_langDir; }
	static const QString &iconDir()     { return m_iconDir; }
	static const QString &cacheDir()    { return m_cacheDir; }
	static const QString &userDataDir() { return m_userDataDir; }
	static const QString &cfgDir()      { return m_cfgDir;  }
	static const QString &cfgFile()     { return m_cfgFile; }

	static const QString &menuFile()       { return m_menuFile; }
    static const QString &menuEditorFile() { return m_menuEditorFile; }
    static const QString &menuEditorName() { return m_menuEditorName; }

private:
    static QString m_appName;
    static QString m_userDir;
    static QString m_tempDir;
    static QString m_dataDir;
    static QString m_langDir;
    static QString m_iconDir;
    static QString m_cacheDir;
	static QString m_userDataDir;
	static QString m_cfgDir;
    static QString m_cfgFile;
    static QString m_menuFile;
    static QString m_menuEditorFile;
    static QString m_menuEditorName;

    static QTranslator m_translator;
};

//==================================================================================
#endif
