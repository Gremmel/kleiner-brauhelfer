#include <QMessageBox>
#include "erweitertezutatimpl.h"
#include "definitionen.h"
#include "rohstoffaustauschen.h"

//
ErweiterteZutatImpl::ErweiterteZutatImpl( QWidget * parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  setupUi(this);
  erg_Menge = 0;
  ausbeute = 0;
  farbe = 0;
  typ = -1;
  animationPos = new QPropertyAnimation(this, "pos");
}

void ErweiterteZutatImpl::WerteNeuAusRohstoffeHolen()
{
  on_comboBox_Zutat_currentIndexChanged(comboBox_Zutat -> currentText());
}

void ErweiterteZutatImpl::setDisabled(bool status, bool statusZeitraum)
{
  QAbstractSpinBox::ButtonSymbols bs;

  if (!status) {
    bs = QAbstractSpinBox::UpDownArrows;
  }
  else {
    bs = QAbstractSpinBox::NoButtons;
  }

  comboBox_Zutat -> setDisabled(status);
  comboBox_Zutat -> setEditable(status);
  comboBox_Zugabezeitpunkt -> setDisabled(status);
  comboBox_Zugabezeitpunkt -> setEditable(status);
  dsb_Menge -> setReadOnly(status);
  dsb_Menge -> setButtonSymbols(bs);
  textEdit_Komentar -> setReadOnly(status);
  pushButton_del -> setDisabled(status);

  dateEdit_zugabezeitpunkt_bis->setDisabled(statusZeitraum);
  dateEdit_zugabezeitpunkt_von->setDisabled(statusZeitraum);
}

void ErweiterteZutatImpl::setBierWurdeGebraut(bool value)
{
  BierWurdeGebraut = value;
}

int ErweiterteZutatImpl::getID()
{
  return ID;
}


void ErweiterteZutatImpl::setID(int value)
{
  ID = value;
}



void ErweiterteZutatImpl::setEwListe(QStringList value)
{
  ewListe = value;
}


void ErweiterteZutatImpl::setHopfenListe(QStringList value)
{
  hopfenListe = value;
  ErstelleAuswahlliste();
}


void ErweiterteZutatImpl::ErstelleAuswahlliste()
{
  if (!BierWurdeGebraut){
    //Aktuelle Auswahl speichern
    QString s = comboBox_Zutat -> currentText();
    comboBox_Zutat -> clear();
    comboBox_Zutat -> addItems(ewListe);
    comboBox_Zutat -> addItems(hopfenListe);

    //zwischengespeicherte auswahl wieder anwählen
    for (int i=0; i < comboBox_Zutat -> count(); i++){
      if (comboBox_Zutat -> itemText(i) == s){
        comboBox_Zutat -> setCurrentIndex(i);
      }
    }
  }

}


