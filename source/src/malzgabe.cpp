#include <QMessageBox>
#include "malzgabe.h"
#include "ui_malzgabe.h"
#include "definitionen.h"
#include "rohstoffaustauschen.h"

malzgabe::malzgabe(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::malzgabe)
{
  ui->setupUi(this);
  prozentOK = false;
  ui->dsb_Menge->setValue(10);

  ergWidget = new doubleEditLineImpl(this);
  ergWidget->setAttribute(Qt::WA_DeleteOnClose);
  ergWidget->setVisible(false);
  ergWidget->label_Einheit->setText("kg");
  ergWidget->label_Einheit2->setText("kg");
}

malzgabe::~malzgabe()
{
  delete ui;
}

void malzgabe::setID(int value)
{
  ID = value;
}

int malzgabe::getID()
{
  return ID;
}

void malzgabe::setMalzListe(QStringList value)
{
  malzListe = value;
  ErstelleAuswahlliste();
}

void malzgabe::setMalzListeFarbe()
{
  double menge = 0;
  for (int i=0; i < ui->comboBox_Zutat->count(); i++) {
    menge = sig_getMalzMenge(ui->comboBox_Zutat->itemText(i));
    //Hintergund einfärben wenn von dieser Zutat nicht mehr da ist
    if (menge == 0) {
      if (StyleDunkel)
        ui->comboBox_Zutat->setItemData(i,QColor::fromRgb(FARBE_COMBO_ROHSTOFF_EMPTY_DUNKEL),Qt::TextColorRole);
      else
        ui->comboBox_Zutat->setItemData(i,QColor::fromRgb(FARBE_COMBO_ROHSTOFF_EMPTY_HELL),Qt::TextColorRole);
    }
    else if (menge < ui->dsb_MengeGramm->value()) {
      if (StyleDunkel)
        ui->comboBox_Zutat->setItemData(i,QColor::fromRgb(FARBE_COMBO_ROHSTOFF_LOW_DUNKEL),Qt::TextColorRole);
      else
        ui->comboBox_Zutat->setItemData(i,QColor::fromRgb(FARBE_COMBO_ROHSTOFF_LOW_HELL),Qt::TextColorRole);
    }
    else {
      if (StyleDunkel)
        ui->comboBox_Zutat->setItemData(i,QColor(Qt::white),Qt::TextColorRole);
      else
        ui->comboBox_Zutat->setItemData(i,QColor(40,40,40),Qt::TextColorRole);
    }
  }
}

void malzgabe::setFehlProzent(double value)
{
  fehlProzent = value;
  ui->pushButton_KorrekturMenge->setText(QLocale().toString(value));
  if (value == 0){
    ui->pushButton_KorrekturMenge->setVisible(false);
    ergWidget->setVisible(true);
  }
  else{
    ui->pushButton_KorrekturMenge->setVisible(true);
    ergWidget->setVisible(false);
  }
}

double malzgabe::getFehlProzent()
{
  return fehlProzent;
}

double malzgabe::getMengeProzent()
{
  return ui->dsb_Menge->value();
}

void malzgabe::setMengeProzent(double value)
{
  ui->dsb_Menge->setValue(value);
}


bool malzgabe::getBierWurdeGebraut() const
{
  return BierWurdeGebraut;
}

void malzgabe::setBierWurdeGebraut(bool value)
{
  BierWurdeGebraut = value;
  if (value)
    setFehlProzent(0);
  ergWidget->setRestVisible(!value);
}

double malzgabe::getFarbe() const
{
  return Farbe;
}

void malzgabe::setFarbe(double value)
{
  Farbe = value;
}

void malzgabe::setErgMenge(double value)
{
  ui->dsb_MengeGramm->setValue(value);
  ergWidget->spinBox_Wert->setValue(value);
  //Setzte Farbwerte der Liste entsprchend der vorhandenen Malzmenge
  setMalzListeFarbe();
}

double malzgabe::getErgMenge()
{
  return ui->dsb_MengeGramm->value();
}

