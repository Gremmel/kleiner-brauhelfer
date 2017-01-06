#include "dialog_berechne_ibuimpl.h"
//
Dialog_Berechne_IBUImpl::Dialog_Berechne_IBUImpl( QWidget * parent, Qt::WindowFlags f) 
	: QDialog(parent, f)
{
	setupUi(this);
}
//


void Dialog_Berechne_IBUImpl::BerAlles()
{
	double IBU = Berechnungen.GetIBU(spinBox_Menge -> value(), 
																		doubleSpinBox_SW -> value(), 
																		doubleSpinBox_Menge_H_1 -> value(), 
																		doubleSpinBox_Alpha_H_1 -> value(), 
																		spinBox_Kochzeit_H_1 -> value()
																		+ spinBox_Nachisomerisierung -> value(), 
																		checkBox_Pellets_H_1 -> isChecked());

	IBU += Berechnungen.GetIBU(spinBox_Menge -> value(), 
																		doubleSpinBox_SW -> value(), 
																		doubleSpinBox_Menge_H_2 -> value(), 
																		doubleSpinBox_Alpha_H_2 -> value(), 
																		spinBox_Kochzeit_H_2 -> value()
																		+ spinBox_Nachisomerisierung -> value(), 
																		checkBox_Pellets_H_2 -> isChecked());

	IBU += Berechnungen.GetIBU(spinBox_Menge -> value(), 
																		doubleSpinBox_SW -> value(), 
																		doubleSpinBox_Menge_H_3 -> value(), 
																		doubleSpinBox_Alpha_H_3 -> value(), 
																		spinBox_Kochzeit_H_3 -> value()
																		+ spinBox_Nachisomerisierung -> value(), 
																		checkBox_Pellets_H_3 -> isChecked());

	IBU += Berechnungen.GetIBU(spinBox_Menge -> value(), 
																		doubleSpinBox_SW -> value(), 
																		doubleSpinBox_Menge_H_4 -> value(), 
																		doubleSpinBox_Alpha_H_4 -> value(), 
																		spinBox_Kochzeit_H_4 -> value()
																		+ spinBox_Nachisomerisierung -> value(), 
																		checkBox_Pellets_H_4 -> isChecked());

	IBU += Berechnungen.GetIBU(spinBox_Menge -> value(), 
																		doubleSpinBox_SW -> value(), 
																		doubleSpinBox_Menge_H_5 -> value(), 
																		doubleSpinBox_Alpha_H_5 -> value(), 
																		spinBox_Kochzeit_H_5 -> value()
																		+ spinBox_Nachisomerisierung -> value(), 
																		checkBox_Pellets_H_5 -> isChecked());

	IBU += Berechnungen.GetIBU(spinBox_Menge -> value(), 
																		doubleSpinBox_SW -> value(), 
																		doubleSpinBox_Menge_H_6 -> value(), 
																		doubleSpinBox_Alpha_H_6 -> value(), 
																		spinBox_Kochzeit_H_6 -> value() 
																		+ spinBox_Nachisomerisierung -> value(), 
																		checkBox_Pellets_H_6 -> isChecked());

	doubleSpinBox_IBU -> setValue(IBU);
}


void Dialog_Berechne_IBUImpl::on_doubleSpinBox_Menge_H_1_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_spinBox_Menge_valueChanged(int )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_doubleSpinBox_SW_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_doubleSpinBox_Alpha_H_1_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_spinBox_Kochzeit_H_1_valueChanged(int )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_doubleSpinBox_Menge_H_2_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_doubleSpinBox_Alpha_H_2_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_spinBox_Kochzeit_H_2_valueChanged(int )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_doubleSpinBox_Menge_H_3_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_doubleSpinBox_Alpha_H_3_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_spinBox_Kochzeit_H_3_valueChanged(int )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_doubleSpinBox_Menge_H_4_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_doubleSpinBox_Alpha_H_4_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_spinBox_Kochzeit_H_4_valueChanged(int )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_doubleSpinBox_Menge_H_5_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_doubleSpinBox_Alpha_H_5_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_spinBox_Kochzeit_H_5_valueChanged(int )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_doubleSpinBox_Menge_H_6_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_doubleSpinBox_Alpha_H_6_valueChanged(double )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_spinBox_Kochzeit_H_6_valueChanged(int )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_spinBox_IBU_valueChanged(int )
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_checkBox_Pellets_H_1_clicked()
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_checkBox_Pellets_H_2_clicked()
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_checkBox_Pellets_H_3_clicked()
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_checkBox_Pellets_H_4_clicked()
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_checkBox_Pellets_H_5_clicked()
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_checkBox_Pellets_H_6_clicked()
{
	BerAlles();
}

void Dialog_Berechne_IBUImpl::on_buttonBox_accepted()
{
    accept();
}

void Dialog_Berechne_IBUImpl::on_buttonBox_rejected()
{
    reject();
}

void Dialog_Berechne_IBUImpl::on_spinBox_Nachisomerisierung_valueChanged(int )
{
	BerAlles();
}