void ErweiterteZutatImpl::on_comboBox_Zutat_currentIndexChanged(QString string)
{
  if (string != ""){
    // Überprüfen ob Eintrag in der Hopfenliste ist
    if ((hopfenListe.indexOf(string) < 0)  && !((typ == 100) && (BierWurdeGebraut))){
      if (!BierWurdeGebraut){
        ausbeute = sig_getEwzAusbeute(string);
        farbe = sig_getEwzFarbe(string);
        typ = sig_getEwzTyp(string);
        preis = sig_getEwzPreis(string);
      }
      //Icon setzten
      //Honig
      if (typ == 0){
        pixmapTyp.load(":/ewz/ewz_typ_0.svg");
        label_Icon -> setPixmap(pixmapTyp);
      }
      //Zucker
      else if (typ == 1){
        pixmapTyp.load(":/ewz/ewz_typ_1.svg");
        label_Icon -> setPixmap(pixmapTyp);
      }
      //Gewürze
      else if (typ == 2){
        pixmapTyp.load(":/ewz/ewz_typ_2.svg");
        label_Icon -> setPixmap(pixmapTyp);
      }
      //Früchte
      else if (typ == 3){
        pixmapTyp.load(":/ewz/ewz_typ_3.svg");
        label_Icon -> setPixmap(pixmapTyp);
      }
      //Sonstiges
      else if (typ == 4){
        pixmapTyp.load(":/ewz/ewz_typ_4.svg");
        label_Icon -> setPixmap(pixmapTyp);
      }

      //Einheit festlegen
      if (!BierWurdeGebraut){
        einheit = sig_getEwzEinheit(string);
      }
      //Kilogramm
      if (einheit == 0){
        label_Mengeneinheit -> setText("g/L");
        ergWidget -> label_Einheit -> setText("Kg");
        ergWidget -> spinBox_Wert -> setDecimals(3);
      }
      //Gramm
      else if (einheit == 1){
        label_Mengeneinheit -> setText("g/L");
        ergWidget -> label_Einheit -> setText("g");
        ergWidget -> spinBox_Wert -> setDecimals(0);
      }
    }
    //Typ ist Hopfen
    else {
      typ = 100;
      pixmapTyp.load(":/ewz/ewz_typ_100.svg");
      label_Icon -> setPixmap(pixmapTyp);
      label_Mengeneinheit -> setText("g/L");
      ergWidget -> label_Einheit -> setText("g");
      ergWidget -> spinBox_Wert -> setDecimals(1);
      //Einheit auf gramm festlegen
      einheit = 1;
      if (!BierWurdeGebraut){
        preis = sig_getEwzPreisHopfen(string);
      }
    }

    //Auswahlliste für Zugabezeitpunkt füllen
    ZugabezeitpunktListe.clear();
    ZugabezeitpunktListe.append(trUtf8("bei der Gärung"));
    // Alles ausser Hopfen
    if (typ == 0 || typ == 1){
      ZugabezeitpunktListe.append(trUtf8("bei Kochbegin"));
      ZugabezeitpunktListe.append(trUtf8("beim Maischen"));
    }
    else if (typ < 100){
      ZugabezeitpunktListe.append(trUtf8("beim Kochen"));
      ZugabezeitpunktListe.append(trUtf8("beim Maischen"));
    }
    //aktuellen zugabezeitpunt merken
    QString s = comboBox_Zugabezeitpunkt -> currentText();
    comboBox_Zugabezeitpunkt -> clear();
    comboBox_Zugabezeitpunkt -> addItems(ZugabezeitpunktListe);
    //zwischengespeicherte auswahl wieder anwählen
    for (int i=0; i < comboBox_Zugabezeitpunkt -> count(); i++){
      if (comboBox_Zugabezeitpunkt -> itemText(i) == s){
        comboBox_Zugabezeitpunkt -> setCurrentIndex(i);
      }
    }

    ergWidget -> label_Beschreibung -> setText(string);
    emit sig_Aenderung();
  }
}

QString ErweiterteZutatImpl::getName()
{
  return comboBox_Zutat -> currentText();
}


double ErweiterteZutatImpl::getMenge()
{
  return dsb_Menge -> value();
}


int ErweiterteZutatImpl::getEinheit()
{
  return einheit;
}

void ErweiterteZutatImpl::setEinheit(int value)
{
  einheit = value;
}


int ErweiterteZutatImpl::getTyp()
{
  return typ;
}

void ErweiterteZutatImpl::setTyp(int value)
{
  typ = value;
}


int ErweiterteZutatImpl::getZeitpunkt()
{
  return comboBox_Zugabezeitpunkt -> currentIndex();
}


QString ErweiterteZutatImpl::getBemerkung()
{
  return textEdit_Komentar -> toPlainText();
}


double ErweiterteZutatImpl::getErg_Menge()
{
  return erg_Menge;
}


void ErweiterteZutatImpl::setErg_Menge(double value)
{
  //Menge in gramm
  erg_Menge = value;
  //Kilogramm
  if (einheit == 0){
    value = value / 1000;
  }
  //Gramm
  else if (einheit == 1){
  }
  ergWidget -> spinBox_Wert -> setValue(value);
}


