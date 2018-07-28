#include "msgdialog.h"
#include "ui_msgdialog.h"
#include "definitionen.h"
#include <QDebug>

MsgDialog::MsgDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::MsgDialog)
{
	ui->setupUi(this);
}

MsgDialog::~MsgDialog()
{
	delete ui;
}

void MsgDialog::checkMsgAnzeigen(QList<int> idList)
{
	IgnorIDListe = idList;
	ladeMessage();
}

void MsgDialog::setURL(QString value)
{
	url.setUrl(value);
}

QList<int> MsgDialog::getIgnorIDList()
{
	return IgnorIDListe;
}

void MsgDialog::httpFinished()
{
	if (reply->error()) {
		msgAnzeigen = false;
	}
	else {
		msgAnzeigen = true;
		Meldungen = rohMeldungen.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
		checkFilter();
	}
	
	//Abfrage war erfolgreich
	if (msgAnzeigen){
		if (Meldungen.count() > 0 ){
			aktMsg = 0;
			MeldungEintragen(aktMsg);
		}
		if (Meldungen.count() > 1){
			ui->button_next->setText(trUtf8("Nächste Meldung"));
		}
		else {
			ui->button_next->setText(trUtf8("OK"));
		}
		
		emit sig_checkFertig(Meldungen.count());
	}
	//Wenn ein Fehler aufgetreten ist signal Melden mit 0 Messages
	else
		emit sig_checkFertig(0);
}

void MsgDialog::httpReadyRead()
{
	rohMeldungen = reply->readAll();
}

void MsgDialog::ladeMessage()
{
	reply = qnam.get(QNetworkRequest(url));
	connect(reply, SIGNAL(finished()), this, SLOT(httpFinished()));
	connect(reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));
}

void MsgDialog::checkFilter()
{
	QStringList ml;
	bool Anzeigen = true;
	QStringList slm;
	//ID Ignor Liste durchgehen und überprüfen ob eine msg Angezeigt werden soll
	for (int i=0; i < Meldungen.count(); i++){
		Anzeigen = true;
		//qDebug() << slm;
		slm = Meldungen[i].split("##");
		//nur weitermachen wenn auf mindestens 5 elemente gesplittet werden konnte
		if (slm.count() >= 5){
			//Überprüfen ob ID ignoriert werden soll
			for (int o=0; o < IgnorIDListe.count(); o++){
				if (IgnorIDListe[o] == slm[0].toInt()){
					Anzeigen = false;
				}
			}
			if (Anzeigen){
				//Überprüfen ob aktuelle Version betroffen ist
				if ((slm[2].toInt() <= VERSION_INT) && (slm[3].toInt() >= VERSION_INT )){
					ml.append(Meldungen[i]);
				}
				else if ((slm[2].toInt() <= VERSION_INT) && (slm[3].toInt() == 0)){
					ml.append(Meldungen[i]);
				}
			}
		}
	}
	Meldungen = ml;
}

void MsgDialog::MeldungEintragen(int nr)
{
	if (nr < Meldungen.count()) {
		aktID = Meldungen[nr].split("##")[0].toInt();
		ui->textEdit->setText(Meldungen[nr].split("##")[1]);
	}
}

void MsgDialog::on_button_next_clicked()
{
	if (ui->checkBox_ignor->isChecked()){
		IgnorIDListe.append(aktID);
	}
	aktMsg++;
	if (aktMsg+1 == Meldungen.count()){
		ui->button_next->setText(trUtf8("OK"));
	}
	if (aktMsg < Meldungen.count())
		MeldungEintragen(aktMsg);
	else
		close();
		
}
