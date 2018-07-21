#ifndef DIALOGSTAMMWUERZE_H
#define DIALOGSTAMMWUERZE_H

#include <QDialog>

namespace Ui {
class DialogStammwuerze;
}

class DialogStammwuerze : public QDialog
{
    Q_OBJECT

public:
    explicit DialogStammwuerze(double value, double sw = 0.0, double temp = 20.0, QWidget *parent = 0);
    ~DialogStammwuerze();
    double value() const;
    double temperature() const;

private:
    void LeseKonfig();
    double brixToPlato();

private slots:
    void on_spinBox_S_SwPlato_valueChanged(double value);
    void on_spinBox_S_SwDichte_valueChanged(double value);
    void on_spinBox_S_Temperatur_valueChanged(double value);
    void on_spinBox_R_SwBrix_valueChanged(double value);
    void on_spinBox_R_Factor_valueChanged(double value);
    void on_comboBox_FormelBrixPlato_currentIndexChanged(const QString &value);

private:
    Ui::DialogStammwuerze *ui;
    double Gaerungskorrektur;
    double SWAnstellen;
};

#endif // DIALOGSTAMMWUERZE_H
