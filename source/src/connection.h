#ifndef CONNECTION_H
#define CONNECTION_H

#include <QApplication>
#include <QSqlDatabase>
#include <QFile>
#include <QString>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDir>
#include <QSettings>

#include "definitionen.h"
#include "errormessage.h"
#include <time.h>


//Überprüft Datenbankeinträge auf Fehler
void CheckDB(){
  QSqlDatabase::database().transaction();

  //Entfernt in den Rohstofflisten eventuell vorhanden leer einträge
  QSqlQuery query;
  QString sql = "DELETE FROM 'Malz' WHERE Beschreibung = \"\"";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
  }
  sql = "DELETE FROM 'Hopfen' WHERE Beschreibung = \"\"";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
  }
  sql = "DELETE FROM 'Hefe' WHERE Beschreibung = \"\"";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
  }
  sql = "DELETE FROM 'WeitereZutaten' WHERE Beschreibung = \"\"";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
  }

  QSqlDatabase::database().commit();
}

//Macht eine Sicherheitskopie der Datenbank
bool BackupDatenbank(const QString& src, const QString& dst){

  if (src.isEmpty() || dst.isEmpty() || src == dst)
    return false;

  //Wenn Backupdatei schon existiert löschen
  if (QFile::exists(dst)) {
    QFile::remove(dst);
  }

  //Datenbank kopieren
  QFile file(src);
  if (file.copy(dst)) {
    return true;
  }
  else {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_DB_KOPIE_BACKUP, TYPE_KRITISCH,
      CANCEL_PROGRAM, QObject::tr("Betroffener Kopierpfad:\n") + dst);
    return false;
  }
}

//Datenbanksicherungskopie löschen
bool RemoveDatenbanksicherung(const QString& file){
  if (QFile::exists(file)) {
    QFile::remove(file);
  }
  return true;
}

//Stellt die Kopierte Datenbank wieder her (wenn sie vorhanden ist)
/*
bool RestorDatenbank(){
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("DB");
  QString dbPfadOhneDateiName = settings.value("DB_Pfad").toString();
  settings.endGroup();

  QString dbPfad = dbPfadOhneDateiName + "/" + DB_USER_NAME;

  //Überprüfen ob Backupdatei existiert
  if (QFile::exists(dbPfad + "~")) {
    //Datenbank löschen
    QSqlDatabase::database().removeDatabase(dbPfad);
    QSqlDatabase::database().close();
    QFile::remove(dbPfad);
    //Datenbank zurück kopieren
    QFile file(dbPfad + "~");
    if (file.copy(dbPfad)) {
      return true;
    }
    else {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_DB_KOPIE_RESTORE, TYPE_KRITISCH,
        CANCEL_PROGRAM, QObject::tr("Betroffener Kopierpfad:\n") + dbPfad + "~");
      return false;
    }
  }
  return true;
}
*/

