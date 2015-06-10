#include "bewertung.h"
#include "definitionen.h"
#include "ui_bewertung.h"
#include <QString>

Bewertung::Bewertung(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Bewertung)
{
	ui->setupUi(this);
	ui->dateEdit->setDate(QDate::currentDate());
	ui->graphicsView->init(false);

	//Einstellungen abhängig vom Betriebssystem
	//Linux
	#ifdef Q_OS_LINUX
	#endif

	//Mac OS
	#ifdef Q_OS_MAC
//        ui->pushButton_del->setMinimumWidth(22);
//        ui->pushButton_del->setMaximumWidth(22);
//        ui->pushButton_del->setMaximumHeight(20);
	#endif

	//Windows
	#ifdef Q_OS_WIN
//		ui->frame->setFrameShape(QFrame::Panel);
//		ui->pushButton_del->setMinimumWidth(22);
//		ui->pushButton_del->setMaximumWidth(22);
//		ui->pushButton_del->setMaximumHeight(18);
	#endif

	Bemerkung = "";
	sterne = 0;
	farbe = 0;
	schaum = 0;
	geruch = 0;
	geschmack = 0;
	antrunk = 0;
	haupttrunk = 0;
	nachtrunk = 0;
	gesamteindruck = 0;
}

void Bewertung::setMaxSterne(int value)
{
	ui->graphicsView->setMaxStar(value);
}

void Bewertung::setWoche()
{
	int tageReifung = Reifedatum.daysTo(ui->dateEdit->date());
	if (tageReifung > 0){
		woche = tageReifung / 7 + 1;
	}
	else
		woche = 0;

	ui->button_woche->setText(trUtf8("Woche: ") + QString::number(woche));
}
bool Bewertung::getStyleDunkel() const
{
	return StyleDunkel;
}

void Bewertung::setStyleDunkel(bool value)
{
	StyleDunkel = value;
	ui->graphicsView->setStyleDunkel(value);
}


void Bewertung::closeEvent(QCloseEvent *)
{
	emit sig_Aenderung();
}


void Bewertung::setReifedatum(QDate date)
{
	Reifedatum = date;
	setWoche();
}

QDate Bewertung::getReifedatum()
{
	return Reifedatum;
}

void Bewertung::setID(int value)
{
	ID = value;
}

int Bewertung::getID()
{
	return ID;
}

void Bewertung::setBewertungsdatum(QDate date)
{
	ui->dateEdit->setDate(date);
	setWoche();
}

QDate Bewertung::getBewertungdatum()
{
	return ui->dateEdit->date();
}

int Bewertung::getSterne()
{
	return sterne;
}

void Bewertung::setSterne(int value)
{
	sterne = value;
	ui->graphicsView->setAnzahlStar(sterne);
}

int Bewertung::getWoche()
{
	return woche;
}

void Bewertung::setBemerkung(QString str)
{
	Bemerkung = str;
}

QString Bewertung::getBemerkung()
{
	return Bemerkung;
}

int Bewertung::getFarbe()
{
	return farbe;
}

void Bewertung::setFarbe(int value)
{
	farbe = value;
}

int Bewertung::getSchaum()
{
	return schaum;
}

void Bewertung::setSchaum(int value)
{
	schaum = value;
}

int Bewertung::getGeruch()
{
	return geruch;
}

void Bewertung::setGeruch(int value)
{
	geruch = value;
}

int Bewertung::getGeschmack()
{
	return geschmack;
}

void Bewertung::setGeschmack(int value)
{
	geschmack = value;
}

int Bewertung::getAntrunk()
{
	return antrunk;
}

void Bewertung::setAntrunk(int value)
{
	antrunk = value;
}

int Bewertung::getHaupttrunk()
{
	return haupttrunk;
}

void Bewertung::setHaupttrunk(int value)
{
	haupttrunk = value;
}

int Bewertung::getNachtrunk()
{
	return nachtrunk;
}

void Bewertung::setNachtrunk(int value)
{
	nachtrunk = value;
}

int Bewertung::getGesamteindruck()
{
	return gesamteindruck;
}

void Bewertung::setGesamteindruck(int value)
{
	gesamteindruck = value;
}

QString Bewertung::getFarbeBemerkung()
{
	return farbeBemerkung;
}

QString Bewertung::getSchaumBemerkung()
{
	return schaumBemerkung;
}

QString Bewertung::getGeruchBemerkung()
{
	return geruchBemerkung;
}

QString Bewertung::getGeschmackBemerkung()
{
	return geschmackBemerkung;
}

QString Bewertung::getAntrunkBemerkung()
{
	return antrunkBemerkung;
}

QString Bewertung::getHaupttrunkBemerkung()
{
	return haupttrunkBemerkung;
}

QString Bewertung::getNachtrunkBemerkung()
{
	return nachtrunkBemerkung;
}

QString Bewertung::getGesamteindruckBemerkung()
{
	return gesamteindruckBemerkung;
}

void Bewertung::setFarbeBemerkung(QString str)
{
	farbeBemerkung = str;
}

void Bewertung::setSchaumBemerkung(QString str)
{
	schaumBemerkung = str;
}

void Bewertung::setGeruchBemerkung(QString str)
{
	geruchBemerkung = str;
}

void Bewertung::setGeschmackBemerkung(QString str)
{
	geschmackBemerkung = str;
}

void Bewertung::setAntrunkBemerkung(QString str)
{
	antrunkBemerkung = str;
}

void Bewertung::setHaupttrunkBemerkung(QString str)
{
	haupttrunkBemerkung = str;
}

void Bewertung::setNachtrunkBemerkung(QString str)
{
	nachtrunkBemerkung = str;
}

void Bewertung::setGesamteindruckBemerkung(QString str)
{
	gesamteindruckBemerkung = str;
}

void Bewertung::setAktiv(bool b)
{
	if (b){
		ui->button_woche->setEnabled(false);
	}
	else {
		ui->button_woche->setEnabled(true);
	}
}

Bewertung::~Bewertung()
{
	delete ui;
}

void Bewertung::on_dateEdit_dateChanged(const QDate &)
{
	setWoche();
	emit sig_Aenderung();
}

void Bewertung::on_button_woche_clicked()
{
	emit sig_wocheClicked(ID);
}

void Bewertung::on_pushButton_del_clicked()
{
	emit sig_vorClose(ID);
	close();
}
