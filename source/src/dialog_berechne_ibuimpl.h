#ifndef DIALOG_BERECHNE_IBUIMPL_H
#define DIALOG_BERECHNE_IBUIMPL_H
//
#include <QDialog>
#include "ui_ber_ibu.h"
#include "berechnungen.h"

//
class Dialog_Berechne_IBUImpl : public QDialog, public Ui::Berechne_IBU
{
Q_OBJECT
private:
	void BerAlles();
	QBerechnungen Berechnungen;
public:
	Dialog_Berechne_IBUImpl( QWidget * parent = 0,  Qt::WindowFlags f = 0 );
private slots:
	void on_spinBox_Nachisomerisierung_valueChanged(int );
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
	void on_checkBox_Pellets_H_1_clicked();
	void on_checkBox_Pellets_H_2_clicked();
	void on_checkBox_Pellets_H_3_clicked();
	void on_checkBox_Pellets_H_4_clicked();
	void on_checkBox_Pellets_H_5_clicked();
	void on_checkBox_Pellets_H_6_clicked();
	void on_spinBox_Menge_valueChanged(int );
	void on_doubleSpinBox_SW_valueChanged(double );
	void on_doubleSpinBox_Alpha_H_1_valueChanged(double );
	void on_spinBox_Kochzeit_H_1_valueChanged(int );
	void on_doubleSpinBox_Menge_H_2_valueChanged(double );
	void on_doubleSpinBox_Alpha_H_2_valueChanged(double );
	void on_spinBox_Kochzeit_H_2_valueChanged(int );
	void on_doubleSpinBox_Menge_H_3_valueChanged(double );
	void on_doubleSpinBox_Alpha_H_3_valueChanged(double );
	void on_spinBox_Kochzeit_H_3_valueChanged(int );
	void on_doubleSpinBox_Menge_H_4_valueChanged(double );
	void on_doubleSpinBox_Alpha_H_4_valueChanged(double );
	void on_spinBox_Kochzeit_H_4_valueChanged(int );
	void on_doubleSpinBox_Menge_H_5_valueChanged(double );
	void on_doubleSpinBox_Alpha_H_5_valueChanged(double );
	void on_spinBox_Kochzeit_H_5_valueChanged(int );
	void on_doubleSpinBox_Menge_H_6_valueChanged(double );
	void on_doubleSpinBox_Alpha_H_6_valueChanged(double );
	void on_spinBox_Kochzeit_H_6_valueChanged(int );
	void on_spinBox_IBU_valueChanged(int );
	void on_doubleSpinBox_Menge_H_1_valueChanged(double );
};
#endif






