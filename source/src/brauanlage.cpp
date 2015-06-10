#include "brauanlage.h"


double Brauanlage::getKorrekturWasser() const
{
  return KorrekturWasser;
}

void Brauanlage::setKorrekturWasser(double value)
{
  KorrekturWasser = value;
}

int Brauanlage::getSudhausausbeute() const
{
  return Sudhausausbeute;
}

void Brauanlage::setSudhausausbeute(int value)
{
  Sudhausausbeute = value;
}

double Brauanlage::getMaischebottich_Hoehe() const
{
  return Maischebottich_Hoehe;
}

void Brauanlage::setMaischebottich_Hoehe(double value)
{
  Maischebottich_Hoehe = value;
}

double Brauanlage::getMaischebottich_Durchmesser() const
{
  return Maischebottich_Durchmesser;
}

void Brauanlage::setMaischebottich_Durchmesser(double value)
{
  Maischebottich_Durchmesser = value;
}

double Brauanlage::getMaischebottich_MaxFuellhoehe() const
{
  return Maischebottich_MaxFuellhoehe;
}

void Brauanlage::setMaischebottich_MaxFuellhoehe(double value)
{
  Maischebottich_MaxFuellhoehe = value;
}

double Brauanlage::getSudpfanne_Hoehe() const
{
  return Sudpfanne_Hoehe;
}

void Brauanlage::setSudpfanne_Hoehe(double value)
{
  Sudpfanne_Hoehe = value;
}

double Brauanlage::getSudpfanne_Durchmesser() const
{
  return Sudpfanne_Durchmesser;
}

void Brauanlage::setSudpfanne_Durchmesser(double value)
{
  Sudpfanne_Durchmesser = value;
}

double Brauanlage::getSudpfanne_MaxFuellhoehe() const
{
  return Sudpfanne_MaxFuellhoehe;
}

void Brauanlage::setSudpfanne_MaxFuellhoehe(double value)
{
  Sudpfanne_MaxFuellhoehe = value;
}

double Brauanlage::getVerdampfungsziffer() const
{
  return Verdampfungsziffer;
}

void Brauanlage::setVerdampfungsziffer(double value)
{
  Verdampfungsziffer = value;
}

int Brauanlage::getID() const
{
  return ID;
}

void Brauanlage::setID(int value)
{
  ID = value;
}

double Brauanlage::getKorrekturFarbe() const
{
  return KorrekturFarbe;
}

void Brauanlage::setKorrekturFarbe(double value)
{
  KorrekturFarbe = value;
}

double Brauanlage::getKosten() const
{
    return Kosten;
}

void Brauanlage::setKosten(double value)
{
    Kosten = value;
}
Brauanlage::Brauanlage()
{
    setIcon(QIcon(":/global/brauanlage.svg"));
    Sudhausausbeute = 60;
    KorrekturWasser = 2;
    KorrekturFarbe = 0;
  Maischebottich_Durchmesser = 36;
  Maischebottich_Hoehe = 29;
  Maischebottich_MaxFuellhoehe = 26;
  Sudpfanne_Durchmesser = 36;
  Sudpfanne_Hoehe = 29;
  Sudpfanne_MaxFuellhoehe= 27;
}
