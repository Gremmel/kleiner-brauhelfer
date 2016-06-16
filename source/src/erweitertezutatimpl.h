#ifndef ERWEITERTEZUTATIMPL_H
#define ERWEITERTEZUTATIMPL_H
//
#include <QWidget>
#include <QPixmap>
#include <QPointer>
#include <QPropertyAnimation>
#include <QDebug>
#include <QMoveEvent>
#include <QTimer>
#include "faderwidget.h"
#include "ui_ErweiterteZutat.h"
#include "doubleeditlineimpl.h"
//
class ErweiterteZutatImpl : public QWidget, public Ui::ErweiterteZutat
{
Q_OBJECT
signals:
  void sig_vorClose(int ID);
  int sig_getEwzTyp(QString zutat);
  int sig_getEwzEinheit(QString zutat);
  int sig_getEwzAusbeute(QString zutat);
  double sig_getEwzFarbe(QString zutat);
  double sig_getEwzPreis(QString zutat);
  double sig_getEwzPreisHopfen(QString zutat);
  void sig_Aenderung();
  void sig_zugeben(QString zutat, int typ, double menge);

private:
  double farbe;
  int ausbeute;
  int typ;
  int einheit;
  double preis;
  void ErstelleAuswahlliste();
  QStringList hopfenListe;
  QStringList ewListe;
  QStringList ZugabezeitpunktListe;
  int ID;
  QPixmap pixmapTyp;
  bool BierWurdeGebraut;
  bool BierWurdeAbgefuellt;
  QPointer<FaderWidget> faderWidget;
  bool animationAktiv;
  QPropertyAnimation *animationPos;
  QPoint pVon, pNach;
  int z;
  bool NativStyle;
  int zugabestatus;
  QDateTime Zugabezeitpunkt_bis;

public:
  double getErg_Kosten();
  double getFarbe();
  void setFarbe(double value);
  int getAusbeute();
  void setAusbeute(int value);
  void setBemerkung(QString Bemerkung);
  void setZeitpunkt(int Zeitpunkt);
  void setMenge(double Menge);
  void setName(QString Name);
  void setErg_Menge(double value);
  double getErg_Menge();
  double erg_Menge;
  QString getBemerkung();
  int getZeitpunkt();
  int getTyp();
  void setTyp(int value);
  int getEinheit();
  void setEinheit(int value);
  double getMenge();
  QString getName();
  void setHopfenListe(QStringList value);
  void setEwListe(QStringList value);
  void setID(int value);
  int getID();
  doubleEditLineImpl* ergWidget;
  ErweiterteZutatImpl( QWidget * parent = 0,  Qt::WindowFlags f = 0 );
  void WerteNeuAusRohstoffeHolen();
  void setUIStatus();
  void setBierWurdeGebraut(bool value);
  void setZugabezeitpunkt(QDate datum_von, QDate datum_bis);
  QDate getZugabezeitpunkt_von();
  QDate getZugabezeitpunkt_bis();
  void setEntnahmeindex(int index);
  int getEntnahmeindex();
  int getZugabestatus() const;
  void setZugabestatus(int value);
  int getDauerTage();
  void setDauerTage(int value);
  //Markiert die Zutat als zugegeben
  void zutatZugeben();
  //Markiert die Zutat als entnommen
  void zutatEntnehmen();

  bool getBierWurdeAbgefuellt() const;
  void setBierWurdeAbgefuellt(bool value);

private slots:
  void on_textEdit_Komentar_textChanged();
  void on_dsb_Menge_valueChanged(double );
  void on_comboBox_Zugabezeitpunkt_currentIndexChanged(int index);
  void on_comboBox_Zutat_currentIndexChanged(QString );
  void on_pushButton_del_clicked();
  void on_fadeout_fertig();
  void on_dateEdit_zugabezeitpunkt_von_dateChanged(const QDate &date);

  void on_comboBox_entnahme_currentIndexChanged(int index);

  void on_buttonZugeben_clicked();

  void on_buttonEntnehmen_clicked();

  void on_spinBox_EWZ_DauerTage_valueChanged(int arg1);

protected:
  void closeEvent(QCloseEvent *event);

};
#endif