static bool UpdateDB_v1_v2(){
  QSqlDatabase::database().transaction();
  //Tabelle für Schnellgärverlauf hinzufügen
  QSqlQuery query;
  QString sql = "CREATE TABLE IF NOT EXISTS 'Schnellgaerverlauf' ('ID' INTEGER PRIMARY KEY  NOT NULL ,'SudID' INTEGER,'Zeitstempel' DATETIME,'SW' NUMERIC DEFAULT (0) ,'Alc' NUMERIC DEFAULT (0) )";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Tabelle für Hauptgärverlauf hinzufügen
  sql = "CREATE TABLE IF NOT EXISTS 'Hauptgaerverlauf' ('ID' INTEGER PRIMARY KEY  NOT NULL ,'SudID' INTEGER,'Zeitstempel' DATETIME,'SW' NUMERIC DEFAULT (0) ,'Alc' NUMERIC DEFAULT (0) )";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Tabelle für Nachgärverlauf hinzufügen
  sql = "CREATE TABLE IF NOT EXISTS  'Nachgaerverlauf' ('ID' INTEGER PRIMARY KEY  NOT NULL ,'SudID' INTEGER,'Zeitstempel' DATETIME,'Druck' NUMERIC DEFAULT (0) ,'Temp' NUMERIC DEFAULT (0) ,'CO2' NUMERIC DEFAULT (0))";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Versionsstand auf 2 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=2";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v2_v3(){
  QSqlDatabase::database().transaction();
  //Eintrag hinzufügen zum Ablegen welcher Tab im Gärverlauf aktiv ist
  QSqlQuery query;
  QString sql = "ALTER TABLE 'Sud' ADD COLUMN 'AktivTab_Gaerverlauf' INTEGER DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Versionsstand auf 3 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=3";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v3_v4(){
  QSqlDatabase::database().transaction();
  //Eintrag hinzufügen für Dauer geplante Reifezeit
  QSqlQuery query;
  QString sql = "ALTER TABLE 'Sud' ADD COLUMN 'Reifezeit' INTEGER DEFAULT 4";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Versionsstand auf 4 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=4";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v4_v5(){
  QSqlDatabase::database().transaction();
  //Eintrag hinzufügen Bier Wurde Verbraucht
  QSqlQuery query;
  QString sql = "ALTER TABLE 'Sud' ADD COLUMN 'BierWurdeVerbraucht' BOOL DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Versionsstand auf 5 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=5";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v5_v6(){
  QSqlDatabase::database().transaction();
  QSqlQuery query;
  //Eintrag Nachisomerisierungs-zeit Hinzufügen
  QString sql = "ALTER TABLE 'Sud' ADD COLUMN 'Nachisomerisierungszeit' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Eintrag Würzemenge vor dem Hopfenseihen
  sql = "ALTER TABLE 'Sud' ADD COLUMN 'WuerzemengeVorHopfenseihen' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Eintrag Stammwürze vor dem Hopfenseihen
  sql = "ALTER TABLE 'Sud' ADD COLUMN 'SWVorHopfenseihen' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Eintrag Höhe Maischebottich Hinzufügen
  sql = "ALTER TABLE 'Ausruestung' ADD COLUMN 'Maischebottich_Hoehe' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Eintrag Durchmesser Maischebottich Hinzufügen
  sql = "ALTER TABLE 'Ausruestung' ADD COLUMN 'Maischebottich_Durchmesser' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Eintrag Maximale Füllhöge Maischebottich Hinzufügen
  sql = "ALTER TABLE 'Ausruestung' ADD COLUMN 'Maischebottich_MaxFuellhoehe' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Eintrag Höhe Sudpfanne Hinzufügen
  sql = "ALTER TABLE 'Ausruestung' ADD COLUMN 'Sudpfanne_Hoehe' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Eintrag Durchmesser Sudpfanne Hinzufügen
  sql = "ALTER TABLE 'Ausruestung' ADD COLUMN 'Sudpfanne_Durchmesser' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Eintrag Maximale Füllhöge Sudpfanne Hinzufügen
  sql = "ALTER TABLE 'Ausruestung' ADD COLUMN 'Sudpfanne_MaxFuellhoehe' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Versionsstand auf 6 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=6";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v6_v7(){
  QSqlDatabase::database().transaction();
  QSqlQuery query;
  //Eintrag Ergebniss für Effektive Ausbeute der Anlage hinzufügen
  QString sql = "ALTER TABLE 'Sud' ADD COLUMN 'erg_EffektiveAusbeute' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Eintrag für Flag das DB Ergebnisse Neu Berechnet werden müssen
  sql = "ALTER TABLE 'Global' ADD COLUMN 'db_NeuBerechnen' INTEGER DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Flag setzen das DB neu Berechnet werden muss
  sql = "UPDATE 'Global' SET 'db_NeuBerechnen'=1";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Versionsstand auf 7 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=7";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v7_v8(){
  QSqlQuery query;
  QSqlDatabase::database().transaction();

  //Tabelle für Wasseranalysewerte erstellen
  QString sql = "CREATE TABLE IF NOT EXISTS 'Wasser' ('ID' INTEGER PRIMARY KEY ,'Calcium' NUMERIC DEFAULT (0),'Magnesium' NUMERIC DEFAULT (0),'Saeurekapazitaet' NUMERIC DEFAULT (0))";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Ersten Eintrag anlegen
  sql = "INSERT INTO 'Wasser' DEFAULT VALUES";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Eintrag für die gewünschte Restalkalität den Suddaten hinzufügen
  sql = "ALTER TABLE 'Sud' ADD COLUMN 'RestalkalitaetSoll' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Rastbeschreibungen austauschen mit Tempereaturbeschreibung
  sql = QObject::trUtf8("UPDATE 'Rastauswahl' SET 'Text' ='Einmaischen' WHERE ID=1");
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  sql = QObject::trUtf8("UPDATE 'Rastauswahl' SET 'Text' ='Gummirast (35°-40°)' WHERE ID=2");
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  sql = QObject::trUtf8("UPDATE 'Rastauswahl' SET 'Text' ='Weizenrast (45°)' WHERE ID=3");
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  sql = QObject::trUtf8("UPDATE 'Rastauswahl' SET 'Text' ='Eiweißrast (57°)' WHERE ID=4");
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  sql = QObject::trUtf8("UPDATE 'Rastauswahl' SET 'Text' ='Maltoserast (60°-65°)' WHERE ID=5");
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  sql = QObject::trUtf8("UPDATE 'Rastauswahl' SET 'Text' ='Kombirast (66°-69°)' WHERE ID=6");
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  sql = QObject::trUtf8("INSERT INTO 'Rastauswahl'  ('Text') VALUES ('Verzuckerung (70°-75°)')");
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  sql = QObject::trUtf8("INSERT INTO 'Rastauswahl'  ('Text') VALUES ('Abmaischen (78°)')");
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Eintrag vorderwürzerast in Hopfengaben hinzufügen
  sql = "ALTER TABLE 'Hopfengaben' ADD COLUMN 'Vorderwuerze' BOOL DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Die Zeitangaben aller bestehenden Sud in Kochzeiten umrechnen
  sql = "SELECT * FROM Sud";

  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  else {
    QSqlQuery query2, query3;
    int KochdauerNachBitterhopfung;
    int ZeitGabe;
    int ZeitEiweisbruch=0;
    //bool Aktiv;
    int id, id_Hopfen;
    int FeldNr;
    QString sql2, sql3;
    //Alle Sude durchgehen
    while (query.next()) {
      //ID von dem Datensatz der gerade berechnet wird
      FeldNr = query.record().indexOf("ID");
      id = query.value(FeldNr).toInt();

      //Kochdauer Nach Bitterhopfung
      FeldNr = query.record().indexOf("KochdauerNachBitterhopfung");
      KochdauerNachBitterhopfung = query.value(FeldNr).toDouble();

      //Zeiten der Hopfengaben abfragen
      sql2 = "SELECT * FROM Hopfengaben WHERE SudID=" + QString::number(id) + ";";
      if (!query2.exec(sql2)) {
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
          CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
          + QObject::trUtf8("\nSQL-Befehl:\n") + sql2);
        return false;
      }
      else {
        int i = 1;
        while (query2.next()){
          int FeldNr_Zeit = query2.record().indexOf("Zeit");
          int FeldNr_ID = query2.record().indexOf("ID");
          ZeitGabe = query2.value(FeldNr_Zeit).toInt();
          //Aktiv = query2.value(FeldNrAktiv).toBool();
          id_Hopfen = query2.value(FeldNr_ID).toInt();
          if (i == 1){
            ZeitEiweisbruch = ZeitGabe;
            ZeitGabe = KochdauerNachBitterhopfung;
          }
          else {
            ZeitGabe = KochdauerNachBitterhopfung - ZeitGabe;
          }
          //Neue Zeit in DB Schreiben
          sql3 = "UPDATE 'Hopfengaben' SET 'Zeit'=" + QString::number(ZeitGabe)
              + " WHERE ID=" + QString::number(id_Hopfen);
          if (!query3.exec(sql3)) {
            ErrorMessage *errorMessage = new ErrorMessage();
            errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
              CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
              + QObject::trUtf8("\nSQL-Befehl:\n") + sql3);
            return false;
          }
          i++;
        }
      }

      KochdauerNachBitterhopfung += ZeitEiweisbruch;
      //Neue Gesammtzeit in DB Schreiben
      sql3 = "UPDATE 'Sud' SET 'KochdauerNachBitterhopfung'=" + QString::number(KochdauerNachBitterhopfung)
          + " WHERE ID=" + QString::number(id);
      if (!query3.exec(sql3)) {
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
          CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
          + QObject::trUtf8("\nSQL-Befehl:\n") + sql3);
        return false;
      }
    }
  }

  //Für alle Bestehenden Sude einen zusätzlichen Hopfeneintrag für die Vorderwürzehopfung anlegen
  sql = "SELECT ID FROM SUD";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  while (query.next()){
    sql = "INSERT INTO 'Hopfengaben' ('SudID','Vorderwuerze') VALUES ('"+query.value(0).toString()+"', '1')";
    QSqlQuery query_2;
    if (!query_2.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      return false;
    }
  }

  //Versionsstand auf 8 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=8";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v8_v9(){
  QSqlDatabase::database().transaction();
  QSqlQuery query,query2;
  QString sql;
  QString SudID;
  //Den Bug von Version 1.0.0.0 ausbügeln
  //Alle Sude durchgehen und überprüfen ob der Vorderwürzehopfung Eintrag nicht gesetzt ist.
  sql = "SELECT ID FROM SUD";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  while (query.next()){
    //Überprüfen ob bei diesem Sud der Vorderwürzeeintrag fehlt
    SudID = query.value(0).toString();
    sql = "SELECT * FROM Hopfengaben WHERE SudID = " + SudID + " AND Vorderwuerze=1";
    if (!query2.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    }
    else {
      if (!query2.first()){
        //Letzte Hopfengabe als Vorderwürzehopfung markieren
        sql = "SELECT * FROM Hopfengaben WHERE SudID = " + SudID;
        if (!query2.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
            CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
            + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
        }
        else {
          if (query2.last()){
            int FeldNr = query2.record().indexOf("ID");
            QString ID = query2.value(FeldNr).toString();
            sql = "UPDATE 'Hopfengaben' SET 'Vorderwuerze' ='1' WHERE ID=" + ID;
            if (!query2.exec(sql)) {
              // Fehlermeldung Datenbankabfrage
              ErrorMessage *errorMessage = new ErrorMessage();
              errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
                + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
            }
          }
        }
      }
    }
  }

  //Versionsstand auf 9 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=9";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v9_v10(){
  QSqlDatabase::database().transaction();
  QSqlQuery query;
  QString sql;
  QString SudID;

  //für Rohstoffe neue Tabelle für die erweiterten zutaten erstellen
  sql = "CREATE  TABLE  IF NOT EXISTS 'WeitereZutaten' ('ID' INTEGER PRIMARY KEY  NOT NULL , 'Beschreibung' TEXT DEFAULT '', 'Menge' NUMERIC DEFAULT 0, 'Einheiten' INTEGER DEFAULT 0, 'Typ' INTEGER DEFAULT 0, 'Ausbeute' NUMERIC DEFAULT 0, 'EBC' NUMERIC DEFAULT 0, 'Preis' NUMERIC DEFAULT 0, 'Bemerkung' TEXT DEFAULT '')";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Beispielzutaten eintragen
  //Haushaltszucker
  sql = "INSERT INTO 'WeitereZutaten' ('Beschreibung','Einheiten','Typ','Ausbeute','EBC','Preis') VALUES ('Haushaltszucker',0,1,100,0,1)";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //WaldHonig
  sql = "INSERT INTO 'WeitereZutaten' ('Beschreibung','Einheiten','Typ','Ausbeute','EBC','Preis') VALUES ('Waldhonig',0,0,80,3,10)";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Ingwer
  sql = "INSERT INTO 'WeitereZutaten' ('Beschreibung','Einheiten','Typ','Ausbeute','EBC','Preis') VALUES ('Ingwer',1,2,0,0,4)";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Orangenschalen
  sql = "INSERT INTO 'WeitereZutaten' ('Beschreibung','Einheiten','Typ','Ausbeute','EBC','Preis') VALUES ('Orangenschalen',1,3,0,0,0)";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Zuckercouleur
  sql = "INSERT INTO 'WeitereZutaten' ('Beschreibung','Einheiten','Typ','Ausbeute','EBC','Preis') VALUES ('Zuckercouleur',1,4,0,20000,12)";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Tabelle für die Erweiterten Zutaten
  sql = "CREATE  TABLE  IF NOT EXISTS 'WeitereZutatenGaben' (";
  sql += "'ID' INTEGER PRIMARY KEY  NOT NULL,";
  sql += "'SudID' INTEGER,";
  sql += "'Name' TEXT DEFAULT '',";
  sql += "'Menge' NUMERIC DEFAULT 0," ;
  sql += "'Einheit' INTEGER DEFAULT 0,";
  sql += "'Typ' INTEGER DEFAULT 0,";
  sql += "'Zeitpunkt' NUMERIC DEFAULT 0,";
  sql += "'Bemerkung' TEXT DEFAULT '',";
  sql += "'erg_Menge' NUMERIC DEFAULT 0,";
  sql += "'Ausbeute' NUMERIC DEFAULT 0,";
  sql += "'Farbe' NUMERIC DEFAULT 0)";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Eintrag Schnellgärprobe aktiv den Suddaten hinzufügen
  sql = "ALTER TABLE 'Sud' ADD COLUMN 'SchnellgaerprobeAktiv' BOOL DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Bei allen bisher gebrauten Suden Schnellgärprobe auf Aktiv setzten
  sql = "UPDATE 'Sud' SET 'SchnellgaerprobeAktiv'='1' WHERE BierWurdeGebraut=1";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }


  //Versionsstand auf 10 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=10";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v10_v11(){
  QSqlQuery query;
  QString sql;
  QString SudID;

  QSqlDatabase::database().transaction();

  //Spalte Temperatur in Schnellgaerverlauf einfügen
  sql = "ALTER TABLE 'Schnellgaerverlauf' ADD COLUMN 'Temp' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Spalte Temperatur in Hauptgaerverlauf einfügen
  sql = "ALTER TABLE 'Hauptgaerverlauf' ADD COLUMN 'Temp' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Versionsstand auf 11 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=11";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v11_v12(){
  QSqlQuery query, query2;
  QString sql,sql2;
  QString SudID;

  QSqlDatabase::database().transaction();

  //Eintrag Jungbiermenge beim Abfüllen hinzufügen
  sql = "ALTER TABLE 'Sud' ADD COLUMN 'JungbiermengeAbfuellen' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Bei allen bisherigen Suden in Jungbiermenge die Anstellmenge eintragen
  sql = "UPDATE 'Sud' SET 'JungbiermengeAbfuellen'=WuerzemengeAnstellen";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Alle Hauptgärungseinträge druchgehen und wenn kein zugehöriger Sud eintrag existiert dann den eintrag löschen
  sql = "SELECT * FROM Hauptgaerverlauf";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  while (query.next()){
    SudID = query.value(1).toString();
    //Überprüfen ob Ein zugehörender Sud existiert
    sql2 = "SELECT ID FROM Sud WHERE ID=" + SudID;
    if (!query2.exec(sql2)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql2);
    }
    if (!query2.first()){
      sql2 = "DELETE FROM Hauptgaerverlauf WHERE ID=" + query.value(0).toString();
      if (!query2.exec(sql2)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
          CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
          + QObject::trUtf8("\nSQL-Befehl:\n") + sql2);
      }
    }
  }

  //Alle Schnellgaereinträge druchgehen und wenn kein zugehöriger Sud eintrag existiert dann den eintrag löschen
  sql = "SELECT * FROM Schnellgaerverlauf";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  while (query.next()){
    SudID = query.value(1).toString();
    //Überprüfen ob Ein zugehörender Sud existiert
    sql2 = "SELECT ID FROM Sud WHERE ID=" + SudID;
    if (!query2.exec(sql2)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql2);
    }
    if (!query2.first()){
      sql2 = "DELETE FROM Schnellgaerverlauf WHERE ID=" + query.value(0).toString();
      if (!query2.exec(sql2)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
          CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
          + QObject::trUtf8("\nSQL-Befehl:\n") + sql2);
      }
    }
  }

  //Alle Nachgaereinträge druchgehen und wenn kein zugehöriger Sud eintrag existiert dann den eintrag löschen
  sql = "SELECT * FROM Nachgaerverlauf";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  while (query.next()){
    SudID = query.value(1).toString();
    //Überprüfen ob Ein zugehörender Sud existiert
    sql2 = "SELECT ID FROM Sud WHERE ID=" + SudID;
    if (!query2.exec(sql2)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql2);
    }
    if (!query2.first()){
      sql2 = "DELETE FROM Nachgaerverlauf WHERE ID=" + query.value(0).toString();
      if (!query2.exec(sql2)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
          CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
          + QObject::trUtf8("\nSQL-Befehl:\n") + sql2);
      }
    }
  }

  //Versionsstand auf 12 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=12";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v12_v13(){
  QSqlQuery query;
  QString sql;
  QString SudID;

  QSqlDatabase::database().transaction();

  //Spalte ergebniss Abgefuellte Biermenge einfügen
  sql = "ALTER TABLE 'Sud' ADD COLUMN 'erg_AbgefuellteBiermenge' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Flag setzen das DB neu Berechnet werden muss (Abgefuellte Biermenge muss für alle Sude errechnet werden)
  sql = "UPDATE 'Global' SET 'db_NeuBerechnen'=2";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Versionsstand auf 13 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=13";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v13_v14(){
  QSqlQuery query;
  QString sql;
  QString SudID;

  QSqlDatabase::database().transaction();

  //Spalte MaxSterne einfügen
  sql = "ALTER TABLE 'Sud' ADD COLUMN 'BewertungMaxSterne' NUMERIC DEFAULT 5";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }
  //Tabelle für die Bewertungen
  sql = "CREATE  TABLE  IF NOT EXISTS 'Bewertungen' (";
  sql += "'ID' INTEGER PRIMARY KEY  NOT NULL,";
  sql += "'SudID' INTEGER,";
  sql += "'Woche' INTEGER DEFAULT 0,";
  sql += "'Datum' DATETIME DEFAULT 0,";
  sql += "'Sterne' NUMERIC DEFAULT 0," ;
  sql += "'Bemerkung' TEXT DEFAULT ''," ;
  sql += "'Farbe' INTEGER DEFAULT 0,";
  sql += "'FarbeBemerkung' TEXT DEFAULT ''," ;
  sql += "'Schaum' INTEGER DEFAULT 0,";
  sql += "'SchaumBemerkung' TEXT DEFAULT ''," ;
  sql += "'Geruch' INTEGER DEFAULT 0,";
  sql += "'GeruchBemerkung' TEXT DEFAULT ''," ;
  sql += "'Geschmack' INTEGER DEFAULT 0,";
  sql += "'GeschmackBemerkung' TEXT DEFAULT ''," ;
  sql += "'Antrunk' INTEGER DEFAULT 0,";
  sql += "'AntrunkBemerkung' TEXT DEFAULT ''," ;
  sql += "'Haupttrunk' INTEGER DEFAULT 0,";
  sql += "'HaupttrunkBemerkung' TEXT DEFAULT ''," ;
  sql += "'Nachtrunk' INTEGER DEFAULT 0,";
  sql += "'NachtrunkBemerkung' TEXT DEFAULT ''," ;
  sql += "'Gesamteindruck' INTEGER DEFAULT 0,";
  sql += "'GesamteindruckBemerkung' TEXT DEFAULT '')" ;
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Versionsstand auf 14 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=14";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v14_v15(){
  QSqlQuery query;
  QString sql;
  QString SudID;

  QSqlDatabase::database().transaction();

  //Bit das dieser Sud neu Brechnet werden muss da sich Rohstoffe etc. verändert haben.
  sql = "ALTER TABLE 'Sud' ADD COLUMN 'NeuBerechnen' BOOL DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Menge der zu verwendenden Hopfeneinheiten
  sql = "ALTER TABLE 'Sud' ADD COLUMN 'HefeAnzahlEinheiten' NUMERIC DEFAULT 1";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Spalte Anwendungsfall (für welche Bierstiele geeignet)
  sql = "ALTER TABLE 'Malz' ADD COLUMN 'Anwendung' TEXT ";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Datum wann das Malz eingelagert wurde
  sql = "ALTER TABLE 'Malz' ADD COLUMN 'Eingelagert' DATETIME ";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Mindesthaltbarkeitsdatum
  sql = "ALTER TABLE 'Malz' ADD COLUMN 'Mindesthaltbar' DATETIME ";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Spalte Eigenschaften (z.B. Blumig, Zitrus, Erdig)
  sql = "ALTER TABLE 'Hopfen' ADD COLUMN 'Eigenschaften' TEXT";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Spalte Typ (z.B. Aroma, Bitter)
  sql = "ALTER TABLE 'Hopfen' ADD COLUMN 'Typ' TEXT ";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Datum wann der Hopfen eingelagert wurde
  sql = "ALTER TABLE 'Hopfen' ADD COLUMN 'Eingelagert' DATETIME ";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Mindesthaltbarkeitsdatum
  sql = "ALTER TABLE 'Hopfen' ADD COLUMN 'Mindesthaltbar' DATETIME ";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }



  //Spalte Typ I (Obergärig, Untergärig)
  sql = "ALTER TABLE 'Hefe' ADD COLUMN 'TypOGUG' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Spalte Typ II (flüssig, trocken)
  sql = "ALTER TABLE 'Hefe' ADD COLUMN 'TypTrFl' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Spalte Verpackungsmenge
  sql = "ALTER TABLE 'Hefe' ADD COLUMN 'Verpackungsmenge' TEXT";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Spalte Würzemenge pro Packungseinheit
  sql = "ALTER TABLE 'Hefe' ADD COLUMN 'Wuerzemenge' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Spalte Eigenschaften (z.B. milde Esternote)
  sql = "ALTER TABLE 'Hefe' ADD COLUMN 'Eigenschaften' TEXT ";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Spalte Sedimentation
  sql = "ALTER TABLE 'Hefe' ADD COLUMN 'SED' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Spalte Endvergärungsgrad
  sql = "ALTER TABLE 'Hefe' ADD COLUMN 'EVG' TEXT";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Spalte Temperaturbereich
  sql = "ALTER TABLE 'Hefe' ADD COLUMN 'Temperatur' TEXT";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Datum wann der Hopfen eingelagert wurde
  sql = "ALTER TABLE 'Hefe' ADD COLUMN 'Eingelagert' DATETIME ";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Mindesthaltbarkeitsdatum
  sql = "ALTER TABLE 'Hefe' ADD COLUMN 'Mindesthaltbar' DATETIME ";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Datum wann die Zutat eingelagert wurde
  sql = "ALTER TABLE 'WeitereZutaten' ADD COLUMN 'Eingelagert' DATETIME ";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Mindesthaltbarkeitsdatum
  sql = "ALTER TABLE 'WeitereZutaten' ADD COLUMN 'Mindesthaltbar' DATETIME ";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //In den Hopfengaben alle -1 Werte in 0 tauschen
  sql = "UPDATE 'Hopfengaben' SET 'Pellets'=0 WHERE Pellets==-1";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //In den Hopfengaben alle 2 Werte in 1 tauschen
  sql = "UPDATE 'Hopfengaben' SET 'Pellets'=1 WHERE Pellets==2";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }


  //Versionsstand auf 15 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=15";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  QSqlDatabase::database().commit();
  return true;
}

