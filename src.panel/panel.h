#ifndef PANELCFG_H
#define PANELCFG_H

#include <QFont>
#include <QColor>
#include <QPixmap>
//==================================================================================

class PanelSettingsDlg;

struct Panel
{
public:
	enum Anchor
	{
		Min,
		Center,
		Max
	};

	enum Orientation
	{
		Horizontal,
		Vertical
	};

	enum LayoutPolicy
	{
		Normal,
		AutoSize,
		FillSpace
	};

	void defaultSettings();
	void loadSettings();
	void saveSettings(PanelSettingsDlg *dlg);

	int panelWidth()  const;
	int panelHeight() const;

public:
	//-------------------------------------------------------------------------------
	// panel
	QString fontName;
	int appletSpacing;

	QColor foregroundColor;
	QColor backgroundColor;
	QColor tooltipColor;
	//-------------------------------------------------------------------------------
	// menu
	int menuIconSize;
	QStringList menuSkipNames;
	//-------------------------------------------------------------------------------
	// taskbar
	QColor taskNormalColor;
	QColor taskActiveColor;
	QColor taskIconicColor;
	int taskIconSize;
	int taskIconOffset;
	int taskTextOffset;
	//-------------------------------------------------------------------------------
	// meter: for pager, network
	int meterMargin;
	int meterPadding;
	int meterSpacing;

	QColor meterColorBlack;
	QColor meterColorRed;
	QColor meterColorYellow;
	QColor meterColorGreen;
	QColor meterColorBlue;
	QColor meterColorPurple;
	QColor meterColorCyan;
	QColor meterColorGray;
	//-------------------------------------------------------------------------------
	// pager
	QColor pagerNormalColor;
	QColor pagerActiveColor;
	QColor pagerBackgroundColor;
	//-------------------------------------------------------------------------------
	// networ
	int networkUnit;	// unit width
	//-------------------------------------------------------------------------------
	// volume
	QColor volumeColor;
	int volumeUnit;
	//-------------------------------------------------------------------------------
	// battery
	QColor batteryColor;
	int batteryUnit;
	//-------------------------------------------------------------------------------
	// clock
	QColor clockTextColor;
	QColor clockShadowColor;
	QString clockFontName;
	//-------------------------------------------------------------------------------

public:
	QPixmap pxDefaultTaskIcon;
	QPixmap pxBase;
	QPixmap pxSeparator;
	QPixmap pxTaskActBase, pxTaskActLeft, pxTaskActRight;
	QPixmap pxTaskHovBase, pxTaskHovLeft, pxTaskHovRight;
	QPixmap pxTaskNorBase, pxTaskNorLeft, pxTaskNorRight;

	//-------------------------------------------------------------------------------
	QString userThemeDir;
	QString defaultThemeDir;

	QString defaultTheme;
	QString defaultIconTheme;

	QString currentTheme;
	QString currentIconTheme;
	QFont fontNormal;
	QFont fontBold;
	QFont clockFont;

	int    verticalPosition;
	Anchor verticalAnchor;

private:
	void loadTheme(const QString &path);
	void loadThemeRc(const QString &path);

	void initSettings();
};

#endif // PANELCFG_H
