#include <QMessageBox>
#include "hopfengabe.h"
#include "ui_hopfengabe.h"
#include <QDebug>
#include "definitionen.h"
#include "rohstoffaustauschen.h"

hopfengabe::hopfengabe(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::hopfengabe)
{
	ui->setupUi(this);
	SollIBU = 1000;
	ui->spinBox_IBUAnteil ->setValue(1000);
	prozentOK = false;
	ui->dsb_Menge->setValue(10);
	ui->dsb_MengeGramm->setMinimum(0);

    ergWidget = new doubleEditLineImpl(this);
    ergWidget->setAttribute(Qt::WA_DeleteOnClose);
    ergWidget->setVisible(false);
    ergWidget->spinBox_Wert->setDecimals(2);
    ergWidget->label_Einheit->setText("g");
    ergWidget->label_Einheit2->setText("g");
}

hopfengabe::~hopfengabe()
{
	delete ui;
}

void hopfengabe::setID(int value)
{
	ID = value;
}

int hopfengabe::getID()
{
	return ID;
}

void hopfengabe::setHopfenListe(QStringList value)
{
	hopfenListe = value;
	ErstelleAuswahlliste();
}
bool hopfengabe::getBierWurdeGebraut() const
{
	return BierWurdeGebraut;
}

void hopfengabe::setBierWurdeGebraut(bool value)
{
	BierWurdeGebraut = value;
	if (value)
		setFehlProzent(0);
    ergWidget->setRestVisible(!value);
}

void hopfengabe::setVWH(bool value)
{
	ui->checkBox_vwh->setChecked(value);
}

int hopfengabe::getMaxKochzeit() const
{
	return MaxKochzeit;
}

void hopfengabe::setMaxKochzeit(int value)
{
	MaxKochzeit = value;
	ui->spinBox_Kochdauer->setMaximum(value);
	if (ui->checkBox_vwh->isChecked()){
		ui->spinBox_Kochdauer->setValue(MaxKochzeit);
  }
}

void hopfengabe::setMinKochzeit(int value)
{
  ui->spinBox_Kochdauer->setMinimum(value);
}

void hopfengabe::setFehlProzent(double value)
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

double hopfengabe::getFehlProzent()
{
  return fehlProzent;
}

double hopfengabe::getMengeProzent()
{
	return ui->dsb_Menge->value();
}

void hopfengabe::setMengeProzent(double value)
{
	ui->dsb_Menge->setValue(value);
}

void hopfengabe::setName(QString Name)
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
      QString text = trUtf8("Rohstoffeintrag in den Hopfengaben ist nicht mehr vorhanden") + " <br><b>" + Name + "</b> ";
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
		ui->comboBox_Zutat -> clear();
		ui->comboBox_Zutat -> addItem(Name);
		ui->comboBox_Zutat -> setCurrentIndex(0);
	}
}

QString hopfengabe::getName()
{
	 return ui->comboBox_Zutat->currentText();
}

void hopfengabe::setErgMenge(double value)
{
	//Menge für 100Prozent berechnen
	if (value == 0)
		Menge100Prozent = 1000;
	else
		Menge100Prozent = value / ui->spinBox_IBUAnteil->value() * SollIBU;
	ui->dsb_MengeGramm->setMaximum(Menge100Prozent);

	ui->dsb_MengeGramm->setValue(value);
	MengeGrammWertAlt = ui->dsb_MengeGramm->value();
	ergWidget->spinBox_Wert->setValue(value);

	QString s;
	s = ui->comboBox_Zutat->currentText();
	if (Pellets)
		s += trUtf8("  (Pellets) ");
	else
		s += trUtf8("  (Dolden) ");
    s += QLocale().toString(Alpha)+" % Alpha";
	ergWidget->label_Beschreibung->setText(s);

	if (ui->checkBox_vwh->isChecked()){
		ui->spinBox_Kochdauer->setReadOnly(true);
		ui->spinBox_Kochdauer->setValue(MaxKochzeit);
	}
	else {
		ui->spinBox_Kochdauer->setReadOnly(false);
	}
  //Setzte Farbwerte der Liste entsprchend der vorhandenen Mengen
  setHopfenListeFarbe();

}

double hopfengabe::getErgMenge()
{
	return ergWidget->spinBox_Wert->value();
}

void hopfengabe::setKochzeit(int value)
{
	ui->spinBox_Kochdauer->setValue(value);
}

int hopfengabe::getKochzeit()
{
	return ui->spinBox_Kochdauer->value();
}

