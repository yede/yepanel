#include <QApplication>
#include <QIcon>
#include <QSettings>
#include <QDebug>

#include "panel.h"
#include "panelsettingsdlg.h"
#include "global.h"
#include "util.h"
//==================================================================================

void Panel::defaultSettings()
{
	// panel
	fontName = "default";
	appletSpacing = 7;

	foregroundColor = Util::color(0xa0a0a0);
	backgroundColor = Util::color(0x202020);
	tooltipColor    = Util::color(0xfafa60);
	//-------------------------------------------------------------------------------
	// menu
	menuIconSize = 16;
	//menuSkipNames = ;
	//-------------------------------------------------------------------------------
	// taskbar
	taskActiveColor = Util::color(0xffa0a0);
	taskNormalColor = Util::color(0xa0a0a0);
	taskIconicColor = Util::color(0x8080c0);
	taskIconSize = 16;
	taskIconOffset = 0;
	taskTextOffset = 0;
	//-------------------------------------------------------------------------------
	// meter: for pager, network
	meterMargin  = 3;
	meterPadding = 2;
	meterSpacing = 2;

	meterColorBlack  = Util::color(0x000000);
	meterColorRed    = Util::color(0xe00000);
	meterColorYellow = Util::color(0xe0e000);
	meterColorGreen  = Util::color(0x00e000);
	meterColorBlue   = Util::color(0x6060e0);
	meterColorPurple = Util::color(0xe000e0);
	meterColorCyan   = Util::color(0x00a0e0);
	meterColorGray   = Util::color(0x909090);
	//-------------------------------------------------------------------------------
	// pager
	pagerNormalColor = Util::color(0x909090);
	pagerActiveColor = Util::color(0x00a0f0);
	pagerBackgroundColor = Util::color(0x000000);
	//-------------------------------------------------------------------------------
	// networ
	networkUnit = 4;
	//-------------------------------------------------------------------------------
	// volume
	volumeColor = Util::color(0xa0c080);
	volumeUnit  = 5;
	//-------------------------------------------------------------------------------
	// battery
	batteryColor = Util::color(0x00a0f0);
	batteryUnit  = 6;
	//-------------------------------------------------------------------------------
	// clock
	clockTextColor   = Util::color(0xa0a0a0);
	clockShadowColor = Util::color(0x202020);
	clockFontName = "default";
	//-------------------------------------------------------------------------------

	userThemeDir = Global::userDataDir() + "/themes";
	defaultThemeDir = Global::dataDir() + "/themes";

	defaultTheme = defaultThemeDir + "/native";
	defaultIconTheme = QIcon::themeName();

	currentTheme = defaultTheme;
	verticalPosition = 1;
}

void Panel::loadSettings()
{
	QSettings settings(Global::cfgFile(), QSettings::IniFormat);

	settings.beginGroup("Common");		// Can't use "General" as group name???
	currentTheme     = settings.value("panelTheme").toString();
	currentIconTheme = settings.value("iconTheme", "default").toString();
	fontName         = settings.value("fontName",  "default").toString();
	verticalPosition = settings.value("verticalPosition", verticalPosition).toInt();
	QString names = settings.value("menuSkipNames").toString();
	settings.endGroup();

	menuSkipNames = names.split(QChar(','));

	loadTheme(currentTheme.isEmpty() ? defaultTheme : currentTheme);
	initSettings();
}

void Panel::saveSettings(PanelSettingsDlg *dlg)
{
	QString panelTheme = dlg->getPanelTheme();
	QString iconTheme = dlg->getIconTheme();
	QString fontName = dlg->getFontName();
	QString menuSkipNames = dlg->getSkipMenus();

	QSettings settings(Global::cfgFile(), QSettings::IniFormat);

	settings.beginGroup("Common");
	settings.setValue("panelTheme", panelTheme);
	settings.setValue("iconTheme", iconTheme);
	settings.setValue("fontName", fontName);
	settings.setValue("menuSkipNames", menuSkipNames);
	settings.endGroup();
}
//==================================================================================
#define THEME_RC_NAME "theme.rc"

