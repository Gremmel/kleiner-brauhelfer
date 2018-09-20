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

void RohstoffAustauschen::SetText(const QString& text)
{
	ui->label_text->setText(text);
}

void RohstoffAustauschen::addAuswahlEintrag(const QString& value)
{
	ui->comboBox_Rohstoffauswahl->addItem(value);
}

QString RohstoffAustauschen::GetAktAuswahl()
{
	return ui->comboBox_Rohstoffauswahl->currentText();
}

void RohstoffAustauschen::setAktAuswahl(const QString& value)
{
	for (int i=0; i < ui->comboBox_Rohstoffauswahl -> count(); i++){
		if (ui->comboBox_Rohstoffauswahl -> itemText(i) == value)
			ui->comboBox_Rohstoffauswahl -> setCurrentIndex(i);
	}
}

void RohstoffAustauschen::setNearest(const QString& value)
{
    if (ui->comboBox_Rohstoffauswahl->count() > 0)
    {
        int dist;
        int minDist = levenshtein_distance(value, ui->comboBox_Rohstoffauswahl->itemText(0));
        int index = 0;
        for (int i = 1; i < ui->comboBox_Rohstoffauswahl->count(); ++i) {
            dist = levenshtein_distance(value, ui->comboBox_Rohstoffauswahl->itemText(i));
            if (dist < minDist) {
                minDist = dist;
                index = i;
            }
        }
        ui->comboBox_Rohstoffauswahl->setCurrentIndex(index);
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

int RohstoffAustauschen::levenshtein_distance(const QString& s1, const QString& s2)
{
    int len1 = s1.size(), len2 = s2.size();
    std::vector<int> col(len2+1), prevCol(len2+1);
    for (unsigned int i = 0; i < prevCol.size(); i++)
        prevCol[i] = i;
    for (unsigned int i = 0; i < len1; i++) {
        col[0] = i+1;
        for (unsigned int j = 0; j < len2; j++)
            col[j+1] = std::min(prevCol[1 + j] + 1, std::min(col[j] + 1, prevCol[j] + (s1[i]==s2[j] ? 0 : 1)));
        col.swap(prevCol);
    }
    return prevCol[len2];
}
