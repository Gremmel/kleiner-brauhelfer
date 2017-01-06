#include "dialog_berschuettungimpl.h"
//
Dialog_BerSchuettungImpl::Dialog_BerSchuettungImpl( QWidget * parent, Qt::WindowFlags f) 
	: QDialog(parent, f)
{
	setupUi(this);
}
//


void Dialog_BerSchuettungImpl::BerAlles()
{
	//Als erstes geschamte Schüttungsmenge ermitteln
	double kg_gesamt;
	kg_gesamt = doubleSpinBox_Menge_S_1 -> value();
	kg_gesamt += doubleSpinBox_Menge_S_2 -> value();
	kg_gesamt += doubleSpinBox_Menge_S_3 -> value();
	kg_gesamt += doubleSpinBox_Menge_S_4 -> value();
	kg_gesamt += doubleSpinBox_Menge_S_5 -> value();
	kg_gesamt += doubleSpinBox_Menge_S_6 -> value();
	//Nun die Prozentuale aufteilung ermitteln
	doubleSpinBox_Prozent_S_1 -> setValue(doubleSpinBox_Menge_S_1 -> value()/kg_gesamt*100);
	doubleSpinBox_Prozent_S_2 -> setValue(doubleSpinBox_Menge_S_2 -> value()/kg_gesamt*100);
	doubleSpinBox_Prozent_S_3 -> setValue(doubleSpinBox_Menge_S_3 -> value()/kg_gesamt*100);
	doubleSpinBox_Prozent_S_4 -> setValue(doubleSpinBox_Menge_S_4 -> value()/kg_gesamt*100);
	doubleSpinBox_Prozent_S_5 -> setValue(doubleSpinBox_Menge_S_5 -> value()/kg_gesamt*100);
	doubleSpinBox_Prozent_S_6 -> setValue(doubleSpinBox_Menge_S_6 -> value()/kg_gesamt*100);
}

void Dialog_BerSchuettungImpl::on_doubleSpinBox_Menge_S_1_valueChanged(double )
{
	BerAlles();
}

void Dialog_BerSchuettungImpl::on_doubleSpinBox_Menge_S_2_valueChanged(double )
{
	BerAlles();
}

void Dialog_BerSchuettungImpl::on_doubleSpinBox_Menge_S_3_valueChanged(double )
{
	BerAlles();
}

void Dialog_BerSchuettungImpl::on_doubleSpinBox_Menge_S_4_valueChanged(double )
{
	BerAlles();
}

void Dialog_BerSchuettungImpl::on_doubleSpinBox_Menge_S_5_valueChanged(double )
{
	BerAlles();
}

void Dialog_BerSchuettungImpl::on_doubleSpinBox_Menge_S_6_valueChanged(double )
{
	BerAlles();
}

void Dialog_BerSchuettungImpl::on_buttonBox_accepted()
{
    accept();
}

void Dialog_BerSchuettungImpl::on_buttonBox_rejected()
{
    reject();
}
