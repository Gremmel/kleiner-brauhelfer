#include "starview.h"
#include <QGraphicsItemAnimation>
#include <QTimeLine>
#include <QMouseEvent>

StarView::StarView(QWidget *parent) :
	QGraphicsView(parent)
{
	myscene = new QGraphicsScene;
	myscene -> clear();
	list_Sterne.clear();

	setScene(myscene);
    setFrameShape(QFrame::NoFrame);
    setStyleSheet("background: transparent");
}

void StarView::setStyleDunkel(bool value)
{
	styleDunkel = value;
}

void StarView::addStar()
{
	if (AnzahlSterne < MaxSterne){
		QGraphicsItem *m_svgItem;
		m_svgItem = new QGraphicsSvgItem(":/global/star.svg");
		list_Sterne.append(m_svgItem);
		myscene -> addItem(m_svgItem);
		AnzahlSterne = list_Sterne.count();
		emit sig_AnzahlStarChanged(AnzahlSterne);
		update();
	}
}

void StarView::remStar()
{
	if (!list_Sterne.isEmpty()){
		myscene->removeItem(list_Sterne.last());
		list_Sterne.removeLast();
		AnzahlSterne = list_Sterne.count();
		emit sig_AnzahlStarChanged(AnzahlSterne);
		update();
	}
}

void StarView::closeEvent(QCloseEvent *)
{
	scene()->clear();;
	list_Sterne.clear();
}

void StarView::setMaxStar(int value)
{
	while (!list_SterneGr.isEmpty()){
		myscene->removeItem(list_SterneGr.last());
		list_SterneGr.removeLast();
	}
	MaxSterne = value;
	list_SterneGr.clear();
	for (int i=0; i<MaxSterne; i++){
		QGraphicsItem *m_svgItem;
		if (styleDunkel)
			m_svgItem = new QGraphicsSvgItem(":/global/star_gr_dark.svg");
		else
			m_svgItem = new QGraphicsSvgItem(":/global/star_gr.svg");
		list_SterneGr.append(m_svgItem);
		myscene -> addItem(m_svgItem);
	}
	update();
	if (AnzahlSterne > MaxSterne){
		setAnzahlStar(MaxSterne);
	}
}

int StarView::getMaxStar()
{
	return MaxSterne;
}

void StarView::setAnzahlStar(int value)
{
	if (value > AnzahlSterne){
		for (int i=AnzahlSterne; i < value; i++){
			addStar();
		}
	}
	else {
		for (int i=AnzahlSterne; i > value; i--){
			remStar();
		}
	}
}

void StarView::init(bool cStyleDunkel)
{
	AnzahlSterne = 0;
	MaxSterne = 5;
	styleDunkel = cStyleDunkel;
	
}

void StarView::resizeEvent(QResizeEvent *)
{
	fitInView(scene() -> itemsBoundingRect(), Qt::KeepAspectRatio);
}

void StarView::update()
{
//	myscene->clear();
	int breite = 0;
	for (int i = 0; i < AnzahlSterne ;i++){
		//myscene -> addItem(list_Sterne[i]);
		breite = i * 48;

		list_Sterne[i]->setPos(breite,0);
	}
	for (int i = 0; i < MaxSterne ;i++){
		//myscene -> addItem(list_Sterne[i]);
		breite = i * 48;

		list_SterneGr[i]->setPos(breite,0);
		if (i < AnzahlSterne){
			list_SterneGr[i]->setVisible(false);
		}
		else {
			list_SterneGr[i]->setVisible(true);
		}
	}
	QRectF rec;
	rec = myscene->sceneRect();
	rec.setRight(48*MaxSterne);
	myscene->setSceneRect(rec);
	resizeEvent(0);
}

void StarView::mousePressEvent(QMouseEvent *event)
{
	QGraphicsView::mousePressEvent(event);
	if (myscene->mouseGrabberItem()){
		myscene->mouseGrabberItem()->setToolTip("Hallo");
	}
}

