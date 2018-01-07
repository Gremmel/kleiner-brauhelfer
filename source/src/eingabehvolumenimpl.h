#ifndef EINGABEHVOLUMENIMPL_H
#define EINGABEHVOLUMENIMPL_H
//
#include <QDialog>
#include "ui_EingabeHVolumen.h"
//
class EingabeHVolumenImpl : public QDialog, public Ui::EingabeHVolumen
{
Q_OBJECT
private:
	double Durchmesser;
	double Hoehe;
public:
	void setVisibleVonUnten(bool value);
	void setVisibleVonOben(bool value);
	void setLiter(double Liter);
    EingabeHVolumenImpl(double durchmesser, double hoehe, QWidget * parent = 0,  Qt::WindowFlags f = 0 );
private slots:
	void on_spinBox_VonOben_valueChanged(double );
	void on_spinBox_VonUnten_valueChanged(double );
	void on_spinBox_Temperatur_valueChanged(double );
	void on_spinBox_Liter_valueChanged(double );
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
private:
    void BerLiter20Grad();
};
#endif





