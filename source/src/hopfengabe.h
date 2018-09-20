#ifndef HOPFENGABE_H
#define HOPFENGABE_H

#include <QWidget>
#include <QPointer>
#include "faderwidget.h"
#include "doubleeditlineimpl.h"

namespace Ui {
class hopfengabe;
}

class hopfengabe : public QWidget
{
    Q_OBJECT
    
signals:
	void sig_vorClose(int ID);
	void sig_Aenderung();
  double sig_getHopfenMenge(QString name);

public:
    explicit hopfengabe(QWidget *parent = 0);
    ~hopfengabe();
	void setID(int value);
	int getID();
	doubleEditLineImpl* ergWidget;
	void setHopfenListe(QStringList value);
	void setFehlProzent(double value);
  double getFehlProzent();
	double getMengeProzent();
	void setMengeProzent(double value);
	void setName(QString Name);
	QString getName();
	void setErgMenge(double value);
	double getErgMenge();
	void setKochzeit(int value);
	int getKochzeit();
	void setDisabled(bool status);
	bool getVWH();
	void setIBUAnteil(double value);
	void setBerIBUProzent(bool value);
	QString getErgebnistext();

	bool getBierWurdeGebraut() const;
	void setBierWurdeGebraut(bool value);

	void setVWH(bool value);

	int getMaxKochzeit() const;
	void setMaxKochzeit(int value);

  void setMinKochzeit(int value);

	bool getPellets() const;
	void setPellets(bool value);

	double getAlpha() const;
	void setAlpha(double value);

	void setSollIBU(double value);
	void setAusbeute(double value);

  void setStyleDunkel(bool value);

private slots:
  void on_pushButton_del_clicked();
    void slot_fadeout_fertig();

	void on_dsb_Menge_valueChanged(double arg1);

	void on_pushButton_KorrekturMenge_clicked();

	void on_comboBox_Zutat_currentIndexChanged(const QString &arg1);

	void on_checkBox_vwh_clicked();

	void on_spinBox_Kochdauer_valueChanged(int arg1);

    void on_dsb_MengeGramm_valueChanged(double value);
	
private:
	bool NativStyle;
	Ui::hopfengabe *ui;
	QPointer<FaderWidget> faderWidget;
	bool animationAktiv;
	int ID;
	QStringList hopfenListe;
	void ErstelleAuswahlliste();
  void setHopfenListeFarbe();
	bool BierWurdeGebraut;
	bool prozentOK;
	int MaxKochzeit;
	bool Pellets;
	double Alpha;
	double SollIBU;
	double Menge100Prozent;
	bool berIBUProzent;
	double MengeGrammWertAlt;
  double fehlProzent;
  bool StyleDunkel;

protected:
	void closeEvent(QCloseEvent *event);
};

#endif // HOPFENGABE_H
