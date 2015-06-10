#ifndef ROHSTOFFAUSTAUSCHEN_H
#define ROHSTOFFAUSTAUSCHEN_H

#include <QDialog>

namespace Ui {
class RohstoffAustauschen;
}

class RohstoffAustauschen : public QDialog
{
	Q_OBJECT
	
public:
	explicit RohstoffAustauschen(QWidget *parent = 0);
	~RohstoffAustauschen();
	void SetText(QString text);
	void addAuswahlEintrag(QString value);
	QString GetAktAuswahl();
	void setAktAuswahl(QString value);
	void setButtonCancelVisible(bool value);
	void setButtonRohstoffUebernehmenVisible(bool value);
	bool b_ok;
	bool b_rohstoff_xml_uebernehmen;

private slots:
	void on_pushButton_ok_clicked();
	void on_pushButton_cancel_clicked();

	void on_pushButton_RohstoffUebernehmen_clicked();

private:
	Ui::RohstoffAustauschen *ui;
};

#endif // ROHSTOFFAUSTAUSCHEN_H
