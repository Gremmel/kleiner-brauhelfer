#include "mainwindowimpl.h"

#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include "definitionen.h"
#include "errormessage.h"

void MainWindowImpl::ErstelleSpickzettel()
{
  // Seitenkopf
  QString seite, kopf, ende, style;

  kopf = "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'> <html><head><meta name='qrichtext' content='1' />";
  style = "<style type='text/css'>";
  //Style für P
  style += "p{color:black;font-size:10pt;padding:0px;margin:0px;}";
  //Style für Variable
  style += ".value{color:blue;margin-left:5px;margin-right:5px;}";
  //Style für div Kommentar
  style += ".koment{}";
  //Style für ul
  style += "ul{color:black;font-size:10pt;}";
  //Style für Überschrift h1
  style += "p.h1{color:black;font-size:12pt;}";
  //Style für Überschrift h2
  style += "p.h2{color:black;font-size:11pt;margin-bottom:5px;}";
  style += "p.version{color:#999999;font-size:11pt;margin-top:5px;}";
  //Style für Div Box ohne Rahmen
  style += "div.r{border:0px solid #dddddd; border-radius: 10px; padding:5px;background-color:#dddddd;}";
  //Style für Div Box mit Rahmen
  style += "div.rm{border:2px solid #dddddd; border-radius: 10px; padding:5px;background-color:#ffffff;}";
  //Style für Hinweis
  style += ".hinweis{color:#d47209;}";
  //Style für Tabelle
  style += "td{padding:2px;margin:0px;}";
  style += "td.r{padding:2px;margin:0px;border-bottom-color:#dddddd;border-bottom-style:solid;border-width:1px;}";
  //Style für Hinweis Wert in Brau und Gärdaten eintragen
  style += "td.we{background-color: #eba328;}";
  style += "tr{padding:0px;margin:0px;}";
  style += "</style>";
  kopf += style;
  kopf += "</head><body align='center' style=' font-family:Ubuntu,Arial; font-size:10pt;font-style:normal;background-color:#fff;'>";
  seite = kopf;

  QString s = "";

  //Tabelle für Bild und Zutaten
  s += "<div class='' width='99%' style='' align='center'>";

  s += "<table width='50%' summary='testtabelle' border='0' cellspacing='5'>";
  s += "<tr >";
  s += "<td valign='middle' style=''>";
  //Solldaten des Rezeptes
  s += "<div class='rm' style='margin-top:10px;margin-bottom:5px;' align='center'>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Name
  s += "<tr style=''>";
  s += "<td>";
  s += "<p class='h1'><b>" + lineEdit_Sudname -> text() + "</b></p>";
  s += "</td>";
  s += "</tr>";
  //Menge
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Menge") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(spinBox_Menge -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Stammwürze
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Stammwürze") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(spinBox_SW -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("°P") + "</p>";
  s += "</td>";
  s += "</tr>";
  //High Gravity Faktor
  if (spinBox_High_Gravity->value() > 0) {
    s += "<tr style=''>";
    s += "<td>";
    s += "<p>" + trUtf8("High Gravity Faktor") + "</p>";
    s += "</td>";
    s += "<td align='right'>";
    s += "<p class='value'>" + QString::number(spinBox_High_Gravity -> value()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("%") + "</p>";
    s += "</td>";
    s += "</tr>";
  }
  //Bittere
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Bittere") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(spinBox_IBU -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("IBU") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Nachisomerisierungs-zeit
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Nachisomerisierungs-Zeit") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(spinBox_NachisomerisierungsZeit -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("min") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Farbe
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Farbe") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(doubleSpinBox_EBC -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("EBC") + "</p>";
  s += "</td>";
  s += "</tr>";
  //CO2 Gehalt
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("CO2 Gehalt") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(doubleSpinBox_CO2 -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("g/Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Brauanlage
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Brauanlage") + "</p>";
  s += "</td>";
  s += "<td colspan=2 align='right'>";
  s += "<p class='value'>" + comboBox_AuswahlBrauanlage->currentText() + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "</tbody></table>";
  s += "</div>";
  s += "</td>";
  s += "</tr>";
  s += "</table>";

  s += "<table width='90%' summary='tabelle' border='0' cellspacing='3'>";
  s += "<tr style=''>";
  s += "<td valign='bottom' style=''>";
  //Schüttung
  s += "<div class='rm' style='margin:0px;margin-bottom:5px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/getreide_300.png' alt='Getreide' width='300px' border=0>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Alle Malzgaben
  double fehlprozent = 0;
  if (list_Malzgaben.count()>0) {
    fehlprozent = list_Malzgaben[0]->getFehlProzent();
  }
  if (fehlprozent == 0) {
    for (int i=0; i < list_Malzgaben.count(); i++){
      s += "<tr style=''>";
      s += "<td>";
      s += "<p>" + list_Malzgaben[i]->getName() + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p class='value'>" + QString::number(list_Malzgaben[i]->getErgMenge()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("kg") + "</p>";
      s += "</td>";
      s += "<td align='right'>";
      s += "<p class='value'>" + QString::number(list_Malzgaben[i]->getMengeProzent()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("%") + "</p>";
      s += "</td>";
      s += "<td align='right'>";
      s += "<p class='value'>" + QString::number(list_Malzgaben[i]->getFarbe()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("EBC") + "</p>";
      s += "</td>";
      s += "</tr>";
    }
  }
  //Wenn die Porzentuale aufteilung der schüttung nicht stimmt
  else {
    s += "<div class='hinweis'>" + trUtf8("Die einzelnen Schüttungen konnten nicht richtig berechnet werden da die aufteilung nicht 100% entspricht!")+"</div>";
  }
  //Gesamt
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Gesamt") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value' style='font-weight:bold;'>" + QString::number(doubleSpinBox_S_Gesammt -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("kg") + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "</tbody></table>";
  s += "</div>";
  s += "</td>";


  s += "<td valign='top' style=''>";
  //bild mit entsprechender Bierfarbe
  QColor farbe;
  farbe = Berechnungen.GetFarbwert(doubleSpinBox_EBC -> value());
  s += "<div class='' style='background-color:" + farbe.name() + ";width:210px;height:210px;margin:0px;padding:0px;'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/global/bier_420x420.png' alt='Bierfarbe' width='210px' height='210px' border=0>";
  s += "</div>";
  s += "</td>";
  s += "<td valign='top' style=''>";
  //Hopfen
  s += "<div class='rm' style='margin:0px;margin-bottom:5px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/hopfen_100.png' alt='Hopfen' width='100px' border=0>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Alle Hopfengaben
  fehlprozent = 0;
  if (list_Hopfengaben.count()>0) {
    fehlprozent = list_Hopfengaben[0]->getFehlProzent();
  }
  if (fehlprozent == 0) {
    for (int i=0; i < list_Hopfengaben.count(); i++){
      s += "<tr style=''>";
      s += "<td>";
      if (list_Hopfengaben[i]->getVWH())
        s += "<p>" + trUtf8("VWH ") + list_Hopfengaben[i]->getErgebnistext() + "</p>";
      else
        s += "<p>" + list_Hopfengaben[i]->getErgebnistext() + "</p>";
      s += "</td>";
      s += "<td align='right'>";
      s += "<p class='value'>" + QString::number(list_Hopfengaben[i]->getErgMenge()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("g") + "</p>";
      s += "</td>";
      s += "<td align='right'>";
      s += "<p class='value'>" + QString::number(list_Hopfengaben[i]->getKochzeit()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("min") + "</p>";
      s += "</td>";
      s += "</tr>";
    }
  }
  //Wenn die Porzentuale aufteilung der schüttung nicht stimmt
  else {
    s += "<div class='hinweis'>" + trUtf8("Die einzelnen Hopfenhaben konnten nicht richtig berechnet werden da die aufteilung nicht 100% entspricht!")+"</div>";
  }
  //Hopfengaben in den Weiteren Zutaten
  for (int i=0; i < list_EwZutat.count(); i++){
    //Nur Hopfengaben
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Hopfen){
      s += "<tr style=''>";
      s += "<td>";
      s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
      s += "</td>";
      s += "<td align='right'>";
      s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("g") + "</p>";
      s += "</td>";
      s += "<td align='right' colspan='2'>";
      if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
        s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
      else
        s += "<p class='value'>" + trUtf8("Anstellen") + "</p>";
      s += "</td>";
      s += "</tr>";
    }
  }
  s += "</tbody></table>";
  s += "</div>";
  s += "</td>";
  s += "</tr>";
  s += "</table>";

  s += "<table width='70%' summary='tabelle' border='0' cellspacing='3'>";
  s += "<tr style=''>";
  s += "<td valign='top' style=''>";
  //Wasser
  s += "<div class='rm' style='margin:0px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/wasser_100x107.png' alt='Hefe' width='50px' border=0>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Hauptguss
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Hauptguss") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(doubleSpinBox_WHauptguss -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "<tr>";
  s += "<td>";
  s += "<p>" + trUtf8("Milchsäure (80%)") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(SpinBox_waMilchsaeureHG_ml -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("ml") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Nachguss
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Nachguss") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(doubleSpinBox_WNachguss -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Milchsäure (80%)") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(SpinBox_waMilchsaeureNG_ml -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("ml") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Gesammt
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + label_37 -> text() + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value' style='font-weight:bold;'>" + QString::number(doubleSpinBox_W_Gesammt -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "</tbody></table>";
  s += "</div>";
  s += "</td>";
  //Hefe
  s += "<td valign='top' style=''>";
  s += "<div class='rm' style='margin:0px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/hefe_50.png' alt='Hefe' width='50px' border=0>";
  s += "<p>" + comboBox_AuswahlHefe -> currentText() + "</p>";
  QString sEinheiten;
  sEinheiten = trUtf8("Anzahl Einheiten:") + " <span class='value'>" + QString::number(spinBox_AnzahlHefeEinheiten->value()) +"</span>";
  //Verpackungsgrösse aus den Rohstoffdaten auslesen
  int AnzahlHefeEintraege = tableWidget_Hefe -> rowCount();
  QString HefeName = comboBox_AuswahlHefe -> currentText();
  QString verpMenge;
  if (HefeName != ""){
    //Würzemenge auslesen
    for (int i=0; i < AnzahlHefeEintraege; i++){
      //wenn Eintrag übereinstimmt
      if (tableWidget_Hefe -> item(i,TableHefeColName) -> text() == HefeName){
        verpMenge = tableWidget_Hefe -> item(i,TableHefeColVerpackungsmenge) -> text();
      }
    }
  }
  if (!verpMenge.isEmpty()) {
    sEinheiten += trUtf8(" zu ") + verpMenge;
  }
  s += "<p>" + sEinheiten +"</p>";
  s += "</div>";
  s += "</td>";
  s += "</tr>";

  s += "</table>";
  //s += "</div>";
  //s += "</div>";

  //Tabelle Weitere Zutaten
  s += "<table width='80%' summary='testtabelle' border='0' cellspacing='5'>";
  s += "<tr >";

  //Honig
  bool HonigVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Honig){
      HonigVorhanden = true;
    }
  }
  if (HonigVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_0_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Honig){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochbeginn") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  //Zucker
  bool ZuckerVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Zucker){
      ZuckerVorhanden = true;
    }
  }
  if (ZuckerVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_1_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Zucker){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochbeginn") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  //Gewürz
  bool GewuerzVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Gewuerz){
      GewuerzVorhanden = true;
    }
  }
  if (GewuerzVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_2_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Gewuerz){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochen") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  //Frucht
  bool FruchtVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Frucht){
      FruchtVorhanden = true;
    }
  }
  if (FruchtVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_3_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Frucht){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochen") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  //Sonstiges
  bool SonstigesVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Sonstiges){
      SonstigesVorhanden = true;
    }
  }
  if (SonstigesVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_4_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Sonstiges){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochen") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  s += "</tr >";
  s += "</table>";

  //s += "<div class='r' width='99%' style='' align='center'>";
  //Kommentar
  if (textEdit_Kommentar -> toPlainText() != "") {
    s += "<div class='rm' style='margin-top:10px;width:90%'>";
    s += textEdit_Kommentar -> toHtml();
    s += "</div>";
  }


  //Geräte und Zubehör
  //-------------------------------------------------------------
  QSqlQuery query;

  int id = getBrauanlagenIDRezept();
  QString sql = "SELECT Bezeichnung FROM Geraete WHERE AusruestungAnlagenID = " + QString::number(id);
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int zaehler = 0;
    if (query.first()) {
      s += "<div align='center' class='rm' style='margin-top:10px;width:90%'>";
      s += "<p><b>";
      s += trUtf8("benötigte Gerätschaften");
      s += "</b></p>";
      s += "<table cellspacing=0 border=0><tbody>";
      if (zaehler == 0)
        s += "<tr>";
      s += "<td align=center><p>";
      s += query.value(0).toString();
      s += "</p></td>";
      if (zaehler == 2)
        s += "</tr>";
      zaehler ++;
      if (zaehler == 3)
        zaehler = 0;
      while (query.next()){
        if (zaehler == 0)
          s += "<tr>";
        s += "<td align=center><p>";
        s += query.value(0).toString();
        s += "</p></td>";
        if (zaehler == 2)
          s += "</tr>";
        zaehler ++;
        if (zaehler == 3)
          zaehler = 0;
      }
      if (zaehler != 0)
        s += "</tr>";
      s += "</tbody></table>";
      s += "</div>";
    }
  }

  //Brauablauf
  //-------------------------------------------------------------
  s += "<div align='center' class='rm' style='margin-top:10px;width:90%'>";
  s += "<p><b>";
  s += trUtf8("Maischen");
  s += "</b></p>";
  s += "<table cellspacing=0 border=0 width='90%'><tbody>";

  //Einmaischen
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Einmaischen: ");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += GetWertString(doubleSpinBox_WHauptguss -> value()) + trUtf8(" Liter Wasser auf ");
  s += GetWertString(spinBox_EinmaischenTemp -> value()) + trUtf8("°C erhitzen und Malzschüttung zugeben ");
  s += "</p></td>";
  s += "</tr>";

  //Rasten in Spickzettel eintragen
  for (int i=0; i < list_Rasten.count(); i++){
    s += "<tr>";
    s += "<td class='r' align=center><p>";
    s += list_Rasten[i]->getRastName();
    s += "</p></td>";
    s += "<td class='r' align=center><p>";
    s += trUtf8("Maische auf ");
    s += GetWertString(list_Rasten[i]->getRastTemp()) + trUtf8("°C erhitzen und ");
    s += GetWertString(list_Rasten[i]->getRastDauer()) + trUtf8(" min Rast einlegen.");
    s += "</p></td>";
    s += "</tr>";
  }

  //Weitere Zutaten beim Maischen
  bool MaischenVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen){
      MaischenVorhanden = true;
    }
  }
  if (MaischenVorhanden) {
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen){
        s += "<tr>";
        s += "<td class='r' align=center><p>";
        s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_"
            + QString::number(list_EwZutat[i] -> getTyp()) + "_50.png' alt='Honig' width='30px' border=0>";
        s += "</p></td>";
        s += "<td class='r' align=center><p>";
        s += list_EwZutat[i] -> getName() + " ";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<span class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</span>";
        else
          s += "<span class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</span>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<span>" + trUtf8(" kg") + "</span>";
        else
          s += "<span>" + trUtf8(" g") + "</span>";
        s += "<br>";
        s += list_EwZutat[i] -> getBemerkung();
        s += "</p></td>";
        s += "</tr>";
      }
    }
  }


  //Läutereimer
  //s += "<tr>";
  //s += "<td align=center colspan=2><p>";
  //s += trUtf8("Wärend der Rast Läutereimer herichten");
  //s += "</p></td>";
  //s += "</tr>";

  //Jodprobe
  //s += "<tr>";
  //s += "<td align=center><p>";
  //s += trUtf8("Jodprobe");
  //s += "</p></td>";
  //s += "<td align=center><p>";
  //s += trUtf8("Überprüfen ob Jodprobe ohne verfärbung ist, ansonsten rast etwas verlängern");
  //s += "</p></td>";
  //s += "</tr>";

  s += "</tbody></table>";
  s += "</div>";

  s += "<div align='center' class='rm' style='margin-top:10px;width:90%'>";
  s += "<p><b>";
  s += trUtf8("Läutern");
  s += "</b></p>";
  s += "<table cellspacing=0 border=0 width='90%'><tbody>";

  //Maische in den Läutereimer
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Läutern vorbereiten");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Maische in den Läutereimer schöpfen");
  s += "</p></td>";
  s += "</tr>";

  //Nachguss erhitzen
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Nachguss vorbereiten: ");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += GetWertString(doubleSpinBox_WNachguss -> value()) + trUtf8(" Liter Wasser auf ") + GetWertString(78) + trUtf8("°C erhitzen");
  s += "</p></td>";
  s += "</tr>";

  //Vorderwürze Klarlaufen lassen
  s += "<tr>";
  s += "<td class='r' align=center colspan=2><p>";
  s += trUtf8("Nach etwa 10 - 20 min Wartezeit solange Vorderwürze ablaufen lassen und wieder zurück in den Läutereimer schütten, bis Würze klar läuft.");
  s += "</p></td>";
  s += "</tr>";

  //Läutern
  s += "<tr>";
  s += "<td class='' align=center><p>";
  s += trUtf8("Läutern");
  s += "</p></td>";
  s += "<td class='' align=center><p>";
  s += trUtf8("Würze langsam ablaufen lassen (Treber darf nicht trockenlaufen!!), bevor Treber trockenläuft immer etwas (ca. 2-3 Liter) Nachguss über Schaumlöffel nachgießen.");
  s += "</p></td>";
  s += "</tr>";


  s += "</tbody></table>";
  s += "</div>";

  s += "<div align='center' class='rm' style='margin-top:10px;width:90%'>";
  s += "<p><b>";
  s += trUtf8("Würze kochen");
  s += "</b></p>";
  s += "<table cellspacing=0 border=0 width='90%'><tbody>";

  //Vorderwürzehopfung
  for (int i=0; i < list_Hopfengaben.count(); i++){
    if (list_Hopfengaben[i]->getVWH()){
      s += "<tr>";
      s += "<td class='r' align=center><p>";
      s += trUtf8("Vorderwürzehopfung: ");
      s += "</p></td>";
      s += "<td class='r' align=center><p>";
      if ((spinBox_Gesammtkochdauer -> value() - list_Hopfengaben[i]->getKochzeit()) > 0){
        s += trUtf8("Nach ") + GetWertString(spinBox_Gesammtkochdauer -> value()
                                             - list_Hopfengaben[i]->getKochzeit()) + trUtf8(" min ");
      }
      s += GetWertString(list_Hopfengaben[i]->getErgMenge()) + trUtf8("g ");
      s += list_Hopfengaben[i]->getName() + trUtf8(" Hopfen vorlegen");
      s += "</p></td>";
      s += "</tr>";
    }
  }

  //Würze zum Kochen bringen
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Würze zum Kochen bringen");
  s += "</p></td>";
  s += "</tr>";

  //Gesammtkochdauer
  int kd = 0;
  //kleinste Kochzeit
  int kkd = spinBox_Gesammtkochdauer -> value();

  //Hopfengaben nach Kochdauer absteigend sortieren
  for (int i=0; i < list_Hopfengaben.count(); i++){
    for (int o=0; o < list_Hopfengaben.count()-1; o++){
      kd = list_Hopfengaben[o]->getKochzeit();
      if (kd < list_Hopfengaben[o+1]->getKochzeit()) {
        list_Hopfengaben.move(o,o+1);
      }
    }
  }
  kd = spinBox_Gesammtkochdauer -> value();
  for (int i=0; i < list_Hopfengaben.count(); i++){
    if (!list_Hopfengaben[i]->getVWH()){
      s += "<tr>";
      s += "<td class='r' align=center><p>";
      s += QString::number(i+1) + trUtf8(". Hopfengabe: ");
      s += "</p></td>";
      s += "<td class='r' align=center><p>";
      if ((spinBox_Gesammtkochdauer -> value() - list_Hopfengaben[i]->getKochzeit()) > 0){
        if (i == 0) {
          kd -= list_Hopfengaben[i]->getKochzeit();
        }
        else {
          kd = list_Hopfengaben[i-1]->getKochzeit() - list_Hopfengaben[i]->getKochzeit();
        }
        s += trUtf8("Nach ") + QString::number(kd) + trUtf8(" min ");
      }
      s += GetWertString(list_Hopfengaben[i]->getErgMenge()) + trUtf8("g ");
      s += list_Hopfengaben[i]->getName() + trUtf8(" Hopfen untermischen");
      s += "</p></td>";
      s += "</tr>";
      if (kkd > list_Hopfengaben[i]->getKochzeit()) {
        kkd = list_Hopfengaben[i]->getKochzeit();
      }
    }
  }
  kd = spinBox_Gesammtkochdauer -> value() - kkd;

  //Fertigkochen
  if (spinBox_Gesammtkochdauer -> value() - kd > 0) {
    s += "<tr>";
    s += "<td class='r' align=center><p>";
    s += trUtf8("Kochen");
    s += "</p></td>";
    s += "<td class='r' align=center><p>";
    s += trUtf8("Noch ") + GetWertString(spinBox_Gesammtkochdauer -> value() - kd) + trUtf8("min weiter kochen");;
    s += "</p></td>";
    s += "</tr>";
  }

  //Weitere Zutaten beim Kochen
  bool KochenVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn){
      KochenVorhanden = true;
    }
  }
  if (KochenVorhanden) {
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn){
        s += "<tr>";
        s += "<td class='r' align=center><p>";
        s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_"
            + QString::number(list_EwZutat[i] -> getTyp()) + "_50.png' alt='Honig' width='30px' border=0>";
        s += "</p></td>";
        s += "<td class='r' align=center><p>";
        s += list_EwZutat[i] -> getName() + " ";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<span class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</span>";
        else
          s += "<span class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</span>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<span>" + trUtf8(" kg") + "</span>";
        else
          s += "<span>" + trUtf8(" g") + "</span>";
        s += "<br>";
        s += list_EwZutat[i] -> getBemerkung();
        s += "</p></td>";
        s += "</tr>";
      }
    }
  }

  //Menge Eintragen Würzemenge zur Berechnung der Sudhausausbeute
  s += "<tr>";
  s += "<td class='we' align=center><p>";
  s += trUtf8("Würzemenge eintragen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würzemenge ermitteln und im Tab (Brau & Gärdaten) in das Eingabefeld (Würzemenge vor dem Hopfenseihen) eintragen");
  s += "</p></td>";
  s += "</tr>";

  //Whirlpool
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Whirlpool");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Kocher abschalten, wenn thermische Bewegung abgeflaut ist einen Whirlpool erzeugen");
  s += "</p></td>";
  s += "</tr>";
  //Warten bis Trubkegel gebildet wurde
  s += "<tr>";
  s += "<td class='' align=center><p>";
  //s += trUtf8("");
  s += "</p></td>";
  s += "<td class='' align=center><p>";
  s += trUtf8("ca. 15 min warten bis sich Trubkegel gebildet hat");
  s += "</p></td>";
  s += "</tr>";

  s += "</tbody></table>";
  s += "</div>";

  s += "<div align='center' class='rm' style='margin-top:10px;width:90%'>";
  s += "<p><b>";
  s += trUtf8("Abseihen / Anstellen");
  s += "</b></p>";
  s += "<table cellspacing=0 border=0 width='90%'><tbody>";

  //Hopfenseihen
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Hopfenseihen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würze durch einen Filter in den Gärbehälter abfüllen");
  s += "</p></td>";
  s += "</tr>";

  //Würzemenge nach Kochende Eintragen
  s += "<tr>";
  s += "<td class='we' align=center><p>";
  s += trUtf8("Würzemenge Eintragen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würzemenge ermitteln und im Tab (Brau & Gärdaten) in das Eingabefeld (Würzemenge nach dem Hopfenseihen) eintragen");
  s += "</p></td>";
  s += "</tr>";

  //Stammwürze nach Kochende Eintragen
  s += "<tr>";
  s += "<td class='we' align=center><p>";
  s += trUtf8("Stammwürze Eintragen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Stammwürze messen und im Tab (Brau & Gärdaten) in das Eingabefeld (Stammwürze nach dem Hopfenseihen) eintragen");
  s += "</p></td>";
  s += "</tr>";

  //Verdünnen
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Verdünnen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Wenn gewünscht die Würze mit der berechneten Menge Wasser auf Sollstammwürze verdünnen");
  s += "</p></td>";
  s += "</tr>";

  //Speise abfüllen
  if (spinBox_Speisemenge -> value() > 0.04){
    s += "<tr>";
    s += "<td class='r' align=center><p>";
    s += trUtf8("Speise abfüllen");
    s += "</p></td>";
    s += "<td class='r' align=center><p>";
    s += QString::number(spinBox_Speisemenge -> value()) + trUtf8(" Liter Speise abfüllen");
    s += "</p></td>";
    s += "</tr>";
  }

  //Abkühlen
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Abkühlen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würze auf Anstelltemperatur bringen.");
  s += "</p></td>";
  s += "</tr>";

  //Würzemenge beim Anstellen Eintragen
  s += "<tr>";
  s += "<td class='we' align=center><p>";
  s += trUtf8("Würzemenge Eintragen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würzemenge ermitteln und im Tab (Brau & Gärdaten) in das Eingabefeld (Würzemenge beim Anstellen) eintragen");
  s += "</p></td>";
  s += "</tr>";

  //Stammwürze beim Anstellen Eintragen
  s += "<tr>";
  s += "<td class='we' align=center><p>";
  s += trUtf8("Stammwürze Eintragen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Stammwürze messen und im Tab (Brau & Gärdaten) in das Eingabefeld (Stammwürze beim Anstellen) eintragen");
  s += "</p></td>";
  s += "</tr>";

  //Hefe zugeben
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Hefe zugeben");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würze mit dem Rührer gut durchlüften und die Hefe unterrühren.");
  s += "</p></td>";
  s += "</tr>";

  //feddich
  s += "<tr>";
  s += "<td class='' align=center colspan=2><p>";
  s += trUtf8("Sud als gebraut markieren, zurücklehnen und ein Bier genießen (am besten ein Selbstgebrautes)");
  s += "</p></td>";
  s += "</tr>";

  s += "</tbody></table>";
  s += "</div>";

  //Weitere Zutaten bei der Gärung
  bool GaerungVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung){
      GaerungVorhanden = true;
    }
  }
  if (GaerungVorhanden) {
    s += "<div align='center' class='rm' style='margin-top:10px;width:90%'>";
    s += "<p><b>";
    s += trUtf8("Bei der Gärung");
    s += "</b></p>";
    s += "<table cellspacing=0 border=0 width='90%'><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung){
        s += "<tr>";
        s += "<td class='r' align=center><p>";
        s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_"
            + QString::number(list_EwZutat[i] -> getTyp()) + "_50.png' alt='Honig' width='30px' border=0>";
        s += "</p></td>";
        s += "<td class='r' align=center><p>";
        s += list_EwZutat[i] -> getName() + " ";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<span class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</span>";
        else
          s += "<span class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</span>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<span>" + trUtf8(" kg") + "</span>";
        else
          s += "<span>" + trUtf8(" g") + "</span>";
        s += "<br>";
        s += list_EwZutat[i] -> getBemerkung();
        s += "</p></td>";
        s += "</tr>";
      }
    }
    s += "</tbody></table>";
    s += "</div>";
  }

  s += "<div><p class='version'>" APP_NAME " v";
  s += VERSION;
  s += "</p></div>";

  seite += s;
  //Seitenende
  ende = "</body></html>";
  seite += ende;

  //textEdit -> setPlainText(seite);

  webView_Zusammenfassung -> setHtml(seite,QUrl::fromLocalFile(QCoreApplication::applicationDirPath()+"/"));
}
