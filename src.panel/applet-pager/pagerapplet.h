#ifndef PAGERAPPLET_H
#define PAGERAPPLET_H

#include <QHash>
#include "applet.h"
//==================================================================================

class PagerApplet: public Applet
{
	Q_OBJECT
public:
	PagerApplet(PanelWindow *panelWindow);
	~PagerApplet();

	bool start();
	bool stop();
	QSize desiredSize();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
			   QWidget *widget);
	void showHelp();

private slots:
	void desktopCountChanged(int desktopCount);
	void currentDesktopChanged(int currentIndex);

protected:
	void wheelEvent(QGraphicsSceneWheelEvent *event);

private:
	void paintUi(QPainter *painter, int X, int Y, int W, int H,
				 bool active, bool hasTask);

private:
	friend class TaskApplet;
	friend class TaskClient;
	static PagerApplet *m_instance;
	QRect m_outRect;
	QStringList m_pagerNames;
	int m_pageCount;
	int m_pageIndex;
	int m_deskW, m_deskH, m_cols, m_rows;
};

#endif
