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
    void SetText(const QString &text);
    void addAuswahlEintrag(const QString &value);
	QString GetAktAuswahl();
    void setAktAuswahl(const QString& value);
    void setNearest(const QString& value);
	void setButtonCancelVisible(bool value);
	void setButtonRohstoffUebernehmenVisible(bool value);
	bool b_ok;
	bool b_rohstoff_xml_uebernehmen;

private slots:
	void on_pushButton_ok_clicked();
	void on_pushButton_cancel_clicked();
	void on_pushButton_RohstoffUebernehmen_clicked();
private:
    int levenshtein_distance(const QString& s1, const QString& s2);
private:
	Ui::RohstoffAustauschen *ui;
};

#endif // ROHSTOFFAUSTAUSCHEN_H
