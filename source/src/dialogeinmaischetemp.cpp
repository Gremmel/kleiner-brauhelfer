#include "dialogeinmaischetemp.h"
#include "ui_dialogeinmaischetemp.h"

DialogEinmaischeTemp::DialogEinmaischeTemp(double schuettung, double schuettungTemp, double hauptguss, double rastTemp, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEinmaischeTemp)
{
    ui->setupUi(this);
    ui->doubleSpinBox_Schuettung->setValue(schuettung);
    ui->doubleSpinBox_SchuettungTemp->setValue(schuettungTemp);
    ui->doubleSpinBox_Hauptguss->setValue(hauptguss);
    ui->doubleSpinBox_RastTemp->setValue(rastTemp);
    doCalc();
}

DialogEinmaischeTemp::~DialogEinmaischeTemp()
{
    delete ui;
}

void DialogEinmaischeTemp::doCalc()
{
    double c_w = 4.2;
    double c_malt = 1.7;
    double m_w = ui->doubleSpinBox_Hauptguss->value() * c_w;
    double m_malt = ui->doubleSpinBox_Schuettung->value() * c_malt;
    double T_malt = ui->doubleSpinBox_SchuettungTemp->value();
    double T_rest = ui->doubleSpinBox_RastTemp->value();
    double T = T_rest + m_malt * (T_rest - T_malt) / m_w;
    ui->doubleSpinBox_EinmaischeTemp->setValue(T);
}

double DialogEinmaischeTemp::value() const
{
    return ui->doubleSpinBox_EinmaischeTemp->value();
}

void DialogEinmaischeTemp::on_doubleSpinBox_Schuettung_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    doCalc();
}

void DialogEinmaischeTemp::on_doubleSpinBox_SchuettungTemp_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    doCalc();
}

void DialogEinmaischeTemp::on_doubleSpinBox_Hauptguss_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    doCalc();
}

void DialogEinmaischeTemp::on_doubleSpinBox_RastTemp_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    doCalc();
}
