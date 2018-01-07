#ifndef RASTWIDGET_H
#define RASTWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QDebug>
#include <QMoveEvent>
#include "faderwidget.h"

namespace Ui {
class Rastwidget;
}

class Rastwidget : public QWidget
{
  Q_OBJECT

public:
  explicit Rastwidget(QWidget *parent = 0);
  ~Rastwidget();
  void setRastName(QString Name);
  QString getRastName();
  void setRastTemp(int Value);
  int getRastTemp();
  void setRastDauer(int Value);
  int getRastDauer();
  void setID(int Value);
  void setAnimationAus(bool value);
  int getID();
  void setDisabled(bool status);

private slots:
  void on_close();
  void slot_animation_fertig();
  void on_pushButton_NachOben_clicked();
  void on_pushButton_NachUnten_clicked();
  void on_pushButton_close_clicked();
  void on_comboBox_Rast_currentIndexChanged(int index);
  void on_spinBox_RastTemp_valueChanged(int arg1);
  void on_spinBox_RastDauer_valueChanged(int arg1);

private:
  Ui::Rastwidget *ui;
  QPointer<FaderWidget> faderWidget;
  int id;
  bool animationAus;
  bool animationAktiv;
  bool NativStyle;
  QPropertyAnimation *animation;

protected:
  void closeEvent(QCloseEvent *event);
  void moveEvent (QMoveEvent * event);

signals:
  void sig_vorClose(int ID);
  void sig_aenderung(int ID);
  void sig_nachOben(int ID);
  void sig_nachUnten(int ID);
};

#endif // RASTWIDGET_H
