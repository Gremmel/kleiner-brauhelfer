#ifndef BEWERTUNG_H
#define BEWERTUNG_H

#include <QWidget>
#include <QDate>

namespace Ui {
class Bewertung;
}

class Bewertung : public QWidget
{
	Q_OBJECT
	
signals:
	void sig_vorClose(int ID);
	void sig_Aenderung();
	void sig_wocheClicked(int ID);

public:
	explicit Bewertung(QWidget *parent = 0);
	void setMaxSterne(int value);
	void setReifedatum(QDate date);
	QDate getReifedatum();
	void setID(int value);
	int getID();
	void setBewertungsdatum(QDate date);
	QDate getBewertungdatum();
	int getSterne();
	void setSterne(int value);
	int getWoche();
	void setBemerkung(QString str);
	QString getBemerkung();
	int getFarbe();
	void setFarbe(int value);
	int getSchaum();
	void setSchaum(int value);
	int getGeruch();
	void setGeruch(int value);
	int getGeschmack();
	void setGeschmack(int value);
	int getAntrunk();
	void setAntrunk(int value);
	int getHaupttrunk();
	void setHaupttrunk(int value);
	int getNachtrunk();
	void setNachtrunk(int value);
	int getGesamteindruck();
	void setGesamteindruck(int value);
	QString getFarbeBemerkung();
	QString getSchaumBemerkung();
	QString getGeruchBemerkung();
	QString getGeschmackBemerkung();
	QString getAntrunkBemerkung();
	QString getHaupttrunkBemerkung();
	QString getNachtrunkBemerkung();
	QString getGesamteindruckBemerkung();
	void setFarbeBemerkung(QString str);
	void setSchaumBemerkung(QString str);
	void setGeruchBemerkung(QString str);
	void setGeschmackBemerkung(QString str);
	void setAntrunkBemerkung(QString str);
	void setHaupttrunkBemerkung(QString str);
	void setNachtrunkBemerkung(QString str);
	void setGesamteindruckBemerkung(QString str);

	void setAktiv(bool b);

	~Bewertung();
	
	bool getStyleDunkel() const;
	void setStyleDunkel(bool value);
	
private slots:
	void on_dateEdit_dateChanged(const QDate &date);
	void on_button_woche_clicked();

	void on_pushButton_del_clicked();

private:
	Ui::Bewertung *ui;
	int woche;
	int ID;
	QDate Reifedatum;
	void setWoche();
	int sterne;
	QString Bemerkung;
	int farbe;
	int schaum;
	int geruch;
	int geschmack;
	int antrunk;
	int haupttrunk;
	int nachtrunk;
	int gesamteindruck;
	QString farbeBemerkung;
	QString schaumBemerkung;
	QString geruchBemerkung;
	QString geschmackBemerkung;
	QString antrunkBemerkung;
	QString haupttrunkBemerkung;
	QString nachtrunkBemerkung;
	QString gesamteindruckBemerkung;
	bool StyleDunkel;


protected:
	void closeEvent(QCloseEvent *event);
};

#endif // BEWERTUNG_H
