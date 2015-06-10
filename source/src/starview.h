#ifndef STARVIEW_H
#define STARVIEW_H

#include <QGraphicsView>
#include <QGraphicsSvgItem>

class StarView : public QGraphicsView
{
	Q_OBJECT
private:
	QGraphicsScene *myscene;
	void resizeEvent(QResizeEvent * event);
	//Sterne
	QList<QGraphicsItem *> list_Sterne;
	QList<QGraphicsItem *> list_SterneGr;
	bool styleDunkel;
	void update();
	int AnzahlSterne;
	int MaxSterne;
	void mousePressEvent ( QMouseEvent * event );
public:
	explicit StarView(QWidget *parent = 0);
	void setStyleDunkel(bool value);
	void addStar();
	void remStar();
	void closeEvent(QCloseEvent *event);
	void setMaxStar(int value);
	int getMaxStar();
	void setAnzahlStar(int value);
	void init(bool cStyleDunkel);
signals:
	void sig_AnzahlStarChanged(int value);
public slots:
	
};

#endif // STARVIEW_H
