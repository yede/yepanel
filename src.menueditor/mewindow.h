#ifndef MCWINDOW_H
#define MCWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QFileDialog>
//==================================================================================

namespace Ui {
class MeWindow;
}

class QTreeWidgetItem;

class MeWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit MeWindow(QWidget *parent = 0);
	~MeWindow();

	void showItem(QTreeWidgetItem *item);
	void readEditor(QTreeWidgetItem *item);

	bool isModified() const { return m_modified; }

public slots:
	void onAddMenu();
	void onAddItem();
	void onAddSep();
	void onDelete();
	void onSave();
	void onApply();
	void onPickIcon();

	void onTextEdited(const QString &text);
	void onIconEdited(const QString &text);
	void onExecEdited(const QString &text);

protected:
	void closeEvent(QCloseEvent*);

private:
	void addNode(const QString &title, int type, const QString &exec,
				 const QString &icon, bool focus);
private:
	Ui::MeWindow *ui;
	QFileDialog  *m_dialog;
	bool m_modified;
};

#endif // MCWINDOW_H
