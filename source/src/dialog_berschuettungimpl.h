#ifndef DIALOG_BERSCHUETTUNGIMPL_H
#define DIALOG_BERSCHUETTUNGIMPL_H
//
#include <QDialog>
#include "ui_ber_schuettung.h"
//
class Dialog_BerSchuettungImpl : public QDialog, public Ui::Dialog_BerSchuettung
{
Q_OBJECT
private:
	void BerAlles();
public:
	Dialog_BerSchuettungImpl( QWidget * parent = 0,  Qt::WindowFlags f = 0 );
private slots:
	void on_buttonBox_rejected();
	void on_buttonBox_accepted();
	void on_doubleSpinBox_Menge_S_1_valueChanged(double );
	void on_doubleSpinBox_Menge_S_2_valueChanged(double );
	void on_doubleSpinBox_Menge_S_3_valueChanged(double );
	void on_doubleSpinBox_Menge_S_4_valueChanged(double );
	void on_doubleSpinBox_Menge_S_5_valueChanged(double );
	void on_doubleSpinBox_Menge_S_6_valueChanged(double );
};
#endif





