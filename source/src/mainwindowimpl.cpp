#include "mainwindowimpl.h"
#include <QSettings>
#include <QString>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QList>
#include <QBrush>
#include <time.h>
#include <QDebug>
#include <QStyleFactory>
#include <QUrl>
#include <QFileInfo>
#include <QDesktopServices>
#include <QTemporaryFile>

#include <qmath.h>
#include "errormessage.h"
#include "definitionen.h"
#include "einstellungsdialogimpl.h"
#include "getrohstoffvorlage.h"
#include "rohstoffaustauschen.h"
#include "database.h"
#include "dialog_berschuettungimpl.h"
#include "dialog_berechne_ibuimpl.h"
#include "dialogberverdampfung.h"
#include "brauanlage.h"
#include "dialoginfo.h"
#include "dialogeinmaischetemp.h"
#include "dialogsudteilen.h"
#include "mytablewidgetitemnumeric.h"
//
MainWindowImpl::MainWindowImpl( QWidget * parent,  Qt::WindowFlags f)
  : QMainWindow(parent, f)
{
  Gestartet = false;
  AenderungRohstofftabelle = false;
  ComboboxWirdGefuellt = false;
  AktuelleSudID = 0;
  AmLaden = false;
  NichtBerechnen = false;
  NeuBerechnen = 0;
  Aenderung = false;
  reconnect = false;

  setupUi(this);
  initUi();

  //Maximale Anzahl Sterne einlesen
  LeseMaxAnzahlSterne();

  graphicsView_bewStar->init(StyleDunkel);
  graphicsView_bewStar->setMaxStar(MaxAnzahlSterne);
  label_bew_ID->setVisible(false);

  //Windowicon setzten
  appIcon.addFile(":/global/logo.svg",QSize(64,64));
  setWindowIcon(appIcon);

  //Überprüfen ob ergebnisse in der Datenbank neu berechnet werden müssen
  if (CheckDBNeuBerechnen()){
    DBErgebnisseNeuBerechnen();
  }

  QList<int> sizes = splitter_Schnellgaerverlauf -> sizes();
  sizes.first() = 1;
  sizes.last() = 1;
  splitter_Schnellgaerverlauf -> setSizes(sizes);
  splitter_Hauptgaerverlauf -> setSizes(sizes);
  splitter_Nachgaerverlauf -> setSizes(sizes);

  splitter_Sudauswahl->setSizes(QList<int>() << INT_MAX << 0);
  splitter_Sudauswahl->setStretchFactor(0, 1);
  splitter_Sudauswahl->setStretchFactor(1, 0);

  //Diagrammfarben setzen
  SetDiagrammFarben();

  //In Brau und Gärdaten Ergenbisfelder entsprechend ein Ausblenden
  SetAnsicht();


  connect(graphicsView_bewStar, SIGNAL( sig_AnzahlStarChanged(int) ), this, SLOT( slot_GraphicsView_AnzahlSterneChanged(int) ));

  //verbinde Button Neuer Eintrag Geräte mit Funktion;
  connect(pushButton_GeraeteNeu, SIGNAL( clicked() ), this, SLOT( slot_pushButton_GeraeteNeu() ));
  //verbinde Button Eintrag Löschen Geräte mit Funktion;
  connect(pushButton_GeraeteDel, SIGNAL( clicked() ), this, SLOT( slot_pushButton_GeraeteDel() ));

  //verbinde Button Sud wurde Gebraut mit Funktion;
  connect(pushButton_RohstoffeAbziehen, SIGNAL( clicked() ), this, SLOT( slot_pushButton_gebraut() ));

  //verbinde Button Sud wurde Abgefüllt mit Funktion;
  connect(pushButton_SudAbgefuellt, SIGNAL( clicked() ), this, SLOT( slot_pushButton_SudAbgefuellt() ));

  //verbinde Button Braudatum Heute mit Funktion
  connect(pushButton_BraudatumHeute, SIGNAL( clicked() ), this, SLOT( slot_pushButton_BraudatumHeute() ));

  //verbinde Button Anstelldatum Heute mit Funktion
  connect(pushButton_AnstelldatumHeute, SIGNAL( clicked() ), this, SLOT( slot_pushButton_AnstelldatumHeute() ));

  //verbinde Button Braudatum Heute mit Funktion
  connect(pushButton_AbfuelldatumHeute, SIGNAL( clicked() ), this, SLOT( slot_pushButton_AbfuelldatumHeute() ));

  //verbinde Button Eingabehilfe Stammwürze Kochende
  connect(pushButton_EingabeHSWKochende, SIGNAL( clicked() ), this, SLOT( slot_pushButton_EingabeHSWKochende() ));

  //verbinde Button Eingabehilfe Stammwürze Anstellen
  connect(pushButton_EingabeHSWAnstellen, SIGNAL( clicked() ), this, SLOT( slot_pushButton_EingabeHSWAnstellen() ));

  //verbinde Button Eingabehilfe Stammwürze Schnellgärprobe
  connect(pushButton_EingabeHSWSchnellgaerprobe, SIGNAL( clicked() ), this, SLOT( slot_pushButton_EingabeHSWSchnellgaerprobe() ));

  //verbinde Button Eingabehilfe Stammwürze Jungbier
  connect(pushButton_EingabeHSWJungbier, SIGNAL( clicked() ), this, SLOT( slot_pushButton_EingabeHSWJungbier() ));

  //verbinde Button Neuen Sud Anlegen mit Funkiton
  connect(pushButton_SudNeu, SIGNAL( clicked() ), this, SLOT( slot_pushButton_SudNeu() ));
  //verbinde Button Neuen Sud löschen mit Funkiton
  connect(pushButton_SudDel, SIGNAL( clicked() ), this, SLOT( slot_pushButton_SudDel() ));
  //verbinde Button Neuen Sud Laden mit Funkiton
  connect(pushButton_SudLaden, SIGNAL( clicked() ), this, SLOT( slot_pushButton_SudLaden() ));
  //verbinde Button Sud Kopieren mit Funkiton
  connect(pushButton_SudKopie, SIGNAL( clicked() ), this, SLOT( slot_pushButton_SudKopie() ));

  //verbinde Filter Radiobutton mit Funktion
  connect(radioButton_FilterAlle, SIGNAL( clicked(bool) ), this, SLOT( slot_FilterClicked(bool) ));
  connect(radioButton_FilterNichtGebraut, SIGNAL( clicked(bool) ), this, SLOT( slot_FilterClicked(bool) ));
  connect(radioButton_FilterGebrautNichtAbgefuellt, SIGNAL( clicked(bool) ), this, SLOT( slot_FilterClicked(bool) ));
  connect(radioButton_Abgefuellt, SIGNAL( clicked(bool) ), this, SLOT( slot_FilterClicked(bool) ));
  connect(radioButton_nichtVerbraucht, SIGNAL( clicked(bool) ), this, SLOT( slot_FilterClicked(bool) ));
  connect(radioButton_Merkliste, SIGNAL( clicked(bool) ), this, SLOT( slot_FilterClicked(bool) ));

  //verbinde changed signale mit gemeinsammer funktion zur Neuberechnung der Daten
  connect(spinBox_AngenommeneAusbeute, SIGNAL( valueChanged(int) ), this, SLOT( slot_spinBoxValueChanged(int) ));
  connect(spinBox_SW, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_Gesammtkochdauer, SIGNAL( valueChanged(int) ), this, SLOT( slot_spinBoxValueChanged(int) ));
  connect(spinBox_IBU, SIGNAL( valueChanged(int) ), this, SLOT( slot_spinBoxValueChanged(int) ));
  connect(spinBox_EinmaischenTemp, SIGNAL( valueChanged(int) ), this, SLOT( slot_spinBoxValueChanged(int) ));
  connect(spinBox_Reifezeit, SIGNAL( valueChanged(int) ), this, SLOT( slot_spinBoxValueChanged(int) ));
  connect(spinBox_NachisomerisierungsZeit, SIGNAL( valueChanged(int) ), this, SLOT( slot_spinBoxValueChanged(int) ));
  connect(spinBox_AnzahlHefeEinheiten, SIGNAL( valueChanged(int) ), this, SLOT( slot_spinBoxValueChanged(int) ));

  connect(checkBox_SchnellgaerprobeAktiv, SIGNAL( stateChanged(int) ), this, SLOT( slot_spinBoxValueChanged(int) ));
  connect(checkBox_Spunden, SIGNAL( stateChanged(int) ), this, SLOT( slot_spinBoxValueChanged(int) ));

  //Setzt den Maxwert Zeit für die Hopfengaben
  connect(spinBox_Gesammtkochdauer, SIGNAL( valueChanged(int) ), this, SLOT( slot_spinBoxGesammtkochdauerChanged(int) ));

  connect(doubleSpinBox_Verdampfung, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(dspinBox_KostenAusruestung, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(dSpinBox_KorrekturNachguss, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_Menge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_WuerzemengeKochende, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_SWKochende, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_Speisemenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_WuerzemengeAnstellen, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_SWAnstellen, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_SWSchnellgaerprobe, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_SWJungbier, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_TemperaturJungbier, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(doubleSpinBox_CO2, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(doubleSpinBox_FaktorHG, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_Nebenkosten, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_WuerzemengeVorHopfenseihen, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_SWVorHopfenseihen, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_JungbiermengeAbfuellen, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));

  connect(spinBox_MaischebottichHoehe, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_MaischebottichDurchmesser, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_MaischebottichMaxFuellhoehe, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_SudpfanneHoehe, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_SudpfanneDurchmesser, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBox_SudpfanneMaxFuellhoehe, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));

  connect(SpinBox_waSollRestalkalitaet_dh, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(SpinBox_wwCalcium_mg, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(SpinBox_wwMagnesium_mg, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(SpinBox_wwSaeurekapazitaet_mmol, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));

  connect(comboBox_AuswahlHefe, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  connect(comboBox_AuswahlBrauanlage, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));

  //textEdit Text änderung
  connect(textEdit_Kommentar, SIGNAL( textChanged() ), this, SLOT( slot_Changed() ));

  //änderung im Tablewidged
  connect(tableWidget_Malz, SIGNAL( cellChanged(int, int) ), this, SLOT( slot_TableWidget_cellChanged(int, int) ));
  connect(tableWidget_Hopfen, SIGNAL( cellChanged(int, int) ), this, SLOT( slot_TableWidget_cellChanged(int, int) ));
  connect(tableWidget_Hefe, SIGNAL( cellChanged(int, int) ), this, SLOT( slot_TableWidget_cellChanged(int, int) ));
  connect(tableWidget_WeitereZutaten, SIGNAL( cellChanged(int, int) ), this, SLOT( slot_TableWidget_cellChanged(int, int) ));

  //Tab wird gewechselt
  connect(tabWidged, SIGNAL( currentChanged(int) ), this, SLOT( slot_tabWidgetChanged(int) ));

  //SpinBox Stammwürze nach Kochende mit SpinBox Stammwüzre vor dem Hopfenseihen verbinden
  connect(spinBox_SWKochende, SIGNAL( valueChanged(double) ), spinBox_SWVorHopfenseihen, SLOT( setValue(double) ));

  //SpinBox Stammwürze vor dem Hopfenseigen ausblenden da nicht mehr benötigt
  label_116 -> hide();
  spinBox_SWVorHopfenseihen -> hide();
  label_117 -> hide();
  pushButton_EingabeHSWVorHopfenseihen -> hide();
  horizontalLayout_107 -> setSpacing(0);
  
  LeseKonfig();

  createActions();
  createMenus();
  retranslate();

  //Überprüfen ob Messages angezeigt werden sollen
  if (!keinInternet)
    checkMsg();

  //Daten Einlesen
  DatenEinlesenDB();

  //Erweiterte Zutatenliste erstellen
  ErstelleZutatenlisten();

  //Sudauswahl Füllen
  FuelleSudauswahl();

  //Bierbild laden
  LadeBild();

  AenderungAusruestung = false;
  AenderungGeraeteliste = false;
  AenderungHauptgaerverlauf = false;

  setButtonsTextMerken();
  setFensterTitel();
  Gestartet = true;

  //letzte Suddaten laden
  if (AktuelleSudID == 0)
      AktuelleSudID = 1;
  LadeSudDB(true);
}

void MainWindowImpl::initUi()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

    // Style
    settings.beginGroup("Style");
    NativStyle = settings.value("NativStyle").toBool();
    StyleDunkel = !NativStyle && settings.value("Farbgebung").toInt() == 2;
    settings.endGroup();

    // Sprache
    settings.beginGroup("Sprache");
    loadSprache(settings.value("sprachauswahl", QLocale::system().name()).toString());
    settings.endGroup();

    settings.beginGroup("MainWindow");

    // tableWidget_Sudauswahl
    tableWidget_Sudauswahl->setColumnHidden(0, true);
    tableWidget_Sudauswahl->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableWidget_Sudauswahl->horizontalHeader()->resizeSection(2, 150);
    tableWidget_Sudauswahl->horizontalHeader()->resizeSection(3, 150);
    tableWidget_Sudauswahl->horizontalHeader()->resizeSection(4, 150);
    tableWidget_Sudauswahl->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableWidget_Sudauswahl->sortByColumn(2, Qt::DescendingOrder);
    for (int i = 0; i < tableWidget_Sudauswahl->columnCount(); ++i)
    {
        QVariant var = settings.value("tableWidget_Sudauswahl_col" + QString::number(i));
        if (var.isValid())
            tableWidget_Sudauswahl->horizontalHeader()->resizeSection(i, var.toInt());
    }

    // tableWidget_Schnellgaerverlauf
    tableWidget_Schnellgaerverlauf->sortByColumn(0, Qt::AscendingOrder);

    // tableWidget_Hauptgaerverlauf
    tableWidget_Hauptgaerverlauf->sortByColumn(0, Qt::AscendingOrder);

    // tableWidget_Nachgaerverlauf
    tableWidget_Nachgaerverlauf->sortByColumn(0, Qt::AscendingOrder);

    // tableWidget_Brauuebersicht
    tableWidget_Brauuebersicht->setColumnHidden(0, true);
    tableWidget_Brauuebersicht->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableWidget_Brauuebersicht->horizontalHeader()->resizeSection(1, 300);
    tableWidget_Brauuebersicht->horizontalHeader()->setMinimumSectionSize(40);
    tableWidget_Brauuebersicht->sortByColumn(2, Qt::DescendingOrder);
    for (int i = 0; i < tableWidget_Brauuebersicht->columnCount(); ++i)
    {
        QVariant var = settings.value("tableWidget_Brauuebersicht_col" + QString::number(i));
        if (var.isValid())
            tableWidget_Brauuebersicht->horizontalHeader()->resizeSection(i, var.toInt());
    }

    // tableWidget_Malz
    tableWidget_Malz->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableWidget_Malz->horizontalHeader()->resizeSection(TableMalzColName, 200);
    tableWidget_Malz->horizontalHeader()->resizeSection(TableMalzColBemerkung, 200);
    tableWidget_Malz->horizontalHeader()->resizeSection(TableMalzColAnwendung, 200);
    tableWidget_Malz->horizontalHeader()->resizeSection(TableMalzColLink, 100);
    tableWidget_Malz->horizontalHeader()->setMinimumSectionSize(80);
    for (int i = 0; i < tableWidget_Malz->columnCount(); ++i)
    {
        QVariant var = settings.value("tableWidget_Malz_col" + QString::number(i));
        if (var.isValid())
            tableWidget_Malz->horizontalHeader()->resizeSection(i, var.toInt());
    }

    // tableWidget_Hopfen
    tableWidget_Hopfen->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableWidget_Hopfen->horizontalHeader()->resizeSection(TableHopfenColName, 200);
    tableWidget_Hopfen->horizontalHeader()->resizeSection(TableHopfenColBemerkung, 200);
    tableWidget_Hopfen->horizontalHeader()->resizeSection(TableHopfenColEigenschaften, 200);
    tableWidget_Hopfen->horizontalHeader()->resizeSection(TableHopfenColLink, 100);
    tableWidget_Hopfen->horizontalHeader()->setMinimumSectionSize(80);
    for (int i = 0; i < tableWidget_Hopfen->columnCount(); ++i)
    {
        QVariant var = settings.value("tableWidget_Hopfen_col" + QString::number(i));
        if (var.isValid())
            tableWidget_Hopfen->horizontalHeader()->resizeSection(i, var.toInt());
    }

    // tableWidget_Hefe
    tableWidget_Hefe->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableWidget_Hefe->horizontalHeader()->resizeSection(TableHefeColName, 200);
    tableWidget_Hefe->horizontalHeader()->resizeSection(TableHefeColBemerkung, 200);
    tableWidget_Hefe->horizontalHeader()->resizeSection(TableHefeColEigenschaften, 200);
    tableWidget_Hefe->horizontalHeader()->resizeSection(TableHefeColLink, 100);
    tableWidget_Hefe->horizontalHeader()->setMinimumSectionSize(80);
    for (int i = 0; i < tableWidget_Hefe->columnCount(); ++i)
    {
        QVariant var = settings.value("tableWidget_Hefe_col" + QString::number(i));
        if (var.isValid())
            tableWidget_Hefe->horizontalHeader()->resizeSection(i, var.toInt());
    }

    // tableWidget_WeitereZutaten
    tableWidget_WeitereZutaten->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableWidget_WeitereZutaten->horizontalHeader()->resizeSection(TableWZutatColName, 200);
    tableWidget_WeitereZutaten->horizontalHeader()->resizeSection(TableWZutatColBemerkung, 200);
    tableWidget_WeitereZutaten->horizontalHeader()->resizeSection(TableWZutatColLink, 100);
    tableWidget_WeitereZutaten->horizontalHeader()->setMinimumSectionSize(80);
    for (int i = 0; i < tableWidget_WeitereZutaten->columnCount(); ++i)
    {
        QVariant var = settings.value("tableWidget_WeitereZutaten_col" + QString::number(i));
        if (var.isValid())
            tableWidget_WeitereZutaten->horizontalHeader()->resizeSection(i, var.toInt());
    }

    settings.endGroup();
}

void MainWindowImpl::retranslate()
{
    ErstelleUeber();
    retranslateMenus();
    tabWidged->setTabText(tabWidged->indexOf(tab_Spickzettel), BierWurdeGebraut ? trUtf8("Zusammenfassung") : trUtf8("Spickzettel"));
}

void MainWindowImpl::on_MsgCheckFertig(int count)
{
  if (count > 0){
    msgdlg.exec();

    //Liste mit Ingnor IDs wieder in der db ablegen
    QList<int> igl;
    igl = msgdlg.getIgnorIDList();

    //erst mal alle vorhandenen ids löschen
    QSqlQuery query;
    QString sql = "DELETE FROM IgnorMsgID";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }

    //dann Liste neu füllen
    for (int i=0; i< igl.count(); i++){
      sql = "INSERT into IgnorMsgID(MsgID) VALUES(" + QString::number(igl[i]) + ")";
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
    }
  }
}

void MainWindowImpl::closeEvent(QCloseEvent *evt)
{
    bool close = true;
    if (Aenderung)
        close = AbfrageSpeichern();
    else
        close = QMessageBox::question(this, APP_NAME,
                                      trUtf8("Anwendung schliessen?"),
                                      QMessageBox::Cancel | QMessageBox::Yes,
                                      QMessageBox::Yes) == QMessageBox::Yes;
    if (close)
    {
        SchreibeKonfig();
        evt->accept();
    }
    else
    {
        evt->ignore();
    }
}

void MainWindowImpl::changeEvent(QEvent* event)
{
    if(event)
    {
        switch (event->type())
        {
        case QEvent::LanguageChange:
            Gestartet = false;
            retranslateUi(this);
            retranslate();
            Gestartet = true;
            break;
        case QEvent::LocaleChange:
            loadSprache(QLocale::system().name());
            break;
        default:
            break;
        }
    }
    QMainWindow::changeEvent(event);
}

int MainWindowImpl::getBrauanlagenIDRezept()
{
  int id = 0;
  for (int i=0; i < listWidget_Brauanlagen->count(); i++) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
    if (item->text() == comboBox_AuswahlBrauanlage->currentText()) {
      id = item->getID();
    }
  }
  return id;
}

int MainWindowImpl::getBrauanlagenIDAusruestung()
{
  int id = 0;
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->currentItem());
  id = item->getID();
  return id;
}

double MainWindowImpl::getAngenommeneSudhausausbeute()
{
  double r=0;
  //Angenommene Sudhausausbeute je nach Brauanlagenauswahl
  for (int i=0; i<listWidget_Brauanlagen->count();i++) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
    if (item != 0) {
      if (item->text() == comboBox_AuswahlBrauanlage->currentText()) {
        r= item->getSudhausausbeute();
      }
    }
  }
  return r;
}

void MainWindowImpl::setAngenommeneSudhausausbeute(int value)
{
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    item->setSudhausausbeute(value);
  }
}

double MainWindowImpl::getKorrekturWassermenge()
{
  double r=0;
  //Angenommene Sudhausausbeute je nach Brauanlagenauswahl
  for (int i=0; i<listWidget_Brauanlagen->count();i++) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
    if (item != 0) {
      if (item->text() == comboBox_AuswahlBrauanlage->currentText()) {
        r= item->getKorrekturWasser();
      }
    }
  }
  return r;
}

void MainWindowImpl::setKorrekturWassermenge(double value)
{
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    item->setKorrekturWasser(value);
  }
}

int MainWindowImpl::getKorrekturFarbe()
{
  double r=0;
  //Angenommene Sudhausausbeute je nach Brauanlagenauswahl
  for (int i=0; i<listWidget_Brauanlagen->count();i++) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
    if (item != 0) {
      if (item->text() == comboBox_AuswahlBrauanlage->currentText()) {
        r= item->getKorrekturFarbe();
      }
    }
  }
  return r;
}

void MainWindowImpl::setKorrekturFarbe(int value)
{
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    item->setKorrekturFarbe(value);
  }
}

double MainWindowImpl::getVerdampfungsziffer()
{
  double r=0;
  //Angenommene Sudhausausbeute je nach Brauanlagenauswahl
  for (int i=0; i<listWidget_Brauanlagen->count();i++) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
    if (item != 0) {
      if (item->text() == comboBox_AuswahlBrauanlage->currentText()) {
        r= item->getVerdampfungsziffer();
      }
    }
  }
  return r;
}

void MainWindowImpl::setVerdampfungsziffer(double value)
{
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    item->setVerdampfungsziffer(value);
  }
}

double MainWindowImpl::getBrauanlageKosten()
{
  double r=0;
  //kosten
  for (int i=0; i<listWidget_Brauanlagen->count();i++) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
    if (item != 0) {
      if (item->text() == comboBox_AuswahlBrauanlage->currentText()) {
        r= item->getKosten();
      }
    }
  }
  return r;
}

void MainWindowImpl::setBrauanlageKosten(double value)
{
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    item->setKosten(value);
  }
}

double MainWindowImpl::getMaischebottichHoehe()
{
  double r=0;
  //Angenommene Sudhausausbeute je nach Brauanlagenauswahl
  for (int i=0; i<listWidget_Brauanlagen->count();i++) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
    if (item != 0) {
      if (item->text() == comboBox_AuswahlBrauanlage->currentText()) {
        r= item->getMaischebottich_Hoehe();
      }
    }
  }
  return r;
}

void MainWindowImpl::setMaischebottichHoehe(double value)
{
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    item->setMaischebottich_Hoehe(value);
  }
}

double MainWindowImpl::getMaischebottichDurchmesser()
{
  double r=0;
  //Angenommene Sudhausausbeute je nach Brauanlagenauswahl
  for (int i=0; i<listWidget_Brauanlagen->count();i++) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
    if (item != 0) {
      if (item->text() == comboBox_AuswahlBrauanlage->currentText()) {
        r= item->getMaischebottich_Durchmesser();
      }
    }
  }
  return r;
}

void MainWindowImpl::setMaischebottichDurchmesser(double value)
{
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    item->setMaischebottich_Durchmesser(value);
  }
}

double MainWindowImpl::getMaischebottichMaxFuellhoehe()
{
  double r=0;
  //Angenommene Sudhausausbeute je nach Brauanlagenauswahl
  for (int i=0; i<listWidget_Brauanlagen->count();i++) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
    if (item != 0) {
      if (item->text() == comboBox_AuswahlBrauanlage->currentText()) {
        r= item->getMaischebottich_MaxFuellhoehe();
      }
    }
  }
  return r;
}

void MainWindowImpl::setMaischebottichMaxFuellhoehe(double value)
{
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    item->setMaischebottich_MaxFuellhoehe(value);
  }
}

double MainWindowImpl::getSudpfanneHoehe()
{
  double r=0;
  //Angenommene Sudhausausbeute je nach Brauanlagenauswahl
  for (int i=0; i<listWidget_Brauanlagen->count();i++) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
    if (item != 0) {
      if (item->text() == comboBox_AuswahlBrauanlage->currentText()) {
        r= item->getSudpfanne_Hoehe();
      }
    }
  }
  return r;
}

void MainWindowImpl::setSudpfanneHoehe(double value)
{
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    item->setSudpfanne_Hoehe(value);
  }
}

double MainWindowImpl::getSudpfanneDurchmesser()
{
  double r=0;
  //Angenommene Sudhausausbeute je nach Brauanlagenauswahl
  for (int i=0; i<listWidget_Brauanlagen->count();i++) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
    if (item != 0) {
      if (item->text() == comboBox_AuswahlBrauanlage->currentText()) {
        r= item->getSudpfanne_Durchmesser();
      }
    }
  }
  return r;
}

void MainWindowImpl::setSudpfanneDurchmesser(double value)
{
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    item->setSudpfanne_Durchmesser(value);
  }
}

double MainWindowImpl::getSudpfanneMaxFuellhoehe()
{
  double r=0;
  //Angenommene Sudhausausbeute je nach Brauanlagenauswahl
  for (int i=0; i<listWidget_Brauanlagen->count();i++) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
    if (item != 0) {
      if (item->text() == comboBox_AuswahlBrauanlage->currentText()) {
        r= item->getSudpfanne_MaxFuellhoehe();
      }
    }
  }
  return r;
}

void MainWindowImpl::setSudpfanneMaxFuellhoehe(double value)
{
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    item->setSudpfanne_MaxFuellhoehe(value);
  }
}

double MainWindowImpl::getMaischenMaxNutzvolumen()
{
  double Grundflaeche;

  Grundflaeche = pow(getMaischebottichDurchmesser()/2, 2) * M_PI/10;
  return double(qRound((Grundflaeche * getMaischebottichMaxFuellhoehe() / 100) *100)) / 100;
}

double MainWindowImpl::getSudpfanneMaxNutzvolumen()
{
  double Grundflaeche;

  Grundflaeche = pow(getSudpfanneDurchmesser()/2, 2) * M_PI/10;
  return double(qRound((Grundflaeche * getSudpfanneMaxFuellhoehe() / 100) *100)) / 100;
}

void MainWindowImpl::switchTranslator(QTranslator& translator, const QString& filename)
{
    qApp->removeTranslator(&translator);
    if(translator.load(filename))
        qApp->installTranslator(&translator);
}

void MainWindowImpl::loadSprache(const QString &rLanguage)
{
    if(m_currLang != rLanguage)
    {
        m_currLang = rLanguage;
        QLocale locale = QLocale(m_currLang);
        QLocale::setDefault(locale);
        tabWidged->setLocale(locale);
        tabWidged->setVisible(false);
        tabWidged->setVisible(true);
        QString langPath = QApplication::applicationDirPath().append("/languages");
        switchTranslator(m_translator, QString(langPath+"/kb_%1.qm").arg(m_currLang));
        switchTranslator(m_translatorQt, QString(langPath+"/qt_%1.qm").arg(m_currLang));
    }
}

void MainWindowImpl::LeseMaxAnzahlSterne()
{
  QString str;
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("Erweitert");

  //Maximale Anzahl Sterne
  str = settings.value("MaxAnzahlSterne").toString();
  if (str == ""){
    settings.setValue("MaxAnzahlSterne", BEW_ANZAHL_STERNE);
  }

  MaxAnzahlSterne = settings.value("MaxAnzahlSterne").toInt();

  settings.endGroup();
}

void MainWindowImpl::checkMsg()
{
  //Nur abrufen wenn in den Einstellungen nicht abgeschaltet wurde
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("Ansicht");

  bool b = settings.value("checkBox_MsgNichtAnzeigen").toBool();

  settings.endGroup();

  if (!b){
    QList<int> idl;

    msgdlg.setURL(MSG_URL);

    //Zu Ignorierende MsgIds abrufen
    QSqlQuery query;
    QString sql = "SELECT * FROM IgnorMsgID";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    else {
      while (query.next()){
        int FeldNr = query.record().indexOf("MsgID");
        idl.append(query.value(FeldNr).toInt());
      }
    }

    msgdlg.checkMsgAnzeigen(idl);
    connect(&msgdlg, SIGNAL(sig_checkFertig(int)), this, SLOT(on_MsgCheckFertig(int)));
  }

}

void MainWindowImpl::setAenderung(bool value)
{
  if (Gestartet && !AmLaden) {
    Aenderung = value;
    setFensterTitel();
  }
}

void MainWindowImpl::LeseAusruestungDB()
{
  QSqlQuery query;
  QString sql = "SELECT * FROM Ausruestung;";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    listWidget_Brauanlagen->clear();
    while (query.next()) {
      int FeldNr;
      Brauanlage *item;
      item = new Brauanlage();
      //ID
      FeldNr = query.record().indexOf("AnlagenID");
      //qDebug() << "FeldNr" << FeldNr;
      item->setID(query.value(FeldNr).toInt());
      //Ausrüstungsname der Auswahlliste hinzufügen
      FeldNr = query.record().indexOf("Name");
      item->setText(query.value(FeldNr).toString());
      listWidget_Brauanlagen->addItem(item);
      //Ausbeute zur Berechnung der Schüttung
      FeldNr = query.record().indexOf("Sudhausausbeute");
      item->setSudhausausbeute(query.value(FeldNr).toInt());
      //Verdampfungsziffer
      FeldNr = query.record().indexOf("Verdampfungsziffer");
      item->setVerdampfungsziffer(query.value(FeldNr).toDouble());
      //Korrektur der Nachgussmenge
      FeldNr = query.record().indexOf("KorrekturWasser");
      item->setKorrekturWasser(query.value(FeldNr).toDouble());
      //Korrektur der Farbe
      FeldNr = query.record().indexOf("KorrekturFarbe");
      item->setKorrekturFarbe(query.value(FeldNr).toInt());
      //Maischebottich Höhe
      FeldNr = query.record().indexOf("Maischebottich_Hoehe");
      item->setMaischebottich_Hoehe(query.value(FeldNr).toDouble());
      //Maischebottich Durchmesser
      FeldNr = query.record().indexOf("Maischebottich_Durchmesser");
      item->setMaischebottich_Durchmesser(query.value(FeldNr).toDouble());
      //Maischebottich Maximal nutzbare Füllhöhe
      FeldNr = query.record().indexOf("Maischebottich_MaxFuellhoehe");
      item->setMaischebottich_MaxFuellhoehe(query.value(FeldNr).toDouble());
      //Sudpfanne Höhe
      FeldNr = query.record().indexOf("Sudpfanne_Hoehe");
      item->setSudpfanne_Hoehe(query.value(FeldNr).toDouble());
      //Sudpfanne Durchmesser
      FeldNr = query.record().indexOf("Sudpfanne_Durchmesser");
      item->setSudpfanne_Durchmesser(query.value(FeldNr).toDouble());
      //Sudpfanne Maximal nutzbare Füllhöhe
      FeldNr = query.record().indexOf("Sudpfanne_MaxFuellhoehe");
      item->setSudpfanne_MaxFuellhoehe(query.value(FeldNr).toDouble());
      //Kosten
      FeldNr = query.record().indexOf("Kosten");
      item->setKosten(query.value(FeldNr).toDouble());

      //Editierbar setzten
      Qt::ItemFlags flags;
      flags = item->flags();
      flags |= Qt::ItemIsSelectable | Qt::ItemIsEditable;
      item->setFlags(flags);
    }
    listWidget_Brauanlagen->setCurrentRow(0);
  }
}


void MainWindowImpl::SchreibeAusruestungDB()
{
  if (AenderungAusruestung) {
    QSqlQuery query;
    //Erst mal alle eintrage löschen dann alle schreiben
    QString sql = "DELETE FROM Ausruestung WHERE ID>0";
    //Abfrage Abschicken
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    for (int i = 0; i < listWidget_Brauanlagen->count(); i++) {
      Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
      sql = "INSERT INTO 'Ausruestung' ('Name', 'AnlagenID', 'Maischebottich_Hoehe','Maischebottich_Durchmesser','Maischebottich_MaxFuellhoehe','Sudpfanne_Hoehe','Sudpfanne_Durchmesser','Sudpfanne_MaxFuellhoehe','KorrekturWasser','KorrekturFarbe','Verdampfungsziffer', 'Kosten', 'Sudhausausbeute') ";
      sql += "VALUES (";
      sql += "'"+item->text().replace("'","''")+"',";
      sql += QString::number(item->getID())+",";
      sql += QString::number(item->getMaischebottich_Hoehe())+",";
      sql += QString::number(item->getMaischebottich_Durchmesser())+",";
      sql += QString::number(item->getMaischebottich_MaxFuellhoehe())+",";
      sql += QString::number(item->getSudpfanne_Hoehe())+",";
      sql += QString::number(item->getSudpfanne_Durchmesser())+",";
      sql += QString::number(item->getSudpfanne_MaxFuellhoehe())+",";
      sql += QString::number(item->getKorrekturWasser())+",";
      sql += QString::number(item->getKorrekturFarbe())+",";
      sql += QString::number(item->getVerdampfungsziffer())+",";
      sql += QString::number(item->getKosten())+",";
      sql += QString::number(item->getSudhausausbeute())+")";
      if (!query.exec(sql)) {
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + QObject::trUtf8("\nSQL Befehl:\n") + sql);
      }
    }
    AenderungAusruestung = false;
  }
}


void MainWindowImpl::DatenEinlesenDB()
{
  LeseAusruestungDB();
  LeseRohstoffeDB();
}


void MainWindowImpl::DatenSchreibenDB()
{
  SchreibeAusruestungDB();
  SchreibeGeraetelisteDB();
  SchreibeRohstoffeDB();
}

void MainWindowImpl::MalzNeueZeile(const QString& name, double ebc, double schuettung, double menge, double preis,
                                      const QString& bemerkung, const QString& anwendung,
                                      const QDate& einlagerung, const QDate& haltbar, const QString& link)
{
    int i = tableWidget_Malz->rowCount();
    tableWidget_Malz->setRowCount(i + 1);

    // Beschreibung
    tableWidget_Malz->setItem(i, TableMalzColName, new QTableWidgetItem(name));

    // Farbe
    MyDoubleSpinBox *spinBoxFarbe = new MyDoubleSpinBox();
    spinBoxFarbe->setAlignment(Qt::AlignHCenter);
    spinBoxFarbe->setMinimum(0);
    spinBoxFarbe->setMaximum(2000);
    spinBoxFarbe->setDecimals(1);
    spinBoxFarbe->setValue(ebc);
    tableWidget_Malz->setCellWidget(i, TableMalzColFarbe, spinBoxFarbe);
    tableWidget_Malz->setItem(i, TableMalzColFarbe, new MyTableWidgetItemNumeric(spinBoxFarbe->value()));
    spinBoxFarbe->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Malz->item(i, TableMalzColFarbe))));
    connect(spinBoxFarbe, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));
    connect(spinBoxFarbe, SIGNAL(valueChanged(double)), this, SLOT(slot_RohstoffFarbeValueChanged(double)));

    // maximaler Schuettungsanteil
    MyDoubleSpinBox *spinBoxMaxSchuettung = new MyDoubleSpinBox();
    spinBoxMaxSchuettung->setAlignment(Qt::AlignHCenter);
    spinBoxMaxSchuettung->setMinimum(0);
    spinBoxMaxSchuettung->setMaximum(100);
    spinBoxMaxSchuettung->setDecimals(0);
    spinBoxMaxSchuettung->setValue(schuettung);
    tableWidget_Malz->setCellWidget(i, TableMalzColMaxSchuettung, spinBoxMaxSchuettung);
    tableWidget_Malz->setItem(i, TableMalzColMaxSchuettung, new MyTableWidgetItemNumeric(spinBoxMaxSchuettung->value()));
    spinBoxMaxSchuettung->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Malz->item(i, TableMalzColMaxSchuettung))));
    connect(spinBoxMaxSchuettung, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));

    // Menge
    MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
    spinBoxMenge->setAlignment(Qt::AlignHCenter);
    spinBoxMenge->setMinimum(0);
    spinBoxMenge->setMaximum(1000);
    spinBoxMenge->setDecimals(3);
    spinBoxMenge->setSingleStep(0.1);
    spinBoxMenge->setValue(menge);
    tableWidget_Malz->setCellWidget(i, TableMalzColMenge, spinBoxMenge);
    tableWidget_Malz->setItem(i, TableMalzColMenge, new MyTableWidgetItemNumeric(spinBoxMenge->value()));
    spinBoxMenge->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Malz->item(i, TableMalzColMenge))));
    connect(spinBoxMenge, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));
    connect(spinBoxMenge, SIGNAL(valueChanged(double)), this, SLOT(slot_RohstoffMengeValueChanged(double)));

    // Preis
    MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
    spinBoxPreis->setAlignment(Qt::AlignHCenter);
    spinBoxPreis->setMinimum(0);
    spinBoxPreis->setMaximum(1000);
    spinBoxPreis->setDecimals(2);
    spinBoxPreis->setSingleStep(0.1);
    spinBoxPreis->setValue(preis);
    tableWidget_Malz->setCellWidget(i, TableMalzColPreis, spinBoxPreis);
    tableWidget_Malz->setItem(i, TableMalzColPreis, new MyTableWidgetItemNumeric(spinBoxPreis->value()));
    spinBoxPreis->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Malz->item(i, TableMalzColPreis))));
    connect(spinBoxPreis, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));

    // Bemerkung
    tableWidget_Malz->setItem(i, TableMalzColBemerkung, new QTableWidgetItem(bemerkung));

    // Anwendung
    tableWidget_Malz->setItem(i, TableMalzColAnwendung, new QTableWidgetItem(anwendung));

    // Datum Eingelagert
    QDateEdit * deEinlagerung = new QDateEdit(einlagerung);
    deEinlagerung->setAlignment(Qt::AlignHCenter);
    deEinlagerung->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));
    deEinlagerung->setCalendarPopup(true);
    connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
    tableWidget_Malz->setCellWidget(i, TableMalzColEinlagerung, deEinlagerung);
    tableWidget_Malz->setItem(i, TableMalzColEinlagerung, new MyTableWidgetItemNumeric(deEinlagerung->date().toJulianDay()));
    deEinlagerung->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Malz->item(i, TableMalzColEinlagerung))));
    connect(deEinlagerung, SIGNAL(dateChanged(const QDate&)), this, SLOT(slot_tableSyncValueChanged(const QDate&)));

    // Mindesthaltbarkeitsdatum
    QDateEdit * deMhd = new QDateEdit(haltbar);
    deMhd->setAlignment(Qt::AlignHCenter);
    deMhd->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));
    deMhd->setCalendarPopup(true);
    connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
    tableWidget_Malz->setCellWidget(i, TableMalzColMindesthalbat, deMhd);
    tableWidget_Malz->setItem(i, TableMalzColMindesthalbat, new MyTableWidgetItemNumeric(deMhd->date().toJulianDay()));
    deMhd->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Malz->item(i, TableMalzColMindesthalbat))));
    connect(deMhd, SIGNAL(dateChanged(const QDate&)), this, SLOT(slot_tableSyncValueChanged(const QDate&)));
    connect(deMhd, SIGNAL(dateChanged(const QDate&)), this, SLOT(slot_RohstoffHaltbarValueChanged(const QDate&)));

    // Link
    QTableWidgetItem *newItemLink = new QTableWidgetItem(link);
    newItemLink->setTextColor(Qt::blue);
    tableWidget_Malz->setItem(i, TableMalzColLink, newItemLink);

    // update
    emit deMhd->dateChanged(deMhd->date());
    emit spinBoxFarbe->valueChanged(spinBoxFarbe->value());
    emit spinBoxMenge->valueChanged(spinBoxMenge->value());
}

void MainWindowImpl::on_pushButton_MalzNeuVorlage_clicked()
{
    GetRohstoffVorlage* grvDia = new GetRohstoffVorlage(this);
    grvDia->ViewMalzauswahl();
    if (grvDia->exec() == QDialog::Accepted)
    {
        tableWidget_Malz->setSortingEnabled(false);
        //Bezeichnungsmerker löschen da sonst Einträge in den Suden geändert werden
        QString s = Malz_Bezeichnung_Merker;
        Malz_Bezeichnung_Merker = "";
        MalzNeueZeile(grvDia->m_Beschreibung, grvDia->m_Farbe, grvDia->m_MaxProzent,
            0.0, 0.0, "", grvDia->m_Eigenschaften,
            QDate::currentDate(), QDate::currentDate().addMonths(1), "");
        setAenderung(true);
        AenderungRohstofftabelle = true;
        Malz_Bezeichnung_Merker = s;
        tableWidget_Malz->setSortingEnabled(true);
    }
    delete grvDia;
}

void MainWindowImpl::HopfenNeueZeile(const QString& name, double alpha, double menge, double preis, bool pellets,
                                     const QString& bemerkung, int typ, const QString& eigenschaft,
                                     const QDate& einlagerung, const QDate& haltbar, const QString& link)
{
    int i = tableWidget_Hopfen->rowCount();
    tableWidget_Hopfen->setRowCount(i+1);

    // Beschreibung
    tableWidget_Hopfen->setItem(i, TableHopfenColName, new QTableWidgetItem(name));

    // Alpha
    MyDoubleSpinBox *spinBoxAlpha = new MyDoubleSpinBox();
    spinBoxAlpha->setAlignment(Qt::AlignHCenter);
    spinBoxAlpha->setMinimum(0);
    spinBoxAlpha->setMaximum(100);
    spinBoxAlpha->setDecimals(1);
    spinBoxAlpha->setSingleStep(0.1);
    spinBoxAlpha->setValue(alpha);
    tableWidget_Hopfen->setCellWidget(i, TableHopfenColAlpha, spinBoxAlpha);
    tableWidget_Hopfen->setItem(i, TableHopfenColAlpha, new MyTableWidgetItemNumeric(spinBoxAlpha->value()));
    spinBoxAlpha->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hopfen->item(i, TableHopfenColAlpha))));
    connect(spinBoxAlpha, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));

    // Menge
    MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
    spinBoxMenge->setAlignment(Qt::AlignHCenter);
    spinBoxMenge->setMinimum(0);
    spinBoxMenge->setMaximum(999999);
    spinBoxMenge->setDecimals(0);
    spinBoxMenge->setSingleStep(1);
    spinBoxMenge->setValue(menge);
    tableWidget_Hopfen->setCellWidget(i, TableHopfenColMenge, spinBoxMenge);
    tableWidget_Hopfen->setItem(i, TableHopfenColMenge, new MyTableWidgetItemNumeric(spinBoxMenge->value()));
    spinBoxMenge->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hopfen->item(i, TableHopfenColMenge))));
    connect(spinBoxMenge, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));
    connect(spinBoxMenge, SIGNAL(valueChanged(double)), this, SLOT(slot_RohstoffMengeValueChanged(double)));

    // Preis
    MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
    spinBoxPreis->setAlignment(Qt::AlignHCenter);
    spinBoxPreis->setMinimum(0);
    spinBoxPreis->setMaximum(999);
    spinBoxPreis->setDecimals(2);
    spinBoxPreis->setSingleStep(1);
    spinBoxPreis->setValue(preis);
    tableWidget_Hopfen->setCellWidget(i, TableHopfenColPreis, spinBoxPreis);
    tableWidget_Hopfen->setItem(i, TableHopfenColPreis, new MyTableWidgetItemNumeric(spinBoxPreis->value()));
    spinBoxPreis->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hopfen->item(i, TableHopfenColPreis))));
    connect(spinBoxPreis, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));

    // Pellets
    QTableWidgetItem *newItem4 = new QTableWidgetItem(trUtf8("Pellets") + (pellets ? " " : ""));
    newItem4->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    newItem4->setCheckState(pellets ? Qt::Checked : Qt::Unchecked);
    newItem4->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    tableWidget_Hopfen->setItem(i, TableHopfenColPellets, newItem4);

    // Bemerkung
    tableWidget_Hopfen->setItem(i, TableHopfenColBemerkung, new QTableWidgetItem(bemerkung));

    // Typ
    MyComboBox *comboBoxTyp = new MyComboBox();
    comboBoxTyp->addItem("");
    comboBoxTyp->addItem(trUtf8("Aroma"));
    comboBoxTyp->addItem(trUtf8("Bitter"));
    comboBoxTyp->addItem(trUtf8("Universal"));
    comboBoxTyp->setCurrentIndex(typ);
    tableWidget_Hopfen->setCellWidget(i, TableHopfenColTyp, comboBoxTyp);
    tableWidget_Hopfen->setItem(i, TableHopfenColTyp, new QTableWidgetItem(comboBoxTyp->currentText()));
    comboBoxTyp->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hopfen->item(i, TableHopfenColTyp))));
    connect(comboBoxTyp, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slot_tableSyncValueChanged(const QString &)));

    // Eigenschaften
    tableWidget_Hopfen->setItem(i, TableHopfenColEigenschaften, new QTableWidgetItem(eigenschaft));

    // Datum Eingelagert
    QDateEdit * deEinlagerung = new QDateEdit(einlagerung);
    deEinlagerung->setAlignment(Qt::AlignHCenter);
    deEinlagerung->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));
    deEinlagerung->setCalendarPopup(true);
    connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
    tableWidget_Hopfen->setCellWidget(i, TableHopfenColEinlagerung, deEinlagerung);
    tableWidget_Hopfen->setItem(i, TableHopfenColEinlagerung, new MyTableWidgetItemNumeric(deEinlagerung->date().toJulianDay()));
    deEinlagerung->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hopfen->item(i, TableHopfenColEinlagerung))));
    connect(deEinlagerung, SIGNAL(dateChanged(const QDate&)), this, SLOT(slot_tableSyncValueChanged(const QDate&)));

    // Mindesthaltbarkeitsdatum
    QDateEdit * deMhd = new QDateEdit(haltbar);
    deMhd->setAlignment(Qt::AlignHCenter);
    deMhd->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));
    deMhd->setCalendarPopup(true);
    connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
    tableWidget_Hopfen->setCellWidget(i, TableHopfenColMindesthalbat, deMhd);
    tableWidget_Hopfen->setItem(i, TableHopfenColMindesthalbat, new MyTableWidgetItemNumeric(deMhd->date().toJulianDay()));
    deMhd->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hopfen->item(i, TableHopfenColMindesthalbat))));
    connect(deMhd, SIGNAL(dateChanged(const QDate&)), this, SLOT(slot_tableSyncValueChanged(const QDate&)));
    connect(deMhd, SIGNAL(dateChanged(const QDate&)), this, SLOT(slot_RohstoffHaltbarValueChanged(const QDate&)));

    // Link
    QTableWidgetItem *newItemLink = new QTableWidgetItem(link);
    newItemLink->setTextColor(Qt::blue);
    tableWidget_Hopfen -> setItem(i, TableHopfenColLink, newItemLink);

    // update
    emit deMhd->dateChanged(deMhd->date());
    emit spinBoxMenge->valueChanged(spinBoxMenge->value());
}

void MainWindowImpl::on_pushButton_HopfenNeuVorlage_clicked()
{
    GetRohstoffVorlage* grvDia = new GetRohstoffVorlage(this);
    grvDia->ViewHopfenauswahl();
    if (grvDia->exec() == QDialog::Accepted)
    {
        tableWidget_Hopfen->setSortingEnabled(false);
        //Bezeichnungsmerker löschen da sonst Einträge in den Suden geändert werden
        QString s = Hopfen_Bezeichnung_Merker;
        Hopfen_Bezeichnung_Merker = "";
        HopfenNeueZeile(grvDia->m_Beschreibung, grvDia->m_Alpha, 0.0,
            0.0, true, "", grvDia->m_Typ, grvDia->m_Eigenschaften,
            QDate::currentDate(), QDate::currentDate().addMonths(1), "");
        setAenderung(true);
        AenderungRohstofftabelle = true;
        Hopfen_Bezeichnung_Merker = s;
        tableWidget_Hopfen->setSortingEnabled(true);
    }
    delete grvDia;
}

void MainWindowImpl::HefeNeueZeile(const QString& name, double menge, double wuerzemenge, double preis,
                      const QString& bemerkung, const QString& verpackung, int typ1, int typ2,
                      const QString& temperatur, const QString& eigenschaft,
                      int sedimentation, const QString& evg,
                      const QDate& einlagerung, const QDate& haltbar, const QString& link)
{
    int i = tableWidget_Hefe->rowCount();
    tableWidget_Hefe->setRowCount(i+1);

    // Beschreibung
    tableWidget_Hefe->setItem(i, TableHefeColName, new QTableWidgetItem(name));

    // Menge
    MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
    spinBoxMenge->setAlignment(Qt::AlignHCenter);
    spinBoxMenge->setMinimum(0);
    spinBoxMenge->setMaximum(999999);
    spinBoxMenge->setDecimals(0);
    spinBoxMenge->setSingleStep(1);
    spinBoxMenge->setValue(menge);
    tableWidget_Hefe->setCellWidget(i, TableHefeColMenge, spinBoxMenge);
    tableWidget_Hefe->setItem(i, TableHefeColMenge, new MyTableWidgetItemNumeric(spinBoxMenge->value()));
    spinBoxMenge->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hefe->item(i, TableHefeColMenge))));
    connect(spinBoxMenge, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));
    connect(spinBoxMenge, SIGNAL(valueChanged(double)), this, SLOT(slot_RohstoffMengeValueChanged(double)));

    // Wuerzemenge
    MyDoubleSpinBox *spinBoxWuerzemenge = new MyDoubleSpinBox();
    spinBoxWuerzemenge->setAlignment(Qt::AlignHCenter);
    spinBoxWuerzemenge->setMinimum(0);
    spinBoxWuerzemenge->setMaximum(999999);
    spinBoxWuerzemenge->setDecimals(0);
    spinBoxWuerzemenge->setSingleStep(1);
    spinBoxWuerzemenge->setValue(wuerzemenge);
    tableWidget_Hefe -> setCellWidget(i, TableHefeColWuerzemenge, spinBoxWuerzemenge);
    tableWidget_Hefe -> setItem(i, TableHefeColWuerzemenge, new MyTableWidgetItemNumeric(spinBoxWuerzemenge->value()));
    spinBoxWuerzemenge->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hefe->item(i, TableHefeColWuerzemenge))));
    connect(spinBoxWuerzemenge, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));
    connect(spinBoxWuerzemenge, SIGNAL(valueChanged(double)), this, SLOT(slot_RohstoffNonZeroValueChanged(double)));

    // Preis
    MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
    spinBoxPreis->setAlignment(Qt::AlignHCenter);
    spinBoxPreis->setMinimum(0);
    spinBoxPreis->setMaximum(999);
    spinBoxPreis->setDecimals(2);
    spinBoxPreis->setSingleStep(0.1);
    spinBoxPreis->setValue(preis);
    tableWidget_Hefe -> setCellWidget(i, TableHefeColPreis, spinBoxPreis);
    tableWidget_Hefe -> setItem(i, TableHefeColPreis, new MyTableWidgetItemNumeric(spinBoxPreis->value()));
    spinBoxPreis->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hefe->item(i, TableHefeColPreis))));
    connect(spinBoxPreis, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));

    // Bemerkung
    tableWidget_Hefe->setItem(i, TableHefeColBemerkung, new QTableWidgetItem(bemerkung));

    // Verpackungsmenge
    tableWidget_Hefe->setItem(i, TableHefeColVerpackungsmenge, new QTableWidgetItem(verpackung));

    // Typ obergaerig untergaerig
    MyComboBox *comboBoxTypOGUG = new MyComboBox();
    comboBoxTypOGUG->addItem("");
    comboBoxTypOGUG->addItem(trUtf8("OG"));
    comboBoxTypOGUG->addItem(trUtf8("UG"));
    comboBoxTypOGUG->setCurrentIndex(typ1);
    tableWidget_Hefe->setCellWidget(i, TableHefeColTypUgOg, comboBoxTypOGUG);
    tableWidget_Hefe->setItem(i, TableHefeColTypUgOg, new QTableWidgetItem(comboBoxTypOGUG->currentText()));
    comboBoxTypOGUG->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hefe->item(i, TableHefeColTypUgOg))));
    connect(comboBoxTypOGUG, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slot_tableSyncValueChanged(const QString &)));

    // Typ fluessig trocken
    MyComboBox *comboBoxTypTrFl = new MyComboBox();
    comboBoxTypTrFl->addItem("");
    comboBoxTypTrFl->addItem(trUtf8("trocken"));
    comboBoxTypTrFl->addItem(trUtf8("flüssig"));
    comboBoxTypTrFl->setCurrentIndex(typ2);
    tableWidget_Hefe->setCellWidget(i, TableHefeColTypTrFlg, comboBoxTypTrFl);
    tableWidget_Hefe->setItem(i, TableHefeColTypTrFlg, new QTableWidgetItem(comboBoxTypTrFl->currentText()));
    comboBoxTypTrFl->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hefe->item(i, TableHefeColTypTrFlg))));
    connect(comboBoxTypTrFl, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slot_tableSyncValueChanged(const QString &)));

    // Temperaturbereich
    tableWidget_Hefe->setItem(i, TableHefeColTemperatur, new QTableWidgetItem(temperatur));

    // Eigenschaften
    tableWidget_Hefe->setItem(i, TableHefeColEigenschaften, new QTableWidgetItem(eigenschaft));

    // Sedimentation
    MyComboBox *comboBoxSED = new MyComboBox();
    comboBoxSED->addItem("");
    comboBoxSED->addItem(trUtf8("hoch"));
    comboBoxSED->addItem(trUtf8("mittel"));
    comboBoxSED->addItem(trUtf8("niedrig"));
    comboBoxSED->setCurrentIndex(sedimentation);
    tableWidget_Hefe->setCellWidget(i, TableHefeColSedimentation, comboBoxSED);
    tableWidget_Hefe->setItem(i, TableHefeColSedimentation, new QTableWidgetItem(comboBoxSED->currentText()));
    comboBoxSED->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hefe->item(i, TableHefeColSedimentation))));
    connect(comboBoxSED, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slot_tableSyncValueChanged(const QString &)));

    // Endvergaerungsgrad
    tableWidget_Hefe->setItem(i, TableHefeColEVG, new QTableWidgetItem(evg));

    // Datum Eingelagert
    QDateEdit * deEinlagerung = new QDateEdit(einlagerung);
    deEinlagerung->setAlignment(Qt::AlignHCenter);
    deEinlagerung->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));
    deEinlagerung->setCalendarPopup(true);
    connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
    tableWidget_Hefe->setCellWidget(i, TableHefeColEinlagerung, deEinlagerung);
    tableWidget_Hefe->setItem(i, TableHefeColEinlagerung, new MyTableWidgetItemNumeric(deEinlagerung->date().toJulianDay()));
    deEinlagerung->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hefe->item(i, TableHefeColEinlagerung))));
    connect(deEinlagerung, SIGNAL(dateChanged(const QDate&)), this, SLOT(slot_tableSyncValueChanged(const QDate&)));

    // Mindesthaltbarkeitsdatum
    QDateEdit * deMhd = new QDateEdit(haltbar);
    deMhd->setAlignment(Qt::AlignHCenter);
    deMhd->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));
    deMhd->setCalendarPopup(true);
    connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
    tableWidget_Hefe->setCellWidget(i, TableHefeColMindesthalbat, deMhd);
    tableWidget_Hefe->setItem(i, TableHefeColMindesthalbat, new MyTableWidgetItemNumeric(deMhd->date().toJulianDay()));
    deMhd->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_Hefe->item(i, TableHefeColMindesthalbat))));
    connect(deMhd, SIGNAL(dateChanged(const QDate&)), this, SLOT(slot_tableSyncValueChanged(const QDate&)));
    connect(deMhd, SIGNAL(dateChanged(const QDate&)), this, SLOT(slot_RohstoffHaltbarValueChanged(const QDate&)));

    // Link
    QTableWidgetItem *newItemLink = new QTableWidgetItem(link);
    newItemLink->setTextColor(Qt::blue);
    tableWidget_Hefe->setItem(i, TableHefeColLink, newItemLink);

    // update
    emit deMhd->dateChanged(deMhd->date());
    emit spinBoxWuerzemenge->valueChanged(spinBoxWuerzemenge->value());
    emit spinBoxMenge->valueChanged(spinBoxMenge->value());
}

void MainWindowImpl::on_pushButton_HefeNeuVorlage_clicked()
{
    GetRohstoffVorlage* grvDia = new GetRohstoffVorlage(this);
    grvDia->ViewHefeauswahl();
    if (grvDia->exec() == QDialog::Accepted)
    {
        tableWidget_Hefe->setSortingEnabled(false);
        //Bezeichnungsmerker löschen da sonst Einträge in den Suden geändert werden
        QString s = Hefe_Bezeichnung_Merker;
        Hefe_Bezeichnung_Merker = "";
        HefeNeueZeile(grvDia->m_Beschreibung, 0.0, grvDia->m_Wuerzemenge, 0.0,
            "", grvDia->m_Verpackungsmenge, grvDia->m_TypOGUG, grvDia->m_TypFlTr,
            grvDia->m_Temperatur, grvDia->m_Eigenschaften,
            grvDia->m_SED, grvDia->m_EVG,
            QDate::currentDate(), QDate::currentDate().addMonths(1), "");
        setAenderung(true);
        AenderungRohstofftabelle = true;
        Hefe_Bezeichnung_Merker = s;
        tableWidget_Hefe->setSortingEnabled(true);
    }
    delete grvDia;
}

void MainWindowImpl::WeitereZutatNeueZeile(const QString& name, double menge, int einheit, int typ,
                           double ausbeute, double ebc, double preis, const QString& bemerkung,
                           const QDate& einlagerung, const QDate& haltbar, const QString& link)
{
    int i = tableWidget_WeitereZutaten -> rowCount();
    tableWidget_WeitereZutaten -> setRowCount(i+1);

    // Beschreibung
    tableWidget_WeitereZutaten->setItem(i, TableWZutatColName, new QTableWidgetItem(name));

    // Menge
    MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
    spinBoxMenge->setAlignment(Qt::AlignHCenter);
    spinBoxMenge->setMinimum(0);
    spinBoxMenge->setMaximum(10000);
    spinBoxMenge->setSingleStep(0.1);
    spinBoxMenge->setValue(menge);
    tableWidget_WeitereZutaten->setCellWidget(i, TableWZutatColMenge, spinBoxMenge);
    tableWidget_WeitereZutaten->setItem(i, TableWZutatColMenge, new MyTableWidgetItemNumeric(spinBoxMenge->value()));
    spinBoxMenge->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_WeitereZutaten->item(i, TableWZutatColMenge))));
    connect(spinBoxMenge, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));
    connect(spinBoxMenge, SIGNAL(valueChanged(double)), this, SLOT(slot_RohstoffMengeValueChanged(double)));

    // Einheiten
    MyComboBox *comboBoxEinheiten = new MyComboBox();
    comboBoxEinheiten->addItem(trUtf8("kg"));
    comboBoxEinheiten->addItem(trUtf8("g"));
    comboBoxEinheiten->setCurrentIndex(einheit);
    tableWidget_WeitereZutaten->setCellWidget(i, TableWZutatColEinheit, comboBoxEinheiten);
    tableWidget_WeitereZutaten->setItem(i, TableWZutatColEinheit, new QTableWidgetItem(comboBoxEinheiten->currentText()));
    comboBoxEinheiten->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_WeitereZutaten->item(i, TableWZutatColEinheit))));
    connect(comboBoxEinheiten, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slot_tableSyncValueChanged(const QString &)));

    // Typ
    MyComboBox *comboBoxTyp = new MyComboBox();
    comboBoxTyp->addItem(trUtf8("Honig"));
    comboBoxTyp->addItem(trUtf8("Zucker"));
    comboBoxTyp->addItem(trUtf8("Gewürz"));
    comboBoxTyp->addItem(trUtf8("Frucht"));
    comboBoxTyp->addItem(trUtf8("Sonstiges"));
    comboBoxTyp->setCurrentIndex(typ);
    tableWidget_WeitereZutaten->setCellWidget(i, TableWZutatColTyp, comboBoxTyp);
    tableWidget_WeitereZutaten->setItem(i, TableWZutatColTyp, new QTableWidgetItem(comboBoxTyp->currentText()));
    comboBoxTyp->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_WeitereZutaten->item(i, TableWZutatColTyp))));
    connect(comboBoxTyp, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slot_tableSyncValueChanged(const QString &)));

    // Ausbeute
    MyDoubleSpinBox *spinBoxAusbeute = new MyDoubleSpinBox();
    spinBoxAusbeute->setAlignment(Qt::AlignHCenter);
    spinBoxAusbeute->setMinimum(0);
    spinBoxAusbeute->setMaximum(100);
    spinBoxAusbeute->setDecimals(0);
    spinBoxAusbeute->setValue(ausbeute);
    tableWidget_WeitereZutaten->setCellWidget(i, TableWZutatColAusbeute, spinBoxAusbeute);
    tableWidget_WeitereZutaten->setItem(i, TableWZutatColAusbeute, new MyTableWidgetItemNumeric(spinBoxAusbeute->value()));
    spinBoxAusbeute->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_WeitereZutaten->item(i, TableWZutatColAusbeute))));
    connect(spinBoxAusbeute, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));

    // Farbe
    MyDoubleSpinBox *spinBoxFarbe = new MyDoubleSpinBox();
    spinBoxFarbe->setAlignment(Qt::AlignHCenter);
    spinBoxFarbe->setMinimum(0);
    spinBoxFarbe->setMaximum(2000);
    spinBoxFarbe->setDecimals(1);
    spinBoxFarbe->setValue(ebc);
    tableWidget_WeitereZutaten->setCellWidget(i, TableWZutatColFarbe, spinBoxFarbe);
    tableWidget_WeitereZutaten->setItem(i, TableWZutatColFarbe, new MyTableWidgetItemNumeric(spinBoxFarbe->value()));
    spinBoxFarbe->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_WeitereZutaten->item(i, TableWZutatColFarbe))));
    connect(spinBoxFarbe, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));
    connect(spinBoxFarbe, SIGNAL(valueChanged(double)), this, SLOT(slot_RohstoffFarbeValueChanged(double)));

    // Preis
    MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
    spinBoxPreis->setAlignment(Qt::AlignHCenter);
    spinBoxPreis -> setMinimum(0);
    spinBoxPreis -> setMaximum(1000);
    spinBoxPreis->setSingleStep(0.1);
    spinBoxPreis->setValue(preis);
    tableWidget_WeitereZutaten->setCellWidget(i, TableWZutatColPreis, spinBoxPreis);
    tableWidget_WeitereZutaten->setItem(i, TableWZutatColPreis, new MyTableWidgetItemNumeric(spinBoxPreis->value()));
    spinBoxPreis->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_WeitereZutaten->item(i, TableWZutatColPreis))));
    connect(spinBoxPreis, SIGNAL(valueChanged(double)), this, SLOT(slot_tableSyncValueChanged(double)));

    // Bemerkung
    tableWidget_WeitereZutaten->setItem(i, TableWZutatColBemerkung, new QTableWidgetItem(bemerkung));

    // Datum Eingelagert
    QDateEdit * deEinlagerung = new QDateEdit(einlagerung);
    deEinlagerung->setAlignment(Qt::AlignHCenter);
    deEinlagerung->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));
    deEinlagerung->setCalendarPopup(true);
    connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
    tableWidget_WeitereZutaten->setCellWidget(i, TableWZutatColEinlagerung, deEinlagerung);
    tableWidget_WeitereZutaten->setItem(i, TableWZutatColEinlagerung, new MyTableWidgetItemNumeric(deEinlagerung->date().toJulianDay()));
    deEinlagerung->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_WeitereZutaten->item(i, TableWZutatColEinlagerung))));
    connect(deEinlagerung, SIGNAL(dateChanged(const QDate&)), this, SLOT(slot_tableSyncValueChanged(const QDate&)));

    // Mindesthaltbarkeitsdatum
    QDateEdit * deMhd = new QDateEdit(haltbar);
    deMhd->setAlignment(Qt::AlignHCenter);
    deMhd->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));
    deMhd->setCalendarPopup(true);
    connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
    tableWidget_WeitereZutaten->setCellWidget(i, TableWZutatColMindesthalbat, deMhd);
    tableWidget_WeitereZutaten->setItem(i, TableWZutatColMindesthalbat, new MyTableWidgetItemNumeric(deMhd->date().toJulianDay()));
    deMhd->setProperty("TableWidgetItem", QVariant::fromValue(static_cast<void*>(tableWidget_WeitereZutaten->item(i, TableWZutatColMindesthalbat))));
    connect(deMhd, SIGNAL(dateChanged(const QDate&)), this, SLOT(slot_tableSyncValueChanged(const QDate&)));
    connect(deMhd, SIGNAL(dateChanged(const QDate&)), this, SLOT(slot_RohstoffHaltbarValueChanged(const QDate&)));

    // Link
    QTableWidgetItem *newItemLink = new QTableWidgetItem(link);
    newItemLink->setTextColor(Qt::blue);
    tableWidget_WeitereZutaten->setItem(i, TableWZutatColLink, newItemLink);

    // update
    emit deMhd->dateChanged(deMhd->date());
    emit spinBoxFarbe->valueChanged(spinBoxFarbe->value());
    emit spinBoxMenge->valueChanged(spinBoxMenge->value());
}

void MainWindowImpl::on_pushButton_WeitereZutatenNeuVorlage_clicked()
{
    GetRohstoffVorlage* grvDia = new GetRohstoffVorlage(this);
    grvDia->ViewWeitereZutatenauswahl();
    if (grvDia->exec() == QDialog::Accepted)
    {
        tableWidget_WeitereZutaten->setSortingEnabled(false);
        //Bezeichnungsmerker löschen da sonst Einträge in den Suden geändert werden
        QString s = WZutaten_Bezeichnung_Merker;
        WZutaten_Bezeichnung_Merker = "";
        WeitereZutatNeueZeile(grvDia->m_Beschreibung, 0.0, -1, grvDia->m_Typ,
                              grvDia->m_Ausbeute, grvDia->m_Farbe, 0.0, "",
                              QDate::currentDate(), QDate::currentDate().addMonths(1), "");
        setAenderung(true);
        AenderungRohstofftabelle = true;
        WZutaten_Bezeichnung_Merker = s;
        tableWidget_WeitereZutaten->setSortingEnabled(true);
    }
    delete grvDia;
}

void MainWindowImpl::SchreibeRohstoffeDB()
{
  QSqlQuery query;

  //Malz
  //----------------------------------------------------------
  //Zuerst alle Einträge in der Tabelle löschen
  QString sql = "DELETE FROM Malz WHERE ID > 0;";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

  // Schreibe Tabelle Malz
  for (int i=0; i < tableWidget_Malz -> rowCount(); i++){
    QDoubleSpinBox* spinBoxFarbe=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,TableMalzColFarbe);
    QDoubleSpinBox* spinBoxMaxSchuettung=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,TableMalzColMaxSchuettung);
    QDoubleSpinBox* spinBoxMenge=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,TableMalzColMenge);
    QDoubleSpinBox* spinBoxPreis=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,TableMalzColPreis);
    QDateEdit* deEinlagerung=(QDateEdit*)tableWidget_Malz -> cellWidget(i,TableMalzColEinlagerung);
    QDateEdit* deMhd=(QDateEdit*)tableWidget_Malz -> cellWidget(i,TableMalzColMindesthalbat);
    sql = "INSERT INTO Malz(Beschreibung, Farbe, MaxProzent, Menge, Preis, Bemerkung, Anwendung, Eingelagert, Mindesthaltbar, Link) VALUES(\'" +
        tableWidget_Malz -> item(i,TableMalzColName) -> text().replace("'","''") +	"\'," +
        QString::number(spinBoxFarbe -> value()) + "," +
        QString::number(spinBoxMaxSchuettung -> value()) + "," +
        QString::number(spinBoxMenge -> value()) + "," +
        QString::number(spinBoxPreis -> value()) + ",\'" +
        tableWidget_Malz -> item(i,TableMalzColBemerkung) -> text().replace("'","''") +	"\',\'" +
        tableWidget_Malz -> item(i,TableMalzColAnwendung) -> text().replace("'","''") + "\','"+
        deEinlagerung -> date().toString(Qt::ISODate) + "','" +
        deMhd -> date().toString(Qt::ISODate) + "','" +
        tableWidget_Malz -> item(i,TableMalzColLink) -> text().replace("'","''") + "')";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
  }
  //Hopfen
  //----------------------------------------------------------
  //Zuerst alle Einträge in der Tabelle löschen
  sql = "DELETE FROM Hopfen WHERE ID > 0;";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

  // Schreibe Tabelle Hopfen
  for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
    bool b = tableWidget_Hopfen -> item(i,TableHopfenColPellets) -> checkState();
    QDoubleSpinBox* spinBoxAlpha=(QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,TableHopfenColAlpha);
    QDoubleSpinBox* spinBoxMenge=(QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,TableHopfenColMenge);
    QDoubleSpinBox* spinBoxPreis=(QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,TableHopfenColPreis);
    QComboBox* comboTyp=(QComboBox*)tableWidget_Hopfen -> cellWidget(i,TableHopfenColTyp);
    QDateEdit* deEinlagerung=(QDateEdit*)tableWidget_Hopfen -> cellWidget(i,TableHopfenColEinlagerung);
    QDateEdit* deMhd=(QDateEdit*)tableWidget_Hopfen -> cellWidget(i,TableHopfenColMindesthalbat);
    sql = "INSERT INTO Hopfen(Beschreibung, Alpha, Menge, Preis, Pellets, Bemerkung, Eigenschaften, Typ, Eingelagert, Mindesthaltbar, Link) VALUES(\'" +
        tableWidget_Hopfen -> item(i,TableHopfenColName) -> text().replace("'","''") +	"\'," +
        QString::number(spinBoxAlpha -> value()) + "," +
        QString::number(spinBoxMenge -> value()) + "," +
        QString::number(spinBoxPreis -> value()) + "," +
        QString::number(b) + ",\'" +
        tableWidget_Hopfen -> item(i,TableHopfenColBemerkung) -> text().replace("'","''") + "\',\'" +
        tableWidget_Hopfen -> item(i,TableHopfenColEigenschaften) -> text().replace("'","''") + "\'," +
        QString::number(comboTyp -> currentIndex()) + ",'" +
        deEinlagerung -> date().toString(Qt::ISODate) + "','" +
        deMhd -> date().toString(Qt::ISODate) + "','" +
        tableWidget_Hopfen -> item(i,TableHopfenColLink) -> text().replace("'","''") + "')";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
  }
  //Hefe
  //----------------------------------------------------------
  //Zuerst alle Einträge in der Tabelle löschen
  sql = "DELETE FROM Hefe WHERE ID > 0;";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

  // Schreibe Tabelle Hefe
  for (int i=0; i < tableWidget_Hefe -> rowCount(); i++){
    QDoubleSpinBox* spinBoxMenge=(QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,TableHefeColMenge);
    QDoubleSpinBox* spinBoxWuerzemenge=(QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,TableHefeColWuerzemenge);
    QDoubleSpinBox* spinBoxPreis=(QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,TableHefeColPreis);
    QComboBox* comboTypOGUG=(QComboBox*)tableWidget_Hefe -> cellWidget(i,TableHefeColTypUgOg);
    QComboBox* comboTypTrFl=(QComboBox*)tableWidget_Hefe -> cellWidget(i,TableHefeColTypTrFlg);
    QComboBox* comboSED=(QComboBox*)tableWidget_Hefe -> cellWidget(i,TableHefeColSedimentation);
    QDateEdit* deEinlagerung=(QDateEdit*)tableWidget_Hefe -> cellWidget(i,TableHefeColEinlagerung);
    QDateEdit* deMhd=(QDateEdit*)tableWidget_Hefe -> cellWidget(i,TableHefeColMindesthalbat);
    sql = "INSERT INTO Hefe(Beschreibung, Menge, Wuerzemenge, Preis, Bemerkung, Verpackungsmenge, TypOGUG, TypTrFl, Temperatur, Eigenschaften, SED, EVG, Eingelagert, Mindesthaltbar, Link) VALUES(\'" +
        tableWidget_Hefe -> item(i,TableHefeColName) -> text().replace("'","''") +	"\'," +
        QString::number(spinBoxMenge -> value()) + "," +
        QString::number(spinBoxWuerzemenge -> value()) + "," +
        QString::number(spinBoxPreis -> value()) + ",\'" +
        tableWidget_Hefe -> item(i,TableHefeColBemerkung) -> text().replace("'","''") + "\',\'" +
        tableWidget_Hefe -> item(i,TableHefeColVerpackungsmenge) -> text().replace("'","''") + "\'," +
        QString::number(comboTypOGUG -> currentIndex()) + "," +
        QString::number(comboTypTrFl -> currentIndex()) + ",\'" +
        tableWidget_Hefe -> item(i,TableHefeColTemperatur) -> text().replace("'","''") + "\',\'" +
        tableWidget_Hefe -> item(i,TableHefeColEigenschaften) -> text().replace("'","''") + "\'," +
        QString::number(comboSED -> currentIndex()) + ",\'" +
        tableWidget_Hefe -> item(i,TableHefeColEVG) -> text().replace("'","''") + "\','" +
        deEinlagerung -> date().toString(Qt::ISODate) + "','" +
        deMhd -> date().toString(Qt::ISODate) + "','" +
        tableWidget_Hefe -> item(i,TableHefeColLink) -> text().replace("'","''") + "')";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
  }


  // Weitere Zutaten
  //----------------------------------------------------------
  //Zuerst alle Einträge in der Tabelle löschen
  sql = "DELETE FROM WeitereZutaten WHERE ID > 0;";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

  //Schreibe Tabelle Weitere Zutaten
  for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
    QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColMenge);
    QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColEinheit);
    QComboBox* comboTyp=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColTyp);
    QDoubleSpinBox* dsbAusbeute=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColAusbeute);
    QDoubleSpinBox* dsbEBC=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColFarbe);
    QDoubleSpinBox* dsbPreis=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColPreis);
    QDateEdit* deEinlagerung=(QDateEdit*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColEinlagerung);
    QDateEdit* deMhd=(QDateEdit*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColMindesthalbat);
    sql = "INSERT INTO WeitereZutaten(Beschreibung, Menge, Einheiten, Typ, Ausbeute, EBC, Preis, Bemerkung, Eingelagert, Mindesthaltbar, Link) VALUES(\'" +
        tableWidget_WeitereZutaten -> item(i,TableWZutatColName) -> text().replace("'","''") +	"\'," +
        QString::number(dsbMenge -> value()) + "," +
        QString::number(comboEinheit -> currentIndex()) + "," +
        QString::number(comboTyp -> currentIndex()) + "," +
        QString::number(dsbAusbeute -> value()) + "," +
        QString::number(dsbEBC -> value()) + "," +
        QString::number(dsbPreis -> value()) + "," +
        "\'" + tableWidget_WeitereZutaten -> item(i,TableWZutatColBemerkung) -> text().replace("'","''") + "\','" +
        deEinlagerung -> date().toString(Qt::ISODate) + "','" +
        deMhd -> date().toString(Qt::ISODate) + "','" +
        tableWidget_WeitereZutaten -> item(i,TableWZutatColLink) -> text().replace("'","''") + "')";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
  }


  //Wasser

  //Calcium
  sql = "UPDATE 'Wasser' SET 'Calcium'=" + QString::number(SpinBox_wwCalcium_mg -> value());
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  //Magnesium
  sql = "UPDATE 'Wasser' SET 'Magnesium'=" + QString::number(SpinBox_wwMagnesium_mg -> value());
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  //Saeurekapazitaet
  sql = "UPDATE 'Wasser' SET 'Saeurekapazitaet'=" + QString::number(SpinBox_wwSaeurekapazitaet_mmol -> value());
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

}


void MainWindowImpl::LeseRohstoffeDB()
{
    QSqlQuery query;

    // Malz einlesen
    QString sql = "SELECT * FROM Malz ORDER BY Beschreibung ASC";
    if (!query.exec(sql))
    {
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage->showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
            CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
            + trUtf8("\nSQL Befehl:\n") + sql);
    }
    else
    {
        tableWidget_Malz->clearSelection();
        Malz_Bezeichnung_Merker = "";
        tableWidget_Malz->clearContents();
        tableWidget_Malz->setRowCount(0);
        while (query.next())
            MalzNeueZeile(query.value(query.record().indexOf("Beschreibung")).toString(),
              query.value(query.record().indexOf("Farbe")).toReal(),
              query.value(query.record().indexOf("MaxProzent")).toReal(),
              query.value(query.record().indexOf("Menge")).toReal(),
              query.value(query.record().indexOf("Preis")).toReal(),
              query.value(query.record().indexOf("Bemerkung")).toString(),
              query.value(query.record().indexOf("Anwendung")).toString(),
              QDate::fromString(query.value(query.record().indexOf("Eingelagert")).toString(),Qt::ISODate),
              QDate::fromString(query.value(query.record().indexOf("Mindesthaltbar")).toString(),Qt::ISODate),
              query.value(query.record().indexOf("Link")).toString());
    }

    // Hopfen Einlesen
    sql = "SELECT * FROM Hopfen ORDER BY Beschreibung ASC";
    if (!query.exec(sql))
    {
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage->showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
            CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
            + trUtf8("\nSQL Befehl:\n") + sql);
    }
    else
    {
        tableWidget_Hopfen->clearSelection();
        Hopfen_Bezeichnung_Merker = "";
        tableWidget_Hopfen->clearContents();
        tableWidget_Hopfen->setRowCount(0);
        while (query.next())
            HopfenNeueZeile(query.value(query.record().indexOf("Beschreibung")).toString(),
                query.value(query.record().indexOf("Alpha")).toReal(),
                query.value(query.record().indexOf("Menge")).toReal(),
                query.value(query.record().indexOf("Preis")).toReal(),
                query.value(query.record().indexOf("Pellets")).toBool(),
                query.value(query.record().indexOf("Bemerkung")).toString(),
                query.value(query.record().indexOf("Typ")).toInt(),
                query.value(query.record().indexOf("Eigenschaften")).toString(),
                QDate::fromString(query.value(query.record().indexOf("Eingelagert")).toString(),Qt::ISODate),
                QDate::fromString(query.value(query.record().indexOf("Mindesthaltbar")).toString(),Qt::ISODate),
                query.value(query.record().indexOf("Link")).toString());
    }

    // Hefe Einlesen
    sql = "SELECT * FROM Hefe ORDER BY Beschreibung ASC";
    if (!query.exec(sql))
    {
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage->showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
            CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
            + trUtf8("\nSQL Befehl:\n") + sql);
    }
    else
    {
        tableWidget_Hefe->clearSelection();
        Hefe_Bezeichnung_Merker = "";
        tableWidget_Hefe->clearContents();
        tableWidget_Hefe->setRowCount(0);
        while (query.next())
            HefeNeueZeile(query.value(query.record().indexOf("Beschreibung")).toString(),
                query.value(query.record().indexOf("Menge")).toReal(),
                query.value(query.record().indexOf("Wuerzemenge")).toReal(),
                query.value(query.record().indexOf("Preis")).toReal(),
                query.value(query.record().indexOf("Bemerkung")).toString(),
                query.value(query.record().indexOf("Verpackungsmenge")).toString(),
                query.value(query.record().indexOf("TypOGUG")).toInt(),
                query.value(query.record().indexOf("TypTrFl")).toInt(),
                query.value(query.record().indexOf("Temperatur")).toString(),
                query.value(query.record().indexOf("Eigenschaften")).toString(),
                query.value(query.record().indexOf("SED")).toInt(),
                query.value(query.record().indexOf("EVG")).toString(),
                QDate::fromString(query.value(query.record().indexOf("Eingelagert")).toString(),Qt::ISODate),
                QDate::fromString(query.value(query.record().indexOf("Mindesthaltbar")).toString(),Qt::ISODate),
                query.value(query.record().indexOf("Link")).toString());
    }

    // Weitere Zutaten einlesen
    sql = "SELECT * FROM WeitereZutaten ORDER BY Beschreibung ASC";
    if (!query.exec(sql))
    {
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage->showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
            CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
            + trUtf8("\nSQL Befehl:\n") + sql);
    }
    else
    {
        tableWidget_WeitereZutaten->clearSelection();
        WZutaten_Bezeichnung_Merker = "";
        tableWidget_WeitereZutaten->clearContents();
        tableWidget_WeitereZutaten->setRowCount(0);
        while (query.next())
            WeitereZutatNeueZeile(query.value(query.record().indexOf("Beschreibung")).toString(),
                query.value(query.record().indexOf("Menge")).toReal(),
                query.value(query.record().indexOf("Einheiten")).toInt(),
                query.value(query.record().indexOf("Typ")).toInt(),
                query.value(query.record().indexOf("Ausbeute")).toReal(),
                query.value(query.record().indexOf("EBC")).toReal(),
                query.value(query.record().indexOf("Preis")).toReal(),
                query.value(query.record().indexOf("Bemerkung")).toString(),
                QDate::fromString(query.value(query.record().indexOf("Eingelagert")).toString(),Qt::ISODate),
                QDate::fromString(query.value(query.record().indexOf("Mindesthaltbar")).toString(),Qt::ISODate),
                query.value(query.record().indexOf("Link")).toString());
    }


    // Wasserwerte einlesen
    sql = "SELECT * FROM Wasser";
    if (!query.exec(sql))
    {
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage->showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
            CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
            + trUtf8("\nSQL Befehl:\n") + sql);
    }
    else
    {
        query.first();
        SpinBox_wwCalcium_mg->setValue(query.value(query.record().indexOf("Calcium")).toDouble());
        SpinBox_wwMagnesium_mg->setValue(query.value(query.record().indexOf("Magnesium")).toDouble());
        SpinBox_wwSaeurekapazitaet_mmol->setValue(query.value(query.record().indexOf("Saeurekapazitaet")).toDouble());
    }
}

void MainWindowImpl::on_pushButton_MalzDel_clicked()
{
  //Überprüfen ob bei nicht gebrauten Suden der Rohstoff verwendet wird.

  //Rohstoffname
  QString del_name = tableWidget_Malz -> item(tableWidget_Malz -> currentRow(),TableMalzColName) -> text();

  bool ok=true;
  //Alle Sude Abfragen die noch nicht gebraut wurden
  QString sql = "SELECT * FROM Sud WHERE BierWurdeGebraut == 0";
  QSqlQuery query;
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    save();
    QString letzeAuswahl;
    QString sudid;
    while (query.next()){
      if (ok){
        QSqlQuery query2;
        //alle gefundenen Sude überprüfen

        int FeldNr = query.record().indexOf("ID");
        sudid = query.value(FeldNr).toString();
        QString sql2 = "SELECT * FROM Malzschuettung WHERE Name='"+del_name.replace("'","''")+"' AND SudID="+sudid;
        if (!query2.exec(sql2)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                      + trUtf8("\nSQL Befehl:\n") + sql2);
        }
        else {
          if ((ok) && (query2.first())){
            //Dialog mit Rohstoffauswahl zum Austauschen des zu löschenden Rohstoffes anzeigen
            RohstoffAustauschen raDia;

            //Text für Dialog erstellen
            QString text = trUtf8("Der Rohstoff <b>")+del_name+trUtf8("</b> den Du löschen frochtest, wird in dem noch nicht gebrauten Sud <b>");
            int FeldNr = query.record().indexOf("Sudname");
            text += query.value(FeldNr).toString()+"</b> verwendet.\n\nSoll dieser Eintrag mit der folgenden Auswahl ersetzt werden?";
            raDia.SetText(text);
            //Auswahl für Ersetzung füllen
            for (int i=0; i < tableWidget_Malz -> rowCount(); i++){
              if (del_name != tableWidget_Malz -> item(i,TableMalzColName) -> text()){
                raDia.addAuswahlEintrag(tableWidget_Malz -> item(i,TableMalzColName) -> text() );
              }
            }
            raDia.setAktAuswahl(letzeAuswahl);
            raDia.exec();
            ok = raDia.b_ok;
            letzeAuswahl = raDia.GetAktAuswahl();
            //Austauschen
            if (ok){
              QString sql2 = "UPDATE Malzschuettung SET Name='"+letzeAuswahl.replace("'","''")+"' WHERE Name='"
                  +del_name+"' AND SudID="+sudid;
              if (!query2.exec(sql2)) {
                // Fehlermeldung Datenbankabfrage
                ErrorMessage *errorMessage = new ErrorMessage();
                errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                            CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                            + trUtf8("\nSQL Befehl:\n") + sql2);
              }
              else {
                //Bit setzen das dieser Sud neu Berechnet werden muss die Berechneten wert nun nicht mehr stimmen.
                sql2 ="UPDATE Sud SET NeuBerechnen= 1 WHERE  ID= " + sudid;
                if (!query2.exec(sql2)) {
                  // Fehlermeldung Datenbankabfrage
                  ErrorMessage *errorMessage = new ErrorMessage();
                  errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                              CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                              + trUtf8("\nSQL Befehl:\n") + sql2);
                }
              }
            }
          }
        }
      }
    }
    LadeSudDB(false);
    ErstelleSudInfo();
  }

  if (ok){
    tableWidget_Malz -> removeRow(tableWidget_Malz -> currentRow());
    setAenderung(true);
    AenderungRohstofftabelle = true;
  }
}

void MainWindowImpl::on_pushButton_HopfenDel_clicked()
{
  //Überprüfen ob bei nicht gebrauten Suden der Rohstoff verwendet wird.

  //Rohstoffname
  QString del_name = tableWidget_Hopfen -> item(tableWidget_Hopfen -> currentRow(),TableHopfenColName) -> text();

  bool ok=true;
  //Alle Sude Abfragen die noch nicht gebraut wurden
  QString sql = "SELECT * FROM Sud WHERE BierWurdeGebraut == 0";
  QSqlQuery query;
  if (!query.exec(sql)) {
    //Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    QString letzeAuswahl;
    QString sudid;
    save();
    while (query.next()){
      if (ok){
        QSqlQuery query2;
        //alle gefundenen Sude überprüfen
        int FeldNr = query.record().indexOf("ID");
        sudid = query.value(FeldNr).toString();

        QString sql2 = "SELECT * FROM WeitereZutatenGaben WHERE Name='"+del_name.replace("'","''")+"' AND SudID="+sudid;
        if (!query2.exec(sql2)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                      + trUtf8("\nSQL Befehl:\n") + sql2);
        }
        else {
          if ((ok) && (query2.first())){
            //Dialog mit Rohstoffauswahl zum Austauschen des zu löschenden Rohstoffes anzeigen
            RohstoffAustauschen raDia;

            //Text für Dialog erstellen
            QString text = trUtf8("Der Rohstoff <b>")+del_name+trUtf8("</b> den Du löschen möchtest, wird in den Weiteren Zutaten in dem noch nicht gebrauten Sud <b>");
            int FeldNr = query.record().indexOf("Sudname");
            text += query.value(FeldNr).toString()+"</b> verwendet.\n\nSoll dieser Eintrag mit der folgenden Auswahl ersetzt werden?";
            raDia.SetText(text);
            //Auswahl für Ersetzung füllen
            for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
              if (del_name != tableWidget_Hopfen -> item(i,TableHopfenColName) -> text()){
                raDia.addAuswahlEintrag(tableWidget_Hopfen -> item(i,TableHopfenColName) -> text() );
              }
            }
            raDia.setAktAuswahl(letzeAuswahl);
            raDia.exec();
            ok = raDia.b_ok;
            letzeAuswahl = raDia.GetAktAuswahl();
            //Austauschen
            if (ok){
              QString sql2 = "UPDATE WeitereZutatenGaben SET Name='"+letzeAuswahl+"' WHERE Name='"
                  +del_name+"' AND SudID="+sudid;
              if (!query2.exec(sql2)) {
                // Fehlermeldung Datenbankabfrage
                ErrorMessage *errorMessage = new ErrorMessage();
                errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                            CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                            + trUtf8("\nSQL Befehl:\n") + sql);
              }
              else {
                //Bit setzen das dieser Sud neu Berechnet werden muss da die Berechneten wert nun nicht mehr stimmen.
                sql2 ="UPDATE Sud SET NeuBerechnen= 1 WHERE  ID= " + sudid;
                if (!query2.exec(sql2)) {
                  // Fehlermeldung Datenbankabfrage
                  ErrorMessage *errorMessage = new ErrorMessage();
                  errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                              CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                              + trUtf8("\nSQL Befehl:\n") + sql2);
                }
              }
            }
          }
        }

        sql2 = "SELECT * FROM Hopfengaben WHERE Name='"+del_name.replace("'","''")+"' AND SudID="+sudid;
        if (!query2.exec(sql2)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                      + trUtf8("\nSQL Befehl:\n") + sql2);
        }
        else {
          if ((ok) && (query2.first())){
            //Dialog mit Rohstoffauswahl zum Austauschen des zu löschenden Rohstoffes anzeigen
            RohstoffAustauschen raDia;

            //Text für Dialog erstellen
            QString text = trUtf8("Der Rohstoff <b>")+del_name+trUtf8("</b> den Du löschen möchtest, wird in dem noch nicht gebrauten Sud <b>");
            int FeldNr = query.record().indexOf("Sudname");
            text += query.value(FeldNr).toString()+"</b> verwendet.\n\nSoll dieser Eintrag mit der folgenden Auswahl ersetzt werden?";
            raDia.SetText(text);
            //Auswahl für Ersetzung füllen
            for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
              if (del_name != tableWidget_Hopfen -> item(i,TableHopfenColName) -> text()){
                raDia.addAuswahlEintrag(tableWidget_Hopfen -> item(i,TableHopfenColName) -> text() );
              }
            }
            raDia.setAktAuswahl(letzeAuswahl);
            raDia.exec();
            ok = raDia.b_ok;
            letzeAuswahl = raDia.GetAktAuswahl();
            //Austauschen
            if (ok){
              QString sql2 = "UPDATE Hopfengaben SET Name='"+letzeAuswahl.replace("'","''")+"' WHERE Name='"
                  +del_name+"' AND SudID="+sudid;
              if (!query2.exec(sql2)) {
                // Fehlermeldung Datenbankabfrage
                ErrorMessage *errorMessage = new ErrorMessage();
                errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                            CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                            + trUtf8("\nSQL Befehl:\n") + sql);
              }
              else {
                //Bit setzen das dieser Sud neu Berechnet werden muss da die Berechneten wert nun nicht mehr stimmen.
                sql2 ="UPDATE Sud SET NeuBerechnen= 1 WHERE  ID= " + sudid;
                if (!query2.exec(sql2)) {
                  // Fehlermeldung Datenbankabfrage
                  ErrorMessage *errorMessage = new ErrorMessage();
                  errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                              CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                              + trUtf8("\nSQL Befehl:\n") + sql2);
                }
              }
            }
          }
        }
      }
    }
    LadeSudDB(false);
    ErstelleSudInfo();
  }
  if (ok){
    tableWidget_Hopfen -> removeRow(tableWidget_Hopfen -> currentRow());
    setAenderung(true);
    AenderungRohstofftabelle = true;
  }
}

void MainWindowImpl::on_pushButton_HefeDel_clicked()
{
  //Überprüfen ob bei nicht gebrauten Suden der Rohstoff verwendet wird.

  //Rohstoffname
  QString del_name = tableWidget_Hefe -> item(tableWidget_Hefe -> currentRow(),TableHefeColName) -> text();

  bool ok=true;
  //Alle Sude Abfragen die noch nicht gebraut wurden
  QString sql = "SELECT * FROM Sud WHERE BierWurdeGebraut == 0";
  QSqlQuery query;
  if (!query.exec(sql)) {
    //Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    QString letzeAuswahl;
    QString sudid;
    save();
    while (query.next()){
      if (ok){
        QSqlQuery query2;
        //alle gefundenen Sude überprüfen

        int FeldNr = query.record().indexOf("ID");
        sudid = query.value(FeldNr).toString();

        FeldNr = query.record().indexOf("AuswahlHefe");
        if (del_name == query.value(FeldNr).toString()){
          //Dialog mit Rohstoffauswahl zum Austauschen des zu löschenden Rohstoffes anzeigen
          RohstoffAustauschen raDia;

          //Text für Dialog erstellen
          QString text = trUtf8("Der Rohstoff <b>")+del_name+trUtf8("</b> den Du löschen möchtest, wird in dem noch nicht gebrauten Sud <b>");
          int FeldNr = query.record().indexOf("Sudname");
          text += query.value(FeldNr).toString()+"</b> verwendet.\n\nSoll dieser Eintrag mit der folgenden Auswahl ersetzt werden?";
          raDia.SetText(text);
          //Auswahl für Ersetzung füllen
          for (int i=0; i < tableWidget_Hefe -> rowCount(); i++){
            if (del_name != tableWidget_Hefe -> item(i,TableHefeColName) -> text()){
              raDia.addAuswahlEintrag(tableWidget_Hefe -> item(i,TableHefeColName) -> text() );
            }
          }
          raDia.setAktAuswahl(letzeAuswahl);
          raDia.exec();
          ok = raDia.b_ok;
          letzeAuswahl = raDia.GetAktAuswahl();
          //Austauschen
          if (ok){
            QString sql2 ="UPDATE Sud SET AuswahlHefe= '"+letzeAuswahl+"' WHERE  ID= " + sudid;
            if (!query2.exec(sql2)) {
              // Fehlermeldung Datenbankabfrage
              ErrorMessage *errorMessage = new ErrorMessage();
              errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                          CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                          + trUtf8("\nSQL Befehl:\n") + sql2);
            }
            else {
              //Bit setzen das dieser Sud neu Berechnet werden muss da die Berechneten wert nun nicht mehr stimmen.
              QString sql2 ="UPDATE Sud SET NeuBerechnen= 1 WHERE  ID= " + sudid;
              if (!query2.exec(sql2)) {
                // Fehlermeldung Datenbankabfrage
                ErrorMessage *errorMessage = new ErrorMessage();
                errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                            CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                            + trUtf8("\nSQL Befehl:\n") + sql2);
              }
            }
          }
        }
      }
    }
    LadeSudDB(false);
    ErstelleSudInfo();
  }
  if (ok){
    tableWidget_Hefe -> removeRow(tableWidget_Hefe -> currentRow());
    setAenderung(true);
    AenderungRohstofftabelle = true;
  }

}

void MainWindowImpl::SchreibeKonfig()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  //Position und Abmessung des Fensters speichern
  settings.beginGroup("MainWindow");
  settings.setValue("geometry", saveGeometry());
  for (int i = 0; i < tableWidget_Sudauswahl->columnCount(); ++i)
    settings.setValue("tableWidget_Sudauswahl_col" + QString::number(i), tableWidget_Sudauswahl->horizontalHeader()->sectionSize(i));
  for (int i = 0; i < tableWidget_Brauuebersicht->columnCount(); ++i)
    settings.setValue("tableWidget_Brauuebersicht_col" + QString::number(i), tableWidget_Brauuebersicht->horizontalHeader()->sectionSize(i));
  for (int i = 0; i < tableWidget_Malz->columnCount(); ++i)
    settings.setValue("tableWidget_Malz_col" + QString::number(i), tableWidget_Malz->horizontalHeader()->sectionSize(i));
  for (int i = 0; i < tableWidget_Hopfen->columnCount(); ++i)
    settings.setValue("tableWidget_Hopfen_col" + QString::number(i), tableWidget_Hopfen->horizontalHeader()->sectionSize(i));
  for (int i = 0; i < tableWidget_Hefe->columnCount(); ++i)
    settings.setValue("tableWidget_Hefe_col" + QString::number(i), tableWidget_Hefe->horizontalHeader()->sectionSize(i));
  for (int i = 0; i < tableWidget_WeitereZutaten->columnCount(); ++i)
    settings.setValue("tableWidget_WeitereZutaten_col" + QString::number(i), tableWidget_WeitereZutaten->horizontalHeader()->sectionSize(i));
  settings.endGroup();

  //Letzten Geladenen Datensatz merken
  settings.beginGroup("Datensatz");
  settings.setValue("AktuelleSudID", AktuelleSudID);
  settings.endGroup();

  //Einstellungen Brauübersicht
  settings.beginGroup("Brauuebersicht");
  settings.setValue("ZeitraumVon", dateEdit_AuswahlVon -> date());
  settings.setValue("ZeitraumBis", dateEdit_AuswahlBis -> date());
  settings.setValue("AuswahlLinie1", comboBox_AuswahlL1 -> currentIndex());
  settings.setValue("AuswahlLinie2", comboBox_AuswahlL2 -> currentIndex());
  settings.endGroup();

  //Einstellungen Sudauswahl
  settings.beginGroup("Sudauswahl");
  settings.setValue("FilterAlle", radioButton_FilterAlle -> isChecked());
  settings.setValue("FilterNichtGebraut", radioButton_FilterNichtGebraut -> isChecked());
  settings.setValue("FilterGebrautNichtAbgefuellt", radioButton_FilterGebrautNichtAbgefuellt -> isChecked());
  settings.setValue("FilterAbgefuellt", radioButton_Abgefuellt -> isChecked());
  settings.setValue("FilterMerkliste", radioButton_Merkliste -> isChecked());
  settings.endGroup();

  //Einstellungen Sonstiges
  settings.beginGroup("sonstiges");
  settings.setValue("MerklisteMengenEinbeziehen", checkBox_MerklisteMengen->checkState());
  settings.endGroup();

}


void MainWindowImpl::LeseKonfig()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  //Position und Abmessungen wiederherstellen
  settings.beginGroup("MainWindow");
  restoreGeometry(settings.value("geometry").toByteArray());
  settings.endGroup();

  //Letzten Geladenen Datensatz auslesen
  settings.beginGroup("Datensatz");
  AktuelleSudID = settings.value("AktuelleSudID").toInt();
  settings.endGroup();

  //Einstellungen Brauübersicht
  settings.beginGroup("Brauuebersicht");
  dateEdit_AuswahlVon -> setDate(settings.value("ZeitraumVon").toDate());
  dateEdit_AuswahlBis -> setDate(settings.value("ZeitraumBis").toDate());
  comboBox_AuswahlL1 -> setCurrentIndex(settings.value("AuswahlLinie1").toInt());
  comboBox_AuswahlL2 -> setCurrentIndex(settings.value("AuswahlLinie2").toInt());
  settings.endGroup();

  //Einstellungen Sudauswahl
  settings.beginGroup("Sudauswahl");
  radioButton_FilterAlle -> setChecked(settings.value("FilterAlle").toBool());
  radioButton_FilterNichtGebraut -> setChecked(settings.value("FilterNichtGebraut").toBool());
  radioButton_FilterGebrautNichtAbgefuellt -> setChecked(settings.value("FilterGebrautNichtAbgefuellt").toBool());
  radioButton_Abgefuellt -> setChecked(settings.value("FilterAbgefuellt").toBool());
  radioButton_Merkliste -> setChecked(settings.value("FilterMerkliste").toBool());
  settings.endGroup();

  //Letzten Geladenen Datensatz auslesen
  settings.beginGroup("Netz");
  QString str;
  str = settings.value("keinInternet").toString();
  if (str == "")
    settings.setValue("keinInternet",false);
  keinInternet = settings.value("keinInternet").toBool();
  settings.endGroup();

  //Einstellungen Sonstiges
  settings.beginGroup("sonstiges");
  checkBox_MerklisteMengen->setChecked(settings.value("MerklisteMengenEinbeziehen", false).toBool());
  settings.endGroup();
}


void MainWindowImpl::createActions()
{
  saveAct = new QAction("", this);
  saveAct->setShortcuts(QKeySequence::Save);
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

  exitAct = new QAction("", this);
  exitAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  for (int i = 0; i < MaxRecentFiles; ++i) {
    recentFileActs[i] = new QAction(this);
    recentFileActs[i] -> setVisible(false);
    connect(recentFileActs[i], SIGNAL(triggered()),	this, SLOT(openRecentFile()));
  }

  //Aktionen in Menü Extras
  einstellungen = new QAction("", this);
  einstellungen->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
  connect(einstellungen, SIGNAL(triggered()), this, SLOT(slot_einstellungen()));

  //Assistent zum übernehmen von einem rezept
  schuettungProzent = new QAction("", this);
  //einstellungen->setShortcuts(QKeySequence::Save);
  connect(schuettungProzent, SIGNAL(triggered()), this, SLOT(slot_schuettungProzent()));

  //Assistent zum berechnen de IBU Wertes eines Rezeptes
  berIBU = new QAction("", this);
  //einstellungen->setShortcuts(QKeySequence::Save);
  connect(berIBU, SIGNAL(triggered()), this, SLOT(slot_berIBU()));

  //Hebt die Eingabesperre von einen als gebraut/Abgefüllt markierten Sud auf
  EntsperreEingabefelder = new QAction("", this);
  connect(EntsperreEingabefelder, SIGNAL(triggered()), this, SLOT(slot_EntsperreEingabefelder()));

  //Setzt das Bit BierGebraut zurück
  ResetBierGebraut = new QAction("", this);
  connect(ResetBierGebraut, SIGNAL(triggered()), this, SLOT(slot_ResetBierWurdeGebraut()));

  //Setzt das Bit Abgefuellt zurück
  ResetAbgefuellt = new QAction("", this);
  connect(ResetAbgefuellt, SIGNAL(triggered()), this, SLOT(slot_ResetAbgefuellt()));

  //Setzt das Bit Abgefuellt zurück
  ResetVerbraucht = new QAction("", this);
  connect(ResetVerbraucht, SIGNAL(triggered()), this, SLOT(slot_ResetBierVerbraucht()));

  //Setzt den Zugabestatus der Weiteren Zutaten zurück
  ResetZugabestatus = new QAction("", this);
  connect(ResetZugabestatus, SIGNAL(triggered()), this, SLOT(slot_ResetWZZugabestatus()));
}

void MainWindowImpl::ErstelleSprachMenu()
{
  sprachMenu = menuBar()->addMenu("");
  QActionGroup* langGroup = new QActionGroup(sprachMenu);
  langGroup->setExclusive(true);

  connect(langGroup, SIGNAL(triggered(QAction *)), this, SLOT(slot_SpracheWechselt(QAction *)));

  // format systems language
  //
  //QString defaultLocale = QLocale::system().name();       // e.g. "de_DE"
  //defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"
  //srachauswahl aus configdatei
  //qDebug() << "defaultLocale" << defaultLocale;

  QString langPath = QApplication::applicationDirPath().append("/languages");
  QDir dir(langPath);
  QStringList fileNames = dir.entryList(QStringList("kb_*.qm"));

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  settings.beginGroup("Sprache");
  QString sprachauswahl = settings.value("sprachauswahl", QLocale::system().name()).toString();
  settings.endGroup();

  for (int i = 0; i < fileNames.size(); ++i)
  {
    // get locale extracted by filename
    QString locale;
    locale = fileNames[i];                  // "TranslationExample_de.qm"
    locale.truncate(locale.lastIndexOf('.'));   // "TranslationExample_de"
    locale.remove(0, locale.indexOf('_') + 1);   // "de"

    QString lang = QLocale::languageToString(QLocale(locale).language());
    QIcon ico(QString("%1/%2.png").arg(langPath).arg(locale));

    QAction *action = new QAction(ico, lang, this);
    action->setCheckable(true);
    action->setData(locale);

    sprachMenu->addAction(action);
    langGroup->addAction(action);

    // set default translators and language checked
    if (sprachauswahl == locale)
    {
      action->setChecked(true);
    }
  }
}

void MainWindowImpl::createMenus()
{
  //Menü geladener Sud
  geladenerSudMenu = menuBar()->addMenu("");
  geladenerSudMenu->addAction(saveAct);
  geladenerSudMenu->addSeparator();
  geladenerSudMenu->addAction(schuettungProzent);
  geladenerSudMenu->addAction(berIBU);
  geladenerSudMenu->addAction(EntsperreEingabefelder);
  geladenerSudMenu->addAction(ResetBierGebraut);
  geladenerSudMenu->addAction(ResetAbgefuellt);
  geladenerSudMenu->addAction(ResetVerbraucht);
  geladenerSudMenu->addAction(ResetZugabestatus);
  geladenerSudMenu->addSeparator();
  for (int i = 0; i < MaxRecentFiles; ++i)
    geladenerSudMenu->addAction(recentFileActs[i]);
  geladenerSudMenu->addSeparator();
  geladenerSudMenu->addAction(exitAct);

  //Menü Extras
  extrasMenu = menuBar()->addMenu("");
  extrasMenu->addAction(einstellungen);

  //Sprachauswahl Menü
  ErstelleSprachMenu();
}

void MainWindowImpl::retranslateMenus()
{
    geladenerSudMenu->setTitle(trUtf8("&Geladener Sud"));
    saveAct->setText(trUtf8("Speichern"));
    saveAct->setStatusTip(trUtf8("Speichere die aktuellen Suddaten"));
    exitAct->setText(trUtf8("Beenden"));
    schuettungProzent->setText(trUtf8("&Rezeptübernahme Schüttung"));
    schuettungProzent->setStatusTip(trUtf8("Öffnet einen Dialog zur unterstützung für die Übernahme der Schüttung"));
    berIBU->setText(trUtf8("&Rezeptübernahme Bittere"));
    berIBU -> setStatusTip(trUtf8("Öffnet einen Dialog zur Berechnung der Bittere"));
    EntsperreEingabefelder->setText(trUtf8("&Entsperre Eingabefelder"));
    EntsperreEingabefelder->setStatusTip(trUtf8("Hebt die Eingabesperre der Eingabefelder auf"));
    ResetBierGebraut->setText(trUtf8("\"Bier &gebraut\" zurücksetzten"));
    ResetBierGebraut->setStatusTip(trUtf8("Setzt das Bit Bier wurde Gebraut von dem aktuellen Sud in der Datenbank zurück"));
    ResetAbgefuellt->setText(trUtf8("\"Bier &abgefüllt\" zurücksetzten"));
    ResetAbgefuellt->setStatusTip(trUtf8("Setzt das Bit Abgefüllt von dem aktuellen Sud in der Datenbank zurück"));
    ResetVerbraucht->setText(trUtf8("\"Bier &verbraucht\" zurücksetzten"));
    ResetVerbraucht->setStatusTip(trUtf8("Setzt das Bit Bier Verbraucht von dem aktuellen Sud in der Datenbank zurück"));
    ResetZugabestatus->setText(trUtf8("&Reset Zugabestatus WZutaten"));
    ResetZugabestatus->setStatusTip(trUtf8("setzt den Zugabestatus der Weiteren Zutaten zurück"));

    extrasMenu->setTitle(trUtf8("&Extras"));
    einstellungen->setText(trUtf8("&Einstellungen"));
    einstellungen->setStatusTip(trUtf8("Öffnet einen Dialog mit Einstellungen"));

    sprachMenu->setTitle(trUtf8("&Sprache"));
}

void MainWindowImpl::save()
{
  //überprüfen ob verbindung zur datenbank noch besteht
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("DB");
  QString dbpfad = settings.value("DB_Pfad").toString() + "/" + DB_USER_NAME;
  settings.endGroup();

  if (QFile::exists(dbpfad)) {
    if (reconnect) {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
      db.setDatabaseName(dbpfad);
      reconnect = false;
    }
    QSqlDatabase::database().transaction();
    SchreibeSuddatenDB();
    DatenSchreibenDB();
    QSqlDatabase::database().commit();
    setAenderung(false);
    BerAlles();
    BerEffektiveAusbeuteMittel();
  }
  else {
    reconnect = true;
    QMessageBox msgBox;
    msgBox.setWindowTitle(trUtf8("Keine verbindung zur Datenbank"));
    msgBox.setText(trUtf8("Die Datenbank-datei: ") + dbpfad + trUtf8(" existiert nicht mehr!"));
    msgBox.setInformativeText(trUtf8("Wenn die Datenbank auf einem USB-Stick oder Netzlaufwerk liegt, stellen sie die verbindung wieder her und speichern sie anschliessend nocheinmal."));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setStandardButtons(QMessageBox::Ok);

    msgBox.exec();
  }
}


void MainWindowImpl::SchreibeSuddatenDB()
{
  QSqlQuery query;
  QString sql = "UPDATE Sud SET ";
  //Sudname
  sql += "Sudname='";
  sql += lineEdit_Sudname -> text().replace("'","''") + "', ";
  //Menge
  sql += "Menge='";
  sql += QString::number(spinBox_Menge -> value()) + "', ";
  //Stammwürze
  sql += "SW='";
  sql += QString::number(spinBox_SW -> value()) + "', ";
  //CO2 Gehalt
  sql += "CO2='";
  sql += QString::number(doubleSpinBox_CO2 -> value()) + "', ";
  //Bitterkeit
  sql += "IBU='";
  sql += QString::number(spinBox_IBU -> value()) + "', ";
  //Reifezeit
  sql += "Reifezeit='";
  sql += QString::number(spinBox_Reifezeit -> value()) + "', ";
  //BrauanlagenAuswahl
  int AuswahlBrauanlage = getBrauanlagenIDRezept();
  if (AuswahlBrauanlage != 0) {
    sql += "AuswahlBrauanlage='";
    sql += QString::number(AuswahlBrauanlage) + "', ";
  }
  //Bauanlagen Name
  sql += "AuswahlBrauanlageName='";
  sql += comboBox_AuswahlBrauanlage->currentText().replace("'","''") + "', ";
  //Kommentar
  sql += "Kommentar='";
  sql += textEdit_Kommentar -> document() -> toPlainText().replace("'","''") + "', ";
  //Braudatum
  sql += "Braudatum='";
  sql += dateEdit_Braudatum -> date().toString(Qt::ISODate) + "', ";
  //Flag ob Bier gebraut wurde
  sql += "BierWurdeGebraut='";
  sql += QString::number(BierWurdeGebraut) + "', ";
  //Flag ob Bier abgefüllt wurde
  sql += "BierWurdeAbgefuellt='";
  sql += QString::number(BierWurdeAbgefuellt) + "', ";
  //Flag ob Bier verbraucht wurde
  sql += "BierWurdeVerbraucht='";
  sql += QString::number(BierWurdeVerbraucht) + "', ";
  //Kochdauer Nach Bitterhopfengabe
  sql += "KochdauerNachBitterhopfung='";
  sql += QString::number(spinBox_Gesammtkochdauer -> value()) + "', ";
  //Temperatur Einmaischen
  sql += "EinmaischenTemp='";
  sql += QString::number(spinBox_EinmaischenTemp -> value()) + "', ";
  //Wasser
  sql += "FaktorHauptguss='";
  sql += QString::number(doubleSpinBox_FaktorHG -> value()) + "', ";
  //Auswahl Hefe
  sql += "AuswahlHefe='";
  sql += comboBox_AuswahlHefe -> currentText().replace("'","''") + "', ";
  //Anzahl Hefe einheiten
  sql += "HefeAnzahlEinheiten='";
  sql += QString::number(spinBox_AnzahlHefeEinheiten -> value()) + "', ";
  //Würzemenge nach dem Hopfenseihen
  sql += "WuerzemengeKochende='";
  sql += QString::number(spinBox_WuerzemengeKochende -> value()) + "', ";
  //Stammwürze nach dem Hopfenseihen
  sql += "SWKochende='";
  sql += QString::number(spinBox_SWKochende -> value()) + "', ";
  //Abgezwackte Speisemenge
  sql += "Speisemenge='";
  sql += QString::number(spinBox_Speisemenge -> value()) + "', ";
  //Datum der Hefezugabe
  sql += "Anstelldatum='";
  sql += dateEdit_Anstelldatum -> date().toString(Qt::ISODate) + "', ";
  //Würzemenge beim Anstellen
  sql += "WuerzemengeAnstellen='";
  sql += QString::number(spinBox_WuerzemengeAnstellen -> value()) + "', ";
  //Stammwürze beim Anstellen
  sql += "SWAnstellen='";
  sql += QString::number(spinBox_SWAnstellen -> value()) + "', ";
  //Abfülldatum
  sql += "Abfuelldatum='";
  sql += dateEdit_Abfuelldatum -> date().toString(Qt::ISODate) + "', ";
  //Stammwürze der Schnellgärprobe
  sql += "SWSchnellgaerprobe='";
  sql += QString::number(spinBox_SWSchnellgaerprobe -> value()) + "', ";
  //Stammwürze Jungbier
  sql += "SWJungbier='";
  sql += QString::number(spinBox_SWJungbier -> value()) + "', ";
  //Temperatur Jungbier beim Abfüllen
  sql += "TemperaturJungbier='";
  sql += QString::number(spinBox_TemperaturJungbier -> value()) + "', ";
  //Datum Gespeichert
  sql += "Gespeichert='";
  sql += QDateTime::currentDateTime().toString(Qt::ISODate) + "', ";
  //Welches Tab Aktiviert werden soll
  sql += "AktivTab='";
  sql += QString::number(tabWidged -> currentIndex()) + "', ";
  //Welches Tab Aktiviert werden soll
  sql += "AktivTab_Gaerverlauf='";
  sql += QString::number(toolBox_Gaerverlauf -> currentIndex()) + "', ";
  //Ergebniss Schüttung Gesammt
  sql += "erg_S_Gesammt='";
  sql += QString::number(doubleSpinBox_S_Gesammt -> value()) + "', ";
  //Ergebniss Wassermenge Gesammt
  sql += "erg_W_Gesammt='";
  sql += QString::number(doubleSpinBox_W_Gesammt -> value()) + "', ";
  //Ergebniss Wassermenge Hauptguss
  sql += "erg_WHauptguss='";
  sql += QString::number(doubleSpinBox_WHauptguss -> value()) + "', ";
  //Ergebniss Wassermenge Nachguss
  sql += "erg_WNachguss='";
  sql += QString::number(doubleSpinBox_WNachguss -> value()) + "', ";
  //Ergebniss Sudhausausbeute
  sql += "erg_Sudhausausbeute='";
  sql += QString::number(spinBox_Sudhausausbeute -> value()) + "', ";
  //Ergebniss Ausbeute Effektiv
  sql += "erg_EffektiveAusbeute='";
  sql += QString::number(spinBox_AusbeuteEffektiv2 -> value()) + "', ";
  //Ergebniss Farbe
  sql += "erg_Farbe='";
  sql += QString::number(doubleSpinBox_EBC -> value()) + "', ";
  //Ergebniss Preis
  sql += "erg_Preis='";
  sql += QString::number(spinBox_Preis -> value()) + "', ";
  //Ergebniss Alkohol
  sql += "erg_Alkohol='";
  sql += QString::number(spinBox_AlkoholVol -> value()) + "', ";
  //Kosten für Wasser und Strom
  sql += "KostenWasserStrom='";
  sql += QString::number(spinBox_Nebenkosten -> value()) + "', ";
  //Nachisomerisierungszeit
  sql += "Nachisomerisierungszeit='";
  sql += QString::number(spinBox_NachisomerisierungsZeit -> value()) + "', ";
  //Würzemenge vor dem Hopfenseihen
  sql += "WuerzemengeVorHopfenseihen='";
  sql += QString::number(spinBox_WuerzemengeVorHopfenseihen -> value()) + "', ";
  //Stammwürze vor dem Hopfenseihen
  sql += "SWVorHopfenseihen='";
  sql += QString::number(spinBox_SWVorHopfenseihen -> value()) + "', ";
  //Gewünschte Restalkalität
  sql += "RestalkalitaetSoll='";
  sql += QString::number(SpinBox_waSollRestalkalitaet_dh -> value()) + "', ";
  //Schnellgärprobe aktiv
  sql += "SchnellgaerprobeAktiv='";
  sql += QString::number(checkBox_SchnellgaerprobeAktiv -> isChecked()) + "', ";
  //Spunden
  sql += "Spunden='";
  sql += QString::number(checkBox_Spunden -> isChecked()) + "', ";
  //Jungbiermenge beim Abfüllen
  sql += "JungbiermengeAbfuellen='";
  sql += QString::number(spinBox_JungbiermengeAbfuellen -> value()) + "', ";
  //Ergebniss Abgefuellte Biermenge
  sql += "erg_AbgefuellteBiermenge='";
  sql += QString::number(spinBox_JungbiermengeAbfuellen -> value() + spinBox_SpeisemengeGesammt -> value()/1000) + "', ";
  //Art der Hopfenberechnung
  sql += "BerechnungsArtHopfen='";
  sql += QString::number(comboBox_BerechnungsArtHopfen->currentIndex()) + "', ";
  sql += "highGravityFaktor='";
  sql += QString::number(spinBox_High_Gravity->value()) + "', ";
  //Ausbeute Ignorieren aktiv
  sql += "AusbeuteIgnorieren='";
  sql += QString::number(checkBox_AusbeuteIgnorieren -> isChecked()) + "', ";
  //Die beste Bewertung Global abspeichern
  int bew = 0;
  QString bewtext = "";
  for (int i=0; i<list_Bewertung.count(); i++){
    int b = list_Bewertung[i]->getSterne();
    if (b > bew){
      bew = b;
      bewtext = QString::number(list_Bewertung[i]->getWoche())+". Woche ";
    }
  }

  //Neu Berechnen zurücksetzten
  sql += "NeuBerechnen='false', ";

  //Bewertung
  sql += "Bewertung='";
  sql += QString::number(bew) + "', ";
  //Bewertung
  sql += "BewertungMaxSterne='";
  sql += QString::number(MaxAnzahlSterne) + "', ";
  //Bewertungstext (Woche)
  sql += "BewertungText='";
  sql += bewtext + "' ";

  sql += "WHERE ID=" + QString::number(AktuelleSudID) + ";";
  //Abfrage Abschicken
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    setAenderung(false);

    //Schreibe Rasten
    SchreibeRastenDB();

    //Schuettung Schreiben
    SchreibeMalzschuettungDB();

    //Hopfengaben Schreiben
    SchreibeHopfengabenDB();

    //Erweiterte Zutaten Schreiben
    SchreibeErweiterteZutatenDB();

    //Tabelle Schnellgärverlauf speichern
    SchreibeSchnellgaerverlaufDB();
    SchreibeHauptgaerverlaufDB();
    SchreibeNachgaerverlaufDB();

    SchreibeBewertungenDB();
    SchreibeAnhangDB();

    FuelleSudauswahl();
  }
}

void MainWindowImpl::SchreibeBewertungenDB()
{
  //Alle Bewertimgem in diesem Sud aus der Datenbank löschen
  // und dann neu schreiben
  QSqlQuery query;
  QString sql = "DELETE FROM Bewertungen WHERE SudID =" + QString::number(AktuelleSudID);
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

  for (int i=0; i < list_Bewertung.count(); i++){
    sql = "INSERT INTO Bewertungen(SudID, Woche, Datum, Sterne, Bemerkung, Farbe,";
    sql += "FarbeBemerkung, Schaum, SchaumBemerkung, Geruch, GeruchBemerkung, Geschmack, GeschmackBemerkung,";
    sql += "Antrunk, AntrunkBemerkung, Haupttrunk, HaupttrunkBemerkung, Nachtrunk, NachtrunkBemerkung,";
    sql += "Gesamteindruck, GesamteindruckBemerkung) VALUES(" +
        QString::number(AktuelleSudID) +	"," +
        QString::number(list_Bewertung[i] -> getWoche()) +	",'" +
        list_Bewertung[i] -> getBewertungdatum().toString(Qt::ISODate) + "', " +
        QString::number(list_Bewertung[i] -> getSterne()) +	"," +
        "'" + list_Bewertung[i] -> getBemerkung().replace("'","''") +	"'," +
        QString::number(list_Bewertung[i] -> getFarbe()) +	"," +
        "'" + list_Bewertung[i] -> getFarbeBemerkung().replace("'","''") +	"'," +
        QString::number(list_Bewertung[i] -> getSchaum()) +	"," +
        "'" + list_Bewertung[i] -> getSchaumBemerkung().replace("'","''") +	"'," +
        QString::number(list_Bewertung[i] -> getGeruch()) +	"," +
        "'" + list_Bewertung[i] -> getGeruchBemerkung().replace("'","''") +	"'," +
        QString::number(list_Bewertung[i] -> getGeschmack()) +	"," +
        "'" + list_Bewertung[i] -> getGeschmackBemerkung().replace("'","''") +	"'," +
        QString::number(list_Bewertung[i] -> getAntrunk()) +	"," +
        "'" + list_Bewertung[i] -> getAntrunkBemerkung().replace("'","''") +	"'," +
        QString::number(list_Bewertung[i] -> getHaupttrunk()) +	"," +
        "'" + list_Bewertung[i] -> getHaupttrunkBemerkung().replace("'","''") +	"'," +
        QString::number(list_Bewertung[i] -> getNachtrunk()) +	"," +
        "'" + list_Bewertung[i] -> getNachtrunkBemerkung().replace("'","''") +	"'," +
        QString::number(list_Bewertung[i] -> getGesamteindruck()) +	"," +
        "'" + list_Bewertung[i] -> getGesamteindruckBemerkung().replace("'","''") +	"'" +
        +")";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
  }
}

void MainWindowImpl::LeseSuddatenDB()
{
  QSqlQuery query_sud;
  int FeldNr;
  QString sql = "SELECT * FROM Sud WHERE ID=" + QString::number(AktuelleSudID) + ";";
  if (!query_sud.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query_sud.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    if (query_sud.first()) {
      //Felder füllen

      //Sudname
      FeldNr = query_sud.record().indexOf("Sudname");
      lineEdit_Sudname -> setText(query_sud.value(FeldNr).toString());
      //Menge
      FeldNr = query_sud.record().indexOf("Menge");
      spinBox_Menge -> setValue(query_sud.value(FeldNr).toDouble());
      //Stammwürze
      FeldNr = query_sud.record().indexOf("SW");
      spinBox_SW -> setValue(query_sud.value(FeldNr).toDouble());
      //CO2 Gehalt
      FeldNr = query_sud.record().indexOf("CO2");
      doubleSpinBox_CO2 -> setValue(query_sud.value(FeldNr).toDouble());
      //Bitterkeit
      FeldNr = query_sud.record().indexOf("IBU");
      spinBox_IBU -> setValue(query_sud.value(FeldNr).toInt());
      //Reifezeit
      FeldNr = query_sud.record().indexOf("Reifezeit");
      spinBox_Reifezeit -> setValue(query_sud.value(FeldNr).toInt());
      //Kommentar
      FeldNr = query_sud.record().indexOf("Kommentar");
      //eventuell vorhandene HTML zeilenumbrüche gegen \n tausschen
      QString s = query_sud.value(FeldNr).toString();
      s.replace("<br>","\n");
      textEdit_Kommentar -> setPlainText(s);
      //Braudatum
      FeldNr = query_sud.record().indexOf("Braudatum");
      dateEdit_Braudatum -> setDate(QDate::fromString(query_sud.value(FeldNr).toString(),Qt::ISODate));
      //Flag ob Bier gebraut wurde
      FeldNr = query_sud.record().indexOf("BierWurdeGebraut");
      BierWurdeGebraut = query_sud.value(FeldNr).toBool();
      //Flag ob Bier abgefuellt wurde
      FeldNr = query_sud.record().indexOf("BierWurdeAbgefuellt");
      BierWurdeAbgefuellt = query_sud.value(FeldNr).toBool();
      //Flag ob Bier verbraucht wurde
      FeldNr = query_sud.record().indexOf("BierWurdeVerbraucht");
      BierWurdeVerbraucht = query_sud.value(FeldNr).toBool();
      //Auswahl Brauanlage
      FeldNr = query_sud.record().indexOf("AuswahlBrauanlage");
      int BrauanlagenID = query_sud.value(FeldNr).toInt();
      s.clear();
      for (int i=0; i < listWidget_Brauanlagen->count(); i++) {
        Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->item(i));
        if (item->getID() == BrauanlagenID) {
          s = item->text();
        }
      }
      if (s.isEmpty()) {
        FeldNr = query_sud.record().indexOf("AuswahlBrauanlageName");
        s = query_sud.value(FeldNr).toString();
      }
      if (BierWurdeGebraut){
        comboBox_AuswahlBrauanlage -> addItem(s);
        comboBox_AuswahlBrauanlage -> setCurrentIndex(comboBox_AuswahlBrauanlage -> count()-1);
      }
      else {
        for (int i=0; i < comboBox_AuswahlBrauanlage -> count(); i++){
          if (comboBox_AuswahlBrauanlage -> itemText(i) == s){
            comboBox_AuswahlBrauanlage -> setCurrentIndex(i);
          }
        }
      }
      //Kochdauer Nach Bitterhopfengabe
      FeldNr = query_sud.record().indexOf("KochdauerNachBitterhopfung");
      spinBox_Gesammtkochdauer -> setValue(query_sud.value(FeldNr).toInt());
      //Wasser
      FeldNr = query_sud.record().indexOf("FaktorHauptguss");
      doubleSpinBox_FaktorHG -> setValue(query_sud.value(FeldNr).toDouble());
      //Auswahl Hefe
      FeldNr = query_sud.record().indexOf("AuswahlHefe");
      s = query_sud.value(FeldNr).toString();
      if (BierWurdeGebraut){
        comboBox_AuswahlHefe -> addItem(s);
        comboBox_AuswahlHefe -> setCurrentIndex(comboBox_AuswahlHefe -> count()-1);
      }
      else {
        for (int i=0; i < comboBox_AuswahlHefe -> count(); i++){
          if (comboBox_AuswahlHefe -> itemText(i) == s){
            comboBox_AuswahlHefe -> setCurrentIndex(i);
          }
        }
      }
      //Anzahl Hefe Einheiten
      FeldNr = query_sud.record().indexOf("HefeAnzahlEinheiten");
      spinBox_AnzahlHefeEinheiten -> setValue(0);
      spinBox_AnzahlHefeEinheiten -> setValue(query_sud.value(FeldNr).toInt());
      //Vor dem Hopfenseihen
      //Würzemenge vor dem Hopfenseihen
      FeldNr = query_sud.record().indexOf("WuerzemengeVorHopfenseihen");
      spinBox_WuerzemengeVorHopfenseihen -> setValue(query_sud.value(FeldNr).toDouble());
      //Stammwürze vor dem Hopfenseihen
      FeldNr = query_sud.record().indexOf("SWVorHopfenseihen");
      spinBox_SWVorHopfenseihen -> setValue(query_sud.value(FeldNr).toDouble());

      //Nach dem Hopfenseihen
      //Würzemenge nach dem Hopfenseihen
      FeldNr = query_sud.record().indexOf("WuerzemengeKochende");
      spinBox_WuerzemengeKochende -> setValue(query_sud.value(FeldNr).toDouble());
      //Stammwürze nach dem Hopfenseihen
      FeldNr = query_sud.record().indexOf("SWKochende");
      spinBox_SWKochende -> setValue(query_sud.value(FeldNr).toDouble());
      //Abgezwackte Speisemenge
      FeldNr = query_sud.record().indexOf("Speisemenge");
      spinBox_Speisemenge -> setValue(query_sud.value(FeldNr).toDouble());

      //Vor der Hefezugabe
      //Datum der Hefezugabe
      FeldNr = query_sud.record().indexOf("Anstelldatum");
      dateEdit_Anstelldatum -> setDate(QDate::fromString(query_sud.value(FeldNr).toString(),Qt::ISODate));
      //Würzemenge beim Anstellen
      FeldNr = query_sud.record().indexOf("WuerzemengeAnstellen");
      spinBox_WuerzemengeAnstellen -> setValue(query_sud.value(FeldNr).toDouble());
      //Stammwürze beim Anstellen
      FeldNr = query_sud.record().indexOf("SWAnstellen");
      spinBox_SWAnstellen -> setValue(query_sud.value(FeldNr).toDouble());

      //Beim Abfüllen
      //Abfülldatum
      FeldNr = query_sud.record().indexOf("Abfuelldatum");
      dateEdit_Abfuelldatum -> setDate(QDate::fromString(query_sud.value(FeldNr).toString(),Qt::ISODate));
      //Stammwürze der Schnellgärprobe
      FeldNr = query_sud.record().indexOf("SWSchnellgaerprobe");
      spinBox_SWSchnellgaerprobe -> setValue(query_sud.value(FeldNr).toDouble());
      //Stammwürze Jungbier
      FeldNr = query_sud.record().indexOf("SWJungbier");
      spinBox_SWJungbier -> setValue(query_sud.value(FeldNr).toDouble());
      //Temperatur Jungbier beim Abfüllen
      FeldNr = query_sud.record().indexOf("TemperaturJungbier");
      spinBox_TemperaturJungbier -> setValue(query_sud.value(FeldNr).toDouble());
      //Temperatur Einmaischen
      FeldNr = query_sud.record().indexOf("EinmaischenTemp");
      spinBox_EinmaischenTemp -> setValue(query_sud.value(FeldNr).toInt());

      //Kosten Wasser Strom
      FeldNr = query_sud.record().indexOf("KostenWasserStrom");
      spinBox_Nebenkosten -> setValue(query_sud.value(FeldNr).toDouble());

      //Nachisomerisierungszeit
      FeldNr = query_sud.record().indexOf("Nachisomerisierungszeit");
      spinBox_NachisomerisierungsZeit -> setValue(query_sud.value(FeldNr).toDouble());

      //Gewünschte Restalkalität
      FeldNr = query_sud.record().indexOf("RestalkalitaetSoll");
      SpinBox_waSollRestalkalitaet_dh -> setValue(query_sud.value(FeldNr).toDouble());

      //Schnellgärprobe aktiv
      FeldNr = query_sud.record().indexOf("SchnellgaerprobeAktiv");
      checkBox_SchnellgaerprobeAktiv -> setChecked(query_sud.value(FeldNr).toBool());

      //Spunden
      FeldNr = query_sud.record().indexOf("Spunden");
      checkBox_Spunden -> setChecked(query_sud.value(FeldNr).toBool());

      //Jungbiermenge Abfüllen
      FeldNr = query_sud.record().indexOf("JungbiermengeAbfuellen");
      spinBox_JungbiermengeAbfuellen -> setValue(query_sud.value(FeldNr).toDouble());

      //Berechnungsart Hopfen
      FeldNr = query_sud.record().indexOf("berechnungsArtHopfen");
      comboBox_BerechnungsArtHopfen -> setCurrentIndex(query_sud.value(FeldNr).toInt());

      //High Gravity Faktor
      FeldNr = query_sud.record().indexOf("highGravityFaktor");
      spinBox_High_Gravity -> setValue(query_sud.value(FeldNr).toInt());

      //Ignor Ausbeute
      FeldNr = query_sud.record().indexOf("AusbeuteIgnorieren");
      checkBox_AusbeuteIgnorieren->setChecked(query_sud.value(FeldNr).toBool());

      //Ergebnisse
      if (BierWurdeGebraut) {
        FeldNr = query_sud.record().indexOf("erg_S_Gesammt");
        doubleSpinBox_S_Gesammt -> setValue(query_sud.value(FeldNr).toDouble());
        FeldNr = query_sud.record().indexOf("erg_W_Gesammt");
        doubleSpinBox_W_Gesammt -> setValue(query_sud.value(FeldNr).toDouble());
        FeldNr = query_sud.record().indexOf("erg_WHauptguss");
        doubleSpinBox_WHauptguss -> setValue(query_sud.value(FeldNr).toDouble());
        FeldNr = query_sud.record().indexOf("erg_WNachguss");
        doubleSpinBox_WNachguss -> setValue(query_sud.value(FeldNr).toDouble());
        //FeldNr = query_sud.record().indexOf("erg_Sudhausausbeute");
        //spinBox_Sudhausausbeute -> setValue(query_sud.value(FeldNr).toDouble());
        FeldNr = query_sud.record().indexOf("erg_Farbe");
        doubleSpinBox_EBC -> setValue(query_sud.value(FeldNr).toDouble());
        FeldNr = query_sud.record().indexOf("erg_Preis");
        spinBox_Preis -> setValue(query_sud.value(FeldNr).toDouble());
        //FeldNr = query_sud.record().indexOf("erg_Alkohol");
        //doubleSpinBox_WNachguss -> setValue(query_sud.value(FeldNr).toDouble());
      }

      //Rasten Abfragen
      QSqlQuery query_rasten;
      sql = "SELECT * FROM Rasten WHERE SudID=" + QString::number(AktuelleSudID) + ";";
      if (!query_rasten.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_rasten.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
      else {
        bool b;
        int i = 1;
        //Erstmal Rastliste leeren
        while (list_Rasten.count() > 0){
          list_Rasten[0] -> close();
        }
        list_Rasten.clear();
        //Alle Rasten einlesen
        while (query_rasten.next()){
          FeldNr = query_rasten.record().indexOf("RastAktiv");
          b = query_rasten.value(FeldNr).toBool();
          //wenn Rast aktiv ist Objekt erstellen und dem Layout hinzufügen
          if (b){
            //Rastobjekt erstellen
            Rastwidget* rast = new Rastwidget(this);
            rast -> setAttribute(Qt::WA_DeleteOnClose);
            //Rastwidget dem Layout zuordnen
            verticalLayout_Rasten -> addWidget(rast);
            list_Rasten.append(rast);
            int FeldNr_Name = query_rasten.record().indexOf("RastName");
            int FeldNr_Temp = query_rasten.record().indexOf("RastTemp");
            int FeldNr_Dauer = query_rasten.record().indexOf("RastDauer");
            rast -> setRastName(query_rasten.value(FeldNr_Name).toString());
            rast->setRastTemp(query_rasten.value(FeldNr_Temp).toInt());
            rast->setRastDauer(query_rasten.value(FeldNr_Dauer).toInt());
            rast->setID((int)time(NULL)+rand());
            if (list_Rasten.count() > RAST_ANIMATION_STOP){
              for (int i=0; i<list_Rasten.count(); i++){
                list_Rasten[i]->setAnimationAus(true);
              }
            }
            else {
              for (int i=0; i<list_Rasten.count(); i++){
                list_Rasten[i]->setAnimationAus(false);
              }
            }
            connect(rast, SIGNAL( sig_vorClose(int) ), this, SLOT( slot_rastClose(int) ));
            connect(rast, SIGNAL( sig_aenderung(int) ), this, SLOT( slot_rastAenderung(int) ));
            connect(rast, SIGNAL( sig_nachOben(int) ), this, SLOT( slot_pushButton_RastNachOben(int) ));
            connect(rast, SIGNAL( sig_nachUnten(int) ), this, SLOT( slot_pushButton_RastNachUnten(int) ));
          }
          i++;
        }
      }
      //Schüttung Abfragen
      QSqlQuery query_Malz;
      sql = "SELECT * FROM Malzschuettung WHERE SudID=" + QString::number(AktuelleSudID) + " ORDER by Prozent DESC;";
      if (!query_Malz.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_Malz.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
      else {
        //Erstmal Zutatenlsite leeren
        for (int i=0; i < list_Malzgaben.count(); i++){
          list_Malzgaben[i] -> close();
        }
        list_Malzgaben.clear();
        while (query_Malz.next()){
          int FeldNr_Name = query_Malz.record().indexOf("Name");
          int FeldNr_Prozent = query_Malz.record().indexOf("Prozent");
          int FeldNr_ergMenge = query_Malz.record().indexOf("erg_Menge");
          int FeldNr_Farbe = query_Malz.record().indexOf("Farbe");
          if (query_Malz.value(FeldNr_Name).toString() != ""){
            AddMalzgabe(query_Malz.value(FeldNr_Name).toString(),
                        query_Malz.value(FeldNr_Prozent).toDouble(),
                        query_Malz.value(FeldNr_ergMenge).toDouble(),
                        query_Malz.value(FeldNr_Farbe).toDouble() );
          }
        }
      }
      //Hopfengaben Abfragen
      QSqlQuery query_Hopfen;
      sql = "SELECT * FROM Hopfengaben WHERE SudID=" + QString::number(AktuelleSudID) + " AND Aktiv=1 ORDER by Vorderwuerze DESC, Zeit DESC;";
      if (!query_Hopfen.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_Hopfen.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
      else {
        //Erstmal Zutatenlsite leeren
        for (int i=0; i < list_Hopfengaben.count(); i++){
          list_Hopfengaben[i] -> close();
        }
        list_Hopfengaben.clear();
        while (query_Hopfen.next()){
          int FeldNr_Name = query_Hopfen.record().indexOf("Name");
          int FeldNr_Zeit = query_Hopfen.record().indexOf("Zeit");
          int FeldNr_Prozent = query_Hopfen.record().indexOf("Prozent");
          int FeldNr_erg_Menge = query_Hopfen.record().indexOf("erg_Menge");
          int FeldNr_Alpha = query_Hopfen.record().indexOf("Alpha");
          int FeldNr_Pellets = query_Hopfen.record().indexOf("Pellets");
          int FeldNr_Vorderwuerze = query_Hopfen.record().indexOf("Vorderwuerze");
          AddHopfengabe(query_Hopfen.value(FeldNr_Vorderwuerze).toBool(),
                        query_Hopfen.value(FeldNr_Name).toString(),
                        query_Hopfen.value(FeldNr_Zeit).toInt(),
                        query_Hopfen.value(FeldNr_Prozent).toDouble(),
                        query_Hopfen.value(FeldNr_erg_Menge).toDouble(),
                        query_Hopfen.value(FeldNr_Alpha).toDouble(),
                        query_Hopfen.value(FeldNr_Pellets).toInt());
        }
      }
      //Erweiterte Zutaten abfragen
      QSqlQuery query_ewz;
      sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(AktuelleSudID) + " ORDER BY Zeitpunkt;";
      if (!query_ewz.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_ewz.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
      else {
        //Erstmal Zutatenlsite leeren
        for (int i=0; i < list_EwZutat.count(); i++){
          list_EwZutat[i] -> close();
        }
        list_EwZutat.clear();
        //Zutaten erstellen
        while (query_ewz.next()){
          //Zutatenobjekt hinzufügen
          ErweiterteZutatImpl* ewz = new ErweiterteZutatImpl(this);
          ewz -> setAttribute(Qt::WA_DeleteOnClose);
          ewz -> setStyleDunkel(StyleDunkel);

          ewz -> setBierWurdeGebraut(BierWurdeGebraut);
          ewz -> setBierWurdeAbgefuellt(BierWurdeAbgefuellt);
          //Funktionen verknüpfen das das objekt die Daten holen kann
          connect(ewz, SIGNAL( sig_vorClose(int) ), this, SLOT( slot_ewzClose(int) ));
          connect(ewz, SIGNAL( sig_getEwzTyp(QString) ), this, SLOT( slot_getEwzTyp(QString) ));
          connect(ewz, SIGNAL( sig_getEwzEinheit(QString) ), this, SLOT( slot_getEwzEinheit(QString) ));
          connect(ewz, SIGNAL( sig_getEwzAusbeute(QString) ), this, SLOT( slot_getEwzAusbeute(QString) ));
          connect(ewz, SIGNAL( sig_getEwzFarbe(QString) ), this, SLOT( slot_getEwzFarbe(QString) ));
          connect(ewz, SIGNAL( sig_getEwzPreis(QString) ), this, SLOT( slot_getEwzPreis(QString) ));
          connect(ewz, SIGNAL( sig_getEwzPreisHopfen(QString) ), this, SLOT( slot_getEwzPreisHopfen(QString) ));
          connect(ewz, SIGNAL( sig_zugeben(QString, int, double) ), this, SLOT( slot_EwzZugegeben(QString, int, double) ));
          connect(ewz, SIGNAL( sig_getHopfenMenge(QString) ), this, SLOT( slot_HopfenGetMenge(QString) ));
          connect(ewz, SIGNAL( sig_getEwzMenge(QString) ), this, SLOT( slot_EwzGetMenge(QString) ));
          if (BierWurdeGebraut) {
            //Da Bier schon gebraut wurde die daten aus der Datenbank benutzten und nicht aus den Rohstoffdaten
            //da der Rohstoff unter umständen nicht mehr existiert oder verändert wurde
            int FeldNr = query_ewz.record().indexOf("Typ");
            ewz -> setTyp(query_ewz.value(FeldNr).toInt());
            FeldNr = query_ewz.record().indexOf("Ausbeute");
            ewz -> setAusbeute(query_ewz.value(FeldNr).toInt());
            FeldNr = query_ewz.record().indexOf("Farbe");
            ewz -> setFarbe(query_ewz.value(FeldNr).toDouble());
            FeldNr = query_ewz.record().indexOf("Einheit");
            ewz -> setEinheit(query_ewz.value(FeldNr).toInt());
          }
          else {
            //Zutatenliste füllen
            ewz -> setEwListe(ewzListe);
            ewz -> setHopfenListe(HopfenListe);
          }
          connect(ewz, SIGNAL( sig_Aenderung() ), this, SLOT( slot_EwzAenderung() ));

          //Ergebnisswidget dem Layout zuordnen
          verticalLayout_BerWeitereZutaten -> addWidget(ewz->ergWidget);
          verticalLayout_WeitereZutaten -> addWidget(ewz);
          list_EwZutat.append(ewz);
          ewz -> setID((int)time(NULL)+rand());
          int FeldNr_Name = query_ewz.record().indexOf("Name");
          ewz -> setName(query_ewz.value(FeldNr_Name).toString());
          FeldNr_Name = query_ewz.record().indexOf("Menge");
          ewz -> setMenge(query_ewz.value(FeldNr_Name).toDouble());
          FeldNr_Name = query_ewz.record().indexOf("Zeitpunkt");
          ewz -> setZeitpunkt(query_ewz.value(FeldNr_Name).toInt());
          FeldNr_Name = query_ewz.record().indexOf("Bemerkung");
          ewz -> setBemerkung(query_ewz.value(FeldNr_Name).toString());
          FeldNr_Name = query_ewz.record().indexOf("Zugabestatus");
          ewz -> setZugabestatus(query_ewz.value(FeldNr_Name).toInt());
          //zugabezeitpunkt
          QDate date_von;
          FeldNr = query_ewz.record().indexOf("Zeitpunkt_von");
          date_von = QDate::fromString(query_ewz.value(FeldNr).toString(),Qt::ISODate);
          QDate date_bis;
          FeldNr = query_ewz.record().indexOf("Zeitpunkt_bis");
          date_bis = QDate::fromString(query_ewz.value(FeldNr).toString(),Qt::ISODate);
          ewz -> setZugabezeitpunkt(date_von,date_bis);
          FeldNr_Name = query_ewz.record().indexOf("Entnahmeindex");
          ewz -> setEntnahmeindex(query_ewz.value(FeldNr_Name).toInt());
          FeldNr_Name = query_ewz.record().indexOf("Zugabedauer");
          ewz -> setDauerMinuten(query_ewz.value(FeldNr_Name).toInt());
        }
      }

      //Bewertungen abfragen
      //Erstmal liste leeren
      graphicsView_bewStar->setAnzahlStar(0);
      for (int i=0; i < list_Bewertung.count(); i++){
        list_Bewertung[i] -> close();
      }
      list_Bewertung.clear();
      widget_bewertung->setEnabled(false);

      //Bewertungen abfragen
      QSqlQuery query_bew;
      sql = "SELECT * FROM Bewertungen WHERE SudID=" + QString::number(AktuelleSudID)  + " ORDER BY Datum;";;
      if (!query_bew.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_bew.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
      else {
        //Bewertungen erstellen
        while (query_bew.next()){
          //Bewertung hinzufügen
          Bewertung* bew = new Bewertung(this);
          bew -> setAttribute(Qt::WA_DeleteOnClose);
          bew -> setStyleDunkel(StyleDunkel);
          bew -> setMaxSterne(MaxAnzahlSterne);

          verticalLayout_Bewertungen -> addWidget(bew);
          list_Bewertung.append(bew);

          //Datum für start Reifezeit ermitteln und setzten

          //Start der Reifung ermitteln indem das letzte Datum vom
          //Nachgärverlauf benutzt wird
          QSqlQuery queryN;
          QString sqlN = "SELECT * FROM Nachgaerverlauf WHERE SudID=" + QString::number(AktuelleSudID) + " ORDER BY Zeitstempel DESC;";
          QDate date;
          date = dateEdit_Abfuelldatum -> date();

          if (!queryN.exec(sqlN)) {
            // Fehlermeldung Datenbankabfrage
            ErrorMessage *errorMessage = new ErrorMessage();
            errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                        CANCEL_NO, trUtf8("Rückgabe:\n") + queryN.lastError().databaseText()
                                        + trUtf8("\nSQL Befehl:\n") + sqlN);
          }
          else {
            if (queryN.first()){
              int FeldNr = queryN.record().indexOf("Zeitstempel");
              date = QDate::fromString(queryN.value(FeldNr).toString(),Qt::ISODate);
            }
          }

          //Abfülldatum setzten
          bew->setReifedatum(date);
          bew->setID((int)time(NULL)+rand());
          connect(bew, SIGNAL( sig_wocheClicked(int)), this, SLOT( slot_BewertungWoche_clicked(int)));
          connect(bew, SIGNAL( sig_vorClose(int)), this, SLOT( slot_bewClose(int)));

          int FeldNr_Name = query_bew.record().indexOf("Datum");
          bew -> setBewertungsdatum(QDate::fromString(query_bew.value(FeldNr_Name).toString(),Qt::ISODate));

          FeldNr_Name = query_bew.record().indexOf("Sterne");
          bew -> setSterne(query_bew.value(FeldNr_Name).toInt());

          FeldNr_Name = query_bew.record().indexOf("Bemerkung");
          bew -> setBemerkung(query_bew.value(FeldNr_Name).toString()) ;

          FeldNr_Name = query_bew.record().indexOf("Farbe");
          bew -> setFarbe(query_bew.value(FeldNr_Name).toInt());

          FeldNr_Name = query_bew.record().indexOf("FarbeBemerkung");
          bew -> setFarbeBemerkung(query_bew.value(FeldNr_Name).toString());

          FeldNr_Name = query_bew.record().indexOf("Schaum");
          bew -> setSchaum(query_bew.value(FeldNr_Name).toInt());

          FeldNr_Name = query_bew.record().indexOf("SchaumBemerkung");
          bew -> setSchaumBemerkung(query_bew.value(FeldNr_Name).toString());

          FeldNr_Name = query_bew.record().indexOf("Geruch");
          bew -> setGeruch(query_bew.value(FeldNr_Name).toInt());

          FeldNr_Name = query_bew.record().indexOf("GeruchBemerkung");
          bew -> setGeruchBemerkung(query_bew.value(FeldNr_Name).toString());

          FeldNr_Name = query_bew.record().indexOf("Geschmack");
          bew -> setGeschmack(query_bew.value(FeldNr_Name).toInt());

          FeldNr_Name = query_bew.record().indexOf("GeschmackBemerkung");
          bew -> setGeschmackBemerkung(query_bew.value(FeldNr_Name).toString());

          FeldNr_Name = query_bew.record().indexOf("Antrunk");
          bew -> setAntrunk(query_bew.value(FeldNr_Name).toInt());

          FeldNr_Name = query_bew.record().indexOf("AntrunkBemerkung");
          bew -> setAntrunkBemerkung(query_bew.value(FeldNr_Name).toString());

          FeldNr_Name = query_bew.record().indexOf("Haupttrunk");
          bew -> setHaupttrunk(query_bew.value(FeldNr_Name).toInt());

          FeldNr_Name = query_bew.record().indexOf("HaupttrunkBemerkung");
          bew -> setHaupttrunkBemerkung(query_bew.value(FeldNr_Name).toString());

          FeldNr_Name = query_bew.record().indexOf("Nachtrunk");
          bew -> setNachtrunk(query_bew.value(FeldNr_Name).toInt());

          FeldNr_Name = query_bew.record().indexOf("NachtrunkBemerkung");
          bew -> setNachtrunkBemerkung(query_bew.value(FeldNr_Name).toString());

          FeldNr_Name = query_bew.record().indexOf("Gesamteindruck");
          bew -> setGesamteindruck(query_bew.value(FeldNr_Name).toInt());

          FeldNr_Name = query_bew.record().indexOf("GesamteindruckBemerkung");
          bew -> setGesamteindruckBemerkung(query_bew.value(FeldNr_Name).toString());

          //Aktivieren
          slot_BewertungWoche_clicked(bew->getID());
        }
      }

      // Anhang Abfragen
      LeseAnhangDB();

      //Gespeichertes Tab wiederherstellen
      //FeldNr = query_sud.record().indexOf("AktivTab");
      //tabWidged -> setCurrentIndex(query_sud.value(FeldNr).toInt());
      //Gespeichertes Tab im Gärverlauf wiederherstellen

      FeldNr = query_sud.record().indexOf("AktivTab_Gaerverlauf");
      toolBox_Gaerverlauf -> setCurrentIndex(query_sud.value(FeldNr).toInt());
    }
  }

  BerWeitereZutaten();
  BerBraudaten();
  //Lese Tabellendaten für den Gärverlauf
  LeseSchnellgaerverlaufDB();
  LeseHauptgaerverlaufDB();
  LeseNachgaerverlaufDB();
  setFensterTitel();
}

void MainWindowImpl::FuelleRezeptComboAuswahlen()
{
  //Wenn Bier schon gebraut wurde nicht neu einlesen
  if (!BierWurdeGebraut){
    //Merker setzten das in der Zeit in der die Comboauswahlen neu
    //gefüllt werden keine berechnungen gemacht werden
    //da sonnst alle Malz werte Auf Null gesetzt werden
    ComboboxWirdGefuellt = true;

    //Brauanlagen
    //Aktuelle Auswahl zwischenspeichern
    int aba = comboBox_AuswahlBrauanlage -> currentIndex();
    comboBox_AuswahlBrauanlage->clear();
    for (int i=0; i < listWidget_Brauanlagen->count(); i++) {
      comboBox_AuswahlBrauanlage->addItem(listWidget_Brauanlagen->item(i)->text());
    }
    //zwischengespeicherte auswahl wieder anwählen
    if (comboBox_AuswahlBrauanlage->count() <= aba || aba < 0) {
      aba = 0;
    }
    comboBox_AuswahlBrauanlage->setCurrentIndex(aba);

    //Malz
    for (int i=0; i < list_Malzgaben.count(); i++){
      list_Malzgaben[i] -> setMalzListe(MalzListe);
    }

    //Hopfen
    for (int i=0; i < list_Hopfengaben.count(); i++){
      list_Hopfengaben[i] -> setHopfenListe(HopfenListe);
    }

    //Hefe
    int AnzahlHefeEintraege;
    //Aktuelle Auswahl zwischenspeichern
    QString s1 = comboBox_AuswahlHefe -> currentText();
    comboBox_AuswahlHefe -> clear();
    AnzahlHefeEintraege = tableWidget_Hefe -> rowCount();
    //Als erstes einen Leereintrag hinzufügen
    comboBox_AuswahlHefe -> addItem("");
    //Dann mit den Rohstoffdaten füllen
    for (int i=0; i < AnzahlHefeEintraege; i++){
      comboBox_AuswahlHefe -> addItem(tableWidget_Hefe -> item(i,TableHefeColName) -> text() );
    }
    //zwischengespeicherte auswahl wieder anwählen
    for (int i=0; i < comboBox_AuswahlHefe -> count(); i++){
      if (comboBox_AuswahlHefe -> itemText(i) == s1)
        comboBox_AuswahlHefe -> setCurrentIndex(i);
    }

    //Weitere Zutaten
    for (int i=0; i < list_EwZutat.count(); i++){
      list_EwZutat[i] -> setEwListe(ewzListe);
      list_EwZutat[i] -> setHopfenListe(HopfenListe);
    }

    ComboboxWirdGefuellt = false;
    BerAlles();
  }
}


void MainWindowImpl::BerAlles()
{
  // Komplett alles durchberechnen nur wenn Programm gestartet ist
  if (Gestartet && !AmLaden && !NichtBerechnen){
    //Berechnungen Ausrüstung
    BerAusruestung();

    //Faktor High Gravity verrechnen
    highGravityFaktor = 1 + (double(spinBox_High_Gravity->value())/100);

    //Sollmenge Würze nach dem Hopfenseigen
    spinBox_MengeSollNachHopfenseihen->setValue(spinBox_Menge->value()/highGravityFaktor);

    //QMessageBox::information ( this, "", "In Berechnung alles") ;
    if (!BierWurdeGebraut){
      //Gesammt Sollstammwürze
      sw_gesammt = spinBox_SW -> value();

      //Mengen der Weiterten Zutaten Berechnen
      BerWeitereZutaten();

      //MalzSchüttung und Anteile Berechnen
      BerSchuettung();

      //Haupt und Nachguss berechnen
      BerWasser();

      //Hopfengaben berechnen
      BerHopfen();

      //Bierfarbe Berechnen
      BerFarbe();

      //Empfehlung Faktor Hauptguss anhand der Bierfarbe berechnen
      BerEmpfehlungFaktorHauptguss();

      //Empfehlung für Anzahl Hefe Einheiten berechnen
      BerEmpfehlungHefeEinheiten();

      //Literpreis berechnen
      BerKosten();
    }
    else {
      //Mengen der Weiterten Zutaten Berechnen
      BerWeitereZutaten();

      //wenn nicht abgefüllt wurde comboliste für weitere zutaten zugeben in hauptgärung füllen
      if (!BierWurdeAbgefuellt) {
        fuelleComboEwzZugeben();
        fuelleComboEwzEntnehmen();
      }

      //Wenn die gespeicherte Bierfarbe 0 ist versuchen die Bierfarbe zu berechnen
      if (doubleSpinBox_EBC -> value() == 0){
        BerFarbe();
      }
      else {
        BerFarbe(doubleSpinBox_EBC -> value());
      }
      //Versuchen die Kosten zu berechnen wenn kein Wert vorliegt
      if (spinBox_Preis -> value() == 0){
        BerKosten();
      }
      else {
        spinBox_Preis->setToolTip("");
        spinBox_Preis->setPalette(spinBox_Haushaltszucker500mlFlasche -> palette());
      }
    }

    //Berechne Volumen Maischen
    BerVolumenMaischen();

    //Berechne Pfanne Voll
    BerPfanneVoll();

    //Braudaten Berechnen
    BerBraudaten();

    //Eingaben auf fehler Überprüfen
    CheckFehler();

    //Wasserwerte Berechnen
    BerWasserwerte();

  }
}

void MainWindowImpl::slot_dateChanged(QDate )
{
  if (Gestartet) {
    setAenderung(true);
  }
}

void MainWindowImpl::slot_AenderungAusruestung(double )
{
  AenderungAusruestung = true;
}

void MainWindowImpl::slot_AenderungAusruestung(int )
{
  AenderungAusruestung = true;
}

void MainWindowImpl::slot_spinBoxValueChanged(double )
{
  if (Gestartet) {
    BerAlles();
    setAenderung(true);
  }
}

void MainWindowImpl::slot_RohstoffNonZeroValueChanged(double menge)
{
    QDoubleSpinBox *spinbox = dynamic_cast<QDoubleSpinBox*>(QObject::sender());
    if (spinbox)
    {
        QColor color = menge > 0 ? qApp->palette().color(QPalette::Active, QPalette::Base) : QColor::fromRgb(200, 80, 80);
        QPalette palette(spinbox->palette());
        palette.setColor(QPalette::Active, QPalette::Base, color);
        palette.setColor(QPalette::Inactive, QPalette::Base, color);
        spinbox->setPalette(palette);
    }
}

void MainWindowImpl::slot_RohstoffMengeValueChanged(double menge)
{
    slot_RohstoffNonZeroValueChanged(menge);
    QVariant prop = QObject::sender()->property("TableWidgetItem");
    if (prop.isValid())
    {
        QTableWidgetItem* item = static_cast<QTableWidgetItem*>(prop.value<void*>());
        if (item)
        {
            QTableWidget *table = item->tableWidget();
            if (table == tableWidget_Malz)
            {
                table->cellWidget(item->row(), TableMalzColEinlagerung)->setEnabled(menge > 0);
                table->cellWidget(item->row(), TableMalzColMindesthalbat)->setEnabled(menge > 0);
            }
            else if (table == tableWidget_Hopfen)
            {
                table->cellWidget(item->row(), TableHopfenColEinlagerung)->setEnabled(menge > 0);
                table->cellWidget(item->row(), TableHopfenColMindesthalbat)->setEnabled(menge > 0);
            }
            else if (table == tableWidget_Hefe)
            {
                table->cellWidget(item->row(), TableHefeColEinlagerung)->setEnabled(menge > 0);
                table->cellWidget(item->row(), TableHefeColMindesthalbat)->setEnabled(menge > 0);
            }
            else if (table == tableWidget_WeitereZutaten)
            {
                table->cellWidget(item->row(), TableWZutatColEinlagerung)->setEnabled(menge > 0);
                table->cellWidget(item->row(), TableWZutatColMindesthalbat)->setEnabled(menge > 0);
            }
        }
    }
}

void MainWindowImpl::slot_RohstoffHaltbarValueChanged(const QDate &date)
{
    QDateEdit *dateedit = dynamic_cast<QDateEdit*>(QObject::sender());
    if (dateedit)
    {
        QColor color = QDate::currentDate().daysTo(date) > 0 ? qApp->palette().color(QPalette::Active, QPalette::Base) : QColor::fromRgb(200, 80, 80);
        QPalette palette(dateedit->palette());
        palette.setColor(QPalette::Active, QPalette::Base, color);
        palette.setColor(QPalette::Inactive, QPalette::Base, color);
        dateedit->setPalette(palette);
    }
}

void MainWindowImpl::slot_RohstoffFarbeValueChanged(double ebc)
{
    QDoubleSpinBox *spinbox = dynamic_cast<QDoubleSpinBox*>(QObject::sender());
    if (spinbox)
    {
        QPalette palette(spinbox->palette());
        if (ebc > 0)
            palette.setColor(QPalette::Active, QPalette::Base, Berechnungen.GetFarbwert(ebc));
        else
            palette.setColor(QPalette::Active, QPalette::Base, qApp->palette().color(QPalette::Active, QPalette::Base));
        palette.setColor(QPalette::Active, QPalette::Text, ebc > 35 ? Qt::white: Qt::black);
        palette.setColor(QPalette::Inactive, QPalette::Base, palette.color(QPalette::Active, QPalette::Base));
        palette.setColor(QPalette::Inactive, QPalette::Text, palette.color(QPalette::Active, QPalette::Text));
        spinbox->setPalette(palette);
    }
}

void MainWindowImpl::slot_tableSyncValueChanged(double value)
{
    // synchronize with table widget item for sorting
    QVariant prop = QObject::sender()->property("TableWidgetItem");
    if (prop.isValid())
    {
        MyTableWidgetItemNumeric* item = static_cast<MyTableWidgetItemNumeric*>(prop.value<void*>());
        if (item)
            item->setValue(value);
    }
}

void MainWindowImpl::slot_tableSyncValueChanged(const QString& value)
{
    // synchronize with table widget item for sorting
    QVariant prop = QObject::sender()->property("TableWidgetItem");
    if (prop.isValid())
    {
        QTableWidgetItem* item = static_cast<QTableWidgetItem*>(prop.value<void*>());
        if (item)
            item->setText(value);
    }
}

void MainWindowImpl::slot_tableSyncValueChanged(const QDate& date)
{
    // synchronize with table widget item for sorting
    QVariant prop = QObject::sender()->property("TableWidgetItem");
    if (prop.isValid())
    {
        MyTableWidgetItemNumeric* item = static_cast<MyTableWidgetItemNumeric*>(prop.value<void*>());
        if (item)
            item->setValue(date.toJulianDay());
    }
}

void MainWindowImpl::slot_spinBoxValueChanged(int )
{
  if (Gestartet) {
    BerAlles();
    setAenderung(true);
  }
}

void MainWindowImpl::slot_ComboBoxIndexChanged(int )
{
  if (Gestartet) {
    if (!ComboboxWirdGefuellt) {
      BerAlles();
      setAenderung(true);
    }
  }
}

void MainWindowImpl::BerSchuettung()
{

  //Anhand der verbleibenden Wunschstammwürze Gesammtschüttung berechnen
  double gs;
  gs = Berechnungen.GetGesammtSchuettung(spinBox_Menge -> value()/highGravityFaktor,
                                         sw_schuettung*highGravityFaktor, getAngenommeneSudhausausbeute());

  //Überprüfen ob die Schüttungsaufteilung auch 100% entspricht
  double p = 100;
  for (int i=0; i < list_Malzgaben.count(); i++){
    p -= list_Malzgaben[i]->getMengeProzent();
  }
  for (int i=0; i < list_Malzgaben.count(); i++){
    if ((p > -0.01) && (p < 0.01))
      p = 0;
    list_Malzgaben[i]->setFehlProzent(p);
    list_Malzgaben[i]->setGesamtMenge(gs);
  }

  doubleSpinBox_S_Gesammt -> setValue(gs);

  //Anhand der Gesammtschüttung die Einzelnen malzsorten berechnen
  for (int i=0; i < list_Malzgaben.count(); i++){
    list_Malzgaben[i]->berMenge();
  }
}

void MainWindowImpl::BerWasser()
{
  //Hauptguss berechnen
  double hg;
  hg = Berechnungen.GetHauptguss(doubleSpinBox_FaktorHG -> value());

  //Nachguss berechnen
  double ng;
  ng = Berechnungen.GetNachguss(getVerdampfungsziffer(),
                                spinBox_Gesammtkochdauer->value(),
                                getKorrekturWassermenge());

  doubleSpinBox_WHauptguss -> setValue(hg);
  doubleSpinBox_WNachguss -> setValue(ng);
  //Gesammtwassermenge
  doubleSpinBox_W_Gesammt -> setValue(hg + ng);

}

void MainWindowImpl::BerHopfen()
{
  //Überprüfen ob die Schüttungsaufteilung auch 100% entspricht
  double p = 100;
  for (int i=0; i < list_Hopfengaben.count(); i++){
    p -= list_Hopfengaben[i]->getMengeProzent();
  }
  for (int i=0; i < list_Hopfengaben.count(); i++){
    if ((p > -0.01) && (p < 0.01))
      p = 0;
    list_Hopfengaben[i]->setFehlProzent(p);
  }

  //Alpha Prozent von den Hopfenauswahlen holen (und Pellets)
  double *Alphaprozent = new double[list_Hopfengaben.count()];
  QString s;
  //Pellets
  bool *Pellets = new bool[list_Hopfengaben.count()];
  //Vorderwürzehopfung
  bool *vwh = new bool[list_Hopfengaben.count()];
  //Kochzeiten
  int *Kochzeiten = new int[list_Hopfengaben.count()];
  //Mengen
  double *MengenProzent = new double[list_Hopfengaben.count()];
  //Ausbeute der einzelnen Hopfengaben
  double *Ausbeute = new double[list_Hopfengaben.count()];
  //Berechnung anschmeissen
  double *HopfenMengen = new double[list_Hopfengaben.count()];
  //IBU Anteil der einzelnen Gaben
  double *IBUAnteil = new double[list_Hopfengaben.count()];
  //Berechnung nach IBUProzent
  bool berIBUProzent;
  if (comboBox_BerechnungsArtHopfen->currentIndex() == 0)
    berIBUProzent = false;
  else
    berIBUProzent = true;

  for (int o=0;o < list_Hopfengaben.count(); o++){
    list_Hopfengaben[o]->setBerIBUProzent(berIBUProzent);
    list_Hopfengaben[o]->setSollIBU(spinBox_IBU -> value()*highGravityFaktor);
    s = list_Hopfengaben[o]->getName();
    vwh[o]=list_Hopfengaben[o]->getVWH();
    Kochzeiten[o] = list_Hopfengaben[o]->getKochzeit()+spinBox_NachisomerisierungsZeit -> value();
    //		qDebug() << "Kochzeiten[o] "<< o << " " << Kochzeiten[o];
    MengenProzent[o] = list_Hopfengaben[o]->getMengeProzent();
    for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
      if (s == tableWidget_Hopfen -> item(i,TableHopfenColName) -> text()){
        QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,TableHopfenColAlpha);
        Alphaprozent[o] = spinBox->value();
        Pellets[o] = tableWidget_Hopfen -> item(i,TableHopfenColPellets) -> checkState();
        list_Hopfengaben[o]->setAlpha(Alphaprozent[o]);
        list_Hopfengaben[o]->setPellets(Pellets[o]);
      }
    }
  }


  //Sollstammwürze für das Hopfenkochen zuweisen
  Berechnungen.setSollStammwuerze(sw_kochen);
  //	qDebug() << "MengenProzent[0] " << MengenProzent[0];
  //	qDebug() << "Kochzeiten[0] "<< Kochzeiten[0];
  //Berechnen nach gewichtsprozent
  //qDebug() << "IBU: " << spinBox_IBU -> value()*highGravityFaktor;
  //qDebug() << "sw_kochen: " << sw_kochen;
  if (!berIBUProzent){
    Berechnungen.BerHopfenGewichtProzent( HopfenMengen,
                                          Alphaprozent,
                                          MengenProzent,
                                          Kochzeiten,
                                          Pellets,
                                          spinBox_IBU -> value()*highGravityFaktor,
                                          list_Hopfengaben.count(),
                                          vwh,
                                          Ausbeute,
                                          IBUAnteil);
  }
  //Berechnen nach IBU prozent
  else {
    Berechnungen.BerHopfenIBUProzent( HopfenMengen,
                                      Alphaprozent,
                                      MengenProzent,
                                      Kochzeiten,
                                      Pellets,
                                      spinBox_IBU -> value()*highGravityFaktor,
                                      list_Hopfengaben.count(),
                                      vwh,
                                      Ausbeute,
                                      IBUAnteil);
  }

  //Berechnette Mengen zuweisen
  for (int i=0; i < list_Hopfengaben.count(); i++){
    list_Hopfengaben[i]->setIBUAnteil(IBUAnteil[i]);
    list_Hopfengaben[i]->setAusbeute(Ausbeute[i]);
    list_Hopfengaben[i]->setErgMenge(HopfenMengen[i]);
  }
  delete [] Alphaprozent;
  delete [] Pellets;
  delete [] vwh;
  delete [] Kochzeiten;
  delete [] MengenProzent;
  delete [] Ausbeute;
  delete [] HopfenMengen;
  delete [] IBUAnteil;
}

void MainWindowImpl::ErstelleTabSpickzettel()
{
    if (BierWurdeGebraut)
        ErstelleZusammenfassung();
    else
        ErstelleSpickzettel();
}

void MainWindowImpl::LeseGeraetelisteDB(int id)
{
  QSqlQuery query;
  fuelleGeraeteliste = true;

  QString sql = "SELECT Bezeichnung FROM Geraete WHERE AusruestungAnlagenID = " + QString::number(id);
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int i=0;
    tableWidget_Geraete -> clearContents();
    tableWidget_Geraete -> setRowCount(0);
    while (query.next()){
      QTableWidgetItem *newItem1 = new QTableWidgetItem("");
      tableWidget_Geraete -> setRowCount(tableWidget_Geraete -> rowCount()+1);
      //Beschreibung
      newItem1 -> setText(query.value(0).toString());
      tableWidget_Geraete -> setItem(i, 0, newItem1);
      //Nächste Beschreibung aus Datei lesen
      i++;
    }
  }
  fuelleGeraeteliste = false;
}


void MainWindowImpl::SchreibeGeraetelisteDB(int id)
{
  if (AenderungGeraeteliste) {
    QSqlQuery query;
    //wenn ID = 0 ist ID ermitteln
    if (id == 0) {
      id = getBrauanlagenIDAusruestung();
    }

    //Zuerst alle Einträge in der Tabelle löschen
    QString sql = "DELETE FROM Geraete WHERE AusruestungAnlagenID = " + QString::number(id);
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }

    //Dann wieder mit den Tabellendaten füllen
    for (int i=0; i < tableWidget_Geraete -> rowCount(); i++){
      sql = "INSERT into Geraete(Bezeichnung, AusruestungAnlagenID) VALUES('"
          + tableWidget_Geraete -> item(i,0) -> text().replace("'","''") + "', "
          + QString::number(id) + ")";
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
    }
    AenderungGeraeteliste = false;
  }
}


void MainWindowImpl::slot_pushButton_GeraeteDel()
{
  tableWidget_Geraete -> removeRow(tableWidget_Geraete -> currentRow());
  AenderungGeraeteliste = true;
}


void MainWindowImpl::slot_pushButton_GeraeteNeu()
{
  QString s;
  s = "";
  QTableWidgetItem *newItem1 = new QTableWidgetItem("");
  int i = tableWidget_Geraete -> rowCount();
  tableWidget_Geraete -> setRowCount(i+1);
  //Beschreibung
  newItem1 -> setText(s);
  tableWidget_Geraete -> setItem(i, 0, newItem1);
  AenderungGeraeteliste = true;
}


void MainWindowImpl::BerBraudaten()
{
  if (BierWurdeGebraut){
    label_Stammwuerze -> setText(trUtf8("Stammwürze beim Anstellen"));
  }
  else {
    label_Stammwuerze -> setText(trUtf8("Stammwürze Soll nach dem Kochen"));
  }
  //Stammwüze setzten
  spinBox_SWSollKochen -> setValue(sw_kochen);
  spinBox_SWSollGesammt -> setValue(sw_gesammt);
  spinBox_SWEwzAnteil -> setValue(sw_ewz);

  //Anzeigefelder der stammwürze ausblenden wenn weitere zutaten keine rolle spielen
  if (sw_ewz == 0){
    widget_SWWeitereZutaten -> setVisible(false);
  }
  else {
    widget_SWWeitereZutaten -> setVisible(true);
  }

  double SWSchnellgaerprobe;
  if (checkBox_SchnellgaerprobeAktiv -> checkState() == Qt::Checked){
    SWSchnellgaerprobe = spinBox_SWSchnellgaerprobe -> value();
    widget_SWSchnellgaerprobe -> setVisible(true);
    toolBox_Gaerverlauf -> setItemEnabled(0,true);
    widget_Gruenschlauchzeitpunkt -> setVisible(true);
  }
  else {
    SWSchnellgaerprobe = spinBox_SWJungbier -> value();
    widget_SWSchnellgaerprobe -> setVisible(false);
    toolBox_Gaerverlauf -> setItemEnabled(0,false);
    widget_Gruenschlauchzeitpunkt -> setVisible(false);
  }

  //Anteil Stammwürze der weiteren Zutaten beim Kochen
  double sw_ewz_kochen = sw_ewz - (sw_gesammt - (sw_kochen/highGravityFaktor));

  //Grünschlauchzeitpunkt
  spinBox_Gruenschlauchzeitpunkt -> setValue(Berechnungen.GetGruenschlauchzeitpunkt(doubleSpinBox_CO2 -> value(),
                                                                                    spinBox_TemperaturJungbier -> value(), SWSchnellgaerprobe));

  //Erforderliche Speisemenge
  spinBox_SpeisemengeGesammt -> setValue(Berechnungen.GetSpeiseGesammt(spinBox_SWAnstellen -> value(),
                                                                       spinBox_SWJungbier -> value(), spinBox_JungbiermengeAbfuellen -> value() , spinBox_Speisemenge -> value(),
                                                                       checkBox_Spunden -> isChecked()));

  // Abgefuellte Biermenge
  spinBox_BiermengeAbfuellen -> setValue(spinBox_JungbiermengeAbfuellen -> value() + spinBox_SpeisemengeGesammt -> value() / 1000);

  //Erforderliche Zuckergabe gesammt
  spinBox_HaushaltszuckerGesammt -> setValue(Berechnungen.GetHaushaltszuckerGesammt());

  //Erforderliche Zuckergabe gesammt
  spinBox_TraubenzuckerGesammt -> setValue(Berechnungen.GetHaushaltszuckerGesammt()*1.16);

  //Erforderliche Speisemenge pro 0,5 Liter Flasche
  spinBox_Speisemenge500mlFlasche -> setValue(spinBox_SpeisemengeGesammt -> value() / spinBox_JungbiermengeAbfuellen -> value() / 2);

  //Erforderliche Zuckergabe pro 0,5 Liter Flasche
  spinBox_Haushaltszucker500mlFlasche -> setValue(spinBox_HaushaltszuckerGesammt -> value() / spinBox_JungbiermengeAbfuellen -> value() / 2);

  //Erforderliche Traubenzuckergabe pro 0,5 Liter Flasche
  spinBox_Traubenzucker500mlFlasche -> setValue(spinBox_HaushaltszuckerGesammt -> value() / spinBox_JungbiermengeAbfuellen -> value() / 2 * 1.16);

  //Erforderliche Speisemenge pro 0,33 Liter Flasche
  spinBox_Speisemenge333mlFlasche -> setValue(spinBox_SpeisemengeGesammt -> value() / spinBox_JungbiermengeAbfuellen -> value() / 3);

  //Erforderliche Zuckergabe pro 0,33 Liter Flasche
  spinBox_Haushaltszucker333mlFlasche -> setValue(spinBox_HaushaltszuckerGesammt -> value() / spinBox_JungbiermengeAbfuellen -> value() / 3);

  //Erforderliche Zuckergabe pro 0,33 Liter Flasche
  spinBox_Traubenzucker333mlFlasche -> setValue(spinBox_HaushaltszuckerGesammt -> value() / spinBox_JungbiermengeAbfuellen -> value() / 3 * 1.16);

  //Alkoholgehalt
  double sw = spinBox_SWAnstellen -> value();
  sw = sw + sw_ewz_gaerung;
  sw += (spinBox_HaushaltszuckerGesammt -> value() / 10) / spinBox_WuerzemengeAnstellen -> value();
  spinBox_AlkoholVol -> setValue(Berechnungen.BerAlkohoVol(sw,SWSchnellgaerprobe));

  //Scheinbarer EVG
  sw = spinBox_SWAnstellen -> value();
  spinBox_SEVG -> setValue(Berechnungen.GetScheinbarerEVG(sw,SWSchnellgaerprobe));
  //Tatsächlicher EVG
  spinBox_TEVG -> setValue(Berechnungen.GetTatsaechlicherEVG(sw,SWSchnellgaerprobe));

  //Wassermenge zur verschneidung
  spinBox_WasserVerschneidung -> setValue(Berechnungen.GetWasserVerschneidung(spinBox_SWSollKochen->value()/highGravityFaktor
                                                                              , spinBox_SWKochende -> value(), spinBox_WuerzemengeKochende -> value()));

  //Spundungsdruck
  spinBox_Spundungsdruck -> setValue(Berechnungen.GetSpundungsdruck());

  //Effektive Ausbeute zur berechnung der Schüttung
  //qDebug() << "sw_ewz_kochen:" << sw_ewz_kochen;
  spinBox_AusbeuteEffektiv2 -> setValue( Berechnungen.GetSudhausausbeute(spinBox_SWAnstellen -> value()*highGravityFaktor - sw_ewz_kochen
                                                                         , (spinBox_WuerzemengeAnstellen -> value() + spinBox_Speisemenge -> value())/highGravityFaktor
                                                                         , doubleSpinBox_S_Gesammt -> value() ) );

  //Sudhausausbeute
  spinBox_Sudhausausbeute -> setValue( Berechnungen.GetSudhausausbeute(spinBox_SWKochende -> value() - sw_ewz_kochen
                                                                       , spinBox_WuerzemengeVorHopfenseihen -> value()
                                                                       , doubleSpinBox_S_Gesammt -> value() ) );

  //Würzemenge bei Kochbeginn
  spinBox_MengeSollKochbegin->setValue(doubleSpinBox_VolumenPfannevoll->value());
  //Füllhöhe von unten
  double Grundflaeche = pow(getSudpfanneDurchmesser()/2, 2) * M_PI ;
  spinBox_MengeSollcmVomBoden -> setValue(spinBox_MengeSollKochbegin -> value() * 1000 / Grundflaeche);
  //Füllhöhe von oben
  spinBox_MengeSollcmVonOben -> setValue(getSudpfanneHoehe() - spinBox_MengeSollcmVomBoden -> value());
  //Stamwürze bei Kochbeginn
  double swvsoll = spinBox_SWSollKochen->value();
  double mengeKochende100grad = Berechnungen.BerVolumenWasser(20,99,spinBox_Menge->value());
  swvsoll = swvsoll/doubleSpinBox_VolumenPfannevoll->value()*mengeKochende100grad;
  spinBox_SWSollKochbegin->setValue(swvsoll);
  //Menge nach Kochende Heiss
  spinBox_MengeSollNachKochende100grad->setValue(mengeKochende100grad);
}

void MainWindowImpl::slot_pushButton_gebraut()
{
  setAenderung(true);

  //Merker setzten das nicht neu Berechnet werden soll
  NichtBerechnen = true;

  // Das Rezept als gebraut Markieren
  BierWurdeGebraut = true;

  // Eingabefelder Disablen
  SetStatusGebraut(true);

  //Abfrage ob Rohstoffe vom Bestand abgezogen werden sollen
  QMessageBox msgBox;
  msgBox.setWindowTitle("kleine-frage");
  msgBox.setInformativeText("");
  msgBox.setText(trUtf8("Sollen die verwendeten Rohstoffe vom Bestand abgezogen werden?\n\nHinweis: Zutaten, die bei der Gärung hinzugegeben werden, werden jetzt nicht verrechnet."));
  //msgBox.setInformativeText(trUtf8("Sollen die verwendeten Rohstoffe vom Bestand abgezogen werden?"));
  msgBox.setIcon(QMessageBox::Question);
  //msgBox.setDefaultButton(QMessageBox::Save);
  QPushButton *JaButton = msgBox.addButton(trUtf8("Ja"), QMessageBox::ActionRole);
  msgBox.addButton(trUtf8("Nein"), QMessageBox::ActionRole);

  msgBox.exec();

  if (msgBox.clickedButton() == JaButton){
    //verwendete Rohstoffe vom Bestand abziehen
    RohstoffeAbziehen();
  }

  //Schnellgärverlauf und Hauptgärverlauf den ersetn Datensatz hinzufügen
  dateTimeEdit_Schnellgaerprobe -> setDateTime(QDateTime::currentDateTime());
  dateTimeEdit_Hautgaerprobe -> setDateTime(QDateTime::currentDateTime());
  doubleSpinBox_SWSchnellgaerprobe -> setValue(spinBox_SWAnstellen -> value()+sw_ewz_gaerung);
  doubleSpinBox_SWHauptgaerprobe -> setValue(spinBox_SWAnstellen -> value()+sw_ewz_gaerung);
  doubleSpinBox_TempSchnellgaerprobe -> setValue(20);
  doubleSpinBox_TempHauptgaerprobe -> setValue(20);
  on_pushButton_AddSchnellgaerMessung_clicked();
  on_pushButton_AddHauptgaerMessung_clicked();

  //den Weiteren Zutaten mitteilen das das Bier gebraut wurde
  for (int i=0; i < list_EwZutat.count(); i++){
    list_EwZutat[i] -> setBierWurdeGebraut(true);
  }
  NichtBerechnen = false;
  BerAlles();

}


void MainWindowImpl::SetStatusGebraut(bool status)
{
  //Wenn Bier schon Gebraut wurde dann wird Rezeptseite ReadOnly gesetzt
  QAbstractSpinBox::ButtonSymbols bs;

  if (!status) {
    bs = QAbstractSpinBox::UpDownArrows;
  }
  else {
    bs = QAbstractSpinBox::NoButtons;
  }

  pushButton_RohstoffeAbziehen -> setDisabled(status);
  pushButton_SudAbgefuellt->setEnabled(status && !BierWurdeAbgefuellt);

  //lineEdit_Sudname -> setDisabled(status);
  spinBox_Menge -> setReadOnly(status);
  spinBox_Menge -> setButtonSymbols(bs);

  spinBox_SW -> setReadOnly(status);
  spinBox_SW -> setButtonSymbols(bs);

  spinBox_IBU -> setReadOnly(status);
  spinBox_IBU -> setButtonSymbols(bs);

  spinBox_EinmaischenTemp -> setReadOnly(status);
  spinBox_EinmaischenTemp -> setButtonSymbols(bs);
  pushButton_CalcEinmaischeTemp->setDisabled(status);

  spinBox_Gesammtkochdauer -> setReadOnly(status);
  spinBox_Gesammtkochdauer -> setButtonSymbols(bs);

  spinBox_NachisomerisierungsZeit -> setReadOnly(status);
  spinBox_NachisomerisierungsZeit -> setButtonSymbols(bs);

  spinBox_High_Gravity->setReadOnly(status);
  spinBox_High_Gravity->setButtonSymbols(bs);

  comboBox_AuswahlHefe -> setDisabled(status);
  comboBox_AuswahlHefe -> setEditable(status);

  comboBox_AuswahlBrauanlage->setDisabled(status);
  comboBox_AuswahlBrauanlage->setEditable(status);

  spinBox_AnzahlHefeEinheiten -> setReadOnly(status);
  spinBox_AnzahlHefeEinheiten -> setButtonSymbols(bs);
  widget_empfohleneHefeEinheiten->setVisible(!status);

  doubleSpinBox_FaktorHGEmpfehlung -> setVisible(!status);

  label_FaktorEmpfehlung -> setVisible(!status);

  //Rasten auf inaktiv setzten
  for (int i=0; i < list_Rasten.count(); i++){
    list_Rasten[i]->setDisabled(status);
  }
  pushButton_NeueRast->setVisible(!status);

  doubleSpinBox_FaktorHG -> setReadOnly(status);
  doubleSpinBox_FaktorHG -> setButtonSymbols(bs);

  //Malzgaben
  for (int i=0; i < list_Malzgaben.count(); i++){
    list_Malzgaben[i] -> setDisabled(status);
  }
  pushButton_MalzHinzufuegen -> setVisible(!status);

  //Hopfengaben
  for (int i=0; i < list_Hopfengaben.count(); i++){
    list_Hopfengaben[i] -> setDisabled(status);
  }
  pushButton_HopfenHinzufuegen -> setVisible(!status);
  comboBox_BerechnungsArtHopfen->setDisabled(status);
  comboBox_BerechnungsArtHopfen->setEditable(status);

  //Tab Wasser
  SpinBox_waSollRestalkalitaet_dh -> setReadOnly(status);
  SpinBox_waSollRestalkalitaet_dh -> setButtonSymbols(bs);

  //Tab Braudaten
  dateEdit_Braudatum -> setDisabled(status);
  pushButton_RohstoffeAbziehen -> setDisabled(status);

  spinBox_WuerzemengeKochende -> setReadOnly(status);
  spinBox_WuerzemengeKochende -> setButtonSymbols(bs);

  spinBox_SWKochende -> setReadOnly(status);
  spinBox_SWKochende -> setButtonSymbols(bs);

  dateEdit_Anstelldatum -> setDisabled(status);

  spinBox_WuerzemengeAnstellen -> setReadOnly(status);
  spinBox_WuerzemengeAnstellen -> setButtonSymbols(bs);

  spinBox_SWAnstellen -> setReadOnly(status);
  spinBox_SWAnstellen -> setButtonSymbols(bs);

  pushButton_BraudatumHeute -> setDisabled(status);
  pushButton_EingabeHSWKochende -> setDisabled(status);
  pushButton_AnstelldatumHeute -> setDisabled(status);
  pushButton_EingabeHSWAnstellen -> setDisabled(status);

  spinBox_WuerzemengeVorHopfenseihen -> setReadOnly(status);
  spinBox_WuerzemengeVorHopfenseihen -> setButtonSymbols(bs);

  spinBox_SWVorHopfenseihen -> setReadOnly(status);
  spinBox_SWVorHopfenseihen -> setButtonSymbols(bs);

  pushButton_EingabeHSWVorHopfenseihen -> setDisabled(status);
  pushButton_EingabeHMengeVHopfenseihen -> setDisabled(status);
  pushButton_EingabeHMengeNHopfenseihen -> setDisabled(status);

  groupBox_Verschneidung -> setVisible(!status);

  tabWidged->setTabText(tabWidged->indexOf(tab_Spickzettel), status ? trUtf8("Zusammenfassung") : trUtf8("Spickzettel"));

  //Tab Gärdaten ausblenden
  tab_Gaerverlauf->setEnabled(status);
  tabWidged->setTabEnabled(tabWidged->indexOf(tab_Gaerverlauf), status);
}


void MainWindowImpl::slot_pushButton_BraudatumHeute()
{
  dateEdit_Braudatum -> setDate(QDate::currentDate());
  setAenderung(true);
}


void MainWindowImpl::slot_pushButton_AnstelldatumHeute()
{
  dateEdit_Anstelldatum -> setDate(QDate::currentDate());
  setAenderung(true);
}


void MainWindowImpl::slot_pushButton_AbfuelldatumHeute()
{
  dateEdit_Abfuelldatum -> setDate(QDate::currentDate());
  setAenderung(true);
}


void MainWindowImpl::slot_pushButton_EingabeHSWKochende()
{
  KorrektSwImpl ksw;
  ksw.spinBox_SwPlato -> setValue(spinBox_SWKochende -> value());
  ksw.BerDichte();
  ksw.exec();
  if (!ksw.abgebrochen){
    spinBox_SWKochende -> setValue(Berechnungen.densityAtX(ksw.spinBox_SwPlato -> value(), ksw.spinBox_Temperatur -> value(), 20));
  }
  setAenderung(true);
}


void MainWindowImpl::slot_pushButton_EingabeHSWAnstellen()
{
  KorrektSwImpl ksw;
  ksw.spinBox_SwPlato -> setValue(spinBox_SWAnstellen -> value());
  ksw.BerDichte();
  ksw.exec();
  if (!ksw.abgebrochen){
    spinBox_SWAnstellen -> setValue(Berechnungen.densityAtX(ksw.spinBox_SwPlato -> value(), ksw.spinBox_Temperatur -> value(), 20));
  }
  setAenderung(true);
}


void MainWindowImpl::slot_pushButton_EingabeHSWSchnellgaerprobe()
{
  KorrektSwImpl ksw;
  ksw.spinBox_SwPlato -> setValue(spinBox_SWSchnellgaerprobe -> value());
  ksw.setSWAnstellen(spinBox_SWAnstellen -> value());
  ksw.BerDichte();
  ksw.exec();
  if (!ksw.abgebrochen){
    if (ksw.spinBox_SwBrix -> value() == 0){
      spinBox_SWSchnellgaerprobe -> setValue(Berechnungen.densityAtX(ksw.spinBox_SwPlato -> value(), ksw.spinBox_Temperatur -> value(), 20));
    }
    else {
      spinBox_SWSchnellgaerprobe -> setValue(ksw.spinBox_SwPlato -> value());
    }
  }
  ksw.setSWAnstellen(0);
  setAenderung(true);
}


void MainWindowImpl::slot_pushButton_EingabeHSWJungbier()
{
  KorrektSwImpl ksw;
  ksw.spinBox_SwPlato -> setValue(spinBox_SWJungbier -> value());
  ksw.setSWAnstellen(spinBox_SWAnstellen -> value());
  ksw.BerDichte();
  ksw.exec();
  if (!ksw.abgebrochen){
    if (ksw.spinBox_SwBrix -> value() == 0){
      spinBox_SWJungbier -> setValue(Berechnungen.densityAtX(ksw.spinBox_SwPlato -> value(), ksw.spinBox_Temperatur -> value(), 20));
    }
    else {
      spinBox_SWJungbier -> setValue(ksw.spinBox_SwPlato -> value());
    }
  }
  ksw.setSWAnstellen(0);
  setAenderung(true);
}


void MainWindowImpl::RohstoffeAbziehen()
{
  // Verwendetes Malz vom Bestand abziehen
  QString s;
  double d;

  //Ausgewähltes Malz im Bestand suchen
  for (int i=0; i < list_Malzgaben.count(); i++){
    s = list_Malzgaben[i]->getName();
    int AnzahlMalzEintraege = tableWidget_Malz -> rowCount();
    for (int o=0; o < AnzahlMalzEintraege; o++){
      //wenn Eintrag übereinstimmt
      if (tableWidget_Malz -> item(o,TableMalzColName) -> text() == s){
        QDoubleSpinBox* dsbMenge = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(o,TableMalzColMenge);
        d = dsbMenge ->value();
        d -= list_Malzgaben[i]->getErgMenge();
        if (d < 0)
          d = 0;
        dsbMenge ->setValue(d);
      }
    }
  }

  //Hopfen
  for (int i=0; i < list_Hopfengaben.count(); i++){
    s = list_Hopfengaben[i]->getName();
    int AnzahlHopfenEintraege = tableWidget_Hopfen -> rowCount();
    for (int o=0; o < AnzahlHopfenEintraege; o++){
      //wenn Eintrag übereinstimmt
      if (tableWidget_Hopfen -> item(o,TableHopfenColName) -> text() == s){
        QDoubleSpinBox* dsbMenge = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(o,TableHopfenColMenge);
        d = dsbMenge ->value();
        d -= list_Hopfengaben[i]->getErgMenge();
        if (d < 0)
          d = 0;
        dsbMenge ->setValue(d);
      }
    }
  }

  //Hefe
  s = comboBox_AuswahlHefe -> currentText();
  int AnzahlHefeEintraege = tableWidget_Hefe -> rowCount();
  for (int i=0; i < AnzahlHefeEintraege; i++){
    //wenn Eintrag übereinstimmt
    if (tableWidget_Hefe -> item(i,TableHefeColName) -> text() == s){
      QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,TableHefeColMenge);
      d = spinBox->value();
      d -= spinBox_AnzahlHefeEinheiten->value();

      if (d < 0)
        d = 0;
      spinBox->setValue(d);
    }
  }

  //Weitere Zutaten vom Bestand abziehen
  for (int i=0; i < list_EwZutat.count(); i++){
    //nur vom bestand abziehen wenn zugabezeitpunkt nicht bei der gärung ist
    if (list_EwZutat[i]->getZeitpunkt() != EWZ_Zeitpunkt_Gaerung) {
      //wenn Weiter Zutat kein Hopfen ist
      if (list_EwZutat[i]->getTyp() != EWZ_Typ_Hopfen){
        s = list_EwZutat[i]->getName();
        for (int o=0; o < tableWidget_WeitereZutaten -> rowCount(); o++){
          //wenn eintrag übereinstimmt
          if (tableWidget_WeitereZutaten -> item(o,TableWZutatColName) -> text() == s){
            QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,TableWZutatColMenge);
            if (list_EwZutat[i]->getEinheit() == EWZ_Einheit_Kg)
              dsbMenge->setValue(dsbMenge->value()-(list_EwZutat[i]->getErg_Menge()/1000));
            else
              dsbMenge->setValue(dsbMenge->value()-list_EwZutat[i]->getErg_Menge());
          }
        }
      }
      //Wenn Hopfen
      else {
        s = list_EwZutat[i]->getName();
        int AnzahlHopfenEintraege = tableWidget_Hopfen -> rowCount();
        for (int o=0; o < AnzahlHopfenEintraege; o++){
          //wenn Eintrag übereinstimmt
          if (tableWidget_Hopfen -> item(o,TableHopfenColName) -> text() == s){
            QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(o,TableHopfenColMenge);
            d = spinBox->value();
            d -= list_EwZutat[i]->getErg_Menge();
            if (d < 0)
              d = 0;
            spinBox->setValue(d);
          }
        }
      }
    }
  }
}


void MainWindowImpl::slot_pushButton_SudAbgefuellt()
{
  if (BierWurdeGebraut) {
    //überprüfen ob noch zutaten für die gärung nicht zugegeben oder noch nicht entnommen wurden
    bool gefunden = false;
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i]->getZeitpunkt() == EWZ_Zeitpunkt_Gaerung) {
        if (list_EwZutat[i]->getZugabestatus() == EWZ_Zugabestatus_nichtZugegeben) {
          gefunden = true;
          i = list_EwZutat.count();
        }
        else if (list_EwZutat[i]->getZugabestatus() == EWZ_Zugabestatus_Zugegeben && list_EwZutat[i]->getEntnahmeindex() == EWZ_Entnahmeindex_MitEntnahme) {
          gefunden = true;
          i = list_EwZutat.count();
        }
      }
    }
    //Es sind noch zutaen vorhanden wo der zugabe oder entnahmezustand nicht definiert wurden
    if (gefunden) {
      QMessageBox msgBox;
      msgBox.setWindowTitle("Hinweis");
      msgBox.setInformativeText("");
      msgBox.setText(trUtf8("Das Bier kann noch nicht als abgefüllt Markiert werden da der Sud Zutaten für die Gärung enthält die noch nicht als zugegeben oder entnommen Markiert wurden."));
      msgBox.setInformativeText(trUtf8("Bitte die Zutaten entfernen oder den Status noch entsprechend setzten"));
      msgBox.setIcon(QMessageBox::Information);
      //msgBox.setDefaultButton(QMessageBox::Save);

      msgBox.exec();
      tabWidged -> setCurrentWidget(tab_Rezept);
      TabWidget_Zutaten -> setCurrentWidget(tab_WeitereZutaten);
    }
    //Bier kann als Abgefüllt markiert werden
    else {
      setAenderung(true);

      // Das Rezept als gebraut Markieren
      BierWurdeAbgefuellt = true;

      //den Weiteren Zutaten mitteilen das das Bier Abgefüllt wurde
      for (int i=0; i < list_EwZutat.count(); i++){
        list_EwZutat[i] -> setBierWurdeAbgefuellt(true);
      }

      // Eingabefelder Disablen
      SetDisabledAbgefuellt(true);

      //Ersten Eintrag in Sudverlauf Nachgärung eintragen
      dateTimeEdit_Nachgaerprobe -> setDateTime(QDateTime::currentDateTime());
      doubleSpinBox_Nachgaerdruck -> setValue(0);
      doubleSpinBox_Nachgaertemp -> setValue(spinBox_TemperaturJungbier -> value());
      on_pushButton_AddNachgaerMessung_clicked();
      pushButton_SudVerbraucht -> setDisabled(false);
    }
  }
}


void MainWindowImpl::SetDisabledAbgefuellt(bool status)
{
  QAbstractSpinBox::ButtonSymbols bs;

  if (!status) {
    bs = QAbstractSpinBox::UpDownArrows;
  }
  else {
    bs = QAbstractSpinBox::NoButtons;
  }

  pushButton_SudAbgefuellt->setDisabled(status || !BierWurdeGebraut);

  doubleSpinBox_CO2 -> setReadOnly(status);
  doubleSpinBox_CO2 -> setButtonSymbols(bs);

  dateEdit_Abfuelldatum -> setDisabled(status);

  pushButton_AbfuelldatumHeute -> setDisabled(status);

  spinBox_SWSchnellgaerprobe -> setReadOnly(status);
  spinBox_SWSchnellgaerprobe -> setButtonSymbols(bs);

  pushButton_EingabeHSWSchnellgaerprobe -> setDisabled(status);

  spinBox_SWJungbier -> setReadOnly(status);
  spinBox_SWJungbier -> setButtonSymbols(bs);

  pushButton_EingabeHSWJungbier -> setDisabled(status);

  spinBox_TemperaturJungbier -> setReadOnly(status);
  spinBox_TemperaturJungbier -> setButtonSymbols(bs);

  spinBox_Speisemenge -> setReadOnly(status);
  spinBox_Speisemenge -> setButtonSymbols(bs);

  checkBox_SchnellgaerprobeAktiv -> setDisabled(status);
  checkBox_Spunden -> setDisabled(status);

  spinBox_JungbiermengeAbfuellen -> setReadOnly(status);
  spinBox_JungbiermengeAbfuellen -> setButtonSymbols(bs);

  pushButton_EWZ_Hinzufuegen -> setVisible(!status);

  //Tab Bewertung ausblenden
  tab_Bewertung->setEnabled(status);
  tabWidged->setTabEnabled(tabWidged->indexOf(tab_Bewertung), status);
}

void MainWindowImpl::SetDisabledVerbraucht(bool status)
{
    pushButton_SudVerbraucht->setDisabled(status);
}

void MainWindowImpl::slot_Changed()
{
  //BerAlles();
  if (Gestartet) {
    setAenderung(true);
  }
}


void MainWindowImpl::CheckFehler()
{
  // Überprüfen ob genügend Rohstoffe zur verfügung stehen
  CheckRohstoffeVorhanden();

  // Überprüfen ob Jungbierstammwürze schon im Grünen bereich ist
  CheckJungbierSW();

  //Überprüfen ob Gesamtmaischemenge kapazität überschreitet
  CheckGesammtMaischeMenge();

  //Überprüfen ob Sudpfanne kapazität überschreitet
  CheckPfannevoll();
}


void MainWindowImpl::CheckRohstoffeVorhanden()
{
  double soll = 0;
  double maxprozent = 0;
  double istprozent = 0;
  double ist = 0;
  QString s;

  //Zu wenig Malz vorhanden
  for (int i=0; i < list_Malzgaben.count(); i++){
    s = list_Malzgaben[i] -> getName();
    soll = list_Malzgaben[i] -> getErgMenge();
    int id = list_Malzgaben[i] -> getID();
    //Nun überprüfen ob die zutat noch einmal vorkommt
    for (int o=0; o < list_Malzgaben.count(); o++){
      if (list_Malzgaben[o] -> getName() ==  s) {
        //wenn der Eintrag der gleiche ist dann natürlich nicht berücksichtigen
        if (list_Malzgaben[o] -> getID() != id){
          soll += list_Malzgaben[o] -> getErgMenge();
        }
      }
    }
    //Wenn angehakt Sude aus der Merkliste abfragen und überprüfen ob da dieser Rohstoff auch verwendet wird.
    if (checkBox_MerklisteMengen->isChecked()) {
      QSqlQuery query;
      QString sql;
      sql = "SELECT ID FROM 'Sud' WHERE MerklistenID=1";
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
      else {
        while (query.next()){
          //ID
          int FeldNr = query.record().indexOf("ID");
          int SudID = query.value(FeldNr).toInt();
          //Aktuellen Sud ignorieren
          if (SudID != AktuelleSudID) {
            QSqlQuery query2;
            //Malzeintrag abfragen
            QString sql2 = "SELECT * FROM 'Malzschuettung' WHERE SudID="+QString::number(SudID)+" AND Name='"+s+"'";
            if (!query2.exec(sql2)) {
              // Fehlermeldung Datenbankabfrage
              ErrorMessage *errorMessage = new ErrorMessage();
              errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                          CANCEL_NO, trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
                                          + trUtf8("\nSQL Befehl:\n") + sql2);
            }
            else {
              while (query2.next()){
                FeldNr = query2.record().indexOf("erg_Menge");
                soll += query2.value(FeldNr).toDouble();
              }
            }
          }
        }
      }
    }
    //Menge die zur verfügung steht
    int Anzahl = tableWidget_Malz -> rowCount();
    bool gefunden = false;
    for (int o=0; o < Anzahl; o++){
      //Wenn Eintrag übereinstimmt
      if (tableWidget_Malz -> item(o,TableMalzColName) -> text() == s){
        gefunden = true;
        QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(o,TableMalzColMenge);
        ist = dsbMenge -> value();
        QDoubleSpinBox* dsbMaxprozent = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(o,TableMalzColMaxSchuettung);
        maxprozent = dsbMaxprozent ->value();
      }
    }
    if (!gefunden){
      ist = 0;
    }
    list_Malzgaben[i] -> ergWidget->setRest(ist - soll);
    //Anzeige Einfärben wenn Rohstoff nicht vorrätig wäre
    if (soll > ist){
      QString sf;
      sf = QString::number(soll - ist) + trUtf8(" kg zu wenig vorhanden");
      list_Malzgaben[i] -> ergWidget -> setToolTip(sf);
      list_Malzgaben[i] -> ergWidget -> icon_achtung -> setVisible(true);
      list_Malzgaben[i] -> ergWidget -> icon_warnung -> setVisible(false);
    }
    else {
      istprozent = list_Malzgaben[i]->getMengeProzent();
      //Überprüfen ob Maximaler Schüttungsanteil überschritten wurde
      if (maxprozent == 0){
        maxprozent = 100;
      }
      if (istprozent > maxprozent){
        QString sf = trUtf8("Maximal empfohlener Schüttungsanteil (") + QString::number(maxprozent) + trUtf8("%) wurde überschritten");
        list_Malzgaben[i] -> ergWidget -> setToolTip(sf);
        list_Malzgaben[i] -> ergWidget -> icon_warnung -> setVisible(true);
        list_Malzgaben[i] -> ergWidget -> icon_achtung -> setVisible(false);
      }
      else {
        list_Malzgaben[i] -> ergWidget -> setToolTip(trUtf8("Rest ") + QString::number(ist - soll) + "kg");
        list_Malzgaben[i] -> ergWidget -> icon_achtung -> setVisible(false);
        list_Malzgaben[i] -> ergWidget -> icon_warnung -> setVisible(false);
      }
    }
  }


  //Hopfen
  //Menge die zur verfügung steht
  int AnzahlHopfenEintraege = tableWidget_Hopfen -> rowCount();
  for (int i=0; i < list_Hopfengaben.count(); i++){
    s = list_Hopfengaben[i] -> getName();
    soll = list_Hopfengaben[i] -> getErgMenge();
    int id = list_Hopfengaben[i] -> getID();
    //Nun überprüfen ob die zutat noch einmal vorkommt
    for (int o=0; o < list_Hopfengaben.count(); o++){
      if (list_Hopfengaben[o] -> getName() ==  s) {
        //wenn der Eintrag der gleiche ist dann natürlich nicht berücksichtigen
        if (list_Hopfengaben[o] -> getID() != id){
          soll += list_Hopfengaben[o] -> getErgMenge();
        }
      }
    }
    //überprüfen ob in den Weiterten zutaten der Gleiche Hopfen verwendet wird
    for (int o=0; o < list_EwZutat.count(); o++){
      if ((list_EwZutat[o] -> getName() ==  s) && (list_EwZutat[o] -> getTyp() == EWZ_Typ_Hopfen)) {
        soll += list_EwZutat[o] -> getErg_Menge();
      }
    }
    //Wenn angehakt Sude aus der Merkliste abfragen und überprüfen ob da dieser Rohstoff auch verwendet wird.
    if (checkBox_MerklisteMengen->isChecked()) {
      QSqlQuery query;
      QString sql;
      sql = "SELECT ID FROM 'Sud' WHERE MerklistenID=1";
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
      else {
        while (query.next()){
          //ID
          int FeldNr = query.record().indexOf("ID");
          int SudID = query.value(FeldNr).toInt();
          //Aktuellen Sud ignorieren
          if (SudID != AktuelleSudID) {
            QSqlQuery query2;
            //Rohstoffeintrag abfragen
            QString sql2 = "SELECT * FROM 'Hopfengaben' WHERE SudID="+QString::number(SudID)+" AND Aktiv=1 AND Name='"+s+"'";
            if (!query2.exec(sql2)) {
              // Fehlermeldung Datenbankabfrage
              ErrorMessage *errorMessage = new ErrorMessage();
              errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                          CANCEL_NO, trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
                                          + trUtf8("\nSQL Befehl:\n") + sql2);
            }
            else {
              while (query2.next()){
                FeldNr = query2.record().indexOf("erg_Menge");
                soll += query2.value(FeldNr).toDouble();
              }
            }
            //Weitere Zutaten auch noch abfragen
            sql2 = "SELECT * FROM 'WeitereZutatenGaben' WHERE SudID="+QString::number(SudID)+" AND Name='"+s+"'";
            if (!query2.exec(sql2)) {
              // Fehlermeldung Datenbankabfrage
              ErrorMessage *errorMessage = new ErrorMessage();
              errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                          CANCEL_NO, trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
                                          + trUtf8("\nSQL Befehl:\n") + sql2);
            }
            else {
              while (query2.next()){
                FeldNr = query2.record().indexOf("erg_Menge");
                soll += query2.value(FeldNr).toDouble();
              }
            }
          }
        }
      }
    }
    //zur verfügungstehende Hopfenmenge
    for (int o=0; o < AnzahlHopfenEintraege; o++){
      //wenn Eintrag übereinstimmt
      if (tableWidget_Hopfen -> item(o,TableHopfenColName) -> text() == s){
        QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(o,TableHopfenColMenge);
        ist = spinBox->value();
      }
    }
    list_Hopfengaben[i] -> ergWidget->setRest(ist - soll);
    //Anzeige Einfärben wenn Rohstoff nicht vorrätig wäre
    if (soll > ist){
      QString sf = QString::number(soll - ist) + trUtf8(" g zu wenig Hopfen vorhanden");
      list_Hopfengaben[i] -> ergWidget -> setToolTip(sf);
      list_Hopfengaben[i] -> ergWidget -> icon_achtung -> setVisible(true);
    }
    else {
      list_Hopfengaben[i] -> ergWidget -> setToolTip(trUtf8("Rest ") + QString::number(ist - soll) + "g");
      list_Hopfengaben[i] -> ergWidget -> icon_achtung -> setVisible(false);
    }
  }

  //Hefe

  //Hefeauswahl
  int AnzahlHefeEintraege = tableWidget_Hefe -> rowCount();
  s = comboBox_AuswahlHefe -> currentText();
  if (s != ""){
    //zur verfügungstehende Malzmenge
    for (int i=0; i < AnzahlHefeEintraege; i++){
      //wenn Eintrag übereinstimmt
      if (tableWidget_Hefe -> item(i,TableHefeColName) -> text() == s){
        QSpinBox *spinBox = (QSpinBox*)tableWidget_Hefe -> cellWidget(i,TableHefeColMenge);
        ist = spinBox->value();
        //benötigte Hefemenge
        soll = spinBox_AnzahlHefeEinheiten->value();
      }
    }
    //Wenn angehakt Sude aus der Merkliste abfragen und überprüfen ob da dieser Rohstoff auch verwendet wird.
    if (checkBox_MerklisteMengen->isChecked()) {
      QSqlQuery query;
      QString sql;
      sql = "SELECT * FROM 'Sud' WHERE MerklistenID=1 AND AuswahlHefe='"+s+"'";
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
      else {
        while (query.next()){
          //ID
          int FeldNr = query.record().indexOf("ID");
          int SudID = query.value(FeldNr).toInt();
          //Aktuellen Sud ignorieren
          if (SudID != AktuelleSudID) {
            FeldNr = query.record().indexOf("HefeAnzahlEinheiten");
            soll += query.value(FeldNr).toDouble();
          }
        }
      }
    }
    //Anzeige Einfärben wenn Rohstoff nicht vorrätig wäre
    if (soll > ist){
      QString sf = QString::number(soll - ist) + trUtf8(" Einheiten zu wenig Hefe vorhanden");
      widget_AnzahlHefeEinheiten->setToolTip(sf);
      icon_Hefe_achtung->setVisible(true);
    }
    else {
      widget_AnzahlHefeEinheiten->setToolTip(trUtf8("Rest ") + QString::number(ist - soll) + trUtf8("Einheiten"));
      icon_Hefe_achtung->setVisible(false);
    }
  }

  //Alle Weitere Zutaten durchgehen
  for (int i=0; i < list_EwZutat.count(); i++){
    s = list_EwZutat[i] -> getName();
    //Wenn Typ = Hopfen ist
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Hopfen){
      soll = list_EwZutat[i] -> getErg_Menge();
      int id = list_EwZutat[i] -> getID();
      //Nun überprüfen ob der Hopfen in den Weiteren Zutaten noch einmal vorkommt
      for (int o=0; o < list_EwZutat.count(); o++){
        if ((list_EwZutat[o] -> getName() ==  s) && (list_EwZutat[o] -> getTyp() == EWZ_Typ_Hopfen)) {
          //wenn der Eintrag der gleiche ist dann natürlich nicht berücksichtigen
          if (list_EwZutat[o] -> getID() != id){
            soll += list_EwZutat[o] -> getErg_Menge();
          }
        }
      }
      //Überprüfen ob in Kochhopfengabe der Hopfen verwendet wird
      for (int o=0; o < list_Hopfengaben.count(); o++){
        if (s == list_Hopfengaben[o]->getName()){
          soll += list_Hopfengaben[o]->getErgMenge();
        }
      }

      //Wenn angehakt Sude aus der Merkliste abfragen und überprüfen ob da dieser Rohstoff auch verwendet wird.
      if (checkBox_MerklisteMengen->isChecked()) {
        QSqlQuery query;
        QString sql;
        sql = "SELECT ID FROM 'Sud' WHERE MerklistenID=1";
        if (!query.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                      + trUtf8("\nSQL Befehl:\n") + sql);
        }
        else {
          while (query.next()){
            //ID
            int FeldNr = query.record().indexOf("ID");
            int SudID = query.value(FeldNr).toInt();
            //Aktuellen Sud ignorieren
            if (SudID != AktuelleSudID) {
              QSqlQuery query2;
              //Rohstoffeintrag abfragen
              QString sql2 = "SELECT * FROM 'Hopfengaben' WHERE SudID="+QString::number(SudID)+" AND Aktiv=1 AND Name='"+s+"'";
              if (!query2.exec(sql2)) {
                // Fehlermeldung Datenbankabfrage
                ErrorMessage *errorMessage = new ErrorMessage();
                errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                            CANCEL_NO, trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
                                            + trUtf8("\nSQL Befehl:\n") + sql2);
              }
              else {
                while (query2.next()){
                  FeldNr = query2.record().indexOf("erg_Menge");
                  soll += query2.value(FeldNr).toDouble();
                }
              }
              //Weitere Zutaten auch noch abfragen
              sql2 = "SELECT * FROM 'WeitereZutatenGaben' WHERE SudID="+QString::number(SudID)+" AND Name='"+s+"'";
              if (!query2.exec(sql2)) {
                // Fehlermeldung Datenbankabfrage
                ErrorMessage *errorMessage = new ErrorMessage();
                errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                            CANCEL_NO, trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
                                            + trUtf8("\nSQL Befehl:\n") + sql2);
              }
              else {
                while (query2.next()){
                  FeldNr = query2.record().indexOf("erg_Menge");
                  soll += query2.value(FeldNr).toDouble();
                }
              }
            }
          }
        }
      }
      //Menge die zur verfügung steht
      for (int o=0; o < AnzahlHopfenEintraege; o++){
        //Wenn Eintrag übereinstimmt
        if (tableWidget_Hopfen -> item(o,TableHopfenColName) -> text() == s){
          QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(o,TableHopfenColMenge);
          ist = spinBox->value();
        }
      }
      list_EwZutat[i] -> ergWidget->setRest(ist - soll);
      //Anzeige Einfärben wenn Rohstoff nicht vorrätig wäre
      if (soll > ist){
        QString sf = QString::number(soll - ist) + trUtf8(" g zu wenig Hopfen vorhanden");
        list_EwZutat[i] -> ergWidget -> setToolTip(sf);
        list_EwZutat[i] -> ergWidget -> icon_achtung -> setVisible(true);
      }
      else {
        list_EwZutat[i] -> ergWidget -> setToolTip(trUtf8("Rest ") + QString::number(ist - soll) + "g");
        list_EwZutat[i] -> ergWidget -> icon_achtung -> setVisible(false);
      }

    }
    //alle anderen zutaten
    else {
      soll = list_EwZutat[i] -> getErg_Menge();
      int id = list_EwZutat[i] -> getID();
      //Nun überprüfen ob die zutat in den weiteren Zutaten noch einmal vorkommt
      for (int o=0; o < list_EwZutat.count(); o++){
        if ((list_EwZutat[o] -> getName() ==  s) && (list_EwZutat[o] -> getTyp() != EWZ_Typ_Hopfen)) {
          //wenn der Eintrag der gleiche ist dann natürlich nicht berücksichtigen
          if (list_EwZutat[o] -> getID() != id){
            soll += list_EwZutat[o] -> getErg_Menge();
          }
        }
      }
      //Wenn angehakt Sude aus der Merkliste abfragen und überprüfen ob da dieser Rohstoff auch verwendet wird.
      if (checkBox_MerklisteMengen->isChecked()) {
        QSqlQuery query;
        QString sql;
        sql = "SELECT ID FROM 'Sud' WHERE MerklistenID=1";
        if (!query.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                      + trUtf8("\nSQL Befehl:\n") + sql);
        }
        else {
          while (query.next()){
            //ID
            int FeldNr = query.record().indexOf("ID");
            int SudID = query.value(FeldNr).toInt();
            //Aktuellen Sud ignorieren
            if (SudID != AktuelleSudID) {
              QSqlQuery query2;
              //Weitere Zutaten abfragen
              QString sql2 = "SELECT * FROM 'WeitereZutatenGaben' WHERE SudID="+QString::number(SudID)+" AND Name='"+s+"'";
              if (!query2.exec(sql2)) {
                // Fehlermeldung Datenbankabfrage
                ErrorMessage *errorMessage = new ErrorMessage();
                errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                            CANCEL_NO, trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
                                            + trUtf8("\nSQL Befehl:\n") + sql2);
              }
              else {
                while (query2.next()){
                  FeldNr = query2.record().indexOf("erg_Menge");
                  soll += query2.value(FeldNr).toDouble();
                }
              }
            }
          }
        }
      }
      //Menge die zur verfügung steht
      int AnzahlWeitereZutaten = tableWidget_WeitereZutaten -> rowCount();
      bool gefunden = false;
      for (int o=0; o < AnzahlWeitereZutaten; o++){
        //Wenn Eintrag übereinstimmt
        if (tableWidget_WeitereZutaten -> item(o,TableWZutatColName) -> text() == s){
          gefunden = true;
          QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,TableWZutatColMenge);
          ist = dsbMenge -> value();
        }
      }
      if (!gefunden){
        ist = 0;
      }
      if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
        soll = soll / 1000;
      list_EwZutat[i] -> ergWidget->setRest(ist - soll);
      //Anzeige Einfärben wenn Rohstoff nicht vorrätig wäre
      if (soll > ist){
        QString sf;
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg){
          sf = QString::number(soll - ist) + trUtf8(" kg zu wenig vorhanden");
        }
        else{
          sf = QString::number(soll - ist) + trUtf8(" g zu wenig vorhanden");
        }
        list_EwZutat[i] -> ergWidget -> setToolTip(sf);
        list_EwZutat[i] -> ergWidget -> icon_achtung -> setVisible(true);
      }
      else {
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg){
          list_EwZutat[i] -> ergWidget -> setToolTip(trUtf8("Rest ") + QString::number(ist - soll) + "kg");
        }
        else {
          list_EwZutat[i] -> ergWidget -> setToolTip(trUtf8("Rest ") + QString::number(ist - soll) + "g");
        }
        list_EwZutat[i] -> ergWidget -> icon_achtung -> setVisible(false);
      }
    }
  }

}


void MainWindowImpl::slot_pushButton_DatenUebertragen()
{
  spinBox_WuerzemengeAnstellen -> setValue(spinBox_WuerzemengeKochende -> value());
  spinBox_SWAnstellen -> setValue(spinBox_SWKochende -> value());
  setAenderung(true);
}


void MainWindowImpl::CheckJungbierSW()
{
  QPalette paletteF = spinBox_SW -> palette();
  QPalette paletteN = spinBox_SW -> palette();
  paletteF.setColor(QPalette::Base , Qt::red);
  //paletteN.setColor(QPalette::Base , Qt::white);
  QPalette paletteF2 = pushButton_SudAbgefuellt -> palette();
  QPalette paletteN2 = pushButton_RohstoffeAbziehen -> palette();
  paletteF2.setColor(QPalette::Button , Qt::red);

  if ((spinBox_SWJungbier -> value() > spinBox_Gruenschlauchzeitpunkt -> value())
      && (checkBox_SchnellgaerprobeAktiv -> isChecked())){
    QString s = trUtf8("!!!Achtung das Jungbier ist noch nicht weit genug endvergoren, der Restextrakt sollte nicht über ") + QString::number(spinBox_Gruenschlauchzeitpunkt -> value()) + trUtf8("°P liegen");
    spinBox_SWJungbier -> setPalette(paletteF);
    spinBox_SWJungbier -> setToolTip(s);
    pushButton_SudAbgefuellt -> setPalette(paletteF2);
    pushButton_SudAbgefuellt -> setToolTip(s);
  }
  else {
    spinBox_SWJungbier -> setToolTip("");
    spinBox_SWJungbier -> setPalette(paletteN);
    pushButton_SudAbgefuellt -> setToolTip("");
    pushButton_SudAbgefuellt -> setPalette(paletteN2);
  }
}


void MainWindowImpl::CheckGesammtMaischeMenge()
{
  QPalette paletteF = spinBox_SW -> palette();
  QPalette paletteN = spinBox_SW -> palette();
  paletteF.setColor(QPalette::Base , QColor::fromRgb(255,138,0));

  if (BierWurdeGebraut) {
    doubleSpinBox_VolumenMaische -> setToolTip("");
    doubleSpinBox_VolumenMaische -> setPalette(paletteN);
  }
  else {
    if (doubleSpinBox_VolumenMaische -> value() > getMaischenMaxNutzvolumen()){
      QString s = trUtf8("Kapazität Maischekessel um ")
          + QString::number(doubleSpinBox_VolumenMaische -> value()
                            - getMaischenMaxNutzvolumen()) + trUtf8(" Liter überschritten");
      doubleSpinBox_VolumenMaische -> setPalette(paletteF);
      doubleSpinBox_VolumenMaische -> setToolTip(s);
    }
    else {
      QString s = trUtf8("noch ")
          + QString::number(getMaischenMaxNutzvolumen()
                            - doubleSpinBox_VolumenMaische -> value()) + trUtf8(" Liter Platz");
      doubleSpinBox_VolumenMaische -> setToolTip(s);
      doubleSpinBox_VolumenMaische -> setPalette(paletteN);
    }
  }
}


void MainWindowImpl::CheckPfannevoll()
{
  QPalette paletteF = spinBox_SW -> palette();
  QPalette paletteN = spinBox_SW -> palette();
  paletteF.setColor(QPalette::Base , QColor::fromRgb(255,138,0));

  if (BierWurdeGebraut) {
    doubleSpinBox_VolumenPfannevoll -> setToolTip("");
    doubleSpinBox_VolumenPfannevoll -> setPalette(paletteN);
  }
  else {
    if (doubleSpinBox_VolumenPfannevoll -> value() > getSudpfanneMaxNutzvolumen()){
      QString s = trUtf8("Kapazität Sudpfanne um ")
          + QString::number(doubleSpinBox_VolumenPfannevoll -> value()
                            - getSudpfanneMaxNutzvolumen()) + trUtf8(" Liter überschritten");
      doubleSpinBox_VolumenPfannevoll -> setPalette(paletteF);
      doubleSpinBox_VolumenPfannevoll -> setToolTip(s);
    }
    else {
      QString s = trUtf8("noch ")
          + QString::number(getSudpfanneMaxNutzvolumen()
                            - doubleSpinBox_VolumenPfannevoll -> value()) + trUtf8(" Liter Platz");
      doubleSpinBox_VolumenPfannevoll -> setToolTip(s);
      doubleSpinBox_VolumenPfannevoll -> setPalette(paletteN);
    }
  }
}


void MainWindowImpl::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        if (Aenderung ? AbfrageSpeichern() : true)
        {
            AktuelleSudID = action -> data().toInt();
            LadeSudDB(true);
        }
    }
}

void MainWindowImpl::setRecentFile(int ID)
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  QStringList files = settings.value("recentFileList").toStringList();
  QString s;
  s = lineEdit_Sudname -> text() + "=" + QString::number(ID);
  files.removeAll(s);
  files.prepend(s);
  while (files.size() > MaxRecentFiles)
    files.removeLast();

  settings.setValue("recentFileList", files);

  foreach (QWidget *widget, QApplication::topLevelWidgets()) {
    MainWindowImpl *mainWin = qobject_cast<MainWindowImpl *>(widget);
    if (mainWin)
      mainWin->updateRecentFileActions();
  }
}

void MainWindowImpl::setFensterTitel()
{
  //Aktuellen Sudnamen in windows titel schreiben
  QString s = APP_NAME;
  s += " v";
  s += VERSION;
  QString s1 = DB_USER_NAME;
  QString s2 = "kb_daten.sqlite";
  if (s1 != s2){
    s += " (Datenbank: ";
    s += DB_USER_NAME;
    s += ") - ";
  }
  else {
    s += " - ";
  }
  s += lineEdit_Sudname -> text();
  if (Aenderung)
    s += "*";
  this -> setWindowTitle(s);
}

void MainWindowImpl::setBewertungFarbe()
{
  int farbe = 0;
  if (radioButton_farbe_0->isChecked()){
    farbe += 1;
  }
  if (radioButton_farbe_1->isChecked()){
    farbe += 2;
  }
  if (radioButton_farbe_2->isChecked()){
    farbe += 4;
  }
  if (radioButton_farbe_3->isChecked()){
    farbe += 8;
  }
  if (radioButton_farbe_4->isChecked()){
    farbe += 16;
  }
  if (radioButton_farbe_5->isChecked()){
    farbe += 32;
  }
  if (radioButton_farbe_6->isChecked()){
    farbe += 64;
  }
  if (radioButton_farbe_7->isChecked()){
    farbe += 128;
  }
  if (radioButton_farbe_8->isChecked()){
    farbe += 256;
  }
  if (radioButton_farbe_9->isChecked()){
    farbe += 512;
  }
  if (radioButton_farbe_10->isChecked()){
    farbe += 1024;
  }
  list_Bewertung[getBewertungsIndex()]->setFarbe(farbe);
  setAenderung(true);
}

void MainWindowImpl::setBewertungSchaum()
{
  int schaum = 0;
  if (radioButton_schaum_0->isChecked()){
    schaum += 1;
  }
  if (radioButton_schaum_1->isChecked()){
    schaum += 2;
  }
  if (radioButton_schaum_2->isChecked()){
    schaum += 4;
  }
  if (radioButton_schaum_3->isChecked()){
    schaum += 8;
  }
  if (radioButton_schaum_4->isChecked()){
    schaum += 16;
  }
  if (radioButton_schaum_5->isChecked()){
    schaum += 32;
  }
  if (radioButton_schaum_6->isChecked()){
    schaum += 64;
  }
  if (radioButton_schaum_7->isChecked()){
    schaum += 128;
  }
  if (radioButton_schaum_8->isChecked()){
    schaum += 256;
  }
  if (radioButton_schaum_9->isChecked()){
    schaum += 512;
  }
  if (radioButton_schaum_10->isChecked()){
    schaum += 1024;
  }
  list_Bewertung[getBewertungsIndex()]->setSchaum(schaum);
  setAenderung(true);

}

void MainWindowImpl::setBewertungGeruch()
{
  int geruch = 0;
  if (checkBox_geruch_0->isChecked()){
    geruch += 1;
  }
  if (checkBox_geruch_1->isChecked()){
    geruch += 2;
  }
  if (checkBox_geruch_2->isChecked()){
    geruch += 4;
  }
  if (checkBox_geruch_3->isChecked()){
    geruch += 8;
  }
  if (checkBox_geruch_4->isChecked()){
    geruch += 16;
  }
  if (checkBox_geruch_5->isChecked()){
    geruch += 32;
  }
  if (checkBox_geruch_6->isChecked()){
    geruch += 64;
  }
  if (checkBox_geruch_7->isChecked()){
    geruch += 128;
  }
  if (checkBox_geruch_8->isChecked()){
    geruch += 256;
  }
  if (checkBox_geruch_9->isChecked()){
    geruch += 512;
  }
  if (checkBox_geruch_10->isChecked()){
    geruch += 1024;
  }
  if (checkBox_geruch_11->isChecked()){
    geruch += 2048;
  }
  list_Bewertung[getBewertungsIndex()]->setGeruch(geruch);
  setAenderung(true);

}

void MainWindowImpl::setBewertungGeschmack()
{
  int geschmack = 0;
  if (checkBox_geschmack_0->isChecked()){
    geschmack += 1;
  }
  if (checkBox_geschmack_1->isChecked()){
    geschmack += 2;
  }
  if (checkBox_geschmack_2->isChecked()){
    geschmack += 4;
  }
  if (checkBox_geschmack_3->isChecked()){
    geschmack += 8;
  }
  if (checkBox_geschmack_4->isChecked()){
    geschmack += 16;
  }
  if (checkBox_geschmack_5->isChecked()){
    geschmack += 32;
  }
  if (checkBox_geschmack_6->isChecked()){
    geschmack += 64;
  }
  if (checkBox_geschmack_7->isChecked()){
    geschmack += 128;
  }
  if (checkBox_geschmack_8->isChecked()){
    geschmack += 256;
  }
  if (checkBox_geschmack_9->isChecked()){
    geschmack += 512;
  }
  if (checkBox_geschmack_10->isChecked()){
    geschmack += 1024;
  }
  if (checkBox_geschmack_11->isChecked()){
    geschmack += 2048;
  }
  if (checkBox_geschmack_12->isChecked()){
    geschmack += 4096;
  }
  list_Bewertung[getBewertungsIndex()]->setGeschmack(geschmack);
  setAenderung(true);

}

void MainWindowImpl::setBewertungAntrunk()
{
  int antrunk = 0;
  if (radioButton_antrunk_0->isChecked()){
    antrunk += 1;
  }
  if (radioButton_antrunk_1->isChecked()){
    antrunk += 2;
  }
  if (radioButton_antrunk_2->isChecked()){
    antrunk += 4;
  }
  if (radioButton_antrunk_3->isChecked()){
    antrunk += 8;
  }
  if (radioButton_antrunk_4->isChecked()){
    antrunk += 16;
  }
  if (radioButton_antrunk_5->isChecked()){
    antrunk += 32;
  }
  if (radioButton_antrunk_6->isChecked()){
    antrunk += 64;
  }
  if (radioButton_antrunk_7->isChecked()){
    antrunk += 128;
  }
  list_Bewertung[getBewertungsIndex()]->setAntrunk(antrunk);
  setAenderung(true);

}

void MainWindowImpl::setBewertungHaupttrunk()
{
  int haupttrunk = 0;
  if (radioButton_haupttrunk_0->isChecked()){
    haupttrunk += 1;
  }
  if (radioButton_haupttrunk_1->isChecked()){
    haupttrunk += 2;
  }
  if (radioButton_haupttrunk_2->isChecked()){
    haupttrunk += 4;
  }
  if (radioButton_haupttrunk_3->isChecked()){
    haupttrunk += 8;
  }
  if (radioButton_haupttrunk_4->isChecked()){
    haupttrunk += 16;
  }
  list_Bewertung[getBewertungsIndex()]->setHaupttrunk(haupttrunk);
  setAenderung(true);

}

void MainWindowImpl::setBewertungNachtrunk()
{
  int nachtrunk = 0;
  if (radioButton_nachtrunk_0->isChecked()){
    nachtrunk += 1;
  }
  if (radioButton_nachtrunk_1->isChecked()){
    nachtrunk += 2;
  }
  if (radioButton_nachtrunk_2->isChecked()){
    nachtrunk += 4;
  }
  if (radioButton_nachtrunk_3->isChecked()){
    nachtrunk += 8;
  }
  if (radioButton_nachtrunk_4->isChecked()){
    nachtrunk += 16;
  }
  if (radioButton_nachtrunk_5->isChecked()){
    nachtrunk += 32;
  }
  if (radioButton_nachtrunk_6->isChecked()){
    nachtrunk += 64;
  }
  if (radioButton_nachtrunk_7->isChecked()){
    nachtrunk += 128;
  }
  if (radioButton_nachtrunk_8->isChecked()){
    nachtrunk += 256;
  }
  list_Bewertung[getBewertungsIndex()]->setNachtrunk(nachtrunk);
  setAenderung(true);

}

void MainWindowImpl::setBewertungGesamteindruck()
{
  int gesamteindruck = 0;
  if (radioButton_gesamteindruck_0->isChecked()){
    gesamteindruck += 1;
  }
  if (radioButton_gesamteindruck_1->isChecked()){
    gesamteindruck += 2;
  }
  if (radioButton_gesamteindruck_2->isChecked()){
    gesamteindruck += 4;
  }
  if (radioButton_gesamteindruck_3->isChecked()){
    gesamteindruck += 8;
  }
  if (radioButton_gesamteindruck_4->isChecked()){
    gesamteindruck += 16;
  }
  if (radioButton_gesamteindruck_5->isChecked()){
    gesamteindruck += 32;
  }
  if (radioButton_gesamteindruck_6->isChecked()){
    gesamteindruck += 64;
  }
  if (radioButton_gesamteindruck_7->isChecked()){
    gesamteindruck += 128;
  }
  list_Bewertung[getBewertungsIndex()]->setGesamteindruck(gesamteindruck);
  setAenderung(true);

}


void MainWindowImpl::updateRecentFileActions()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  QStringList files = settings.value("recentFileList").toStringList();

  int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

  for (int i = 0; i < numRecentFiles; ++i) {
    QString text = QString("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
    recentFileActs[i] -> setText(text);
    recentFileActs[i] -> setData(strippedID(files[i]));
    recentFileActs[i] -> setVisible(true);
    recentFileActs[i] -> setStatusTip(strippedName(files[i]));
  }
  for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
    recentFileActs[j]->setVisible(false);
}


QString MainWindowImpl::strippedName(const QString &fullFileName)
{
  return fullFileName.left(fullFileName.indexOf("="));
}

int MainWindowImpl::strippedID(const QString &fullFileName)
{
  //QMessageBox::information( this, tr( "File info." ),fullFileName.right(fullFileName.length() - fullFileName.indexOf("=") - 1));
  return fullFileName.right(fullFileName.length() - fullFileName.indexOf("=") - 1).toInt();
}

void MainWindowImpl::LadeSudDB(bool aktivateTab)
{
    AmLaden = true;
    ErstelleZutatenlisten();
    FuelleRezeptComboAuswahlen();
    LeseSuddatenDB();
    SetStatusGebraut(BierWurdeGebraut);
    SetDisabledAbgefuellt(BierWurdeAbgefuellt);
    SetDisabledVerbraucht(BierWurdeVerbraucht || !BierWurdeAbgefuellt);
    pushButton_SudTeilen->setDisabled(BierWurdeVerbraucht);
    setRecentFile(AktuelleSudID);
    AmLaden = false;
    BerAlles();
    setAenderung(false);
    if (aktivateTab)
    {
        if (BierWurdeAbgefuellt)
            tabWidged->setCurrentWidget(tab_Spickzettel);
        else if (BierWurdeGebraut)
            tabWidged->setCurrentWidget(tab_Gaerverlauf);
        else
            tabWidged->setCurrentWidget(tab_Rezept);
    }
}


void MainWindowImpl::slot_TableWidget_cellChanged(int , int )
{
  if (Gestartet){
    setAenderung(true);
    //Comboboxen neu Fuellen (wenn Tab gewechselt wird)
    AenderungRohstofftabelle = true;

    BerAlles();
  }
}


bool MainWindowImpl::AbfrageSpeichern()
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(APP_NAME);
  msgBox.setText(trUtf8("Die Daten wurden verändert."));
  msgBox.setInformativeText(trUtf8("Sollen die Änderungen gespeichert werden?"));
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  int ret = msgBox.exec();

  // abbrechen
  if (ret == QMessageBox::Cancel) {
      return false;
  }
  //Änderungen speichern
  else if (ret == QMessageBox::Save) {
    save();
    setAenderung(false);
    return true;
  }
  //Änderungen nicht speichern
  else {
    //für den fall das sich die Rohstoffe geändert haben müssen sie neu geladen werden
    // in der Zeit des neu einlesens der Rohstoffe merker gestartet zurücksetzten
    // das hat den effekt das bei einer änderung an den Tabellen nicht neu berechnet wird
    // was zu einem absturz fürhen würde
    Gestartet = false;
    DatenEinlesenDB();
    Gestartet = true;
    return true;
  }
}


void MainWindowImpl::AddHopfengabe(bool vwh, QString Name, int Zeit, double Menge, double erg_Menge, double Alpha, int Pellets)
{
  //Hopfen hinzufügen
  //Zutatenobjekt hinzufügen
  hopfengabe* hopfen = new hopfengabe(this);
  hopfen -> setStyleDunkel(StyleDunkel);
  hopfen -> setAttribute(Qt::WA_DeleteOnClose);

  //Zutatenliste füllen
  hopfen -> setBierWurdeGebraut(BierWurdeGebraut);
  hopfen -> setHopfenListe(HopfenListe);

  connect(hopfen, SIGNAL( sig_vorClose(int) ), this, SLOT( slot_hopfenClose(int) ));
  connect(hopfen, SIGNAL( sig_Aenderung() ), this, SLOT( slot_HopfenAenderung() ));
  connect(hopfen, SIGNAL( sig_getHopfenMenge(QString) ), this, SLOT( slot_HopfenGetMenge(QString) ));

  verticalLayout_Hopfengaben -> addWidget(hopfen);
  list_Hopfengaben.append(hopfen);
  hopfen->setMaxKochzeit(spinBox_Gesammtkochdauer->value());
  hopfen->setMinKochzeit(spinBox_NachisomerisierungsZeit->value()*-1);
  hopfen -> setID((int)time(NULL)+rand());
  hopfen->setKochzeit(Zeit);
  hopfen->setVWH(vwh);
  if (Name != "")
    hopfen->setName(Name);
  hopfen->setMengeProzent(Menge);
  hopfen->setAlpha(Alpha);
  hopfen->setPellets(Pellets);
  hopfen->setErgMenge(erg_Menge);

  //Ergebnisswidget dem Layout zuordnen
  verticalLayout_BerHopfengaben -> addWidget(hopfen->ergWidget);
}


void MainWindowImpl::slot_spinBoxGesammtkochdauerChanged(int value)
{
  //Maximalen wert für Hopfenzeiten Setzten
  for (int i=0; i < list_Hopfengaben.count(); i++){
    list_Hopfengaben[i]->setMaxKochzeit(value);
  }
}

void MainWindowImpl::slot_tabWidgetChanged(int)
{
  //Wenn sich die Rohstoffe geändert haben müssen die comboauswahlen und Erweiterte
  //Zutaten die Auswahl neu eingelesen werden
  if (AenderungRohstofftabelle  && Gestartet){
    AenderungRohstofftabelle = false;
    ErstelleZutatenlisten();
    FuelleRezeptComboAuswahlen();
  }
}

void MainWindowImpl::on_pushButton_MalzKopie_clicked()
{
    int i = tableWidget_Malz->currentRow();
    if (i >= 0 && i < tableWidget_Malz->rowCount())
    {
        QString s = Malz_Bezeichnung_Merker;
        Malz_Bezeichnung_Merker = "";
        tableWidget_Malz->setSortingEnabled(false);
        MalzNeueZeile(tableWidget_Malz->item(i,TableMalzColName)->text() + trUtf8(" Kopie"),
            ((QDoubleSpinBox*)tableWidget_Malz->cellWidget(i,TableMalzColFarbe))->value(),
            ((QDoubleSpinBox*)tableWidget_Malz->cellWidget(i,TableMalzColMaxSchuettung))->value(),
            ((QDoubleSpinBox*)tableWidget_Malz->cellWidget(i,TableMalzColMenge))->value(),
            ((QDoubleSpinBox*)tableWidget_Malz->cellWidget(i,TableMalzColPreis))->value(),
            tableWidget_Malz->item(i,TableMalzColBemerkung)->text(),
            tableWidget_Malz->item(i,TableMalzColAnwendung)->text(),
            QDate::currentDate(),
            QDate::currentDate().addMonths(1),
            tableWidget_Malz->item(i,TableMalzColLink)->text());
        setAenderung(true);
        AenderungRohstofftabelle = true;
        tableWidget_Malz->setSortingEnabled(true);
        Malz_Bezeichnung_Merker = s;
    }
}

void MainWindowImpl::on_pushButton_HopfenKopie_clicked()
{
    int i = tableWidget_Hopfen->currentRow();
    if (i >= 0 && i < tableWidget_Hopfen->rowCount())
    {
        QString s = Hopfen_Bezeichnung_Merker;
        Hopfen_Bezeichnung_Merker = "";
        tableWidget_Hopfen->setSortingEnabled(false);
        HopfenNeueZeile(tableWidget_Hopfen->item(i,TableHopfenColName)->text() + trUtf8(" Kopie"),
            ((QDoubleSpinBox*)tableWidget_Hopfen->cellWidget(i,TableHopfenColAlpha))->value(),
            ((QDoubleSpinBox*)tableWidget_Hopfen->cellWidget(i,TableHopfenColMenge))->value(),
            ((QDoubleSpinBox*)tableWidget_Hopfen->cellWidget(i,TableHopfenColPreis))->value(),
            tableWidget_Hopfen->item(i,TableHopfenColPellets)->checkState() == Qt::Checked,
            tableWidget_Hopfen ->item(i,TableHopfenColBemerkung)->text(),
            ((QComboBox*)tableWidget_Hopfen->cellWidget(i,TableHopfenColTyp))->currentIndex(),
            tableWidget_Hopfen->item(i,TableHopfenColEigenschaften)->text(),
            QDate::currentDate(),
            QDate::currentDate().addMonths(1),
            tableWidget_Hopfen->item(i,TableHopfenColLink)->text());
        setAenderung(true);
        AenderungRohstofftabelle = true;
        tableWidget_Hopfen->setSortingEnabled(true);
        Hopfen_Bezeichnung_Merker = s;
    }
}

void MainWindowImpl::on_pushButton_HefeKopie_clicked()
{
    int i = tableWidget_Hefe->currentRow();
    if (i >= 0 && i < tableWidget_Hefe->rowCount())
    {
        QString s = Hefe_Bezeichnung_Merker;
        Hefe_Bezeichnung_Merker = "";
        tableWidget_Hefe->setSortingEnabled(false);
        HefeNeueZeile(tableWidget_Hefe->item(i,TableHefeColName)->text() + trUtf8(" Kopie"),
            ((QDoubleSpinBox*)tableWidget_Hefe->cellWidget(i,TableHefeColMenge))->value(),
            ((QDoubleSpinBox*)tableWidget_Hefe->cellWidget(i,TableHefeColWuerzemenge))->value(),
            ((QDoubleSpinBox*)tableWidget_Hefe->cellWidget(i,TableHefeColPreis))->value(),
            tableWidget_Hefe->item(i,TableHefeColBemerkung)->text(),
            tableWidget_Hefe->item(i,TableHefeColVerpackungsmenge)->text(),
            ((QComboBox*)tableWidget_Hefe->cellWidget(i,TableHefeColTypUgOg))->currentIndex(),
            ((QComboBox*)tableWidget_Hefe->cellWidget(i,TableHefeColTypTrFlg))->currentIndex(),
            tableWidget_Hefe->item(i,TableHefeColTemperatur)->text(),
            tableWidget_Hefe->item(i,TableHefeColEigenschaften)->text(),
            ((QComboBox*)tableWidget_Hefe->cellWidget(i,TableHefeColSedimentation))->currentIndex(),
            tableWidget_Hefe->item(i,TableHefeColEVG)->text(),
            QDate::currentDate(),
            QDate::currentDate().addMonths(1),
            tableWidget_Hefe->item(i,TableHefeColLink)->text());
        setAenderung(true);
        AenderungRohstofftabelle = true;
        tableWidget_Hefe->setSortingEnabled(true);
        Hefe_Bezeichnung_Merker = s;
    }
}


void MainWindowImpl::on_pushButton_WeitereZutatenKopie_clicked()
{
    int i = tableWidget_WeitereZutaten->currentRow();
    if (i >= 0 && i < tableWidget_WeitereZutaten->rowCount())
    {
        QString s = WZutaten_Bezeichnung_Merker;
        WZutaten_Bezeichnung_Merker = "";
        tableWidget_WeitereZutaten->setSortingEnabled(false);
        WeitereZutatNeueZeile(tableWidget_WeitereZutaten->item(i,TableWZutatColName)->text() + trUtf8(" Kopie"),
            ((QDoubleSpinBox*)tableWidget_WeitereZutaten->cellWidget(i,TableWZutatColMenge))->value(),
            ((QComboBox*)tableWidget_WeitereZutaten->cellWidget(i,TableWZutatColEinheit))->currentIndex(),
            ((QComboBox*)tableWidget_WeitereZutaten->cellWidget(i,TableWZutatColTyp))->currentIndex(),
            ((QDoubleSpinBox*)tableWidget_WeitereZutaten->cellWidget(i,TableWZutatColAusbeute))->value(),
            ((QDoubleSpinBox*)tableWidget_WeitereZutaten->cellWidget(i,TableWZutatColFarbe))->value(),
            ((QDoubleSpinBox*)tableWidget_WeitereZutaten->cellWidget(i,TableWZutatColPreis))->value(),
            tableWidget_WeitereZutaten->item(i,TableWZutatColBemerkung)->text(),
            QDate::currentDate(),
            QDate::currentDate().addMonths(1),
            tableWidget_WeitereZutaten->item(i,TableWZutatColLink)->text());
        setAenderung(true);
        AenderungRohstofftabelle = true;
        tableWidget_WeitereZutaten->setSortingEnabled(true);
        WZutaten_Bezeichnung_Merker = s;
    }
}

void MainWindowImpl::on_pushButton_SpickzettelPDF_clicked()
{
  //allen nochmal durchrechnen
  BerAlles();

  //Zusammenfassung/Spickzettel neue erstellen
  ErstelleTabSpickzettel();

  QString Sudname = lineEdit_Sudname->text();

  // letzten Pfad einlesen
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  settings.beginGroup("PDF");
  QString p = settings.value("recentPDFPath", QDir::homePath()).toString();

  QString fileName = QFileDialog::getSaveFileName(this, trUtf8("PDF speichern unter"), p + "/" + Sudname + ".pdf", "PDF (*.pdf)");
  if (!fileName.isEmpty())
  {
      bool merker = StyleDunkel;
      if (merker)
      {
          StyleDunkel = false;
          ErstelleSudInfo();
      }

      // pdf speichern
      webView_Zusammenfassung->printToPdf(fileName);

      if (merker)
      {
          StyleDunkel = merker;
          ErstelleSudInfo();
      }

      // Pfad abspeichern
      QFileInfo fi(fileName);
      settings.setValue("recentPDFPath", fi.absolutePath());

      // open PDF
      QDesktopServices::openUrl(QUrl("file:///" + fileName));
  }

  settings.endGroup();
}

void MainWindowImpl::LadeBild()
{

  graphicsView -> setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

  scene = new QGraphicsScene;

  if (StyleDunkel){
    m_svgItem = new QGraphicsSvgItem(":/global/bier_dark.svg");
  }
  else {
    m_svgItem = new QGraphicsSvgItem(":/global/bier.svg");
  }
  scene -> addItem(m_svgItem);

  graphicsView -> setScene(scene);
}


void MainWindowImpl::BerFarbe(double cEBC)
{
  //Farbe berechnen

  QPalette paletteN = spinBox_Haushaltszucker500mlFlasche -> palette();
  QPalette paletteF = spinBox_Haushaltszucker500mlFlasche -> palette();
  paletteF.setColor(QPalette::Base , Qt::red);
  QColor Bierfarbe;
  if (cEBC == 0){
    double *schuettung = new double[list_Malzgaben.count() + list_EwZutat.count()];
    double *farbe = new double[list_Malzgaben.count() + list_EwZutat.count()];
    QString s;
    int gefunden = 0;
    for (int z = 0; z < list_Malzgaben.count(); z++){
      s = list_Malzgaben[z]->getName();
      if (s != ""){
        schuettung[z] = list_Malzgaben[z]->getErgMenge();
        for (int i=0; i < tableWidget_Malz -> rowCount(); i++){
          if (s == tableWidget_Malz -> item(i,TableMalzColName) -> text()){
            QDoubleSpinBox* dsbFarbe = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,TableMalzColFarbe);
            farbe[z] = dsbFarbe ->value();
            list_Malzgaben[z]->setFarbe(farbe[z]);
            gefunden++;
          }
        }
      }
    }

    //Farbwerte und Mengen der weiteren Zutaten
    int anzahlWZAuswerten = 0;
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getFarbe() > 0){
        schuettung[i + list_Malzgaben.count()] = list_EwZutat[i] -> getErg_Menge() / 1000;
        farbe[i + list_Malzgaben.count()] = list_EwZutat[i] -> getFarbe();
        gefunden++;
        anzahlWZAuswerten++;
      }
    }

    //wenn nicht alle Farbwerte ermittelt werden konnten keinen Farbwert berechnen
    double EBC;
    if (gefunden != list_Malzgaben.count() + anzahlWZAuswerten){
      Bierfarbe = QColor::fromRgb(255,255,255);
      EBC = 0;
      doubleSpinBox_EBC -> setToolTip(trUtf8("Farbe konnte nicht berechnet werden da eine Auswahl in der Schüttungs-zusammenstellung\nin den Rohstoffdaten nicht mehr vorhanden ist"));
      doubleSpinBox_EBC -> setPalette(paletteF);
    }
    else {
      doubleSpinBox_EBC -> setToolTip("");
      doubleSpinBox_EBC -> setPalette(paletteN);
      Bierfarbe = Berechnungen.GetFarbwert(schuettung, farbe, list_Malzgaben.count() + anzahlWZAuswerten,sw_schuettung, getKorrekturFarbe() );
      EBC = Berechnungen.getEBC();
    }
    doubleSpinBox_EBC -> setValue(EBC);
    delete [] schuettung;
    delete [] farbe;
  }
  else {
    Bierfarbe = Berechnungen.GetFarbwert(cEBC);
  }

  scene = new QGraphicsScene;
  scene -> clear();
  QPen pen;
  pen.setColor(Qt::white);
  QBrush brush;
  brush.setColor(Bierfarbe);
  brush.setStyle(Qt::SolidPattern);
  rect = scene -> addRect(10,10,980,980,pen,brush);
  scene -> addItem(m_svgItem);
  graphicsView -> setScene(scene);
}



void MainWindowImpl::slot_lineEdit_editingFinished()
{
  setAenderung(true);
  BerAlles();
}


void MainWindowImpl::FuelleSudauswahl()
{
  QSqlQuery query;
  int FeldNr;
  QString sql;
  int SelZeile = tableWidget_Sudauswahl -> currentRow();
  if (SelZeile == -1)
    SelZeile = 0;

  //Malz einlesen
  //Alle Anzeigen
  if (radioButton_FilterAlle -> isChecked())
    sql = "SELECT * FROM Sud";
  //Nur die anzeigen dien noch nicht Gebraut wurden
  else if (radioButton_FilterNichtGebraut -> isChecked())
    sql = "SELECT * FROM Sud WHERE BierWurdeGebraut=0";
  //Gebraut aber noch nicht Abgefüllt
  else if (radioButton_FilterGebrautNichtAbgefuellt -> isChecked())
    sql = "SELECT * FROM Sud WHERE BierWurdeGebraut=1 AND BierWurdeAbgefuellt=0";
  //Abgefüllt
  else if (radioButton_Abgefuellt -> isChecked())
    sql = "SELECT * FROM Sud WHERE BierWurdeAbgefuellt=1";
  //nicht verbraucht
  else if (radioButton_nichtVerbraucht -> isChecked())
    sql = "SELECT * FROM Sud WHERE BierWurdeVerbraucht=0";
  //Merkliste
  else if (radioButton_Merkliste -> isChecked())
    sql = "SELECT * FROM Sud WHERE MerklistenID=1";
  else
    sql = "SELECT * FROM Sud";

  sql += " ORDER BY Braudatum DESC";

  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int i=0;
    tableWidget_Sudauswahl -> clearContents();
    tableWidget_Sudauswahl -> setRowCount(0);
    tableWidget_Sudauswahl -> setSortingEnabled(false);
    while (query.next()){
      //Auslesen ob Bier schon gebraut wurde
      bool gebraut = query.value(query.record().indexOf("BierWurdeGebraut")).toBool();
      bool abgefuellt = query.value(query.record().indexOf("BierWurdeAbgefuellt")).toBool();
      bool verbraucht = query.value(query.record().indexOf("BierWurdeVerbraucht")).toBool();
      QTableWidgetItem *newItem1 = new QTableWidgetItem("");
      QTableWidgetItem *newItem2 = new QTableWidgetItem("");
      QTableWidgetItem *newItem3 = new QTableWidgetItem("");
      QTableWidgetItem *newItem4 = new QTableWidgetItem("");
      QTableWidgetItem *newItem5 = new QTableWidgetItem("");
      tableWidget_Sudauswahl -> setRowCount(tableWidget_Sudauswahl -> rowCount()+1);
      //ID
      FeldNr = query.record().indexOf("ID");
      newItem1 -> setText(query.value(FeldNr).toString());
      tableWidget_Sudauswahl -> setItem(i, 0, newItem1);
      //Sudname
      FeldNr = query.record().indexOf("Sudname");
      newItem2 -> setText(query.value(FeldNr).toString());
      tableWidget_Sudauswahl -> setItem(i, 1, newItem2);
      //Braudatum (wenn schon gebraut)
      if (gebraut){
        newItem2 -> setBackgroundColor(Qt::green);
        FeldNr = query.record().indexOf("Braudatum");
        newItem3 -> setData(Qt::DisplayRole, QDate::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      }
      tableWidget_Sudauswahl -> setItem(i, 2, newItem3);
      //Erstellt
      FeldNr = query.record().indexOf("Erstellt");
      newItem4 -> setData(Qt::DisplayRole, QDateTime::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      tableWidget_Sudauswahl -> setItem(i, 3, newItem4);
      //Gespeichert
      FeldNr = query.record().indexOf("Gespeichert");
      newItem5 -> setData(Qt::DisplayRole, QDateTime::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      tableWidget_Sudauswahl -> setItem(i, 4, newItem5);
      //in Merkliste
      FeldNr = query.record().indexOf("MerklistenID");
      int MerklistenID = query.value(FeldNr).toInt();

      //Mittig ausrichten
      newItem3 -> setTextAlignment(Qt::AlignCenter);
      newItem4 -> setTextAlignment(Qt::AlignCenter);
      newItem5 -> setTextAlignment(Qt::AlignCenter);
      //Farbe setzen wenn gebraut
      QColor color;
      if (verbraucht){
        if (StyleDunkel)
          color.setRgb(50, 50, 50);
        else
          color.setRgb(200, 200, 200);
        newItem2 -> setBackgroundColor(color);
        newItem3 -> setBackgroundColor(color);
        newItem4 -> setBackgroundColor(color);
        newItem5 -> setBackgroundColor(color);
      }
      else if (abgefuellt){
        if (StyleDunkel)
          color.setRgb(99, 125, 21);
        else
          color.setRgb(193, 225, 178);
        newItem2 -> setBackgroundColor(color);
        newItem3 -> setBackgroundColor(color);
        newItem4 -> setBackgroundColor(color);
        newItem5 -> setBackgroundColor(color);
      }
      else if (gebraut){
        if (StyleDunkel)
          color.setRgb(125, 99, 21);
        else
          color.setRgb(225, 216, 184);
        newItem2 -> setBackgroundColor(color);
        newItem3 -> setBackgroundColor(color);
        newItem4 -> setBackgroundColor(color);
        newItem5 -> setBackgroundColor(color);
      }
      if (MerklistenID == 1) {
        if (StyleDunkel)
          color.setRgb(80,120,188);
        else
          color.setRgb(122,163,233);
        newItem2 -> setBackgroundColor(color);
      }
      i++;
    }
  }
  tableWidget_Sudauswahl -> setSortingEnabled(true);
  tableWidget_Sudauswahl -> setCurrentCell(SelZeile,0);
}

//Legt einen Neuen Sud mit Defaultwerten an
int MainWindowImpl::slot_pushButton_SudNeu()
{
  pushButton_SudNeu -> setEnabled(false);
  QSqlQuery query;
  int SudID = 0;

  //Neuen Sud Datensatz anlegen
  QString sql = "INSERT INTO Sud(Sudname, Erstellt, berechnungsArtHopfen) ";
  sql += "VALUES(";
  sql += "'Neuer Sud', ";
  sql += "'" + QDateTime::currentDateTime().toString(Qt::ISODate) + "', ";
  sql += "'1'";
  sql += ");";

  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    //SudID auslesen
    sql = "SELECT last_insert_rowid();";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    else {
      query.first();
      SudID = query.value(0).toInt();
    }

    //Schüttungsdatensätze anlegen
    sql = "INSERT INTO Malzschuettung(SudID) ";
    sql += "VALUES(";
    sql += QString::number(SudID);
    sql += ");";

    for (int i = 0; i < MAX_MALZGABEN; i++){
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
    }

    //Hopfengabendatensätze anlegen
    sql = "INSERT INTO Hopfengaben(SudID) ";
    sql += "VALUES(";
    sql += QString::number(SudID);
    sql += ");";

    for (int i = 0; i < MAX_HOPFENGABEN; i++){
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
    }
    //Letzte Hopfengabe als Vorderwürzehopfung markieren
    sql = "SELECT * FROM Hopfengaben WHERE SudID = " + QString::number(SudID);
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    else {
      if (query.last()){
        int FeldNr = query.record().indexOf("ID");
        QString ID = query.value(FeldNr).toString();
        sql = "UPDATE 'Hopfengaben' SET 'Vorderwuerze' ='1' WHERE ID=" + ID;
        if (!query.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                      + trUtf8("\nSQL Befehl:\n") + sql);
        }
      }
    }

  }

  //Sudauswahl aktuallisieren
  FuelleSudauswahl();

  pushButton_SudNeu -> setEnabled(true);
  return SudID;
}

void MainWindowImpl::slot_pushButton_SudKopie()
{
    QString id = tableWidget_Sudauswahl->item(tableWidget_Sudauswahl->currentRow(),0)->text();
    QString name = tableWidget_Sudauswahl->item(tableWidget_Sudauswahl->currentRow(),1)->text() + trUtf8(" Kopie");
    pushButton_SudKopie -> setEnabled(false);
    if (Database::SudKopieren(id, name, false) > 0)
        FuelleSudauswahl();
    pushButton_SudKopie -> setEnabled(true);
}

void MainWindowImpl::slot_pushButton_SudDel()
{
  pushButton_SudDel -> setEnabled(false);
  int row = tableWidget_Sudauswahl -> currentRow();
  QString SudID = tableWidget_Sudauswahl -> item(row,0) -> text();
  QString Sudname = tableWidget_Sudauswahl -> item(row,1) -> text();
  //Abfrage ob gelöscht werden soll
  QMessageBox msgBox;
  msgBox.setWindowTitle(trUtf8("Löschen?"));
  msgBox.setText(trUtf8("Soll der Sud >") + Sudname + trUtf8("< gelöscht werden?"));
  msgBox.setInformativeText(trUtf8("Achtung: dies kann nicht rückgängig gemacht werden!"));
  msgBox.setIcon(QMessageBox::Warning);
  QPushButton *LoeschenButton = msgBox.addButton(trUtf8("Löschen"), QMessageBox::ActionRole);
  msgBox.setDefaultButton(msgBox.addButton(trUtf8("Abrechen"), QMessageBox::ActionRole));

  msgBox.exec();

  if (msgBox.clickedButton() == LoeschenButton){
    QSqlQuery query;

    //Sud Datensatz löschen
    QString sql = "DELETE FROM Sud WHERE ID="+ SudID +";";

    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    //Alle zugehörigen Rasten löschen
    sql = "DELETE FROM Rasten WHERE SudID="+ SudID +";";

    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    //Alle zugehörigen Malzgaben löschen
    sql = "DELETE FROM Malzschuettung WHERE SudID="+ SudID +";";

    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    //Alle zugehörigen Hopfengaben löschen
    sql = "DELETE FROM Hopfengaben WHERE SudID="+ SudID +";";

    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    //Alle zugehörigen Schnellgärungsdaten löschen
    sql = "DELETE FROM Schnellgaerverlauf WHERE SudID="+ SudID +";";

    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    //Alle zugehörigen Hauptgärungsdaten löschen
    sql = "DELETE FROM Hauptgaerverlauf WHERE SudID="+ SudID +";";

    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    //Alle zugehörigen Nachgärungsdaten löschen
    sql = "DELETE FROM Nachgaerverlauf WHERE SudID="+ SudID +";";

    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    //Alle zugehörigen Weitere Zutaten löschen
    sql = "DELETE FROM WeitereZutatenGaben WHERE SudID="+ SudID +";";

    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }

    //Alle zugehörigen Bewertungen löschen
    sql = "DELETE FROM Bewertungen WHERE SudID="+ SudID +";";

    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }

    //Alle zugehörigen Anhänge löschen
    sql = "DELETE FROM Anhang WHERE SudID="+ SudID +";";

    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }

    //Sudauswahl aktuallisieren
    FuelleSudauswahl();
  }
  pushButton_SudDel -> setEnabled(true);
}


void MainWindowImpl::slot_pushButton_SudLaden()
{
    int row = tableWidget_Sudauswahl->currentRow();
    if (row >= 0)
    {
        if (Aenderung ? AbfrageSpeichern() : true)
        {
            pushButton_SudLaden->setEnabled(false);
            AktuelleSudID = tableWidget_Sudauswahl->item(row, 0)->text().toInt();
            LadeSudDB(true);
            pushButton_SudLaden->setEnabled(true);
        }
    }
}

void MainWindowImpl::on_pushButton_SudLadenBUebersicht_clicked()
{
    int row = tableWidget_Brauuebersicht->currentRow();
    if (row >= 0)
    {
        if (Aenderung ? AbfrageSpeichern() : true)
        {
            pushButton_SudLadenBUebersicht->setEnabled(false);
            AktuelleSudID = tableWidget_Brauuebersicht->item(row, 0)->text().toInt();
            LadeSudDB(true);
            pushButton_SudLadenBUebersicht->setEnabled(true);
        }
    }
}

void MainWindowImpl::AddMalzgabe(QString Name, double Prozent, double erg_Menge, double Farbe)
{
  //Malz hinzufügen
  //Zutatenobjekt hinzufügen
  malzgabe* malz = new malzgabe(this);
  malz -> setStyleDunkel(StyleDunkel);
  malz -> setAttribute(Qt::WA_DeleteOnClose);

  connect(malz, SIGNAL( sig_vorClose(int) ), this, SLOT( slot_malzClose(int) ));
  connect(malz, SIGNAL( sig_Aenderung() ), this, SLOT( slot_MalzAenderung() ));
  connect(malz, SIGNAL( sig_getMalzMenge(QString) ), this, SLOT( slot_MalzGetMenge(QString) ));
  //Zutatenliste füllen
  malz -> setBierWurdeGebraut(BierWurdeGebraut);
  malz -> setMalzListe(MalzListe);
  malz -> setID((int)time(NULL)+rand());
  if (Name != ""){
    malz->setName(Name);
  }
  malz->setMengeProzent(Prozent);
  malz->setErgMenge(erg_Menge);
  malz->setFarbe(Farbe);

  verticalLayout_Malzgaben -> addWidget(malz);
  list_Malzgaben.append(malz);

  //Ergebnisswidget dem Layout zuordnen
  verticalLayout_BerMalzgaben -> addWidget(malz->ergWidget);
}


void MainWindowImpl::slot_FilterClicked(bool )
{
  FuelleSudauswahl();
}


void MainWindowImpl::SchreibeRastenDB()
{
  QSqlQuery query;
  QString sql = "DELETE FROM Rasten WHERE SudID =" + QString::number(AktuelleSudID);
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

  for (int i=0; i < list_Rasten.count(); i++){
    sql = "INSERT INTO Rasten(SudID, RastAktiv, RastTemp, RastDauer, RastName) VALUES(" +
        QString::number(AktuelleSudID) +	"," +
        QString::number(1) +	"," +
        QString::number(list_Rasten[i] -> getRastTemp()) +	"," +
        QString::number(list_Rasten[i] -> getRastDauer()) +	"," +
        "'" + list_Rasten[i] -> getRastName().replace("'","''") +	"'" +
        +")";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
  }
}

void MainWindowImpl::SchreibeHopfengabenDB()
{

  //zuerst alle vorhandenen Malzschüttungsdaten entfernen
  QSqlQuery query;
  //Malzauswahl 1
  QString sql = "DELETE FROM Hopfengaben WHERE SudID="+QString::number(AktuelleSudID);
  //Abfrage Abschicken
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

  for (int i=0; i < list_Hopfengaben.count(); i++){
    sql = "INSERT INTO Hopfengaben (SudID, Aktiv, Zeit, Prozent, erg_Hopfentext, erg_Menge, Alpha, Pellets, Vorderwuerze, Name)";
    //Prozentwert
    sql += " VALUES(";
    //SudID
    sql += QString::number(AktuelleSudID) + ", ";
    //Rast Aktiv
    sql += "'1', ";
    //Zeit
    sql += QString::number(list_Hopfengaben[i]->getKochzeit()) + ", ";
    //Prozentwert
    sql += QString::number(list_Hopfengaben[i]->getMengeProzent()) + ", ";
    //Ergebniss Text
    sql += "'";
    sql += list_Hopfengaben[i]->getErgebnistext().replace("'","''") + "', ";
    //Ergebniss Menge
    sql += QString::number(list_Hopfengaben[i]->getErgMenge()) + ", ";
    //Alpha Prozent und Pellets nur schreiben wenn nicht gebraut oder wert auf 0 ist
    QString Alphaprozent = "0";
    QString Pellets = "0";
    if (!BierWurdeGebraut || (list_Hopfengaben[i] -> getAlpha() == 0)){
      for (int o=0; o < tableWidget_Hopfen -> rowCount(); o++){
        if (list_Hopfengaben[i]->getName() == tableWidget_Hopfen -> item(o,TableHopfenColName) -> text()){
          QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(o,TableHopfenColAlpha);
          Alphaprozent = QString::number(spinBox->value());
          bool b = tableWidget_Hopfen -> item(o,TableHopfenColPellets) -> checkState();
          Pellets = QString::number(b);
        }
      }
    }
    else {
      Alphaprozent = QString::number(list_Hopfengaben[i]->getAlpha());
      Pellets = QString::number(list_Hopfengaben[i]->getPellets());
    }
    sql += Alphaprozent + ", ";
    //Pellets
    sql += Pellets + ", ";
    //Vorderwürzehopfung
    sql += QString::number(list_Hopfengaben[i]->getVWH())+", ";
    //Name
    sql += "'";
    sql += list_Hopfengaben[i]->getName().replace("'","''") + "'); ";

    //Abfrage Abschicken
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
  }
}

void MainWindowImpl::SchreibeMalzschuettungDB()
{
  //zuerst alle vorhandenen Malzschüttungsdaten entfernen
  QSqlQuery query;
  //Malzauswahl 1
  QString sql = "DELETE FROM Malzschuettung WHERE SudID="+QString::number(AktuelleSudID);
  //Abfrage Abschicken
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

  for (int i=0; i < list_Malzgaben.count(); i++){
    QSqlQuery query;
    //Malzauswahl 1
    QString sql = "INSERT INTO Malzschuettung (SudID, Prozent, erg_Menge, Farbe, Name)";
    //Prozentwert
    sql += " VALUES(";
    sql += QString::number(AktuelleSudID) + ", ";
    sql += QString::number(list_Malzgaben[i]->getMengeProzent()) + ", ";
    //Ergebnis Menge in Kilogramm
    sql += QString::number(list_Malzgaben[i]->getErgMenge()) + ", ";
    //Farbwert nur neu ermitteln wenn gespeicherter wert 0 ist
    QString farbe="0";
    QString Name = list_Malzgaben[i]->getName();
    if (!BierWurdeGebraut || (list_Malzgaben[i]->getFarbe() == 0)){
      for (int i=0; i < tableWidget_Malz -> rowCount(); i++){
        if (Name == tableWidget_Malz -> item(i,TableMalzColName) -> text()){
          QDoubleSpinBox* dsbFarbe = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,TableMalzColFarbe);
          farbe = QString::number(dsbFarbe ->value());
        }
      }
    }
    else {
      farbe = QString::number(list_Malzgaben[i]->getFarbe());
    }
    sql += farbe + ", '";
    //Name
    sql += Name.replace("'","''") + "') ";

    //Abfrage Abschicken
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
  }

}


void MainWindowImpl::BerKosten()
{
  double summe = 0;
  QPalette paletteF = spinBox_Haushaltszucker500mlFlasche -> palette();
  QPalette paletteN = spinBox_Haushaltszucker500mlFlasche -> palette();
  paletteF.setColor(QPalette::Base , Qt::red);

  //Kosten für die Schüttung berechnen
  double kg;
  double preis = 0;
  QString s;
  int z = 0;
  int gefunden = 0;
  bool KostenrechnungIO = true;
  bool Schuettung = true;
  bool Hopfen = true;
  bool Hefe = true;

  double kostenSchuettung = 0;
  for (int o=0; o < list_Malzgaben.count(); o++){
    s = list_Malzgaben[o]->getName();
    if (s != ""){
      kg = list_Malzgaben[o]->getErgMenge();
      for (int i=0; i < tableWidget_Malz -> rowCount(); i++){
        if (s == tableWidget_Malz -> item(i,TableMalzColName) -> text()){
          QDoubleSpinBox* dsbPreis = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,TableMalzColPreis);
          preis = dsbPreis ->value();
          kostenSchuettung += preis * kg;
          gefunden++;
        }
      }
      z++;
    }
  }
  //wenn nicht alle preise gefunden wurden kann die Kostenrechnung nicht vollständig erfolgen
  if (z != gefunden){
    KostenrechnungIO = false;
    Schuettung = false;
  }
  summe += kostenSchuettung;

  //Hopfenkosten berechnen
  double kostenHopfen = 0;
  z = 0;
  gefunden = 0;
  for (int o=0; o < list_Hopfengaben.count(); o++){
    s = list_Hopfengaben[o]->getName();
    kg = list_Hopfengaben[o]->getErgMenge()/1000;
    for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
      if (s == tableWidget_Hopfen -> item(i,TableHopfenColName) -> text()){
        QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,TableHopfenColPreis);
        preis = spinBox->value();
        kostenHopfen += preis * kg;
        gefunden++;
      }
    }
    z++;
  }
  //wenn nicht alle preise gefunden wurden kann die Kostenrechnung nicht vollständig erfolgen
  if (z != gefunden){
    KostenrechnungIO = false;
    Hopfen = false;
  }
  summe += kostenHopfen;

  //Hefekosten
  double kostenHefe = 0;
  //Hefeauswahl
  int AnzahlHefeEintraege = tableWidget_Hefe -> rowCount();
  int anzahl = 0;
  gefunden = 0;
  s = comboBox_AuswahlHefe -> currentText();
  if (s != ""){
    for (int i=0; i < AnzahlHefeEintraege; i++){
      //wenn Eintrag übereinstimmt
      if (tableWidget_Hefe -> item(i,TableHefeColName) -> text() == s){
        QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,TableHefeColPreis);
        preis = spinBox->value();
        //benötigte Hefemenge
        anzahl = spinBox_AnzahlHefeEinheiten->value();
        gefunden++;
      }
    }
  }
  //Hefekosten konnten nicht ermittelt werden
  if (gefunden == 0){
    KostenrechnungIO = false;
    Hefe = false;
  }
  else
    kostenHefe += preis * anzahl;

  summe += kostenHefe;


  //Kosten der Weiteren Zutaten
  double kostenWeitereZutaten = 0;
  for (int i=0; i < list_EwZutat.count(); i++){
    kostenWeitereZutaten += list_EwZutat[i] -> getErg_Kosten();
  }
  summe += kostenWeitereZutaten;

  //sonstige Kosten
  double kostenSonstiges = 0;
  kostenSonstiges += spinBox_Nebenkosten -> value();

  summe += kostenSonstiges;

  //Kosten der Brauanlage
  double kostenAnlage;
  kostenAnlage = getBrauanlageKosten();

  summe += kostenAnlage;

  QString str = "";
  int prozent = 0;
  prozent = round(kostenSchuettung/summe*100);
  str = trUtf8("Schüttung: ") + QString::number(prozent) + "% " + QString::number(round(kostenSchuettung*100)/100)+trUtf8(" €")+"\n";
  prozent = round(kostenHopfen/summe*100);
  str += trUtf8("Hopfen: ") + QString::number(prozent) + "% " + QString::number(round(kostenHopfen*100)/100)+trUtf8(" €")+"\n";
  prozent = round(kostenWeitereZutaten/summe*100);
  str += trUtf8("Weitere Zutaten: ") + QString::number(prozent) + "% " + QString::number(round(kostenWeitereZutaten*100)/100)+trUtf8(" €")+"\n";
  prozent = round(kostenHefe/summe*100);
  str += trUtf8("Hefe: ") + QString::number(prozent) + "% " + QString::number(round(kostenHefe*100)/100)+trUtf8(" €")+"\n";
  prozent = round(kostenSonstiges/summe*100);
  str += trUtf8("Nebenkosten: ") + QString::number(prozent) + "% " + QString::number(round(kostenSonstiges*100)/100)+trUtf8(" €")+"\n";
  prozent = round(kostenAnlage/summe*100);
  str += trUtf8("Brauanlage: ") + QString::number(prozent) + "% " + QString::number(round(kostenAnlage*100)/100)+trUtf8(" €")+"\n";
  str += trUtf8("gesamt: ") + QString::number(round(summe*100)/100)+trUtf8(" €") + " / " + QString::number(spinBox_JungbiermengeAbfuellen -> value()) + " Liter";

  if (KostenrechnungIO){
    spinBox_Preis -> setToolTip(str);
    spinBox_Preis -> setPalette(paletteN);
    spinBox_Preis -> setValue(summe / spinBox_JungbiermengeAbfuellen -> value());
  }
  else{
    str += trUtf8("\n\nKosten konnten nicht berechnet werden da die Angaben im Rezept unvollständig\noder die Auswahl in den Rohstoffdaten nicht mehr vorhanden ist\n\n");
    if (!Schuettung)
      str += trUtf8("Überprüfe die Malzschüttung\n");
    if (!Hopfen)
      str += trUtf8("Überprüfe die Hopfengaben\n");
    if (!Hefe)
      str += trUtf8("Überprüfe die Hefegabe");
    spinBox_Preis -> setToolTip(str);
    spinBox_Preis -> setPalette(paletteF);
    spinBox_Preis -> setValue(0);
  }
}

void MainWindowImpl::FuelleBrauuebersicht()
{
  //Daten abfragen
  QSqlQuery query, queryN;
  int FeldNr;
  QString sql, abfrage;
  int SelZeile = tableWidget_Brauuebersicht -> currentRow();

  if (SelZeile == -1)
    SelZeile = 0;

  QString datumB, datumE;
  datumB = dateEdit_AuswahlVon -> date().toString(Qt::ISODate);
  datumE = dateEdit_AuswahlBis -> date().toString(Qt::ISODate);
  abfrage = "SELECT * FROM Sud WHERE Braudatum BETWEEN date('" + datumB + "') AND date('" + datumE + "') AND BierWurdeAbgefuellt=1 ";
  sql = abfrage + " ORDER BY Braudatum DESC";

  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int i=0;
    tableWidget_Brauuebersicht -> clearContents();
    tableWidget_Brauuebersicht -> setRowCount(0);
    tableWidget_Brauuebersicht -> setSortingEnabled(false);
    while (query.next()){
      QTableWidgetItem *newItem1 = new QTableWidgetItem("");
      QTableWidgetItem *newItem2 = new QTableWidgetItem("");
      QTableWidgetItem *newItem3 = new QTableWidgetItem("");
      QTableWidgetItem *newItem4 = new QTableWidgetItem("");
      MyTableWidgetItemNumeric *newItem5 = new MyTableWidgetItemNumeric();
      MyTableWidgetItemNumeric *newItem6 = new MyTableWidgetItemNumeric();
      MyTableWidgetItemNumeric *newItem7 = new MyTableWidgetItemNumeric();
      MyTableWidgetItemNumeric *newItem8 = new MyTableWidgetItemNumeric();
      MyTableWidgetItemNumeric *newItem9 = new MyTableWidgetItemNumeric();
      MyTableWidgetItemNumeric *newItem10 = new MyTableWidgetItemNumeric();
      QTableWidgetItem *newItem11 = new QTableWidgetItem("");
      MyTableWidgetItemNumeric *newItem12 = new MyTableWidgetItemNumeric();
      MyTableWidgetItemNumeric *newItem13 = new MyTableWidgetItemNumeric();
      tableWidget_Brauuebersicht -> setRowCount(tableWidget_Brauuebersicht -> rowCount()+1);
      //ID
      FeldNr = query.record().indexOf("ID");
      newItem1 -> setText(query.value(FeldNr).toString());
      QString id = query.value(FeldNr).toString();
      tableWidget_Brauuebersicht -> setItem(i, 0, newItem1);
      //Sudname
      FeldNr = query.record().indexOf("Sudname");
      newItem2 -> setText(query.value(FeldNr).toString());
      tableWidget_Brauuebersicht -> setItem(i, 1, newItem2);
      //Bierfarbe setzten
      QColor farbe;
      FeldNr = query.record().indexOf("erg_Farbe");
      double ebc = query.value(FeldNr).toDouble();
      farbe = Berechnungen.GetFarbwert(query.value(FeldNr).toDouble());
      newItem2 -> setBackground(farbe);
      if (ebc > 35) {
        newItem2 -> setTextColor(QColor::fromRgb(255,255,255));
      }
      else
        newItem2 -> setTextColor(QColor::fromRgb(0,0,0));

      //Braudatum
      FeldNr = query.record().indexOf("Braudatum");
      newItem3 -> setData(Qt::DisplayRole, QDate::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      if (StyleDunkel)
        newItem3 -> setBackground(QColor::fromRgb(FARBE_BRAUUEBERSICHT_SPALTE_ACHSE_X_DARK));
      else
        newItem3 -> setBackground(QColor::fromRgb(FARBE_BRAUUEBERSICHT_SPALTE_ACHSE_X));
      tableWidget_Brauuebersicht -> setItem(i, 2, newItem3);
      //Abfülldatum
      FeldNr = query.record().indexOf("Abfuelldatum");
      newItem4 -> setData(Qt::DisplayRole, QDate::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      tableWidget_Brauuebersicht -> setItem(i, 3, newItem4);
      //Tage bis Bier fertig ist
      QDate date;
      //Start der Reifung ermitteln indem das letzte Datum vom
      //Nachgärverlauf benutzt wird
      QString sqlN = "SELECT * FROM Nachgaerverlauf WHERE SudID=" + id + " ORDER BY Zeitstempel DESC;";
      date = QDate::fromString(query.value(FeldNr).toString(),Qt::ISODate);

      if (!queryN.exec(sqlN)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + queryN.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sqlN);
      }
      else {
        if (queryN.first()){
          FeldNr = queryN.record().indexOf("Zeitstempel");
          date = QDate::fromString(queryN.value(FeldNr).toString(),Qt::ISODate);
        }
      }

      int tageReifung = date.daysTo(QDate::currentDate());
      FeldNr = query.record().indexOf("Reifezeit");
      date = date.addDays(query.value(FeldNr).toInt() * 7);
      int tage = QDate::currentDate().daysTo(date);
      FeldNr = query.record().indexOf("BierWurdeVerbraucht");
      bool bwv = query.value(FeldNr).toBool();
      if (bwv) {
        newItem11 -> setText(trUtf8("Verbraucht"));
        if (StyleDunkel)
          newItem11 -> setBackground(QColor::fromRgb(FARBE_BierLeer_DARK));
        else
          newItem11 -> setBackground(QColor::fromRgb(FARBE_BierLeer));
      }
      else if (tage > 0){
        int w = tageReifung / 7;
        newItem11 -> setText(trUtf8("%1. Woche").arg(w+1) + ", " + trUtf8("noch %1 Tage").arg(tage));
        newItem11 -> setTextColor(QColor::fromRgb(0,0,0));
        if (StyleDunkel)
          newItem11 -> setBackground(QColor::fromRgb(FARBE_BierReift_DARK));
        else
          newItem11 -> setBackground(QColor::fromRgb(FARBE_BierReift));
      }
      else {
        int w = tageReifung / 7;
        newItem11 -> setText(trUtf8("%1. Woche").arg(w+1));
        newItem11 -> setTextColor(QColor::fromRgb(0,0,0));
        if (StyleDunkel)
          newItem11 -> setBackground(QColor::fromRgb(FARBE_BierFertig_DARK));
        else
          newItem11 -> setBackground(QColor::fromRgb(FARBE_BierFertig));
      }
      tableWidget_Brauuebersicht -> setItem(i, 4, newItem11);
      //Menge
      FeldNr = query.record().indexOf("erg_AbgefuellteBiermenge");
      double d = query.value(FeldNr).toDouble();
      d = qRound(d * 100);
      d = d / 100;
      newItem5 -> setValue(d);
      tableWidget_Brauuebersicht -> setItem(i, 5, newItem5);
      if (comboBox_AuswahlL1 -> currentText() == trUtf8("Menge")){
        newItem5 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem5 -> setBackground(ColorBrauUebersicht_Spalte1);
      }
      if (comboBox_AuswahlL2 -> currentText() == trUtf8("Menge")){
        newItem5 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem5 -> setBackground(ColorBrauUebersicht_Spalte2);
      }
      //Stammwürze
      FeldNr = query.record().indexOf("SWAnstellen");
      double sw = query.value(FeldNr).toDouble();
      sw = qRound(sw * 100);
      sw = sw / 100;
      newItem6 -> setValue(sw);
      tableWidget_Brauuebersicht -> setItem(i, 6, newItem6);
      if (comboBox_AuswahlL1 -> currentText() == trUtf8("Stammwürze")){
        newItem6 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem6 -> setBackground(ColorBrauUebersicht_Spalte1);
      }
      if (comboBox_AuswahlL2 -> currentText() == trUtf8("Stammwürze")){
        newItem6 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem6 -> setBackground(ColorBrauUebersicht_Spalte2);
      }
      //Sudhausausbeute
      FeldNr = query.record().indexOf("erg_Sudhausausbeute");
      double sha = query.value(FeldNr).toDouble();
      sha = qRound(sha * 100);
      sha = sha / 100;
      newItem7 -> setValue(sha);
      tableWidget_Brauuebersicht -> setItem(i, 7, newItem7);
      if (comboBox_AuswahlL1 -> currentText() == trUtf8("Sudhausausbeute")){
        newItem7 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem7 -> setBackground(ColorBrauUebersicht_Spalte1);
      }
      if (comboBox_AuswahlL2 -> currentText() == trUtf8("Sudhausausbeute")){
        newItem7 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem7 -> setBackground(ColorBrauUebersicht_Spalte2);
      }
      //Menge Schüttung
      FeldNr = query.record().indexOf("erg_S_Gesammt");
      double msch = query.value(FeldNr).toDouble();
      msch = qRound(msch * 100);
      msch = msch / 100;
      newItem8 -> setValue(msch);
      tableWidget_Brauuebersicht -> setItem(i, 8, newItem8);
      if (comboBox_AuswahlL1 -> currentText() == trUtf8("Menge Schüttung")){
        newItem8 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem8 -> setBackground(ColorBrauUebersicht_Spalte1);
      }
      if (comboBox_AuswahlL2 -> currentText() == trUtf8("Menge Schüttung")){
        newItem8 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem8 -> setBackground(ColorBrauUebersicht_Spalte2);
      }
      //Kosten Pro Liter
      FeldNr = query.record().indexOf("erg_Preis");
      double kost = query.value(FeldNr).toDouble();
      kost = qRound(kost * 100);
      kost = kost / 100;
      newItem9 -> setValue(kost);
      tableWidget_Brauuebersicht -> setItem(i, 9, newItem9);
      if (comboBox_AuswahlL1 -> currentText() == trUtf8("Kosten/Liter")){
        newItem9 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem9 -> setBackground(ColorBrauUebersicht_Spalte1);
      }
      if (comboBox_AuswahlL2 -> currentText() == trUtf8("Kosten/Liter")){
        newItem9 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem9 -> setBackground(ColorBrauUebersicht_Spalte2);
      }
      //Alc Vol
      FeldNr = query.record().indexOf("erg_Alkohol");
      double abv = query.value(FeldNr).toDouble();
      abv = qRound(abv * 100);
      abv = abv / 100;
      newItem10 -> setValue(abv);
      tableWidget_Brauuebersicht -> setItem(i, 10, newItem10);
      if (comboBox_AuswahlL1 -> currentText() == trUtf8("Alkohol")){
        newItem10 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem10 -> setBackground(ColorBrauUebersicht_Spalte1);
      }
      if (comboBox_AuswahlL2 -> currentText() == trUtf8("Alkohol")){
        newItem10 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem10 -> setBackground(ColorBrauUebersicht_Spalte2);
      }
      //Scheinbarer Endvergärungsgrad
      //wenn keine Schnellgärprobe gemacht wurde restextrakt nehmen
      FeldNr = query.record().indexOf("SchnellgaerprobeAktiv");
      if (query.value(FeldNr).toBool()){
        FeldNr = query.record().indexOf("SWSchnellgaerprobe");
      }
      else {
        FeldNr = query.record().indexOf("SWJungbier");
      }
      double svp = query.value(FeldNr).toDouble();
      double evg = qRound(Berechnungen.GetScheinbarerEVG(sw,svp)*100);
      evg = evg / 100;
      newItem12 -> setValue(evg);
      tableWidget_Brauuebersicht -> setItem(i, 11, newItem12);
      if (comboBox_AuswahlL1 -> currentText() == trUtf8("Scheinbarer EVG")){
        newItem12 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem12 -> setBackground(ColorBrauUebersicht_Spalte1);
      }
      if (comboBox_AuswahlL2 -> currentText() == trUtf8("Scheinbarer EVG")){
        newItem12 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem12 -> setBackground(ColorBrauUebersicht_Spalte2);
      }
      //Effektive Ausbeute
      FeldNr = query.record().indexOf("erg_EffektiveAusbeute");
      d = query.value(FeldNr).toDouble();
      d = qRound(d*100);
      d = d / 100;
      newItem13 -> setValue(d);
      tableWidget_Brauuebersicht -> setItem(i, 12, newItem13);
      if (comboBox_AuswahlL1 -> currentText() == trUtf8("Effektive Ausbeute")){
        newItem13 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem13 -> setBackground(ColorBrauUebersicht_Spalte1);
      }
      if (comboBox_AuswahlL2 -> currentText() == trUtf8("Effektive Ausbeute")){
        newItem13 -> setTextColor(QColor::fromRgb(0,0,0));
        newItem13 -> setBackground(ColorBrauUebersicht_Spalte2);
      }
      //Mittig ausrichten
      newItem3 -> setTextAlignment(Qt::AlignCenter);
      newItem4 -> setTextAlignment(Qt::AlignCenter);
      newItem5 -> setTextAlignment(Qt::AlignCenter);
      newItem6 -> setTextAlignment(Qt::AlignCenter);
      newItem7 -> setTextAlignment(Qt::AlignCenter);
      newItem8 -> setTextAlignment(Qt::AlignCenter);
      newItem9 -> setTextAlignment(Qt::AlignCenter);
      newItem10 -> setTextAlignment(Qt::AlignCenter);
      newItem11 -> setTextAlignment(Qt::AlignCenter);
      newItem12 -> setTextAlignment(Qt::AlignCenter);
      newItem13 -> setTextAlignment(Qt::AlignCenter);

      i++;
    }
    tableWidget_Brauuebersicht -> setSortingEnabled(true);

    //Diagramm füllen wenn 2 oder mehr Datensätze vorhanden sind
    if (i > 1){
      sql = abfrage + " ORDER BY Braudatum";
      //Linie 1
      widget_DiaBrauUebersicht -> L1Daten.clear();
      widget_DiaBrauUebersicht -> L1Datum.clear();
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
      else {
        while (query.next()){
          //Menge
          if (comboBox_AuswahlL1 -> currentText() == trUtf8("Menge")){
            FeldNr = query.record().indexOf("erg_AbgefuellteBiermenge");
            double d = query.value(FeldNr).toDouble();
            d = qRound(d*100);
            d = d / 100;
            widget_DiaBrauUebersicht -> L1Daten.append(d);
            widget_DiaBrauUebersicht -> BezeichnungL1 = trUtf8("Liter");
            widget_DiaBrauUebersicht -> KurzbezeichnungL1 = trUtf8("L");
          }
          //Sudhausausbeute
          else if (comboBox_AuswahlL1 -> currentText() == trUtf8("Sudhausausbeute")){
            FeldNr = query.record().indexOf("erg_Sudhausausbeute");
            widget_DiaBrauUebersicht -> L1Daten.append(query.value(FeldNr).toDouble());
            widget_DiaBrauUebersicht -> BezeichnungL1 = trUtf8("%");
            widget_DiaBrauUebersicht -> KurzbezeichnungL1 = trUtf8("%");
          }
          //Stammwürze
          else if (comboBox_AuswahlL1 -> currentText() == trUtf8("Stammwürze")){
            FeldNr = query.record().indexOf("SWAnstellen");
            widget_DiaBrauUebersicht -> L1Daten.append(query.value(FeldNr).toDouble());
            widget_DiaBrauUebersicht -> BezeichnungL1 = trUtf8("°P");
            widget_DiaBrauUebersicht -> KurzbezeichnungL1 = trUtf8("°P");
          }
          //Menge Schüttung
          else if (comboBox_AuswahlL1 -> currentText() == trUtf8("Menge Schüttung")){
            FeldNr = query.record().indexOf("erg_S_Gesammt");
            widget_DiaBrauUebersicht -> L1Daten.append(query.value(FeldNr).toDouble());
            widget_DiaBrauUebersicht -> BezeichnungL1 = trUtf8("kg");
            widget_DiaBrauUebersicht -> KurzbezeichnungL1 = trUtf8("kg");
          }
          //Kosten / Liter
          else if (comboBox_AuswahlL1 -> currentText() == trUtf8("Kosten/Liter")){
            FeldNr = query.record().indexOf("erg_Preis");
            widget_DiaBrauUebersicht -> L1Daten.append(query.value(FeldNr).toDouble());
            widget_DiaBrauUebersicht -> BezeichnungL1 = trUtf8("€");
            widget_DiaBrauUebersicht -> KurzbezeichnungL1 = trUtf8("€");
          }
          //Alkohol
          else if (comboBox_AuswahlL1 -> currentText() == trUtf8("Alkohol")){
            FeldNr = query.record().indexOf("erg_Alkohol");
            widget_DiaBrauUebersicht -> L1Daten.append(query.value(FeldNr).toDouble());
            widget_DiaBrauUebersicht -> BezeichnungL1 = trUtf8("%vol");
            widget_DiaBrauUebersicht -> KurzbezeichnungL1 = trUtf8("%");
          }
          //Scheinbarer EVG
          else if (comboBox_AuswahlL1 -> currentText() == trUtf8("Scheinbarer EVG")){
            FeldNr = query.record().indexOf("SWAnstellen");
            double sw = query.value(FeldNr).toDouble();
            //wenn keine Schnellgärprobe gemacht wurde restextrakt nehmen
            FeldNr = query.record().indexOf("SchnellgaerprobeAktiv");
            if (query.value(FeldNr).toBool()){
              FeldNr = query.record().indexOf("SWSchnellgaerprobe");
            }
            else {
              FeldNr = query.record().indexOf("SWJungbier");
            }
            double svp = query.value(FeldNr).toDouble();
            double evg = qRound(Berechnungen.GetScheinbarerEVG(sw,svp)*100);
            evg = evg / 100;
            widget_DiaBrauUebersicht -> L1Daten.append(evg);
            widget_DiaBrauUebersicht -> BezeichnungL1 = trUtf8("%");
            widget_DiaBrauUebersicht -> KurzbezeichnungL1 = trUtf8("%");
          }
          //Effektive Ausbeute
          else if (comboBox_AuswahlL1 -> currentText() == trUtf8("Effektive Ausbeute")){
            FeldNr = query.record().indexOf("erg_EffektiveAusbeute");
            double d = query.value(FeldNr).toDouble();
            d = qRound(d*100);
            d = d / 100;
            widget_DiaBrauUebersicht -> L1Daten.append(d);
            widget_DiaBrauUebersicht -> BezeichnungL1 = trUtf8("%");
            widget_DiaBrauUebersicht -> KurzbezeichnungL1 = trUtf8("%");
          }
          FeldNr = query.record().indexOf("Braudatum");
          QDateTime dt = QDateTime::fromString(query.value(FeldNr).toString()+"T12:00:00",Qt::ISODate);
          //QMessageBox::information ( this, "", dt.toString(Qt::ISODate)) ;

          widget_DiaBrauUebersicht -> L1Datum.append(dt);
        }
      }
      //Linie 2
      widget_DiaBrauUebersicht -> L2Daten.clear();
      widget_DiaBrauUebersicht -> L2Datum.clear();
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
      else {
        while (query.next()){
          //Menge
          if (comboBox_AuswahlL2 -> currentText() == trUtf8("Menge")){
            FeldNr = query.record().indexOf("erg_AbgefuellteBiermenge");
            double d = query.value(FeldNr).toDouble();
            d = qRound(d*100);
            d = d / 100;
            widget_DiaBrauUebersicht -> L2Daten.append(d);
            widget_DiaBrauUebersicht -> BezeichnungL2 = trUtf8("Liter");
            widget_DiaBrauUebersicht -> KurzbezeichnungL2 = trUtf8("L");
          }
          //Sudhausausbeute
          else if (comboBox_AuswahlL2 -> currentText() == trUtf8("Sudhausausbeute")){
            FeldNr = query.record().indexOf("erg_Sudhausausbeute");
            widget_DiaBrauUebersicht -> L2Daten.append(query.value(FeldNr).toDouble());
            widget_DiaBrauUebersicht -> BezeichnungL2 = trUtf8("%");
            widget_DiaBrauUebersicht -> KurzbezeichnungL2 = trUtf8("%");
          }
          //Stammwürze
          else if (comboBox_AuswahlL2 -> currentText() == trUtf8("Stammwürze")){
            FeldNr = query.record().indexOf("SWAnstellen");
            widget_DiaBrauUebersicht -> L2Daten.append(query.value(FeldNr).toDouble());
            widget_DiaBrauUebersicht -> BezeichnungL2 = trUtf8("°P");
            widget_DiaBrauUebersicht -> KurzbezeichnungL2 = trUtf8("°P");
          }
          //Menge Schüttung
          else if (comboBox_AuswahlL2 -> currentText() == trUtf8("Menge Schüttung")){
            FeldNr = query.record().indexOf("erg_S_Gesammt");
            widget_DiaBrauUebersicht -> L2Daten.append(query.value(FeldNr).toDouble());
            widget_DiaBrauUebersicht -> BezeichnungL2 = trUtf8("kg");
            widget_DiaBrauUebersicht -> KurzbezeichnungL2 = trUtf8("kg");
          }
          //Kosten / Liter
          else if (comboBox_AuswahlL2 -> currentText() == trUtf8("Kosten/Liter")){
            FeldNr = query.record().indexOf("erg_Preis");
            widget_DiaBrauUebersicht -> L2Daten.append(query.value(FeldNr).toDouble());
            widget_DiaBrauUebersicht -> BezeichnungL2 = trUtf8("€");
            widget_DiaBrauUebersicht -> KurzbezeichnungL2 = trUtf8("€");
          }
          //Alkohol
          else if (comboBox_AuswahlL2 -> currentText() == trUtf8("Alkohol")){
            FeldNr = query.record().indexOf("erg_Alkohol");
            widget_DiaBrauUebersicht -> L2Daten.append(query.value(FeldNr).toDouble());
            widget_DiaBrauUebersicht -> BezeichnungL2 = trUtf8("vol%");
            widget_DiaBrauUebersicht -> KurzbezeichnungL2 = trUtf8("%");
          }
          //Scheinbarer EVG
          else if (comboBox_AuswahlL2 -> currentText() == trUtf8("Scheinbarer EVG")){
            FeldNr = query.record().indexOf("SWAnstellen");
            double sw = query.value(FeldNr).toDouble();
            //wenn keine Schnellgärprobe gemacht wurde restextrakt nehmen
            FeldNr = query.record().indexOf("SchnellgaerprobeAktiv");
            if (query.value(FeldNr).toBool()){
              FeldNr = query.record().indexOf("SWSchnellgaerprobe");
            }
            else {
              FeldNr = query.record().indexOf("SWJungbier");
            }
            double svp = query.value(FeldNr).toDouble();
            double evg = qRound(Berechnungen.GetScheinbarerEVG(sw,svp)*100);
            evg = evg / 100;
            widget_DiaBrauUebersicht -> L2Daten.append(evg);
            widget_DiaBrauUebersicht -> BezeichnungL2 = trUtf8("%");
            widget_DiaBrauUebersicht -> KurzbezeichnungL2 = trUtf8("%");
          }
          //Effektive Ausbeute
          else if (comboBox_AuswahlL2 -> currentText() == trUtf8("Effektive Ausbeute")){
            FeldNr = query.record().indexOf("erg_EffektiveAusbeute");
            double d = query.value(FeldNr).toDouble();
            d = qRound(d*100);
            d = d / 100;
            widget_DiaBrauUebersicht -> L2Daten.append(d);
            widget_DiaBrauUebersicht -> BezeichnungL2 = trUtf8("%");
            widget_DiaBrauUebersicht -> KurzbezeichnungL2 = trUtf8("%");
          }
          FeldNr = query.record().indexOf("Braudatum");
          QDateTime dt = QDateTime::fromString(query.value(FeldNr).toString()+"T12:00:00",Qt::ISODate);
          widget_DiaBrauUebersicht -> L2Datum.append(dt);
        }
      }
    }
  }
}


void MainWindowImpl::on_comboBox_AuswahlL1_currentIndexChanged(int )
{
  FuelleBrauuebersicht();
  widget_DiaBrauUebersicht -> repaint();
}

void MainWindowImpl::on_comboBox_AuswahlL2_currentIndexChanged(int )
{
  FuelleBrauuebersicht();
  widget_DiaBrauUebersicht -> repaint();
}

void MainWindowImpl::on_pushButton_BrauuebersichtRefresh_clicked()
{
  FuelleBrauuebersicht();
  widget_DiaBrauUebersicht -> repaint();
}

void MainWindowImpl::BerEffektiveAusbeuteMittel()
{
  //Berechnette Effektive Ausbeuet abfragen
  QSqlQuery query;
  QString sql;
  double erg = 0;
  int i=0;
  int AnlagenID = 0;
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    AnlagenID = item->getID();
  }
  sql = "SELECT erg_EffektiveAusbeute FROM Sud WHERE BierWurdeGebraut=1 AND AusbeuteIgnorieren=0 AND AuswahlBrauanlage="+QString::number(AnlagenID);
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    while (query.next()){
      i++;
      if (query.value(0) != 0){
        erg += query.value(0).toDouble();
      }
    }
    if (i > 0){
      erg = erg / i;
      hSlider_dAusbeuteSude->setMinimum(1);
    }
    else
      erg = 0;
    hSlider_dAusbeuteSude->setMaximum(i);
    hSlider_dAusbeuteSude->setValue(i);
    spinBox_AusbeuteEffektiv_Mittel -> setValue(erg);
  }
}


void MainWindowImpl::FuelleGaerverlauf()
{
  // Aktuelles Datum und Uhrzeit eintragen
  dateTimeEdit_Schnellgaerprobe -> setDateTime(QDateTime::currentDateTime());
  dateTimeEdit_Hautgaerprobe -> setDateTime(QDateTime::currentDateTime());
  dateTimeEdit_Nachgaerprobe -> setDateTime(QDateTime::currentDateTime());
  widget_DiaSchnellgaerverlauf -> BezeichnungL1 = trUtf8("Restextrakt °P");
  widget_DiaSchnellgaerverlauf -> KurzbezeichnungL1 = trUtf8("°P");
  widget_DiaSchnellgaerverlauf -> BezeichnungL2 = trUtf8("Alkoholgehalt vol%");
  widget_DiaSchnellgaerverlauf -> KurzbezeichnungL2 = trUtf8("%");
  widget_DiaSchnellgaerverlauf -> BezeichnungL3 = trUtf8("Temperatur °C");
  widget_DiaSchnellgaerverlauf -> KurzbezeichnungL3 = trUtf8("°C");

  widget_DiaHauptgaerverlauf -> BezeichnungL1 = trUtf8("Restextrakt °P");
  widget_DiaHauptgaerverlauf -> KurzbezeichnungL1 = trUtf8("°P");
  widget_DiaHauptgaerverlauf -> BezeichnungL2 = trUtf8("Alkoholgehalt vol%");
  widget_DiaHauptgaerverlauf -> KurzbezeichnungL2 = trUtf8("%");
  widget_DiaHauptgaerverlauf -> BezeichnungL3 = trUtf8("Temperatur °C");
  widget_DiaHauptgaerverlauf -> KurzbezeichnungL3 = trUtf8("°C");

  widget_DiaNachgaerverlauf -> BezeichnungL1 = trUtf8("CO2 Gehalt g/L");
  widget_DiaNachgaerverlauf -> KurzbezeichnungL1 = trUtf8("g/L");
  widget_DiaNachgaerverlauf -> BezeichnungL2 = trUtf8("Temperatur °C");
  widget_DiaNachgaerverlauf -> KurzbezeichnungL2 = trUtf8("°C");
}


void MainWindowImpl::on_pushButton_AddSchnellgaerMessung_clicked()
{
  QDateTime dt;
  double sw, alc, temperatur;
  // Eintrag für den Schnellgärverlauf der Tabelle hinzufügen
  QTableWidgetItem *newItem1 = new QTableWidgetItem();
  QTableWidgetItem *newItem2 = new QTableWidgetItem();
  QTableWidgetItem *newItem3 = new QTableWidgetItem();
  QTableWidgetItem *newItem4 = new QTableWidgetItem();
  dt = dateTimeEdit_Schnellgaerprobe -> dateTime();
  newItem1 -> setData(Qt::DisplayRole, dt);
  sw = doubleSpinBox_SWSchnellgaerprobe -> value();
  newItem2 -> setData(Qt::DisplayRole, sw);
  //Alkoholgehalt berechnen
  alc = Berechnungen.BerAlkohoVol(spinBox_SWSollGesammt -> value(), doubleSpinBox_SWSchnellgaerprobe -> value() );
  alc = double(qRound(alc * 10)) / 10;
  newItem3 -> setData(Qt::DisplayRole, alc);
  newItem3->setFlags(newItem3->flags() & (~Qt::ItemIsEditable));
  //Temperatur
  temperatur = doubleSpinBox_TempSchnellgaerprobe -> value();
  newItem4 -> setData(Qt::DisplayRole, temperatur);
  int i = tableWidget_Schnellgaerverlauf -> rowCount();

  //Mittig ausrichten
  newItem1 -> setTextAlignment(Qt::AlignCenter);
  newItem2 -> setTextAlignment(Qt::AlignCenter);
  newItem3 -> setTextAlignment(Qt::AlignCenter);
  newItem4 -> setTextAlignment(Qt::AlignCenter);

  AmLaden = true;
  tableWidget_Schnellgaerverlauf -> insertRow(i);
  tableWidget_Schnellgaerverlauf -> setItem(i, 0, newItem1);
  tableWidget_Schnellgaerverlauf -> setItem(i, 1, newItem2);
  tableWidget_Schnellgaerverlauf -> setItem(i, 2, newItem3);
  tableWidget_Schnellgaerverlauf -> setItem(i, 3, newItem4);
  AmLaden = false;

  //Diagramm füllen
  FuelleDiagrammSchnellgaerverlauf();
  setAenderung(true);

  //Wenn Bier noch nicht abgefüllt wurde dann in Wert in Brau & Gärdaten übernehmen
  if (!BierWurdeAbgefuellt){
    spinBox_SWSchnellgaerprobe -> setValue(sw);
  }
}

void MainWindowImpl::SchreibeSchnellgaerverlaufDB()
{
  QSqlQuery query;

  //Zuerst alle Einträge in der Tabelle löschen
  QString sql = "DELETE FROM Schnellgaerverlauf WHERE SudID = " + QString::number(AktuelleSudID) + ";";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

  // Tabelleninhalt schreiben
  for (int i=0; i < tableWidget_Schnellgaerverlauf -> rowCount(); i++){
    QDateTime dt;
    dt = tableWidget_Schnellgaerverlauf -> item(i,0) -> data(Qt::DisplayRole).toDateTime();
    sql = "INSERT INTO Schnellgaerverlauf(SudID, Zeitstempel, SW, Alc, Temp) VALUES(" +
        QString::number(AktuelleSudID) +	"," +
        "'" + dt.toString(Qt::ISODate) + "'," +
        tableWidget_Schnellgaerverlauf -> item(i,1) -> text() + "," +
        tableWidget_Schnellgaerverlauf -> item(i,2) -> text() + "," +
        tableWidget_Schnellgaerverlauf -> item(i,3) -> text() + ")";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
  }
}


void MainWindowImpl::LeseSchnellgaerverlaufDB()
{
  QSqlQuery query;
  int FeldNr;

  //Tabelle einlesen
  QString sql = "SELECT * FROM Schnellgaerverlauf WHERE SudID=" + QString::number(AktuelleSudID) + " ORDER BY Zeitstempel;";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int i=0;
    tableWidget_Schnellgaerverlauf -> clearContents();
    tableWidget_Schnellgaerverlauf -> setRowCount(0);
    double sw = 0;
    double temperatur = 0;
    while (query.next()){
      QTableWidgetItem *newItem1 = new QTableWidgetItem("");
      QTableWidgetItem *newItem2 = new QTableWidgetItem("");
      QTableWidgetItem *newItem3 = new QTableWidgetItem("");
      QTableWidgetItem *newItem4 = new QTableWidgetItem("");
      tableWidget_Schnellgaerverlauf -> setRowCount(tableWidget_Schnellgaerverlauf -> rowCount()+1);
      //Zeitstempel
      FeldNr = query.record().indexOf("Zeitstempel");
      newItem1 -> setData(Qt::DisplayRole, QDateTime::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      tableWidget_Schnellgaerverlauf -> setItem(i, 0, newItem1);
      //Stammwürze
      FeldNr = query.record().indexOf("SW");
      sw = query.value(FeldNr).toDouble();
      newItem2 -> setData(Qt::DisplayRole, sw);
      tableWidget_Schnellgaerverlauf -> setItem(i, 1, newItem2);
      //Alkoholgehalt wird berechnet
      double alc = Berechnungen.BerAlkohoVol(spinBox_SWSollGesammt -> value(), sw );
      alc = double(qRound(alc * 10)) / 10;
      newItem3 -> setData(Qt::DisplayRole, alc);
      newItem3->setFlags(newItem3->flags() & (~Qt::ItemIsEditable));
      tableWidget_Schnellgaerverlauf -> setItem(i, 2, newItem3);
      //Temperatur
      FeldNr = query.record().indexOf("Temp");
      temperatur = query.value(FeldNr).toDouble();
      newItem4 -> setData(Qt::DisplayRole, temperatur);
      tableWidget_Schnellgaerverlauf -> setItem(i, 3, newItem4);
      i++;
      //Mittig ausrichten
      newItem1 -> setTextAlignment(Qt::AlignCenter);
      newItem2 -> setTextAlignment(Qt::AlignCenter);
      newItem3 -> setTextAlignment(Qt::AlignCenter);
      newItem4 -> setTextAlignment(Qt::AlignCenter);
    }
    tableWidget_Schnellgaerverlauf->setSortingEnabled(true);
    tableWidget_Schnellgaerverlauf->setSortingEnabled(false);
    FuelleDiagrammSchnellgaerverlauf();
    //Eingabefelder mit den letzten werten füttern
    doubleSpinBox_SWSchnellgaerprobe -> setValue(sw);
    doubleSpinBox_TempSchnellgaerprobe -> setValue(temperatur);
  }
}


void MainWindowImpl::FuelleDiagrammSchnellgaerverlauf()
{
  // Tabelle in Diagramm Schreiben
  widget_DiaSchnellgaerverlauf -> DiagrammLeeren();
  for (int i=0; i < tableWidget_Schnellgaerverlauf -> rowCount(); i++){
    QDateTime dt;
    dt = tableWidget_Schnellgaerverlauf -> item(i,0) -> data(Qt::DisplayRole).toDateTime();
    widget_DiaSchnellgaerverlauf -> L1Datum.append(dt);
    widget_DiaSchnellgaerverlauf -> L2Datum.append(dt);
    widget_DiaSchnellgaerverlauf -> L3Datum.append(dt);
    widget_DiaSchnellgaerverlauf -> L1Daten.append(tableWidget_Schnellgaerverlauf -> item(i,1) -> data(Qt::DisplayRole).toDouble());
    widget_DiaSchnellgaerverlauf -> L2Daten.append(tableWidget_Schnellgaerverlauf -> item(i,2) -> data(Qt::DisplayRole).toDouble());
    widget_DiaSchnellgaerverlauf -> L3Daten.append(tableWidget_Schnellgaerverlauf -> item(i,3) -> data(Qt::DisplayRole).toDouble());
  }
  widget_DiaSchnellgaerverlauf -> repaint();
}


void MainWindowImpl::on_pushButton_EingabeSWSchnellgaerverlauf_clicked()
{
  KorrektSwImpl ksw;
  ksw.spinBox_SwPlato -> setValue(doubleSpinBox_SWSchnellgaerprobe -> value());
  ksw.spinBox_Temperatur -> setValue(doubleSpinBox_TempSchnellgaerprobe -> value());
  ksw.setSWAnstellen(spinBox_SWAnstellen -> value());
  ksw.BerDichte();
  ksw.exec();
  if (!ksw.abgebrochen){
    if (ksw.spinBox_SwBrix -> value() == 0){
      doubleSpinBox_SWSchnellgaerprobe -> setValue(Berechnungen.densityAtX(ksw.spinBox_SwPlato -> value(), ksw.spinBox_Temperatur -> value(), 20));
    }
    else {
      doubleSpinBox_SWSchnellgaerprobe -> setValue(ksw.spinBox_SwPlato -> value());
    }
    doubleSpinBox_TempSchnellgaerprobe -> setValue(ksw.spinBox_Temperatur -> value());
  }
  ksw.setSWAnstellen(0);
}

void MainWindowImpl::on_pushButton_DelSchnellgaerMessung_clicked()
{
  tableWidget_Schnellgaerverlauf -> removeRow(tableWidget_Schnellgaerverlauf -> currentRow());
  FuelleDiagrammSchnellgaerverlauf();
  setAenderung(true);
}

void MainWindowImpl::on_pushButton_AddHauptgaerMessung_clicked()
{
  QDateTime dt;
  double sw, alc, temperatur;
  // Eintrag für den Schnellgärverlauf der Tabelle hinzufügen
  QTableWidgetItem *newItem1 = new QTableWidgetItem();
  QTableWidgetItem *newItem2 = new QTableWidgetItem();
  QTableWidgetItem *newItem3 = new QTableWidgetItem();
  QTableWidgetItem *newItem4 = new QTableWidgetItem();
  dt = dateTimeEdit_Hautgaerprobe -> dateTime();
  newItem1 -> setData(Qt::DisplayRole, dt);
  sw = doubleSpinBox_SWHauptgaerprobe -> value();
  newItem2 -> setData(Qt::DisplayRole, sw);
  //Alkoholgehalt berechnen
  alc = Berechnungen.BerAlkohoVol(spinBox_SWSollGesammt -> value(), sw );
  alc = double(qRound(alc * 10)) / 10;
  newItem3 -> setData(Qt::DisplayRole, alc);
  newItem3->setFlags(newItem3->flags() & (~Qt::ItemIsEditable));
  //Temperatur
  temperatur = doubleSpinBox_TempHauptgaerprobe -> value();
  newItem4 -> setData(Qt::DisplayRole, temperatur);
  int i = tableWidget_Hauptgaerverlauf -> rowCount();

  //Mittig ausrichten
  newItem1 -> setTextAlignment(Qt::AlignCenter);
  newItem2 -> setTextAlignment(Qt::AlignCenter);
  newItem3 -> setTextAlignment(Qt::AlignCenter);
  newItem4 -> setTextAlignment(Qt::AlignCenter);

  AmLaden = true;
  tableWidget_Hauptgaerverlauf -> insertRow(i);
  tableWidget_Hauptgaerverlauf -> setItem(i, 0, newItem1);
  tableWidget_Hauptgaerverlauf -> setItem(i, 1, newItem2);
  tableWidget_Hauptgaerverlauf -> setItem(i, 2, newItem3);
  tableWidget_Hauptgaerverlauf -> setItem(i, 3, newItem4);
  AmLaden = false;

  //Diagramm füllen
  FuelleDiagrammHauptgaerverlauf();
  setAenderung(true);
  AenderungHauptgaerverlauf = true;

  //Wenn Bier noch nicht abgefüllt wurde dann in Wert in Brau & Gärdaten übernehmen
  if (!BierWurdeAbgefuellt){
    spinBox_SWJungbier -> setValue(sw);
    spinBox_TemperaturJungbier -> setValue(temperatur);
  }
}

void MainWindowImpl::SchreibeHauptgaerverlaufDB()
{
  if (AenderungHauptgaerverlauf){
    QSqlQuery query;

    //Zuerst alle Einträge in der Tabelle löschen
    QString sql = "DELETE FROM Hauptgaerverlauf WHERE SudID = " + QString::number(AktuelleSudID) + ";";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }

    // Tabelleninhalt schreiben
    for (int i=0; i < tableWidget_Hauptgaerverlauf -> rowCount(); i++){
      QDateTime dt;
      dt = tableWidget_Hauptgaerverlauf -> item(i,0) -> data(Qt::DisplayRole).toDateTime();
      sql = "INSERT INTO Hauptgaerverlauf(SudID, Zeitstempel, SW, Alc, Temp) VALUES(" +
          QString::number(AktuelleSudID) +	"," +
          "'" + dt.toString(Qt::ISODate) + "'," +
          tableWidget_Hauptgaerverlauf -> item(i,1) -> text() + "," +
          tableWidget_Hauptgaerverlauf -> item(i,2) -> text() + "," +
          tableWidget_Hauptgaerverlauf -> item(i,3) -> text() + ")";
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
    }
    AenderungHauptgaerverlauf = false;
  }
}


void MainWindowImpl::LeseHauptgaerverlaufDB()
{
  QSqlQuery query;
  int FeldNr;

  //Tabelle einlesen
  QString sql = "SELECT * FROM Hauptgaerverlauf WHERE SudID=" + QString::number(AktuelleSudID) + " ORDER BY Zeitstempel;";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int i=0;
    tableWidget_Hauptgaerverlauf -> clearContents();
    tableWidget_Hauptgaerverlauf -> setRowCount(0);
    double sw = 0;
    double temperatur = 0;
    while (query.next()){
      QTableWidgetItem *newItem1 = new QTableWidgetItem("");
      QTableWidgetItem *newItem2 = new QTableWidgetItem("");
      QTableWidgetItem *newItem3 = new QTableWidgetItem("");
      QTableWidgetItem *newItem4 = new QTableWidgetItem("");
      tableWidget_Hauptgaerverlauf -> setRowCount(tableWidget_Hauptgaerverlauf -> rowCount()+1);
      //Zeitstempel
      FeldNr = query.record().indexOf("Zeitstempel");
      newItem1 -> setData(Qt::DisplayRole, QDateTime::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      tableWidget_Hauptgaerverlauf -> setItem(i, 0, newItem1);
      //Stammwürze
      FeldNr = query.record().indexOf("SW");
      sw = query.value(FeldNr).toDouble();
      newItem2 -> setData(Qt::DisplayRole, sw);
      tableWidget_Hauptgaerverlauf -> setItem(i, 1, newItem2);
      //Alkoholgehalt wird berechnet
      double alc = Berechnungen.BerAlkohoVol(spinBox_SWSollGesammt -> value(), sw );
      alc = double(qRound(alc * 10)) / 10;
      newItem3 -> setData(Qt::DisplayRole, alc);
      newItem3->setFlags(newItem3->flags() & (~Qt::ItemIsEditable));
      tableWidget_Hauptgaerverlauf -> setItem(i, 2, newItem3);
      //Temperatur
      FeldNr = query.record().indexOf("Temp");
      temperatur = query.value(FeldNr).toDouble();
      newItem4 -> setData(Qt::DisplayRole, temperatur);
      tableWidget_Hauptgaerverlauf -> setItem(i, 3, newItem4);
      i++;
      //Mittig ausrichten
      newItem1 -> setTextAlignment(Qt::AlignCenter);
      newItem2 -> setTextAlignment(Qt::AlignCenter);
      newItem3 -> setTextAlignment(Qt::AlignCenter);
      newItem4 -> setTextAlignment(Qt::AlignCenter);
    }
    tableWidget_Hauptgaerverlauf->setSortingEnabled(true);
    tableWidget_Hauptgaerverlauf->setSortingEnabled(false);
    FuelleDiagrammHauptgaerverlauf();
    //Eingabefelder mit den letzten werten füttern
    doubleSpinBox_SWHauptgaerprobe -> setValue(sw);
    doubleSpinBox_TempHauptgaerprobe -> setValue(temperatur);
  }
}


void MainWindowImpl::FuelleDiagrammHauptgaerverlauf()
{
  // Tabelle in Diagramm Schreiben
  widget_DiaHauptgaerverlauf -> DiagrammLeeren();
  for (int i=0; i < tableWidget_Hauptgaerverlauf -> rowCount(); i++){
    QDateTime dt;
    dt = tableWidget_Hauptgaerverlauf -> item(i,0) -> data(Qt::DisplayRole).toDateTime();
    widget_DiaHauptgaerverlauf -> L1Datum.append(dt);
    widget_DiaHauptgaerverlauf -> L2Datum.append(dt);
    widget_DiaHauptgaerverlauf -> L3Datum.append(dt);
    widget_DiaHauptgaerverlauf -> L1Daten.append(tableWidget_Hauptgaerverlauf -> item(i,1) -> data(Qt::DisplayRole).toDouble());
    widget_DiaHauptgaerverlauf -> L2Daten.append(tableWidget_Hauptgaerverlauf -> item(i,2) -> data(Qt::DisplayRole).toDouble());
    widget_DiaHauptgaerverlauf -> L3Daten.append(tableWidget_Hauptgaerverlauf -> item(i,3) -> data(Qt::DisplayRole).toDouble());
  }
  widget_DiaHauptgaerverlauf -> repaint();
}


void MainWindowImpl::on_pushButton_EingabeSWHauptgaerverlauf_clicked()
{
  KorrektSwImpl ksw;
  ksw.spinBox_SwPlato -> setValue(doubleSpinBox_SWHauptgaerprobe -> value());
  ksw.spinBox_Temperatur -> setValue(doubleSpinBox_TempHauptgaerprobe -> value());
  ksw.setSWAnstellen(spinBox_SWAnstellen -> value());
  ksw.BerDichte();
  ksw.exec();
  if (!ksw.abgebrochen){
    if (ksw.spinBox_SwBrix -> value() == 0){
      doubleSpinBox_SWHauptgaerprobe -> setValue(Berechnungen.densityAtX(ksw.spinBox_SwPlato -> value(), ksw.spinBox_Temperatur -> value(), 20));
    }
    else {
      doubleSpinBox_SWHauptgaerprobe -> setValue(ksw.spinBox_SwPlato -> value());
    }
    doubleSpinBox_TempHauptgaerprobe -> setValue(ksw.spinBox_Temperatur -> value());
  }
  ksw.setSWAnstellen(0);
}

void MainWindowImpl::on_pushButton_DelHauptgaerMessung_clicked()
{
  tableWidget_Hauptgaerverlauf -> removeRow(tableWidget_Hauptgaerverlauf -> currentRow());
  FuelleDiagrammHauptgaerverlauf();
  setAenderung(true);
  AenderungHauptgaerverlauf = true;
}

void MainWindowImpl::on_pushButton_AddNachgaerMessung_clicked()
{
  QDateTime dt;
  double druck, temp, co2;
  // Eintrag für den Schnellgärverlauf der Tabelle hinzufügen
  QTableWidgetItem *newItem1 = new QTableWidgetItem();
  QTableWidgetItem *newItem2 = new QTableWidgetItem();
  QTableWidgetItem *newItem3 = new QTableWidgetItem();
  QTableWidgetItem *newItem4 = new QTableWidgetItem();
  dt = dateTimeEdit_Nachgaerprobe -> dateTime();
  newItem1 -> setData(Qt::DisplayRole, dt);
  druck = doubleSpinBox_Nachgaerdruck -> value();
  newItem2 -> setData(Qt::DisplayRole, druck);
  temp = doubleSpinBox_Nachgaertemp -> value();
  newItem3 -> setData(Qt::DisplayRole, temp);
  //CO2 Gehalt berechnen
  co2 = Berechnungen.BerCO2Gehalt(druck, temp);
  co2 = double(qRound(co2 * 100)) / 100;
  newItem4 -> setData(Qt::DisplayRole, co2);
  newItem4->setFlags(newItem4->flags() & (~Qt::ItemIsEditable));
  int i = tableWidget_Nachgaerverlauf -> rowCount();

  //Mittig ausrichten
  newItem1 -> setTextAlignment(Qt::AlignCenter);
  newItem2 -> setTextAlignment(Qt::AlignCenter);
  newItem3 -> setTextAlignment(Qt::AlignCenter);
  newItem4 -> setTextAlignment(Qt::AlignCenter);

  AmLaden = true;
  tableWidget_Nachgaerverlauf -> insertRow(i);
  tableWidget_Nachgaerverlauf -> setItem(i, 0, newItem1);
  tableWidget_Nachgaerverlauf -> setItem(i, 1, newItem2);
  tableWidget_Nachgaerverlauf -> setItem(i, 2, newItem3);
  tableWidget_Nachgaerverlauf -> setItem(i, 3, newItem4);
  AmLaden = false;

  //Diagramm füllen
  FuelleDiagrammNachgaerverlauf();
  setAenderung(true);
}

void MainWindowImpl::SchreibeNachgaerverlaufDB()
{
  QSqlQuery query;

  //Zuerst alle Einträge in der Tabelle löschen
  QString sql = "DELETE FROM Nachgaerverlauf WHERE SudID = " + QString::number(AktuelleSudID) + ";";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

  // Tabelleninhalt schreiben
  for (int i=0; i < tableWidget_Nachgaerverlauf -> rowCount(); i++){
    QDateTime dt;
    dt = tableWidget_Nachgaerverlauf -> item(i,0) -> data(Qt::DisplayRole).toDateTime();
    sql = "INSERT INTO Nachgaerverlauf(SudID, Zeitstempel, Druck, Temp, CO2) VALUES(" +
        QString::number(AktuelleSudID) +	"," +
        "'" + dt.toString(Qt::ISODate) + "'," +
        tableWidget_Nachgaerverlauf -> item(i,1) -> text() + "," +
        tableWidget_Nachgaerverlauf -> item(i,2) -> text() + "," +
        tableWidget_Nachgaerverlauf -> item(i,3) -> text() + ")";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
  }
}


void MainWindowImpl::LeseNachgaerverlaufDB()
{
  QSqlQuery query;
  int FeldNr;

  //Tabelle einlesen
  QString sql = "SELECT * FROM Nachgaerverlauf WHERE SudID=" + QString::number(AktuelleSudID) + " ORDER BY Zeitstempel;";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int i=0;
    tableWidget_Nachgaerverlauf -> clearContents();
    tableWidget_Nachgaerverlauf -> setRowCount(0);
    double druck = 0;
    double temp = 0;
    double co2 = 0;
    while (query.next()){
      QTableWidgetItem *newItem1 = new QTableWidgetItem("");
      QTableWidgetItem *newItem2 = new QTableWidgetItem("");
      QTableWidgetItem *newItem3 = new QTableWidgetItem("");
      QTableWidgetItem *newItem4 = new QTableWidgetItem("");
      tableWidget_Nachgaerverlauf -> setRowCount(tableWidget_Nachgaerverlauf -> rowCount()+1);
      //Zeitstempel
      FeldNr = query.record().indexOf("Zeitstempel");
      newItem1 -> setData(Qt::DisplayRole, QDateTime::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      tableWidget_Nachgaerverlauf -> setItem(i, 0, newItem1);
      //Druck
      FeldNr = query.record().indexOf("Druck");
      druck = query.value(FeldNr).toDouble();
      newItem2 -> setData(Qt::DisplayRole, druck);
      tableWidget_Nachgaerverlauf -> setItem(i, 1, newItem2);
      //Temperatur
      FeldNr = query.record().indexOf("Temp");
      temp = query.value(FeldNr).toDouble();
      newItem3 -> setData(Qt::DisplayRole, temp);
      tableWidget_Nachgaerverlauf -> setItem(i, 2, newItem3);
      //Co2 Gehalt
      FeldNr = query.record().indexOf("CO2");
      co2 = query.value(FeldNr).toDouble();
      newItem4 -> setData(Qt::DisplayRole, co2);
      newItem4->setFlags(newItem4->flags() & (~Qt::ItemIsEditable));
      tableWidget_Nachgaerverlauf -> setItem(i, 3, newItem4);
      i++;
      //Mittig ausrichten
      newItem1 -> setTextAlignment(Qt::AlignCenter);
      newItem2 -> setTextAlignment(Qt::AlignCenter);
      newItem3 -> setTextAlignment(Qt::AlignCenter);
      newItem4 -> setTextAlignment(Qt::AlignCenter);
    }
    tableWidget_Nachgaerverlauf->setSortingEnabled(true);
    tableWidget_Nachgaerverlauf->setSortingEnabled(false);
    FuelleDiagrammNachgaerverlauf();
    //Eingabefelder mit den letzten werten füttern
    doubleSpinBox_Nachgaerdruck -> setValue(druck);
    doubleSpinBox_Nachgaertemp -> setValue(temp);
  }
}


void MainWindowImpl::FuelleDiagrammNachgaerverlauf()
{
  // Tabelle in Diagramm Schreiben
  widget_DiaNachgaerverlauf -> DiagrammLeeren();
  for (int i=0; i < tableWidget_Nachgaerverlauf -> rowCount(); i++){
    QDateTime dt;
    dt = tableWidget_Nachgaerverlauf -> item(i,0) -> data(Qt::DisplayRole).toDateTime();
    widget_DiaNachgaerverlauf -> L1Datum.append(dt);
    widget_DiaNachgaerverlauf -> L2Datum.append(dt);
    widget_DiaNachgaerverlauf -> L1Daten.append(tableWidget_Nachgaerverlauf -> item(i,3) -> data(Qt::DisplayRole).toDouble());
    widget_DiaNachgaerverlauf -> L2Daten.append(tableWidget_Nachgaerverlauf -> item(i,2) -> data(Qt::DisplayRole).toDouble());
  }
  widget_DiaNachgaerverlauf -> setWertLinie1(doubleSpinBox_CO2 -> value());
  widget_DiaNachgaerverlauf -> repaint();
}


void MainWindowImpl::on_pushButton_DelNachgaerMessung_clicked()
{
  tableWidget_Nachgaerverlauf -> removeRow(tableWidget_Nachgaerverlauf -> currentRow());
  FuelleDiagrammNachgaerverlauf();
  setAenderung(true);
}

void MainWindowImpl::SetDiagrammFarben()
{
  //Farben aus Konfigdatei auslesen und im Dialogfeld setzten
  QColor color;
  QString HtmlColor = "";

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("Farben");

  //Farbe Linie 1 Schnellgärverlauf
  HtmlColor = settings.value("FARBE_GAERVERLAUF_DIAGRAMM_S_L1").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_S_L1);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    widget_DiaSchnellgaerverlauf -> colorL1 = color;
  }

  //Farbe Linie 2 Schnellgärverlauf
  HtmlColor = settings.value("FARBE_GAERVERLAUF_DIAGRAMM_S_L2").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_S_L2);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    widget_DiaSchnellgaerverlauf -> colorL2 = color;
  }

  //Farbe Linie 3 Schnellgärverlauf
  HtmlColor = settings.value("FARBE_GAERVERLAUF_DIAGRAMM_S_L3").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_S_L3);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    widget_DiaSchnellgaerverlauf -> colorL3 = color;
  }

  //Farbe Linie 1 Hauptgärverlauf
  HtmlColor = settings.value("FARBE_GAERVERLAUF_DIAGRAMM_H_L1").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_H_L1);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    widget_DiaHauptgaerverlauf -> colorL1 = color;
  }

  //Farbe Linie 2 Hauptgärverlauf
  HtmlColor = settings.value("FARBE_GAERVERLAUF_DIAGRAMM_H_L2").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_H_L2);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    widget_DiaHauptgaerverlauf -> colorL2 = color;
  }

  //Farbe Linie 3 Hauptgärverlauf
  HtmlColor = settings.value("FARBE_GAERVERLAUF_DIAGRAMM_H_L3").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_H_L3);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    widget_DiaHauptgaerverlauf -> colorL3 = color;
  }

  //Farbe Linie 1 Nachgärverlauf
  HtmlColor = settings.value("FARBE_GAERVERLAUF_DIAGRAMM_N_L1").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_N_L1);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    widget_DiaNachgaerverlauf -> colorL1 = color;
  }

  //Farbe Linie 2 Nachgärverlauf
  HtmlColor = settings.value("FARBE_GAERVERLAUF_DIAGRAMM_N_L2").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_N_L2);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    widget_DiaNachgaerverlauf -> colorL2 = color;
  }

  //Farbe Linie 1 Brauübersicht
  HtmlColor = settings.value("FARBE_BRAUUEBERSICHT_DIAGRAMM_L1").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_BRAUUEBERSICHT_DIAGRAMM_L1);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    widget_DiaBrauUebersicht -> colorL1 = color;
  }

  //Farbe Linie 2 Brauübersicht
  HtmlColor = settings.value("FARBE_BRAUUEBERSICHT_DIAGRAMM_L2").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_BRAUUEBERSICHT_DIAGRAMM_L2);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    widget_DiaBrauUebersicht -> colorL2 = color;
  }

  //Farbwert einlesen für Spalten Brauübersicht
  HtmlColor = settings.value("FARBE_BRAUUEBERSICHT_SPALTE_L1").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_BRAUUEBERSICHT_SPALTE_L1);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    ColorBrauUebersicht_Spalte1 = color;
  }
  HtmlColor = settings.value("FARBE_BRAUUEBERSICHT_SPALTE_L2").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_BRAUUEBERSICHT_SPALTE_L2);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    ColorBrauUebersicht_Spalte2 = color;
  }


  settings.endGroup();
}

void MainWindowImpl::SetAnsicht()
{
  QString str;
  bool gruppeAusblenden;
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("Ansicht");

  gruppeAusblenden = true;
  //tatsächlicher Endvergärungsgrad (EVG)
  str = settings.value("checkBox_Ansicht_BuG_tevg").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_tevg", false);
  }

  if (settings.value("checkBox_Ansicht_BuG_tevg").toBool()){
    label_tevg->setVisible(true);
    label_etevg->setVisible(true);
    spinBox_TEVG->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_tevg->setVisible(false);
    label_etevg->setVisible(false);
    spinBox_TEVG->setVisible(false);
  }

  //Scheinbarer Endvergärungsgrad (EVG)
  str = settings.value("checkBox_Ansicht_BuG_sevg").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_sevg", true);
  }

  if (settings.value("checkBox_Ansicht_BuG_sevg").toBool()){
    label_sevg->setVisible(true);
    label_esevg->setVisible(true);
    spinBox_SEVG->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_sevg->setVisible(false);
    label_esevg->setVisible(false);
    spinBox_SEVG->setVisible(false);
  }
  if (gruppeAusblenden){
    groupBox_evg->setVisible(false);
  }
  else {
    groupBox_evg->setVisible(true);
  }

  gruppeAusblenden = true;
  //Sudhausausbeute
  str = settings.value("checkBox_Ansicht_BuG_shab").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_shab", false);
  }

  if (settings.value("checkBox_Ansicht_BuG_shab").toBool()){
    label_sha->setVisible(true);
    label_esha->setVisible(true);
    spinBox_Sudhausausbeute->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_sha->setVisible(false);
    label_esha->setVisible(false);
    spinBox_Sudhausausbeute->setVisible(false);
  }

  //effektive Sudhausausbeute
  str = settings.value("checkBox_Ansicht_BuG_eshab").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_eshab", true);
  }

  if (settings.value("checkBox_Ansicht_BuG_eshab").toBool()){
    label_efsha->setVisible(true);
    label_eefsha->setVisible(true);
    spinBox_AusbeuteEffektiv2->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_efsha->setVisible(false);
    label_eefsha->setVisible(false);
    spinBox_AusbeuteEffektiv2->setVisible(false);
  }
  if (gruppeAusblenden){
    groupBox_sha->setVisible(false);
  }
  else {
    groupBox_sha->setVisible(true);
  }

  gruppeAusblenden = true;
  //Akloholgehalt
  str = settings.value("checkBox_Ansicht_BuG_alc").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_alc", true);
  }

  if (settings.value("checkBox_Ansicht_BuG_alc").toBool()){
    label_alc->setVisible(true);
    label_ealc->setVisible(true);
    spinBox_AlkoholVol->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_alc->setVisible(false);
    label_ealc->setVisible(false);
    spinBox_AlkoholVol->setVisible(false);
  }

  //Kosten pro Liter
  str = settings.value("checkBox_Ansicht_BuG_kpl").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_kpl", true);
  }

  if (settings.value("checkBox_Ansicht_BuG_kpl").toBool()){
    label_kpl->setVisible(true);
    label_ekpl->setVisible(true);
    spinBox_Preis->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_kpl->setVisible(false);
    label_ekpl->setVisible(false);
    spinBox_Preis->setVisible(false);
  }
  if (gruppeAusblenden){
    groupBox_alk->setVisible(false);
  }
  else {
    groupBox_alk->setVisible(true);
  }

  gruppeAusblenden = true;
  //Grünschlauchzeitpunkt
  str = settings.value("checkBox_Ansicht_BuG_gszp").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_gszp", true);
  }

  if (settings.value("checkBox_Ansicht_BuG_gszp").toBool()){
    label_gszp->setVisible(true);
    label_egszp->setVisible(true);
    spinBox_Gruenschlauchzeitpunkt->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_gszp->setVisible(false);
    label_egszp->setVisible(false);
    spinBox_Gruenschlauchzeitpunkt->setVisible(false);
  }

  //Spundungsdruck
  str = settings.value("checkBox_Ansicht_BuG_sd").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_sd", true);
  }

  if (settings.value("checkBox_Ansicht_BuG_sd").toBool()){
    label_sp->setVisible(true);
    label_esp->setVisible(true);
    spinBox_Spundungsdruck->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_sp->setVisible(false);
    label_esp->setVisible(false);
    spinBox_Spundungsdruck->setVisible(false);
  }
  if (gruppeAusblenden){
    groupBox_afg->setVisible(false);
  }
  else {
    groupBox_afg->setVisible(true);
  }

  gruppeAusblenden = true;
  //Speisemenge gesamt
  str = settings.value("checkBox_Ansicht_BuG_smg").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_smg", true);
  }

  if (settings.value("checkBox_Ansicht_BuG_smg").toBool()){
    label_smg->setVisible(true);
    label_esmg->setVisible(true);
    spinBox_SpeisemengeGesammt->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_smg->setVisible(false);
    label_esmg->setVisible(false);
    spinBox_SpeisemengeGesammt->setVisible(false);
  }

  //Haushaltszuckergabe gesamt
  str = settings.value("checkBox_Ansicht_BuG_hzmg").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_hzmg", true);
  }

  if (settings.value("checkBox_Ansicht_BuG_hzmg").toBool()){
    label_hzmg->setVisible(true);
    label_ehzmg->setVisible(true);
    spinBox_HaushaltszuckerGesammt->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_hzmg->setVisible(false);
    label_ehzmg->setVisible(false);
    spinBox_HaushaltszuckerGesammt->setVisible(false);
  }

  //Traubenzuckergabe gesamt
  str = settings.value("checkBox_Ansicht_BuG_tzmg").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_tzmg", false);
  }

  if (settings.value("checkBox_Ansicht_BuG_tzmg").toBool()){
    label_tzmg->setVisible(true);
    label_etzmg->setVisible(true);
    spinBox_TraubenzuckerGesammt->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_tzmg->setVisible(false);
    label_etzmg->setVisible(false);
    spinBox_TraubenzuckerGesammt->setVisible(false);
  }
  if (gruppeAusblenden){
    groupBox_smg->setVisible(false);
  }
  else {
    groupBox_smg->setVisible(true);
  }

  gruppeAusblenden = true;
  //Speisemenge pro 0,5 Liter Flasche
  str = settings.value("checkBox_Ansicht_BuG_s05").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_s05", true);
  }

  if (settings.value("checkBox_Ansicht_BuG_s05").toBool()){
    label_s05->setVisible(true);
    label_es05->setVisible(true);
    spinBox_Speisemenge500mlFlasche->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_s05->setVisible(false);
    label_es05->setVisible(false);
    spinBox_Speisemenge500mlFlasche->setVisible(false);
  }

  //Haushaltszuckergabe pro Flasche 0,5 Liter
  str = settings.value("checkBox_Ansicht_BuG_hz05").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_hz05", true);
  }

  if (settings.value("checkBox_Ansicht_BuG_hz05").toBool()){
    label_hz05->setVisible(true);
    label_ehz05->setVisible(true);
    spinBox_Haushaltszucker500mlFlasche->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_hz05->setVisible(false);
    label_ehz05->setVisible(false);
    spinBox_Haushaltszucker500mlFlasche->setVisible(false);
  }

  //Traubenzuckergabe pro Flasche 0,5 Liter
  str = settings.value("checkBox_Ansicht_BuG_tz05").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_tz05", false);
  }

  if (settings.value("checkBox_Ansicht_BuG_tz05").toBool()){
    label_tz05->setVisible(true);
    label_etz05->setVisible(true);
    spinBox_Traubenzucker500mlFlasche->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_tz05->setVisible(false);
    label_etz05->setVisible(false);
    spinBox_Traubenzucker500mlFlasche->setVisible(false);
  }
  if (gruppeAusblenden){
    groupBox_sm05->setVisible(false);
  }
  else {
    groupBox_sm05->setVisible(true);
  }

  gruppeAusblenden = true;
  //Speisemenge pro 0,33 Liter Flasche
  str = settings.value("checkBox_Ansicht_BuG_s033").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_s033", false);
  }

  if (settings.value("checkBox_Ansicht_BuG_s033").toBool()){
    label_s033->setVisible(true);
    label_es033->setVisible(true);
    spinBox_Speisemenge333mlFlasche->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_s033->setVisible(false);
    label_es033->setVisible(false);
    spinBox_Speisemenge333mlFlasche->setVisible(false);
  }

  //Haushaltszuckergabe pro Flasche 0,33 Liter
  str = settings.value("checkBox_Ansicht_BuG_hz033").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_hz033", false);
  }

  if (settings.value("checkBox_Ansicht_BuG_hz033").toBool()){
    label_hz033->setVisible(true);
    label_ehz033->setVisible(true);
    spinBox_Haushaltszucker333mlFlasche->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_hz033->setVisible(false);
    label_ehz033->setVisible(false);
    spinBox_Haushaltszucker333mlFlasche->setVisible(false);
  }

  //Traubenzuckergabe pro Flasche 0,33 Liter
  str = settings.value("checkBox_Ansicht_BuG_tz033").toString();
  if (str == ""){
    settings.setValue("checkBox_Ansicht_BuG_tz033", false);
  }

  if (settings.value("checkBox_Ansicht_BuG_tz033").toBool()){
    label_tz033->setVisible(true);
    label_etz033->setVisible(true);
    spinBox_Traubenzucker333mlFlasche->setVisible(true);
    gruppeAusblenden = false;
  }
  else {
    label_tz033->setVisible(false);
    label_etz033->setVisible(false);
    spinBox_Traubenzucker333mlFlasche->setVisible(false);
  }
  if (gruppeAusblenden){
    groupBox_sm033->setVisible(false);
  }
  else {
    groupBox_sm033->setVisible(true);
  }

  settings.endGroup();
}

void MainWindowImpl::SetMaxAnzahlSterne()
{
  LeseMaxAnzahlSterne();
  graphicsView_bewStar->setMaxStar(MaxAnzahlSterne);
  for (int i=0; i<list_Bewertung.count(); i++){
    list_Bewertung[i]->setMaxSterne(MaxAnzahlSterne);
  }
}


void MainWindowImpl::slot_einstellungen()
{
  EinstellungsdialogImpl edia;
  if (edia.exec() == QDialog::Accepted) {
    if (edia.NeuerDBPfad) {
      close();
    }
    //Farben für Diagramme neu einlesen
    SetMaxAnzahlSterne();
    SetDiagrammFarben();
    FuelleBrauuebersicht();
    SetAnsicht();
  }
}


void MainWindowImpl::slot_schuettungProzent()
{
  Dialog_BerSchuettungImpl rdia;
  //Malzauswahlboxen füllen
  //Alls erstes einen Leereintrag hinzufügen
  rdia.comboBox_S_1 -> addItem("");
  rdia.comboBox_S_2 -> addItem("");
  rdia.comboBox_S_3 -> addItem("");
  rdia.comboBox_S_4 -> addItem("");
  rdia.comboBox_S_5 -> addItem("");
  rdia.comboBox_S_6 -> addItem("");
  //Dann mit den Rohstoffdaten füllen
  for (int i=0; i < tableWidget_Malz -> rowCount(); i++){
    rdia.comboBox_S_1 -> addItem(tableWidget_Malz -> item(i,TableMalzColName) -> text() );
    rdia.comboBox_S_2 -> addItem(tableWidget_Malz -> item(i,TableMalzColName) -> text() );
    rdia.comboBox_S_3 -> addItem(tableWidget_Malz -> item(i,TableMalzColName) -> text() );
    rdia.comboBox_S_4 -> addItem(tableWidget_Malz -> item(i,TableMalzColName) -> text() );
    rdia.comboBox_S_5 -> addItem(tableWidget_Malz -> item(i,TableMalzColName) -> text() );
    rdia.comboBox_S_6 -> addItem(tableWidget_Malz -> item(i,TableMalzColName) -> text() );
  }

  //Schüttung übernehmen
  if (rdia.exec() == QDialog::Accepted  && !BierWurdeGebraut){
    //Erstmal Zutatenlsite leeren
    for (int i=0; i < list_Malzgaben.count(); i++){
      list_Malzgaben[i] -> close();
    }
    list_Malzgaben.clear();

    if (rdia.comboBox_S_1->currentText() != ""){
      AddMalzgabe(rdia.comboBox_S_1->currentText(),rdia.doubleSpinBox_Prozent_S_1 -> value(),double(0), double(0));
    }
    if (rdia.comboBox_S_2->currentText() != ""){
      AddMalzgabe(rdia.comboBox_S_2->currentText(),rdia.doubleSpinBox_Prozent_S_2 -> value(),double(0), double(0));
    }
    if (rdia.comboBox_S_3->currentText() != ""){
      AddMalzgabe(rdia.comboBox_S_3->currentText(),rdia.doubleSpinBox_Prozent_S_3 -> value(),double(0), double(0));
    }
    if (rdia.comboBox_S_4->currentText() != ""){
      AddMalzgabe(rdia.comboBox_S_4->currentText(),rdia.doubleSpinBox_Prozent_S_4 -> value(),double(0), double(0));
    }
    if (rdia.comboBox_S_5->currentText() != ""){
      AddMalzgabe(rdia.comboBox_S_5->currentText(),rdia.doubleSpinBox_Prozent_S_5 -> value(),double(0), double(0));
    }
    if (rdia.comboBox_S_6->currentText() != ""){
      AddMalzgabe(rdia.comboBox_S_6->currentText(),rdia.doubleSpinBox_Prozent_S_6 -> value(),double(0), double(0));
    }
    BerAlles();

  }
}

void MainWindowImpl::slot_berIBU()
{
  Dialog_Berechne_IBUImpl rdia;
  //IBU Wert übernehmen
  if (rdia.exec() == QDialog::Accepted  && !BierWurdeGebraut){
    spinBox_IBU -> setValue(qRound(rdia.doubleSpinBox_IBU -> value()));
  }
}

void MainWindowImpl::on_tableWidget_Schnellgaerverlauf_itemChanged(QTableWidgetItem* item)
{
  if (Gestartet && !AmLaden){
    // Alkoholgehalt dieser Zeile neu berechnen
    int zeile;
    double sw, alc;
    zeile = item->row();
    sw = tableWidget_Schnellgaerverlauf -> item(zeile,1) -> data(Qt::DisplayRole).toDouble();
    //Alkoholgehalt berechnen
    alc = Berechnungen.BerAlkohoVol(spinBox_SWSollGesammt -> value(), sw );
    alc = double(qRound(alc * 10)) / 10;
    tableWidget_Schnellgaerverlauf -> item(zeile,2) -> setData(Qt::DisplayRole, alc);
    tableWidget_Schnellgaerverlauf->setSortingEnabled(true);
    tableWidget_Schnellgaerverlauf->setSortingEnabled(false);
    setAenderung(true);
    FuelleDiagrammSchnellgaerverlauf();
  }
}

void MainWindowImpl::on_tableWidget_Hauptgaerverlauf_itemChanged(QTableWidgetItem* item)
{
  if (Gestartet && !AmLaden){
    // Alkoholgehalt dieser Zeile neu berechnen
    int zeile;
    double sw, alc;
    zeile = item->row();
    sw = tableWidget_Hauptgaerverlauf -> item(zeile,1) -> data(Qt::DisplayRole).toDouble();
    //Alkoholgehalt berechnen
    alc = Berechnungen.BerAlkohoVol(spinBox_SWSollGesammt -> value(), sw );
    alc = double(qRound(alc * 10)) / 10;
    tableWidget_Hauptgaerverlauf -> item(zeile,2) -> setData(Qt::DisplayRole, alc);
    tableWidget_Hauptgaerverlauf->setSortingEnabled(true);
    tableWidget_Hauptgaerverlauf->setSortingEnabled(false);
    setAenderung(true);
    AenderungHauptgaerverlauf = true;
    FuelleDiagrammHauptgaerverlauf();
  }
}

void MainWindowImpl::on_tableWidget_Nachgaerverlauf_itemChanged(QTableWidgetItem* item)
{
  if (Gestartet && !AmLaden){
    // CO2 Gehalt dieser Zeile Neu berechnen
    int zeile;
    double druck, temp, co2;
    zeile = item->row();
    druck = tableWidget_Nachgaerverlauf -> item(zeile,1) -> data(Qt::DisplayRole).toDouble();
    temp = tableWidget_Nachgaerverlauf -> item(zeile,2) -> data(Qt::DisplayRole).toDouble();
    co2 = Berechnungen.BerCO2Gehalt(druck, temp);
    co2 = double(qRound(co2 * 100)) / 100;
    tableWidget_Nachgaerverlauf -> item(zeile,3) -> setData(Qt::DisplayRole, co2);
    tableWidget_Nachgaerverlauf->setSortingEnabled(true);
    tableWidget_Nachgaerverlauf->setSortingEnabled(false);
    setAenderung(true);
    FuelleDiagrammNachgaerverlauf();
  }
}

void MainWindowImpl::on_pushButton_SudVerbraucht_clicked()
{
  BierWurdeVerbraucht = true;
  SetDisabledVerbraucht(true);
  setAenderung(true);
}


void MainWindowImpl::on_lineEdit_Sudname_editingFinished()
{
}

void MainWindowImpl::on_lineEdit_Sudname_textEdited(QString )
{
  if (Gestartet){
    setAenderung(true);
    BerAlles();
  }
}


void MainWindowImpl::on_pushButton_EingabeHSWVorHopfenseihen_clicked()
{
  KorrektSwImpl ksw;
  ksw.spinBox_SwPlato -> setValue(spinBox_SWVorHopfenseihen -> value());
  ksw.BerDichte();
  ksw.exec();
  if (!ksw.abgebrochen){
    spinBox_SWVorHopfenseihen -> setValue(Berechnungen.densityAtX(ksw.spinBox_SwPlato -> value(), ksw.spinBox_Temperatur -> value(), 20));
  }
}

bool MainWindowImpl::CheckDBNeuBerechnen()
{
  //Überprüfung Ob Datenbank neu Berechnet werden muss
  QSqlQuery query;
  QString sql = "SELECT db_NeuBerechnen FROM Global;";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
    return false;
  }
  else {
    if (!query.first()) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
      return false;
    }
    else {
      NeuBerechnen = query.value(0).toInt();
      if (NeuBerechnen >= 1){
        return true;
      }
      else {
        return false;
      }
    }
  }
}


void MainWindowImpl::DBErgebnisseNeuBerechnen()
{
  //Von Allen Suden die EffektiveAusbeute neu Berechnen da bisher die abgezwackte
  //Speisemenge nicht berücksichtigt wurde
  QString sql2;
  QString sql = "SELECT * FROM Sud WHERE BierWurdeGebraut=1";
  QSqlQuery query, query2;
  QSqlDatabase::database().transaction();

  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    double effausbeute;
    double sudhausausbeute;
    double sw;
    double menge;
    double schuettung;
    int id;
    int FeldNr;
    //Alle Sude die gebraut wurden durchgehen und Effektive Ausbeute neu berechnen
    while (query.next()) {
      //ID von dem Datensatz der gerade berechnet wird
      FeldNr = query.record().indexOf("ID");
      id = query.value(FeldNr).toInt();

      //Stammwürze
      FeldNr = query.record().indexOf("SWAnstellen");
      sw = query.value(FeldNr).toDouble();

      //Würzemenge
      FeldNr = query.record().indexOf("WuerzemengeAnstellen");
      menge = query.value(FeldNr).toDouble();
      //pluss abgezwackte Speisemenge
      FeldNr = query.record().indexOf("Speisemenge");
      menge = menge + query.value(FeldNr).toDouble();

      //Schüttung
      FeldNr = query.record().indexOf("erg_S_Gesammt");
      schuettung = query.value(FeldNr).toDouble();

      //Effektive Ausbeute Berechnen und in DB Schreiben
      if (NeuBerechnen == 1){
        effausbeute = Berechnungen.GetSudhausausbeute(sw,menge,schuettung);
        sql2 = "UPDATE 'Sud' SET 'erg_EffektiveAusbeute'=" + QString::number(effausbeute)
            + " WHERE ID=" + QString::number(id);
        if (!query2.exec(sql2)) {
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
                                      + QObject::trUtf8("\nSQL Befehl:\n") + sql2);
        }
      }

      //Sudhausausbeute neu berechnen
      //Stammwürze
      FeldNr = query.record().indexOf("SWVorHopfenseihen");
      sw = query.value(FeldNr).toDouble();

      //Würzemenge
      FeldNr = query.record().indexOf("WuerzemengeVorHopfenseihen");
      menge = query.value(FeldNr).toDouble();

      if (NeuBerechnen == 1){
        sudhausausbeute = Berechnungen.GetSudhausausbeute(sw,menge,schuettung);
        //Sudhausausbeute in DB Schreiben
        sql2 = "UPDATE 'Sud' SET 'erg_Sudhausausbeute'=" + QString::number(sudhausausbeute)
            + " WHERE ID=" + QString::number(id);
        if (!query2.exec(sql2)) {
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
                                      + QObject::trUtf8("\nSQL Befehl:\n") + sql2);
        }
      }

      //Abgefuellte Menge berechnen
      if (NeuBerechnen == 2){
        //CO2
        FeldNr = query.record().indexOf("CO2");
        double co2 = query.value(FeldNr).toDouble();

        //Temperatur Jungbier
        FeldNr = query.record().indexOf("TemperaturJungbier");
        double TempJungbier = query.value(FeldNr).toDouble();

        //SWSchnellgärprobe
        FeldNr = query.record().indexOf("SchnellgaerprobeAktiv");
        bool SchnellgaerprobeAktiv = query.value(FeldNr).toBool();
        double SWSchnellgaerprobe;
        if (SchnellgaerprobeAktiv){
          FeldNr = query.record().indexOf("SWSchnellgaerprobe");
          SWSchnellgaerprobe = query.value(FeldNr).toDouble();
        }
        else {
          FeldNr = query.record().indexOf("SWJungbier");
          SWSchnellgaerprobe = query.value(FeldNr).toDouble();
        }

        //Grünschlauchzeitpunkt
        Berechnungen.GetGruenschlauchzeitpunkt(co2,	TempJungbier, SWSchnellgaerprobe);

        //SWAnstellen
        FeldNr = query.record().indexOf("SWAnstellen");
        double SWAnstellen = query.value(FeldNr).toDouble();

        //SWJungbier
        FeldNr = query.record().indexOf("SWJungbier");
        double SWJungbier = query.value(FeldNr).toDouble();

        //Jungbiermenge Abfuellen
        FeldNr = query.record().indexOf("JungbiermengeAbfuellen");
        double JungbiermengeAbfuellen = query.value(FeldNr).toDouble();

        //Abgefuellte Speisemenge
        FeldNr = query.record().indexOf("Speisemenge");
        double AbgefuellteSpeisemenge = query.value(FeldNr).toDouble();

        //Erforderliche Speisemenge
        double SpeisemengeGesammt = Berechnungen.GetSpeiseGesammt(SWAnstellen, SWJungbier, JungbiermengeAbfuellen , AbgefuellteSpeisemenge, false)/1000;

        //Abgefuellte Menge
        double AbgefuellteMenge = JungbiermengeAbfuellen + SpeisemengeGesammt;
        sql2 = "UPDATE 'Sud' SET 'erg_AbgefuellteBiermenge'=" + QString::number(AbgefuellteMenge)
            + " WHERE ID=" + QString::number(id);
        if (!query2.exec(sql2)) {
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query2.lastError().databaseText()
                                      + QObject::trUtf8("\nSQL Befehl:\n") + sql2);
        }

      }
    }
    //Flag das neu Berechnet werden muss wieder zurücksetzten
    sql = "UPDATE 'Global' SET 'db_NeuBerechnen'=0";
    if (!query.exec(sql)) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, QObject::trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + QObject::trUtf8("\nSQL Befehl:\n") + sql);
    }
  }
  QSqlDatabase::database().commit();
}

void MainWindowImpl::on_pushButton_EingabeHMengeVHopfenseihen_clicked()
{
  EingabeHVolumenImpl dia(getSudpfanneDurchmesser(), getSudpfanneHoehe());
  dia.setLiter(spinBox_WuerzemengeVorHopfenseihen -> value());
  dia.setWindowTitle(trUtf8("Eingabehilfe für Volumen Sudpfanne"));
  if (dia.exec() == QDialog::Accepted){
    spinBox_WuerzemengeVorHopfenseihen -> setValue(dia.spinBox_Liter20Grad -> value());
  }
}

void MainWindowImpl::BerAusruestung()
{
  // Volumen Maischebottich berechnen
  double Grundflaeche;

  Grundflaeche = pow(spinBox_MaischebottichDurchmesser->value()/2, 2) * M_PI/10;
  spinBox_MaischenVolumen -> setValue(Grundflaeche * spinBox_MaischebottichHoehe->value() / 100 );
  spinBox_MaischenMaxNutzvolumen -> setValue(Grundflaeche * spinBox_MaischebottichMaxFuellhoehe->value() / 100 );

  // Volumen Sudpfanne berechnen
  Grundflaeche = pow(spinBox_SudpfanneDurchmesser -> value()/2, 2) * M_PI/10;
  spinBox_SudpfanneVolumen -> setValue(Grundflaeche * spinBox_SudpfanneHoehe -> value() / 100 );
  spinBox_SudpfanneMaxNutzvolumen -> setValue(Grundflaeche * spinBox_SudpfanneMaxFuellhoehe -> value() / 100 );

}


void MainWindowImpl::on_pushButton_EingabeHMengeNHopfenseihen_clicked()
{
  EingabeHVolumenImpl dia(getSudpfanneDurchmesser(), getSudpfanneHoehe());
  dia.setLiter(spinBox_WuerzemengeKochende -> value());
  dia.setWindowTitle(trUtf8("Eingabehilfe für Volumen nach dem Hopfenseihen"));
  dia.setVisibleVonOben(false);
  dia.setVisibleVonUnten(false);
  if (dia.exec() == QDialog::Accepted){
    spinBox_WuerzemengeKochende -> setValue(dia.spinBox_Liter20Grad -> value());
  }
}

void MainWindowImpl::BerEmpfehlungFaktorHauptguss()
{
  double ebc = doubleSpinBox_EBC -> value();

  if (ebc < 50){
    doubleSpinBox_FaktorHGEmpfehlung -> setValue(4 - ebc * 0.02);
  }
  else {
    doubleSpinBox_FaktorHGEmpfehlung -> setValue(3);
  }
}

void MainWindowImpl::BerEmpfehlungHefeEinheiten()
{
  //Anzahl Liter der Ausgewählten Hefe
  int Wuerzemenge=0;
  int EmpfohleneMenge=0;
  int AnzahlHefeEintraege = tableWidget_Hefe -> rowCount();
  QString s = comboBox_AuswahlHefe -> currentText();
  if (s != ""){
    //Würzemenge auslesen
    for (int i=0; i < AnzahlHefeEintraege; i++){
      //wenn Eintrag übereinstimmt
      if (tableWidget_Hefe -> item(i,TableHefeColName) -> text() == s){
        QSpinBox *spinBox = (QSpinBox*)tableWidget_Hefe -> cellWidget(i,TableHefeColWuerzemenge);
        Wuerzemenge = spinBox->value();
      }
    }
  }
  if (Wuerzemenge > 0){
    EmpfohleneMenge = ceil(spinBox_Menge->value()/Wuerzemenge);
  }
  spinBox_empfohleneHefeEinheiten->setValue(EmpfohleneMenge);
  if (spinBox_AnzahlHefeEinheiten->value() == 0){
    spinBox_AnzahlHefeEinheiten->setValue(EmpfohleneMenge);
  }
}

void MainWindowImpl::setHefeAuswahlListeFarbe()
{
  double menge = 0;
  for (int i=0; i < comboBox_AuswahlHefe->count(); i++) {
    menge = slot_HefeGetMenge(comboBox_AuswahlHefe->itemText(i));
    //Hintergund einfärben wenn von dieser Zutat nicht mehr da ist
    if (menge == 0) {
      if (StyleDunkel)
        comboBox_AuswahlHefe->setItemData(i,QColor::fromRgb(FARBE_COMBO_ROHSTOFF_EMPTY_DUNKEL),Qt::TextColorRole);
      else
        comboBox_AuswahlHefe->setItemData(i,QColor::fromRgb(FARBE_COMBO_ROHSTOFF_EMPTY_HELL),Qt::TextColorRole);
    }
    else if (menge < spinBox_AnzahlHefeEinheiten->value()) {
      if (StyleDunkel)
        comboBox_AuswahlHefe->setItemData(i,QColor::fromRgb(FARBE_COMBO_ROHSTOFF_LOW_DUNKEL),Qt::TextColorRole);
      else
        comboBox_AuswahlHefe->setItemData(i,QColor::fromRgb(FARBE_COMBO_ROHSTOFF_LOW_HELL),Qt::TextColorRole);
    }
    else {
      if (StyleDunkel)
        comboBox_AuswahlHefe->setItemData(i,QColor(Qt::white),Qt::TextColorRole);
      else
        comboBox_AuswahlHefe->setItemData(i,QColor(40,40,40),Qt::TextColorRole);
    }
  }
}

void MainWindowImpl::slot_EntsperreEingabefelder()
{
  SetStatusGebraut(false);
  SetDisabledAbgefuellt(false);
  if (BierWurdeGebraut)
    pushButton_RohstoffeAbziehen -> setDisabled(true);
  if (BierWurdeAbgefuellt)
    pushButton_SudAbgefuellt -> setDisabled(true);

}

void MainWindowImpl::slot_ResetBierWurdeGebraut()
{
  QSqlQuery query;
  QString sql = "UPDATE Sud SET BierWurdeGebraut=0 WHERE ID=" + QString::number(AktuelleSudID) + ";";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  sql = "UPDATE Sud SET BierWurdeAbgefuellt=0 WHERE ID=" + QString::number(AktuelleSudID) + ";";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  sql = "UPDATE Sud SET BierWurdeVerbraucht=0 WHERE ID=" + QString::number(AktuelleSudID) + ";";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  LadeSudDB(true);
}

void MainWindowImpl::slot_ResetAbgefuellt()
{
  QSqlQuery query;
  QString sql = "UPDATE Sud SET BierWurdeAbgefuellt=0 WHERE ID=" + QString::number(AktuelleSudID) + ";";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  sql = "UPDATE Sud SET BierWurdeVerbraucht=0 WHERE ID=" + QString::number(AktuelleSudID) + ";";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  LadeSudDB(true);
}

void MainWindowImpl::slot_ResetWZZugabestatus()
{
  //Zugabestatus der Weiteren Zutaten zurücksetzten
  for (int i=0; i < list_EwZutat.count(); i++){
    list_EwZutat[i] -> setZugabestatus(0);
  }

}

void MainWindowImpl::slot_ResetBierVerbraucht()
{
  QSqlQuery query;
  QString sql = "UPDATE Sud SET BierWurdeVerbraucht=0 WHERE ID=" + QString::number(AktuelleSudID) + ";";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  LadeSudDB(true);
}


void MainWindowImpl::BerVolumenMaischen()
{
  //Ein kg Malzschrot verdrängt eingemaischt je nach
  //Feinheitsgrad der Schrotung ein Volumen von 0,65 bis 0,8 l Wasser. Näherungsweise rechnet man mit
  //0,7 l Wasserverdrängung pro kg Malzschrot. Dann ergibt sich:
  //Gesamtmaische [l] = Hauptguß [l] + (Schüttung in kg * 0,7 l/kg)
  double wasser = doubleSpinBox_WHauptguss -> value();
  doubleSpinBox_VolumenMaische -> setValue(wasser + (doubleSpinBox_S_Gesammt -> value() * 0.7));
}


void MainWindowImpl::BerPfanneVoll()
{
  doubleSpinBox_VolumenPfannevoll -> setValue(
        Berechnungen.GetPfanneVoll(getVerdampfungsziffer(),
                                   spinBox_Gesammtkochdauer->value(),
                                   spinBox_Menge -> value()/highGravityFaktor ) );
}


//Exportiert den Aktuell ausgewählten Sud in eine xsud Exportdatei
void MainWindowImpl::on_pushButton_SudExport_clicked()
{
  int row = tableWidget_Sudauswahl -> currentRow();
  int SudID = tableWidget_Sudauswahl -> item(row,0) -> text().toInt();
  QString Sudname = tableWidget_Sudauswahl -> item(row,1) -> text();

  QString s;
  QString p;
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  p = settings.value("recentExportPath").toString();
  if (p == "") {
    p = QDir::homePath();
  }
  //Sudname anhängen
  p += "/" + Sudname + ".xsud";
  s = QFileDialog::getSaveFileName(this, trUtf8("Export Sud"), p, trUtf8("KBH Sud Export Dateien (*.xsud)") + ";;" + trUtf8("BeerXML (*.xml)"),0);
  if (!s.isEmpty()) {
    QFileInfo fileinfo(s);
    settings.setValue("recentExportPath",fileinfo.path());
    if (s.right(4) == ".xml") {
      Export.ExportBeerXML(SudID, s);
    }
    else {
      Export.ExportSudXML(SudID, s);
    }
  }

}

//Importiert eine xsud Exportdatei
void MainWindowImpl::on_pushButton_SudImport_clicked()
{
  //Dateiauswahldialog starten
  QString s;
  QString p;
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  p = settings.value("recentExportPath").toString();
  if (p == "") {
    p = QDir::homePath();
  }
  s = QFileDialog::getOpenFileName(this, trUtf8("Suddatei öffnen"), p, trUtf8("Sud Export Dateien (*.xsud);; Maische Malz und Mehr (*.json)"),0);
  if (!s.isEmpty()) {
    QFileInfo fileinfo(s);
    settings.setValue("recentExportPath",fileinfo.path());
    // Überprüfen ob eine gültige JSON Datei vorliegt
    QTemporaryFile file;
    if (s.endsWith(".json")) {
        if (file.open()) {
            QString tmpFile = file.fileName();
            Export.convertJSON(s,tmpFile);
            s = tmpFile;
        }
    }
    //Überprüfen ob eine gültige xml Datei vorliegt
    int r = Export.IfXmlOK(s);
    //Datei konnte nicht geöffnet werden
    if (r == 1) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_XML_OPEN, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Dateiname: ") + s);
    }
    //Fehler beim Parsen der Datei
    else if (r == 2){
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_XML_PARSEN, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Dateiname: ") + s + "\n"
                                  + trUtf8("Message: ") + Export.errMsg + "\n"
                                  + trUtf8("Zeile: ") + QString::number(Export.errLine) + "\n"
                                  + trUtf8("Spalte: ") + QString::number(Export.errCol) + "\n");
    }
    //Fehler beim Auslesen der Verionen
    else if (r == 3){
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_XML_PARSEN, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Dateiname: ") + s + "\n"
                                  + trUtf8("Message: ") + Export.errMsg + "\n");
    }

    //Importieren starten
    r = Export.ImportSudXML(s);
    //Datei konnte nicht geöffnet werden
    if (r == 1) {
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_XML_OPEN, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Dateiname: ") + s);
    }
    //Fehler beim Parsen der Datei
    else if (r == 2){
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_XML_PARSEN, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Dateiname: ") + s + "\n"
                                  + trUtf8("Message: ") + Export.errMsg + "\n"
                                  + trUtf8("Zeile: ") + QString::number(Export.errLine) + "\n"
                                  + trUtf8("Spalte: ") + QString::number(Export.errCol) + "\n");
    }
    //Fehler beim Auslesen der Suddaten
    else if (r == 3){
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_XML_PARSEN, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Dateiname: ") + s + "\n"
                                  + trUtf8("Message: ") + Export.errMsg + "\n");
    }
    else if (r == 0){
      //Sudauswahl aktuallisieren
      FuelleSudauswahl();
      //Rohstofftabellen neu einlesen für den Fall das neue Rohstoffe hinzugefügt wurden
      Gestartet = false;
      LeseRohstoffeDB();
      AenderungRohstofftabelle = true;
      Gestartet = true;
      QMessageBox::information ( this, "", trUtf8("Sud wurde Importiert"));
    }
  }

}

void MainWindowImpl::on_tableWidget_Brauuebersicht_itemSelectionChanged()
{
  int i = tableWidget_Brauuebersicht -> currentRow();
  QDateTime dt;
  dt.setDate(tableWidget_Brauuebersicht -> item(i,2) -> data(Qt::DisplayRole).toDate());
  widget_DiaBrauUebersicht -> MarkierePunkt(dt);
}

void MainWindowImpl::on_SpinBox_wwCalcium_mg_valueChanged(double )
{
  if (SpinBox_wwCalcium_mg -> hasFocus()){
    //umrechnen in mmol
    double d;
    d = SpinBox_wwCalcium_mg -> value();
    d = d / 40.8;
    SpinBox_wwCalcium_mmol -> setValue(d);
    //in dh Umrechnen
    d = d / 0.1783;
    SpinBox_wwCalciumhaerte_dh -> setValue(d);
    BerRestalkalitaet();
  }
}

void MainWindowImpl::on_SpinBox_wwCalcium_mmol_valueChanged(double )
{
  if (SpinBox_wwCalcium_mmol -> hasFocus()){
    //umrechnen in mg
    double d;
    d = SpinBox_wwCalcium_mmol -> value();
    d = d * 40.8;
    SpinBox_wwCalcium_mg -> setValue(d);
    //in dh Umrechnen
    d = SpinBox_wwCalcium_mmol -> value() / 0.1783;
    SpinBox_wwCalciumhaerte_dh -> setValue(d);
    BerRestalkalitaet();
  }
}

void MainWindowImpl::on_SpinBox_wwMagnesium_mg_valueChanged(double )
{
  if (SpinBox_wwMagnesium_mg -> hasFocus()){
    //umrechnen in mmol
    double d;
    d = SpinBox_wwMagnesium_mg -> value();
    d = d / 24.3;
    SpinBox_wwMagnesium_mmol -> setValue(d);
    //in dh Umrechnen
    d = d / 0.1783;
    SpinBox_wwMagnesiumhaerte_dh -> setValue(d);
    BerRestalkalitaet();
  }
}

void MainWindowImpl::on_SpinBox_wwMagnesium_mmol_valueChanged(double )
{
  if (SpinBox_wwMagnesium_mmol -> hasFocus()){
    //umrechnen in mg
    double d;
    d = SpinBox_wwMagnesium_mmol -> value();
    d = d * 24.3;
    SpinBox_wwMagnesium_mg -> setValue(d);
    //in dh Umrechnen
    d = SpinBox_wwMagnesium_mmol -> value() / 0.1783;
    SpinBox_wwMagnesiumhaerte_dh -> setValue(d);
    BerRestalkalitaet();
  }
}

void MainWindowImpl::on_SpinBox_wwSaeurekapazitaet_mmol_valueChanged(double )
{
  if (SpinBox_wwSaeurekapazitaet_mmol -> hasFocus()){
    SpinBox_wwCarbonathaerte_dh -> setValue(SpinBox_wwSaeurekapazitaet_mmol -> value() * 2.8);
    BerRestalkalitaet();
  }
}

void MainWindowImpl::on_SpinBox_wwCarbonathaerte_dh_valueChanged(double )
{
  if (SpinBox_wwCarbonathaerte_dh -> hasFocus()){
    SpinBox_wwSaeurekapazitaet_mmol -> setValue(SpinBox_wwCarbonathaerte_dh -> value() / 2.8);
    BerRestalkalitaet();
  }
}

void MainWindowImpl::BerWasserwerte()
{
  //Calcium
  double d;
  d = SpinBox_wwCalcium_mg -> value();
  d = d / 40.8;
  SpinBox_wwCalcium_mmol -> setValue(d);
  //in dh Umrechnen
  d = d / 0.1783;
  SpinBox_wwCalciumhaerte_dh -> setValue(d);

  //Magnesium
  d = SpinBox_wwMagnesium_mg -> value();
  d = d / 24.3;
  SpinBox_wwMagnesium_mmol -> setValue(d);
  //in dh Umrechnen
  d = d / 0.1783;
  SpinBox_wwMagnesiumhaerte_dh -> setValue(d);

  //Carbonathärte
  SpinBox_wwCarbonathaerte_dh -> setValue(SpinBox_wwSaeurekapazitaet_mmol -> value() * 2.8);

  //Restalkalität berechnen
  BerRestalkalitaet();

  //Milchsäurezugabe berechnen
  //gewünschte RA Reduzierung
  double RA_Reduzierung = SpinBox_wwRestalkalitaet_dh -> value()
      - SpinBox_waSollRestalkalitaet_dh -> value();

  //Hauptguss
  SpinBox_waMilchsaeureHG_ml -> setValue(RA_Reduzierung * 0.033333333
                                         * doubleSpinBox_WHauptguss -> value());
  SpinBox_waSauermalz_pr -> setValue(RA_Reduzierung * 0.4);
  SpinBox_waSauermalz_g -> setValue(doubleSpinBox_S_Gesammt -> value() / 100 * SpinBox_waSauermalz_pr -> value() * 1000);
  //Nachguss
  SpinBox_waMilchsaeureNG_ml -> setValue(RA_Reduzierung * 0.033333333
                                         * doubleSpinBox_WNachguss -> value());

  widget_MilchsauereHG->setVisible(SpinBox_waMilchsaeureHG_ml->value() > 0.0);
  widget_MilchsauereNG->setVisible(SpinBox_waMilchsaeureNG_ml->value() > 0.0);
  widget_SauermalzHG->setVisible(false);
}


void MainWindowImpl::BerRestalkalitaet()
{
  // Restalkalität
  double d;
  d = SpinBox_wwCarbonathaerte_dh -> value() - (
        (SpinBox_wwCalciumhaerte_dh -> value() + 0.5
         * SpinBox_wwMagnesiumhaerte_dh -> value()) / 3.5);
  SpinBox_wwRestalkalitaet_dh -> setValue(d);
  //Maxwert der Sollrestalkalität setzten
  SpinBox_waSollRestalkalitaet_dh -> setMaximum(d);
}


void MainWindowImpl::on_spinBox_SWVorHopfenseihen_valueChanged(double )
{
  //if (spinBox_SWVorHopfenseihen -> hasFocus()) {
  //spinBox_SWKochende -> setValue(spinBox_SWVorHopfenseihen -> value());
  //}
}

void MainWindowImpl::on_tableWidget_Sudauswahl_cellDoubleClicked(int , int )
{
  slot_pushButton_SudLaden();
}

void MainWindowImpl::on_tableWidget_Brauuebersicht_cellDoubleClicked(int , int )
{
  on_pushButton_SudLadenBUebersicht_clicked();
}

void MainWindowImpl::on_tableWidget_Sudauswahl_itemSelectionChanged()
{
  ErstelleSudInfo();
  //Buttons zum Laden etc. ein/Ausblenden
  if (tableWidget_Sudauswahl -> selectedItems().count() == 4
      || tableWidget_Sudauswahl -> selectedItems().count() == 5) {
    //Alle Buttons enablen
    pushButton_SudDel -> setDisabled(false);
    pushButton_SudExport -> setDisabled(false);
    pushButton_SudKopie -> setDisabled(false);
    pushButton_SudLaden -> setDisabled(false);
    pushButton_merken->setDisabled(false);
    pushButton_vergessen->setDisabled(false);
  }
  else {
    pushButton_SudDel -> setDisabled(true);
    pushButton_SudExport -> setDisabled(true);
    pushButton_SudKopie -> setDisabled(true);
    pushButton_SudLaden -> setDisabled(true);
    pushButton_merken->setDisabled(true);
    pushButton_vergessen->setDisabled(true);
  }
  tableWidget_Sudauswahl->setFocus();
}

void MainWindowImpl::on_TabWidget_Zutaten_currentChanged(int index)
{
    TabWidget_RezeptErgebnisse->setCurrentIndex(index);
}

void MainWindowImpl::on_TabWidget_RezeptErgebnisse_currentChanged(int index)
{
    TabWidget_Zutaten->setCurrentIndex(index);
}


void MainWindowImpl::on_pushButton_WeitereZutatenDel_clicked()
{
  //Überprüfen ob bei nicht gebrauten Suden der Rohstoff verwendet wird.

  //Rohstoffname
  QString del_name = tableWidget_WeitereZutaten -> item(tableWidget_WeitereZutaten -> currentRow(),TableWZutatColName) -> text();

  bool ok=true;
  //Alle Sude Abfragen die noch nicht gebraut wurden
  QString sql = "SELECT * FROM Sud WHERE BierWurdeGebraut == 0";
  QSqlQuery query;
  if (!query.exec(sql)) {
    //Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    QString letzeAuswahl;
    QString sudid;
    save();
    while (query.next()){
      if (ok){
        QSqlQuery query2;
        //alle gefundenen Sude überprüfen
        int FeldNr = query.record().indexOf("ID");
        sudid = query.value(FeldNr).toString();



        QString sql2 = "SELECT * FROM WeitereZutatenGaben WHERE Name='"+del_name.replace("'","''")+"' AND SudID="+sudid;
        if (!query2.exec(sql2)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                      + trUtf8("\nSQL Befehl:\n") + sql2);
        }
        else {
          if ((ok) && (query2.first())){
            //Dialog mit Rohstoffauswahl zum Austauschen des zu löschenden Rohstoffes anzeigen
            RohstoffAustauschen raDia;

            //Text für Dialog erstellen
            QString text = trUtf8("Der Rohstoff <b>")+del_name+trUtf8("</b> den Du löschen möchtest, wird in den Weiteren Zutaten in dem noch nicht gebrauten Sud <b>");
            int FeldNr = query.record().indexOf("Sudname");
            text += query.value(FeldNr).toString()+"</b> verwendet.\n\nSoll dieser Eintrag mit der folgenden Auswahl ersetzt werden?";
            raDia.SetText(text);
            //Auswahl für Ersetzung füllen
            for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
              if (del_name != tableWidget_WeitereZutaten -> item(i,TableWZutatColName) -> text()){
                raDia.addAuswahlEintrag(tableWidget_WeitereZutaten -> item(i,TableWZutatColName) -> text() );
              }
            }
            raDia.setAktAuswahl(letzeAuswahl);
            raDia.exec();
            ok = raDia.b_ok;
            letzeAuswahl = raDia.GetAktAuswahl();
            //Austauschen
            if (ok){
              QString sql2 = "UPDATE WeitereZutatenGaben SET Name='"+letzeAuswahl+"' WHERE Name='"
                  +del_name+"' AND SudID="+sudid;
              if (!query2.exec(sql2)) {
                // Fehlermeldung Datenbankabfrage
                ErrorMessage *errorMessage = new ErrorMessage();
                errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                            CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                            + trUtf8("\nSQL Befehl:\n") + sql);
              }
              else {
                //Bit setzen das dieser Sud neu Berechnet werden muss da die Berechneten wert nun nicht mehr stimmen.
                sql2 ="UPDATE Sud SET NeuBerechnen= 1 WHERE  ID= " + sudid;
                if (!query2.exec(sql2)) {
                  // Fehlermeldung Datenbankabfrage
                  ErrorMessage *errorMessage = new ErrorMessage();
                  errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                              CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                              + trUtf8("\nSQL Befehl:\n") + sql2);
                }
              }
            }
          }
        }
      }
    }
    LadeSudDB(false);
    ErstelleSudInfo();
  }
  if (ok){
    tableWidget_WeitereZutaten -> removeRow(tableWidget_WeitereZutaten -> currentRow());
    setAenderung(true);
    AenderungRohstofftabelle = true;
  }
}


void MainWindowImpl::on_pushButton_EWZ_Hinzufuegen_clicked()
{
  //Zutatenobjekt hinzufügen
  ErweiterteZutatImpl* ewz = new ErweiterteZutatImpl(this);
  ewz -> setStyleDunkel(StyleDunkel);
  ewz -> setAttribute(Qt::WA_DeleteOnClose);

  ewz -> setBierWurdeGebraut(false);
  ewz -> setBierWurdeAbgefuellt(false);
  connect(ewz, SIGNAL( sig_vorClose(int) ), this, SLOT( slot_ewzClose(int) ));
  connect(ewz, SIGNAL( sig_getEwzTyp(QString) ), this, SLOT( slot_getEwzTyp(QString) ));
  connect(ewz, SIGNAL( sig_getEwzEinheit(QString) ), this, SLOT( slot_getEwzEinheit(QString) ));
  connect(ewz, SIGNAL( sig_getEwzAusbeute(QString) ), this, SLOT( slot_getEwzAusbeute(QString) ));
  connect(ewz, SIGNAL( sig_getEwzFarbe(QString) ), this, SLOT( slot_getEwzFarbe(QString) ));
  connect(ewz, SIGNAL( sig_getEwzPreis(QString) ), this, SLOT( slot_getEwzPreis(QString) ));
  connect(ewz, SIGNAL( sig_getEwzPreisHopfen(QString) ), this, SLOT( slot_getEwzPreisHopfen(QString) ));
  connect(ewz, SIGNAL( sig_Aenderung() ), this, SLOT( slot_EwzAenderung() ));
  connect(ewz, SIGNAL( sig_zugeben(QString, int, double) ), this, SLOT( slot_EwzZugegeben(QString, int, double) ));
  connect(ewz, SIGNAL( sig_getHopfenMenge(QString) ), this, SLOT( slot_HopfenGetMenge(QString) ));
  connect(ewz, SIGNAL( sig_getEwzMenge(QString) ), this, SLOT( slot_EwzGetMenge(QString) ));
  //Zutatenliste füllen
  ewz -> setEwListe(ewzListe);
  ewz -> setHopfenListe(HopfenListe);

  verticalLayout_WeitereZutaten -> addWidget(ewz);
  list_EwZutat.append(ewz);
  ewz -> setID((int)time(NULL)+rand());

  //Ergebnisswidget dem Layout zuordnen
  verticalLayout_BerWeitereZutaten -> addWidget(ewz->ergWidget);

  setAenderung(true);
}

void MainWindowImpl::on_pushButton_HopfenHinzufuegen_clicked()
{
  //Hopfen hinzufügen
  AddHopfengabe(false,"",10,10,double(10),double(10),1);

  setAenderung(true);
  BerAlles();
}

void MainWindowImpl::on_pushButton_MalzHinzufuegen_clicked()
{
  //Malz hinzufügen
  AddMalzgabe("",10,0,0);

  setAenderung(true);
  BerAlles();
}


void MainWindowImpl::slot_ewzClose(int id)
{
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getID() == id){
      list_EwZutat.removeAt(i);
      i = list_EwZutat.count();
      setAenderung(true);
    }
  }
}

void MainWindowImpl::slot_malzClose(int id)
{
  for (int i=0; i < list_Malzgaben.count(); i++){
    if (list_Malzgaben[i] -> getID() == id){
      list_Malzgaben.removeAt(i);
      i = list_Malzgaben.count();
      setAenderung(true);
    }
  }
}

void MainWindowImpl::slot_hopfenClose(int id)
{
  for (int i=0; i < list_Hopfengaben.count(); i++){
    if (list_Hopfengaben[i] -> getID() == id){
      list_Hopfengaben.removeAt(i);
      i = list_Hopfengaben.count();
      setAenderung(true);
    }
  }
}

void MainWindowImpl::slot_rastClose(int id)
{
  for (int i=0; i < list_Rasten.count(); i++){
    if (list_Rasten[i] -> getID() == id){
      list_Rasten.removeAt(i);
      i = list_Rasten.count();
      setAenderung(true);
    }
  }
  if (list_Rasten.count() > RAST_ANIMATION_STOP){
    for (int i=0; i<list_Rasten.count(); i++){
      list_Rasten[i]->setAnimationAus(true);
    }
  }
  else {
    for (int i=0; i<list_Rasten.count(); i++){
      list_Rasten[i]->setAnimationAus(false);
    }
  }
}

void MainWindowImpl::slot_bewClose(int id)
{
  if (label_bew_ID->text().toInt() == id ){
    widget_bewertung->setEnabled(false);
    graphicsView_bewStar->setAnzahlStar(0);
  }
  for (int i=0; i < list_Bewertung.count(); i++){
    if (list_Bewertung[i]->getID() == id){
      list_Bewertung.removeAt(i);
      i = list_Bewertung.count();
      setAenderung(true);
    }
  }
}

void MainWindowImpl::slot_rastAenderung(int )
{
  setAenderung(true);
}


void MainWindowImpl::ErstelleZutatenlisten()
{
  ewzListe.clear();
  //Alle Einträge aus den Rohstoffen Erweiterte Zutaten Hinzufügen
  for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
    ewzListe.append(tableWidget_WeitereZutaten -> item(i,TableWZutatColName) -> text());
  }
  //Alle Hopfeneintäge hinzufügen
  HopfenListe.clear();
  for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
    HopfenListe.append(tableWidget_Hopfen -> item(i,TableHopfenColName) -> text());
  }
  //Liste für Malzeinträge
  MalzListe.clear();
  for (int i=0; i < tableWidget_Malz -> rowCount(); i++){
    MalzListe.append(tableWidget_Malz -> item(i,TableMalzColName) -> text());
  }

}

int MainWindowImpl::getBewertungsIndex()
{
  int id=(label_bew_ID->text().toInt());
  int r=-1;
  for (int i=0; i<list_Bewertung.count(); i++){
    if (list_Bewertung[i] -> getID() == id){
      r = i;
    }
  }
  return r;
}


int MainWindowImpl::slot_getEwzTyp(QString zutat)
{
  for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
    if (tableWidget_WeitereZutaten -> item(i,TableWZutatColName) -> text() == zutat){
      QComboBox* comboTyp=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColTyp);
      return comboTyp -> currentIndex();
    }
  }
  return -1;
}


int MainWindowImpl::slot_getEwzEinheit(QString zutat)
{
  for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
    if (tableWidget_WeitereZutaten -> item(i,TableWZutatColName) -> text() == zutat){
      QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColEinheit);
      return comboEinheit -> currentIndex();
    }
  }
  return -1;
}


int MainWindowImpl::slot_getEwzAusbeute(QString zutat)
{
  for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
    if (tableWidget_WeitereZutaten -> item(i,TableWZutatColName) -> text() == zutat){
      QDoubleSpinBox *spinBoxAusbeute =(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColAusbeute);
      return spinBoxAusbeute -> value();
    }
  }
  return -1;
}


double MainWindowImpl::slot_getEwzFarbe(QString zutat)
{
  for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
    if (tableWidget_WeitereZutaten -> item(i,TableWZutatColName) -> text() == zutat){
      QDoubleSpinBox *spinBoxFarbe =(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColFarbe);
      return spinBoxFarbe -> value();
    }
  }
  return -1;
}


double MainWindowImpl::slot_getEwzPreis(QString zutat)
{
  for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
    if (tableWidget_WeitereZutaten -> item(i,TableWZutatColName) -> text() == zutat){
      QDoubleSpinBox *spinBoxPreis =(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColPreis);
      return spinBoxPreis -> value();
    }
  }
  return -1;
}


double MainWindowImpl::slot_getEwzPreisHopfen(QString zutat)
{
  for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
    if (tableWidget_Hopfen -> item(i,TableHopfenColName) -> text() == zutat){
      QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,TableHopfenColPreis);
      return spinBox->value();
    }
  }
  return -1;
}

void MainWindowImpl::slot_EwzZugegeben(QString zutat, int typ, double menge)
{

  //Abfrage ob Rohstoffe vom Bestand abgezogen werden sollen
  QMessageBox msgBox;
  msgBox.setWindowTitle("kleine-frage");
  msgBox.setInformativeText("");
  msgBox.setText(trUtf8("Soll die Zutat") + " " + zutat + " " + trUtf8("vom bestand abgezogen werden?"));
  msgBox.setIcon(QMessageBox::Question);
  //msgBox.setDefaultButton(QMessageBox::Save);
  QPushButton *JaButton = msgBox.addButton(trUtf8("Ja"), QMessageBox::ActionRole);
  msgBox.addButton(trUtf8("Nein"), QMessageBox::ActionRole);

  msgBox.exec();

  if (msgBox.clickedButton() == JaButton){
    double d;
    //wenn hopfen
    if (typ == EWZ_Typ_Hopfen) {
      //verwendeten Rohstoff vom Bestand abziehen
      for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
        if (tableWidget_Hopfen -> item(i,TableHopfenColName) -> text() == zutat){
          QDoubleSpinBox* dsbMenge = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,TableHopfenColMenge);
          d = dsbMenge ->value();
          d -= menge;
          if (d < 0)
            d = 0;
          dsbMenge ->setValue(d);
        }
      }
    }
    //alles andere
    else {
      for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
        if (tableWidget_WeitereZutaten -> item(i,TableWZutatColName) -> text() == zutat){
          QDoubleSpinBox *dsbMenge =(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColMenge);
          d = dsbMenge ->value();
          d -= menge;
          if (d < 0)
            d = 0;
          dsbMenge ->setValue(d);
        }
      }
    }
  }

}


void MainWindowImpl::SchreibeErweiterteZutatenDB()
{
  //Alle Erweiterten Zutaten in diesem Sud aus der Datenbank löschen
  // und dann neu schreiben
  QSqlQuery query;
  QString sql = "DELETE FROM WeitereZutatenGaben WHERE SudID =" + QString::number(AktuelleSudID);
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

  for (int i=0; i < list_EwZutat.count(); i++){
    sql = "INSERT INTO WeitereZutatenGaben(SudID, Name, Menge, Einheit, Typ, Zeitpunkt,";
    sql += "Bemerkung, erg_Menge, Ausbeute, Zeitpunkt_von, Zeitpunkt_bis, Entnahmeindex, Zugabestatus, Zugabedauer, Farbe) VALUES(" +
        QString::number(AktuelleSudID) +	"," +
        "'" + list_EwZutat[i] -> getName().replace("'","''") +	"'," +
        QString::number(list_EwZutat[i] -> getMenge()) +	"," +
        QString::number(list_EwZutat[i] -> getEinheit()) +	"," +
        QString::number(list_EwZutat[i] -> getTyp()) +	"," +
        QString::number(list_EwZutat[i] -> getZeitpunkt()) +	"," +
        "'" + list_EwZutat[i] -> getBemerkung().replace("'","''") +	"'," +
        QString::number(list_EwZutat[i] -> getErg_Menge()) +	"," +
        QString::number(list_EwZutat[i] -> getAusbeute()) +	"," +
        "\"" + list_EwZutat[i] -> getZugabezeitpunkt_von().toString(Qt::ISODate) +	"\"," +
        "\"" + list_EwZutat[i] -> getZugabezeitpunkt_bis().toString(Qt::ISODate) +	"\"," +
        QString::number(list_EwZutat[i] -> getEntnahmeindex()) +	"," +
        QString::number(list_EwZutat[i] -> getZugabestatus()) +	"," +
        QString::number(list_EwZutat[i] -> getDauerMinuten()) +	"," +
        QString::number(list_EwZutat[i] -> getFarbe()) +	"" +
        +")";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
  }
}


void MainWindowImpl::slot_EwzAenderung()
{
  if (Gestartet && !AmLaden) {
    setAenderung(true);
    BerAlles();
  }
}

void MainWindowImpl::slot_MalzAenderung()
{
  if (Gestartet && !AmLaden) {
    setAenderung(true);
    BerAlles();
  }
}

void MainWindowImpl::slot_HopfenAenderung()
{
  if (Gestartet && !AmLaden) {
    setAenderung(true);
    BerAlles();
  }
}

//Gibt die noch vorhandene Restmenge zurück
double MainWindowImpl::slot_MalzGetMenge(QString name)
{
  double rest = 0;
  for (int i=0; i < tableWidget_Malz -> rowCount(); i++){
    if (tableWidget_Malz -> item(i,TableMalzColName) -> text() == name){
      QDoubleSpinBox *spinBoxMenge =(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,TableMalzColMenge);
      if (spinBoxMenge)
        rest = spinBoxMenge -> value();
    }
  }
  //Alle verwendeten Malzgaben mit dem gleichen Namen abfragen
  double verwendet = 0;
  if (rest > 0) {
    for (int i=0; i < list_Malzgaben.count(); i++){
      if (list_Malzgaben[i] -> getName() == name)
        verwendet  += list_Malzgaben[i] -> getErgMenge();
    }
  }
  return rest - verwendet;
}

//Gibt die noch vorhandene Restmenge zurück
double MainWindowImpl::slot_EwzGetMenge(QString name)
{
  double rest = 0;
  int Einheit=0;
  for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
    if (tableWidget_WeitereZutaten -> item(i,TableWZutatColName) -> text() == name){
      QDoubleSpinBox *spinBoxMenge =(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColMenge);
      if (spinBoxMenge)
        rest = spinBoxMenge -> value();
      QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(i,TableWZutatColEinheit);
      if (comboEinheit)
        Einheit = comboEinheit -> currentIndex();
      if (Einheit == EWZ_Einheit_Kg){
        rest = rest * 1000;
      }
    }
  }
  //Nun überprüfen ob die zutat in den weiteren Zutaten noch einmal vorkommt
  double verwendet = 0;
  for (int o=0; o < list_EwZutat.count(); o++){
    if ((list_EwZutat[o] -> getName() ==  name) && (list_EwZutat[o] -> getTyp() != EWZ_Typ_Hopfen)) {
      verwendet += list_EwZutat[o] -> getErg_Menge();
    }
  }
  return rest - verwendet;
}

//Gibt die noch vorhandene Restmenge zurück
double MainWindowImpl::slot_HopfenGetMenge(QString name)
{
  double rest = 0;
  for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
    if (tableWidget_Hopfen -> item(i,TableHopfenColName) -> text() == name){
      QDoubleSpinBox *spinBoxMenge =(QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,TableHopfenColMenge);
      if (spinBoxMenge)
        rest = spinBoxMenge -> value();
    }
  }
  //Alle verwendeten Hopfengaben mit dem gleichen Namen abfragen
  double verwendet = 0;
  //Bei den Hopfengaben
  if (rest > 0) {
    for (int i=0; i < list_Hopfengaben.count(); i++){
      if (list_Hopfengaben[i] -> getName() == name)
        verwendet += list_Hopfengaben[i] -> getErgMenge();
    }
    //Bei den Weiteren Zutaten
    for (int o=0; o < list_EwZutat.count(); o++){
      if ((list_EwZutat[o] -> getName() ==  name) && (list_EwZutat[o] -> getTyp() == EWZ_Typ_Hopfen)) {
        verwendet += list_EwZutat[o] -> getErg_Menge();
      }
    }
  }
  rest = rest - verwendet;
  if (rest < 0)
    rest = 0;
  return rest;
}

double MainWindowImpl::slot_HefeGetMenge(QString name)
{
  for (int i=0; i < tableWidget_Hefe -> rowCount(); i++){
    if (tableWidget_Hefe -> item(i,TableHefeColName) -> text() == name){
      QDoubleSpinBox *spinBoxMenge =(QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,TableHefeColMenge);
      if (spinBoxMenge)
        return spinBoxMenge -> value();
    }
  }
  return -1;
}


void MainWindowImpl::BerWeitereZutaten()
{
  //Alle Mengen der Weiteren Zutaten Berechnen
  for (int i=0; i < list_EwZutat.count(); i++){
    list_EwZutat[i] -> setErg_Menge(list_EwZutat[i] -> getMenge() * spinBox_Menge -> value());
  }
  sw_ewz = 0;

  //gesammtmenge der erweiterten zutaten mit Ausbeute verrechnet (pro Liter).
  double gs_ewz = 0;
  //menge der weiteren zutaten die beim Kochen dabei sind
  double gs_ewz_kochen = 0;
  //anteil der stammwürze der weiteren zutaten beim kochen
  double sw_ewz_kochen = 0;
  for (int i=0; i < list_EwZutat.count(); i++){
    //wenn die erweiterte Zutat eine Ausbeute über 0 hat wird sie berücksichtigt
    if (list_EwZutat[i] -> getAusbeute() > 0){
      //wenn Typ Hopfen ist ignorieren
      if (list_EwZutat[i] -> getTyp() != 100){
        gs_ewz += list_EwZutat[i] -> getMenge() * list_EwZutat[i] -> getAusbeute() / 100;
        //wenn die zutat schon beim kochen dabei ist
        if (list_EwZutat[i] -> getZeitpunkt() > 0) {
          gs_ewz_kochen += list_EwZutat[i] -> getMenge() * list_EwZutat[i] -> getAusbeute() / 100;
        }
      }
    }
  }
  //Berechnen wieviel der soll-Stammwürze durch die erweiterten Zutaten erziehlt wird.
  if (BierWurdeGebraut){
    sw_kochen = spinBox_SWAnstellen -> value();
    sw_ewz = gs_ewz / 10;
    sw_ewz_kochen = gs_ewz_kochen / 10;
    sw_ewz_gaerung = sw_ewz - sw_ewz_kochen;
    sw_gesammt = sw_kochen + (sw_ewz - sw_ewz_kochen);
    sw_schuettung = sw_gesammt - sw_ewz;
  }
  else{
    sw_ewz = gs_ewz / 10;
    sw_schuettung = sw_gesammt - sw_ewz;
    sw_ewz_kochen = gs_ewz_kochen / 10;
    sw_ewz_gaerung = sw_ewz - sw_ewz_kochen;
    sw_kochen = sw_schuettung + sw_ewz_kochen;
  }

  //Stammwürze beim Kochen mit High Gravity Faktor verrechnen
  sw_kochen = sw_kochen*highGravityFaktor;

}

void MainWindowImpl::fuelleComboEwzZugeben()
{
  comboBox_GaerungEwzAuswahl->clear();
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i]->getZeitpunkt() == EWZ_Zeitpunkt_Gaerung) {
      if (list_EwZutat[i]->getZugabestatus() == EWZ_Zugabestatus_nichtZugegeben) {
        comboBox_GaerungEwzAuswahl->addItem(list_EwZutat[i]->getName());
      }
    }
  }
  if (comboBox_GaerungEwzAuswahl->count() == 0) {
    widget_EwzZugeben->setVisible(false);
  }
  else {
    widget_EwzZugeben->setVisible(true);
  }

}

void MainWindowImpl::fuelleComboEwzEntnehmen()
{
  comboBox_GaerungEwzAuswahlEntnahme->clear();
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i]->getZeitpunkt() == EWZ_Zeitpunkt_Gaerung) {
      if (list_EwZutat[i]->getZugabestatus() == EWZ_Zugabestatus_Zugegeben && list_EwZutat[i]->getEntnahmeindex() == EWZ_Entnahmeindex_MitEntnahme) {
        comboBox_GaerungEwzAuswahlEntnahme->addItem(list_EwZutat[i]->getName());
      }
    }
  }
  if (comboBox_GaerungEwzAuswahlEntnahme->count() == 0) {
    widget_EwzEntnehmen->setVisible(false);
  }
  else {
    widget_EwzEntnehmen->setVisible(true);
  }

}


void MainWindowImpl::on_pushButton_NeueRast_clicked()
{
  //Rastobjekt erstellen
  Rastwidget* rast = new Rastwidget(scrollArea_7);
  rast -> setAttribute(Qt::WA_DeleteOnClose);

  verticalLayout_Rasten -> addWidget(rast);
  list_Rasten.append(rast);
  rast->setID((int)time(NULL)+rand());

  if (list_Rasten.count() > RAST_ANIMATION_STOP){
    for (int i=0; i<list_Rasten.count(); i++){
      list_Rasten[i]->setAnimationAus(true);
    }
  }
  else {
    for (int i=0; i<list_Rasten.count(); i++){
      list_Rasten[i]->setAnimationAus(false);
    }
  }
  connect(rast, SIGNAL( sig_vorClose(int) ), this, SLOT( slot_rastClose(int) ));
  connect(rast, SIGNAL( sig_aenderung(int) ), this, SLOT( slot_rastAenderung(int) ));
  connect(rast, SIGNAL( sig_nachOben(int) ), this, SLOT( slot_pushButton_RastNachOben(int) ));
  connect(rast, SIGNAL( sig_nachUnten(int) ), this, SLOT( slot_pushButton_RastNachUnten(int) ));
}

void MainWindowImpl::slot_pushButton_RastNachOben(int id)
{
  //Es darf nicht das erste Widget sein
  if (id != list_Rasten[0]->getID()){
    //alle elemente aus dem Layout entfernen
    for (int o=0; o < verticalLayout_Rasten->count(); o++){
      verticalLayout_Rasten->removeWidget(verticalLayout_Rasten->itemAt(0)->widget());
    }
    //in der Gespeicherten liste die Reihenfolge ändern
    for (int i=0; i<list_Rasten.count(); i++){
      if (id == list_Rasten[i]->getID()){
        list_Rasten.move(i,i-1);
        i = list_Rasten.count();
      }
    }
    //und die widgets dem layout wieder hinzufügen
    for (int i=0; i<list_Rasten.count(); i++){
      verticalLayout_Rasten -> addWidget(list_Rasten[i]);
    }
  }
}

void MainWindowImpl::slot_pushButton_RastNachUnten(int id)
{
  //Es darf nicht das letzte Widget sein
  if (id != list_Rasten[list_Rasten.count()-1]->getID()){
    //alle elemente aus dem Layout entfernen
    for (int o=0; o < verticalLayout_Rasten->count(); o++){
      verticalLayout_Rasten->removeWidget(verticalLayout_Rasten->itemAt(0)->widget());
    }
    //in der Gespeicherten liste die Reihenfolge ändern
    for (int i=0; i<list_Rasten.count(); i++){
      if (id == list_Rasten[i]->getID()){
        list_Rasten.move(i,i+1);
        i = list_Rasten.count();
      }
    }
    //und die widgets dem layout wieder hinzufügen
    for (int i=0; i<list_Rasten.count(); i++){
      verticalLayout_Rasten -> addWidget(list_Rasten[i]);
    }
  }
}

void MainWindowImpl::on_spinBox_WuerzemengeAnstellen_valueChanged(double arg1)
{
    if (Gestartet)
    {
        spinBox_JungbiermengeAbfuellen->setValue(arg1);
    }
}

void MainWindowImpl::on_spinBox_SWKochende_valueChanged(double arg1)
{
    if (Gestartet)
    {
        if (checkBox_zumischen->isChecked())
        {
            spinBox_WuerzemengeAnstellen->setValue(spinBox_WuerzemengeKochende->value() - spinBox_Speisemenge->value() + spinBox_WasserVerschneidung->value());
            spinBox_SWAnstellen->setValue(spinBox_SW->value());
        }
        else
        {
            spinBox_WuerzemengeAnstellen->setValue(spinBox_WuerzemengeKochende->value() - spinBox_Speisemenge->value());
            spinBox_SWAnstellen->setValue(arg1);
        }
    }
}

void MainWindowImpl::on_spinBox_WuerzemengeKochende_valueChanged(double arg1)
{
    if (Gestartet)
    {
        if (checkBox_zumischen->isChecked())
        {
            spinBox_WuerzemengeAnstellen->setValue(arg1 - spinBox_Speisemenge->value() + spinBox_WasserVerschneidung->value());
            spinBox_SWAnstellen->setValue(spinBox_SW->value());
        }
        else
        {
            spinBox_WuerzemengeAnstellen->setValue(arg1 - spinBox_Speisemenge->value());
            spinBox_SWAnstellen->setValue(spinBox_SWKochende->value());
        }
    }
}

void MainWindowImpl::on_spinBox_Speisemenge_valueChanged(double )
{
    if (Gestartet)
    {
        if (checkBox_zumischen->isChecked())
        {
            spinBox_WuerzemengeAnstellen->setValue(spinBox_WuerzemengeKochende->value() - spinBox_Speisemenge->value() + spinBox_WasserVerschneidung->value());
            spinBox_SWAnstellen->setValue(spinBox_SW->value());
        }
        else
        {
            spinBox_WuerzemengeAnstellen->setValue(spinBox_WuerzemengeKochende->value() - spinBox_Speisemenge->value());
            spinBox_SWAnstellen->setValue(spinBox_SWKochende->value());
        }
    }
}

void MainWindowImpl::on_spinBox_SW_valueChanged(double arg1)
{
    if (Gestartet)
    {
        spinBox_SWVorHopfenseihen->setValue(arg1);
        spinBox_SWKochende->setValue(arg1);
    }
}

void MainWindowImpl::on_spinBox_WuerzemengeVorHopfenseihen_valueChanged(double arg1)
{
    if (Gestartet)
    {
        spinBox_WuerzemengeKochende->setValue(arg1);
    }
}

void MainWindowImpl::on_pushButton_BewertungNeu_clicked()
{
  //Bewertung hinzufügen
  Bewertung* bew = new Bewertung(this);
  bew -> setAttribute(Qt::WA_DeleteOnClose);
  bew->setStyleDunkel(StyleDunkel);
  bew->setMaxSterne(MaxAnzahlSterne);

  verticalLayout_Bewertungen -> addWidget(bew);
  list_Bewertung.append(bew);

  //Datum für start Reifezeit ermitteln und setzten

  //Start der Reifung ermitteln indem das letzte Datum vom
  //Nachgärverlauf benutzt wird
  QSqlQuery queryN;
  QString sqlN = "SELECT * FROM Nachgaerverlauf WHERE SudID=" + QString::number(AktuelleSudID) + " ORDER BY Zeitstempel DESC;";
  QDate date;
  date = dateEdit_Abfuelldatum -> date();

  if (!queryN.exec(sqlN)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + queryN.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sqlN);
  }
  else {
    if (queryN.first()){
      int FeldNr = queryN.record().indexOf("Zeitstempel");
      date = QDate::fromString(queryN.value(FeldNr).toString(),Qt::ISODate);
    }
  }

  //Abfülldatum setzten
  bew->setReifedatum(date);
  bew->setID((int)time(NULL)+rand());
  connect(bew, SIGNAL( sig_wocheClicked(int)), this, SLOT( slot_BewertungWoche_clicked(int)));
  connect(bew, SIGNAL( sig_vorClose(int)), this, SLOT( slot_bewClose(int)));
  slot_BewertungWoche_clicked(bew->getID());
  setAenderung(true);
}

void MainWindowImpl::slot_BewertungWoche_clicked(int id)
{
  //aktuelle Bewertung aktiv auf falsch setzten
  int index = getBewertungsIndex();
  if (index > -1)
    list_Bewertung[getBewertungsIndex()]->setAktiv(false);

  //Bewertungsdaten füllen
  label_bew_ID->setText(QString::number(id));

  //Anhand der ID den Index für die Bewertung in der Objektliste ermitteln
  index = getBewertungsIndex();

  if (index > -1){
    //neue Bewertung auf aktiv setzten
    list_Bewertung[index]->setAktiv(true);
    //Bewertungdatum setzen
    dateEdit_Bewertungsdatum -> setDate(list_Bewertung[index]->getBewertungdatum());
    //woche
    label_bewWoche->setText(trUtf8("Woche ") + QString::number(list_Bewertung[index]->getWoche()));
    //Sterne
    graphicsView_bewStar->setAnzahlStar(list_Bewertung[index]->getSterne());

    //Globale Bemerkung
    textEdit_bewBemerkung->setText(list_Bewertung[index]->getBemerkung());

    //radio Buttons farbe
    int farbe = list_Bewertung[index]->getFarbe();
    radioButton_farbe_0->setAutoExclusive(false);
    radioButton_farbe_0->setChecked(false);
    radioButton_farbe_0->setAutoExclusive(true);

    radioButton_farbe_1->setAutoExclusive(false);
    radioButton_farbe_1->setChecked(false);
    radioButton_farbe_1->setAutoExclusive(true);

    radioButton_farbe_2->setAutoExclusive(false);
    radioButton_farbe_2->setChecked(false);
    radioButton_farbe_2->setAutoExclusive(true);

    radioButton_farbe_3->setAutoExclusive(false);
    radioButton_farbe_3->setChecked(false);
    radioButton_farbe_3->setAutoExclusive(true);

    radioButton_farbe_4->setAutoExclusive(false);
    radioButton_farbe_4->setChecked(false);
    radioButton_farbe_4->setAutoExclusive(true);

    radioButton_farbe_5->setAutoExclusive(false);
    radioButton_farbe_5->setChecked(false);
    radioButton_farbe_5->setAutoExclusive(true);

    radioButton_farbe_6->setAutoExclusive(false);
    radioButton_farbe_6->setChecked(false);
    radioButton_farbe_6->setAutoExclusive(true);

    radioButton_farbe_7->setAutoExclusive(false);
    radioButton_farbe_7->setChecked(false);
    radioButton_farbe_7->setAutoExclusive(true);

    radioButton_farbe_8->setAutoExclusive(false);
    radioButton_farbe_8->setChecked(false);
    radioButton_farbe_8->setAutoExclusive(true);

    radioButton_farbe_9->setAutoExclusive(false);
    radioButton_farbe_9->setChecked(false);
    radioButton_farbe_9->setAutoExclusive(true);

    radioButton_farbe_10->setAutoExclusive(false);
    radioButton_farbe_10->setChecked(false);
    radioButton_farbe_10->setAutoExclusive(true);

    if (farbe&1){
      radioButton_farbe_0->setChecked(true);
    }
    if (farbe&2){
      radioButton_farbe_1->setChecked(true);
    }
    if (farbe&4){
      radioButton_farbe_2->setChecked(true);
    }
    if (farbe&8){
      radioButton_farbe_3->setChecked(true);
    }
    if (farbe&16){
      radioButton_farbe_4->setChecked(true);
    }
    if (farbe&32){
      radioButton_farbe_5->setChecked(true);
    }
    if (farbe&64){
      radioButton_farbe_6->setChecked(true);
    }
    if (farbe&128){
      radioButton_farbe_7->setChecked(true);
    }
    if (farbe&256){
      radioButton_farbe_8->setChecked(true);
    }
    if (farbe&512){
      radioButton_farbe_9->setChecked(true);
    }
    if (farbe&1024){
      radioButton_farbe_10->setChecked(true);
    }

    //radio Buttons schaum
    int schaum = list_Bewertung[index]->getSchaum();
    radioButton_schaum_0->setAutoExclusive(false);
    radioButton_schaum_0->setChecked(false);
    radioButton_schaum_0->setAutoExclusive(true);

    radioButton_schaum_1->setAutoExclusive(false);
    radioButton_schaum_1->setChecked(false);
    radioButton_schaum_1->setAutoExclusive(true);

    radioButton_schaum_2->setAutoExclusive(false);
    radioButton_schaum_2->setChecked(false);
    radioButton_schaum_2->setAutoExclusive(true);

    radioButton_schaum_3->setAutoExclusive(false);
    radioButton_schaum_3->setChecked(false);
    radioButton_schaum_3->setAutoExclusive(true);

    radioButton_schaum_4->setAutoExclusive(false);
    radioButton_schaum_4->setChecked(false);
    radioButton_schaum_4->setAutoExclusive(true);

    radioButton_schaum_5->setAutoExclusive(false);
    radioButton_schaum_5->setChecked(false);
    radioButton_schaum_5->setAutoExclusive(true);

    radioButton_schaum_6->setAutoExclusive(false);
    radioButton_schaum_6->setChecked(false);
    radioButton_schaum_6->setAutoExclusive(true);

    radioButton_schaum_7->setAutoExclusive(false);
    radioButton_schaum_7->setChecked(false);
    radioButton_schaum_7->setAutoExclusive(true);

    radioButton_schaum_8->setAutoExclusive(false);
    radioButton_schaum_8->setChecked(false);
    radioButton_schaum_8->setAutoExclusive(true);

    radioButton_schaum_9->setAutoExclusive(false);
    radioButton_schaum_9->setChecked(false);
    radioButton_schaum_9->setAutoExclusive(true);

    radioButton_schaum_10->setAutoExclusive(false);
    radioButton_schaum_10->setChecked(false);
    radioButton_schaum_10->setAutoExclusive(true);

    if (schaum&1){
      radioButton_schaum_0->setChecked(true);
    }
    if (schaum&2){
      radioButton_schaum_1->setChecked(true);
    }
    if (schaum&4){
      radioButton_schaum_2->setChecked(true);
    }
    if (schaum&8){
      radioButton_schaum_3->setChecked(true);
    }
    if (schaum&16){
      radioButton_schaum_4->setChecked(true);
    }
    if (schaum&32){
      radioButton_schaum_5->setChecked(true);
    }
    if (schaum&64){
      radioButton_schaum_6->setChecked(true);
    }
    if (schaum&128){
      radioButton_schaum_7->setChecked(true);
    }
    if (schaum&256){
      radioButton_schaum_8->setChecked(true);
    }
    if (schaum&512){
      radioButton_schaum_9->setChecked(true);
    }
    if (schaum&1024){
      radioButton_schaum_10->setChecked(true);
    }

    //checkbox geruch
    int geruch = list_Bewertung[index]->getGeruch();
    checkBox_geruch_0->setChecked(false);
    checkBox_geruch_1->setChecked(false);
    checkBox_geruch_2->setChecked(false);
    checkBox_geruch_3->setChecked(false);
    checkBox_geruch_4->setChecked(false);
    checkBox_geruch_5->setChecked(false);
    checkBox_geruch_6->setChecked(false);
    checkBox_geruch_7->setChecked(false);
    checkBox_geruch_8->setChecked(false);
    checkBox_geruch_9->setChecked(false);
    checkBox_geruch_10->setChecked(false);
    checkBox_geruch_11->setChecked(false);

    if (geruch&1){
      checkBox_geruch_0->setChecked(true);
    }
    if (geruch&2){
      checkBox_geruch_1->setChecked(true);
    }
    if (geruch&4){
      checkBox_geruch_2->setChecked(true);
    }
    if (geruch&8){
      checkBox_geruch_3->setChecked(true);
    }
    if (geruch&16){
      checkBox_geruch_4->setChecked(true);
    }
    if (geruch&32){
      checkBox_geruch_5->setChecked(true);
    }
    if (geruch&64){
      checkBox_geruch_6->setChecked(true);
    }
    if (geruch&128){
      checkBox_geruch_7->setChecked(true);
    }
    if (geruch&256){
      checkBox_geruch_8->setChecked(true);
    }
    if (geruch&512){
      checkBox_geruch_9->setChecked(true);
    }
    if (geruch&1024){
      checkBox_geruch_10->setChecked(true);
    }
    if (geruch&2048){
      checkBox_geruch_11->setChecked(true);
    }

    //checkbox geschmack
    int geschmack = list_Bewertung[index]->getGeschmack();
    checkBox_geschmack_0->setChecked(false);
    checkBox_geschmack_1->setChecked(false);
    checkBox_geschmack_2->setChecked(false);
    checkBox_geschmack_3->setChecked(false);
    checkBox_geschmack_4->setChecked(false);
    checkBox_geschmack_5->setChecked(false);
    checkBox_geschmack_6->setChecked(false);
    checkBox_geschmack_7->setChecked(false);
    checkBox_geschmack_8->setChecked(false);
    checkBox_geschmack_9->setChecked(false);
    checkBox_geschmack_10->setChecked(false);
    checkBox_geschmack_11->setChecked(false);
    checkBox_geschmack_12->setChecked(false);

    if (geschmack&1){
      checkBox_geschmack_0->setChecked(true);
    }
    if (geschmack&2){
      checkBox_geschmack_1->setChecked(true);
    }
    if (geschmack&4){
      checkBox_geschmack_2->setChecked(true);
    }
    if (geschmack&8){
      checkBox_geschmack_3->setChecked(true);
    }
    if (geschmack&16){
      checkBox_geschmack_4->setChecked(true);
    }
    if (geschmack&32){
      checkBox_geschmack_5->setChecked(true);
    }
    if (geschmack&64){
      checkBox_geschmack_6->setChecked(true);
    }
    if (geschmack&128){
      checkBox_geschmack_7->setChecked(true);
    }
    if (geschmack&256){
      checkBox_geschmack_8->setChecked(true);
    }
    if (geschmack&512){
      checkBox_geschmack_9->setChecked(true);
    }
    if (geschmack&1024){
      checkBox_geschmack_10->setChecked(true);
    }
    if (geschmack&2048){
      checkBox_geschmack_11->setChecked(true);
    }
    if (geschmack&4096){
      checkBox_geschmack_12->setChecked(true);
    }

    //radio Buttons antrunk
    int antrunk = list_Bewertung[index]->getAntrunk();
    radioButton_antrunk_0->setAutoExclusive(false);
    radioButton_antrunk_0->setChecked(false);
    radioButton_antrunk_0->setAutoExclusive(true);

    radioButton_antrunk_1->setAutoExclusive(false);
    radioButton_antrunk_1->setChecked(false);
    radioButton_antrunk_1->setAutoExclusive(true);

    radioButton_antrunk_2->setAutoExclusive(false);
    radioButton_antrunk_2->setChecked(false);
    radioButton_antrunk_2->setAutoExclusive(true);

    radioButton_antrunk_3->setAutoExclusive(false);
    radioButton_antrunk_3->setChecked(false);
    radioButton_antrunk_3->setAutoExclusive(true);

    radioButton_antrunk_4->setAutoExclusive(false);
    radioButton_antrunk_4->setChecked(false);
    radioButton_antrunk_4->setAutoExclusive(true);

    radioButton_antrunk_5->setAutoExclusive(false);
    radioButton_antrunk_5->setChecked(false);
    radioButton_antrunk_5->setAutoExclusive(true);

    radioButton_antrunk_6->setAutoExclusive(false);
    radioButton_antrunk_6->setChecked(false);
    radioButton_antrunk_6->setAutoExclusive(true);

    radioButton_antrunk_7->setAutoExclusive(false);
    radioButton_antrunk_7->setChecked(false);
    radioButton_antrunk_7->setAutoExclusive(true);

    if (antrunk&1){
      radioButton_antrunk_0->setChecked(true);
    }
    if (antrunk&2){
      radioButton_antrunk_1->setChecked(true);
    }
    if (antrunk&4){
      radioButton_antrunk_2->setChecked(true);
    }
    if (antrunk&8){
      radioButton_antrunk_3->setChecked(true);
    }
    if (antrunk&16){
      radioButton_antrunk_4->setChecked(true);
    }
    if (antrunk&32){
      radioButton_antrunk_5->setChecked(true);
    }
    if (antrunk&64){
      radioButton_antrunk_6->setChecked(true);
    }
    if (antrunk&128){
      radioButton_antrunk_7->setChecked(true);
    }

    //radio Buttons haupttrunk
    int haupttrunk = list_Bewertung[index]->getHaupttrunk();
    radioButton_haupttrunk_0->setAutoExclusive(false);
    radioButton_haupttrunk_0->setChecked(false);
    radioButton_haupttrunk_0->setAutoExclusive(true);

    radioButton_haupttrunk_1->setAutoExclusive(false);
    radioButton_haupttrunk_1->setChecked(false);
    radioButton_haupttrunk_1->setAutoExclusive(true);

    radioButton_haupttrunk_2->setAutoExclusive(false);
    radioButton_haupttrunk_2->setChecked(false);
    radioButton_haupttrunk_2->setAutoExclusive(true);

    radioButton_haupttrunk_3->setAutoExclusive(false);
    radioButton_haupttrunk_3->setChecked(false);
    radioButton_haupttrunk_3->setAutoExclusive(true);

    radioButton_haupttrunk_4->setAutoExclusive(false);
    radioButton_haupttrunk_4->setChecked(false);
    radioButton_haupttrunk_4->setAutoExclusive(true);


    if (haupttrunk&1){
      radioButton_haupttrunk_0->setChecked(true);
    }
    if (haupttrunk&2){
      radioButton_haupttrunk_1->setChecked(true);
    }
    if (haupttrunk&4){
      radioButton_haupttrunk_2->setChecked(true);
    }
    if (haupttrunk&8){
      radioButton_haupttrunk_3->setChecked(true);
    }
    if (haupttrunk&16){
      radioButton_haupttrunk_4->setChecked(true);
    }

    //radio Buttons nachtrunk
    int nachtrunk = list_Bewertung[index]->getNachtrunk();
    radioButton_nachtrunk_0->setAutoExclusive(false);
    radioButton_nachtrunk_0->setChecked(false);
    radioButton_nachtrunk_0->setAutoExclusive(true);

    radioButton_nachtrunk_1->setAutoExclusive(false);
    radioButton_nachtrunk_1->setChecked(false);
    radioButton_nachtrunk_1->setAutoExclusive(true);

    radioButton_nachtrunk_2->setAutoExclusive(false);
    radioButton_nachtrunk_2->setChecked(false);
    radioButton_nachtrunk_2->setAutoExclusive(true);

    radioButton_nachtrunk_3->setAutoExclusive(false);
    radioButton_nachtrunk_3->setChecked(false);
    radioButton_nachtrunk_3->setAutoExclusive(true);

    radioButton_nachtrunk_4->setAutoExclusive(false);
    radioButton_nachtrunk_4->setChecked(false);
    radioButton_nachtrunk_4->setAutoExclusive(true);

    radioButton_nachtrunk_5->setAutoExclusive(false);
    radioButton_nachtrunk_5->setChecked(false);
    radioButton_nachtrunk_5->setAutoExclusive(true);

    radioButton_nachtrunk_6->setAutoExclusive(false);
    radioButton_nachtrunk_6->setChecked(false);
    radioButton_nachtrunk_6->setAutoExclusive(true);

    radioButton_nachtrunk_7->setAutoExclusive(false);
    radioButton_nachtrunk_7->setChecked(false);
    radioButton_nachtrunk_7->setAutoExclusive(true);

    radioButton_nachtrunk_8->setAutoExclusive(false);
    radioButton_nachtrunk_8->setChecked(false);
    radioButton_nachtrunk_8->setAutoExclusive(true);


    if (nachtrunk&1){
      radioButton_nachtrunk_0->setChecked(true);
    }
    if (nachtrunk&2){
      radioButton_nachtrunk_1->setChecked(true);
    }
    if (nachtrunk&4){
      radioButton_nachtrunk_2->setChecked(true);
    }
    if (nachtrunk&8){
      radioButton_nachtrunk_3->setChecked(true);
    }
    if (nachtrunk&16){
      radioButton_nachtrunk_4->setChecked(true);
    }
    if (nachtrunk&32){
      radioButton_nachtrunk_5->setChecked(true);
    }
    if (nachtrunk&64){
      radioButton_nachtrunk_6->setChecked(true);
    }
    if (nachtrunk&128){
      radioButton_nachtrunk_7->setChecked(true);
    }
    if (nachtrunk&256){
      radioButton_nachtrunk_8->setChecked(true);
    }

    //radio Buttons Gesamteindruck
    int gesamteindruck = list_Bewertung[index]->getGesamteindruck();
    radioButton_gesamteindruck_0->setAutoExclusive(false);
    radioButton_gesamteindruck_0->setChecked(false);
    radioButton_gesamteindruck_0->setAutoExclusive(true);

    radioButton_gesamteindruck_1->setAutoExclusive(false);
    radioButton_gesamteindruck_1->setChecked(false);
    radioButton_gesamteindruck_1->setAutoExclusive(true);

    radioButton_gesamteindruck_2->setAutoExclusive(false);
    radioButton_gesamteindruck_2->setChecked(false);
    radioButton_gesamteindruck_2->setAutoExclusive(true);

    radioButton_gesamteindruck_3->setAutoExclusive(false);
    radioButton_gesamteindruck_3->setChecked(false);
    radioButton_gesamteindruck_3->setAutoExclusive(true);

    radioButton_gesamteindruck_4->setAutoExclusive(false);
    radioButton_gesamteindruck_4->setChecked(false);
    radioButton_gesamteindruck_4->setAutoExclusive(true);

    radioButton_gesamteindruck_5->setAutoExclusive(false);
    radioButton_gesamteindruck_5->setChecked(false);
    radioButton_gesamteindruck_5->setAutoExclusive(true);

    radioButton_gesamteindruck_6->setAutoExclusive(false);
    radioButton_gesamteindruck_6->setChecked(false);
    radioButton_gesamteindruck_6->setAutoExclusive(true);

    radioButton_gesamteindruck_7->setAutoExclusive(false);
    radioButton_gesamteindruck_7->setChecked(false);
    radioButton_gesamteindruck_7->setAutoExclusive(true);


    if (gesamteindruck&1){
      radioButton_gesamteindruck_0->setChecked(true);
    }
    if (gesamteindruck&2){
      radioButton_gesamteindruck_1->setChecked(true);
    }
    if (gesamteindruck&4){
      radioButton_gesamteindruck_2->setChecked(true);
    }
    if (gesamteindruck&8){
      radioButton_gesamteindruck_3->setChecked(true);
    }
    if (gesamteindruck&16){
      radioButton_gesamteindruck_4->setChecked(true);
    }
    if (gesamteindruck&32){
      radioButton_gesamteindruck_5->setChecked(true);
    }
    if (gesamteindruck&64){
      radioButton_gesamteindruck_6->setChecked(true);
    }
    if (gesamteindruck&128){
      radioButton_gesamteindruck_7->setChecked(true);
    }

    textEdit_farbe->setText(list_Bewertung[index]->getFarbeBemerkung());
    textEdit_schaum->setText(list_Bewertung[index]->getSchaumBemerkung());
    textEdit_geruch->setText(list_Bewertung[index]->getGeruchBemerkung());
    textEdit_geschmack->setText(list_Bewertung[index]->getGeschmackBemerkung());
    textEdit_antrunk->setText(list_Bewertung[index]->getAntrunkBemerkung());
    textEdit_haupttrunk->setText(list_Bewertung[index]->getHaupttrunkBemerkung());
    textEdit_nachtrunk->setText(list_Bewertung[index]->getNachtrunkBemerkung());
    textEdit_gesamteindruck->setText(list_Bewertung[index]->getGesamteindruckBemerkung());

    //Bewertungseingaben enablen
    widget_bewertung->setEnabled(true);
  }

}

void MainWindowImpl::slot_GraphicsView_AnzahlSterneChanged(int value)
{
  int bi = getBewertungsIndex();
  if (bi > -1)
    list_Bewertung[getBewertungsIndex()]->setSterne(value);
}

void MainWindowImpl::on_dateEdit_Bewertungsdatum_dateChanged(const QDate &date)
{
  int index = getBewertungsIndex();
  list_Bewertung[index]->setBewertungsdatum(date);
  label_bewWoche->setText(trUtf8("Woche ") + QString::number(list_Bewertung[index]->getWoche()));
  setAenderung(true);

}

void MainWindowImpl::on_textEdit_bewBemerkung_textChanged()
{
  list_Bewertung[getBewertungsIndex()]->setBemerkung(textEdit_bewBemerkung->document()->toPlainText());
  setAenderung(true);

}

void MainWindowImpl::on_radioButton_farbe_0_clicked()
{
  setBewertungFarbe();
}

void MainWindowImpl::on_radioButton_farbe_1_clicked()
{
  setBewertungFarbe();
}

void MainWindowImpl::on_radioButton_farbe_2_clicked()
{
  setBewertungFarbe();
}

void MainWindowImpl::on_radioButton_farbe_3_clicked()
{
  setBewertungFarbe();
}

void MainWindowImpl::on_radioButton_farbe_4_clicked()
{
  setBewertungFarbe();
}

void MainWindowImpl::on_radioButton_farbe_5_clicked()
{
  setBewertungFarbe();
}

void MainWindowImpl::on_radioButton_farbe_6_clicked()
{
  setBewertungFarbe();
}

void MainWindowImpl::on_radioButton_farbe_7_clicked()
{
  setBewertungFarbe();
}

void MainWindowImpl::on_radioButton_farbe_8_clicked()
{
  setBewertungFarbe();
}

void MainWindowImpl::on_radioButton_farbe_9_clicked()
{
  setBewertungFarbe();
}

void MainWindowImpl::on_radioButton_farbe_10_clicked()
{
  setBewertungFarbe();
}

void MainWindowImpl::on_radioButton_schaum_0_clicked()
{
  setBewertungSchaum();
}

void MainWindowImpl::on_radioButton_schaum_1_clicked()
{
  setBewertungSchaum();
}

void MainWindowImpl::on_radioButton_schaum_2_clicked()
{
  setBewertungSchaum();
}

void MainWindowImpl::on_radioButton_schaum_3_clicked()
{
  setBewertungSchaum();
}

void MainWindowImpl::on_radioButton_schaum_4_clicked()
{
  setBewertungSchaum();
}

void MainWindowImpl::on_radioButton_schaum_5_clicked()
{
  setBewertungSchaum();
}

void MainWindowImpl::on_radioButton_schaum_6_clicked()
{
  setBewertungSchaum();
}

void MainWindowImpl::on_radioButton_schaum_7_clicked()
{
  setBewertungSchaum();
}

void MainWindowImpl::on_radioButton_schaum_8_clicked()
{
  setBewertungSchaum();
}

void MainWindowImpl::on_radioButton_schaum_9_clicked()
{
  setBewertungSchaum();
}

void MainWindowImpl::on_radioButton_schaum_10_clicked()
{
  setBewertungSchaum();
}

void MainWindowImpl::on_checkBox_geruch_0_clicked()
{
  setBewertungGeruch();
}

void MainWindowImpl::on_checkBox_geruch_1_clicked()
{
  setBewertungGeruch();
}

void MainWindowImpl::on_checkBox_geruch_2_clicked()
{
  setBewertungGeruch();
}

void MainWindowImpl::on_checkBox_geruch_3_clicked()
{
  setBewertungGeruch();
}

void MainWindowImpl::on_checkBox_geruch_4_clicked()
{
  setBewertungGeruch();
}

void MainWindowImpl::on_checkBox_geruch_5_clicked()
{
  setBewertungGeruch();
}

void MainWindowImpl::on_checkBox_geruch_6_clicked()
{
  setBewertungGeruch();
}

void MainWindowImpl::on_checkBox_geruch_7_clicked()
{
  setBewertungGeruch();
}

void MainWindowImpl::on_checkBox_geruch_8_clicked()
{
  setBewertungGeruch();
}

void MainWindowImpl::on_checkBox_geruch_9_clicked()
{
  setBewertungGeruch();
}

void MainWindowImpl::on_checkBox_geruch_10_clicked()
{
  setBewertungGeruch();
}

void MainWindowImpl::on_checkBox_geruch_11_clicked()
{
  setBewertungGeruch();
}

void MainWindowImpl::on_checkBox_geschmack_0_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_checkBox_geschmack_1_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_checkBox_geschmack_2_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_checkBox_geschmack_3_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_checkBox_geschmack_4_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_checkBox_geschmack_5_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_checkBox_geschmack_6_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_checkBox_geschmack_7_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_checkBox_geschmack_8_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_checkBox_geschmack_9_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_checkBox_geschmack_10_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_checkBox_geschmack_11_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_checkBox_geschmack_12_clicked()
{
  setBewertungGeschmack();
}

void MainWindowImpl::on_radioButton_antrunk_0_clicked()
{
  setBewertungAntrunk();
}

void MainWindowImpl::on_radioButton_antrunk_1_clicked()
{
  setBewertungAntrunk();
}

void MainWindowImpl::on_radioButton_antrunk_2_clicked()
{
  setBewertungAntrunk();
}

void MainWindowImpl::on_radioButton_antrunk_3_clicked()
{
  setBewertungAntrunk();
}

void MainWindowImpl::on_radioButton_antrunk_4_clicked()
{
  setBewertungAntrunk();
}

void MainWindowImpl::on_radioButton_antrunk_5_clicked()
{
  setBewertungAntrunk();
}

void MainWindowImpl::on_radioButton_antrunk_6_clicked()
{
  setBewertungAntrunk();
}

void MainWindowImpl::on_radioButton_antrunk_7_clicked()
{
  setBewertungAntrunk();
}

void MainWindowImpl::on_radioButton_haupttrunk_0_clicked()
{
  setBewertungHaupttrunk();
}

void MainWindowImpl::on_radioButton_haupttrunk_1_clicked()
{
  setBewertungHaupttrunk();
}

void MainWindowImpl::on_radioButton_haupttrunk_2_clicked()
{
  setBewertungHaupttrunk();
}

void MainWindowImpl::on_radioButton_haupttrunk_3_clicked()
{
  setBewertungHaupttrunk();
}

void MainWindowImpl::on_radioButton_haupttrunk_4_clicked()
{
  setBewertungHaupttrunk();
}

void MainWindowImpl::on_radioButton_nachtrunk_0_clicked()
{
  setBewertungNachtrunk();
}

void MainWindowImpl::on_radioButton_nachtrunk_1_clicked()
{
  setBewertungNachtrunk();
}

void MainWindowImpl::on_radioButton_nachtrunk_2_clicked()
{
  setBewertungNachtrunk();
}

void MainWindowImpl::on_radioButton_nachtrunk_3_clicked()
{
  setBewertungNachtrunk();
}

void MainWindowImpl::on_radioButton_nachtrunk_4_clicked()
{
  setBewertungNachtrunk();
}

void MainWindowImpl::on_radioButton_nachtrunk_5_clicked()
{
  setBewertungNachtrunk();
}

void MainWindowImpl::on_radioButton_nachtrunk_6_clicked()
{
  setBewertungNachtrunk();
}

void MainWindowImpl::on_radioButton_nachtrunk_7_clicked()
{
  setBewertungNachtrunk();
}

void MainWindowImpl::on_radioButton_nachtrunk_8_clicked()
{
  setBewertungNachtrunk();
}

void MainWindowImpl::on_radioButton_gesamteindruck_0_clicked()
{
  setBewertungGesamteindruck();
}

void MainWindowImpl::on_radioButton_gesamteindruck_1_clicked()
{
  setBewertungGesamteindruck();
}

void MainWindowImpl::on_radioButton_gesamteindruck_2_clicked()
{
  setBewertungGesamteindruck();
}

void MainWindowImpl::on_radioButton_gesamteindruck_3_clicked()
{
  setBewertungGesamteindruck();
}

void MainWindowImpl::on_radioButton_gesamteindruck_4_clicked()
{
  setBewertungGesamteindruck();
}

void MainWindowImpl::on_radioButton_gesamteindruck_5_clicked()
{
  setBewertungGesamteindruck();
}

void MainWindowImpl::on_radioButton_gesamteindruck_6_clicked()
{
  setBewertungGesamteindruck();
}

void MainWindowImpl::on_radioButton_gesamteindruck_7_clicked()
{
  setBewertungGesamteindruck();
}

void MainWindowImpl::on_textEdit_farbe_textChanged()
{
  list_Bewertung[getBewertungsIndex()]->setFarbeBemerkung(textEdit_farbe->document()->toPlainText());
  setAenderung(true);

}

void MainWindowImpl::on_textEdit_schaum_textChanged()
{
  list_Bewertung[getBewertungsIndex()]->setSchaumBemerkung(textEdit_schaum->document()->toPlainText());
  setAenderung(true);

}

void MainWindowImpl::on_textEdit_geruch_textChanged()
{
  list_Bewertung[getBewertungsIndex()]->setGeruchBemerkung(textEdit_geruch->document()->toPlainText());
  setAenderung(true);

}

void MainWindowImpl::on_textEdit_geschmack_textChanged()
{
  list_Bewertung[getBewertungsIndex()]->setGeschmackBemerkung(textEdit_geschmack->document()->toPlainText());
  setAenderung(true);

}

void MainWindowImpl::on_textEdit_antrunk_textChanged()
{
  list_Bewertung[getBewertungsIndex()]->setAntrunkBemerkung(textEdit_antrunk->document()->toPlainText());
  setAenderung(true);

}

void MainWindowImpl::on_textEdit_haupttrunk_textChanged()
{
  list_Bewertung[getBewertungsIndex()]->setHaupttrunkBemerkung(textEdit_haupttrunk->document()->toPlainText());
  setAenderung(true);

}

void MainWindowImpl::on_textEdit_nachtrunk_textChanged()
{
  list_Bewertung[getBewertungsIndex()]->setNachtrunkBemerkung(textEdit_nachtrunk->document()->toPlainText());
  setAenderung(true);

}

void MainWindowImpl::on_textEdit_gesamteindruck_textChanged()
{
  list_Bewertung[getBewertungsIndex()]->setGesamteindruckBemerkung(textEdit_gesamteindruck->document()->toPlainText());
  setAenderung(true);

}

void MainWindowImpl::on_pushButton_addStar_clicked()
{
  graphicsView_bewStar->addStar();
  setAenderung(true);

}

void MainWindowImpl::on_pushButton_remStar_clicked()
{
  graphicsView_bewStar->remStar();
  setAenderung(true);

}

void MainWindowImpl::on_tableWidget_Malz_cellChanged(int row, int column)
{
  if (Gestartet){
    //Wenn sich der Malz eintrag geändert hat dann Sude nach verwendung durchsuchen und abändern
    QTableWidgetItem *newItem = tableWidget_Malz -> item(row,TableMalzColName);

    if (newItem->text() == ""){
      newItem->setText(trUtf8("Bitte eine Bezeichnung angeben"));
    }

    //Hier eine Überprüfung vornehmen ob der Name doppelt vorkommt
    bool doppelt = true;
    while (doppelt){
      doppelt = false;
      for (int i=0; i < tableWidget_Malz->rowCount(); i++){
        if (i != row){
          if (tableWidget_Malz->item(i,TableMalzColName)->text() == newItem->text()){
            doppelt = true;
            //qDebug() << "Malzeintrag ist doppelt: " << newItem->text();
            newItem->setText(newItem->text()+"_");
            i = tableWidget_Malz->rowCount();
          }
        }
      }
    }
    if ((Malz_Bezeichnung_Merker != "") && (column == 0)) {
      if (Malz_Bezeichnung_Merker != newItem->text()){
        QSqlQuery query;
        //Alle Sude Abfragen die noch nicht gebraut wurden
        QString sql = "SELECT * FROM Sud WHERE BierWurdeGebraut == 0";
        if (!query.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                      + trUtf8("\nSQL Befehl:\n") + sql);
        }
        else {
          //Sud Speichern da der geladene vielleicht geändert wird
          save();
          while (query.next()){
            QSqlQuery query2;
            //Bei allen gefundenen Suden eventuell vorhandene Schüttungseinträge austauschen
            int FeldNr = query.record().indexOf("ID");
            QString str = Malz_Bezeichnung_Merker;
            QString sql2 = "UPDATE Malzschuettung SET Name='"+newItem->text().replace("'","''")+"' WHERE Name='"
                +str.replace("'","''")+"' AND SudID="+query.value(FeldNr).toString();
            if (!query2.exec(sql2)) {
              // Fehlermeldung Datenbankabfrage
              ErrorMessage *errorMessage = new ErrorMessage();
              errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                          CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                          + trUtf8("\nSQL Befehl:\n") + sql2);
            }
          }
          //Sud wieder laden da er evetuell geändert wurde.
          LadeSudDB(false);
          ErstelleSudInfo();
        }
        //Da eventuell Datenbankeinträge der Sude geändert wurden müssen die Rohstoffe jetzt auch gespeichert werden
        //da ansonsten (wenn beim beenden nicht geseichert wird) die Rohstoffeinträge nicht mehr zu den Suden passen
        Malz_Bezeichnung_Merker = newItem->text();
      }
    }
  }
}

void MainWindowImpl::on_tableWidget_Hopfen_cellChanged(int row, int column)
{
  if (Gestartet){
    //Wenn sich der Hopfen eintrag geändert hat dann Sude nach verwendung durchsuchen und abändern
    QTableWidgetItem *newItem = tableWidget_Hopfen -> item(row,TableHopfenColName);

    if (newItem->text() == ""){
      newItem->setText(trUtf8("Bitte eine Bezeichnung angeben"));
    }

    //Hier eine Überprüfung vornehmen ob der Name doppelt vorkommt
    bool doppelt = true;
    while (doppelt){
      doppelt = false;
      for (int i=0; i < tableWidget_Hopfen->rowCount(); i++){
        if (i != row){
          if (tableWidget_Hopfen->item(i,TableHopfenColName)->text() == newItem->text()){
            doppelt = true;
            //qDebug() << "Hopfeneintrag ist doppelt: " << newItem->text();
            newItem->setText(newItem->text()+"_");
            i = tableWidget_Hopfen->rowCount();
          }
        }
      }
    }

    if ((Hopfen_Bezeichnung_Merker != "")  && (column == 0)){
      if (Hopfen_Bezeichnung_Merker != newItem->text()){
        QSqlQuery query;
        //Alle Sude Abfragen die noch nicht gebraut wurden
        QString sql = "SELECT * FROM Sud WHERE BierWurdeGebraut == 0";
        if (!query.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                      + trUtf8("\nSQL Befehl:\n") + sql);
        }
        else {
          //Sud Speichern da der geladene vielleicht geändert wird
          save();
          while (query.next()){
            QSqlQuery query2;
            //Bei allen gefundenen Suden eventuell vorhandene Schüttungseinträge austauschen
            int FeldNr = query.record().indexOf("ID");
            QString str = Hopfen_Bezeichnung_Merker;
            QString sql2 = "UPDATE Hopfengaben SET Name='"+newItem->text().replace("'","''")+"' WHERE Name='"
                +str.replace("'","''")+"' AND SudID="+query.value(FeldNr).toString();
            if (!query2.exec(sql2)) {
              // Fehlermeldung Datenbankabfrage
              ErrorMessage *errorMessage = new ErrorMessage();
              errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                          CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                          + trUtf8("\nSQL Befehl:\n") + sql2);
            }
            sql2 = "UPDATE WeitereZutatenGaben SET Name='"+newItem->text().replace("'","''")+"' WHERE Name='"
                +str.replace("'","''")+"' AND SudID="+query.value(FeldNr).toString();
            if (!query2.exec(sql2)) {
              // Fehlermeldung Datenbankabfrage
              ErrorMessage *errorMessage = new ErrorMessage();
              errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                          CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                          + trUtf8("\nSQL Befehl:\n") + sql2);
            }
          }
          //Sud wieder laden da er evetuell geändert wurde.
          LadeSudDB(false);
          ErstelleSudInfo();
        }
        Hopfen_Bezeichnung_Merker = newItem->text();
      }
    }
  }
}

void MainWindowImpl::on_tableWidget_Hefe_cellChanged(int row, int column)
{
  if (Gestartet){
    //Wenn sich der Hefe eintrag geändert hat dann Sude nach verwendung durchsuchen und abändern
    QTableWidgetItem *newItem = tableWidget_Hefe -> item(row,TableHefeColName);

    if (newItem->text() == ""){
      newItem->setText(trUtf8("Bitte eine Bezeichnung angeben"));
    }

    //Hier eine Überprüfung vornehmen ob der Name doppelt vorkommt
    bool doppelt = true;
    while (doppelt){
      doppelt = false;
      for (int i=0; i < tableWidget_Hefe->rowCount(); i++){
        if (i != row){
          if (tableWidget_Hefe->item(i,TableHefeColName)->text() == newItem->text()){
            doppelt = true;
            //qDebug() << "Hefeeintrag ist doppelt: " << newItem->text();
            newItem->setText(newItem->text()+"_");
            i = tableWidget_Hefe->rowCount();
          }
        }
      }
    }

    if ((Hefe_Bezeichnung_Merker != "") && (column == 0)) {
      if (Hefe_Bezeichnung_Merker != newItem->text()){
        //Sud Speichern da der geladene vielleicht geändert wird
        save();

        QSqlQuery query;
        //Alle Sude Abfragen die noch nicht gebraut wurden
        QString str = Hefe_Bezeichnung_Merker;
        QString sql = "UPDATE Sud SET AuswahlHefe='"+newItem->text().replace("'","''")
            +"' WHERE BierWurdeGebraut == 0 AND AuswahlHefe=='" +str.replace("'","''")+"'";
        if (!query.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                      + trUtf8("\nSQL Befehl:\n") + sql);
        }
        else {
          //Sud wieder laden da er evetuell geändert wurde.
          LadeSudDB(false);
          ErstelleSudInfo();
        }
        Hefe_Bezeichnung_Merker = newItem->text();
      }
    }
  }
}

void MainWindowImpl::on_tableWidget_WeitereZutaten_cellChanged(int row, int column)
{
  if (Gestartet){
    //Wenn sich der Eintrag geändert hat dann Sude nach verwendung durchsuchen und abändern
    QTableWidgetItem *newItem = tableWidget_WeitereZutaten -> item(row,TableWZutatColName);

    if (newItem->text() == ""){
      newItem->setText(trUtf8("Bitte eine Bezeichnung angeben"));
    }

    //Hier eine Überprüfung vornehmen ob der Name doppelt vorkommt
    bool doppelt = true;
    while (doppelt){
      doppelt = false;
      for (int i=0; i < tableWidget_WeitereZutaten->rowCount(); i++){
        if (i != row){
          if (tableWidget_WeitereZutaten->item(i,TableWZutatColName)->text() == newItem->text()){
            doppelt = true;
            //qDebug() << "Eintrag in den Weiteren Zutaten ist doppelt: " << newItem->text();
            newItem->setText(newItem->text()+"_");
            i = tableWidget_WeitereZutaten->rowCount();
          }
        }
      }
    }

    if ((WZutaten_Bezeichnung_Merker != "") && (column == 0)) {
      if (WZutaten_Bezeichnung_Merker != newItem->text()){
        QSqlQuery query;
        //Alle Sude Abfragen die noch nicht gebraut wurden
        QString sql = "SELECT * FROM Sud WHERE BierWurdeGebraut == 0";
        if (!query.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                      + trUtf8("\nSQL Befehl:\n") + sql);
        }
        else {
          //Sud Speichern da der geladene vielleicht geändert wird
          save();
          while (query.next()){
            QSqlQuery query2;
            //Bei allen gefundenen Suden eventuell vorhandene Schüttungseinträge austauschen
            int FeldNr = query.record().indexOf("ID");
            QString str = WZutaten_Bezeichnung_Merker;
            QString sql2 = "UPDATE WeitereZutatenGaben SET Name='"+newItem->text().replace("'","''")+"' WHERE Name='"
                +str.replace("'","''")+"' AND SudID="+query.value(FeldNr).toString();
            if (!query2.exec(sql2)) {
              // Fehlermeldung Datenbankabfrage
              ErrorMessage *errorMessage = new ErrorMessage();
              errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                          CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                          + trUtf8("\nSQL Befehl:\n") + sql2);
            }
          }
          //Sud wieder laden da er evetuell geändert wurde.
          LadeSudDB(false);
          ErstelleSudInfo();
        }
        //Da eventuell Datenbankeinträge der Sude geändert wurden müssen die Rohstoffe jetzt auch gespeichert werden
        //da ansonsten (wenn beim beenden nicht gespeichert wird) die Rohstoffeinträge nicht mehr zu den Suden passen
        WZutaten_Bezeichnung_Merker = newItem->text();
      }
    }
  }
}

void MainWindowImpl::on_tableWidget_Malz_currentCellChanged(int currentRow, int currentColumn, int , int )
{
  //wenn eine Zelle von der Beschreibung angewählt wurde Text merken für späteren vergleich ob sich Beschreibungstext geändert hat
  //und Sude angepasst werden müssen
  if (currentColumn == 0) {
    QTableWidgetItem *newItem = tableWidget_Malz -> item(currentRow,currentColumn);
    if (newItem)
        Malz_Bezeichnung_Merker = newItem->text();
  }
}

void MainWindowImpl::on_tableWidget_Hopfen_currentCellChanged(int currentRow, int currentColumn, int , int )
{
  //wenn eine Zelle von der Beschreibung angewählt wurde Text merken für späteren vergleich ob sich Beschreibungstext geändert hat
  //und Sude angepasst werden müssen
  if (currentColumn == 0) {
    QTableWidgetItem *newItem = tableWidget_Hopfen -> item(currentRow,currentColumn);
    if (newItem)
        Hopfen_Bezeichnung_Merker = newItem->text();
  }
}


void MainWindowImpl::on_tableWidget_Hefe_currentCellChanged(int currentRow, int currentColumn, int , int )
{
  //wenn eine Zelle von der Beschreibung angewählt wurde Text merken für späteren vergleich ob sich Beschreibungstext geändert hat
  //und Sude angepasst werden müssen
  if (currentColumn == 0) {
    QTableWidgetItem *newItem = tableWidget_Hefe -> item(currentRow,currentColumn);
    if (newItem)
        Hefe_Bezeichnung_Merker = newItem->text();
  }
}

void MainWindowImpl::on_tableWidget_WeitereZutaten_currentCellChanged(int currentRow, int currentColumn, int , int )
{
  //wenn eine Zelle von der Beschreibung angewählt wurde Text merken für späteren vergleich ob sich Beschreibungstext geändert hat
  //und Sude angepasst werden müssen
  if (currentColumn == 0) {
    QTableWidgetItem *newItem = tableWidget_WeitereZutaten -> item(currentRow,currentColumn);
    if (newItem)
        WZutaten_Bezeichnung_Merker = newItem->text();
  }
}

void MainWindowImpl::on_tableWidget_Malz_itemSelectionChanged()
{
    if (tableWidget_Malz->selectedItems().count() > 0)
    {
        pushButton_MalzKopie->setEnabled(true);
        pushButton_MalzDel->setEnabled(true);
    }
    else
    {
        pushButton_MalzKopie->setEnabled(false);
        pushButton_MalzDel->setEnabled(false);
    }
}

void MainWindowImpl::on_tableWidget_Hopfen_itemSelectionChanged()
{
    if (tableWidget_Hopfen->selectedItems().count() > 0)
    {
        pushButton_HopfenKopie->setEnabled(true);
        pushButton_HopfenDel->setEnabled(true);
    }
    else
    {
        pushButton_HopfenKopie->setEnabled(false);
        pushButton_HopfenDel->setEnabled(false);
    }
}

void MainWindowImpl::on_tableWidget_Hefe_itemSelectionChanged()
{
    if (tableWidget_Hefe->selectedItems().count() > 0)
    {
        pushButton_HefeKopie->setEnabled(true);
        pushButton_HefeDel->setEnabled(true);
    }
    else
    {
        pushButton_HefeKopie->setEnabled(false);
        pushButton_HefeDel->setEnabled(false);
    }
}

void MainWindowImpl::on_tableWidget_WeitereZutaten_itemSelectionChanged()
{
    if (tableWidget_WeitereZutaten->selectedItems().count() > 0)
    {
        pushButton_WeitereZutatenDel->setEnabled(true);
        pushButton_WeitereZutatenKopie->setEnabled(true);
    }
    else
    {
        pushButton_WeitereZutatenDel->setEnabled(false);
        pushButton_WeitereZutatenKopie->setEnabled(false);
    }
}

void MainWindowImpl::on_spinBox_Menge_valueChanged(double arg1)
{
    if (Gestartet)
    {
        spinBox_WuerzemengeVorHopfenseihen->setValue(arg1);
        spinBox_WuerzemengeKochende->setValue(arg1);
    }
}

void MainWindowImpl::on_comboBox_BerechnungsArtHopfen_currentIndexChanged(int )
{
  if (Gestartet) {
    BerAlles();
    setAenderung(true);
  }
}

void MainWindowImpl::on_tabWidged_currentChanged(int index)
{
  Q_UNUSED(index)

  QWidget* currentTab = tabWidged->currentWidget();

  //Brauübersicht
  if (currentTab == tab_Brauuebersicht){
    FuelleBrauuebersicht();
  }
  //Ausrüstung
  else if (currentTab == tab_Ausruestung){
    BerEffektiveAusbeuteMittel();
  }
  //Gärverlauf
  else if (currentTab == tab_Gaerverlauf){
    FuelleGaerverlauf();
  }
  //Zusammenfassung / Spickzettel
  else if (currentTab == tab_Spickzettel){
    //Seite Spickzettel erstellen
    ErstelleTabSpickzettel();
  }
  //Brau && Gärdaten
  else if (currentTab == tab_Gaerverlauf){
    //Datum setzten
    if (!BierWurdeGebraut){
      dateEdit_Braudatum -> setDate(QDate::currentDate());
      dateEdit_Anstelldatum -> setDate(QDate::currentDate());
      dateEdit_Abfuelldatum -> setDate(QDate::currentDate());
    }
    else if (!BierWurdeAbgefuellt){
      dateEdit_Abfuelldatum -> setDate(QDate::currentDate());
    }
  }
  //Anleitung
  else if (currentTab == tab_help){
    if (!keinInternet) {
      if (webView_Anleitung->url().isEmpty())
        webView_Anleitung -> setUrl(QUrl(URL_ANLEITUNG));
    }
  }
}

void MainWindowImpl::on_spinBox_High_Gravity_valueChanged(int)
{
  BerAlles();
  setAenderung(true);
}

void MainWindowImpl::slot_SpracheWechselt(QAction *action)
{
  if(0 != action)	{
    //Ladet die ausgewählte sprache
    loadSprache(action->data().toString());
    //schreibe sprachauswahl in config datei
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
    settings.beginGroup("Sprache");
    settings.setValue("sprachauswahl", action->data().toString());
    settings.endGroup();
  }
}

void MainWindowImpl::on_pushButton_EingabeHVerdampfungsziffer_clicked()
{
  DialogBerVerdampfung bver;
  bver.setKochdauer(spinBox_Gesammtkochdauer->value());
  bver.setDurchmesser(spinBox_SudpfanneDurchmesser->value());
  bver.setHoehe(spinBox_SudpfanneHoehe->value());
  bver.setMenge1(doubleSpinBox_VolumenPfannevoll->value());
  bver.setMenge2(Berechnungen.BerVolumenWasser(20,99,spinBox_WuerzemengeKochende->value()));
  if (bver.exec() == QDialog::Accepted) {
    doubleSpinBox_Verdampfung->setValue(bver.getVerdampfungsziffer());
  }
}

void MainWindowImpl::on_pushButton_SudinfoPDF_clicked()
{
    QString defaultFileName;
    if (tableWidget_Sudauswahl -> selectedItems().count() == 4 ||
        tableWidget_Sudauswahl -> selectedItems().count() == 5)
        defaultFileName = tableWidget_Sudauswahl->item(tableWidget_Sudauswahl->currentRow(), 1)->text() + "_Info.pdf";
    else
        defaultFileName =  trUtf8("Rohstoffe") + ".pdf";

    // letzten Pfad einlesen
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
    settings.beginGroup("PDF");
    QString p = settings.value("recentPDFPath", QDir::homePath()).toString();

    QString fileName = QFileDialog::getSaveFileName(this, trUtf8("PDF speichern unter"), p + "/" + defaultFileName, "PDF (*.pdf)");
    if (!fileName.isEmpty())
    {
        bool merker = StyleDunkel;
        if (merker)
        {
            StyleDunkel = false;
            ErstelleSudInfo();
        }

        // pdf speichern
        webView_Info->printToPdf(fileName);

        if (merker)
        {
            StyleDunkel = merker;
            ErstelleSudInfo();
        }

        // Pfad abspeichern
        QFileInfo fi(fileName);
        settings.setValue("recentPDFPath", fi.absolutePath());

        // open PDF
        qDebug() << "file:///" + fileName;
        QDesktopServices::openUrl(QUrl("file:///" + fileName));
    }

    settings.endGroup();
}

void MainWindowImpl::on_pushButton_NeueBrauanlage_clicked()
{
  Brauanlage *item;
  item = new Brauanlage();
  //Editierbar setzten
  Qt::ItemFlags flags;
  flags = item->flags();
  flags |= Qt::ItemIsSelectable | Qt::ItemIsEditable;
  item->setFlags(flags);
  item->setText(trUtf8("Neue Brauanlage"));
  item->setID((int)time(NULL)+rand());
  item->setKosten(0);
  item->setKorrekturWasser(0);
  item->setVerdampfungsziffer(10);
  listWidget_Brauanlagen->addItem(item);
  AenderungAusruestung = true;
  AenderungRohstofftabelle = true;
}

void MainWindowImpl::on_pushButton_loescheBrauanlage_clicked()
{
  if (listWidget_Brauanlagen->count() > 1) {
    QListWidgetItem* item = listWidget_Brauanlagen->selectedItems().first();
    delete item;
    AenderungAusruestung = true;
    AenderungRohstofftabelle = true;
  }
}

void MainWindowImpl::on_listWidget_Brauanlagen_itemSelectionChanged()
{
  if ( (listWidget_Brauanlagen->selectedItems().count() == 1) && (listWidget_Brauanlagen->count() > 1)) {
    pushButton_loescheBrauanlage->setEnabled(true);
  }
  else {
    pushButton_loescheBrauanlage->setEnabled(false);
  }
  if (listWidget_Brauanlagen->selectedItems().count() == 1) {
    bool merker = Gestartet;
    Gestartet = false;
    Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
    //Eingabefelder Füllen
    //Ausbeute zur Berechnung der Schüttung
    spinBox_AngenommeneAusbeute -> setValue(item->getSudhausausbeute());
    //Verdampfungsziffer
    doubleSpinBox_Verdampfung -> setValue(item->getVerdampfungsziffer());
    //Kosten
    dspinBox_KostenAusruestung->setValue(item->getKosten());
    //Korrektur der Nachgussmenge
    dSpinBox_KorrekturNachguss -> setValue(item->getKorrekturWasser());
    //Korrektur der Nachgussmenge
    spinBox_KorrekturFarbe -> setValue(item->getKorrekturFarbe());
    //Maischebottich Höhe
    spinBox_MaischebottichHoehe -> setValue(item->getMaischebottich_Hoehe());
    //Maischebottich Durchmesser
    spinBox_MaischebottichDurchmesser -> setValue(item->getMaischebottich_Durchmesser());
    //Maischebottich Maximal nutzbare Füllhöhe
    spinBox_MaischebottichMaxFuellhoehe -> setValue(item->getMaischebottich_MaxFuellhoehe());
    //Sudpfanne Höhe
    spinBox_SudpfanneHoehe -> setValue(item->getSudpfanne_Hoehe());
    //Sudpfanne Durchmesser
    spinBox_SudpfanneDurchmesser -> setValue(item->getSudpfanne_Durchmesser());
    //Sudpfanne Maximal nutzbare Füllhöhe
    spinBox_SudpfanneMaxFuellhoehe -> setValue(item->getSudpfanne_MaxFuellhoehe());
    BerEffektiveAusbeuteMittel();
    Gestartet = merker;
    BerAusruestung();
    LeseGeraetelisteDB(item->getID());
  }
}

void MainWindowImpl::on_listWidget_Brauanlagen_itemChanged(QListWidgetItem *)
{
  if (Gestartet) {
    AenderungAusruestung = true;
    AenderungRohstofftabelle = true;
    setAenderung(true);
  }
}

void MainWindowImpl::on_spinBox_AngenommeneAusbeute_valueChanged(int arg1)
{
  if (Gestartet) {
    setAenderung(true);
    AenderungAusruestung = true;
    setAngenommeneSudhausausbeute(arg1);
  }
}

void MainWindowImpl::on_dSpinBox_KorrekturNachguss_valueChanged(double arg1)
{
  if (Gestartet) {
    setAenderung(true);
    AenderungAusruestung = true;
    setKorrekturWassermenge(arg1);
  }
}

void MainWindowImpl::on_doubleSpinBox_Verdampfung_valueChanged(double arg1)
{
  if (Gestartet) {
    setAenderung(true);
    AenderungAusruestung = true;
    setVerdampfungsziffer(arg1);
  }
}

void MainWindowImpl::on_dspinBox_KostenAusruestung_valueChanged(double arg1)
{
  if (Gestartet) {
    setAenderung(true);
    AenderungAusruestung = true;
    setBrauanlageKosten(arg1);
  }
}

void MainWindowImpl::on_spinBox_MaischebottichHoehe_valueChanged(double arg1)
{
  if (Gestartet) {
    setAenderung(true);
    AenderungAusruestung = true;
    setMaischebottichHoehe(arg1);
  }
}

void MainWindowImpl::on_spinBox_MaischebottichDurchmesser_valueChanged(double arg1)
{
  if (Gestartet) {
    setAenderung(true);
    AenderungAusruestung = true;
    setMaischebottichDurchmesser(arg1);
  }
}

void MainWindowImpl::on_spinBox_MaischebottichMaxFuellhoehe_valueChanged(double arg1)
{
  if (Gestartet) {
    setAenderung(true);
    AenderungAusruestung = true;
    setMaischebottichMaxFuellhoehe(arg1);
  }
}

void MainWindowImpl::on_spinBox_SudpfanneHoehe_valueChanged(double arg1)
{
  if (Gestartet) {
    setAenderung(true);
    AenderungAusruestung = true;
    setSudpfanneHoehe(arg1);
  }
}

void MainWindowImpl::on_spinBox_SudpfanneDurchmesser_valueChanged(double arg1)
{
  if (Gestartet) {
    setAenderung(true);
    AenderungAusruestung = true;
    setSudpfanneDurchmesser(arg1);
  }
}

void MainWindowImpl::on_spinBox_SudpfanneMaxFuellhoehe_valueChanged(double arg1)
{
  if (Gestartet) {
    setAenderung(true);
    AenderungAusruestung = true;
    setSudpfanneMaxFuellhoehe(arg1);
  }
}

void MainWindowImpl::on_spinBox_KorrekturFarbe_valueChanged(int arg1)
{
  if (Gestartet) {
    setAenderung(true);
    AenderungAusruestung = true;
    setKorrekturFarbe(arg1);
    BerAlles();
  }
}


void MainWindowImpl::on_hSlider_dAusbeuteSude_valueChanged(int )
{
  //Berechnette Effektive Ausbeuet abfragen
  QSqlQuery query;
  QString sql;
  double erg = 0;
  int i=0;
  int AnlagenID = 0;
  int Limit = hSlider_dAusbeuteSude->value();
  Brauanlage* item = dynamic_cast<Brauanlage*>(listWidget_Brauanlagen->selectedItems().first());
  if (item != 0) {
    AnlagenID = item->getID();
  }
  sql = "SELECT erg_EffektiveAusbeute FROM Sud WHERE BierWurdeGebraut=1 AND AusbeuteIgnorieren=0 AND AuswahlBrauanlage="+QString::number(AnlagenID)+" ORDER BY Braudatum DESC LIMIT "+QString::number(Limit);
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    while (query.next()){
      i++;
      if (query.value(0) != 0){
        erg += query.value(0).toDouble();
      }
    }
    if (i > 0)
      erg = erg / i;
    else
      erg = 0;
    spinBox_AusbeuteEffektiv_Mittel -> setValue(erg);
  }
}

//setzt den Text der Merklisten Buttons mit Counter für anzahl in der Merkliste
void MainWindowImpl::setButtonsTextMerken()
{
  QSqlQuery query;
  QString sql;
  sql = "SELECT * FROM 'Sud' WHERE MerklistenID=1";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int anzahl = 0;
    while (query.next()) {
      anzahl++;
    }
    QString text = trUtf8("Alle vergessen");
    text += " (" + QString::number(anzahl) + ")";
    pushButton_alleVergessen->setText(text);
  }
}

void MainWindowImpl::on_pushButton_merken_clicked()
{
  int row = tableWidget_Sudauswahl -> currentRow();
  if (row >= 0){
    int SudID = tableWidget_Sudauswahl -> item(row,0) -> text().toInt();
    QSqlQuery query;
    QString sql;
    sql = "UPDATE 'Sud' SET 'MerklistenID'=1 WHERE ID="+QString::number(SudID);
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    setButtonsTextMerken();
    FuelleSudauswahl();
  }
}

void MainWindowImpl::on_pushButton_vergessen_clicked()
{
  int row = tableWidget_Sudauswahl -> currentRow();
  if (row >= 0){
    int SudID = tableWidget_Sudauswahl -> item(row,0) -> text().toInt();
    QSqlQuery query;
    QString sql;
    sql = "UPDATE 'Sud' SET 'MerklistenID'=0 WHERE ID="+QString::number(SudID);
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    setButtonsTextMerken();
    FuelleSudauswahl();
  }
}

void MainWindowImpl::on_pushButton_alleVergessen_clicked()
{
  QSqlQuery query;
  QString sql;
  sql = "UPDATE 'Sud' SET 'MerklistenID'=0 WHERE MerklistenID=1";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  setButtonsTextMerken();
  FuelleSudauswahl();
}

void MainWindowImpl::on_checkBox_MerklisteMengen_clicked()
{
  BerAlles();
}

void MainWindowImpl::on_spinBox_NachisomerisierungsZeit_valueChanged(int arg1)
{
  //Maximalen wert für Hopfenzeiten Setzten
  for (int i=0; i < list_Hopfengaben.count(); i++){
    list_Hopfengaben[i]->setMinKochzeit(arg1*-1);
  }
}

void MainWindowImpl::on_pushButton_MalzNeu_clicked()
{
    tableWidget_Malz->setSortingEnabled(false);
    QString s = Malz_Bezeichnung_Merker;
    Malz_Bezeichnung_Merker = "";
    MalzNeueZeile(trUtf8("Neuer Eintrag"), 0.0, 100,
           0.0, 0.0, "", "",
           QDate::currentDate(), QDate::currentDate().addMonths(1), "");
    setAenderung(true);
    AenderungRohstofftabelle = true;
    Malz_Bezeichnung_Merker = s;
    tableWidget_Malz->setSortingEnabled(true);
}

void MainWindowImpl::on_pushButton_HopfenNeu_clicked()
{
    tableWidget_Hopfen->setSortingEnabled(false);
    QString s = Hopfen_Bezeichnung_Merker;
    Hopfen_Bezeichnung_Merker = "";
    HopfenNeueZeile(trUtf8("Neuer Eintrag"), 0.0, 0.0,
                  0.0, true, "", 0, "",
                  QDate::currentDate(), QDate::currentDate().addMonths(1), "");
    setAenderung(true);
    AenderungRohstofftabelle = true;
    Hopfen_Bezeichnung_Merker = s;
    tableWidget_Hopfen->setSortingEnabled(true);
}

void MainWindowImpl::on_pushButton_HefeNeu_clicked()
{
    tableWidget_Hefe->setSortingEnabled(false);
    QString s = Hefe_Bezeichnung_Merker;
    Hefe_Bezeichnung_Merker = "";
    HefeNeueZeile(trUtf8("Neuer Eintrag"), 0.0, 0.0, 0.0,
                         "", "", 0, 0,
                         "", "", 0, "",
                         QDate::currentDate(), QDate::currentDate().addMonths(1), "");
    setAenderung(true);
    AenderungRohstofftabelle = true;
    Hefe_Bezeichnung_Merker = s;
    tableWidget_Hefe->setSortingEnabled(true);
}

void MainWindowImpl::on_pushButton_WeitereZutatenNeu_clicked()
{
    tableWidget_WeitereZutaten->setSortingEnabled(false);
    QString s = WZutaten_Bezeichnung_Merker;
    WZutaten_Bezeichnung_Merker = "";
    WeitereZutatNeueZeile(trUtf8("Neuer Eintrag"), 0.0, 0, 4,
                          0.0, 0.0, 0.0, "",
                          QDate::currentDate(), QDate::currentDate().addMonths(1), "");
    setAenderung(true);
    AenderungRohstofftabelle = true;
    WZutaten_Bezeichnung_Merker = s;
    tableWidget_WeitereZutaten->setSortingEnabled(true);
}


void MainWindowImpl::on_listWidget_Brauanlagen_currentItemChanged(QListWidgetItem *, QListWidgetItem *previous)
{
  if (Gestartet) {
    Brauanlage* item = dynamic_cast<Brauanlage*>(previous);
    //Wenn Einträge geändert wurden speichern
    if (AenderungGeraeteliste) {
      SchreibeGeraetelisteDB(item->getID());
      AenderungGeraeteliste = false;
    }
  }
}

void MainWindowImpl::on_tableWidget_Geraete_itemChanged(QTableWidgetItem *)
{
  if (Gestartet) {
    if (!fuelleGeraeteliste) {
      AenderungGeraeteliste = true;
      setAenderung(true);
    }
  }
}

void MainWindowImpl::on_tableWidget_Malz_cellClicked(int row, int column)
{
  if (column == TableMalzColLink) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
      QDesktopServices::openUrl(QUrl(tableWidget_Malz->item(row, column)->text()));
  }
}

void MainWindowImpl::on_tableWidget_Hopfen_cellClicked(int row, int column)
{
  if (column == TableHopfenColLink) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
      QDesktopServices::openUrl(QUrl(tableWidget_Hopfen->item(row, column)->text()));
  }
}

void MainWindowImpl::on_tableWidget_Hefe_cellClicked(int row, int column)
{
  if (column == TableHefeColLink) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
      QDesktopServices::openUrl(QUrl(tableWidget_Hefe->item(row, column)->text()));
  }
}

void MainWindowImpl::on_tableWidget_WeitereZutaten_cellClicked(int row, int column)
{
  if (column == TableWZutatColLink) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
      QDesktopServices::openUrl(QUrl(tableWidget_WeitereZutaten->item(row, column)->text()));
  }
}

void MainWindowImpl::on_pushButton_CO2_Info_clicked()
{
  DialogInfo::Info(this, trUtf8("CO2 Gehalt"), trUtf8("<b>Typischer CO2 Gehalt in g/Liter:</b></br><table>\
                                                      <tr><td>Lager, Pilsner </td><td>4,00 - 5,50</td></tr>\
                                                      <tr><td>Weizenbier </td><td>6,50 - 9,00</td></tr>\
                                                      <tr><td>Britische Ales </td><td>3,00 - 4,00</td></tr>\
                                                      <tr><td>Porter Stout </td><td>3,40 - 4,50</td></tr>\
                                                      <tr><td>Belgische Ales </td><td>3,80 - 4,80</td></tr>\
                                                      <tr><td>Lambic </td><td>4,80 - 5,50</td></tr>\
                                                      <tr><td>Frucht-Lambic </td><td>6,00 - 9,00</td></tr>\
                                                      </table>"));
}

void MainWindowImpl::on_pushButton_IBU_Info_clicked()
{
  DialogInfo::Info(this, trUtf8("Bittere"), trUtf8("<b>Typische Bitterwerte in IBU:</b></br><table>\
                                                   <tr><td>Weissbier </td><td>10-15</td></tr>\
                                                   <tr><td>Märzen </td><td>18 - 28</td></tr>\
                                                   <tr><td>Export </td><td>23 - 29</td></tr>\
                                                   <tr><td>Kölsch </td><td>20 - 34</td></tr>\
                                                   <tr><td>Stout </td><td>25 - 40</td></tr>\
                                                   <tr><td>Altbier </td><td>28 - 40</td></tr>\
                                                   <tr><td>Pils </td><td>20 - 50</td></tr>\
                                                   <tr><td>IPA </td><td>&gt; 60</td></tr>\
                                                   </table></br>\
                                                   Neutrales Geschmacksempfinden bei IBU = 2*°P Stammwürze"));
}

void MainWindowImpl::on_pushButton_SW_Info_clicked()
{
  DialogInfo::Info(this, trUtf8("Stammwürze"), trUtf8("<b>Typische Stammwürze in °P:</b></br><table>\
                                                      <tr><td>Bockbier </td><td>16–17,9</td></tr>\
                                                      <tr><td>Doppelbock </td><td>&gt; 18</td></tr>\
                                                      <tr><td>Exportbier </td><td>12–13,5</td></tr>\
                                                      <tr><td>Altbier </td><td>11,9</td></tr>\
                                                      <tr><td>Kölsch </td><td>11,3</td></tr>\
                                                      <tr><td>Pilsener </td><td>11,3–12,3</td></tr>\
                                                      <tr><td>Weizenbier </td><td>11–13</td></tr>\
                                                      <tr><td>Helles </td><td>11–13</td></tr>\
                                                      <tr><td>Berliner Weisse </td><td>7–8</td></tr>\
                                                      </table>"));
}

void MainWindowImpl::on_pushButton_High_Gravity_Info_clicked()
{
  DialogInfo::Info(this, trUtf8("High Gravity Faktor"), trUtf8("Mit High Gravity kann die Ausschlagmenge \
                                                               erhöht werden (wenn die Sudpfanne an ihre Grenze kommt) indem stärker \
                                                               eingebraut wird und dann vor der Hefezugabe wieder auf die gewünschte \
                                                               Stammwürze verdünnt wird."));
}

void MainWindowImpl::on_pushButton_NeuerAnhang_clicked()
{
  AddAnhang(NULL);
  setAenderung(true);
}

void MainWindowImpl::AddAnhang(QString pfad)
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  settings.beginGroup("DB");
  QString dbpfad = settings.value("DB_Pfad").toString();
  settings.endGroup();

  //Bildobjekt erstellen
  AnhangWidget* anhang = new AnhangWidget(scrollArea_7);
  anhang -> setAttribute(Qt::WA_DeleteOnClose);
  anhang->setBasisPfad(dbpfad);
  if (pfad == NULL)
      anhang->openDialog();
  else
      anhang->setPfad(pfad);
  anhang->setID((int)time(NULL)+rand());

  if (anhang->getPfad() != "")
  {
    verticalLayout_Anhang -> addWidget(anhang);
    list_Anhang.append(anhang);

    connect(anhang, SIGNAL( sig_vorClose(int) ), this, SLOT( slot_anhangClose(int) ));
    connect(anhang, SIGNAL( sig_Aenderung() ), this, SLOT( slot_anhangAenderung() ));
  }
}

void MainWindowImpl::slot_anhangClose(int id)
{
  for (int i=0; i < list_Anhang.count(); i++){
    if (list_Anhang[i] -> getID() == id){
      list_Anhang.removeAt(i);
      i = list_Anhang.count();
      setAenderung(true);
    }
  }
}

void MainWindowImpl::slot_anhangAenderung()
{
  setAenderung(true);
}

void MainWindowImpl::LeseAnhangDB()
{
  // Anhang Abfragen
  QSqlQuery query_anhang;
  QString sql = "SELECT * FROM Anhang WHERE SudID=" + QString::number(AktuelleSudID) + ";";
  if (!query_anhang.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query_anhang.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    //Erstmal Anhangliste leeren
    while (list_Anhang.count() > 0){
      list_Anhang[0] -> close();
    }
    list_Anhang.clear();
    //Alle Rasten einlesen
    while (query_anhang.next()){
      int FeldNr = query_anhang.record().indexOf("Pfad");
      AddAnhang(query_anhang.value(FeldNr).toString());
    }
  }
}

void MainWindowImpl::SchreibeAnhangDB()
{
  QSqlQuery query;
  QString sql = "DELETE FROM Anhang WHERE SudID =" + QString::number(AktuelleSudID);
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }

  for (int i=0; i < list_Anhang.count(); i++){
    sql = "INSERT INTO Anhang('SudID', 'Pfad') VALUES(" +
        QString::number(AktuelleSudID) + ","
        + "'" + list_Anhang[i] -> getPfad().replace("'","''") + "')";
    if (!query.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
  }
}

void MainWindowImpl::on_comboBox_GaerungEwzAuswahl_currentIndexChanged(const QString &arg1)
{
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i]->getName() == arg1) {
      label_gaerungEwzZugenMenge->setText(QString::number(list_EwZutat[i]->getErg_Menge())+"g");
    }
  }
}

void MainWindowImpl::on_pushButton_GaerungEwzZugeben_clicked()
{
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i]->getName() == comboBox_GaerungEwzAuswahl->currentText()) {
      list_EwZutat[i]->zutatZugeben();
      i = list_EwZutat.count();
    }
  }
}

void MainWindowImpl::on_pushButton_GaerungEwzEntnehmen_clicked()
{
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i]->getName() == comboBox_GaerungEwzAuswahlEntnahme->currentText()) {
      list_EwZutat[i]->zutatEntnehmen();
      i = list_EwZutat.count();
    }
  }
}

void MainWindowImpl::on_checkBox_zumischen_clicked()
{
  if (checkBox_zumischen->isChecked()) {
    spinBox_WuerzemengeAnstellen -> setValue(spinBox_WuerzemengeKochende->value() - spinBox_Speisemenge -> value() + spinBox_WasserVerschneidung->value());
    spinBox_SWAnstellen->setValue(spinBox_SW->value());
  }
  else {
    spinBox_WuerzemengeAnstellen -> setValue(spinBox_WuerzemengeKochende->value() - spinBox_Speisemenge -> value());
    spinBox_SWAnstellen->setValue(spinBox_SWKochende->value());
  }
}

void MainWindowImpl::on_pushButton_CalcEinmaischeTemp_clicked()
{
    DialogEinmaischeTemp* dlg = new DialogEinmaischeTemp(doubleSpinBox_S_Gesammt->value(),
                                                         18.0,
                                                         doubleSpinBox_WHauptguss->value(),
                                                         list_Rasten.count() > 0 ? list_Rasten[0]->getRastTemp() : 57.0,
                                                         this);
    if (dlg->exec() == QDialog::Accepted)
        spinBox_EinmaischenTemp->setValue(dlg->value());
    delete dlg;
}

void MainWindowImpl::on_spinBox_AnzahlHefeEinheiten_valueChanged(int)
{
    setHefeAuswahlListeFarbe();
}

void MainWindowImpl::on_pushButton_SudTeilen_clicked()
{
    if (Aenderung ? AbfrageSpeichern() : true)
    {
        DialogSudTeilen* dlg = new DialogSudTeilen(lineEdit_Sudname->text(), spinBox_WuerzemengeAnstellen->value(), this);
        if (dlg->exec() == QDialog::Accepted)
        {
            int id = Database::SudKopieren(QString::number(AktuelleSudID), dlg->nameTeil2(), true);
            if (id > 0)
            {
                double factor = 0.0;
                int lastId = AktuelleSudID;
                bool wurdeGebraut = BierWurdeGebraut;
                bool wurdeAbgefuellt = BierWurdeAbgefuellt;

                // Teil 2
                AktuelleSudID = id;
                LadeSudDB(false);
                Gestartet = false;
                factor = dlg->prozent();
                spinBox_Menge->setValue(spinBox_Menge->value() * factor);
                spinBox_WuerzemengeVorHopfenseihen->setValue(spinBox_WuerzemengeVorHopfenseihen->value() * factor);
                spinBox_WuerzemengeKochende->setValue(spinBox_WuerzemengeKochende->value() * factor);
                spinBox_Speisemenge->setValue(spinBox_Speisemenge->value() * factor);
                spinBox_WuerzemengeAnstellen->setValue(spinBox_WuerzemengeAnstellen->value() * factor);
                spinBox_JungbiermengeAbfuellen->setValue(spinBox_JungbiermengeAbfuellen->value() * factor);
                spinBox_AnzahlHefeEinheiten->setValue(qRound(spinBox_AnzahlHefeEinheiten->value() * factor));
                Gestartet = true;
                if (wurdeGebraut)
                {
                    BierWurdeGebraut = false;
                    BerAlles();
                    BierWurdeGebraut = true;
                }
                if (wurdeAbgefuellt)
                {
                    BierWurdeAbgefuellt = false;
                    BerAlles();
                    BierWurdeAbgefuellt = true;
                }
                BerAlles();
                save();

                // Teil 1
                AktuelleSudID = lastId;
                LadeSudDB(false);
                Gestartet = false;
                factor = 1.0 - dlg->prozent();
                lineEdit_Sudname->setText(dlg->nameTeil1());
                spinBox_Menge->setValue(spinBox_Menge->value() * factor);
                spinBox_WuerzemengeVorHopfenseihen->setValue(spinBox_WuerzemengeVorHopfenseihen->value() * factor);
                spinBox_WuerzemengeKochende->setValue(spinBox_WuerzemengeKochende->value() * factor);
                spinBox_Speisemenge->setValue(spinBox_Speisemenge->value() * factor);
                spinBox_WuerzemengeAnstellen->setValue(spinBox_WuerzemengeAnstellen->value() * factor);
                spinBox_JungbiermengeAbfuellen->setValue(spinBox_JungbiermengeAbfuellen->value() * factor);
                spinBox_AnzahlHefeEinheiten->setValue(qRound(spinBox_AnzahlHefeEinheiten->value() * factor));
                Gestartet = true;
                if (wurdeGebraut)
                {
                    BierWurdeGebraut = false;
                    BerAlles();
                    BierWurdeGebraut = true;
                }
                if (wurdeAbgefuellt)
                {
                    BierWurdeAbgefuellt = false;
                    BerAlles();
                    BierWurdeAbgefuellt = true;
                }
                BerAlles();
                save();
            }
        }
        delete dlg;
    }
}
