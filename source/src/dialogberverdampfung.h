#ifndef DIALOGBERVERDAMPFUNG_H
#define DIALOGBERVERDAMPFUNG_H

#include <QDialog>

namespace Ui {
class DialogBerVerdampfung;
}

class DialogBerVerdampfung : public QDialog
{
	Q_OBJECT

public:
	explicit DialogBerVerdampfung(QWidget *parent = 0);
	~DialogBerVerdampfung();

private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
	void on_dSpinBox_Menge1_valueChanged(double arg1);
	void on_dSpinBox_Menge2_valueChanged(double arg1);
  void on_spinBox_Kochdauer_valueChanged(int arg1);

  void on_dSpinBox_cmVonObenMenge1_valueChanged(double arg1);

  void on_dSpinBox_cmVomBodenMenge1_valueChanged(double arg1);

  void on_dSpinBox_cmVonObenMenge2_valueChanged(double arg1);

  void on_dSpinBox_cmVomBodenMenge2_valueChanged(double arg1);

private:
	Ui::DialogBerVerdampfung *ui;
	void berechne();
  double Durchmesser;
  double Hoehe;

public:
  void setMenge1(double value);
  void setMenge2(double value);
  void setKochdauer(int value);
  double getVerdampfungsziffer();
  double getDurchmesser() const;
  void setDurchmesser(double value);
  double getHoehe() const;
  void setHoehe(double value);
};

#endif // DIALOGBERVERDAMPFUNG_H
