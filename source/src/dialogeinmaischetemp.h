#ifndef DIALOGEINMAISCHETEMP_H
#define DIALOGEINMAISCHETEMP_H

#include <QDialog>

namespace Ui {
class DialogEinmaischeTemp;
}

class DialogEinmaischeTemp : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEinmaischeTemp(double schuettung, double schuettungTemp, double hauptguss, double rastTemp, QWidget *parent = 0);
    ~DialogEinmaischeTemp();
    double value() const;

private slots:
    void on_doubleSpinBox_Schuettung_valueChanged(double arg1);
    void on_doubleSpinBox_SchuettungTemp_valueChanged(double arg1);
    void on_doubleSpinBox_Hauptguss_valueChanged(double arg1);
    void on_doubleSpinBox_RastTemp_valueChanged(double arg1);

private:
    void doCalc();

private:
    Ui::DialogEinmaischeTemp *ui;
};

#endif // DIALOGEINMAISCHETEMP_H