void hopfengabe::setDisabled(bool status)
{
	QAbstractSpinBox::ButtonSymbols bs;

	if (!status) {
		bs = QAbstractSpinBox::UpDownArrows;
	}
	else {
		bs = QAbstractSpinBox::NoButtons;
	}

	ui->comboBox_Zutat -> setDisabled(status);
	ui->comboBox_Zutat->setEditable(status);
	ui->dsb_MengeGramm -> setReadOnly(status);
	ui->dsb_MengeGramm -> setButtonSymbols(bs);
	ui->dsb_Menge -> setReadOnly(status);
	ui->dsb_Menge -> setButtonSymbols(bs);
	ui->checkBox_vwh -> setDisabled(status);
	ui->pushButton_KorrekturMenge->setVisible(!status);
	ui->pushButton_del -> setDisabled(status);
	ui->spinBox_Kochdauer -> setReadOnly(status);
	ui->spinBox_Kochdauer -> setButtonSymbols(bs);
	ui->spinBox_IBUAnteil->setVisible(!status);
	ui->label_IBUAnteil->setVisible(!status);
	ui->label_IBUAnteil_2->setVisible(!status);
	ui->spinBox_Ausbeute->setVisible(!status);
	ui->label_Ausbeute->setVisible(!status);
	ui->label_Ausbeute_2->setVisible(!status);
	ui->label_Mengeneinheit_2->setVisible(!status);
	ui->dsb_MengeGramm->setVisible(!status);
}

bool hopfengabe::getVWH()
{
	return ui->checkBox_vwh->checkState();
}

void hopfengabe::setIBUAnteil(double value)
{
	ui->spinBox_IBUAnteil->setValue(value);
}

void hopfengabe::setBerIBUProzent(bool value)
{
	QAbstractSpinBox::ButtonSymbols bs;

	berIBUProzent = value;
	if (value){
		bs = QAbstractSpinBox::UpDownArrows;
		ui->dsb_MengeGramm->setDisabled(false);
		ui->dsb_MengeGramm->setButtonSymbols(bs);
	}
	else {
		bs = QAbstractSpinBox::NoButtons;
		ui->dsb_MengeGramm->setDisabled(true);
		ui->dsb_MengeGramm->setButtonSymbols(bs);
	}
	
}

QString hopfengabe::getErgebnistext()
{
	return ergWidget->label_Beschreibung->text();
}


bool hopfengabe::getPellets() const
{
	return Pellets;
}

void hopfengabe::setPellets(bool value)
{
	Pellets = value;
}


void hopfengabe::on_pushButton_del_clicked()
{
	if (faderWidget)
		faderWidget->close();

	faderWidget = new FaderWidget(this);
    connect(faderWidget, SIGNAL(sig_fertig()), this, SLOT(slot_fadeout_fertig()));
	animationAktiv = true;
	faderWidget->start();
}

void hopfengabe::slot_fadeout_fertig()
{
	emit sig_vorClose(ID);
	close();
}

void hopfengabe::ErstelleAuswahlliste()
{
	if (!BierWurdeGebraut){
		//Aktuelle Auswahl speichern
		QString s = ui->comboBox_Zutat -> currentText();
		ui->comboBox_Zutat -> clear();
		ui->comboBox_Zutat -> addItems(hopfenListe);

		//zwischengespeicherte auswahl wieder anwählen
		for (int i=0; i < ui->comboBox_Zutat -> count(); i++){
			if (ui->comboBox_Zutat -> itemText(i) == s){
				ui->comboBox_Zutat -> setCurrentIndex(i);
			}
		}
  }
}

void hopfengabe::setHopfenListeFarbe()
{
  double menge = 0;
  for (int i=0; i < ui->comboBox_Zutat->count(); i++) {
    menge = sig_getHopfenMenge(ui->comboBox_Zutat->itemText(i));
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

void hopfengabe::setStyleDunkel(bool value)
{
  StyleDunkel = value;
}

void hopfengabe::closeEvent(QCloseEvent *)
{
  emit sig_Aenderung();
  ergWidget -> close();
}

void hopfengabe::on_dsb_Menge_valueChanged(double )
{
	emit sig_Aenderung();
}

double hopfengabe::getAlpha() const
{
	return Alpha;
}

void hopfengabe::setAlpha(double value)
{
	Alpha = value;
}

void hopfengabe::setSollIBU(double value)
{
	SollIBU = value;
}

void hopfengabe::setAusbeute(double value)
{
	ui->spinBox_Ausbeute->setValue(value);
}

void hopfengabe::on_pushButton_KorrekturMenge_clicked()
{
    ui->dsb_Menge->setValue(ui->dsb_Menge->value()+QLocale().toDouble(ui->pushButton_KorrekturMenge->text()));
}

void hopfengabe::on_comboBox_Zutat_currentIndexChanged(const QString &)
{
	emit sig_Aenderung();
}

void hopfengabe::on_checkBox_vwh_clicked()
{
	emit sig_Aenderung();
}

void hopfengabe::on_spinBox_Kochdauer_valueChanged(int )
{
	emit sig_Aenderung();
}

void hopfengabe::on_dsb_MengeGramm_valueChanged(double )
{
}

void hopfengabe::on_dsb_MengeGramm_editingFinished()
{
	if (ui->dsb_MengeGramm->value() != MengeGrammWertAlt){
		MengeGrammWertAlt = ui->dsb_MengeGramm->value();
		//qDebug() << "dsb_MengeGramm" << ui->dsb_MengeGramm->value();
		if (berIBUProzent)
			ui->dsb_Menge->setValue(ui->dsb_MengeGramm->value()/Menge100Prozent*100);
	}
}
