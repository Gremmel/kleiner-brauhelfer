#include "dialogberverdampfung.h"
#include "ui_dialogberverdampfung.h"
#include <qmath.h>

DialogBerVerdampfung::DialogBerVerdampfung(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogBerVerdampfung)
{
	ui->setupUi(this);
  abgebrochen = true;
}

DialogBerVerdampfung::~DialogBerVerdampfung()
{
	delete ui;
	abgebrochen = true;
}

void DialogBerVerdampfung::on_buttonBox_accepted()
{
	abgebrochen = false;
	accept();
	hide();
}

void DialogBerVerdampfung::on_buttonBox_rejected()
{
	abgebrochen = true;
	rejected();
	hide();
}

void DialogBerVerdampfung::on_dSpinBox_Menge1_valueChanged(double )
{
  if (ui->dSpinBox_Menge1->hasFocus()) {
    double Grundflaeche;

    Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
    ui->dSpinBox_cmVomBodenMenge1 -> setValue(ui->dSpinBox_Menge1 -> value() * 1000 / Grundflaeche);
    ui->dSpinBox_cmVonObenMenge1 -> setValue(Hoehe - ui->dSpinBox_cmVomBodenMenge1 -> value());
  }
  berechne();
}

void DialogBerVerdampfung::on_dSpinBox_Menge2_valueChanged(double )
{
  if (ui->dSpinBox_Menge2->hasFocus()) {
    double Grundflaeche;

    Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
    ui->dSpinBox_cmVomBodenMenge2 -> setValue(ui->dSpinBox_Menge2 -> value() * 1000 / Grundflaeche);
    ui->dSpinBox_cmVonObenMenge2 -> setValue(Hoehe - ui->dSpinBox_cmVomBodenMenge2 -> value());
  }
  berechne();
}

void DialogBerVerdampfung::on_spinBox_Kochdauer_valueChanged(int )
{
  berechne();
}

void DialogBerVerdampfung::on_dSpinBox_cmVonObenMenge1_valueChanged(double )
{
  if (ui->dSpinBox_cmVonObenMenge1->hasFocus()){
    double Grundflaeche;

    Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
    ui->dSpinBox_Menge1 -> setValue(Grundflaeche * (Hoehe - ui->dSpinBox_cmVonObenMenge1 -> value()) / 1000 );
    ui->dSpinBox_cmVomBodenMenge1 -> setValue(Hoehe - ui->dSpinBox_cmVonObenMenge1 -> value());
  }
  berechne();
}

void DialogBerVerdampfung::on_dSpinBox_cmVomBodenMenge1_valueChanged(double )
{
  if (ui->dSpinBox_cmVomBodenMenge1->hasFocus()) {
    double Grundflaeche;

    Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
    ui->dSpinBox_Menge1 -> setValue(Grundflaeche * (ui->dSpinBox_cmVomBodenMenge1 -> value()) / 1000 );
    ui->dSpinBox_cmVonObenMenge1 -> setValue(Hoehe - ui->dSpinBox_cmVomBodenMenge1 -> value());
  }
  berechne();
}

void DialogBerVerdampfung::on_dSpinBox_cmVonObenMenge2_valueChanged(double )
{
  if (ui->dSpinBox_cmVonObenMenge2->hasFocus()){
    double Grundflaeche;

    Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
    ui->dSpinBox_Menge2 -> setValue(Grundflaeche * (Hoehe - ui->dSpinBox_cmVonObenMenge2 -> value()) / 1000 );
    ui->dSpinBox_cmVomBodenMenge2 -> setValue(Hoehe - ui->dSpinBox_cmVonObenMenge2 -> value());
  }
  berechne();
}

void DialogBerVerdampfung::on_dSpinBox_cmVomBodenMenge2_valueChanged(double )
{
  if (ui->dSpinBox_cmVomBodenMenge2->hasFocus()) {
    double Grundflaeche;

    Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
    ui->dSpinBox_Menge2 -> setValue(Grundflaeche * (ui->dSpinBox_cmVomBodenMenge2 -> value()) / 1000 );
    ui->dSpinBox_cmVonObenMenge2 -> setValue(Hoehe - ui->dSpinBox_cmVomBodenMenge2 -> value());
  }
  berechne();
}

void DialogBerVerdampfung::berechne()
{
	double erg = ((ui->dSpinBox_Menge1->value() - ui->dSpinBox_Menge2->value()) * 100 *60)
      / (ui->dSpinBox_Menge2->value() * ui->spinBox_Kochdauer->value());
  ui->dSpinBox_Verdampfungsziffer->setValue(erg);
}
double DialogBerVerdampfung::getHoehe() const
{
  return Hoehe;
}

void DialogBerVerdampfung::setHoehe(double value)
{
  Hoehe = value;
}


double DialogBerVerdampfung::getDurchmesser() const
{
  return Durchmesser;
}

void DialogBerVerdampfung::setDurchmesser(double value)
{
  Durchmesser = value;
}


void DialogBerVerdampfung::setMenge1(double value)
{
  ui->dSpinBox_Menge1->setValue(value);
  double Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
  ui->dSpinBox_cmVomBodenMenge1 -> setValue(ui->dSpinBox_Menge1 -> value() * 1000 / Grundflaeche);
  ui->dSpinBox_cmVonObenMenge1 -> setValue(Hoehe - ui->dSpinBox_cmVomBodenMenge1 -> value());
}

void DialogBerVerdampfung::setMenge2(double value)
{
  ui->dSpinBox_Menge2->setValue(value);
  double Grundflaeche = pow(Durchmesser/2, 2) * M_PI ;
  ui->dSpinBox_cmVomBodenMenge2 -> setValue(ui->dSpinBox_Menge2 -> value() * 1000 / Grundflaeche);
  ui->dSpinBox_cmVonObenMenge2 -> setValue(Hoehe - ui->dSpinBox_cmVomBodenMenge2 -> value());
}

void DialogBerVerdampfung::setKochdauer(int value)
{
  ui->spinBox_Kochdauer->setValue(value);
}

double DialogBerVerdampfung::getVerdampfungsziffer()
{
  return ui->dSpinBox_Verdampfungsziffer->value();
}


