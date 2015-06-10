
#ifndef DEFINITIONEN_H
#define DEFINITIONEN_H

#define DEBUG true

//Version
#define VERSION "1.4.1.1"
#define VERSION_INT 1040101
//Datenbankversion
//V17 highGravityFaktor Prozentwert
//    in Tabelle Ausruestunpog Verdampfungsziffer hinzugefügt (Korrektur Nachgussmenge wird nun nicht mehr gebraucht)
//V18 Tabelle Ausruestung um Kosten erweitert
#define DB_VERSION 18
//Version der XML Datei
//Version 2 mit Weiteren Zutaten
//Version 3 mit Temperaturen im Schnellgärverlauf/Hauptgärverlauf
//Version 4 mit Jungbiermenge beim Abfällen
//Version 5 mit Abgefuellter Biermenge
//Version 6 mit Bewertungen
//Version 7 mit Erweiterten Rohstoffeinträgen
//Version 8 mit Berechnungsart Hopfen
//Version 9 mit High Gravity Faktor
#define XSUD_VERSION "9"

//Portable USB-Stick Version
//#define PORTABLE true
#define PORTABLE false
#define KONFIG_NAME_PORTABLE "./konfig"

//Ablagepfad Konfigdatei
#define KONFIG_ORDNER "Gremmelsoft"

//Programmname
#define APP_NAME	"kleiner-brauhelfer"
#define APP_VERZEICHNIS ".kleiner-brauhelfer"

#ifdef DEBUG
  #define APP_KONFIG "kleiner-brauhelfer-test"
#else
  #define APP_KONFIG "kleiner-brauhelfer"
#endif

//URL für die Meldungen beim Start
#ifdef DEBUG
  #define MSG_URL "http://www.joerum.de/kleiner-brauhelfer/kb_meldungen_v2_test"
#else
  #define MSG_URL "http://www.joerum.de/kleiner-brauhelfer/kb_meldungen_v2"
#endif

//URL Anleitung
#define URL_ANLEITUNG "http://www.joerum.de/kleiner-brauhelfer/doku.php"

//Datenbankvorlage
#define DB_VORLAGE	":/db/vorlage.sqlite"

//Datenbank mir Rohstoffvorlagen
#define DB_ROHSTOFFVORLAGE  ":/db/rohstofflisten.sqlite"
#define DB_ROHSTOFFLISTEN  "rohstofflisten.sqlite"
//Userdatenbankname
#ifdef DEBUG
  #define DB_USER_NAME	"kb_daten_test.sqlite"
#else
  #define DB_USER_NAME	"kb_daten.sqlite"
#endif

//Maximale Anzahl Rasten
#define MAX_RASTEN 6
//Maximale Anzahl Schüttungen
#define MAX_MALZGABEN 6
//Maximale Anzahl Hofpengaben
#define MAX_HOPFENGABEN 7

//Farben

//Farbe für Highlighted Text Style Auswahl
#define FARBE_STYLE_HIGHLIGHT 0,85,255

//Farbe für Spalte in Brauübersicht Tabelle Skalierung X Achse für Diagramm
#define FARBE_BRAUUEBERSICHT_SPALTE_ACHSE_X 164,164,164
#define FARBE_BRAUUEBERSICHT_SPALTE_ACHSE_X_DARK 100,100,100
//Farbe für Spalte in Brauübersicht Tabelle Daten für Linie 1
#define FARBE_BRAUUEBERSICHT_SPALTE_L1 151,125,166
//Farbe für Spalte in Brauübersicht Tabelle Daten für Linie 2
#define FARBE_BRAUUEBERSICHT_SPALTE_L2 141,154,127

//Farbe für Linie 1 in Diagramm Brauübersicht
#define FARBE_BRAUUEBERSICHT_DIAGRAMM_L1 116,30,166
//Farbe für Linie 2 in Diagramm Brauübersicht
#define FARBE_BRAUUEBERSICHT_DIAGRAMM_L2 56,104,2

//Farbe für Linie 1 in Diagramm Schnellgärprobe
#define FARBE_GAERVERLAUF_DIAGRAMM_S_L1 116,30,166
//Farbe für Linie 2 in Diagrmmm Schnellgärprobe
#define FARBE_GAERVERLAUF_DIAGRAMM_S_L2 56,104,2
//Farbe für Linie 3 in Diagrmmm Schnellgärprobe
#define FARBE_GAERVERLAUF_DIAGRAMM_S_L3 170,0,0

