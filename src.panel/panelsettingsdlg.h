#ifndef PANELSETTINGSDLG_H
#define PANELSETTINGSDLG_H

#include <QWidget>
#include <QListWidgetItem>
//==================================================================================

namespace Ui {
class PanelSettingsDlg;
}

class PanelApplication;
class Panel;

class PanelSettingsDlg : public QWidget
{
	Q_OBJECT

public:
	explicit PanelSettingsDlg(PanelApplication *app, QWidget *parent = 0);
	~PanelSettingsDlg();

	void showDialog();

	QString getPanelTheme();
	QString getIconTheme();
	QString getFontName();
	QString getSkipMenus();

private:
	void restartPanel();

public slots:
	void onClose();
	void onRestartPanel();
	void onIndexClicked(QListWidgetItem *item);
	void onThemeClicked(QListWidgetItem *item);
	void onEditSettings();

private:
	Ui::PanelSettingsDlg *ui;
	PanelApplication     *m_app;
	Panel                &m_cfg;
};

#endif // PANELSETTINGSDLG_H
