#ifndef QEXPORT_H
#define QEXPORT_H
//
#include <QObject>
//
class QExport : public QObject
{
Q_OBJECT
private:
	void CheckHopfenEintrag(QString Name);
	void CheckMalzEintrag(QString Name);
	void CheckHefeEintrag(QString Name);
	void CheckWeitereZutatEintrag(QString Name);
	void RohstoffHopfenUebernehmen(QString Name);
	void RohstoffMalzUebernehmen(QString Name);
	void RohstoffWeitereZutatUebernehmen(QString Name);
	void RohstoffHefeUebernehmen(QString Name);
	QString Dateiname;
	bool AbfrageRohstoffuebernahme(QString str, QString rohstoff);
	void HinweisAusgeben(QString Text);
	int VersionDatenbank;
	int VersionXML;
	QString AustauschRohstoff;
public:
	int ImportSudXML(QString cDateiname);
	int errCol;
	int errLine;
	QString errMsg;
	int IfXmlOK(QString cDateiname);
	int ExportSudXML(int SudNr, QString Dateiname);
	int ExportBeerXML(int SudNr, QString Dateiname);
	QExport();
	
};
#endif
