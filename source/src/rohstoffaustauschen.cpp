#include "rohstoffaustauschen.h"
#include "ui_rohstoffaustauschen.h"

RohstoffAustauschen::RohstoffAustauschen(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::RohstoffAustauschen)
{
	ui->setupUi(this);
	b_ok = false;
	b_rohstoff_xml_uebernehmen = false;
	ui->pushButton_RohstoffUebernehmen->setVisible(false);
}

RohstoffAustauschen::~RohstoffAustauschen()
{
	delete ui;
}

void RohstoffAustauschen::SetText(QString text)
{
	ui->label_text->setText(text);
}

void RohstoffAustauschen::addAuswahlEintrag(QString value)
{
	ui->comboBox_Rohstoffauswahl->addItem(value);
}

QString RohstoffAustauschen::GetAktAuswahl()
{
	return ui->comboBox_Rohstoffauswahl->currentText();
}

void RohstoffAustauschen::setAktAuswahl(QString value)
{
	for (int i=0; i < ui->comboBox_Rohstoffauswahl -> count(); i++){
		if (ui->comboBox_Rohstoffauswahl -> itemText(i) == value)
			ui->comboBox_Rohstoffauswahl -> setCurrentIndex(i);
	}
}

void RohstoffAustauschen::setButtonCancelVisible(bool value)
{
	ui->pushButton_cancel->setVisible(value);
}

void RohstoffAustauschen::setButtonRohstoffUebernehmenVisible(bool value)
{
	ui->pushButton_RohstoffUebernehmen->setVisible(value);
}

void RohstoffAustauschen::on_pushButton_ok_clicked()
{
	b_ok = true;
	close();
}

void RohstoffAustauschen::on_pushButton_cancel_clicked()
{
	b_ok = false;
	close();
}

void RohstoffAustauschen::on_pushButton_RohstoffUebernehmen_clicked()
{
	b_rohstoff_xml_uebernehmen = true;
	close();
}
