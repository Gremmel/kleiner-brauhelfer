#ifndef GETROHSTOFFVORLAGE_H
#define GETROHSTOFFVORLAGE_H

#include "ui_getrohstoffvorlage.h"
#include <QDialog>

namespace Ui {
class GetRohstoffVorlage;
}

class GetRohstoffVorlage : public QDialog
{
	Q_OBJECT
	
public:
    explicit GetRohstoffVorlage(QWidget *parent = 0);
	~GetRohstoffVorlage();
    void ViewMalzauswahl();
    void ViewHopfenauswahl();
    void ViewHefeauswahl();
    void ViewWeitereZutatenauswahl();
	QString m_Beschreibung;
    QString m_Verpackungsmenge;
	QString m_Temperatur;
	int m_Typ;
	int m_TypOGUG;
	int m_TypFlTr;
	int m_SED;
	QString m_EVG;
	double m_Alpha;
	double m_Wuerzemenge;
	double m_Farbe;
    double m_Ausbeute;
	int m_MaxProzent;
	QString m_Eigenschaften;
	int Rohstoffart;
	
private slots:
    void slot_save();
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
    void on_btn_Add_clicked();
    void on_btn_Remove_clicked();
    void on_btn_Import_clicked();
    void on_btn_Export_clicked();
    void on_btn_Restore_clicked();

private:
    QString getFileName(bool withPath) const;
    void viewImpl(int art);

private:
	Ui::GetRohstoffVorlage *ui;
};

#endif // GETROHSTOFFVORLAGE_H