void ErweiterteZutatImpl::setName(QString Name)
{
  if (!BierWurdeGebraut){
    bool gefunden = false;
    for (int i=0; i < comboBox_Zutat -> count(); i++){
      if (comboBox_Zutat -> itemText(i) == Name){
        comboBox_Zutat -> setCurrentIndex(i);
        ergWidget -> label_Beschreibung -> setText(Name);
        gefunden = true;
      }
    }
    if (!gefunden){
      //Dialog mit Rohstoffauswahl zum Austauschen des zu löschenden Rohstoffes anzeigen
      RohstoffAustauschen raDia;
      raDia.setButtonCancelVisible(false);

      //Text für Dialog erstellen
      QString text = trUtf8("Rohstoffeintrag in den Weiteren Zutaten ist nicht mehr vorhanden") + " <br><b>" + Name + "</b> ";
      text += "<br>Bitte einen Ersatz auswählen?";
      raDia.SetText(text);
      //Auswahl für Ersetzung füllen
      for (int i=0; i < comboBox_Zutat->count(); i++){
        if (Name != comboBox_Zutat -> itemText(i)){
          raDia.addAuswahlEintrag(comboBox_Zutat -> itemText(i) );
        }
      }
      //raDia.setAktAuswahl(letzeAuswahl);
      raDia.exec();
      //Austauschen
      QString auswahl = raDia.GetAktAuswahl();
      for (int i=0; i < comboBox_Zutat -> count(); i++){
        if (comboBox_Zutat -> itemText(i) == auswahl){
          comboBox_Zutat -> setCurrentIndex(i);
          ergWidget -> label_Beschreibung -> setText(auswahl);
        }
      }
    }
  }
  else {
    comboBox_Zutat -> addItem(Name);
    comboBox_Zutat -> setCurrentIndex(0);
    ergWidget -> label_Beschreibung -> setText(Name);
  }
}


void ErweiterteZutatImpl::setMenge(double Menge)
{
  dsb_Menge -> setValue(Menge);
}


void ErweiterteZutatImpl::setZeitpunkt(int Zeitpunkt)
{
  comboBox_Zugabezeitpunkt -> setCurrentIndex(Zeitpunkt);
}


void ErweiterteZutatImpl::setBemerkung(QString Bemerkung)
{
  textEdit_Komentar -> setPlainText(Bemerkung);
}


void ErweiterteZutatImpl::on_comboBox_Zugabezeitpunkt_currentIndexChanged(int index)
{
  if (index == 0) {
    label_von->setVisible(true);
    label_bis->setVisible(true);
    dateEdit_zugabezeitpunkt_bis->setVisible(true);
    dateEdit_zugabezeitpunkt_von->setVisible(true);
  }
  else {
    label_von->setVisible(false);
    label_bis->setVisible(false);
    dateEdit_zugabezeitpunkt_bis->setVisible(false);
    dateEdit_zugabezeitpunkt_von->setVisible(false);
  }
  emit sig_Aenderung();
}

void ErweiterteZutatImpl::on_dsb_Menge_valueChanged(double )
{
  emit sig_Aenderung();
}

void ErweiterteZutatImpl::on_textEdit_Komentar_textChanged()
{
  emit sig_Aenderung();
}

void ErweiterteZutatImpl::closeEvent(QCloseEvent *)
{
  emit sig_Aenderung();
  ergWidget -> close();
}

void ErweiterteZutatImpl::on_pushButton_del_clicked()
{
  if (faderWidget)
    faderWidget->close();

  faderWidget = new FaderWidget(this);
  connect(faderWidget, SIGNAL(sig_fertig()), this, SLOT(on_fadeout_fertig()));
  animationAktiv = true;
  faderWidget->start();
}

void ErweiterteZutatImpl::on_fadeout_fertig()
{
  emit sig_vorClose(ID);
  close();
}

int ErweiterteZutatImpl::getAusbeute()
{
  return ausbeute;
}

void ErweiterteZutatImpl::setAusbeute(int value)
{
  ausbeute = value;
}

double ErweiterteZutatImpl::getFarbe()
{
  return farbe;
}

void ErweiterteZutatImpl::setFarbe(double value)
{
  farbe = value;
}


double ErweiterteZutatImpl::getErg_Kosten()
{
  //Menge in Gramm Preis ist in Kg
  return erg_Menge * preis/1000;
}


void ErweiterteZutatImpl::on_dateEdit_zugabezeitpunkt_von_dateChanged(const QDate &date)
{
  if (dateEdit_zugabezeitpunkt_bis->date() < date) {
    dateEdit_zugabezeitpunkt_bis->setDate(date);
  }
  emit sig_Aenderung();
}

void ErweiterteZutatImpl::on_dateEdit_zugabezeitpunkt_bis_dateChanged(const QDate &date)
{
  if (date < dateEdit_zugabezeitpunkt_von->date()) {
    dateEdit_zugabezeitpunkt_bis->setDate(dateEdit_zugabezeitpunkt_von->date());
  }
  emit sig_Aenderung();
}
