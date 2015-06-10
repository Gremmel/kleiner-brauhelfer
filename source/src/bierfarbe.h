#ifndef BIERFARBE_H
#define BIERFARBE_H
//
#include <QGraphicsView>
//
class BierFarbe : public QGraphicsView
{
Q_OBJECT
public:
	bool BildSpeichern(QString Name);
	void resizeEvent(QResizeEvent * event);
	BierFarbe(QWidget * parent);
	
};
#endif
