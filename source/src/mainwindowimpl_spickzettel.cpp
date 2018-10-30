#include "mainwindowimpl.h"

#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include "definitionen.h"
#include "errormessage.h"
#include <QFile>
#include "mustache.h"

QString MainWindowImpl::GetWertString(double value)
{
  return "<span class='value'>" + QString::number(value) + "</span>";
}

void MainWindowImpl::ErstelleSpickzettel()
{
  QString _seite;

  QString s;
  QVariantHash contextVariables;
  contextVariables["AppName"] = APP_NAME;
  contextVariables["AppVersion"] = VERSION;
  contextVariables["Style"] = StyleDunkel ? "style_dunkel.css" : "style_hell.css";
  contextVariables["Sudname"] = lineEdit_Sudname->text();

  // Rezept
  s = "<table><tbody>";
  s += "<tr>";
  s += "<td>" + trUtf8("Menge") + "</td>";
  s += "<td class='value' align='right'>" + spinBox_Menge -> text() + "</td>";
  s += "<td>" + trUtf8("Liter") + "</td>";
  s += "</tr>";
  s += "<tr>";
  s += "<td>" + trUtf8("Stammwürze") + "</td>";
  s += "<td class='value' align='right'>" + spinBox_SW -> text() + "</td>";
  s += "<td>" + trUtf8("°P") + "</td>";
  s += "</tr>";
  if (spinBox_High_Gravity->value() > 0) {
    s += "<tr>";
    s += "<td>" + trUtf8("High-Gravity-Faktor") + "</td>";
    s += "<td class='value' align='right'>" + spinBox_High_Gravity -> text() + "</td>";
    s += "<td>" + trUtf8("%") + "</td>";
    s += "</tr>";
  }
  s += "<tr>";
  s += "<td>" + trUtf8("Bittere") + "</td>";
  s += "<td class='value' align='right'>" + spinBox_IBU -> text() + "</td>";
  s += "<td>" + trUtf8("IBU") + "</td>";
  s += "</tr>";
  s += "<tr>";
  s += "<td>" + trUtf8("Nachisomerisierungszeit") + "</td>";
  s += "<td class='value' align='right'>" + spinBox_NachisomerisierungsZeit -> text() + "</td>";
  s += "<td>" + trUtf8("min") + "</td>";
  s += "</tr>";
  s += "<tr>";
  s += "<td>" + trUtf8("Farbe") + "</td>";
  s += "<td class='value' align='right'>" + doubleSpinBox_EBC -> text() + "</td>";
  s += "<td>" + trUtf8("EBC") + "</td>";
  s += "</tr>";
  s += "<tr>";
  s += "<td>" + trUtf8("CO₂-Gehalt") + "</td>";
  s += "<td class='value' align='right'>" + doubleSpinBox_CO2 -> text() + "</td>";
  s += "<td>" + trUtf8("g/l") + "</td>";
  s += "</tr>";
  s += "<tr>";
  s += "<td>" + trUtf8("Brauanlage") + "</td>";
  s += "<td colspan=2  class='value' align='center'>" + comboBox_AuswahlBrauanlage->currentText() + "</td>";
  s += "</tr>";
  s += "</tbody></table>";
  contextVariables["Rezept"] = s;

  // Glas
  QColor farbe = Berechnungen.GetFarbwert(doubleSpinBox_EBC -> value());
  s = "<div style='background-color:" + farbe.name() + ";padding:0px;margin:0px;width:100%;height:100%'>";
  if (StyleDunkel)
    s += "<img style='padding:0px;margin:0px;width:100%;height:100%' src='qrc:/global/bier_dark_420x420.png' alt='Bierfarbe'>";
  else
    s += "<img style='padding:0px;margin:0px;width:100%;height:100%' src='qrc:/global/bier_420x420.png' alt='Bierfarbe'>";
  s += "</div>";
  contextVariables["Glas"] = s;

  // Malz
  double fehlprozent = list_Malzgaben.count() > 0 ? list_Malzgaben[0]->getFehlProzent() : 0.0;
  if (fehlprozent == 0.0) {
    s = "<table><tbody>";
    for (int i=0; i < list_Malzgaben.count(); i++){
      s += "<tr>";
      s += "<td>" + list_Malzgaben[i]->getName() + "</td>";
      s += "<td class='value' align='right'>" + QString::number(list_Malzgaben[i]->getErgMenge()) + "</td>";
      s += "<td>" + trUtf8("kg") + "</td>";
      s += "<td class='value' align='right'>" + QString::number(list_Malzgaben[i]->getMengeProzent()) + "</td>";
      s += "<td>" + trUtf8("%") + "</td>";
      s += "<td class='value' align='right'>" + QString::number(list_Malzgaben[i]->getFarbe()) + "</td>";
      s += "<td>" + trUtf8("EBC") + "</td>";
      s += "</tr>";
    }
    s += "<tr>";
    s += "<td>" + trUtf8("Gesamt") + "</td>";
    s += "<td class='value' style='font-weight:bold;' align='right'>" + QString::number(doubleSpinBox_S_Gesammt -> value()) + "</td>";
    s += "<td>" + trUtf8("kg") + "</td>";
    s += "</tr>";
    s += "</tbody></table>";
  }
  else {
    s = "<div class='hinweis'>" + trUtf8("Die einzelnen Schüttungen konnten nicht richtig berechnet werden, da die Aufteilung nicht 100% entspricht.") + "</div>";
  }
  contextVariables["Malz"] = s;

  // Hopfen
  fehlprozent = fehlprozent = list_Hopfengaben.count() > 0 ? list_Hopfengaben[0]->getFehlProzent() : 0.0;
  if (fehlprozent == 0.0) {
    s = "<table><tbody>";
    for (int i=0; i < list_Hopfengaben.count(); i++){
      s += "<tr>";
      if (list_Hopfengaben[i]->getVWH())
        s += "<td>" + trUtf8("VWH ") + list_Hopfengaben[i]->getErgebnistext() + "</td>";
      else
        s += "<td>" + list_Hopfengaben[i]->getErgebnistext() + "</td>";
      s += "<td class='value' align='right'>" + QString::number(list_Hopfengaben[i]->getErgMenge()) + "</td>";
      s += "<td>" + trUtf8("g") + "</td>";
      s += "<td class='value' align='right'>" + QString::number(list_Hopfengaben[i]->getKochzeit()) + "</td>";
      s += "<td>" + trUtf8("min") + "</td>";
      s += "</tr>";
    }
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Hopfen){
        s += "<tr>";
        s += "<td>" + list_EwZutat[i] -> getName() + "</td>";
        s += "<td class='value' align='right'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</td>";
        s += "<td>" + trUtf8("g") + "</td>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<td class='value' align='right' colspan='2'>" + trUtf8("Gärung") + "</td>";
        else
          s += "<td class='value' align='right' colspan='2'>" + trUtf8("Anstellen") + "</td>";
        s += "</tr>";
        if (list_EwZutat[i]->getBemerkung() != "") {
          s += "<tr>";
          s += "<td colspan='5' class='kommentar'>" + list_EwZutat[i]->getBemerkung().replace("\n","<br>") + "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</tbody></table>";
  }
  else {
    s = "<div class='hinweis'>" + trUtf8("Die einzelnen Hopfengaben konnten nicht richtig berechnet werden, da die Aufteilung nicht 100% entspricht.")+"</div>";
  }
  contextVariables["Hopfen"] = s;

  // Wasser
  s = "<table><tbody>";
  s += "<tr>";
  s += "<td>" + trUtf8("Hauptguss") + "</td>";
  s += "<td class='value' align='right'>" + doubleSpinBox_WHauptguss -> text() + "</td>";
  s += "<td>" + trUtf8("Liter") + "</td>";
  s += "</tr>";
  s += "<tr>";
  s += "<td>" + trUtf8("Milchsäure (80%)") + "</td>";
  s += "<td class='value' align='right'>" + SpinBox_waMilchsaeureHG_ml -> text() + "</td>";
  s += "<td>" + trUtf8("ml") + "</td>";
  s += "</tr>";
  s += "<tr>";
  s += "<td>" + trUtf8("Nachguss") + "</td>";
  s += "<td class='value' align='right'>" + doubleSpinBox_WNachguss -> text() + "</td>";
  s += "<td>" + trUtf8("Liter") + "</td>";
  s += "</tr>";
  s += "<tr>";
  s += "<td>" + trUtf8("Milchsäure (80%)") + "</td>";
  s += "<td class='value' align='right'>" + SpinBox_waMilchsaeureNG_ml -> text() + "</td>";
  s += "<td>" + trUtf8("ml") + "</td>";
  s += "</tr>";
  s += "<tr>";
  s += "<td>" + trUtf8("Gesamt") + "</td>";
  s += "<td class='value' style='font-weight:bold;' align='right'>" + doubleSpinBox_W_Gesammt -> text() + "</td>";
  s += "<td>" + trUtf8("Liter") + "</td>";
  s += "</tr>";
  s += "</tbody></table>";
  contextVariables["Wasser"] = s;

  // Hefe
  s = "<p>" + comboBox_AuswahlHefe -> currentText() + "</p>";
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
  s += "<p>" + sEinheiten + "</p>";
  contextVariables["Hefe"] = s;

  //Honig
  bool HonigVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Honig){
      HonigVorhanden = true;
    }
  }
  if (HonigVorhanden){
    s = "<table><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Honig){
        s += "<tr>";
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
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4' class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</table>";
  }
  else {
    s = "";
  }
  contextVariables["Honig"] = s;

  //Zucker
  bool ZuckerVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Zucker){
      ZuckerVorhanden = true;
    }
  }
  if (ZuckerVorhanden){
    s = "<table><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Zucker){
        s += "<tr>";
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
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4' class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</table>";
  }
  else {
    s = "";
  }
  contextVariables["Zucker"] = s;

  //Gewürz
  bool GewuerzVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Gewuerz){
      GewuerzVorhanden = true;
    }
  }
  if (GewuerzVorhanden){
    s = "<table><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Gewuerz){
        s += "<tr>";
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
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4' class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</table>";
  }
  else {
    s = "";
  }
  contextVariables["Gewuerz"] = s;

  //Frucht
  bool FruchtVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Frucht){
      FruchtVorhanden = true;
    }
  }
  if (FruchtVorhanden){
    s = "<table><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Frucht){
        s += "<tr>";
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
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4' class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</table>";
  }
  else {
    s = "";
  }
  contextVariables["Frucht"] = s;

  //Sonstiges
  bool SonstigesVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Sonstiges){
      SonstigesVorhanden = true;
    }
  }
  if (SonstigesVorhanden){
    s = "<table><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Sonstiges){
        s += "<tr>";
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
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4' class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</table>";
  }
  else {
    s = "";
  }
  contextVariables["Sonstiges"] = s;

  // Kommentar
  contextVariables["Kommentar"] = textEdit_Kommentar->toPlainText();


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
                                + trUtf8("\nSQL-Befehl:\n") + sql);
  }
  else {
    if (query.first()) {
      contextVariables["GeraeteTitel"] = trUtf8("Benötigte Gerätschaften");
      s = "<table><tbody>";
      s += "<tr><td align=center>" + query.value(0).toString() + "</td></tr>";
      while (query.next())
        s += "<tr><td align=center>" + query.value(0).toString() + "</td></tr>";
      s += "</tbody></table>";
      contextVariables["Geraete"] = s;
    }
  }

  //Brauablauf
  //-------------------------------------------------------------
  contextVariables["MaischenTitel"] = trUtf8("Maischen");
  s = "<table width='90%'><tbody>";

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
        s += "<br><span class='kommentar'>";
        s += list_EwZutat[i] -> getBemerkung();
        s += "</span></p></td>";
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
  contextVariables["Maischen"] = s;

  contextVariables["LaeuternTitel"] = trUtf8("Läutern");
  s = "<table width='90%'><tbody>";

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
  s += trUtf8("Nach etwa 10-20 min Wartezeit solange Vorderwürze ablaufen lassen und wieder zurück in den Läutereimer schütten, bis Würze klar läuft.");
  s += "</p></td>";
  s += "</tr>";

  //Läutern
  s += "<tr>";
  s += "<td class='' align=center><p>";
  s += trUtf8("Läutern");
  s += "</p></td>";
  s += "<td class='' align=center><p>";
  s += trUtf8("Würze langsam ablaufen lassen (Treber darf nicht trockenlaufen!). Bevor Treber trockenläuft immer etwas (ca. 2-3 Liter) Nachguss über Schaumlöffel nachgießen.");
  s += "</p></td>";
  s += "</tr>";


  s += "</tbody></table>";
  contextVariables["Laeutern"] = s;

  contextVariables["KochenTitel"] = trUtf8("Würze kochen");
  s = "<table width='90%'><tbody>";

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
        s += "<br><span class='kommentar'>";
        s += list_EwZutat[i] -> getBemerkung();
        s += "</span></p></td>";
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
  s += trUtf8("Kocher abschalten. Wenn thermische Bewegung abgeflaut ist, einen Whirlpool erzeugen");
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
  contextVariables["Kochen"] = s;

  contextVariables["AnstellenTitel"] = trUtf8("Abseihen & Anstellen");
  s = "<table width='90%'><tbody>";

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
  s += trUtf8("Würzemenge eintragen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würzemenge ermitteln und im Tab (Brau & Gärdaten) in das Eingabefeld (Würzemenge nach dem Hopfenseihen) eintragen");
  s += "</p></td>";
  s += "</tr>";

  //Stammwürze nach Kochende Eintragen
  s += "<tr>";
  s += "<td class='we' align=center><p>";
  s += trUtf8("Stammwürze eintragen");
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
  s += trUtf8("Wenn gewünscht, die Würze mit der berechneten Menge Wasser auf Sollstammwürze verdünnen");
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
  s += trUtf8("Würzemenge eintragen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würzemenge ermitteln und im Tab (Brau & Gärdaten) in das Eingabefeld (Würzemenge beim Anstellen) eintragen");
  s += "</p></td>";
  s += "</tr>";

  //Stammwürze beim Anstellen Eintragen
  s += "<tr>";
  s += "<td class='we' align=center><p>";
  s += trUtf8("Stammwürze eintragen");
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
  contextVariables["Anstellen"] = s;

  //Weitere Zutaten bei der Gärung
  bool GaerungVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung){
      GaerungVorhanden = true;
    }
  }
  if (GaerungVorhanden) {
    contextVariables["GaerungTitel"] = trUtf8("Bei der Gärung");
    s = "</b></p>";
    s += "<table width='90%'><tbody>";
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
        s += "<br><span class='kommentar'>";
        s += list_EwZutat[i] -> getBemerkung();
        s += "</span></p></td>";
        s += "</tr>";
      }
    }
    s += "</tbody></table>";
    contextVariables["Gaerung"] = s;
  }


  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  QString settingsPath = QFileInfo(settings.fileName()).absolutePath() + "/";

  QFile file(settingsPath + "spickzettel.html");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      return;
  QString html_template = file.readAll();
  file.close();

  Mustache::Renderer renderer;
  Mustache::QtVariantContext context(contextVariables);
  QString seite = renderer.render(html_template, &context);
  if (webView_Zusammenfassung->url().isEmpty())
      MyWebView::clearMemoryCaches();
  webView_Zusammenfassung->setHtml(seite, QUrl::fromLocalFile(settingsPath));
  webView_Zusammenfassung->setZoomFactor(horizontalSlider_ScalePDF->value() / 100.0);
}
