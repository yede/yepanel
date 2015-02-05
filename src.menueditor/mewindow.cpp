#include <QUrl>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QDebug>
#include "metree.h"
#include "meutil.h"
#include "mewindow.h"
#include "ui_mewindow.h"
#include "global.h"
//==================================================================================

MeWindow::MeWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MeWindow)
	, m_modified(false)
{
	ui->setupUi(this);
	ui->treeWidget->setWindow(this);

	QList<QUrl> urls;
	urls << QUrl::fromLocalFile(QString::fromUtf8(getenv("HOME")))
		 << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation))
		 << QUrl::fromLocalFile("/usr/share/pixmaps")
		 << QUrl::fromLocalFile("/usr/share/icons");

	m_dialog = new QFileDialog(this, tr("Choose icon file"),
							   QString::fromUtf8(getenv("HOME")),
							   QString("%1;;%2")
							   .arg(tr("Image files (*.png *.xpm *.jpg)"))
							   .arg(tr("All files (*.*)")));
	m_dialog->setSidebarUrls(urls);
	m_dialog->setFileMode(QFileDialog::ExistingFile);

	connect(ui->actionNewMenu, SIGNAL(triggered()), this, SLOT(onAddMenu()));
	connect(ui->actionNewItem, SIGNAL(triggered()), this, SLOT(onAddItem()));
	connect(ui->actionNewSep,  SIGNAL(triggered()), this, SLOT(onAddSep()));
	connect(ui->actionDel,  SIGNAL(triggered()), this, SLOT(onDelete()));
	connect(ui->actionApp,  SIGNAL(triggered()), this, SLOT(onApply()));
	connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(onSave()));

	connect(ui->tbPickIcon, SIGNAL(clicked()), this, SLOT(onPickIcon()));

	connect(ui->edName, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited(QString)));
	connect(ui->edIcon, SIGNAL(textEdited(QString)), this, SLOT(onIconEdited(QString)));
	connect(ui->edExec, SIGNAL(textEdited(QString)), this, SLOT(onExecEdited(QString)));

	ui->treeWidget->loadFile();
}

MeWindow::~MeWindow()
{
	delete ui;
}

void MeWindow::closeEvent(QCloseEvent *event)
{
	if (m_modified) {
		//
	}
	event->accept();
}
//==================================================================================

void MeWindow::showItem(QTreeWidgetItem *item)
{
	ui->edName->setText(MeUtil::getItemText(item));
	ui->edExec->setText(MeUtil::getItemExec(item));
	ui->edIcon->setText(MeUtil::getItemIcon(item));

	int type = MeUtil::getItemType(item);
	ui->edName->setEnabled(item != 0 && type != NodeType::SepItem);
	ui->edExec->setEnabled(item != 0 && type == NodeType::AppItem);
	ui->edIcon->setEnabled(item != 0 && type != NodeType::SepItem);
	ui->tbPickIcon->setEnabled(ui->edIcon->isEnabled());

	if (item && MeUtil::getItemType(item) == NodeType::SepItem)
		ui->edName->setText(tr("<Separator>"));
}

void MeWindow::readEditor(QTreeWidgetItem *item)
{
	if (!item) return;
	if (MeUtil::getItemType(item) == NodeType::SepItem) return;

	MeUtil::setItemText(item, ui->edName->text());
	MeUtil::setItemExec(item, ui->edExec->text());
	MeUtil::setItemIcon(item, ui->edIcon->text());
}
//==================================================================================

void MeWindow::onTextEdited(const QString &text)
{
	QTreeWidgetItem *item = ui->treeWidget->currentItem();
	if (!item) return;

	switch (MeUtil::getItemType(item)) {
		case NodeType::AppMenu:
		case NodeType::AppItem: MeUtil::setItemText(item, text); m_modified = true; break;
		case NodeType::SepItem: break;
	}
}

void MeWindow::onIconEdited(const QString &text)
{
	QTreeWidgetItem *item = ui->treeWidget->currentItem();
	if (!item) return;

	switch (MeUtil::getItemType(item)) {
		case NodeType::AppMenu:
		case NodeType::AppItem: MeUtil::setItemIcon(item, text); m_modified = true; break;
		case NodeType::SepItem: break;
	}
}

void MeWindow::onExecEdited(const QString &text)
{
	QTreeWidgetItem *item = ui->treeWidget->currentItem();
	if (!item) return;

	switch (MeUtil::getItemType(item)) {
		case NodeType::AppMenu: break;
		case NodeType::AppItem: MeUtil::setItemExec(item, text); m_modified = true; break;
		case NodeType::SepItem: break;
	}
}

void MeWindow::onPickIcon()
{
	if (m_dialog->exec() != QDialog::Accepted) return;

	QStringList files = m_dialog->selectedFiles();
	if (files.count() > 0) {
		QString icon = files.at(0);
		ui->edIcon->setText(icon);
		onIconEdited(icon);
	}
}
//==================================================================================

void MeWindow::addNode(const QString &title, int type, const QString &exec,
					   const QString &icon, bool focus)
{
	int pos;
	QTreeWidgetItem *menu = MeUtil::getInsertPos(ui->treeWidget, pos);
	ui->treeWidget->newItem(title, type, icon, exec, menu, pos, focus);
}

void MeWindow::onAddMenu()
{
	addNode(tr("New Menu"), NodeType::AppMenu, "", "", true);
}

void MeWindow::onAddItem()
{
	addNode(tr("New Exec"), NodeType::AppItem, "", "", true);
}

void MeWindow::onAddSep()
{
	addNode("", NodeType::SepItem, "", "", false);
}

void MeWindow::onDelete()
{
	QTreeWidgetItem *item = ui->treeWidget->currentItem();
	if (item) delete item;
}

void MeWindow::onSave()
{
	ui->treeWidget->saveFile();
	m_modified = false;
}

void MeWindow::onApply()
{
	QTreeWidgetItem *item = ui->treeWidget->currentItem();
	if (!item) return;

//	m_modified = true;
}
//==================================================================================
