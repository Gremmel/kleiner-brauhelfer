#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>

#include "definitionen.h"
#include "errormessage.h"

void Database::error(const QSqlQuery& query)
{
    ErrorMessage *dlg = new ErrorMessage();
    dlg->showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG, CANCEL_NO,
                     trUtf8("Query") + ":\n" + query.lastQuery() + "\n" +
                     trUtf8("Fehler") + ":\n" + query.lastError().databaseText());
    delete dlg;
}

bool Database::transaction()
{
    return QSqlDatabase::database().transaction();
}

bool Database::commit()
{
    return QSqlDatabase::database().commit();
}

bool Database::rollback()
{
    return QSqlDatabase::database().rollback();
}

int Database::SudKopieren(const QString& id, const QString& name, bool full)
{
    QString sql;
    QSqlQuery query;
    QString idNew;

    if (!transaction())
        return 0;

    // Sud
    sql = "INSERT INTO Sud (";
    sql += "Sudname,";
    sql += "Menge,";
    sql += "SW,";
    sql += "CO2,";
    sql += "IBU,";
    sql += "Kommentar,";
    sql += "Braudatum,";
    sql += "BierWurdeGebraut,";
    sql += "Anstelldatum,";
    sql += "WuerzemengeAnstellen,";
    sql += "SWAnstellen,";
    sql += "Abfuelldatum,";
    sql += "BierWurdeAbgefuellt,";
    sql += "SWSchnellgaerprobe,";
    sql += "SWJungbier,";
    sql += "TemperaturJungbier,";
    sql += "WuerzemengeKochende,";
    sql += "Speisemenge,";
    sql += "SWKochende,";
    sql += "AuswahlHefe,";
    sql += "FaktorHauptguss,";
    sql += "KochdauerNachBitterhopfung,";
    sql += "EinmaischenTemp,";
    sql += "Erstellt,";
    sql += "Gespeichert,";
    sql += "AktivTab,";
    sql += "erg_S_Gesammt,";
    sql += "erg_W_Gesammt,";
    sql += "erg_WHauptguss,";
    sql += "erg_WNachguss,";
    sql += "erg_Sudhausausbeute,";
    sql += "erg_Farbe,";
    sql += "erg_Preis,";
    sql += "erg_Alkohol,";
    sql += "KostenWasserStrom,";
    if (full) {
      sql += "Bewertung,";
      sql += "BewertungText,";
      sql += "MerklistenID,";
    }
    sql += "AktivTab_Gaerverlauf,";
    sql += "Reifezeit,";
    sql += "BierWurdeVerbraucht,";
    sql += "Nachisomerisierungszeit,";
    sql += "WuerzemengeVorHopfenseihen,";
    sql += "SWVorHopfenseihen,";
    sql += "erg_EffektiveAusbeute,";
    sql += "RestalkalitaetSoll,";
    sql += "SchnellgaerprobeAktiv,";
    sql += "JungbiermengeAbfuellen,";
    sql += "erg_AbgefuellteBiermenge,";
    sql += "BewertungMaxSterne,";
    sql += "NeuBerechnen,";
    sql += "HefeAnzahlEinheiten,";
    sql += "berechnungsArtHopfen,";
    sql += "highGravityFaktor,";
    sql += "AuswahlBrauanlage,";
    sql += "AuswahlBrauanlageName,";
    sql += "AusbeuteIgnorieren,";
    sql += "Spunden";
    sql += ")SELECT ";
    sql += ":sudname,";
    sql += "Menge,";
    sql += "SW,";
    sql += "CO2,";
    sql += "IBU,";
    sql += "Kommentar,";
    sql += "Braudatum,";
    sql += full ? "BierWurdeGebraut," : "0,";
    sql += "Anstelldatum,";
    sql += "WuerzemengeAnstellen,";
    sql += "SWAnstellen,";
    sql += "Abfuelldatum,";
    sql += full ? "BierWurdeAbgefuellt," : "0,";
    sql += "SWSchnellgaerprobe,";
    sql += "SWJungbier,";
    sql += "TemperaturJungbier,";
    sql += "WuerzemengeKochende,";
    sql += "Speisemenge,";
    sql += "SWKochende,";
    sql += "AuswahlHefe,";
    sql += "FaktorHauptguss,";
    sql += "KochdauerNachBitterhopfung,";
    sql += "EinmaischenTemp,";
    sql += full ? "Erstellt," : ("'" + QDateTime::currentDateTime().toString(Qt::ISODate) + "',");
    sql += "'" + QDateTime::currentDateTime().toString(Qt::ISODate) + "',"; // Gespeichert
    sql += "AktivTab,";
    sql += "erg_S_Gesammt,";
    sql += "erg_W_Gesammt,";
    sql += "erg_WHauptguss,";
    sql += "erg_WNachguss,";
    sql += "erg_Sudhausausbeute,";
    sql += "erg_Farbe,";
    sql += "erg_Preis,";
    sql += "erg_Alkohol,";
    sql += "KostenWasserStrom,";
    if (full) {
      sql += "Bewertung,";
      sql += "BewertungText,";
      sql += "MerklistenID,";
    }
    sql += "AktivTab_Gaerverlauf,";
    sql += "Reifezeit,";
    sql += full ? "BierWurdeVerbraucht," : "0,";
    sql += "Nachisomerisierungszeit,";
    sql += "WuerzemengeVorHopfenseihen,";
    sql += "SWVorHopfenseihen,";
    sql += "erg_EffektiveAusbeute,";
    sql += "RestalkalitaetSoll,";
    sql += "SchnellgaerprobeAktiv,";
    sql += "JungbiermengeAbfuellen,";
    sql += "erg_AbgefuellteBiermenge,";
    sql += "BewertungMaxSterne,";
    sql += "NeuBerechnen,";
    sql += "HefeAnzahlEinheiten,";
    sql += "berechnungsArtHopfen,";
    sql += "highGravityFaktor,";
    sql += "AuswahlBrauanlage,";
    sql += "AuswahlBrauanlageName,";
    sql += "AusbeuteIgnorieren,";
    sql += "Spunden";
    sql += " FROM Sud WHERE ID=:sudid";

    query.prepare(sql);
    query.bindValue(":sudid", id);
    query.bindValue(":sudname", name);
    if (!query.exec())
    {
        error(query);
        commit();
        return 0;
    }

    // neue Sud ID
    sql = "SELECT last_insert_rowid()";
    if (!query.exec(sql))
    {
        error(query);
        commit();
        return 0;
    }
    query.first();
    idNew = query.value(0).toString();

    // Rasten
    sql = "INSERT INTO Rasten (";
    sql += "SudID,";
    sql += "RastAktiv,";
    sql += "RastTemp,";
    sql += "RastDauer,";
    sql += "RastName";
    sql += ")SElECT ";
    sql += ":idNew,";
    sql += "RastAktiv,";
    sql += "RastTemp,";
    sql += "RastDauer,";
    sql += "RastName";
    sql += " FROM Rasten WHERE SudID=:sudid";

    query.prepare(sql);
    query.bindValue(":idNew", idNew);
    query.bindValue(":sudid", id);
    if (!query.exec())
        error(query);

    // Malzschuettung
    sql = "INSERT INTO Malzschuettung (";
    sql += "SudID,";
    sql += "Name,";
    sql += "Prozent,";
    sql += "erg_Menge,";
    sql += "Farbe";
    sql += ")SElECT ";
    sql += ":idNew,";
    sql += "Name,";
    sql += "Prozent,";
    sql += "erg_Menge,";
    sql += "Farbe";
    sql += " FROM Malzschuettung WHERE SudID=:sudid";

    query.prepare(sql);
    query.bindValue(":idNew", idNew);
    query.bindValue(":sudid", id);
    if (!query.exec())
        error(query);

    // Hopfengaben
    sql = "INSERT INTO Hopfengaben (";
    sql += "SudID,";
    sql += "Aktiv,";
    sql += "Name,";
    sql += "Prozent,";
    sql += "Zeit,";
    sql += "erg_Menge,";
    sql += "erg_Hopfentext,";
    sql += "Alpha,";
    sql += "Pellets,";
    sql += "Vorderwuerze";
    sql += ")SElECT ";
    sql += ":idNew,";
    sql += "Aktiv,";
    sql += "Name,";
    sql += "Prozent,";
    sql += "Zeit,";
    sql += "erg_Menge,";
    sql += "erg_Hopfentext,";
    sql += "Alpha,";
    sql += "Pellets,";
    sql += "Vorderwuerze";
    sql += " FROM Hopfengaben WHERE SudID=:sudid";

    query.prepare(sql);
    query.bindValue(":idNew", idNew);
    query.bindValue(":sudid", id);
    if (!query.exec())
        error(query);

    // WeitereZutatenGaben
    sql = "INSERT INTO WeitereZutatenGaben (";
    sql += "SudID,";
    sql += "Name,";
    sql += "Menge,";
    sql += "Einheit,";
    sql += "Typ,";
    sql += "Zeitpunkt,";
    sql += "Bemerkung,";
    sql += "erg_Menge,";
    sql += "Ausbeute,";
    sql += "Farbe,";
    sql += "Zeitpunkt_von,";
    sql += "Zeitpunkt_bis,";
    sql += "Zugabestatus,";
    sql += "Entnahmeindex,";
    sql += "Zugabedauer";
    sql += ")SElECT ";
    sql += ":idNew,";
    sql += "Name,";
    sql += "Menge,";
    sql += "Einheit,";
    sql += "Typ,";
    sql += "Zeitpunkt,";
    sql += "Bemerkung,";
    sql += "erg_Menge,";
    sql += "Ausbeute,";
    sql += "Farbe,";
    sql += "Zeitpunkt_von,";
    sql += "Zeitpunkt_bis,";
    sql += full ? "Zugabestatus," : "0,";
    sql += "Entnahmeindex,";
    sql += "Zugabedauer";
    sql += " FROM WeitereZutatenGaben WHERE SudID=:sudid";

    query.prepare(sql);
    query.bindValue(":idNew", idNew);
    query.bindValue(":sudid", id);
    if (!query.exec())
        error(query);

    // Anhang
    sql = "INSERT INTO Anhang (";
    sql += "SudID,";
    sql += "Pfad";
    sql += ")SElECT ";
    sql += ":idNew,";
    sql += "Pfad";
    sql += " FROM Anhang WHERE SudID=:sudid";

    query.prepare(sql);
    query.bindValue(":idNew", idNew);
    query.bindValue(":sudid", id);
    if (!query.exec())
        error(query);

    // FlaschenlabelTags
    sql = "INSERT INTO FlaschenlabelTags (\
          SudID,\
          Tagname,\
          Value\
          ) SELECT \
          :idNew,\
          Tagname,\
          Value\
          FROM FlaschenlabelTags WHERE SudID=:sudid";

    query.prepare(sql);
    query.bindValue(":idNew", idNew);
    query.bindValue(":sudid", id);
    if (!query.exec())
        error(query);

    // Flaschenlabel
    sql = "INSERT INTO Flaschenlabel (\
          SudID,\
          Auswahl,\
          BreiteLabel,\
          AnzahlLabels,\
          Abstandlabels,\
          SRandOben,\
          SRandLinks,\
          SRandRechts,\
          SRandUnten\
          ) SELECT \
          :idNew,\
          Auswahl,\
          BreiteLabel,\
          AnzahlLabels,\
          Abstandlabels,\
          SRandOben,\
          SRandLinks,\
          SRandRechts,\
          SRandUnten\
          FROM Flaschenlabel WHERE SudID=:sudid";

    query.prepare(sql);
    query.bindValue(":idNew", idNew);
    query.bindValue(":sudid", id);
    if (!query.exec())
        error(query);

    if (!full)
    {
        commit();
        return idNew.toInt();
    }

    // Schnellgaerverlauf
    sql = "INSERT INTO Schnellgaerverlauf (";
    sql += "SudID,";
    sql += "Zeitstempel,";
    sql += "SW,";
    sql += "Alc,";
    sql += "Temp";
    sql += ")SElECT ";
    sql += ":idNew,";
    sql += "Zeitstempel,";
    sql += "SW,";
    sql += "Alc,";
    sql += "Temp";
    sql += " FROM Schnellgaerverlauf WHERE SudID=:sudid";

    query.bindValue(":idNew", idNew);
    query.bindValue(":sudid", id);
    if (!query.exec())
        error(query);

    // Hauptgaerverlauf
    sql = "INSERT INTO Hauptgaerverlauf (";
    sql += "SudID,";
    sql += "Zeitstempel,";
    sql += "SW,";
    sql += "Alc,";
    sql += "Temp";
    sql += ")SElECT ";
    sql += ":idNew,";
    sql += "Zeitstempel,";
    sql += "SW,";
    sql += "Alc,";
    sql += "Temp";
    sql += " FROM Hauptgaerverlauf WHERE SudID=:sudid";

    query.bindValue(":idNew", idNew);
    query.bindValue(":sudid", id);
    if (!query.exec())
        error(query);

    // Nachgaerverlauf
    sql = "INSERT INTO Nachgaerverlauf (";
    sql += "SudID,";
    sql += "Zeitstempel,";
    sql += "Druck,";
    sql += "Temp,";
    sql += "CO2";
    sql += ")SElECT ";
    sql += ":idNew,";
    sql += "Zeitstempel,";
    sql += "Druck,";
    sql += "Temp,";
    sql += "CO2";
    sql += " FROM Nachgaerverlauf WHERE SudID=:sudid";

    query.prepare(sql);
    query.bindValue(":idNew", idNew);
    query.bindValue(":sudid", id);
    if (!query.exec())
        error(query);

    // Bewertungen
    sql = "INSERT INTO Bewertungen (";
    sql += "SudID,";
    sql += "Woche,";
    sql += "Datum,";
    sql += "Sterne,";
    sql += "Bemerkung,";
    sql += "Farbe,";
    sql += "FarbeBemerkung,";
    sql += "Schaum,";
    sql += "SchaumBemerkung,";
    sql += "Geruch,";
    sql += "GeruchBemerkung,";
    sql += "Geschmack,";
    sql += "GeschmackBemerkung,";
    sql += "Antrunk,";
    sql += "AntrunkBemerkung,";
    sql += "Haupttrunk,";
    sql += "HaupttrunkBemerkung,";
    sql += "Nachtrunk,";
    sql += "NachtrunkBemerkung,";
    sql += "Gesamteindruck,";
    sql += "GesamteindruckBemerkung";
    sql += ")SElECT ";
    sql += ":idNew,";
    sql += "Woche,";
    sql += "Datum,";
    sql += "Sterne,";
    sql += "Bemerkung,";
    sql += "Farbe,";
    sql += "FarbeBemerkung,";
    sql += "Schaum,";
    sql += "SchaumBemerkung,";
    sql += "Geruch,";
    sql += "GeruchBemerkung,";
    sql += "Geschmack,";
    sql += "GeschmackBemerkung,";
    sql += "Antrunk,";
    sql += "AntrunkBemerkung,";
    sql += "Haupttrunk,";
    sql += "HaupttrunkBemerkung,";
    sql += "Nachtrunk,";
    sql += "NachtrunkBemerkung,";
    sql += "Gesamteindruck,";
    sql += "GesamteindruckBemerkung";
    sql += " FROM Bewertungen WHERE SudID=:sudid";

    query.prepare(sql);
    query.bindValue(":idNew", idNew);
    query.bindValue(":sudid", id);
    if (!query.exec())
        error(query);

    commit();
    return idNew.toInt();
}
