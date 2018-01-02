#include "getrohstoffvorlage.h"

#include <QSettings>
#include <QDir>
#include <QFile>
#include <QFileDialog>

#include "definitionen.h"
#include "errormessage.h"
#include "mydsvtablemodel.h"

#define R_Malz 1
#define R_Hopfen 2
#define R_Hefe 3

GetRohstoffVorlage::GetRohstoffVorlage(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GetRohstoffVorlage)
{
    ui->setupUi(this);
}

GetRohstoffVorlage::~GetRohstoffVorlage()
{
	delete ui;
}

QString GetRohstoffVorlage::getFileName(bool withPath) const
{
    QString fileName;
    switch (Rohstoffart)
    {
    case R_Malz:
        fileName = "Malz.csv";
        break;
    case R_Hopfen:
        fileName = "Hopfen.csv";
        break;
    case R_Hefe:
        fileName = "Hefe.csv";
        break;
    }
    if (withPath)
    {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
        return QFileInfo(settings.fileName()).absolutePath() + "/" + fileName;
    }
    else
    {
        return fileName;
    }
}

void GetRohstoffVorlage::viewImpl(int art)
{
    QString dbPath;

    Rohstoffart = art;
    switch (Rohstoffart)
    {
    case R_Malz:
        dbPath = ":/data/Malz.csv";
        break;
    case R_Hopfen:
        dbPath = ":/data/Hopfen.csv";
        break;
    case R_Hefe:
        dbPath = ":/data/Hefe.csv";
        break;
    }

    // copy list from resources to settings directory
    QFile file(getFileName(true));
    if (!file.exists())
    {
        QFile file2(dbPath);
        if (file2.copy(file.fileName()))
            QFile::setPermissions(file.fileName(), QFile::ReadOwner | QFile::WriteOwner);
    }

    // open list from settings directory
    MyDsvTableModel* model = new MyDsvTableModel(this);
    model->loadFromFile(file.fileName(), true, ';');
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
    connect(ui->tableView->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)), this, SLOT(slot_save()));
    connect(ui->tableView->model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(slot_save()));
    connect(ui->tableView->model(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(slot_save()));
}

void GetRohstoffVorlage::ViewMalzauswahl()
{
    viewImpl(R_Malz);
}

void GetRohstoffVorlage::ViewHopfenauswahl()
{
    viewImpl(R_Hopfen);
}

void GetRohstoffVorlage::ViewHefeauswahl()
{
    viewImpl(R_Hefe);
}

void GetRohstoffVorlage::on_buttonBox_accepted()
{
	//Aktuell ausgewählte Zeile
	int row = ui->tableView->currentIndex().row();
	//Wenn eine Zeile ausgewählt ist werte aus der Tabelle in die Entsprechenden Variablen auslesen
	if (row >= 0){
		if (Rohstoffart == R_Malz){
			//Beschreibung
            QModelIndex index = ui->tableView->model()->index(row,0);
			m_Beschreibung = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
			//Farbe
            index = ui->tableView->model()->index(row,1);
			m_Farbe = ui->tableView->model()->data(index, Qt::DisplayRole).toReal();
			//MaxPorzent
            index = ui->tableView->model()->index(row,2);
			m_MaxProzent = ui->tableView->model()->data(index, Qt::DisplayRole).toInt();
			//Eigenschaften
            index = ui->tableView->model()->index(row,3);
			m_Eigenschaften = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
		}
		else if (Rohstoffart == R_Hopfen){
			//Beschreibung
            QModelIndex index = ui->tableView->model()->index(row,0);
			m_Beschreibung = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
			//Typ
            index = ui->tableView->model()->index(row,1);
			if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "Aroma")
				m_Typ = 1;
			else if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "Bitter")
				m_Typ = 2;
			else if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "Universal")
				m_Typ = 3;
			//Alpha
            index = ui->tableView->model()->index(row,2);
			m_Alpha = ui->tableView->model()->data(index, Qt::DisplayRole).toReal();
			//Eigenschaften
            index = ui->tableView->model()->index(row,3);
			m_Eigenschaften = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
		}
		else if (Rohstoffart == R_Hefe){
			//Wuerzemenge
            QModelIndex index = ui->tableView->model()->index(row,4);
			m_Wuerzemenge = ui->tableView->model()->data(index, Qt::DisplayRole).toReal();
			//Beschreibung
            index = ui->tableView->model()->index(row,0);
			m_Beschreibung = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
			//Verpackungsmenge
            index = ui->tableView->model()->index(row,3);
			m_Verpackungsmenge = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
			//Typ OG UG
            index = ui->tableView->model()->index(row,1);
			if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "OG")
				m_TypOGUG = 1;
			else
				m_TypOGUG = 2;
			//Typ Flüssig Trocken
            index = ui->tableView->model()->index(row,2);
			if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "trocken")
				m_TypFlTr = 1;
			else
				m_TypFlTr = 2;
			//Temperaturbereich
            index = ui->tableView->model()->index(row,5);
			m_Temperatur = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
			//Eigenschaften
            index = ui->tableView->model()->index(row,6);
			m_Eigenschaften = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
			//Sedimentation
            index = ui->tableView->model()->index(row,7);
			if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "hoch")
				m_SED = 1;
			else if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "mittel")
				m_SED = 2;
			else if (ui->tableView->model()->data(index, Qt::DisplayRole).toString() == "niedrig")
				m_SED = 3;
			else
				m_SED = 0;
			//EVG
            index = ui->tableView->model()->index(row,8);
			m_EVG = ui->tableView->model()->data(index, Qt::DisplayRole).toString();
		}
	}
    accept();
}


void GetRohstoffVorlage::slot_save()
{
    MyDsvTableModel *model = qobject_cast<MyDsvTableModel*>(ui->tableView->model());
    model->save(getFileName(true), ';');
}

void GetRohstoffVorlage::on_buttonBox_rejected()
{
    reject();
}

void GetRohstoffVorlage::on_btn_Add_clicked()
{
    ui->tableView->model()->insertRow(ui->tableView->model()->rowCount());
    ui->tableView->scrollToBottom();
}

void GetRohstoffVorlage::on_btn_Remove_clicked()
{
    int row = ui->tableView->currentIndex().row();
    if (row >= 0)
        ui->tableView->model()->removeRow(row);
}

void GetRohstoffVorlage::on_btn_Import_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Rohstoffliste importieren"),
                                                    QDir::currentPath() + "/" + getFileName(false),
                                                    "CSV (*.csv)");
    if (!fileName.isEmpty())
    {
        QFile file(getFileName(true));
        QFile::remove(file.fileName());
        QFile file2(fileName);
        if (file2.copy(file.fileName()))
            QFile::setPermissions(file.fileName(), QFile::ReadOwner | QFile::WriteOwner);
        viewImpl(Rohstoffart);
    }
}

void GetRohstoffVorlage::on_btn_Export_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Rohstoffliste exportieren"),
                                                    QDir::currentPath() + "/" + getFileName(false),
                                                    "CSV (*.csv)");
    if (!fileName.isEmpty())
    {
        MyDsvTableModel *model = qobject_cast<MyDsvTableModel*>(ui->tableView->model());
        model->save(fileName, ';');
    }
}

void GetRohstoffVorlage::on_btn_Restore_clicked()
{
    QFile::remove(getFileName(true));
    viewImpl(Rohstoffart);
}
