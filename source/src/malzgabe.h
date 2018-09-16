#ifndef MALZGABE_H
#define MALZGABE_H

#include <QWidget>
#include <QPointer>
#include "faderwidget.h"
#include "doubleeditlineimpl.h"

namespace Ui {
class malzgabe;
}

class malzgabe : public QWidget
{
    Q_OBJECT
signals:
	void sig_vorClose(int ID);
	void sig_Aenderung();
  double sig_getMalzMenge(QString name);

public:
  explicit malzgabe(QWidget *parent = 0);
  ~malzgabe();
	void setID(int value);
	int getID();
	doubleEditLineImpl* ergWidget;
	void setMalzListe(QStringList value);
  void setMalzListeFarbe();
	void setFehlProzent(double value);
  double getFehlProzent();
	double getMengeProzent();
	void setMengeProzent(double value);
	void setErgMenge(double value);
	double getErgMenge();
	void setName(QString Name);
	QString getName();
	void setDisabled(bool status);

	bool getBierWurdeGebraut() const;
	void setBierWurdeGebraut(bool value);

	double getFarbe() const;
	void setFarbe(double value);

	double getGesamtMenge() const;
	void setGesamtMenge(double value);

	void berMenge();

  void setStyleDunkel(bool value);

private slots:
  void on_pushButton_del_clicked();
    void slot_fadeout_fertig();

	void on_dsb_Menge_valueChanged(double arg1);

	void on_pushButton_KorrekturMenge_clicked();

    void on_dsb_MengeGramm_valueChanged(double value);

	void on_comboBox_Zutat_currentIndexChanged(const QString &arg1);

private:
	bool NativStyle;
	Ui::malzgabe *ui;
	QPointer<FaderWidget> faderWidget;
	bool animationAktiv;
	int ID;
	QStringList malzListe;
	void ErstelleAuswahlliste();
	bool BierWurdeGebraut;
	bool prozentOK;
	double Farbe;
	double GesamtMenge;
  double fehlProzent;
  bool StyleDunkel;

protected:
	void closeEvent(QCloseEvent *event);
};

#endif // MALZGABE_H
