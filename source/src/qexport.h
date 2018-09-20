#ifndef QEXPORT_H
#define QEXPORT_H
//
#include <QObject>
//
class QExport : public QObject
{
Q_OBJECT
private:
    void CheckHopfenEintrag(const QString& Name);
    void CheckMalzEintrag(const QString& Name);
    void CheckHefeEintrag(const QString& Name);
    void CheckWeitereZutatEintrag(const QString& Name);
    void RohstoffHopfenUebernehmen(const QString& Name);
    void RohstoffMalzUebernehmen(const QString& Name);
    void RohstoffWeitereZutatUebernehmen(const QString& Name);
    void RohstoffHefeUebernehmen(const QString& Name);
	QString Dateiname;
    bool AbfrageRohstoffuebernahme(const QString& str, const QString& rohstoff);
    void HinweisAusgeben(const QString& Text);
	int VersionDatenbank;
	int VersionXML;
	QString AustauschRohstoff;
    void encodeHtml(QString& str);
public:
    int ImportSudXML(const QString& cDateiname);
	int errCol;
	int errLine;
	QString errMsg;
    int IfXmlOK(const QString& cDateiname);
    int ExportSudXML(int SudNr, const QString& Dateiname);
    int ExportBeerXML(int SudNr, const QString& Dateiname);
    void convertJSON(const QString& json, const QString& xsud);
	QExport();
	
};
#endif
