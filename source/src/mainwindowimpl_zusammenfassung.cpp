#include "mainwindowimpl.h"

#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include "definitionen.h"
#include "errormessage.h"

void MainWindowImpl::ErstelleZusammenfassung()
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
  //Style für Kommentar
  style += "p.kommentar{color:#555555;font-size:10pt;margin-bottom:5px;margin-left:5px;}";
  //Style für Div Box ohne Rahmen
  style += "p.zugegeben{color:#555555;font-size:10pt;margin-bottom:2px;margin-left:5px;}";
  //Style für Div Box ohne Rahmen
  style += "div.r{border:0px solid #dddddd; border-radius: 10px; padding:5px;background-color:#dddddd;}";
  //Style für Div Box mit Rahmen
  style += "div.rm{border:2px solid #dddddd; border-radius: 10px; padding:5px;background-color:#ffffff;}";
  //Style für Tabelle
  style += "td{padding:2px;margin:0px;font-size:10pt;}";
  style += "td.r{padding:2px;margin:0px;border-bottom-color:#dddddd;border-bottom-style:solid;border-width:1px;}";
  //Style für Hinweis Wert in Brau und Gärdaten eintragen
  style += "td.we{background-color: #eba328;}";
  style += "tr{padding:0px;margin:0px;}";
  style += "</style>";
  kopf += style;
  kopf += "</head><body align='center' style='font-family:Ubuntu,Arial; font-size:10pt; font-style:normal;background-color:#fff;'>";
  seite = kopf;

  QString s = "";

  //Tabelle für Bild und Zutaten
  s += "<div class='' width='99%' style='' align='center'>";

  s += "<div class='rm' style='margin:0px;margin-bottom:5px;width:60%;' align='center'>";
  s += "<table style='width:99%;' cellspacing=0 border=0><tbody>";
  s += "<tr style=''>";
  s += "<td>";
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
  s += "<td>";
  s += "<p class='value'>" + QString::number(spinBox_BiermengeAbfuellen -> value()) + "</p>";
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
  s += "<td>";
  s += "<p class='value'>" + QString::number(spinBox_SWSollGesammt -> value()) + "</p>";
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
    s += "<td>";
    s += "<p class='value'>" + QString::number(spinBox_High_Gravity -> value()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("%") + "</p>";
    s += "</td>";
    s += "</tr>";
  }
  //Alkoholgehalt
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Alkoholgehalt") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + QString::number(spinBox_AlkoholVol -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("%vol") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Bittere
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Bittere") + "</p>";
  s += "</td>";
  s += "<td>";
  //Bittere anhand der Ausschlagmenge Berechnen
  double ibu = spinBox_IBU -> value() * highGravityFaktor;
  //wenn abgefüllt wurde mit Soll Ist Biermenge verrechnen
  if (BierWurdeAbgefuellt) {
    ibu = spinBox_MengeSollNachHopfenseihen -> value() / spinBox_BiermengeAbfuellen -> value() * ibu;
  }
  //wenn gebraut wurde mit würze menge beim Anstellen berechnen
  else if (BierWurdeGebraut) {
    ibu = spinBox_MengeSollNachHopfenseihen -> value() / spinBox_WuerzemengeAnstellen->value() * ibu;
  }

  ibu = qRound(ibu*10);
  ibu = ibu/10;
  s += "<p class='value'>" + QString::number(ibu) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("IBU") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Farbe
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Farbe") + "</p>";
  s += "</td>";
  s += "<td>";
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
  s += "<td>";
  //Bei mehr als einem Eintrag im Nachgärverlauf wird der CO2 Gehalt aus dem
  //Nachgärverlauf entnommen ansonsten der Teoretische
  double d = doubleSpinBox_CO2 -> value();
  QString sqlN = "SELECT * FROM Nachgaerverlauf WHERE SudID="
      + QString::number(AktuelleSudID) + " ORDER BY Zeitstempel DESC;";
  QSqlQuery queryN;
  if (!queryN.exec(sqlN)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + queryN.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sqlN);
  }
  else {
    if (queryN.first()){
      int FeldNr = queryN.record().indexOf("Druck");
      //Wert nur Übernehmen wenn der Druck größer 0 ist
      //Ansonsten ist davon auszugehen das der Eintrag der automatisch angelegete ist
      if (queryN.value(FeldNr).toDouble() > 0){
        FeldNr = queryN.record().indexOf("CO2");
        d = queryN.value(FeldNr).toDouble();
      }
    }
  }
  s += "<p class='value'>" + QString::number(d) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("g/Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  if (BierWurdeAbgefuellt){
    //Scheinbarer Endvergärungsgrad
    s += "<tr style=''>";
    s += "<td>";
    s += "<p>" + trUtf8("scheinbarer Endvergärungsgrad") + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p class='value'>" + QString::number(spinBox_SEVG -> value()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("%") + "</p>";
    s += "</td>";
    s += "</tr>";
  }
  //Effektive Sudhausausbeute
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("effektive Sudhausausbeute") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + QString::number(spinBox_AusbeuteEffektiv2 -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("%") + "</p>";
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
  //Kosten pro Liter
  //s += "<tr style=''>";
  //s += "<td>";
  //s += "<p>" + trUtf8("Kosten") + "</p>";
  //s += "</td>";
  //s += "<td>";
  //s += "<p class='value'>" + QString::number(spinBox_Preis -> value()) + "</p>";
  //s += "</td>";
  //s += "<td>";
  //s += "<p>" + trUtf8("€/Liter") + "</p>";
  //s += "</td>";
  //s += "</tr>";
  s += "</tbody></table>";
  s += "</td>";
  //Datumsangaben
  s += "<td style='vertical-align:top;' align=right>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Braudatum
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Braudatum") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + dateEdit_Braudatum -> date().toString("dd.MM.yyyy") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Abfülldatum
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Abfülldatum") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + dateEdit_Abfuelldatum -> date().toString("dd.MM.yyyy") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Datum der Angepeilten Reifezeit
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Angepeiltes Reifezeitende") + "</p>";
  s += "</td>";
  s += "<td>";
  //Start der Reifung ermitteln indem das letzte Datum vom
  //Nachgärverlauf benutzt wird
  sqlN = "SELECT * FROM Nachgaerverlauf WHERE SudID="
      + QString::number(AktuelleSudID) + " ORDER BY Zeitstempel DESC;";
  QDate date = dateEdit_Abfuelldatum -> date();
  if (!queryN.exec(sqlN)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + queryN.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sqlN);
  }
  else {
    if (queryN.first()){
      int FeldNr = queryN.record().indexOf("Zeitstempel");
      date = QDate::fromString(queryN.value(FeldNr).toString(),Qt::ISODate);
    }
  }
  date = date.addDays(spinBox_Reifezeit -> value() * 7);
  s += "<p class='value'>" + date.toString("dd.MM.yyyy") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Beste Bewertung
  int bew=0;
  QString bewtext="";
  if (list_Bewertung.count() > 0){
    for (int i=0; i<list_Bewertung.count(); i++){
      int b = list_Bewertung[i]->getSterne();
      if (b > bew){
        bew = b;
        bewtext = QString::number(list_Bewertung[i]->getWoche())+". Woche ";
      }
    }
    s += "<tr style=''>";
    s += "<td>";
    s += "<p>" + trUtf8("Beste Bewertung") + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p class='value'>" + bewtext + "</p>";
    s += "</td>";
    s += "</tr>";
    s += "<tr style=''>";
    s += "<td colspan=2>";
    s += "<div class='bew' style='' align='center'>";
    for (int i = 0; i<bew; i++){
      s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_24.png' width='24' border=0>";
    }
    for (int i = bew; i<MaxAnzahlSterne; i++){
      s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_gr_24.png' width='24' border=0>";
    }
    s += "</div>";
    s += "</td>";
    s += "</tr>";
  }

  s += "</tbody></table>";
  s += "</td>";
  s += "</tr>";
  s += "</tbody></table>";
  s += "</div>";

  s += "<table width='90%' summary='tabelle' border='0' cellspacing='3'>";
  s += "<tr style=''>";
  s += "<td valign='bottom' style=''>";
  //Schüttung
  s += "<div class='rm' style='margin:0px;margin-bottom:5px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/getreide_300.png' alt='Getreide' width='300px' border=0>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Alle Malzgaben
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
      //Zugabezeitpunkt
      if (list_EwZutat[i]->getZugabestatus() > EWZ_Zugabestatus_nichtZugegeben) {
        s += "<tr style=''>";
        s += "<td colspan='5'>";
        s += "<p class='zugegeben'>Zugegeben am "+ list_EwZutat[i]->getZugabezeitpunkt_von().toString("dd.MM.yyyy")+"</p>";
        //wenn entnahme
        if (list_EwZutat[i]->getEntnahmeindex() == EWZ_Entnahmeindex_MitEntnahme) {
          //wenn entnommen
          if (list_EwZutat[i]->getZugabestatus() == EWZ_Zugabestatus_Entnommen) {
            s += "<p class='kommentar'>"+ trUtf8("Entnommen am ")+ list_EwZutat[i]->getZugabezeitpunkt_bis().toString("dd.MM.yyyy")
                + " (" + trUtf8("Tage: ") +
                QString::number(list_EwZutat[i]->getDauerMinuten()/1440)+")</p>";
            //wenn entnahme
          }
        }
        s += "</td>";
        s += "</tr>";
      }
      //wenn ein Kommentar vorhanden ist eine Zeile für den Komentar einfügen
      if (list_EwZutat[i]->getBemerkung() != "") {
        s += "<tr style=''>";
        s += "<td colspan='5'>";
        s += "<p class='kommentar'>"+ list_EwZutat[i]->getBemerkung().replace("\n","<br>")+"</p>";
        s += "</td>";
        s += "</tr>";
      }
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
        //Wenn vorhanden Kommentar anzeigen
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4'>";
          s += "<p class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</p>";
          s += "</td>";
          s += "</tr>";
        }
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
        //Wenn vorhanden Kommentar anzeigen
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4'>";
          s += "<p class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</p>";
          s += "</td>";
          s += "</tr>";
        }
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
          s += "<p class='value'>" + trUtf8("Kochen") + " (" + QString::number(list_EwZutat[i]->getDauerMinuten()) + "min) </p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
        //Wenn vorhanden Kommentar anzeigen
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4'>";
          s += "<p class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</p>";
          s += "</td>";
          s += "</tr>";
        }
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
          s += "<p class='value'>" + trUtf8("Kochen") + " (" + QString::number(list_EwZutat[i]->getDauerMinuten()) + "min) </p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
        //Wenn vorhanden Kommentar anzeigen
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4'>";
          s += "<p class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</p>";
          s += "</td>";
          s += "</tr>";
        }
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
          s += "<p class='value'>" + trUtf8("Kochen") + " (" + QString::number(list_EwZutat[i]->getDauerMinuten()) + "min) </p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
        //Wenn vorhanden Kommentar anzeigen
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4'>";
          s += "<p class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</p>";
          s += "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  s += "</tr >";
  s += "</table>";

  //Kommentar
  if (textEdit_Kommentar -> toPlainText() != ""){
    s += "<div class='rm' style='margin:10px;width:80%;'>";
    s += textEdit_Kommentar -> toHtml();
    s += "</div>";
  }


  //Maischen
  s += "<div div class='rm' style='width:80%;'>";
  s += "<p class='h2'>" + trUtf8("Maischen:") + "</p>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Einmaischen
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Einmaischen bei") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + QString::number(spinBox_EinmaischenTemp -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("°C") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Rasten in Zusammenfassung eintragen
  for (int i=0; i<list_Rasten.count();i++){
    s += "<tr style=''>";
    s += "<td>";
    s += "<p>" + list_Rasten[i]->getRastName() + trUtf8(" bei ") + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p class='value'>" + QString::number(list_Rasten[i]->getRastTemp()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("°C") + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p class='value'>" + QString::number(list_Rasten[i]->getRastDauer()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("min") + "</p>";
    s += "</td>";
    s += "</tr>";
  }
  s += "</tbody></table>";
  s += "</div>";
  //Bewertungen
  //asdf

  //Gärverlauf
  //FuelleGaerverlauf();
  //widget_DiaSchnellgaerverlauf -> BildSpeichern("svg");
  //s += "<div div class='r' style='width:99%;float:left;margin-top:10px;'>";
  //s += "<p class='h2'>" + trUtf8("Gärverlauf:") + "</p>";
  //s += "<p>" + trUtf8("Schnellvergärprobe:") + "</p>";
  //s += "<img src='svg.png' style='width:98%'>";
  //s += "</div>";
  s += "";
  s += "";
  s += "";
  s += "";
  s += "";
  s += "";
  s += "";
  s += "";
  s += "";
  //s += QUrl::fromLocalFile(QCoreApplication::applicationDirPath()).toString();

  //Anhänge
  if (list_Anhang.count() > 0) {
    s += "</br>";
    s += "<div div class='rm' style='width:80%;'>";
    s += "<p class='h2'>" + trUtf8("Anhänge:") + "</p>";
    for (int i=0; i<list_Anhang.count();i++){
      if (AnhangWidget::isImage(list_Anhang[i]->getPfad()))
        s += "<img style=\"max-width:80%;\" src=\"file:///" + list_Anhang[i]->getFullPfad() + "\"></br></br>";
      else
        s += "<a href=\"file:///" + list_Anhang[i]->getFullPfad() + "\" target=\"_blank\">" + list_Anhang[i]->getPfad() + "</a></br></br>";
    }
    s += "</div>";
  }

  s += "<div><p class='version'>" APP_NAME " v";
  s += VERSION;
  s += "</p></div>";

  seite += s;
  //Seitenende
  ende = "</body></html>";
  seite += ende;

  webView_Zusammenfassung -> setHtml(seite,QUrl::fromLocalFile(QCoreApplication::applicationDirPath()+"/"));
}
