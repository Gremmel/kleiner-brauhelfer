#include "mainwindowimpl.h"

#include <QFile>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include "mustache.h"
#include "definitionen.h"
#include "errormessage.h"

void MainWindowImpl::ErstelleSudInfo()
{
    QVariantHash contextVariables;
    contextVariables["Style"] = StyleDunkel ? "style_dunkel.css" : "style_hell.css";

    // Seitenkopf
    QString s, SudnameFehler;
    int NeuBerechnen = 0;

    //Liste der SudIds
    QList<int> ListSudID;

    //Bei einer Singleauswahl Datensatz abfragen und Eckdaten anzeigen
    QList<QTableWidgetItem *> sList;
    sList = tableWidget_Sudauswahl -> selectedItems();

    if (sList.count() > 0 && sList.count() <= tableWidget_Sudauswahl->columnCount()) {

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
            contextVariables["Sudname"] = query_sud.value(FeldNr).toString();

            //bild mit entsprechender Bierfarbe
            FeldNr = query_sud.record().indexOf("erg_Farbe");
            QColor farbe = Berechnungen.GetFarbwert(query_sud.value(FeldNr).toDouble());
            FeldNr = query_sud.record().indexOf("Bewertung");
            int bewertung = query_sud.value(FeldNr).toInt();

            //Bewertung
            if (bewertung > 0){
              if (bewertung > MaxAnzahlSterne)
                bewertung = MaxAnzahlSterne;
              s = "";
              for (int i = 0; i<bewertung; i++){
                s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_24.png' width='24' border=0>";
              }
              for (int i = bewertung; i<MaxAnzahlSterne; i++){
                if (StyleDunkel)
                  s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_gr_dark_24.png' width='24' border=0>";
                else
                  s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_gr_24.png' width='24' border=0>";
              }
              contextVariables["Sterne"] = s;
            }

            // Glas
            s = "<div style='background-color:" + farbe.name() +";padding:0px;margin:0px;width:100%;height:100%'>";
            if (StyleDunkel)
              s += "<img style='padding:0px;margin:0px;width:100%;height:100%' src='qrc:/global/bier_dark_200x200.png' alt='Bierfarbe'>";
            else
              s += "<img style='padding:0px;margin:0px;width:100%;height:100%' src='qrc:/global/bier_200x200.png' alt='Bierfarbe'>";
            s += "</div>";
            contextVariables["Glas"] = s;

            s = "<div style='background-color:" + farbe.name() +";padding:0px;margin:0px;width:100%;height:100%'>";
              s += "<img style='padding:0px;margin:0px;width:100%;height:100%' src='qrc:/global/bier_200x200.png' alt='Bierfarbe'>";
            s += "</div>";
            contextVariables["Glas-hell"] = s;

            // Glas
            s = "<div style='background-color:" + farbe.name() +";padding:0px;margin:0px;width:100%;height:100%'>";
            s += "<img style='padding:0px;margin:0px;width:100%;height:100%' src='qrc:/global/bier_dark_200x200.png' alt='Bierfarbe'>";
            s += "</div>";
            contextVariables["Glas-dunkel"] = s;

            //Solldaten des Rezeptes
            s = "<table><tbody>";
            FeldNr = query_sud.record().indexOf("Menge");
            s += "<tr>";
            s += "<td>" + trUtf8("Menge") + "</td>";
            s += "<td class='value' align='right'>" + QString::number(query_sud.value(FeldNr).toInt()) + "</td>";
            s += "<td>" + trUtf8("Liter") + "</td>";
            s += "</tr>";
            FeldNr = query_sud.record().indexOf("SW");
            s += "<tr>";
            s += "<td>" + trUtf8("Stammwürze") + "</td>";
            s += "<td class='value' align='right'>" + QString::number(query_sud.value(FeldNr).toDouble()) + "</td>";
            s += "<td>" + trUtf8("°P") + "</td>";
            s += "</tr>";
            FeldNr = query_sud.record().indexOf("IBU");
            s += "<tr>";
            s += "<td>" + trUtf8("Bittere") + "</td>";
            s += "<td class='value' align='right'>" + QString::number(query_sud.value(FeldNr).toInt()) + "</td>";
            s += "<td>" + trUtf8("IBU") + "</td>";
            s += "</tr>";
            FeldNr = query_sud.record().indexOf("erg_Farbe");
            s += "<tr>";
            s += "<td>" + trUtf8("Farbe") + "</td>";
            s += "<td class='value' align='right'>" + QString::number(query_sud.value(FeldNr).toDouble()) + "</td>";
            s += "<td>" + trUtf8("EBC") + "</td>";
            s += "</tr>";
            FeldNr = query_sud.record().indexOf("CO2");
            s += "<tr>";
            s += "<td>" + trUtf8("CO₂-Gehalt") + "</td>";
            s += "<td class='value' align='right'>" + QString::number(query_sud.value(FeldNr).toDouble()) + "</td>";
            s += "<td>" + trUtf8("g/l") + "</td>";
            s += "</tr>";
            s += "</tbody></table>";
            contextVariables["Rezept"] = s;

            // Kommentar
            FeldNr = query_sud.record().indexOf("Kommentar");
            contextVariables["Kommentar"] = query_sud.value(FeldNr).toString();
          }
        }
      }
    }
    else {
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
    }

    if (NeuBerechnen <= 0){
      //Benötigte Rohstoffe mit Vorhandenen verechnen

      //Alle Malzeinträge abrufen
      QList<Rohstoff> ListMalz;
      for (int sid = 0; sid < ListSudID.size(); ++sid){
        //Schüttung Abfragen
        QSqlQuery query_Malz;
        query_Malz.prepare("SELECT * FROM Malzschuettung WHERE SudID=:sudid");
        query_Malz.bindValue(":sudid", ListSudID.at(sid));
        if (!query_Malz.exec()) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_Malz.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + query_Malz.lastQuery());
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
        query_Hopfen.prepare("SELECT * FROM Hopfengaben WHERE SudID=:sudid");
        query_Hopfen.bindValue(":sudid", ListSudID.at(sid));
        if (!query_Hopfen.exec()) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_Hopfen.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + query_Hopfen.lastQuery());
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
        query_Hopfen.prepare("SELECT * FROM WeitereZutatenGaben WHERE SudID=:sudid AND Typ=100;");
        query_Hopfen.bindValue(":sudid", ListSudID.at(sid));
        if (!query_Hopfen.exec()) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_Hopfen.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + query_Hopfen.lastQuery());
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
        query_Hefe.prepare("SELECT AuswahlHefe,HefeAnzahlEinheiten FROM Sud WHERE ID=:sudid");
        query_Hefe.bindValue(":sudid", ListSudID.at(sid));
        if (!query_Hefe.exec()) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_Hefe.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + query_Hefe.lastQuery());
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
        QSqlQuery query_weitereZutaten;
        query_weitereZutaten.prepare("SELECT * FROM WeitereZutatenGaben WHERE SudID=:sudid AND Typ=0;");
        query_weitereZutaten.bindValue(":sudid", ListSudID.at(sid));
        if (!query_weitereZutaten.exec()) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + query_weitereZutaten.lastQuery());
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
        QSqlQuery query_weitereZutaten;
        query_weitereZutaten.prepare("SELECT * FROM WeitereZutatenGaben WHERE SudID=:sudid AND Typ=1;");
        query_weitereZutaten.bindValue(":sudid", ListSudID.at(sid));
        if (!query_weitereZutaten.exec()) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + query_weitereZutaten.lastQuery());
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
        QSqlQuery query_weitereZutaten;
        query_weitereZutaten.prepare("SELECT * FROM WeitereZutatenGaben WHERE SudID=:sudid AND Typ=2;");
        query_weitereZutaten.bindValue(":sudid", ListSudID.at(sid));
        if (!query_weitereZutaten.exec()) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + query_weitereZutaten.lastQuery());
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
        QSqlQuery query_weitereZutaten;
        query_weitereZutaten.prepare("SELECT * FROM WeitereZutatenGaben WHERE SudID=:sudid AND Typ=3;");
        query_weitereZutaten.bindValue(":sudid", ListSudID.at(sid));
        if (!query_weitereZutaten.exec()) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + query_weitereZutaten.lastQuery());
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
        QSqlQuery query_weitereZutaten;
        query_weitereZutaten.prepare("SELECT * FROM WeitereZutatenGaben WHERE SudID=:sudid AND Typ=4;");
        query_weitereZutaten.bindValue(":sudid", ListSudID.at(sid));
        if (!query_weitereZutaten.exec()) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                      + trUtf8("\nSQL-Befehl:\n") + query_weitereZutaten.lastQuery());
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
      contextVariables["Rohstoffe"] = trUtf8("Rohstoffe");

      //Malz
      if (ListMalz.count() > 0){
          s = "<table width='100%'>";
          s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + " (" + trUtf8("kg") + ")" + "</td><td align='center'>" + trUtf8("vorhanden") + " (" + trUtf8("kg") + ")" + "</td>";
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
            //double rest = ist - ListMalz.at(i).Menge;
            s += "<tr valign='middle'>";
            if (ist < ListMalz.at(i).Menge)
              s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
            else
              s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
            //Rohstoff ist aufgeführt
            if (gefunden){
              s += "<td align='left'>" + ListMalz.at(i).Name + "</td>";
            }
            //Rohstoff ist nicht vorhanden
            else {
              s += "<td align='left' style='color: grey;'>" + ListMalz.at(i).Name + "</td>";
            }
            s += "<td align='center'>" + QString::number(ListMalz.at(i).Menge) + "</td>";
            if (ist < ListMalz.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist) + "</b></td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist) + "</b></td>";
            s += "</tr>";
          }
          s += "</table>";
          contextVariables["Malz"] = s;
      }
      //Malz mit Restanzeige
      if (ListMalz.count() > 0){
          s = "<table width='100%'>";
          s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + " (" + trUtf8("kg") + ")" + "</td><td align='center'>" + trUtf8("rest") + " (" + trUtf8("kg") + ")" + "</td>";
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
            //double rest = ist - ListMalz.at(i).Menge;
            s += "<tr valign='middle'>";
            if (ist < ListMalz.at(i).Menge)
              s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
            else
              s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
            //Rohstoff ist aufgeführt
            if (gefunden){
              s += "<td align='left'>" + ListMalz.at(i).Name + "</td>";
            }
            //Rohstoff ist nicht vorhanden
            else {
              s += "<td align='left' style='color: grey;'>" + ListMalz.at(i).Name + "</td>";
            }
            s += "<td align='center'>" + QString::number(ListMalz.at(i).Menge) + "</td>";
            if (ist < ListMalz.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist - ListMalz.at(i).Menge) + "</b></td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist - ListMalz.at(i).Menge) + "</b></td>";
            s += "</tr>";
          }
          s += "</table>";
          contextVariables["Malz-rest"] = s;
      }


      //Hopfen Mengen anzeigen
      //Bild für Hopfen anzeigen
      if (ListHopfen.count() > 0){
          s = "<table width='100%'>";
          s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + " (" + trUtf8("g") + ")" + "</td><td align='center'>" + trUtf8("vorhanden") + " (" + trUtf8("g") + ")" + "</td>";
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
            s += "<tr valign='middle'>";
            if (ist < ListHopfen.at(i).Menge)
              s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
            else
              s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
            //Rohstoff ist aufgeführt
            if (gefunden)
              s += "<td align='left'>" + ListHopfen.at(i).Name + "</td>";
            else
              s += "<td align='left' style='color: grey;'>" + ListHopfen.at(i).Name + "</td>";
            s += "<td align='center'>" + QString::number(ListHopfen.at(i).Menge) + "</td>";
            if (ist < ListHopfen.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist) + "</b></td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist) + "</b></td>";
            s += "</tr>";
          }
          s += "</table>";
          contextVariables["Hopfen"] = s;
      }

      //Hopfen Mengen mit Rest anzeigen
      //Bild für Hopfen anzeigen
      if (ListHopfen.count() > 0){
          s = "<table width='100%'>";
          s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + " (" + trUtf8("g") + ")" + "</td><td align='center'>" + trUtf8("rest") + " (" + trUtf8("g") + ")" + "</td>";
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
            s += "<tr valign='middle'>";
            if (ist < ListHopfen.at(i).Menge)
              s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
            else
              s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
            //Rohstoff ist aufgeführt
            if (gefunden)
              s += "<td align='left'>" + ListHopfen.at(i).Name + "</td>";
            else
              s += "<td align='left' style='color: grey;'>" + ListHopfen.at(i).Name + "</td>";
            s += "<td align='center'>" + QString::number(ListHopfen.at(i).Menge) + "</td>";
            if (ist < ListHopfen.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist - ListHopfen.at(i).Menge) + "</b></td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist - ListHopfen.at(i).Menge) + "</b></td>";
            s += "</tr>";
          }
          s += "</table>";
          contextVariables["Hopfen-rest"] = s;
      }

      //Hefe Mengen anzeigen
      //Bild für Hefe anzeigen
      if (ListHefe.count() > 0){
          s = "<table width='100%'>";
          s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("vorhanden") + "</td>";
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
            s += "<tr valign='middle'>";
            if (ist < ListHefe.at(i).Menge)
              s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
            else
              s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
            //Rohstoff ist aufgeführt
            if (gefunden)
              s += "<td align='left'>" + ListHefe.at(i).Name + "</td>";
            else
              s += "<td align='left' style='color: grey;'>" + ListHefe.at(i).Name + "</td>";
            s += "<td align='center'>" + QString::number(ListHefe.at(i).Menge) + "</td>";
            if (ist < ListHefe.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist) + "</b></td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist) + "</b></td>";
            s += "</tr>";
          }
          s += "</table>";
          contextVariables["Hefe"] = s;
      }

      //Hefe Mengen mit Rest anzeigen
      //Bild für Hefe anzeigen
      if (ListHefe.count() > 0){
          s = "<table width='100%'>";
          s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("rest") + "</td>";
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
            s += "<tr valign='middle'>";
            if (ist < ListHefe.at(i).Menge)
              s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
            else
              s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
            //Rohstoff ist aufgeführt
            if (gefunden)
              s += "<td align='left'>" + ListHefe.at(i).Name + "</td>";
            else
              s += "<td align='left' style='color: grey;'>" + ListHefe.at(i).Name + "</td>";
            s += "<td align='center'>" + QString::number(ListHefe.at(i).Menge) + "</td>";
            if (ist < ListHefe.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist - ListHefe.at(i).Menge) + "</b></td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist - ListHefe.at(i).Menge) + "</b></td>";
            s += "</tr>";
          }
          s += "</table>";
          contextVariables["Hefe-rest"] = s;
      }

      //WeitereZutaten Honig Mengen anzeigen
      //Bild für Honig anzeigen
      if (ListWeitereZutatenHonig.count() > 0){
        int Einheit=0;
        s = "<table width='100%'>";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("vorhanden") + "</td>";
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
          s += "<tr valign='middle'>";
          if (ist < ListWeitereZutatenHonig.at(i).Menge)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden)
            s += "<td align='left'>" + ListWeitereZutatenHonig.at(i).Name + "</td>";
          else
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenHonig.at(i).Name + "</td>";
          if (Einheit == EWZ_Einheit_Kg){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenHonig.at(i).Menge / 1000) + " kg</td>";
            if (ist < ListWeitereZutatenHonig.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenHonig.at(i).Menge) + " g</td>";
            if (ist < ListWeitereZutatenHonig.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        contextVariables["Honig"] = s;
      }


      //WeitereZutaten Honig rest Mengen anzeigen
      //Bild für Honig anzeigen
      if (ListWeitereZutatenHonig.count() > 0){
        int Einheit=0;
        s = "<table width='100%'>";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("rest") + "</td>";
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
          s += "<tr valign='middle'>";
          if (ist < ListWeitereZutatenHonig.at(i).Menge)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden)
            s += "<td align='left'>" + ListWeitereZutatenHonig.at(i).Name + "</td>";
          else
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenHonig.at(i).Name + "</td>";
          if (Einheit == EWZ_Einheit_Kg){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenHonig.at(i).Menge / 1000) + " kg</td>";
            if (ist < ListWeitereZutatenHonig.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number((ist - ListWeitereZutatenHonig.at(i).Menge)/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number((ist - ListWeitereZutatenHonig.at(i).Menge)/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenHonig.at(i).Menge) + " g</td>";
            if (ist < ListWeitereZutatenHonig.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist - ListWeitereZutatenHonig.at(i).Menge) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist - ListWeitereZutatenHonig.at(i).Menge) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        contextVariables["Honig-rest"] = s;
      }


      //WeitereZutaten Zucker Mengen anzeigen
      //Bild für Zucker anzeigen
      if (ListWeitereZutatenZucker.count() > 0){
        int Einheit=0;
        s = "<table width='100%'>";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("vorhanden") + "</td>";
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
          s += "<tr valign='middle'>";
          if (ist < ListWeitereZutatenZucker.at(i).Menge)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden)
            s += "<td align='left'>" + ListWeitereZutatenZucker.at(i).Name + "</td>";
          else
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenZucker.at(i).Name + "</td>";
          if (Einheit == 0){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenZucker.at(i).Menge / 1000) + " kg</td>";
            if (ist < ListWeitereZutatenZucker.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenZucker.at(i).Menge) + " g</td>";
            if (ist < ListWeitereZutatenZucker.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        contextVariables["Zucker"] = s;
      }

      //WeitereZutaten Zucker Mengen mit Rest anzeigen
      //Bild für Zucker anzeigen
      if (ListWeitereZutatenZucker.count() > 0){
        int Einheit=0;
        s = "<table width='100%'>";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("rest") + "</td>";
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
          s += "<tr valign='middle'>";
          if (ist < ListWeitereZutatenZucker.at(i).Menge)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden)
            s += "<td align='left'>" + ListWeitereZutatenZucker.at(i).Name + "</td>";
          else
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenZucker.at(i).Name + "</td>";
          if (Einheit == 0){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenZucker.at(i).Menge / 1000) + " kg</td>";
            if (ist < ListWeitereZutatenZucker.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number((ist - ListWeitereZutatenZucker.at(i).Menge)/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number((ist - ListWeitereZutatenZucker.at(i).Menge)/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenZucker.at(i).Menge) + " g</td>";
            if (ist < ListWeitereZutatenZucker.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist - ListWeitereZutatenZucker.at(i).Menge) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist - ListWeitereZutatenZucker.at(i).Menge) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        contextVariables["Zucker-rest"] = s;
      }


      //WeitereZutaten Gewuerz Mengen anzeigen
      //Bild für Gewuerz anzeigen
      if (ListWeitereZutatenGewuerz.count() > 0){
        int Einheit=0;
        s = "<table width='100%'>";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("vorhanden") + "</td>";
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
          s += "<tr valign='middle'>";
          if (ist < ListWeitereZutatenGewuerz.at(i).Menge)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden)
            s += "<td align='left'>" + ListWeitereZutatenGewuerz.at(i).Name + "</td>";
          else
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenGewuerz.at(i).Name + "</td>";
          if (Einheit == 0){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenGewuerz.at(i).Menge / 1000) + " kg</td>";
            if (ist < ListWeitereZutatenGewuerz.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenGewuerz.at(i).Menge) + " g</td>";
            if (ist < ListWeitereZutatenGewuerz.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        contextVariables["Gewuerz"] = s;
      }

      //WeitereZutaten Gewuerz mit Rest Mengen anzeigen
      //Bild für Gewuerz anzeigen
      if (ListWeitereZutatenGewuerz.count() > 0){
        int Einheit=0;
        s = "<table width='100%'>";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("rest") + "</td>";
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
          s += "<tr valign='middle'>";
          if (ist < ListWeitereZutatenGewuerz.at(i).Menge)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden)
            s += "<td align='left'>" + ListWeitereZutatenGewuerz.at(i).Name + "</td>";
          else
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenGewuerz.at(i).Name + "</td>";
          if (Einheit == 0){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenGewuerz.at(i).Menge / 1000) + " kg</td>";
            if (ist < ListWeitereZutatenGewuerz.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number((ist - ListWeitereZutatenGewuerz.at(i).Menge)/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number((ist - ListWeitereZutatenGewuerz.at(i).Menge)/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenGewuerz.at(i).Menge) + " g</td>";
            if (ist < ListWeitereZutatenGewuerz.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist - ListWeitereZutatenGewuerz.at(i).Menge) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist - ListWeitereZutatenGewuerz.at(i).Menge) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        contextVariables["Gewuerz-rest"] = s;
      }


      //WeitereZutaten Frucht Mengen anzeigen
      //Bild für Frucht anzeigen
      if (ListWeitereZutatenFrucht.count() > 0){
        int Einheit=0;
        s = "<table width='100%'>";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("vorhanden") + "</td>";
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
          s += "<tr valign='middle'>";
          if (ist < ListWeitereZutatenFrucht.at(i).Menge)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden)
            s += "<td align='left'>" + ListWeitereZutatenFrucht.at(i).Name + "</td>";
          else
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenFrucht.at(i).Name + "</td>";
          if (Einheit == 0){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenFrucht.at(i).Menge / 1000) + " kg</td>";
            if (ist < ListWeitereZutatenFrucht.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenFrucht.at(i).Menge) + " g</td>";
            if (ist < ListWeitereZutatenFrucht.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
      contextVariables["Frucht"] = s;
      }

      //WeitereZutaten Frucht mit Rest Mengen anzeigen
      //Bild für Frucht anzeigen
      if (ListWeitereZutatenFrucht.count() > 0){
        int Einheit=0;
        s = "<table width='100%'>";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("rest") + "</td>";
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
          s += "<tr valign='middle'>";
          if (ist < ListWeitereZutatenFrucht.at(i).Menge)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden)
            s += "<td align='left'>" + ListWeitereZutatenFrucht.at(i).Name + "</td>";
          else
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenFrucht.at(i).Name + "</td>";
          if (Einheit == 0){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenFrucht.at(i).Menge / 1000) + " kg</td>";
            if (ist < ListWeitereZutatenFrucht.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number((ist - ListWeitereZutatenFrucht.at(i).Menge)/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number((ist - ListWeitereZutatenFrucht.at(i).Menge)/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenFrucht.at(i).Menge) + " g</td>";
            if (ist < ListWeitereZutatenFrucht.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist - ListWeitereZutatenFrucht.at(i).Menge) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist - ListWeitereZutatenFrucht.at(i).Menge) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
      contextVariables["Frucht-rest"] = s;
      }


      //WeitereZutaten Sonstiges Mengen anzeigen
      //Bild für Sonstiges anzeigen
      if (ListWeitereZutatenSonstiges.count() > 0){
        int Einheit=0;
        s = "<table width='100%'>";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("vorhanden") + "</td>";
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
          s += "<tr valign='middle'>";
          if (ist < ListWeitereZutatenSonstiges.at(i).Menge)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden)
            s += "<td align='left'>" + ListWeitereZutatenSonstiges.at(i).Name + "</td>";
          else
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenSonstiges.at(i).Name + "</td>";
          if (Einheit == 0){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenSonstiges.at(i).Menge / 1000) + " kg</td>";
            if (ist < ListWeitereZutatenSonstiges.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenSonstiges.at(i).Menge) + " g</td>";
            if (ist < ListWeitereZutatenSonstiges.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        contextVariables["Sonstiges"] = s;
      }


      //WeitereZutaten Sonstiges Mengen mit Rest anzeigen
      //Bild für Sonstiges anzeigen
      if (ListWeitereZutatenSonstiges.count() > 0){
        int Einheit=0;
        s = "<table width='100%'>";
        s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("rest") + "</td>";
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
          s += "<tr valign='middle'>";
          if (ist < ListWeitereZutatenSonstiges.at(i).Menge)
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
          else
            s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
          //Rohstoff ist aufgeführt
          if (gefunden)
            s += "<td align='left'>" + ListWeitereZutatenSonstiges.at(i).Name + "</td>";
          else
            s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenSonstiges.at(i).Name + "</td>";
          if (Einheit == 0){
            s += "<td align='center'>" + QString::number(ListWeitereZutatenSonstiges.at(i).Menge / 1000) + " kg</td>";
            if (ist < ListWeitereZutatenSonstiges.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number((ist - ListWeitereZutatenSonstiges.at(i).Menge)/1000) + "</b> kg</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number((ist - ListWeitereZutatenSonstiges.at(i).Menge)/1000) + "</b> kg</td>";
          }
          else {
            s += "<td align='center'>" + QString::number(ListWeitereZutatenSonstiges.at(i).Menge) + " g</td>";
            if (ist < ListWeitereZutatenSonstiges.at(i).Menge)
              s += "<td align='center' style='color: red;'><b>" + QString::number(ist - ListWeitereZutatenSonstiges.at(i).Menge) + "</b> g</td>";
            else
              s += "<td align='center' style='color: green;'><b>" + QString::number(ist - ListWeitereZutatenSonstiges.at(i).Menge) + "</b> g</td>";
          }

          s += "</tr>";
        }
        s += "</table>";
        contextVariables["Sonstiges-rest"] = s;
      }

    }


    QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
    settings.beginGroup("DB");
    QDir dbpfad = QDir(settings.value("DB_Pfad").toString());
    settings.endGroup();
    contextVariables["AnhangTitel"] = trUtf8("Anhänge");
    s = "";
    for (int sid = 0; sid < ListSudID.size(); ++sid){
      QSqlQuery query_anhang;
      query_anhang.prepare("SELECT * FROM Anhang WHERE SudID=:sudid");
      query_anhang.bindValue(":sudid", ListSudID.at(sid));
      if (!query_anhang.exec()) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_anhang.lastError().databaseText()
                                    + trUtf8("\nSQL-Befehl:\n") + query_anhang.lastQuery());
      }
      else {
        while (query_anhang.next()){
          int FeldNr = query_anhang.record().indexOf("Pfad");
          QString pfad = query_anhang.value(FeldNr).toString();
          if (QDir::isRelativePath(pfad))
            pfad = dbpfad.filePath(pfad);
          if (AnhangWidget::isImage(pfad))
            s += "<img style=\"max-width:90%;\" src=\"file:///" + pfad + "\"></br></br>";
          else
            s += "<a href=\"file:///" + pfad + "\">" + pfad + "</a></br></br>";
        }
      }
    }
    contextVariables["Anhang"] = s;

    QString settingsPath = QFileInfo(settings.fileName()).absolutePath() + "/";
    QFile file(settingsPath + "sudinfo.html");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString html_template = file.readAll();
    file.close();

    Mustache::Renderer renderer;
    Mustache::QtVariantContext context(contextVariables);
    QString seite = renderer.render(html_template, &context);
    if (webView_Info->url().isEmpty())
        MyWebView::clearMemoryCaches();
    webView_Info->setHtml(seite, QUrl::fromLocalFile(settingsPath));
}