void malzgabe::setName(QString Name)
{
  if (!BierWurdeGebraut){
    bool gefunden = false;
    for (int i=0; i < ui->comboBox_Zutat -> count(); i++){
      if (ui->comboBox_Zutat -> itemText(i) == Name){
        ui->comboBox_Zutat -> setCurrentIndex(i);
        ergWidget -> label_Beschreibung -> setText(Name);
        gefunden = true;
      }
    }
    if (!gefunden){
      //Dialog mit Rohstoffauswahl zum Austauschen des zu löschenden Rohstoffes anzeigen
      RohstoffAustauschen raDia;
      raDia.setButtonCancelVisible(false);

      //Text für Dialog erstellen
      QString text = trUtf8("Rohstoffeintrag in den Malz Zutaten ist nicht mehr vorhanden") + " <br><b>" + Name + "</b> ";
      text += "<br>Bitte einen Ersatz auswählen?";
      raDia.SetText(text);
      //Auswahl für Ersetzung füllen
      for (int i=0; i < ui->comboBox_Zutat->count(); i++){
        if (Name != ui->comboBox_Zutat -> itemText(i)){
          raDia.addAuswahlEintrag(ui->comboBox_Zutat -> itemText(i) );
        }
      }
      //raDia.setAktAuswahl(letzeAuswahl);
      raDia.exec();
      //Austauschen
      QString auswahl = raDia.GetAktAuswahl();
      for (int i=0; i < ui->comboBox_Zutat -> count(); i++){
        if (ui->comboBox_Zutat -> itemText(i) == auswahl){
          ui->comboBox_Zutat -> setCurrentIndex(i);
          ergWidget -> label_Beschreibung -> setText(auswahl);
        }
      }
    }
  }
  else {
    ui->comboBox_Zutat -> addItem(Name);
    ui->comboBox_Zutat -> setCurrentIndex(0);
    ergWidget -> label_Beschreibung -> setText(Name);
  }
}

QString malzgabe::getName()
{
  return ui->comboBox_Zutat->currentText();
}

void malzgabe::setDisabled(bool status)
{
  QAbstractSpinBox::ButtonSymbols bs;

  if (!status) {
    bs = QAbstractSpinBox::UpDownArrows;
  }
  else {
    bs = QAbstractSpinBox::NoButtons;
  }

  ui->comboBox_Zutat -> setDisabled(status);
  ui->comboBox_Zutat -> setEditable(status);
  ui->dsb_MengeGramm -> setVisible(!status);
  ui->label_Mengeneinheit_2 -> setVisible(!status);
  ui->dsb_MengeGramm -> setButtonSymbols(bs);
  ui->dsb_Menge -> setReadOnly(status);
  ui->dsb_Menge -> setButtonSymbols(bs);
  ui->pushButton_KorrekturMenge -> setVisible(!status);
  ui->pushButton_del -> setDisabled(status);
}

void malzgabe::on_pushButton_del_clicked()
{
  if (faderWidget)
    faderWidget->close();

  faderWidget = new FaderWidget(this);
  connect(faderWidget, SIGNAL(sig_fertig()), this, SLOT(slot_fadeout_fertig()));
  animationAktiv = true;
  faderWidget->start();
}

void malzgabe::slot_fadeout_fertig()
{
  emit sig_vorClose(ID);
  close();
}

void malzgabe::ErstelleAuswahlliste()
{
  if (!BierWurdeGebraut){
    //Aktuelle Auswahl speichern
    QString s = ui->comboBox_Zutat -> currentText();
    ui->comboBox_Zutat -> clear();
    ui->comboBox_Zutat -> addItems(malzListe);

    //zwischengespeicherte auswahl wieder anwählen
    for (int i=0; i < ui->comboBox_Zutat -> count(); i++){
      if (ui->comboBox_Zutat -> itemText(i) == s){
        ui->comboBox_Zutat -> setCurrentIndex(i);
      }
    }
  }
}

void malzgabe::setStyleDunkel(bool value)
{
  StyleDunkel = value;
}

void malzgabe::closeEvent(QCloseEvent *)
{
  emit sig_Aenderung();
  ergWidget -> close();
}


void malzgabe::on_dsb_Menge_valueChanged(double )
{
  emit sig_Aenderung();
}

void malzgabe::on_pushButton_KorrekturMenge_clicked()
{
  ui->dsb_Menge->setValue(ui->dsb_Menge->value()+QLocale().toDouble(ui->pushButton_KorrekturMenge->text()));
}


double malzgabe::getGesamtMenge() const
{
  return GesamtMenge;
}

void malzgabe::setGesamtMenge(double value)
{
  GesamtMenge = value;
  ui->dsb_MengeGramm->setMaximum(value);
}

void malzgabe::berMenge()
{
  setErgMenge(GesamtMenge / 100 * ui->dsb_Menge->value());
}

void malzgabe::on_dsb_MengeGramm_valueChanged(double )
{
}

void malzgabe::on_dsb_MengeGramm_editingFinished()
{
  //Anhand der veränderten Menge den Prozentualen wert errechnen
  ui->dsb_Menge->setValue(ui->dsb_MengeGramm->value()/GesamtMenge*100);
}

void malzgabe::on_comboBox_Zutat_currentIndexChanged(const QString &)
{
  ergWidget->label_Beschreibung->setText(ui->comboBox_Zutat->currentText());
  emit sig_Aenderung();
}