static bool UpdateDB_v15_v16(){
  QSqlQuery query;
  QString sql;
  QString SudID;

  QSqlDatabase::database().transaction();

  //Spalte Berechnungsart für Hopfen einfügen
  sql = "ALTER TABLE 'Sud' ADD COLUMN 'berechnungsArtHopfen' NUMERIC DEFAULT 0";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Tabelle für die zu Ignorierenden Messages IDs
  sql = "CREATE  TABLE  IF NOT EXISTS 'IgnorMsgID' (";
  sql += "'ID' INTEGER PRIMARY KEY  NOT NULL,";
  sql += "'MsgID' INTEGER DEFAULT 0)";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  //Versionsstand auf 16 setzen
  sql = "UPDATE 'Global' SET 'db_Version'=16";
  if (!query.exec(sql)) {
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
      + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
    return false;
  }

  QSqlDatabase::database().commit();
  return true;
}

// verbindung aufbauen zur SQLite datenbank
static bool ErstelleVerbindung() {
  //Datenbank ist vorhanden
  //bool dbVorhanden = false;
  // Pfad zur Datenbank
  QString dbPfad, dbPfadBackup;
  // Dateiname Datenbankvorlage
  QString dbNameVorlage = DB_VORLAGE;
  // Dateiname der Datenbank Benutzer
  QString dbNameUser = DB_USER_NAME;
  // Verbindung zu SQLite
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

  QApplication::addLibraryPath("./");

  QString dbPfadOhneDateiName;
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("DB");
  dbPfadOhneDateiName = settings.value("DB_Pfad").toString();
  if (settings.value("DB_BckEn").toBool())
    dbPfadBackup = settings.value("DB_BckPfad").toString();

  if (dbPfadOhneDateiName != ""){
    dbPfad = dbPfadOhneDateiName + "/" + dbNameUser;
    if (!QFile::exists(dbPfad)) {
      dbPfadOhneDateiName = QDir::homePath() + "/" + APP_VERZEICHNIS;
      settings.setValue("DB_Pfad", dbPfadOhneDateiName);
      // Fehlermeldung Keine Pfadangabe
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_DB_PFAD_NIO, TYPE_WARNUNG,
        CANCEL_NO, QObject::tr("Betroffener Pfad:\n") + dbPfad);
    }
  }

  if (dbPfadOhneDateiName == ""){
    dbPfadOhneDateiName = QDir::homePath() + "/" + APP_VERZEICHNIS;
    settings.setValue("DB_Pfad", dbPfadOhneDateiName);
  }
  settings.endGroup();

  dbPfad = dbPfadOhneDateiName + "/" + dbNameUser;
  //dbPfad = QDir::currentPath() + "/" + APP_VERZEICHNIS + "/" + dbNameUser;

  // wenn Pfad angegeben wurde
  if (dbPfad != "") {
    //Überprüfen ob Datenbankdatei existiert
    if (QFile::exists(dbPfad)) {
      // User path and file exist
    }
    else {
      // Vorlage wird Kopiert wenn vorhanden
      if (QFile::exists(dbNameVorlage)) {
        //überprüfen ob verzeichniss für Datenbank schon existiert
        //und gegebenenfalls erstellen
        QDir dir;
        if (!dir.exists(dbPfadOhneDateiName)){
          dir.mkdir(dbPfadOhneDateiName);
        }
        //Datenbank wird zu Homeverzeichniss kopiert
        QFile file(dbNameVorlage);
        if (file.copy(dbPfad)) {
          QFile::setPermissions(dbPfad, QFile::ReadOwner | QFile::WriteOwner);
        }
        else {
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_USER_KOPIE, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffener Kopierpfad:\n") + dbPfad);
            return false;
        }
      }
      else {
        // Fehlermeldung keine Datenbankvorlage gefunden
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_VORLAGE, TYPE_KRITISCH,
          CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbNameVorlage);
        return false;
      }
    }
  }
  else {
    // Fehlermeldung Keine Pfadangabe
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_PFAD, TYPE_KRITISCH,
      CANCEL_PROGRAM, QObject::tr("Betroffener Pfad:\n") + dbPfad);
    return false;
  }

  // Setzte Datenbank
  db.setDatabaseName(dbPfad);
  // Datenbank öffnen
  if (!db.open()) {
    // Fehlermeldung
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_OPEN, TYPE_KRITISCH,
      CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
    return false;
  }

  QSqlQuery query;

  //Automatische Komprimierung aktivieren
