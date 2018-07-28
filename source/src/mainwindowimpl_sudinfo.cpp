#include "mainwindowimpl.h"

#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include "definitionen.h"
#include "errormessage.h"

void MainWindowImpl::ErstelleSudInfo()
{
    // Seitenkopf
    QString seite, kopf, ende, style, s, SudnameFehler;
    int NeuBerechnen = 0;

    kopf = "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'> <html><head><meta name='qrichtext' content='1' />";
    if (StyleDunkel){
      style = "<style type='text/css'>";
      //Style für P
      style += "p{color:#fff;font-size:10pt;padding:0px;margin:0px;}";
      //Style für Variable
      style += "p.value{color:#eee;margin-left:5px;margin-right:5px;}";
      //Style für div Kommentar
      style += ".koment{}";
      //Style für ul
      style += "ul{color:#fff;font-size:10pt;}";
      //Style für Überschrift h1
      style += "p.h1{color:#fff;font-size:12pt;}";
      //Style für Überschrift h2
      style += "p.h2{color:#fff;font-size:11pt;margin-bottom:5px;}";
      //Style für Div Box bewertung
      style += "div.bew{border:0px solid #fff; border-radius: 5px; padding:0px;background-color:#222222;}";
      //Style für Div Box ohne Rahmen
      style += "div.r{border:0px solid #444444; border-radius: 10px; padding:5px;background-color:#444444;}";
      //Style für Div Box mit Rahmen
      style += "div.rm{border:2px solid #444444; border-radius: 10px; padding:5px;background-color:#222222;font-size:12pt;}";
      //Style für Div Box mit Rahmen für Hinweis
      style += "div.rmh{border:2px solid #ff0000; border-radius: 10px; padding:5px;background-color:#222222;font-size:12pt;}";
      //Style für Tabelle
      style += "td{padding:2px;margin:0px;font-size:10pt;}";
      style += "td.r{padding:2px;margin:0px;border-bottom-color:#aaaaaa;border-bottom-style:solid;border-width:1px;}";
      style += "tr{padding:0px;margin:0px;}";
      style += "body{font-family:Ubuntu,Arial; font-size:10pt; font-style:normal; background-color:#222222; color:#fff;}";
      style += "</style>";
    }
    else {
      style = "<style type='text/css'>";
      //Style für P
      style += "p{color:black;font-size:10pt;padding:0px;margin:0px;}";
      //Style für Variable
      style += "p.value{color:blue;margin-left:5px;margin-right:5px;}";
      //Style für div Kommentar
      style += ".koment{}";
      //Style für ul
      style += "ul{color:black;font-size:10pt;}";
      //Style für Überschrift h1
      style += "p.h1{color:black;font-size:12pt;}";
      //Style für Überschrift h2
      style += "p.h2{color:black;font-size:11pt;margin-bottom:5px;}";
      //Style für Div Box bewertung
      style += "div.bew{border:0px solid #fff; border-radius: 5px; padding:0px;background-color:#fff;}";
      //Style für Div Box ohne Rahmen
      style += "div.r{border:0px solid #dddddd; border-radius: 10px; padding:5px;background-color:#dddddd;}";
      //Style für Div Box mit Rahmen
      style += "div.rm{border:2px solid #dddddd; border-radius: 10px; padding:5px;background-color:#ffffff;font-size:12pt;}";
      //Style für Div Box mit Rahmen für Hinweis
      style += "div.rmh{border:2px solid #ff0000; border-radius: 10px; padding:5px;background-color:#ffffff;font-size:12pt;}";
      //Style für Tabelle
      style += "td{padding:2px;margin:0px;font-size:10pt;}";
      style += "td.r{padding:2px;margin:0px;border-bottom-color:#aaaaaa;border-bottom-style:solid;border-width:1px;}";
      style += "tr{padding:0px;margin:0px;}";
      style += "body{font-family:Ubuntu,Arial; font-size:10pt; font-style:normal; background-color:#fff;}";
      style += "</style>";
    }
    kopf += style;
    kopf += "</head><body>";
    seite = kopf;

    //Liste der SudIds
    QList<int> ListSudID;

    //Bei einer Singleauswahl Datensatz abfragen und Eckdaten anzeigen
    QList<QTableWidgetItem *> sList;
    sList = tableWidget_Sudauswahl -> selectedItems();

    if (sList.count() == 4 || sList.count() == 5) {

      //Sud ID ermitteln
      int row = sList.first() -> row();
      int SudID = tableWidget_Sudauswahl -> item(row,0) -> text().toInt();
      ListSudID.append(SudID);

      //Datensatz abfragen
      QSqlQuery query_sud;
      int FeldNr;
      query_sud.prepare("SELECT * FROM Sud WHERE ID=:sudid;");
      query_sud.bindValue(":sudid", SudID);
      if (!query_sud.exec()) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_sud.lastError().databaseText()
                                    + trUtf8("\nSQL-Befehl:\n") + query_sud.lastQuery());
      }
      else {
        if (query_sud.first()) {
          //Wem Sud neu Berechnet werden muss Hinweis anzeigen das dieser Sud erst geladen und gespeichert werden muss
          //da die Berechneten werte nicht mehr stimmen
          FeldNr = query_sud.record().indexOf("NeuBerechnen");
          if (query_sud.value(FeldNr).toBool()){
            NeuBerechnen = SudID;
            FeldNr = query_sud.record().indexOf("Sudname");
            SudnameFehler = query_sud.value(FeldNr).toString();
          }
          else {
            //Überschrift
            FeldNr = query_sud.record().indexOf("Sudname");
            seite += "<div class='r' style='margin-bottom:10px;' align='center'><p class='h1'><b>" + query_sud.value(FeldNr).toString() + "</b></p></div>";

            //bild mit entsprechender Bierfarbe
            QColor farbe;
            FeldNr = query_sud.record().indexOf("erg_Farbe");
            farbe = Berechnungen.GetFarbwert(query_sud.value(FeldNr).toDouble());
            FeldNr = query_sud.record().indexOf("Bewertung");
            int bewertung = query_sud.value(FeldNr).toInt();

            //Solldaten des Rezeptes
            s += "<div class='rm' style='margin-top:10px;margin-bottom:5px;' align='center'>";
            //Bewertung
            if (bewertung > 0){
              if (bewertung > MaxAnzahlSterne)
                bewertung = MaxAnzahlSterne;
              s += "<div class='bew' style='' align='center'>";
              for (int i = 0; i<bewertung; i++){
                s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_24.png' width='24' border=0>";
              }
              for (int i = bewertung; i<MaxAnzahlSterne; i++){
                if (StyleDunkel)
                  s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_gr_dark_24.png' width='24' border=0>";
                else
                  s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_gr_24.png' width='24' border=0>";
              }
              s += "</div>";
            }
            s += "<table cellspacing=0 border=0><tbody>";
            //Menge
            FeldNr = query_sud.record().indexOf("Menge");
            s += "<tr style=''>";
            s += "<td rowspan=5>";
            s += "<div class='r' style='background-color:" + farbe.name() +
                ";width:100px;height:100px;margin:0px;padding:0px;'>";
            if (StyleDunkel)
              s += "<img style='padding:0px;margin:0px;' src='qrc:/global/bier_dark_200x200.png' alt='Bierfarbe' width='100' height='100' border=0>";
            else
              s += "<img style='padding:0px;margin:0px;' src='qrc:/global/bier_200x200.png' alt='Bierfarbe' width='100' height='100' border=0>";
            s += "</div>";
            s += "</td>";
            s += "<td>";
            s += "<p>" + trUtf8("Menge") + "</p>";
            s += "</td>";
            s += "<td align='right'>";
            s += "<p class='value'>" + QString::number(query_sud.value(FeldNr).toInt()) + "</p>";
            s += "</td>";
            s += "<td>";
            s += "<p>" + trUtf8("Liter") + "</p>";
            s += "</td>";
            s += "</tr>";
            //Stammwürze
            FeldNr = query_sud.record().indexOf("SW");
            s += "<tr style=''>";
            s += "<td>";
            s += "<p>" + trUtf8("Stammwürze") + "</p>";
            s += "</td>";
            s += "<td align='right'>";
            s += "<p class='value'>" + QString::number(query_sud.value(FeldNr).toDouble()) + "</p>";
            s += "</td>";
            s += "<td>";
            s += "<p>" + trUtf8("°P") + "</p>";
            s += "</td>";
            s += "</tr>";
            //Bittere
            FeldNr = query_sud.record().indexOf("IBU");
            s += "<tr style=''>";
            s += "<td>";
            s += "<p>" + trUtf8("Bittere") + "</p>";
            s += "</td>";
            s += "<td align='right'>";
            s += "<p class='value'>" + QString::number(query_sud.value(FeldNr).toInt()) + "</p>";
            s += "</td>";
            s += "<td>";
            s += "<p>" + trUtf8("IBU") + "</p>";
            s += "</td>";
            s += "</tr>";
            //Farbe
            FeldNr = query_sud.record().indexOf("erg_Farbe");
            s += "<tr style=''>";
            s += "<td>";
            s += "<p>" + trUtf8("Farbe") + "</p>";
            s += "</td>";
            s += "<td align='right'>";
            s += "<p class='value'>" + QString::number(query_sud.value(FeldNr).toDouble()) + "</p>";
            s += "</td>";
            s += "<td>";
            s += "<p>" + trUtf8("EBC") + "</p>";
            s += "</td>";
            s += "</tr>";
            //CO₂-Gehalt
            FeldNr = query_sud.record().indexOf("CO2");
            s += "<tr style=''>";
            s += "<td>";
            s += "<p>" + trUtf8("CO₂-Gehalt") + "</p>";
            s += "</td>";
            s += "<td align='right'>";
            s += "<p class='value'>" + QString::number(query_sud.value(FeldNr).toDouble()) + "</p>";
            s += "</td>";
            s += "<td>";
            s += "<p>" + trUtf8("g/l") + "</p>";
            s += "</td>";
            s += "</tr>";
            s += "</tbody></table>";
            s += "</div>";
          }
        }
      }
    }
    else {
      //Überschrift Auswahl
      //s += "<div class='r' style='margin-bottom:10px;' align='center'><p class='h1'><b>" + trUtf8("Auswahl") + "</b></p></div>";
      //s += "<div class='rm' style='margin-bottom:10px;'>";
      QList<int> row_merker;
      //Bei Mehrfachauswahl alle Sudnamen anzeigen
      for (int i = 0; i < sList.size(); ++i) {
        //Sud ID ermitteln
        int row = sList.at(i) -> row();
        //wenn sich die zeile ändert
        if (row_merker.indexOf(row) == -1){
          row_merker.append(row);
          int SudID = tableWidget_Sudauswahl -> item(row,0) -> text().toInt();
          ListSudID.append(SudID);
          //Datensatz abfragen
          QSqlQuery query_sud;
          query_sud.prepare("SELECT Sudname,NeuBerechnen FROM Sud WHERE ID=:sudid;");
          query_sud.bindValue(":sudid", SudID);
          if (!query_sud.exec()) {
            // Fehlermeldung Datenbankabfrage
            ErrorMessage *errorMessage = new ErrorMessage();
            errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                        CANCEL_NO, trUtf8("Rückgabe:\n") + query_sud.lastError().databaseText()
                                        + trUtf8("\nSQL-Befehl:\n") + query_sud.lastQuery());
          }
          else {
            if (query_sud.first()) {
              //Wenn Sud neu Berechnet werden muss Hinweis anzeigen das dieser Sud erst geladen und gespeichert werden muss
              //da die Berechneten werte nicht mehr stimmen
              int FeldNr = query_sud.record().indexOf("NeuBerechnen");
              if (query_sud.value(FeldNr).toBool()){
                NeuBerechnen = SudID;
                FeldNr = query_sud.record().indexOf("Sudname");
                SudnameFehler = query_sud.value(FeldNr).toString();
              }
            }
          }
        }
      }
      //s += "</div>";
    }

    if (NeuBerechnen > 0){
      //Meldung ausgeben das der Sud zum Neu Berechnen geladen werden muss.
      seite += "<div class='rmh' style='margin-bottom:10px;' align='center'>";
      seite += trUtf8("Bei dem Sud &gt;") + SudnameFehler + trUtf8("&lt; wurde ein Rohstoff verändert. Die berechneten Werte stimmen  nicht mehr. Zum Neuberechnen bitte den Sud laden und wieder speichern.");
      seite += "</div>";
    }
    else {
      //Benötigte Rohstoffe mit Vorhandenen verechnen

      //Alle Malzeinträge abrfuen
      QList<Rohstoff> ListMalz;
      for (int sid = 0; sid < ListSudID.size(); ++sid){
        //Schüttung Abfragen
        QSqlQuery query_Malz;
        QString sql = "SELECT * FROM Malzschuettung WHERE SudID=" + QString::number(ListSudID.at(sid)) + ";";
        if (!query_Malz.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_Malz.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + sql);
        }
        else {
          while (query_Malz.next()){
            int FeldNr_Name = query_Malz.record().indexOf("Name");
            int FeldNr_ID = query_Malz.record().indexOf("ID");
            int FeldNr_ergMenge = query_Malz.record().indexOf("erg_Menge");

            Rohstoff eMalz;
            eMalz.ID = query_Malz.value(FeldNr_ID).toInt();
            eMalz.Menge = query_Malz.value(FeldNr_ergMenge).toDouble();
            eMalz.Name = query_Malz.value(FeldNr_Name).toString();

            if (eMalz.Name != ""){
              //Überprüfen ob Rohstoff schon einmal vorhanden ist
              bool b=false;
              for (int i = 0; i < ListMalz.size(); ++i){
                //wenn Eintrag schon vorhanden ist die Menge adieren
                if (eMalz.Name == ListMalz.at(i).Name){
                  b = true;
                  eMalz.Menge += ListMalz.at(i).Menge;
                  ListMalz.replace(i,eMalz);
                }
              }
              //wenn Rohstoff noch nicht vorhanden ist Eintrag der Liste hinzufügen
              if (!b)
                ListMalz.append(eMalz);
            }
          }
        }
      }

      //Alle Hopfeneinträge abrfuen
      QList<Rohstoff> ListHopfen;
      for (int sid = 0; sid < ListSudID.size(); ++sid){
        //Hopfen Abfragen
        QSqlQuery query_Hopfen;
        QString sql = "SELECT * FROM Hopfengaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + ";";
        if (!query_Hopfen.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_Hopfen.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + sql);
        }
        else {
          while (query_Hopfen.next()){
            int FeldNr_Name = query_Hopfen.record().indexOf("Name");
            int FeldNr_ID = query_Hopfen.record().indexOf("ID");
            int FeldNr_ergMenge = query_Hopfen.record().indexOf("erg_Menge");
            int FeldNrAktiv = query_Hopfen.record().indexOf("Aktiv");


            Rohstoff eHopfen;
            eHopfen.ID = query_Hopfen.value(FeldNr_ID).toInt();
            eHopfen.Menge = query_Hopfen.value(FeldNr_ergMenge).toDouble();
            eHopfen.Name = query_Hopfen.value(FeldNr_Name).toString();

            if (query_Hopfen.value(FeldNrAktiv).toBool()){
              //Überprüfen ob Rohstoff schon einmal vorhanden ist
              bool b=false;
              for (int i = 0; i < ListHopfen.size(); ++i){
                //wenn Eintrag schon vorhanden ist die Menge adieren
                if (eHopfen.Name == ListHopfen.at(i).Name){
                  b = true;
                  eHopfen.Menge += ListHopfen.at(i).Menge;
                  ListHopfen.replace(i,eHopfen);
                }
              }
              //wenn Rohstoff noch nicht vorhanden ist Eintrag der Liste hinzufügen
              if (!b)
                ListHopfen.append(eHopfen);
            }
          }
        }
        //Alle Hopfengaben aus den Weiteren Zutaten abfragen
        sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + " AND Typ=100;";
        if (!query_Hopfen.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_Hopfen.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + sql);
        }
        else {
          while (query_Hopfen.next()){
            int FeldNr_Name = query_Hopfen.record().indexOf("Name");
            int FeldNr_ID = query_Hopfen.record().indexOf("ID");
            int FeldNr_ergMenge = query_Hopfen.record().indexOf("erg_Menge");


            Rohstoff eHopfen;
            eHopfen.ID = query_Hopfen.value(FeldNr_ID).toInt();
            eHopfen.Menge = query_Hopfen.value(FeldNr_ergMenge).toDouble();
            eHopfen.Name = query_Hopfen.value(FeldNr_Name).toString();
            //Überprüfen ob Rohstoff schon einmal vorhanden ist
            bool b=false;
            for (int i = 0; i < ListHopfen.size(); ++i){
              //wenn Eintrag schon vorhanden ist die Menge adieren
              if (eHopfen.Name == ListHopfen.at(i).Name){
                b = true;
                eHopfen.Menge += ListHopfen.at(i).Menge;
                ListHopfen.replace(i,eHopfen);
              }
            }
            //wenn Rohstoff noch nicht vorhanden ist Eintrag der Liste hinzufügen
            if (!b)
              ListHopfen.append(eHopfen);
          }
        }
      }

      //Alle Hefe einträge abrfuen
      QList<Rohstoff> ListHefe;
      for (int sid = 0; sid < ListSudID.size(); ++sid){
        //Hefe Abfragen
        QSqlQuery query_Hefe;
        QString sql = "SELECT AuswahlHefe,HefeAnzahlEinheiten FROM Sud WHERE ID=" + QString::number(ListSudID.at(sid)) + ";";
        if (!query_Hefe.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_Hefe.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + sql);
        }
        else {
          while (query_Hefe.next()){
            int FeldNr_Name = query_Hefe.record().indexOf("AuswahlHefe");

            Rohstoff eHefe;
            eHefe.ID = 1;
            eHefe.Name = query_Hefe.value(FeldNr_Name).toString();

            int AnzahlHefeEintraege = tableWidget_Hefe -> rowCount();
            if (eHefe.Name != ""){
              //zur verfügungstehende Malzmenge
              for (int i=0; i < AnzahlHefeEintraege; i++){
                //wenn Eintrag übereinstimmt
                if (tableWidget_Hefe -> item(i,TableHefeColName) -> text() == eHefe.Name){
                  QSpinBox *spinBox = (QSpinBox*)tableWidget_Hefe -> cellWidget(i,TableHefeColMenge);
                  eHefe.MengeIst = spinBox->value();
                  //benötigte Hefemenge
                  FeldNr_Name = query_Hefe.record().indexOf("HefeAnzahlEinheiten");
                  eHefe.Menge = query_Hefe.value(FeldNr_Name).toInt();
                }
              }
            }
            //Überprüfen ob Rohstoff schon einmal vorhanden ist
            bool b=false;
            for (int i = 0; i < ListHefe.size(); ++i){
              //wenn Eintrag schon vorhanden ist die Menge adieren
              if (eHefe.Name == ListHefe.at(i).Name){
                b = true;
                eHefe.Menge += ListHefe.at(i).Menge;
                ListHefe.replace(i,eHefe);
              }
            }
            //wenn Rohstoff noch nicht vorhanden ist Eintrag der Liste hinzufügen
            if (!b){
              if (eHefe.Name != "")
                ListHefe.append(eHefe);
            }
          }
        }
      }

      //Honig
      QList<Rohstoff> ListWeitereZutatenHonig;
      for (int sid = 0; sid < ListSudID.size(); ++sid){
        QString sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + " AND Typ=0;";
        QSqlQuery query_weitereZutaten;
        if (!query_weitereZutaten.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + sql);
        }
        else {
          while (query_weitereZutaten.next()){
            int FeldNr_Name = query_weitereZutaten.record().indexOf("Name");
            int FeldNr_ID = query_weitereZutaten.record().indexOf("ID");
            int FeldNr_ergMenge = query_weitereZutaten.record().indexOf("erg_Menge");

            Rohstoff eWeitereZutat;
            eWeitereZutat.ID = query_weitereZutaten.value(FeldNr_ID).toInt();
            eWeitereZutat.Menge = query_weitereZutaten.value(FeldNr_ergMenge).toDouble();
            eWeitereZutat.Name = query_weitereZutaten.value(FeldNr_Name).toString();
            //Überprüfen ob Rohstoff schon einmal vorhanden ist
            bool b=false;
            for (int i = 0; i < ListWeitereZutatenHonig.size(); ++i){
              //wenn Eintrag schon vorhanden ist die Menge adieren
              if (eWeitereZutat.Name == ListWeitereZutatenHonig.at(i).Name){
                b = true;
                eWeitereZutat.Menge += ListWeitereZutatenHonig.at(i).Menge;
                ListWeitereZutatenHonig.replace(i,eWeitereZutat);
              }
            }
            //wenn Rohstoff noch nicht vorhanden ist Eintrag der Liste hinzufügen
            if (!b)
              ListWeitereZutatenHonig.append(eWeitereZutat);
          }
        }
      }

      //Zucker
      QList<Rohstoff> ListWeitereZutatenZucker;
      for (int sid = 0; sid < ListSudID.size(); ++sid){
        QString sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + " AND Typ=1;";
        QSqlQuery query_weitereZutaten;
        if (!query_weitereZutaten.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + sql);
        }
        else {
          while (query_weitereZutaten.next()){
            int FeldNr_Name = query_weitereZutaten.record().indexOf("Name");
            int FeldNr_ID = query_weitereZutaten.record().indexOf("ID");
            int FeldNr_ergMenge = query_weitereZutaten.record().indexOf("erg_Menge");


            Rohstoff eWeitereZutat;
            eWeitereZutat.ID = query_weitereZutaten.value(FeldNr_ID).toInt();
            eWeitereZutat.Menge = query_weitereZutaten.value(FeldNr_ergMenge).toDouble();
            eWeitereZutat.Name = query_weitereZutaten.value(FeldNr_Name).toString();
            //Überprüfen ob Rohstoff schon einmal vorhanden ist
            bool b=false;
            for (int i = 0; i < ListWeitereZutatenZucker.size(); ++i){
              //wenn Eintrag schon vorhanden ist die Menge adieren
              if (eWeitereZutat.Name == ListWeitereZutatenZucker.at(i).Name){
                b = true;
                eWeitereZutat.Menge += ListWeitereZutatenZucker.at(i).Menge;
                ListWeitereZutatenZucker.replace(i,eWeitereZutat);
              }
            }
            //wenn Rohstoff noch nicht vorhanden ist Eintrag der Liste hinzufügen
            if (!b)
              ListWeitereZutatenZucker.append(eWeitereZutat);
          }
        }
      }

      //Gewuerz
      QList<Rohstoff> ListWeitereZutatenGewuerz;
      for (int sid = 0; sid < ListSudID.size(); ++sid){
        QString sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + " AND Typ=2;";
        QSqlQuery query_weitereZutaten;
        if (!query_weitereZutaten.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + sql);
        }
        else {
          while (query_weitereZutaten.next()){
            int FeldNr_Name = query_weitereZutaten.record().indexOf("Name");
            int FeldNr_ID = query_weitereZutaten.record().indexOf("ID");
            int FeldNr_ergMenge = query_weitereZutaten.record().indexOf("erg_Menge");


            Rohstoff eWeitereZutat;
            eWeitereZutat.ID = query_weitereZutaten.value(FeldNr_ID).toInt();
            eWeitereZutat.Menge = query_weitereZutaten.value(FeldNr_ergMenge).toDouble();
            eWeitereZutat.Name = query_weitereZutaten.value(FeldNr_Name).toString();
            //Überprüfen ob Rohstoff schon einmal vorhanden ist
            bool b=false;
            for (int i = 0; i < ListWeitereZutatenGewuerz.size(); ++i){
              //wenn Eintrag schon vorhanden ist die Menge adieren
              if (eWeitereZutat.Name == ListWeitereZutatenGewuerz.at(i).Name){
                b = true;
                eWeitereZutat.Menge += ListWeitereZutatenGewuerz.at(i).Menge;
                ListWeitereZutatenGewuerz.replace(i,eWeitereZutat);
              }
            }
            //wenn Rohstoff noch nicht vorhanden ist Eintrag der Liste hinzufügen
            if (!b)
              ListWeitereZutatenGewuerz.append(eWeitereZutat);
          }
        }
      }


      //Frucht
      QList<Rohstoff> ListWeitereZutatenFrucht;
      for (int sid = 0; sid < ListSudID.size(); ++sid){
        QString sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + " AND Typ=3;";
        QSqlQuery query_weitereZutaten;
        if (!query_weitereZutaten.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + sql);
        }
        else {
          while (query_weitereZutaten.next()){
            int FeldNr_Name = query_weitereZutaten.record().indexOf("Name");
            int FeldNr_ID = query_weitereZutaten.record().indexOf("ID");
            int FeldNr_ergMenge = query_weitereZutaten.record().indexOf("erg_Menge");


            Rohstoff eWeitereZutat;
            eWeitereZutat.ID = query_weitereZutaten.value(FeldNr_ID).toInt();
            eWeitereZutat.Menge = query_weitereZutaten.value(FeldNr_ergMenge).toDouble();
            eWeitereZutat.Name = query_weitereZutaten.value(FeldNr_Name).toString();
            //Überprüfen ob Rohstoff schon einmal vorhanden ist
            bool b=false;
            for (int i = 0; i < ListWeitereZutatenFrucht.size(); ++i){
              //wenn Eintrag schon vorhanden ist die Menge adieren
              if (eWeitereZutat.Name == ListWeitereZutatenFrucht.at(i).Name){
                b = true;
                eWeitereZutat.Menge += ListWeitereZutatenFrucht.at(i).Menge;
                ListWeitereZutatenFrucht.replace(i,eWeitereZutat);
              }
            }
            //wenn Rohstoff noch nicht vorhanden ist Eintrag der Liste hinzufügen
            if (!b)
              ListWeitereZutatenFrucht.append(eWeitereZutat);
          }
        }
      }


      //Sonstiges
      QList<Rohstoff> ListWeitereZutatenSonstiges;
      for (int sid = 0; sid < ListSudID.size(); ++sid){
        QString sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + " AND Typ=4;";
        QSqlQuery query_weitereZutaten;
        if (!query_weitereZutaten.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + sql);
        }
        else {
          while (query_weitereZutaten.next()){
            int FeldNr_Name = query_weitereZutaten.record().indexOf("Name");
            int FeldNr_ID = query_weitereZutaten.record().indexOf("ID");
            int FeldNr_ergMenge = query_weitereZutaten.record().indexOf("erg_Menge");


            Rohstoff eWeitereZutat;
            eWeitereZutat.ID = query_weitereZutaten.value(FeldNr_ID).toInt();
            eWeitereZutat.Menge = query_weitereZutaten.value(FeldNr_ergMenge).toDouble();
            eWeitereZutat.Name = query_weitereZutaten.value(FeldNr_Name).toString();
            //Überprüfen ob Rohstoff schon einmal vorhanden ist
            bool b=false;
            for (int i = 0; i < ListWeitereZutatenSonstiges.size(); ++i){
              //wenn Eintrag schon vorhanden ist die Menge adieren
              if (eWeitereZutat.Name == ListWeitereZutatenSonstiges.at(i).Name){
                b = true;
                eWeitereZutat.Menge += ListWeitereZutatenSonstiges.at(i).Menge;
                ListWeitereZutatenSonstiges.replace(i,eWeitereZutat);
              }
            }
            //wenn Rohstoff noch nicht vorhanden ist Eintrag der Liste hinzufügen
            if (!b)
              ListWeitereZutatenSonstiges.append(eWeitereZutat);
          }
        }
      }

      //Überschrift Benötigte Rohstoffe
      s += "<div class='r' style='margin-bottom:10px;' align='center'><p class='h1'><b>" + trUtf8("Rohstoffe") + "</b></p></div>";

      //Malz Mengen anzeigen
      //Bild für getreide anzeigen
      s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/zutaten/getreide_300.png' alt='Getreide' width='300px' border=0></div>";
      s += "<div align='center' style='font-size:12pt;'>";
      s += "<table border=0 cellspacing=0 >";
      s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt (kg)") + "</td><td align='center'>" + trUtf8("übrig (kg)") + "</td>";
      for (int i = 0; i < ListMalz.size(); ++i){
        double ist = 0;
        //Vorhandene Menge von diesem Malz
        bool gefunden = false;
        for (int o=0; o < tableWidget_Malz -> rowCount(); o++){
          //wenn Eintrag übereinstimmt
          if (tableWidget_Malz -> item(o, TableMalzColName) -> text() == ListMalz.at(i).Name){
            QDoubleSpinBox* dsbMenge = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(o,TableMalzColMenge);
            ist = dsbMenge ->value();
            gefunden = true;
          }
        }
        double rest = ist - ListMalz.at(i).Menge;
        s += "<tr valign='middle'>";
        if (rest < 0){
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
        }
        else {
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
        }
        //Rohstoff ist aufgeführt
        if (gefunden){
          s += "<td align='left'>" + ListMalz.at(i).Name + "</td>";
        }
        //Rohstoff ist nicht vorhanden
        else {
          s += "<td align='left' style='color: grey;'>" + ListMalz.at(i).Name + "</td>";
        }
        s += "<td align='center'>" + QString::number(ListMalz.at(i).Menge) + "</td>";
        if (rest < 0)
          s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b></td>";
        else
          s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b></td>";
        s += "</tr>";
      }
      s += "</table>";
      s += "</div>";


      //Hopfen Mengen anzeigen
      //Bild für Hopfen anzeigen
      s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/zutaten/hopfen_100.png' alt='Hopfen' width='100px' border=0></div>";
      s += "<div align='center' style='font-size:12pt;'>";
      s += "<table border=0 cellspacing=0 >";
      s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt (g)") + "</td><td align='center'>" + trUtf8("übrig (g)") + "</td>";
      for (int i = 0; i < ListHopfen.size(); ++i){
        double ist = 0;
        bool gefunden = false;
        //Vorhandene Menge von diesem Hopfen
        for (int o=0; o < tableWidget_Hopfen -> rowCount(); o++){
          //wenn Eintrag übereinstimmt
          if (tableWidget_Hopfen -> item(o,TableHopfenColName) -> text() == ListHopfen.at(i).Name){
            QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(o,TableHopfenColMenge);
            ist = spinBox->value();
            gefunden = true;
          }
        }
        double rest = ist - ListHopfen.at(i).Menge;
        s += "<tr valign='middle'>";
        if (rest < 0)
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
        else
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
        //Rohstoff ist aufgeführt
        if (gefunden){
          s += "<td align='left'>" + ListHopfen.at(i).Name + "</td>";
        }
        //Rohstoff ist nicht vorhanden
        else {
          s += "<td align='left' style='color: grey;'>" + ListHopfen.at(i).Name + "</td>";
        }
        s += "<td align='center'>" + QString::number(ListHopfen.at(i).Menge) + "</td>";
        if (rest < 0)
          s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b></td>";
        else
          s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b></td>";
        s += "</tr>";
      }
      s += "</table>";
      s += "</div>";


      //Hefe Mengen anzeigen
      //Bild für Hefe anzeigen
      s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/zutaten/hefe_50.png' alt='Hefe' width='50px' border=0></div>";
      s += "<div align='center' style='font-size:12pt;'>";
      s += "<table border=0 cellspacing=0 >";
      s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("übrig") + "</td>";
      for (int i = 0; i < ListHefe.size(); ++i){
        double ist;
        bool gefunden = false;
        int AnzahlHefeEintraege = tableWidget_Hefe -> rowCount();
        for (int o=0; o < AnzahlHefeEintraege; o++){
          //wenn Eintrag übereinstimmt
          if (tableWidget_Hefe -> item(o,TableHefeColName) -> text() == ListHefe.at(i).Name){
            gefunden = true;
          }
        }
        //Vorhandene Menge von diesem Hopfen
        ist = ListHefe.at(i).MengeIst;
        double rest = ist - ListHefe.at(i).Menge;
        s += "<tr valign='middle'>";
        if (rest < 0)
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
        else
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
        //Rohstoff ist aufgeführt
        if (gefunden){
          s += "<td align='left'>" + ListHefe.at(i).Name + "</td>";
        }
        //Rohstoff ist nicht vorhanden
        else {
          s += "<td align='left' style='color: grey;'>" + ListHefe.at(i).Name + "</td>";
        }
        s += "<td align='center'>" + QString::number(ListHefe.at(i).Menge) + "</td>";
        if (rest < 0)
          s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b></td>";
        else
          s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b></td>";
        s += "</tr>";
      }
      s += "</table>";
      s += "</div>";

      //WeitereZutaten Honig Mengen anzeigen
      //Bild für Honig anzeigen
      if (ListWeitereZutatenHonig.count() > 0){
        int Einheit=0;
        s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_0_50.png' alt='Honig' width='50px' border=0></div>";
        s += "<div align='center' style='font-size:12pt;'>";
        s += "<table border=0 cellspacing=0 >";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("übrig") + "</td>";
        for (int i = 0; i < ListWeitereZutatenHonig.size(); ++i){
          double ist=0;
          bool gefunden = false;
          //Vorhandene Menge von diesem Honig
          for (int o=0; o < tableWidget_WeitereZutaten -> rowCount(); o++){
            //wenn Eintrag übereinstimmt
            if (tableWidget_WeitereZutaten -> item(o,TableWZutatColName) -> text() == ListWeitereZutatenHonig.at(i).Name){
              QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,TableWZutatColMenge);
              ist = dsbMenge -> value();
              QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(o,TableWZutatColEinheit);
              Einheit = comboEinheit -> currentIndex();
              if (Einheit == EWZ_Einheit_Kg){
                ist = ist * 1000;
              }
              gefunden = true;
            }
          }
          double rest = ist - ListWeitereZutatenHonig.at(i).Menge;
          s += "<tr valign='middle'>";
          if (rest < 0)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden){
            s += "<td align='left'>" + ListWeitereZutatenHonig.at(i).Name + "</td>";
          }
          //Rohstoff ist nicht vorhanden
          else {
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenHonig.at(i).Name + "</td>";
          }
          if (Einheit == EWZ_Einheit_Kg){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenHonig.at(i).Menge / 1000) + " kg</td>";
            if (rest < 0)
              s += "<td align='center' style='color: red;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenHonig.at(i).Menge) + " g</td>";
            if (rest < 0)
              s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        s += "</div>";
      }
      //WeitereZutaten Zucker Mengen anzeigen
      //Bild für Zucker anzeigen
      if (ListWeitereZutatenZucker.count() > 0){
        int Einheit=0;
        s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_1_50.png' alt='Honig' width='50px' border=0></div>";
        s += "<div align='center' style='font-size:12pt;'>";
        s += "<table border=0 cellspacing=0 >";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("übrig") + "</td>";
        for (int i = 0; i < ListWeitereZutatenZucker.size(); ++i){
          double ist=0;
          bool gefunden = false;
          //Vorhandene Menge von diesem Honig
          for (int o=0; o < tableWidget_WeitereZutaten -> rowCount(); o++){
            //wenn Eintrag übereinstimmt
            if (tableWidget_WeitereZutaten -> item(o,TableWZutatColName) -> text() == ListWeitereZutatenZucker.at(i).Name){
              QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,TableWZutatColMenge);
              ist = dsbMenge -> value();
              QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(o,TableWZutatColEinheit);
              Einheit = comboEinheit -> currentIndex();
              if (Einheit == 0){
                ist = ist * 1000;
              }
              gefunden = true;
            }
          }
          double rest = ist - ListWeitereZutatenZucker.at(i).Menge;
          s += "<tr valign='middle'>";
          if (rest < 0)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden){
            s += "<td align='left'>" + ListWeitereZutatenZucker.at(i).Name + "</td>";
          }
          //Rohstoff ist nicht vorhanden
          else {
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenZucker.at(i).Name + "</td>";
          }
          if (Einheit == 0){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenZucker.at(i).Menge / 1000) + " kg</td>";
            if (rest < 0)
              s += "<td align='center' style='color: red;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenZucker.at(i).Menge) + " g</td>";
            if (rest < 0)
              s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        s += "</div>";
      }
      //WeitereZutaten Gewuerz Mengen anzeigen
      //Bild für Gewuerz anzeigen
      if (ListWeitereZutatenGewuerz.count() > 0){
        int Einheit=0;
        s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_2_50.png' alt='Honig' width='50px' border=0></div>";
        s += "<div align='center' style='font-size:12pt;'>";
        s += "<table border=0 cellspacing=0 >";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("übrig") + "</td>";
        for (int i = 0; i < ListWeitereZutatenGewuerz.size(); ++i){
          double ist=0;
          bool gefunden = false;
          //Vorhandene Menge von diesem Honig
          for (int o=0; o < tableWidget_WeitereZutaten -> rowCount(); o++){
            //wenn Eintrag übereinstimmt
            if (tableWidget_WeitereZutaten -> item(o,TableWZutatColName) -> text() == ListWeitereZutatenGewuerz.at(i).Name){
              QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,TableWZutatColMenge);
              ist = dsbMenge -> value();
              QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(o,TableWZutatColEinheit);
              Einheit = comboEinheit -> currentIndex();
              if (Einheit == 0){
                ist = ist * 1000;
              }
              gefunden = true;
            }
          }
          double rest = ist - ListWeitereZutatenGewuerz.at(i).Menge;
          s += "<tr valign='middle'>";
          if (rest < 0)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden){
            s += "<td align='left'>" + ListWeitereZutatenGewuerz.at(i).Name + "</td>";
          }
          //Rohstoff ist nicht vorhanden
          else {
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenGewuerz.at(i).Name + "</td>";
          }
          if (Einheit == 0){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenGewuerz.at(i).Menge / 1000) + " kg</td>";
            if (rest < 0)
              s += "<td align='center' style='color: red;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenGewuerz.at(i).Menge) + " g</td>";
            if (rest < 0)
              s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        s += "</div>";
      }
      //WeitereZutaten Frucht Mengen anzeigen
      //Bild für Frucht anzeigen
      if (ListWeitereZutatenFrucht.count() > 0){
        int Einheit=0;
        s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_3_50.png' alt='Honig' width='50px' border=0></div>";
        s += "<div align='center' style='font-size:12pt;'>";
        s += "<table border=0 cellspacing=0 >";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("übrig") + "</td>";
        for (int i = 0; i < ListWeitereZutatenFrucht.size(); ++i){
          double ist=0;
          bool gefunden = false;
          //Vorhandene Menge von diesem Honig
          for (int o=0; o < tableWidget_WeitereZutaten -> rowCount(); o++){
            //wenn Eintrag übereinstimmt
            if (tableWidget_WeitereZutaten -> item(o,TableWZutatColName) -> text() == ListWeitereZutatenFrucht.at(i).Name){
              QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,TableWZutatColMenge);
              ist = dsbMenge -> value();
              QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(o,TableWZutatColEinheit);
              Einheit = comboEinheit -> currentIndex();
              if (Einheit == 0){
                ist = ist * 1000;
              }
              gefunden = true;
            }
          }
          double rest = ist - ListWeitereZutatenFrucht.at(i).Menge;
          s += "<tr valign='middle'>";
          if (rest < 0)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden){
            s += "<td align='left'>" + ListWeitereZutatenFrucht.at(i).Name + "</td>";
          }
          //Rohstoff ist nicht vorhanden
          else {
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenFrucht.at(i).Name + "</td>";
          }
          if (Einheit == 0){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenFrucht.at(i).Menge / 1000) + " kg</td>";
            if (rest < 0)
              s += "<td align='center' style='color: red;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenFrucht.at(i).Menge) + " g</td>";
            if (rest < 0)
              s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        s += "</div>";
      }
      //WeitereZutaten Sonstiges Mengen anzeigen
      //Bild für Sonstiges anzeigen
      if (ListWeitereZutatenSonstiges.count() > 0){
        int Einheit=0;
        s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_4_50.png' alt='Honig' width='50px' border=0></div>";
        s += "<div align='center' style='font-size:12pt;'>";
        s += "<table border=0 cellspacing=0 >";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("übrig") + "</td>";
        for (int i = 0; i < ListWeitereZutatenSonstiges.size(); ++i){
          double ist=0;
          bool gefunden = false;
          //Vorhandene Menge von diesem Honig
          for (int o=0; o < tableWidget_WeitereZutaten -> rowCount(); o++){
            //wenn Eintrag übereinstimmt
            if (tableWidget_WeitereZutaten -> item(o,TableWZutatColName) -> text() == ListWeitereZutatenSonstiges.at(i).Name){
              QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,TableWZutatColMenge);
              ist = dsbMenge -> value();
              QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(o,TableWZutatColEinheit);
              Einheit = comboEinheit -> currentIndex();
              if (Einheit == 0){
                ist = ist * 1000;
              }
              gefunden = true;
            }
          }
          double rest = ist - ListWeitereZutatenSonstiges.at(i).Menge;
          s += "<tr valign='middle'>";
          if (rest < 0)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden){
            s += "<td align='left'>" + ListWeitereZutatenSonstiges.at(i).Name + "</td>";
          }
          //Rohstoff ist nicht vorhanden
          else {
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenSonstiges.at(i).Name + "</td>";
          }
          if (Einheit == 0){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenSonstiges.at(i).Menge / 1000) + " kg</td>";
            if (rest < 0)
              s += "<td align='center' style='color: red;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenSonstiges.at(i).Menge) + " g</td>";
            if (rest < 0)
              s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        s += "</div>";
      }
    }

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
    settings.beginGroup("DB");
    QDir dbpfad = QDir(settings.value("DB_Pfad").toString());
    settings.endGroup();
    bool kopzeile = false;
    for (int sid = 0; sid < ListSudID.size(); ++sid){
      QString sql = "SELECT * FROM Anhang WHERE SudID=" + QString::number(ListSudID.at(sid));
      QSqlQuery query_anhang;
      if (!query_anhang.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_anhang.lastError().databaseText()
                                    + trUtf8("\nSQL-Befehl:\n") + sql);
      }
      else {
        while (query_anhang.next()){
          if (!kopzeile) {
            s += "<div class='r' style='margin-bottom:10px;' align='center'><p class='h1'><b>" + trUtf8("Anhänge") + "</b></p></div>";
            s += "<div align='center'>";
            kopzeile = true;
          }
          int FeldNr = query_anhang.record().indexOf("Pfad");
          QString pfad = query_anhang.value(FeldNr).toString();
          if (QDir::isRelativePath(pfad))
            pfad = dbpfad.filePath(pfad);
          if (AnhangWidget::isImage(pfad))
            s += "<img style=\"max-width:90%;\" src=\"file:///" + pfad + "\"></br></br>";
          else
            s += "<a href=\"file:///" + pfad + "\">" + pfad + "</a></br></br>";
        }
        if (kopzeile) {
            s += "</div>";
        }
      }
    }

    seite += s;
    //Seitenende
    ende = "</body></html>";
    seite += ende;

    webView_Info -> setHtml(seite, QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/"));
}
