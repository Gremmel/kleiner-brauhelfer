#include "mainwindowimpl.h"

#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include "definitionen.h"
#include "errormessage.h"
#include <QFile>
#include "mustache.h"

void MainWindowImpl::ErstelleZusammenfassung()
{
    QString s;
    QVariantHash contextVariables;
    ErstelleTagListe(contextVariables);

    double menge= 0.0;
    double mengeFaktor = 1.0;
    if (BierWurdeAbgefuellt) {
      menge = spinBox_BiermengeAbfuellen->value();
      mengeFaktor = spinBox_MengeSollNachHopfenseihen->value() / spinBox_BiermengeAbfuellen->value() * highGravityFaktor;
    }
    else if (BierWurdeGebraut) {
      menge = spinBox_WuerzemengeAnstellen->value();
      mengeFaktor = spinBox_MengeSollNachHopfenseihen->value() / spinBox_WuerzemengeAnstellen->value() * highGravityFaktor;
    }

    // SudInfo1
    s = "<table><tbody>";
    //Menge
    s += "<tr>";
    s += "<td>" + trUtf8("Menge") + "</td>";
    s += "<td class='value' align='right'>" + QString::number(menge, 'f', spinBox_BiermengeAbfuellen->decimals()) + "</td>";
    s += "<td align='right'>(" + spinBox_Menge->text() + ")</td>";
    s += "<td>" + trUtf8("Liter") + "</td>";
    s += "</tr>";
    //Stammwürze
    s += "<tr>";
    s += "<td>" + trUtf8("Stammwürze") + "</td>";
    s += "<td class='value' align='right'>" + spinBox_SWSollGesammt->text() + "</td>";
    s += "<td align='right'>(" + spinBox_SW->text() + ")</td>";
    s += "<td>" + trUtf8("°P") + "</td>";
    s += "</tr>";
    //High-Gravity-Faktor
    if (spinBox_High_Gravity->value() > 0) {
      s += "<tr>";
      s += "<td>" + trUtf8("High-Gravity-Faktor") + "</td>";
      s += "<td class='value' align='right'>" + spinBox_High_Gravity->text() + "</td>";
      s += "<td align='right'></td>";
      s += "<td>" + trUtf8("%") + "</td>";
      s += "</tr>";
    }
    //Alkoholgehalt
    s += "<tr>";
    s += "<td>" + trUtf8("Alkoholgehalt") + "</td>";
    s += "<td class='value' align='right'>" + spinBox_AlkoholVol->text() + "</td>";
    s += "<td align='right'></td>";
    s += "<td>" + trUtf8("Vol.%") + "</td>";
    s += "</tr>";
    //Bittere
    s += "<tr>";
    s += "<td>" + trUtf8("Bittere") + "</td>";
    s += "<td class='value' align='right'>" + QString::number(spinBox_IBU->value() * mengeFaktor, 'f', 0) + "</td>";
    s += "<td align='right'>(" + spinBox_IBU->text() + ")</td>";
    s += "<td>" + trUtf8("IBU") + "</td>";
    s += "</tr>";
    //Farbe
    s += "<tr>";
    s += "<td>" + trUtf8("Farbe") + "</td>";
    s += "<td class='value' align='right'>" + QString::number(doubleSpinBox_EBC->value() * mengeFaktor, 'f', doubleSpinBox_EBC->decimals()) + "</td>";
    s += "<td align='right'>(" + doubleSpinBox_EBC->text() + ")</td>";
    s += "<td>" + trUtf8("EBC") + "</td>";
    s += "</tr>";
    //CO₂-Gehalt
    s += "<tr>";
    s += "<td>" + trUtf8("CO₂-Gehalt") + "</td>";
    //Bei mehr als einem Eintrag im Nachgärverlauf wird der CO₂-Gehalt aus dem
    //Nachgärverlauf entnommen ansonsten der Teoretische
    double d = 0.0;
    QSqlQuery queryN;
    queryN.prepare("SELECT * FROM Nachgaerverlauf WHERE SudID=:sudid ORDER BY Zeitstempel DESC;");
    queryN.bindValue(":sudid", AktuelleSudID);
    if (!queryN.exec()) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + queryN.lastError().databaseText()
                                  + trUtf8("\nSQL-Befehl:\n") + queryN.lastQuery());
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
    s += "<td class='value' align='right'>" + QString::number(d, 'f', doubleSpinBox_CO2->decimals()) + "</td>";
    s += "<td align='right'>(" + doubleSpinBox_CO2->text() + ")</td>";
    s += "<td>" + trUtf8("g/l") + "</td>";
    s += "</tr>";
    if (BierWurdeAbgefuellt){
      //Scheinbarer Endvergärungsgrad
      s += "<tr>";
      s += "<td>" + trUtf8("sEVG") + "</td>";
      s += "<td class='value' align='right'>" + spinBox_SEVG->text() + "</td>";
      s += "<td align='right'></td>";
      s += "<td>" + trUtf8("%") + "</td>";
      s += "</tr>";
    }
    //Effektive Sudhausausbeute
    s += "<tr>";
    s += "<td>" + trUtf8("eff. SHA") + "</td>";
    s += "<td class='value' align='right'>" + spinBox_AusbeuteEffektiv2->text() + "</td>";
    s += "<td align='right'></td>";
    s += "<td>" + trUtf8("%") + "</td>";
    s += "</tr>";
    s += "</tbody></table>";
    contextVariables["Sudinfo1"] = s;

    // SudInfo2
    s = "<table><tbody>";
    //Brauanlage
    s += "<tr>";
    s += "<td>" + trUtf8("Brauanlage") + "</td>";
    s += "<td class='value' align='right'>" + comboBox_AuswahlBrauanlage->currentText() + "</td>";
    s += "</tr>";
    //Braudatum
    s += "<tr>";
    s += "<td>" + trUtf8("Braudatum") + "</td>";
    s += "<td class='value' align='right'>" + dateEdit_Braudatum->text() + "</td>";
    s += "</tr>";
    if (BierWurdeAbgefuellt){
        //Abfülldatum
        s += "<tr>";
        s += "<td>" + trUtf8("Abfülldatum") + "</td>";
        s += "<td class='value' align='right'>" + dateEdit_Abfuelldatum->text() + "</td>";
        s += "</tr>";
        //Datum der Angepeilten Reifezeit
        s += "<tr>";
        s += "<td>" + trUtf8("Angepeiltes Reifezeitende") + "</td>";
        //Start der Reifung ermitteln indem das letzte Datum vom
        //Nachgärverlauf benutzt wird
        queryN.prepare("SELECT * FROM Nachgaerverlauf WHERE SudID=:sudid ORDER BY Zeitstempel DESC;");
        queryN.bindValue(":sudid", AktuelleSudID);
        QDate date = dateEdit_Abfuelldatum -> date();
        if (!queryN.exec()) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + queryN.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + queryN.lastQuery());
        }
        else {
          if (queryN.first()){
            int FeldNr = queryN.record().indexOf("Zeitstempel");
            date = QDate::fromString(queryN.value(FeldNr).toString(),Qt::ISODate);
          }
        }
        date = date.addDays(spinBox_Reifezeit -> value() * 7);
        s += "<td class='value' align='right'>" + date.toString("dd.MM.yyyy") + "</td>";
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
          s += "<tr>";
          s += "<td>" + trUtf8("Beste Bewertung") + "</td>";
          s += "<td class='value' align='right'>" + bewtext + "</td>";
          s += "</tr>";
          s += "<tr>";
          s += "<td></td>";
          s += "<td align='right'>";
          for (int i = 0; i<bew; i++){
            s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_24.png' width='24' border=0>";
          }
          for (int i = bew; i<MaxAnzahlSterne; i++){
            s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_gr_24.png' width='24' border=0>";
          }
          s += "</td>";
          s += "</tr>";
        }
    }
    s += "</tbody></table>";
    contextVariables["Sudinfo2"] = s;

    // Glas
    QColor farbe = Berechnungen.GetFarbwert(doubleSpinBox_EBC -> value() * mengeFaktor);
    s = "<div style='background-color:" + farbe.name() + ";padding:0px;margin:0px;width:100%;height:100%'>";
    if (StyleDunkel)
      s += "<img style='padding:0px;margin:0px;width:100%;height:100%' src='qrc:/global/bier_dark_420x420.png' alt='Bierfarbe'>";
    else
      s += "<img style='padding:0px;margin:0px;width:100%;height:100%' src='qrc:/global/bier_420x420.png' alt='Bierfarbe'>";
    s += "</div>";
    contextVariables["Glas"] = s;

    s = "<div style='background-color:" + farbe.name() + ";padding:0px;margin:0px;width:100%;height:100%'>";
    s += "<img style='padding:0px;margin:0px;width:100%;height:100%' src='qrc:/global/bier_420x420.png' alt='Bierfarbe'>";
    s += "</div>";
    contextVariables["Glas-hell"] = s;

    s = "<div style='background-color:" + farbe.name() + ";padding:0px;margin:0px;width:100%;height:100%'>";
    s += "<img style='padding:0px;margin:0px;width:100%;height:100%' src='qrc:/global/bier_dark_420x420.png' alt='Bierfarbe'>";
    s += "</div>";
    contextVariables["Glas-dunkel"] = s;

    // Malz
    double fehlprozent = list_Malzgaben.count() > 0 ? list_Malzgaben[0]->getFehlProzent() : 0.0;
    s = "<table><tbody>";
    for (int i=0; i < list_Malzgaben.count(); i++){
      s += "<tr>";
      s += "<td>" + list_Malzgaben[i]->getName() + "</td>";
      s += "<td class='value' align='right'>" + QString::number(list_Malzgaben[i]->getErgMenge()) + "</td>";
      s += "<td>" + trUtf8("kg") + "</td>";
      if (fehlprozent == 0.0) {
        s += "<td class='value' align='right'>" + QString::number(list_Malzgaben[i]->getMengeProzent()) + "</td>";
        s += "<td>" + trUtf8("%") + "</td>";
      }
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
   contextVariables["Malz"] = s;

    // Hopfen
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
        if (list_EwZutat[i]->getZugabestatus() > EWZ_Zugabestatus_nichtZugegeben) {
          s += "<tr>";
          s += "<td colspan='5' class='kommentar'>";
          s += "<p>Zugegeben am "+ list_EwZutat[i]->getZugabezeitpunkt_von().toString("dd.MM.yyyy")+"</p>";
          if (list_EwZutat[i]->getEntnahmeindex() == EWZ_Entnahmeindex_MitEntnahme) {
            if (list_EwZutat[i]->getZugabestatus() == EWZ_Zugabestatus_Entnommen) {
              s += "<p>"+ trUtf8("Entnommen am ")+ list_EwZutat[i]->getZugabezeitpunkt_bis().toString("dd.MM.yyyy")
                  + " (" + trUtf8("Tage: ") +
                  QString::number(list_EwZutat[i]->getDauerMinuten()/1440)+")</p>";
            }
          }
          s += "</td>";
          s += "</tr>";
        }
        if (list_EwZutat[i]->getBemerkung() != "") {
          s += "<tr>";
          s += "<td colspan='5' class='kommentar'>" + list_EwZutat[i]->getBemerkung().replace("\n","<br>") + "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</tbody></table>";
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
    s += "<p>" + sEinheiten +"</p>";
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

    // Rasten
    if (list_Rasten.count() > 0) {
      s = "<table><tbody>";
      s += "<tr>";
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
      for (int i=0; i<list_Rasten.count();i++){
        s += "<tr>";
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
      contextVariables["Rasten"] = s;
      contextVariables["RastenTitel"] = trUtf8("Rasten");
    }

    // Anhang
    s = "";
    if (list_Anhang.count() > 0) {
      for (int i=0; i<list_Anhang.count();i++){
        if (AnhangWidget::isImage(list_Anhang[i]->getPfad()))
          s += "<img style=\"max-width:80%;\" src=\"file:///" + list_Anhang[i]->getFullPfad() + "\"></br></br>";
        else
          s += "<a href=\"file:///" + list_Anhang[i]->getFullPfad() + "\" target=\"_blank\">" + list_Anhang[i]->getPfad() + "</a></br></br>";
      }
    }
    contextVariables["Anhang"] = s;
    contextVariables["AnhangTitel"] = trUtf8("Anhänge");

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
    QString settingsPath = QFileInfo(settings.fileName()).absolutePath() + "/";

    QFile file(settingsPath + "zusammenfassung.html");
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

