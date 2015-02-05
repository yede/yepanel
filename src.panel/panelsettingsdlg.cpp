#include <QDirIterator>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QDebug>

#include "panelsettingsdlg.h"
#include "ui_panelsettingsdlg.h"
#include "panelapplication.h"
//==================================================================================

static void addIndexItem(QListWidget *view, const QString &title)
{
	QListWidgetItem *item = new QListWidgetItem(title);
	item->setSizeHint(QSize(10, 20));
	view->addItem(item);
}

PanelSettingsDlg::PanelSettingsDlg(PanelApplication *app, QWidget *parent)
	: QWidget(parent, Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint)
	, ui(new Ui::PanelSettingsDlg)
	, m_app(app)
	, m_cfg(app->m_panel)
{
	ui->setupUi(this);

	addIndexItem(ui->indexList, tr("Panel Settings"));
	addIndexItem(ui->indexList, tr("Select Theme"));
	ui->indexList->setCurrentRow(0);
	ui->stackedWidget->setCurrentIndex(0);

	connect(ui->indexList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onIndexClicked(QListWidgetItem*)));
	connect(ui->themeList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onThemeClicked(QListWidgetItem*)));
	connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(onEditSettings()));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(onClose()));
}

PanelSettingsDlg::~PanelSettingsDlg()
{
	delete ui;
}

void PanelSettingsDlg::onClose()
{
	this->hide();
}
//==================================================================================

QString PanelSettingsDlg::getPanelTheme()
{
	QListWidgetItem *item = ui->themeList->currentItem();
	if (item == NULL) return QString();

	QString theme = item->data(Qt::UserRole).toString() + "/" + item->text();
	return theme;
}

QString PanelSettingsDlg::getIconTheme() { return ui->edIconTheme->text().trimmed(); }
QString PanelSettingsDlg::getFontName()  { return ui->edFontName->text().trimmed();  }
QString PanelSettingsDlg::getSkipMenus() { return ui->edSkipMenus->text().trimmed(); }
//==================================================================================

static void listThemes(QListWidget *view, const QString &path, const QString &currTheme,
					   const QString &group, const QIcon &iconGroup, const QIcon &iconTheme)
{
	if (!QDir(path).exists()) return;

	QDirIterator dirIterator(path);
	QSize size = QSize(10, 18);
	QString flagTheme;
	bool flag = currTheme.startsWith(path);
	if (flag) flagTheme = currTheme.mid(currTheme.lastIndexOf("/") + 1);
	QListWidgetItem *item = NULL;

	while (dirIterator.hasNext()) {
		dirIterator.next();
		QString   fileName = dirIterator.fileName();
		QFileInfo fileInfo = dirIterator.fileInfo();
		if (fileInfo.isDir() && fileName != "." && fileName != "..") {
			if (QFile(fileInfo.filePath() + "/theme.rc").exists()) {
				if (item == NULL) {
					item = new QListWidgetItem(group);
					item->setIcon(iconGroup);
					item->setFlags(Qt::NoItemFlags);
					view->addItem(item);
				}
				item = new QListWidgetItem(fileName);
				item->setSizeHint(size);
				item->setIcon(iconTheme);
				item->setData(Qt::UserRole, path);
				view->addItem(item);
				if (flag && fileName == flagTheme) {
					flag = false;
					view->setCurrentItem(item);
				}
			}
		}
	}
}

void PanelSettingsDlg::showDialog()
{
	ui->edFontName->setText(m_cfg.fontName);
	ui->edIconTheme->setText(m_cfg.currentIconTheme);
	ui->edSkipMenus->setText(m_cfg.menuSkipNames.join(","));

	if (m_cfg.verticalAnchor == Panel::Max) ui->rbBottom->setChecked(true);
	else ui->rbTop->setChecked(true);

	QIcon iconApp = QIcon(":/16/lock");
	QIcon iconUser = QIcon(":/16/user");
	QIcon iconTheme = QIcon(":/16/theme");
	QString userTheme = tr("User Themes:");
	QString defTheme = tr("Default Themes:");
	ui->themeList->clear();
	listThemes(ui->themeList, m_cfg.defaultThemeDir, m_cfg.currentTheme, defTheme, iconApp, iconTheme);
	listThemes(ui->themeList, m_cfg.userThemeDir, m_cfg.currentTheme, userTheme, iconUser, iconTheme);

	if (isMinimized()) this->showNormal();
	else this->show();

	this->raise();
	this->setFocus();
}
//==================================================================================

void PanelSettingsDlg::onIndexClicked(QListWidgetItem *item)
{
	if (item == NULL) return;

	int i = ui->indexList->row(item);
	if (i < 0) return;

	ui->stackedWidget->setCurrentIndex(i);
}

void PanelSettingsDlg::onThemeClicked(QListWidgetItem *item)
{
	if (item == NULL) return;

	QString theme = item->text();
	if (theme.isEmpty()) return;

	onEditSettings();
}

void PanelSettingsDlg::onEditSettings()
{
	m_cfg.saveSettings(this);
	restartPanel();
}
//==================================================================================

void PanelSettingsDlg::restartPanel()
{
	QTimer::singleShot(80, this, SLOT(onRestartPanel()));
}

void PanelSettingsDlg::onRestartPanel()
{
	emit m_app->restart();
}
