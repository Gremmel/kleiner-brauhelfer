#ifndef GETROHSTOFFVORLAGE_H
#define GETROHSTOFFVORLAGE_H

#include "ui_getrohstoffvorlage.h"
#include "QSqlQueryModel"
#include "QSettings"
#include "QDir"
#include "QFile"

#include "definitionen.h"
#include "errormessage.h"

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
	bool b_ok;
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
	int m_MaxProzent;
	QString m_Eigenschaften;
	int Rohstoffart;
	
private slots:
	void on_buttonBox_accepted();

	void on_buttonBox_rejected();

private:
	QSqlDatabase dbr;
	Ui::GetRohstoffVorlage *ui;
};

#endif // GETROHSTOFFVORLAGE_H
