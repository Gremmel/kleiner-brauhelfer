
#include <QMessageBox>

#include "errormessage.h"
#include "definitionen.h"

ErrorMessage::ErrorMessage(QWidget *parent) : QWidget(parent) {
}

void ErrorMessage::showMessage(int errorNo, int errorType, int cancelProcedure,
	QString addon) {
	QString message;
	// Fehlertext
	message = getErrorText(errorNo);
	// wenn Text noch angehängt werden soll
	if (addon != "") {
		message.append("\n" + addon);
	}

	// Abbruchtext
	message.append("\n\n" + getCancelText(cancelProcedure));

	// Style
	switch (errorType) {
		case TYPE_INFO:
			QMessageBox::information(0, APP_NAME, message);
			break;
		case TYPE_WARNUNG:
			QMessageBox::warning(0, APP_NAME, message);
			break;
		case TYPE_KRITISCH:
		default:
			QMessageBox::critical(0, APP_NAME, message);
			break;
	}
}

QString ErrorMessage::getCancelText(int number) {
	QString cancelText = "";
	switch (number) {
		case CANCEL_NO:
			cancelText = "";
			break;
		case CANCEL_AKTION:
			cancelText = trUtf8("Der Vorgang wird abgebrochen.");
			break;
		case CANCEL_PROGRAM:
		default:
			cancelText = trUtf8("Die Anwendung wird beendet.");
			break;
	}
	return cancelText;
}

QString ErrorMessage::getErrorText(int number) {
	QString errorText = "";
	switch (number) {
		case ERR_SQL_DB_USER_KOPIE:
			errorText = trUtf8("Vorlagendatenbank konnte nicht Kopiert werden");
			break;
		case ERR_SQL_DB_VORLAGE:
			errorText = trUtf8("Vorlagendatenbank ist nicht vorhanden");
			break;
		case ERR_SQL_DB_PFAD:
			errorText = trUtf8("Es Konnte kein Pfad für die Datenbankdatei ermittelt werden");
			break;
		case ERR_SQL_DB_OPEN:
			errorText = trUtf8("Kann Datenbank nicht öffnen");
			break;
		case ERR_DB_KOPIE_NEU_ORT:
			errorText = trUtf8("Kann Datenbank nicht an neuen Ort Kopieren, wahrscheinlich keine ausreichende Berechtigung oder es ist schon eine Datenbankdatei vorhanden.\nDer Pfad wird nicht geändert");
			break;
		case ERR_DB_DEL_ALT:
			errorText = trUtf8("Kann Datenbank am alten Platz nicht löschen, Die Datei sollte von Hand entfernt werden.");
			break;
		case ERR_SQL_DB_VERSION:
			errorText = trUtf8("Kann Version der Datenbank nicht abfragen");
			break;
		case ERR_SQL_DB_PROG_VERALTET:
			errorText = trUtf8("Die Datenbank-version ist neuer als das Programm lesen kann. Das Programm muss upgedatet werden.");
			break;
		case ERR_SQL_DB_ABFRAGE:
			errorText = trUtf8("Fehler bei SQL Abfrage");
			break;
		case ERR_VERSION_SUD_V1:
			errorText = trUtf8("Sud Dateien der Version 1 können leider nicht Importiert werden. Bitte Speichern Sie die Suddaten vorher noch mit der Vorgängerversion (0.9.4.2) vom kleinen-brauhelfer ab.");
			break;
		case ERR_SQL_DB_UPDATE_V1_V2:
			errorText = trUtf8("Konnte Datenbank nicht von Version 1 auf Version 2 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V2_V3:
			errorText = trUtf8("Konnte Datenbank nicht von Version 2 auf Version 3 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V3_V4:
			errorText = trUtf8("Konnte Datenbank nicht von Version 3 auf Version 4 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V4_V5:
			errorText = trUtf8("Konnte Datenbank nicht von Version 4 auf Version 5 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V5_V6:
			errorText = trUtf8("Konnte Datenbank nicht von Version 5 auf Version 6 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V6_V7:
			errorText = trUtf8("Konnte Datenbank nicht von Version 6 auf Version 7 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V7_V8:
			errorText = trUtf8("Konnte Datenbank nicht von Version 7 auf Version 8 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V8_V9:
			errorText = trUtf8("Konnte Datenbank nicht von Version 8 auf Version 9 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V9_V10:
			errorText = trUtf8("Konnte Datenbank nicht von Version 9 auf Version 10 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V10_V11:
			errorText = trUtf8("Konnte Datenbank nicht von Version 10 auf Version 11 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V11_V12:
			errorText = trUtf8("Konnte Datenbank nicht von Version 11 auf Version 12 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V12_V13:
			errorText = trUtf8("Konnte Datenbank nicht von Version 12 auf Version 13 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V13_V14:
			errorText = trUtf8("Konnte Datenbank nicht von Version 13 auf Version 14 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V14_V15:
			errorText = trUtf8("Konnte Datenbank nicht von Version 14 auf Version 15 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V15_V16:
			errorText = trUtf8("Konnte Datenbank nicht von Version 15 auf Version 16 updaten");
			break;
		case ERR_SQL_DB_UPDATE_V16_V17:
			errorText = trUtf8("Konnte Datenbank nicht von Version 16 auf Version 17 updaten");
			break;
    case ERR_SQL_DB_UPDATE_V17_V18:
      errorText = trUtf8("Konnte Datenbank nicht von Version 17 auf Version 18 updaten");
      break;
    case ERR_VERSION_UNBEKANNT:
			errorText = trUtf8("Die Datei ist keine gültige Sud Datei");
			break;
		case ERR_XML_OPEN:
			errorText = trUtf8("Die Datei konnte nicht geöffnet werden");
			break;
		case ERR_XML_PARSEN:
			errorText = trUtf8("Fehler beim analysieren der Datei");
			break;
		case ERR_DB_KOPIE_BACKUP:
			errorText = trUtf8("Es konnte keine Backupkopie der Datenbank erstellt werden");
			break;
		case ERR_DB_KOPIE_RESTORE:
			errorText = trUtf8("Die Datenbank konnte nicht zurückkopiert werden");
			break;
		case ERR_DB_BACKUP_VORHANDEN:
			errorText = trUtf8("Die Backupdatei existiert nicht");
			break;
		case ERR_DB_ROHSTOFFLISTEN_KOPIE:
			errorText = trUtf8("Die Vorlagendatei für die Rohstoffe konnte nicht kopiert werden.");
			break;
		case ERR_DB_PFAD_NIO:
			errorText = trUtf8("Kann Datenbankpfad nicht mehr finden! Der Pfad wird auf den Defaultpfad umgestellt und die Vorlagendatenbank angelegt.\nBitte den Pfad zur Datenbankdatei in dein Einstellungen korrigieren.");
			break;


		default:
			errorText = trUtf8("Ein Fehler ist aufgetreten.");
			break;
	}
	// Fehlernummer Anhängen
	errorText.append(trUtf8("\n(Fehlernummer: ") + QString::number(number) + trUtf8(")\n"));
	return errorText;
}
