#include "eingabehvolumenimpl.h"
#include <qmath.h>
#include "berechnungen.h"
//
EingabeHVolumenImpl::EingabeHVolumenImpl( QWidget * parent, Qt::WindowFlags f) 
	: QDialog(parent, f)
{
	setupUi(this);
	abgebrochen = true;
}
//


void EingabeHVolumenImpl::setHoehe(double value)
{
	Hoehe = value;
	spinBox_VonOben -> setMaximum(Hoehe);
	spinBox_VonUnten -> setMaximum(Hoehe);

	double Grundflaeche;
	Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
	spinBox_Liter -> setMaximum(Grundflaeche * (Hoehe / 100 ) / 10); 
}

void EingabeHVolumenImpl::setDurchmesser(double value)
{
	Durchmesser = value;
	double Grundflaeche;
	Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
	spinBox_Liter -> setMaximum(Grundflaeche * (Hoehe / 100 ) / 10); 
}

void EingabeHVolumenImpl::on_buttonBox_accepted()
{
	abgebrochen = false;
	accept();
	hide();
}

void EingabeHVolumenImpl::on_buttonBox_rejected()
{
	abgebrochen = true;
	rejected();
	hide();
}

void EingabeHVolumenImpl::on_spinBox_VonOben_valueChanged(double )
{
	if (spinBox_VonOben -> hasFocus()){
		double Grundflaeche;
		
		Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
		spinBox_Liter -> setValue(Grundflaeche * (Hoehe - spinBox_VonOben -> value()) / 1000 ); 
		spinBox_VonUnten -> setValue(Hoehe - spinBox_VonOben -> value());
		BerLiter20Grad();
	}
}

void EingabeHVolumenImpl::on_spinBox_VonUnten_valueChanged(double )
{
	if (spinBox_VonUnten -> hasFocus()){
		double Grundflaeche;
		
		Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
		spinBox_Liter -> setValue(Grundflaeche * (spinBox_VonUnten -> value()) / 1000 ); 
		spinBox_VonOben -> setValue(Hoehe - spinBox_VonUnten -> value());
		BerLiter20Grad();
	}
}

void EingabeHVolumenImpl::on_spinBox_Temperatur_valueChanged(double )
{
	if (spinBox_Temperatur -> hasFocus()){
		BerLiter20Grad();
	}
}

void EingabeHVolumenImpl::on_spinBox_Liter_valueChanged(double )
{
	if (spinBox_Liter -> hasFocus()){
		double Grundflaeche;
		
		Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
		spinBox_VonUnten -> setValue(spinBox_Liter -> value() * 1000 / Grundflaeche);
		spinBox_VonOben -> setValue(Hoehe - spinBox_VonUnten -> value());
		BerLiter20Grad();
	}
}

void EingabeHVolumenImpl::BerLiter20Grad()
{
	// Volumen bei 20Grad berechnen
	QBerechnungen ber;
	spinBox_Liter20Grad -> setValue(ber.BerVolumenWasser(spinBox_Temperatur -> value(), 20, 
			spinBox_Liter -> value()));
}


void EingabeHVolumenImpl::setLiter(double Liter)
{
	spinBox_Liter20Grad -> setValue(Liter);
	QBerechnungen ber;
	spinBox_Liter -> setValue(ber.BerVolumenWasser(20, spinBox_Temperatur -> value(), Liter));
	double Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
	spinBox_VonUnten -> setValue(spinBox_Liter -> value() * 1000 / Grundflaeche);
	spinBox_VonOben -> setValue(Hoehe - spinBox_VonUnten -> value());
}


void EingabeHVolumenImpl::setVisibleVonOben(bool value)
{
	widget_VonOben -> setVisible(value);
}


void EingabeHVolumenImpl::setVisibleVonUnten(bool value)
{
	widget_VonUnten -> setVisible(value);
}