//	if (!query.exec("VACUUM")){
//		// Fehlermeldung Datenbankabfrage
//		ErrorMessage *errorMessage = new ErrorMessage();
//		errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
//			CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
//			+ QObject::trUtf8("\nSQL-Befehl:\n") + "VACUUM");
//	}

  int updateNr = 0;
  //Überprüfung Versionstand Datenbank
  if (!query.exec("SELECT db_Version FROM Global;")) {
    // Fehlermeldung Kann Versionstand der Datenbank nicht abfragen
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_VERSION, TYPE_KRITISCH,
      CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
    return false;
  }
  else {
    if (!query.first()) {
      // Fehlermeldung Kann Versionstand der Datenbank nicht abfragen
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_VERSION, TYPE_KRITISCH,
        CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
      return false;
    }
    else {
      int updateNr = query.value(0).toInt();

      //Backup der Datenbank anlegen für update
      if (updateNr < DB_VERSION) {
        BackupDatenbank(dbPfad, dbPfad+"~");
      }

      //Wenn Version der Datenbank 1 ist dann auf versionstand 2 Updaten
      if (updateNr == 1){
        if (!UpdateDB_v1_v2()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V1_V2, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v2_v3()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V2_V3, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v3_v4()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V3_V4, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v4_v5()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V4_V5, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v5_v6()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V5_V6, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v6_v7()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V6_V7, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v7_v8()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V7_V8, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v8_v9()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V8_V9, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v9_v10()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V9_V10, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v10_v11()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V10_V11, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v11_v12()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V11_V12, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v12_v13()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V12_V13, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 2 ist dann auf versionstand 3 Updaten
      else if (updateNr == 2){
        if (!UpdateDB_v2_v3()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V2_V3, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v3_v4()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V3_V4, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v4_v5()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V4_V5, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v5_v6()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V5_V6, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v6_v7()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V6_V7, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v7_v8()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V7_V8, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v8_v9()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V8_V9, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v9_v10()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V9_V10, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v10_v11()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V10_V11, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v11_v12()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V11_V12, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v12_v13()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V12_V13, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 3 ist dann auf versionstand 4 Updaten
      else if (updateNr == 3){
        if (!UpdateDB_v3_v4()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V3_V4, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v4_v5()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V4_V5, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v5_v6()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V5_V6, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v6_v7()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V6_V7, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v7_v8()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V7_V8, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v8_v9()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V8_V9, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v9_v10()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V9_V10, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v10_v11()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V10_V11, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v11_v12()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V11_V12, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v12_v13()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V12_V13, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 4 ist dann auf versionstand 5 Updaten
      else if (updateNr == 4){
        if (!UpdateDB_v4_v5()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V4_V5, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v5_v6()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V5_V6, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v6_v7()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V6_V7, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v7_v8()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V7_V8, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v8_v9()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V8_V9, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v9_v10()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V9_V10, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v10_v11()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V10_V11, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v11_v12()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V11_V12, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v12_v13()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V12_V13, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 5 ist dann auf versionstand 6 Updaten
      else if (updateNr == 5){
        if (!UpdateDB_v5_v6()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V5_V6, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v6_v7()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V6_V7, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v7_v8()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V7_V8, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v8_v9()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V8_V9, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v9_v10()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V9_V10, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v10_v11()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V10_V11, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v11_v12()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V11_V12, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v12_v13()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V12_V13, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 6 ist dann auf versionstand 6 Updaten
      else if (updateNr == 6){
        if (!UpdateDB_v6_v7()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V6_V7, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v7_v8()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V7_V8, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v8_v9()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V8_V9, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v9_v10()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V9_V10, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v10_v11()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V10_V11, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v11_v12()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V11_V12, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v12_v13()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V12_V13, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 7 ist dann auf versionstand 8 Updaten
      else if (updateNr == 7){
        if (!UpdateDB_v7_v8()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V7_V8, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v8_v9()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V8_V9, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v9_v10()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V9_V10, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v10_v11()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V10_V11, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v11_v12()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V11_V12, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v12_v13()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V12_V13, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 8 ist dann auf versionstand 9 Updaten
      else if (updateNr == 8){
        if (!UpdateDB_v8_v9()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V8_V9, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v9_v10()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V9_V10, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v10_v11()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V10_V11, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v11_v12()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V11_V12, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v12_v13()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V12_V13, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 9 ist dann auf versionstand 10 Updaten
      else if (updateNr == 9){
        if (!UpdateDB_v9_v10()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V9_V10, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v10_v11()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V10_V11, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v11_v12()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V11_V12, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v12_v13()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V12_V13, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 10 ist dann auf versionstand 11 Updaten
      else if (updateNr == 10){
        if (!UpdateDB_v10_v11()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V10_V11, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v11_v12()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V11_V12, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v12_v13()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V12_V13, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 11 ist dann auf versionstand 12 Updaten
      else if (updateNr == 11){
        if (!UpdateDB_v11_v12()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V11_V12, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v12_v13()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V12_V13, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 12 ist dann auf versionstand 13 Updaten
      else if (updateNr == 12){
        if (!UpdateDB_v12_v13()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V12_V13, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 13 ist dann auf versionstand 14 Updaten
      else if (updateNr == 13){
        if (!UpdateDB_v13_v14()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V13_V14, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 14 ist dann auf versionstand 15 Updaten
      else if (updateNr == 14){
        if (!UpdateDB_v14_v15()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V14_V15, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //Wenn Version der Datenbank 15 ist dann auf versionstand 16 Updaten
      else if (updateNr == 15){
        if (!UpdateDB_v15_v16()){
          // Fehlermeldung Konnte Datenbank nicht updaten
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V15_V16, TYPE_KRITISCH,
            CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
          return false;
        }
        updateNr = 16;
      }
      //wenn Version der Datenbank > der aktuellen ist dann ist das Programm hier veraltet
      else if (updateNr > DB_VERSION) {
        //Fehlermeldung Die Software ist veraltet.
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_PROG_VERALTET, TYPE_KRITISCH,
          CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
        return false;
      }
    }
  }

  //----------------------------------------Update Begin------------------------------------
  //Hier werden die Uptdates ab version 16 aufgerufen da die sql Tabelle an dieser stelle
  //nicht mehr gelockt ist
  if (updateNr == 16) {
    bool io = true;
    int brauanlagenID = 0;
    QSqlDatabase::database().transaction();
    //Spalte High-Gravity-Faktor einfügen
    QString sql = "ALTER TABLE 'Sud' ADD COLUMN 'highGravityFaktor' NUMERIC DEFAULT 0";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Spalte AuswahlBrauanlage einfügen
    sql = "ALTER TABLE 'Sud' ADD COLUMN 'AuswahlBrauanlage' INTEGER DEFAULT 0";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Spalte AuswahlBrauanlage Name einfügen
    sql = "ALTER TABLE 'Sud' ADD COLUMN 'AuswahlBrauanlageName' Text DEFAULT ''";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Spalte Sud bei der durchschnittlichen Ausbeute berechnung berücksichtigen
    sql = "ALTER TABLE 'Sud' ADD COLUMN 'AusbeuteIgnorieren' BOOL DEFAULT 0";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Spalte MerklistenID einfügen
    sql = "ALTER TABLE 'Sud' ADD COLUMN 'MerklistenID' INTEGER DEFAULT 0";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Neue Tabelle Ausrüstung erstellen
    // Muss leider so gemacht werden da ich nachträglich keinen primary key hinzufügen kann
    sql = "CREATE TABLE AusruestungV2 ('ID' INTEGER PRIMARY KEY  NOT NULL, 'AnlagenID' INTEGER, KorrekturWasser NUMERIC, KorrekturFarbe NUMERIC DEFAULT 0, Sudhausausbeute NUMERIC, 'Maischebottich_Hoehe' NUMERIC DEFAULT 0, 'Maischebottich_Durchmesser' NUMERIC DEFAULT 0, 'Maischebottich_MaxFuellhoehe' NUMERIC DEFAULT 0, 'Sudpfanne_Hoehe' NUMERIC DEFAULT 0, 'Sudpfanne_Durchmesser' NUMERIC DEFAULT 0, 'Sudpfanne_MaxFuellhoehe' NUMERIC DEFAULT 0, 'Name' TEXT DEFAULT 'Meine Brauanlage', 'Verdampfungsziffer' NUMERIC DEFAULT 20)";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Daten von der Alten in die neue kopieren
    sql = "SELECT * FROM 'Ausruestung'";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    else {
      while (query.next()) {
        int Sudhausausbeute;
        double KorrekturWasser;
        double Maischebottich_Hoehe;
        double Maischebottich_Durchmesser;
        double Maischebottich_MaxFuellhoehe;
        double Sudpfanne_Hoehe;
        double Sudpfanne_Durchmesser;
        double Sudpfanne_MaxFuellhoehe;
        int FeldNr;
        //Ausbeute zur Berechnung der Schüttung
        FeldNr = query.record().indexOf("Sudhausausbeute");
        Sudhausausbeute=query.value(FeldNr).toInt();
        //Korrektur der Nachgussmenge
        FeldNr = query.record().indexOf("KorrekturWasser");
        KorrekturWasser=query.value(FeldNr).toDouble();
        //Maischebottich Höhe
        FeldNr = query.record().indexOf("Maischebottich_Hoehe");
        Maischebottich_Hoehe=query.value(FeldNr).toDouble();
        //Maischebottich Durchmesser
        FeldNr = query.record().indexOf("Maischebottich_Durchmesser");
        Maischebottich_Durchmesser=query.value(FeldNr).toDouble();
        //Maischebottich Maximal nutzbare Füllhöhe
        FeldNr = query.record().indexOf("Maischebottich_MaxFuellhoehe");
        Maischebottich_MaxFuellhoehe=query.value(FeldNr).toDouble();
        //Sudpfanne Höhe
        FeldNr = query.record().indexOf("Sudpfanne_Hoehe");
        Sudpfanne_Hoehe=query.value(FeldNr).toDouble();
        //Sudpfanne Durchmesser
        FeldNr = query.record().indexOf("Sudpfanne_Durchmesser");
        Sudpfanne_Durchmesser=query.value(FeldNr).toDouble();
        //Sudpfanne Maximal nutzbare Füllhöhe
        FeldNr = query.record().indexOf("Sudpfanne_MaxFuellhoehe");
        Sudpfanne_MaxFuellhoehe=query.value(FeldNr).toDouble();

        sql = "INSERT INTO 'AusruestungV2' ('AnlagenID', 'Maischebottich_Hoehe','Maischebottich_Durchmesser','Maischebottich_MaxFuellhoehe','Sudpfanne_Hoehe','Sudpfanne_Durchmesser','Sudpfanne_MaxFuellhoehe','KorrekturWasser','Sudhausausbeute') ";
        sql += "VALUES (";
        brauanlagenID = (int)time(NULL)+rand();
        sql += QString::number(brauanlagenID)+",";
        sql += QString::number(Maischebottich_Hoehe)+",";
        sql += QString::number(Maischebottich_Durchmesser)+",";
        sql += QString::number(Maischebottich_MaxFuellhoehe)+",";
        sql += QString::number(Sudpfanne_Hoehe)+",";
        sql += QString::number(Sudpfanne_Durchmesser)+",";
        sql += QString::number(Sudpfanne_MaxFuellhoehe)+",";
        sql += QString::number(KorrekturWasser)+",";
        sql += QString::number(Sudhausausbeute)+")";
        if (!query.exec(sql)) {
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
            CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
            + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
          io = false;
        }

      }
    }

    //Alte Tabelle löschen
    sql = "DROP TABLE 'Ausruestung'";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Neue wieder zur alten umbenennen
    sql = "ALTER TABLE 'AusruestungV2' RENAME TO 'Ausruestung'";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Brauanlagen ID bei allen Suden eintragen
    sql = "UPDATE Sud SET 'AuswahlBrauanlage'="+QString::number(brauanlagenID)+" WHERE ID > 0";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Versionsstand auf 17 setzen
    sql = "UPDATE 'Global' SET 'db_Version'=17";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    if (!io){
      // Fehlermeldung Konnte Datenbank nicht updaten
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V16_V17, TYPE_KRITISCH,
        CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
      return false;
    }
    QSqlDatabase::database().commit();
    updateNr = 17;
  }
  //Update von 17 auf 18
  if (updateNr == 17) {
    bool io = true;
    QSqlDatabase::database().transaction();
    //Spalte High-Gravity-Faktor einfügen
    QString sql = "ALTER TABLE 'Ausruestung' ADD COLUMN 'Kosten' NUMERIC DEFAULT 0";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Versionsstand auf 18 setzen
    sql = "UPDATE 'Global' SET 'db_Version'=18";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    if (!io){
      // Fehlermeldung Konnte Datenbank nicht updaten
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V17_V18, TYPE_KRITISCH,
        CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
      return false;
    }
    QSqlDatabase::database().commit();
    updateNr = 18;
  }
  //Update von 18 auf 19
  if (updateNr == 18) {
    bool io = true;
    QSqlDatabase::database().transaction();
    //Spalte Spunden einfügen
    QString sql = "ALTER TABLE 'Sud' ADD COLUMN 'Spunden' BOOL DEFAULT 0";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Spalte Brauanlagen ID in Geräteliste einfügen und mit ID der ersten Brauanlage füllen
    sql = "ALTER TABLE 'Geraete' ADD COLUMN 'AusruestungAnlagenID' NUMERIC DEFAULT 0";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Geräte mit einer ID von den Ausrüstungen füllen
    sql = "SELECT * FROM 'Ausruestung'";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    if (query.first()) {
      QString id = query.value("AnlagenID").toString();
      sql = "UPDATE 'Geraete' SET 'AusruestungAnlagenID'=" + id;
      if (!query.exec(sql)) {
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
          CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
          + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
        io = false;
      }
    }
    //Versionsstand auf 19 setzen
    sql = "UPDATE 'Global' SET 'db_Version'=19";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    if (!io){
      // Fehlermeldung Konnte Datenbank nicht updaten
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V18_V19, TYPE_KRITISCH,
        CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
      return false;
    }
    QSqlDatabase::database().commit();
    updateNr = 19;
  }
  //Update von 19 auf 20
  if (updateNr == 19) {
    bool io = true;
    QSqlDatabase::database().transaction();
    //Spalte Link in Tabelle Malz
    QString sql = "ALTER TABLE 'Malz' ADD COLUMN 'Link' TEXT ";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Spalte Link in Tabelle Hopfen
    sql = "ALTER TABLE 'Hopfen' ADD COLUMN 'Link' TEXT ";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Spalte Link in Tabelle Hefe
    sql = "ALTER TABLE 'Hefe' ADD COLUMN 'Link' TEXT ";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Spalte Link in Tabelle WeitereZutaten
    sql = "ALTER TABLE 'WeitereZutaten' ADD COLUMN 'Link' TEXT ";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Tabelle für Anhänge hinzufügen
    sql = "CREATE TABLE IF NOT EXISTS 'Anhang' ('ID' INTEGER PRIMARY KEY  NOT NULL ,'SudID' INTEGER,'Pfad' TEXT)";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      return false;
    }
    //Versionsstand auf 20 setzen
    sql = "UPDATE 'Global' SET 'db_Version'=20";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    if (!io){
      // Fehlermeldung Konnte Datenbank nicht updaten
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V19_V20, TYPE_KRITISCH,
        CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
      return false;
    }
    QSqlDatabase::database().commit();
    updateNr = 20;
  }
  //Update von 20 auf 21
  if (updateNr == 20) {
    bool io = true;
    QSqlDatabase::database().transaction();
    //Spalte Datum Zugabezeitpunkt Weitere Zutaten von
    QString sql = "ALTER TABLE 'WeitereZutatenGaben' ADD COLUMN 'Zeitpunkt_von' DATETIME";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Spalte Datum Zugabezeitpunkt Weitere Zutaten bis
    sql = "ALTER TABLE 'WeitereZutatenGaben' ADD COLUMN 'Zeitpunkt_bis' DATETIME";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Spalte Zugabestatus
    sql = "ALTER TABLE 'WeitereZutatenGaben' ADD COLUMN 'Zugabestatus' INTEGER DEFAULT 0";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Spalte Keine Entnahme
    sql = "ALTER TABLE 'WeitereZutatenGaben' ADD COLUMN 'Entnahmeindex' INTEGER DEFAULT 0";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Versionsstand auf 21 setzen
    sql = "UPDATE 'Global' SET 'db_Version'=21";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    if (!io){
      // Fehlermeldung Konnte Datenbank nicht updaten
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V20_V21, TYPE_KRITISCH,
        CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
      return false;
    }
    QSqlDatabase::database().commit();
    updateNr = 21;
  }
  //Update von 21 auf 22
  if (updateNr == 21) {
    bool io = true;
    QSqlDatabase::database().transaction();
    //Spalte Zugabestatus
    QString sql = "ALTER TABLE 'WeitereZutatenGaben' ADD COLUMN 'Zugabedauer' NUMERIC DEFAULT 0";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    //Versionsstand auf 22 setzen
    sql = "UPDATE 'Global' SET 'db_Version'=22";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
        CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
        + QObject::trUtf8("\nSQL-Befehl:\n") + sql);
      io = false;
    }
    if (!io){
      // Fehlermeldung Konnte Datenbank nicht updaten
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_UPDATE_V21_V22, TYPE_KRITISCH,
        CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + dbPfad);
      return false;
    }
    QSqlDatabase::database().commit();
    //updateNr = 21;
  }

  //Sicherungsdatei löschen
  RemoveDatenbanksicherung(dbPfad + "~");

  //Backup der Datenbank anlegen
  if (!dbPfadBackup.isEmpty())
    BackupDatenbank(dbPfad, dbPfadBackup);

  //Überprüfungen Druchführen
  CheckDB();

  return true;
}

#endif // CONNECTION_H
