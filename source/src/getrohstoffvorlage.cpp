#include "getrohstoffvorlage.h"
#include <QDebug>

GetRohstoffVorlage::GetRohstoffVorlage(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GetRohstoffVorlage)
{
	ui->setupUi(this);

	QString ressource = DB_ROHSTOFFVORLAGE;
	QString dbPfad;

	dbPfad = QDir::tempPath() + "/" + DB_ROHSTOFFLISTEN;

	//Wenn Datei schon existiert vorher löschen
	bool dbVorhanden = false;
	if (QFile::exists(dbPfad)) {
		if (!QFile::remove(dbPfad)){
			dbVorhanden = true;
		}
	}

	//Rohstoffdatenbank aus ressource kopieren
	QFile file(ressource);
	if (file.copy(dbPfad)) {
		QFile::setPermissions(dbPfad, QFile::ReadOwner | QFile::WriteOwner);
	}
	else {
		if (!dbVorhanden){
			ErrorMessage *errorMessage = new ErrorMessage();
			errorMessage -> showMessage(ERR_DB_ROHSTOFFLISTEN_KOPIE, TYPE_WARNUNG,
					CANCEL_PROGRAM, QObject::tr("Betroffener Kopierpfad:\n") + dbPfad);
		}
	}

	dbr = QSqlDatabase::addDatabase("QSQLITE","Rohstoffliste");
	// Setzte Datenbank
	dbr.setDatabaseName(dbPfad);
	// Datenbank öffnen
	if (!dbr.open()) {
		// Fehlermeldung
		ErrorMessage *errorMessage = new ErrorMessage();
		errorMessage -> showMessage(ERR_SQL_DB_OPEN, TYPE_KRITISCH,
			CANCEL_PROGRAM, QObject::tr("Betroffene Datei:\n") + DB_ROHSTOFFVORLAGE);

	}


}

GetRohstoffVorlage::~GetRohstoffVorlage()
{
	delete ui;
}

void GetRohstoffVorlage::ViewMalzauswahl()
{
	Rohstoffart = R_Malz;
	QSqlQueryModel *model = new QSqlQueryModel();
	model->setQuery("SELECT * FROM Malz ORDER BY Beschreibung",dbr);

	ui->tableView->setModel(model);
	ui->tableView->setColumnHidden(0,true);
	ui->tableView->resizeColumnsToContents();
}

void GetRohstoffVorlage::ViewHopfenauswahl()
{
	Rohstoffart = R_Hopfen;
	QSqlQueryModel *model = new QSqlQueryModel();
	model->setQuery("SELECT * FROM Hopfen ORDER BY Bezeichnung",dbr);

	ui->tableView->setModel(model);
	ui->tableView->setColumnHidden(0,true);
	ui->tableView->resizeColumnsToContents();
}

void GetRohstoffVorlage::ViewHefeauswahl()
{
	Rohstoffart = R_Hefe;
	QSqlQueryModel *model = new QSqlQueryModel();
	model->setQuery("SELECT * FROM Hefe ORDER BY Bezeichnung",dbr);

	ui->tableView->setModel(model);
	ui->tableView->setColumnHidden(0,true);
	ui->tableView->resizeColumnsToContents();
}

void GetRohstoffVorlage::on_buttonBox_accepted()
{
	//Aktuell ausgewählte Zeile
	int row = ui->tableView->currentIndex().row();
	//Wenn eine Zeile ausgewählt ist werte aus der Tabelle in die Entsprechenden Variablen auslesen
	if (row >= 0){
		if (Rohstoffart == R_Malz){
			//Beschreibung
			QModelIndex index = ui->tableView->model()->index(row,1);
			m_Beschreibung = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
			//Farbe
			index = ui->tableView->model()->index(row,2);
			m_Farbe = ui->tableView->model()->data(index, Qt::DisplayRole).toReal();
			//MaxPorzent
			index = ui->tableView->model()->index(row,3);
			m_MaxProzent = ui->tableView->model()->data(index, Qt::DisplayRole).toInt();
			//Eigenschaften
			index = ui->tableView->model()->index(row,4);
			m_Eigenschaften = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
		}
		else if (Rohstoffart == R_Hopfen){
			//Beschreibung
			QModelIndex index = ui->tableView->model()->index(row,1);
			m_Beschreibung = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
			//Typ
			index = ui->tableView->model()->index(row,2);
			if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "Aroma")
				m_Typ = 1;
			else if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "Bitter")
				m_Typ = 2;
			else if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "Universal")
				m_Typ = 3;
			//Alpha
			index = ui->tableView->model()->index(row,3);
			m_Alpha = ui->tableView->model()->data(index, Qt::DisplayRole).toReal();
			//Eigenschaften
			index = ui->tableView->model()->index(row,4);
			m_Eigenschaften = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
		}
		else if (Rohstoffart == R_Hefe){
			//Wuerzemenge
			QModelIndex index = ui->tableView->model()->index(row,5);
			m_Wuerzemenge = ui->tableView->model()->data(index, Qt::DisplayRole).toReal();
			//Beschreibung
			index = ui->tableView->model()->index(row,1);
			m_Beschreibung = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
			//Verpackungsmenge
			index = ui->tableView->model()->index(row,4);
			m_Verpackungsmenge = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
			//Typ OG UG
			index = ui->tableView->model()->index(row,2);
			if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "OG")
				m_TypOGUG = 1;
			else
				m_TypOGUG = 2;
			//Typ Flüssig Trocken
			index = ui->tableView->model()->index(row,3);
			if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "trocken")
				m_TypFlTr = 1;
			else
				m_TypFlTr = 2;
			//Temperaturbereich
			index = ui->tableView->model()->index(row,6);
			m_Temperatur = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
			//Eigenschaften
			index = ui->tableView->model()->index(row,7);
			m_Eigenschaften = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
			//Sedimentation
			index = ui->tableView->model()->index(row,8);
			if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "hoch")
				m_SED = 1;
			else if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "mittel")
				m_SED = 2;
			else if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "niedrig")
				m_SED = 3;
			else
				m_SED = 0;
			//EVG
			index = ui->tableView->model()->index(row,9);
			m_EVG = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
		}
	}
	b_ok = true;
}

void GetRohstoffVorlage::on_buttonBox_rejected()
{
	b_ok = false;
}
