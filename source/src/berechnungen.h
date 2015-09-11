#ifndef BERECHNUNGEN_H
#define BERECHNUNGEN_H
//
#include <QColor>
//
class QBerechnungen  
{
private:
	double densityAtBaseTemp(double base, double plato);
	void interpolateTemperature(double* result, double temperature);
	double interpolate(double x1,double y1,double x2,double y2,double x3,double y3,double x4,double y4,double x);
	double ErforderlicherZuckerEndvergoren;
	double ErforderlicherZucker;
	double ErforderlicheSpeiseGesammt;
	double HaushaltszuckerGesammt;
	double CO2SaettigungJungbier;
	double Gruenschlauchzeitpunkt;
	double RestextraktSchnellgaerprobe;
	double RestextraktJungbier;
	double TemperaturJungbier;
	double SollCO2;
	double Nachguss;
	double Hauptguss;
	double PfanneVoll;
	//Git die Bitterstoffausbeute anhand einer Tabelle zurück
	double GetHopfenausbeute(double Kochzeit, double Stammwuerze, bool pellets);
	//Erforderliche Extraktmenge in (Gramm)
	double GesammtExtraktmenge;
	//Gesammte Masse der Würze (KG)
	double GesammtmasseWuerze;
	//Dichte der Stammwürze
	double DichteSollStammwuerze;
	//Gewünschte Menge an Bier (Liter)
	double Wunschmenge; 
	//Stammwürze die das Bier haben soll (°P)
	double SollStammwuerze; 
	//Prozentuale ausbeute Malz (%)
	double Sudhausausbeute;
	//Praktisch genutzter Extraktgehalt (g/Kg)
	double EffektiverExtraktgehalt;
	//Gesammte Schüttung (Kg)
	double GesammtSchuettung;

public:
	void setSollStammwuerze(double sw);
	double BerDichteWasser(double Temperatur);
	double BerVolumenWasser(double Temperatur1, double Temperatur2, double VolumenT1);
	double GetTatsaechlicherEVG(double SW, double SVP);
	double GetScheinbarerEVG(double SW, double SVP);
	double GetIBU(double Wuerzemenge, double SW, double Hopfenmenge, double AlphaP, double Kochzeit, bool Pellets);
	double BerCO2Gehalt(double druck, double temperatur);
	QColor GetFarbwert(double EBC);
	double getEBC();
	double EBC;
	//Gibt den Berechneten Bierfarwert zurück
  QColor GetFarbwert(double* aSchuettung, double* aFarbwerte, int Anzahl, double SW, int Korrektur);
	double GetSudhausausbeute(double SW, double Menge, double Schuettung);
	double GetGradPlato(double Dichte);
	double densityAtX(double platoMeasure, double temperature, double calibrationTemp);
	double GetSpundungsdruck();
	double GetWasserVerschneidung(double SWSoll, double SWIst, double Menge);
	double GetHaushaltszuckerGesammt();
  double GetSpeiseGesammt(double SW, double SWJungbier, double Jungbiermenge, double SpeiseVerfuegbar, bool Spunden);
	double GetGruenschlauchzeitpunkt(double cSollCO2, double cTemperatur, double cRestextrakt);
  double GetPfanneVoll(double verdampfungsziffer, int kochdauer, double cWunschmenge);
	double BerAlkohoVol(double SW, double SVP);
	//Berechnet die Menge der Hopfengaben nach Gewichtsprozentanteil - Gesammtschüttung muss vorher berechnet sein
	void BerHopfenGewichtProzent( double* resultMenge,
									double* Alphaprozent,
									double* Mengeprozent,
									int* Kochdauer,
									bool* Pellets,
									double SollIBU,
                  int ArrayAnzahl,
									bool* Vorderwuerzehopfung,
									double* Ausbeute,
									double* IBUAnteil);
	//Berechnet die Menge der Hopfengaben nach IBU Prozentanteil - Gesammtschüttung muss vorher berechnet sein
	void BerHopfenIBUProzent( double* resultMenge,
									double* Alphaprozent,
									double* Mengeprozent,
									int* Kochdauer,
									bool* Pellets,
									double SollIBU,
									int ArrayAnzahl,
									bool* Vorderwuerzehopfung,
									double* Ausbeute,
									double* IBUAnteil);
	//Hauptguss muss vorher berechnet sein
  double GetNachguss(double verdampfungsziffer, int kochdauer, double korrektur);
	//Gesammtschuettung muss vorher berechnet sein
	double GetHauptguss(double faktor);
	double GetDichte(double GradPlato);
	double GetGesammtSchuettung(double cWunschmenge, double cSollStammwuerze, double cSudhausausbeute);
	QBerechnungen();
	
};
#endif