//Farbe für Linie 1 in Diagramm Hauptgärung
#define FARBE_GAERVERLAUF_DIAGRAMM_H_L1 116,30,166
//Farbe für Linie 2 in Diagrmmm Hauptgärung
#define FARBE_GAERVERLAUF_DIAGRAMM_H_L2 56,104,2
//Farbe für Linie 3 in Diagrmmm Hauptgärung
#define FARBE_GAERVERLAUF_DIAGRAMM_H_L3 170,0,0

//Farbe für Linie 1 in Diagramm Nachgärung
#define FARBE_GAERVERLAUF_DIAGRAMM_N_L1 116,30,166
//Farbe für Linie 2 in Diagrmmm Nachgärung
#define FARBE_GAERVERLAUF_DIAGRAMM_N_L2 170,0,0

//Farbe Bier ist in Reifung
#define FARBE_BierReift 214,194,60
#define FARBE_BierReift_DARK 147,130,12
//Farbe Bier ist fertig
#define FARBE_BierFertig 149,194,12
#define FARBE_BierFertig_DARK 149,194,12
//Farbe Bier ist Leer
#define FARBE_BierLeer 255,255,255
#define FARBE_BierLeer_DARK 34,34,34

//Weitere Zutaten typen
#define EWZ_Typ_Honig 0
#define EWZ_Typ_Zucker 1
#define EWZ_Typ_Gewuerz 2
#define EWZ_Typ_Frucht 3
#define EWZ_Typ_Sonstiges 4
#define EWZ_Typ_Hopfen 100

//Weitere Zutaten Zugabezeitpunkt
#define EWZ_Zeitpunkt_Gaerung 0
#define EWZ_Zeitpunkt_Kochbeginn 1
#define EWZ_Zeitpunkt_Maischen 2

//Weitere Zutaten Einheit
#define EWZ_Einheit_Kg 0
#define EWZ_Einheit_g 1

//Rohstoffart Rohstoffvorlagen
#define R_Malz 1
#define R_Hopfen 2
#define R_Hefe 3

//Animationsstop für die Rasten (Anzahl Rasten)
#define RAST_ANIMATION_STOP 6

//Anzahl Sterne im Bewertungsystem
#define BEW_ANZAHL_STERNE 5

//Fehlermeldungen

#define TYPE_INFO 1
#define TYPE_WARNUNG 2
#define TYPE_KRITISCH 3

#define CANCEL_NO 10
#define CANCEL_AKTION 11
#define CANCEL_PROGRAM 12

#define ERR_SQL_DB_USER_KOPIE 101
#define ERR_SQL_DB_VORLAGE 102
#define ERR_SQL_DB_PFAD 103
#define ERR_SQL_DB_OPEN 104
#define ERR_SQL_DB_VERSION 105
#define ERR_SQL_DB_PROG_VERALTET 106
#define ERR_SQL_DB_ABFRAGE 107
#define ERR_SQL_DB_UPDATE_V1_V2 108
#define ERR_DB_KOPIE_NEU_ORT 109
#define ERR_DB_DEL_ALT 110
#define ERR_SQL_DB_UPDATE_V2_V3 111
#define ERR_SQL_DB_UPDATE_V3_V4 112
#define ERR_SQL_DB_UPDATE_V4_V5 113
#define ERR_SQL_DB_UPDATE_V5_V6 114
#define ERR_SQL_DB_UPDATE_V6_V7 115
#define ERR_SQL_DB_UPDATE_V7_V8 116
#define ERR_SQL_DB_UPDATE_V8_V9 117
#define ERR_SQL_DB_UPDATE_V9_V10 118
#define ERR_DB_KOPIE_BACKUP 119
#define ERR_DB_KOPIE_RESTORE 120
#define ERR_DB_BACKUP_VORHANDEN 121
#define ERR_SQL_DB_UPDATE_V10_V11 122
#define ERR_SQL_DB_UPDATE_V11_V12 123
#define ERR_SQL_DB_UPDATE_V12_V13 124
#define ERR_SQL_DB_UPDATE_V13_V14 125
#define ERR_SQL_DB_UPDATE_V14_V15 126
#define ERR_SQL_DB_UPDATE_V15_V16 127
#define ERR_SQL_DB_UPDATE_V16_V17 128
#define ERR_DB_PFAD_NIO 129
#define ERR_SQL_DB_UPDATE_V17_V18 130



#define ERR_VERSION_SUD_V1 200
#define ERR_VERSION_UNBEKANNT 201
#define ERR_XML_OPEN 202
#define ERR_XML_PARSEN 203

#define ERR_DB_ROHSTOFFLISTEN_KOPIE 300

#define TAB_REZEPT 1
#define TAB_GAERVERLAUF 4
#define TAB_ZUSAMMENFASSUNG 5


#endif