static void getSettingColor(QColor &result, QSettings &s, const QString &colorKey)
{
	QString colorStr = s.value(colorKey).toString();
	if (!colorStr.isEmpty())
		Util::decodeColor(result, colorStr);
}

void Panel::loadThemeRc(const QString &path)
{
	QString rcFile = path + "/" + THEME_RC_NAME;
	QSettings settings(rcFile, QSettings::IniFormat);
	//-------------------------------------------------------------------------------

	settings.beginGroup("panel");
	getSettingColor(foregroundColor, settings, "foregroundColor");
	getSettingColor(backgroundColor, settings, "backgroundColor");
	getSettingColor(tooltipColor,    settings, "tooltipColor");
	appletSpacing = settings.value("appletSpacing", appletSpacing).toInt();
	settings.endGroup();

	if (appletSpacing < 4) appletSpacing = 4;	// when < 4: cause next applet repaint
	//-------------------------------------------------------------------------------

	settings.beginGroup("taskbar");
	getSettingColor(taskActiveColor, settings, "taskActiveColor");
	getSettingColor(taskNormalColor, settings, "taskNormalColor");
	getSettingColor(taskIconicColor, settings, "taskIconicColor");
	taskIconSize = settings.value("taskIconSize", 16).toInt();
	taskIconOffset = settings.value("taskIconOffset", 0).toInt();
	taskTextOffset = settings.value("taskTextOffset", 0).toInt();
	settings.endGroup();
	//-------------------------------------------------------------------------------

	settings.beginGroup("volume");
	getSettingColor(volumeColor, settings, "volumeColor");
	volumeUnit = settings.value("volumeUnit", 5).toInt();
	settings.endGroup();
	//-------------------------------------------------------------------------------

	settings.beginGroup("clock");
	getSettingColor(clockTextColor, settings, "clockTextColor");
	getSettingColor(clockShadowColor, settings, "clockShadowColor");
	clockFontName = settings.value("clockFontName", "default").toString();
	settings.endGroup();
}

void Panel::loadTheme(const QString &path)
{
	pxDefaultTaskIcon = QPixmap(path + "/default_icon.png");
	pxBase = QPixmap(path + "/base.png");
//	pxSeparator = QPixmap(path + "/separator.png");
//	qDebug() << pxBase.size() << path;

	pxTaskActBase  = QPixmap(path + "/task_button_active_base.png");
	pxTaskActLeft  = QPixmap(path + "/task_button_active_left.png");
	pxTaskActRight = QPixmap(path + "/task_button_active_right.png");

	pxTaskHovBase  = QPixmap(path + "/task_button_hover_base.png");
	pxTaskHovLeft  = QPixmap(path + "/task_button_hover_left.png");
	pxTaskHovRight = QPixmap(path + "/task_button_hover_right.png");

//	pxTaskNorBase  = QPixmap(path + "/");
//	pxTaskNorLeft  = QPixmap(path + "/");
//	pxTaskNorRight = QPixmap(path + "/");

	loadThemeRc(path);
}

int Panel::panelWidth() const { return 1024; }
int Panel::panelHeight() const { int h = pxBase.height(); return h < 20 ? 25 : h; }
//==================================================================================

static QFont createFont(const QString fontStr)
{
	if (fontStr != "default") {									// Parse font name. Example: "Droid Sans 11".
		int lastSpacePos = fontStr.lastIndexOf(' ');
		if (lastSpacePos >= 0) {								// Should have at least one space,
			int fontSize = fontStr.mid(lastSpacePos).toInt();	// otherwise string is malformed,
			QString fontFamily = fontStr;						// keep default in that case.
			fontFamily.truncate(lastSpacePos);
			return QFont(fontFamily, fontSize);
		}
	}
	return QApplication::font();
}

void Panel::initSettings()
{
	switch (verticalPosition) {
		case 0 : verticalAnchor = Panel::Min; break;
		case 1 :
		default: verticalAnchor = Panel::Max;
	}

	clockFont  = createFont(clockFontName);
	fontNormal = createFont(fontName);
	fontBold = fontNormal;
	fontBold.setBold(true);

	QIcon::setThemeName(currentIconTheme == "default" ? defaultIconTheme : currentIconTheme);
}
//==================================================================================
