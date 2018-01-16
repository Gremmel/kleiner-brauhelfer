#ifndef DIALOGSUDTEILEN_H
#define DIALOGSUDTEILEN_H

#include <QDialog>

namespace Ui {
class DialogSudTeilen;
}

class DialogSudTeilen : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSudTeilen(const QString& name, double amount, QWidget *parent = 0);
    ~DialogSudTeilen();
    double prozent() const;
    QString nameTeil1() const;
    QString nameTeil2() const;

private slots:
    void on_horizontalSlider_Percent_valueChanged(int position);
    void on_spinBox_Percent1_valueChanged(int position);
    void on_spinBox_Percent2_valueChanged(int position);
    void on_doubleSpinBox_Amount1_valueChanged(double amount);
    void on_doubleSpinBox_Amount2_valueChanged(double amount);

private:
    Ui::DialogSudTeilen *ui;
    double mTotalAmount;
};

#endif // DIALOGSUDTEILEN_H
