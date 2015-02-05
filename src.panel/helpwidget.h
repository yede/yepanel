#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QList>
//==================================================================================

namespace HelpAlign {
	enum { Left, Center, Right };
}

struct HelpString
{
	HelpString(int x, int y, int width, Qt::Alignment align, const QString &text);
	HelpString(int x, int y, const QString &text);
	HelpString();

	void setString(int x, int y, int width, Qt::Alignment align, const QString &text);
	void setString(int x, int y, const QString &text);
	void setString(const QString &text);

	int x, y;
	int width;
	Qt::Alignment alignment;
	QString text;
	int tag;
};

struct HelpInfo
{
	HelpInfo(int width, int height);
	HelpInfo(int width);
	HelpInfo();

	void addString(const HelpString *hs);
	void clear();

	int width;
	int height;
	QList<const HelpString *> strings;
};

struct HelpPosition
{
	int orientation;
	int limitLeft, limitTop, limitRight, limitBottom;
	int x, y;
};
//==================================================================================

class PanelWindow;

class HelpWidget : public QWidget
{
	Q_OBJECT
public:
	explicit HelpWidget();
	~HelpWidget();

	QSize sizeHint() const { return m_size; }

	static HelpWidget *instance() { return m_instance; }
	static void createInstance();
	static void destroyInstance();

	static void closeHelp();
	static void calcPosition(const QPointF &pos, PanelWindow *panelWindow);
	static void showHelp(const QString &text);
	static void showHelp(const QStringList &strs);
	static void showHelp(const HelpInfo &info);

	void showHelp(const HelpInfo &info, const HelpPosition &pos);
	void showHelp(const QStringList &strs, const HelpPosition &pos);
	void showHelp(const QString &text, const HelpPosition &pos);

private:
	void calcSize(const HelpInfo &info);
	void calcSize(const QStringList &strs);
	void calcSize(const QString &text);
	void showText(const HelpPosition &pos);

protected:
	void paintEvent(QPaintEvent *event);

private:
	static HelpWidget  *m_instance;
	static HelpPosition m_position;
	QSize    m_size;
	HelpInfo m_info;
	QStringList m_strs;
	QString m_text;
	int m_helpType;
	int m_padX, m_padY, m_lineHeight;
	int m_desktopWidth, m_desktopHeight;
};

#endif
