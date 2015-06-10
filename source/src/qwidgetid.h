#ifndef QWIDGETID_H
#define QWIDGETID_H
//
#include <QWidget>
//
class QWidgetID : public QWidget
{
Q_OBJECT
private:
	double AlphaProzent;
	double Farbe;
	int Pellets;
	int ID;
public:
	void resetWerte();
	void setAlphaProzent(double value);
	double getAlphaProzent();
	void setFarbe(double value);
	double getFarbe();
	void setPellets(int value);
	int getPellets();
	void setID(int value) { ID = value; }
	int getID() { return ID; }
	QWidgetID(QWidget * parent = 0,  Qt::WindowFlags f = 0);
	
};
#endif
