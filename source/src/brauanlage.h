#ifndef BRAUANLAGE_H
#define BRAUANLAGE_H

#include <QListWidgetItem>

class Brauanlage : public QListWidgetItem
{
private:
  double KorrekturWasser;
  double KorrekturFarbe;
  int Sudhausausbeute;
  double Maischebottich_Hoehe;
  double Maischebottich_Durchmesser;
  double Maischebottich_MaxFuellhoehe;
  double Sudpfanne_Hoehe;
  double Sudpfanne_Durchmesser;
  double Sudpfanne_MaxFuellhoehe;
  double Verdampfungsziffer;
  double Kosten;
  int ID;

public:
  Brauanlage();
  double getKorrekturWasser() const;
  void setKorrekturWasser(double value);
  int getSudhausausbeute() const;
  void setSudhausausbeute(int value);
  double getMaischebottich_Hoehe() const;
  void setMaischebottich_Hoehe(double value);
  double getMaischebottich_Durchmesser() const;
  void setMaischebottich_Durchmesser(double value);
  double getMaischebottich_MaxFuellhoehe() const;
  void setMaischebottich_MaxFuellhoehe(double value);
  double getSudpfanne_Hoehe() const;
  void setSudpfanne_Hoehe(double value);
  double getSudpfanne_Durchmesser() const;
  void setSudpfanne_Durchmesser(double value);
  double getSudpfanne_MaxFuellhoehe() const;
  void setSudpfanne_MaxFuellhoehe(double value);
  double getVerdampfungsziffer() const;
  void setVerdampfungsziffer(double value);
  int getID() const;
  void setID(int value);
  double getKorrekturFarbe() const;
  void setKorrekturFarbe(double value);
  double getKosten() const;
  void setKosten(double value);
};

#endif // BRAUANLAGE_H
