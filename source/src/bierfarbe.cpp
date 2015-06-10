#include "bierfarbe.h"
//
BierFarbe::BierFarbe( QWidget *  )
	: QGraphicsView()
{
	// TODO
}
//

void BierFarbe::resizeEvent(QResizeEvent * )
{
	fitInView(scene() -> itemsBoundingRect(), Qt::KeepAspectRatio);
}


bool BierFarbe::BildSpeichern(QString )
{
	fitInView(scene() -> itemsBoundingRect(), Qt::KeepAspectRatio);
	QPainter *painter = new QPainter();
	QImage *image = new QImage(QSize(width() - 1, height() - 1), QImage::Format_ARGB32);
	
	painter->begin(image);
	this->render(painter);
	painter->end();
	image->save("test.png","PNG"); 
	delete painter;
	delete image; 
	return (true);
}
