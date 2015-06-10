#include "qwidgetid.h"
//
QWidgetID::QWidgetID(QWidget * parent,  Qt::WindowFlags f) 
	: QWidget(parent, f)
{
	// TODO
}
//

int QWidgetID::getPellets()
{
	return Pellets;
}

void QWidgetID::setPellets(int value)
{
	if (value == Qt::Checked)
		value = 1;
	else if (value == -1)
		value = 0;
	Pellets = value;
}

double QWidgetID::getFarbe()
{
	return Farbe;
}

void QWidgetID::setFarbe(double value)
{
	Farbe = value;
}

double QWidgetID::getAlphaProzent()
{
	return AlphaProzent;
}

void QWidgetID::setAlphaProzent(double value)
{
	AlphaProzent = value;
}

void QWidgetID::resetWerte()
{
	AlphaProzent = 0;
	Farbe = 0;
	Pellets = 0;
}

