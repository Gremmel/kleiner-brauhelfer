#include "mainwindowimpl.h"
#include <QSettings>
#include <QString>
#include <QTableWidgetItem>
#include <QPrinter>
#include <QPrintDialog>
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
#include <QPrintPreviewDialog>
#include <QDebug>
#include <QStyleFactory>
#include <QUrl>
#include <QFileInfo>
#include <QProcess>
#include <QDesktopServices>

#include <qmath.h>
#include "errormessage.h"
#include "definitionen.h"
#include "infotexts.h"
#include "einstellungsdialogimpl.h"
#include "getrohstoffvorlage.h"
#include "rohstoffaustauschen.h"
#include "dialog_berschuettungimpl.h"
#include "dialog_berechne_ibuimpl.h"
#include "dialogberverdampfung.h"
#include "brauanlage.h"
#include "dialoginfo.h"
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
  NeueMessungWirdAngelegt = false;
  NeuBerechnen = 0;
  Aenderung = false;
  reconnect = false;

  setupUi(this);

  pushButton_loescheBrauanlage->setEnabled(false);

  //Styleeinstellung aus Konfigdatei einlesen
  LeseStyleEinstellung();

  //Maximale Anzahl Sterne einlesen
  LeseMaxAnzahlSterne();

  graphicsView_bewStar->init(StyleDunkel);
  graphicsView_bewStar->setMaxStar(MaxAnzahlSterne);
  label_bew_ID->setVisible(false);

  pushButton_SudVerbraucht -> setDisabled(true);
  pushButton_SudAbgefuellt -> setDisabled(true);

  pushButton_MalzKopie -> setDisabled(true);
  pushButton_MalzDel -> setDisabled(true);

  pushButton_HopfenKopie -> setDisabled(true);
  pushButton_HopfenDel -> setDisabled(true);

  //Windowicon setzten
  appIcon.addFile(":/global/logo.svg",QSize(64,64));
  setWindowIcon(appIcon);

  //String Listen füllen

  //Liste für Einheiten
  EinheitenListe.append(trUtf8("Kg"));
  EinheitenListe.append(trUtf8("g"));
  //EinheitenListe.append(trUtf8("L"));
  //EinheitenListe.append(trUtf8("ml"));
  //EinheitenListe.append(trUtf8("Stück"));

  //Liste für die Zutatentypen
  ZutatenTypListe.append(trUtf8("Honig"));
  ZutatenTypListe.append(trUtf8("Zucker"));
  ZutatenTypListe.append(trUtf8("Gewürz"));
  ZutatenTypListe.append(trUtf8("Frucht"));
  ZutatenTypListe.append(trUtf8("Sonstiges"));

  //Liste für Hopfentypen
  HopfenTypListe.append(trUtf8(""));
  HopfenTypListe.append(trUtf8("Aroma"));
  HopfenTypListe.append(trUtf8("Bitter"));
  HopfenTypListe.append(trUtf8("Universal"));

  //Liste für Hefe Typ OG UG
  HefeTypOGUGListe.append("");
  HefeTypOGUGListe.append(trUtf8("OG"));
  HefeTypOGUGListe.append(trUtf8("UG"));

  //Liste für Hefe Typ Trocken flüssig
  HefeTypTrFlListe.append("");
  HefeTypTrFlListe.append(trUtf8("Trocken"));
  HefeTypTrFlListe.append(trUtf8("Flüssig"));

  //Liste für Sedimentation
  HefeSedListe.append("");
  HefeSedListe.append(trUtf8("hoch"));
  HefeSedListe.append(trUtf8("mittel"));
  HefeSedListe.append(trUtf8("niedrig"));

  label_waOder -> setVisible(false);
  frame_Sauermalz -> setVisible(false);

  //Überprüfen ob ergebnisse in der Datenbank neu berechnet werden müssen
  if (CheckDBNeuBerechnen()){
    DBErgebnisseNeuBerechnen();
  }

  //Sortierreihenfolge der Sudauswahl auf Braudatum setzten
  tableWidget_Sudauswahl -> sortByColumn(2,Qt::DescendingOrder);

  QList<int> sizes = splitter_Schnellgaerverlauf -> sizes();
  sizes.first() = 1;
  sizes.last() = 1;
  splitter_Schnellgaerverlauf -> setSizes(sizes);
  splitter_Hauptgaerverlauf -> setSizes(sizes);
  splitter_Nachgaerverlauf -> setSizes(sizes);

  //Diagrammfarben setzen
  SetDiagrammFarben();

  //In Brau und Gärdaten Ergenbisfelder entsprechend ein Ausblenden
  SetAnsicht();

  radioButton_FilterAlle -> setChecked(true);


  connect(graphicsView_bewStar, SIGNAL( sig_AnzahlStarChanged(int) ), this, SLOT( slot_GraphicsView_AnzahlSterneChanged(int) ));
  //verbinde Button Korrektur Prozent mit Fuktonen

  //verbinde Button Neuer Eintrag Malz aus Vorlage mit Funktion;
  connect(pushButton_MalzNeuVorlage, SIGNAL( clicked() ), this, SLOT( slot_pushButton_MalzNeuVorlage() ));
  //verbinde Button Eintrag Löschen Malz mit Funktion;
  connect(pushButton_MalzDel, SIGNAL( clicked() ), this, SLOT( slot_pushButton_MalzDel() ));
  //verbinde Button Kopie mit Funktion;
  connect(pushButton_MalzKopie, SIGNAL( clicked() ), this, SLOT( slot_pushButton_MalzKopie() ));

  //verbinde Button Eintrag Löschen Hopfen mit Funktion;
  connect(pushButton_HopfenDel, SIGNAL( clicked() ), this, SLOT( slot_pushButton_HopfenDel() ));
  //verbinde Button Kopie mit Funktion;
  connect(pushButton_HopfenKopie, SIGNAL( clicked() ), this, SLOT( slot_pushButton_HopfenKopie() ));

  //verbinde Button Eintrag Löschen Hefe mit Funktion;
  connect(pushButton_HefeDel, SIGNAL( clicked() ), this, SLOT( slot_pushButton_HefeDel() ));
  //verbinde Button Kopie mit Funktion;
  connect(pushButton_HefeKopie, SIGNAL( clicked() ), this, SLOT( slot_pushButton_HefeKopie() ));

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
  connect(tableWidget_Hopfen, SIGNAL( cellChanged(int, int) ), this, SLOT( slot_EwzAenderungRohstoffe()) );
  connect(tableWidget_Hefe, SIGNAL( cellChanged(int, int) ), this, SLOT( slot_TableWidget_cellChanged(int, int) ));
  connect(tableWidget_WeitereZutaten, SIGNAL( cellChanged(int, int) ), this, SLOT( slot_TableWidget_cellChanged(int, int) ));

  //Tab wird gewechselt
  connect(tabWidged, SIGNAL( currentChanged(int) ), this, SLOT( slot_tabWidgetChanged(int) ));

  //SpinBox Stammwürze nach Kochende mit SpinBox Stammwüzre vor dem Hopfenseihen verbinden
  connect(spinBox_SWKochende, SIGNAL( valueChanged(double) ), spinBox_SWVorHopfenseihen, SLOT( setValue(double) ));

  // links extern weiterleiten
  webView_Info->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
  connect(webView_Info, SIGNAL(linkClicked (const QUrl &)), this, SLOT(slot_urlClicked(const QUrl &)));
  webView_Zusammenfassung->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
  connect(webView_Zusammenfassung, SIGNAL(linkClicked (const QUrl &)), this, SLOT(slot_urlClicked(const QUrl &)));

  //SpinBox Stammwürze vor dem Hopfenseigen ausblenden da nicht mehr benötigt
  label_116 -> hide();
  spinBox_SWVorHopfenseihen -> hide();
  label_117 -> hide();
  pushButton_EingabeHSWVorHopfenseihen -> hide();
  horizontalLayout_107 -> setSpacing(0);
  on_tableWidget_WeitereZutaten_itemSelectionChanged();

  LeseKonfig();

  createActions();
  createMenus();

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

  //letzte Suddaten laden
  if (AktuelleSudID != 0)
    LadeSudDB(true);
  //RezeptTab und Brau und Gärdaten disablen
  else {
    //Beispielsud laden
    AktuelleSudID = 1;
    LadeSudDB(true);
  }

  setAenderung(false);
  AenderungAusruestung = false;
  AenderungGeraeteliste = false;
  AenderungHauptgaerverlauf = false;

  BerAlles();
  FuelleGaerverlauf();

  setButtonsTextMerken();
  setFensterTitel();
  Gestartet = true;
  BerAlles();
  //Seite Spickzettel erstellen
  ErstelleTabSpickzettel();

  // Spaltenbreite von "Link"
  tableWidget_Malz->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Fixed);
  tableWidget_Malz->horizontalHeader()->resizeSection(9, 120);
  tableWidget_Hopfen->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Fixed);
  tableWidget_Hopfen->horizontalHeader()->resizeSection(10, 120);
  tableWidget_Hefe->horizontalHeader()->setSectionResizeMode(14, QHeaderView::Fixed);
  tableWidget_Hefe->horizontalHeader()->resizeSection(14, 120);
  tableWidget_WeitereZutaten->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Fixed);
  tableWidget_WeitereZutaten->horizontalHeader()->resizeSection(10, 120);
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
//
void MainWindowImpl::closeEvent(QCloseEvent *)
{
  //bei Änderung frage nach speichern
  if (Aenderung){
    if (AbfrageSpeichern()){
    }
  }
  SchreibeKonfig();
}

void MainWindowImpl::changeEvent(QEvent* event)
{
  if(0 != event)
  {
    // this event is send if a translator is loaded
    if (event->type() == QEvent::LanguageChange) {
      //qDebug() << "LanguageChange";
      Gestartet = false;
      retranslateUi(this);
      Gestartet = true;
    }
    // this event is send, if the system, language changes
    else if (event->type() == QEvent::LocaleChange) {
      //qDebug() << "LocaleChange";
      QString locale = QLocale::system().name();
      locale.truncate(locale.lastIndexOf('_'));
      loadSprache(locale);
    }
  }

  QMainWindow::changeEvent(event);
}

void MainWindowImpl::showEvent ( QShowEvent *)
{
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

void switchTranslator(QTranslator& translator, const QString& filename)
{
  // remove the old translator
  qApp->removeTranslator(&translator);

  // load the new translator
  //qDebug() << "vor install translator " << filename;
  if(translator.load(filename)) {
    //qDebug() << "install translator " << filename;
    qApp->installTranslator(&translator);
  }
}

void MainWindowImpl::loadSprache(const QString &rLanguage)
{
  qDebug() << "m_currLang: " << m_currLang;
  qDebug() << "rLanguage: " << rLanguage;
  if(m_currLang != rLanguage)
  {
    m_currLang = rLanguage;
    QLocale locale = QLocale(m_currLang);
    QLocale::setDefault(locale);
    QString languageName = QLocale::languageToString(locale.language());
    switchTranslator(m_translator, QString(m_langPath+"/kb_%1.qm").arg(rLanguage));
    switchTranslator(m_translatorQt, QString(m_langPath+"/qt_%1.qm").arg(rLanguage));
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

void MainWindowImpl::LeseStyleEinstellung()
{

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("Style");

  //Nativen Betriebssystem Style verwenden
  NativStyle = settings.value("NativStyle").toBool();

  int farbgebung = settings.value("Farbgebung").toInt();
  if ((farbgebung == 2) && (!NativStyle))
    StyleDunkel = true;
  else
    StyleDunkel = false;

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
  if (Gestartet) {
    Aenderung = value;
    setFensterTitel();
  }
}

void MainWindowImpl::resizeEvent(QResizeEvent *)
{
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


void MainWindowImpl::MalzNeueZeile()
{
  QString s = Malz_Bezeichnung_Merker;
  Malz_Bezeichnung_Merker = "";

  QTableWidgetItem *newItem1 = new QTableWidgetItem(trUtf8("Neuer Eintrag"));
  QTableWidgetItem *newItem2 = new QTableWidgetItem("");
  QTableWidgetItem *newItem3 = new QTableWidgetItem("");

  int i = tableWidget_Malz -> rowCount();
  tableWidget_Malz -> setRowCount(i+1);
  //Beschreibung
  tableWidget_Malz -> setItem(i, 0, newItem1);
  //Bemerkung
  tableWidget_Malz -> setItem(i, 5, newItem2);
  //Anwendung
  tableWidget_Malz -> setItem(i, 6, newItem3);

  //Farbe
  MyDoubleSpinBox *spinBoxFarbe = new MyDoubleSpinBox();
  spinBoxFarbe -> setMinimum(0);
  spinBoxFarbe -> setMaximum(10000);
  spinBoxFarbe -> setDecimals(1);
  connect(spinBoxFarbe, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Malz -> setCellWidget(i, 1, spinBoxFarbe);

  //Maximaler Schüttungsanteil
  MyDoubleSpinBox *spinBoxMaxSchuettung = new MyDoubleSpinBox();
  spinBoxMaxSchuettung -> setMinimum(0);
  spinBoxMaxSchuettung -> setMaximum(100);
  spinBoxMaxSchuettung -> setDecimals(0);
  spinBoxMaxSchuettung -> setValue(100);
  connect(spinBoxMaxSchuettung, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Malz -> setCellWidget(i, 2, spinBoxMaxSchuettung);

  //Menge
  MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
  spinBoxMenge -> setMinimum(0);
  spinBoxMenge -> setMaximum(1000);
  spinBoxMenge -> setDecimals(3);
  spinBoxMenge -> setSingleStep(0.1);
  connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Malz -> setCellWidget(i, 3, spinBoxMenge);

  //Preis
  MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
  spinBoxPreis -> setMinimum(0);
  spinBoxPreis -> setMaximum(1000);
  spinBoxPreis -> setDecimals(2);
  spinBoxPreis -> setSingleStep(0.1);
  connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Malz -> setCellWidget(i, 4, spinBoxPreis);

  //Datum Eingelagert
  QDateEdit * deEinlagerung = new QDateEdit(QDate::currentDate());
  deEinlagerung->setDisplayFormat("dd.MM.yyyy");
  deEinlagerung->setCalendarPopup(true);
  connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_Malz -> setCellWidget(i, 7, deEinlagerung);

  //Mindesthaltbarkeitsdatum
  QDateEdit * deMhd = new QDateEdit(QDate::currentDate());
  deMhd->setDisplayFormat("dd.MM.yyyy");
  deMhd->setCalendarPopup(true);
  connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_Malz -> setCellWidget(i, 8, deMhd);

  //Link
  QTableWidgetItem *newItem4 = new QTableWidgetItem("");
  newItem4->setTextColor(Qt::blue);
  tableWidget_Malz -> setItem(i, 9, newItem4);

  setAenderung(true);
  AenderungRohstofftabelle = true;

  Malz_Bezeichnung_Merker = s;

}

void MainWindowImpl::slot_pushButton_MalzNeuVorlage()
{
  GetRohstoffVorlage grvDia;
  grvDia.ViewMalzauswahl();
  grvDia.exec();

  //Wenn Rohstoff übernommen werden soll
  if (grvDia.b_ok) {
    //Bezeichnungsmerker löschen da sonst Einträge in den Suden geändert werden
    QString s = Malz_Bezeichnung_Merker;
    Malz_Bezeichnung_Merker = "";
    tableWidget_Malz->setSortingEnabled(false);
    MalzNeueZeile();
    //Neuen Eintrag mit den entsprechenden Werten füllen
    int row = tableWidget_Malz -> rowCount();
    //Beschreibung
    QTableWidgetItem *newItem = tableWidget_Malz -> item(row-1,0);
    newItem->setText(grvDia.m_Beschreibung);
    //Farbe
    QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(row-1,1);
    spinBox->setValue(grvDia.m_Farbe);
    //MaxProzent
    spinBox = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(row-1,2);
    spinBox->setValue(grvDia.m_MaxProzent);
    //Anwendung
    newItem = tableWidget_Malz -> item(row-1,6);
    newItem->setText(grvDia.m_Eigenschaften);
    Malz_Bezeichnung_Merker = s;
    tableWidget_Malz->setSortingEnabled(true);
  }
}

void MainWindowImpl::HopfenNeueZeile()
{
  QString s = Hopfen_Bezeichnung_Merker;
  Hopfen_Bezeichnung_Merker = "";

  QTableWidgetItem *newItem1 = new QTableWidgetItem(trUtf8("Neuer Eintrag"));
  QTableWidgetItem *newItem5 = new QTableWidgetItem(trUtf8("Pellets"));
  QTableWidgetItem *newItem6 = new QTableWidgetItem(" ");
  QTableWidgetItem *newItem7 = new QTableWidgetItem(" ");
  int i = tableWidget_Hopfen -> rowCount();
  tableWidget_Hopfen -> setRowCount(i+1);
  //Checkbox für Pellets erstellen
  newItem5 -> setCheckState(Qt::Checked);
  newItem5 -> setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
  tableWidget_Hopfen -> setItem(i, 0, newItem1);
  tableWidget_Hopfen -> setItem(i, 4, newItem5);
  tableWidget_Hopfen -> setItem(i, 5, newItem6);
  tableWidget_Hopfen -> setItem(i, 7, newItem7);

  //Alpha
  MyDoubleSpinBox *spinBoxAlpha = new MyDoubleSpinBox();
  spinBoxAlpha -> setMinimum(0);
  spinBoxAlpha -> setMaximum(100);
  spinBoxAlpha -> setDecimals(1);
  spinBoxAlpha -> setSingleStep(0.1);
  connect(spinBoxAlpha, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Hopfen -> setCellWidget(i, 1, spinBoxAlpha);

  //Menge
  MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
  spinBoxMenge -> setMinimum(0);
  spinBoxMenge -> setMaximum(999999);
  spinBoxMenge -> setDecimals(0);
  spinBoxMenge -> setSingleStep(10);
  connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Hopfen -> setCellWidget(i, 2, spinBoxMenge);

  //Preis
  MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
  spinBoxPreis -> setMinimum(0);
  spinBoxPreis -> setMaximum(999);
  spinBoxPreis -> setDecimals(2);
  spinBoxPreis -> setSingleStep(1);
  connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Hopfen -> setCellWidget(i, 3, spinBoxPreis);

  //Combobox Typ
  MyComboBox *comboBoxTyp = new MyComboBox();
  comboBoxTyp -> addItems(HopfenTypListe);
  connect(comboBoxTyp, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  tableWidget_Hopfen -> setCellWidget(i, 6, comboBoxTyp);

  //Datum Eingelagert
  QDateEdit * deEinlagerung = new QDateEdit(QDate::currentDate());
  deEinlagerung->setDisplayFormat("dd.MM.yyyy");
  deEinlagerung->setCalendarPopup(true);
  connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_Hopfen -> setCellWidget(i, 8, deEinlagerung);

  //Mindesthaltbarkeitsdatum
  QDateEdit * deMhd = new QDateEdit(QDate::currentDate());
  deMhd->setDisplayFormat("dd.MM.yyyy");
  deMhd->setCalendarPopup(true);
  connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_Hopfen -> setCellWidget(i, 9, deMhd);

  //Link
  QTableWidgetItem *newItemLink = new QTableWidgetItem("");
  newItemLink->setTextColor(Qt::blue);
  tableWidget_Hopfen -> setItem(i, 10, newItemLink);

  setAenderung(true);
  AenderungRohstofftabelle = true;

  Hopfen_Bezeichnung_Merker = s;

}

void MainWindowImpl::on_pushButton_HopfenNeuVorlage_clicked()
{
  GetRohstoffVorlage grvDia;
  grvDia.ViewHopfenauswahl();
  grvDia.exec();

  tableWidget_Hopfen->setSortingEnabled(false);
  //Wenn Rohstoff übernommen werden soll
  if (grvDia.b_ok) {
    //Bezeichnungsmerker löschen da sonst Einträge in den Suden geändert werden
    QString s = Hopfen_Bezeichnung_Merker;
    Hopfen_Bezeichnung_Merker = "";
    HopfenNeueZeile();
    //Neuen Eintrag mit den entsprechenden Werten füllen
    int row = tableWidget_Hopfen -> rowCount();
    //Beschreibung
    QTableWidgetItem *newItem = tableWidget_Hopfen -> item(row-1,0);
    newItem->setText(grvDia.m_Beschreibung);
    //Typ
    QComboBox* comboTyp=(QComboBox*)tableWidget_Hopfen -> cellWidget(row-1,6);
    comboTyp->setCurrentIndex(grvDia.m_Typ);
    //Alpha
    QDoubleSpinBox* spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(row-1,1);
    spinBox->setValue(grvDia.m_Alpha);
    //Anwendung
    newItem = tableWidget_Hopfen -> item(row-1,7);
    newItem->setText(grvDia.m_Eigenschaften);
    Hopfen_Bezeichnung_Merker = s;
  }
  tableWidget_Hopfen->setSortingEnabled(true);
}

void MainWindowImpl::on_pushButton_HefeNeuVorlage_clicked()
{
  GetRohstoffVorlage grvDia;
  grvDia.ViewHefeauswahl();
  grvDia.exec();
  tableWidget_Hefe->setSortingEnabled(false);

  //Wenn Rohstoff übernommen werden soll
  if (grvDia.b_ok) {
    //Bezeichnungsmerker löschen da sonst Einträge in den Suden geändert werden
    QString s = Hefe_Bezeichnung_Merker;
    Hefe_Bezeichnung_Merker = "";
    HefeNeueZeile();
    //Neuen Eintrag mit den entsprechenden Werten füllen
    int row = tableWidget_Hefe -> rowCount();
    //Würzemenge
    QSpinBox* spinBox = (QSpinBox*)tableWidget_Hefe -> cellWidget(row-1,2);
    spinBox->setValue(grvDia.m_Wuerzemenge);
    //Beschreibung
    QTableWidgetItem *newItem = tableWidget_Hefe -> item(row-1,0);
    newItem->setText(grvDia.m_Beschreibung);
    //Verpackungsmenge
    newItem = tableWidget_Hefe -> item(row-1,5);
    newItem->setText(grvDia.m_Verpackungsmenge);
    //Typ UG OG
    QComboBox* comboTypUGOG=(QComboBox*)tableWidget_Hefe -> cellWidget(row-1,6);
    comboTypUGOG->setCurrentIndex(grvDia.m_TypOGUG);
    //Typ Flüssig Trocken
    QComboBox* comboTypFlTr=(QComboBox*)tableWidget_Hefe -> cellWidget(row-1,7);
    comboTypFlTr->setCurrentIndex(grvDia.m_TypFlTr);
    //Temperatur bereich
    newItem = tableWidget_Hefe -> item(row-1,8);
    newItem->setText(grvDia.m_Temperatur);
    //Eigenschaften
    newItem = tableWidget_Hefe -> item(row-1,9);
    newItem->setText(grvDia.m_Eigenschaften);
    //Sedimentation
    QComboBox* comboSED=(QComboBox*)tableWidget_Hefe -> cellWidget(row-1,10);
    comboSED->setCurrentIndex(grvDia.m_SED);
    //EVG
    newItem = tableWidget_Hefe -> item(row-1,11);
    newItem->setText(grvDia.m_EVG);

    Hefe_Bezeichnung_Merker = s;
  }
  tableWidget_Hefe->setSortingEnabled(true);
}

void MainWindowImpl::HefeNeueZeile()
{

  QString s = Hefe_Bezeichnung_Merker;
  Hefe_Bezeichnung_Merker = "";

  QTableWidgetItem *newItem1 = new QTableWidgetItem("");
  QTableWidgetItem *newItem4 = new QTableWidgetItem("");
  QTableWidgetItem *newItem5 = new QTableWidgetItem("");
  QTableWidgetItem *newItem8 = new QTableWidgetItem("");
  QTableWidgetItem *newItem9 = new QTableWidgetItem("");
  QTableWidgetItem *newItem11 = new QTableWidgetItem("");
  int i = tableWidget_Hefe -> rowCount();
  tableWidget_Hefe -> setRowCount(i+1);

  //Beschreibung
  tableWidget_Hefe -> setItem(i, 0, newItem1);

  //Menge
  MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
  spinBoxMenge -> setMinimum(0);
  spinBoxMenge -> setMaximum(999999);
  spinBoxMenge -> setDecimals(0);
  spinBoxMenge -> setSingleStep(1);
  spinBoxMenge -> setValue(0);
  connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Hefe -> setCellWidget(i, 1, spinBoxMenge);

  //Würzemenge
  MyDoubleSpinBox *spinBoxWuerzemenge = new MyDoubleSpinBox();
  spinBoxWuerzemenge -> setMinimum(0);
  spinBoxWuerzemenge -> setMaximum(999999);
  spinBoxWuerzemenge -> setDecimals(0);
  spinBoxWuerzemenge -> setSingleStep(1);
  spinBoxWuerzemenge -> setValue(0);
  connect(spinBoxWuerzemenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Hefe -> setCellWidget(i, 2, spinBoxWuerzemenge);

  //Preis
  MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
  spinBoxPreis -> setMinimum(0);
  spinBoxPreis -> setMaximum(999);
  spinBoxPreis -> setDecimals(2);
  spinBoxPreis -> setSingleStep(0.1);
  spinBoxPreis -> setValue(0);
  connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Hefe -> setCellWidget(i, 3, spinBoxPreis);

  //Bemerkung
  tableWidget_Hefe -> setItem(i, 4, newItem4);

  //Verpackungsmenge
  tableWidget_Hefe -> setItem(i, 5, newItem5);

  //Combobox Typ Obergärig Untergärig
  MyComboBox *comboBoxTypOGUG = new MyComboBox();
  comboBoxTypOGUG -> addItems(HefeTypOGUGListe);
  connect(comboBoxTypOGUG, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  tableWidget_Hefe -> setCellWidget(i, 6, comboBoxTypOGUG);

  //Combobox Typ Flüssig Trocken
  MyComboBox *comboBoxTypTrFl = new MyComboBox();
  comboBoxTypTrFl -> addItems(HefeTypTrFlListe);
  connect(comboBoxTypTrFl, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  tableWidget_Hefe -> setCellWidget(i, 7, comboBoxTypTrFl);

  //Temperaturbereich
  tableWidget_Hefe -> setItem(i, 8, newItem8);

  //Eigenschaften
  tableWidget_Hefe -> setItem(i, 9, newItem9);

  //Combobox Sedimentation
  MyComboBox *comboBoxSED = new MyComboBox();
  comboBoxSED -> addItems(HefeSedListe);
  connect(comboBoxSED, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  tableWidget_Hefe -> setCellWidget(i, 10, comboBoxSED);

  //Endvergärungsgrad
  tableWidget_Hefe -> setItem(i, 11, newItem11);


  //Datum Eingelagert
  QDateEdit * deEinlagerung = new QDateEdit(QDate::currentDate());
  deEinlagerung->setDisplayFormat("dd.MM.yyyy");
  deEinlagerung->setCalendarPopup(true);
  connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_Hefe -> setCellWidget(i, 12, deEinlagerung);

  //Mindesthaltbarkeitsdatum
  QDateEdit * deMhd = new QDateEdit(QDate::currentDate());
  deMhd->setDisplayFormat("dd.MM.yyyy");
  deMhd->setCalendarPopup(true);
  connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_Hefe -> setCellWidget(i, 13, deMhd);

  //Link
  QTableWidgetItem *newItemLink = new QTableWidgetItem("");
  newItemLink->setTextColor(Qt::blue);
  tableWidget_Hefe -> setItem(i, 14, newItemLink);

  setAenderung(true);
  AenderungRohstofftabelle = true;

  Hefe_Bezeichnung_Merker = s;

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
    QDoubleSpinBox* spinBoxFarbe=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,1);
    QDoubleSpinBox* spinBoxMaxSchuettung=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,2);
    QDoubleSpinBox* spinBoxMenge=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,3);
    QDoubleSpinBox* spinBoxPreis=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,4);
    QDateEdit* deEinlagerung=(QDateEdit*)tableWidget_Malz -> cellWidget(i,7);
    QDateEdit* deMhd=(QDateEdit*)tableWidget_Malz -> cellWidget(i,8);
    sql = "INSERT INTO Malz(Beschreibung, Farbe, MaxProzent, Menge, Preis, Bemerkung, Anwendung, Eingelagert, Mindesthaltbar, Link) VALUES(\'" +
        tableWidget_Malz -> item(i,0) -> text().replace("'","''") +	"\'," +
        QString::number(spinBoxFarbe -> value()) + "," +
        QString::number(spinBoxMaxSchuettung -> value()) + "," +
        QString::number(spinBoxMenge -> value()) + "," +
        QString::number(spinBoxPreis -> value()) + ",\'" +
        tableWidget_Malz -> item(i,5) -> text().replace("'","''") +	"\',\'" +
        tableWidget_Malz -> item(i,6) -> text().replace("'","''") + "\','"+
        deEinlagerung -> date().toString(Qt::ISODate) + "','" +
        deMhd -> date().toString(Qt::ISODate) + "','" +
        tableWidget_Malz -> item(i,9) -> text().replace("'","''") + "')";
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
    bool b = tableWidget_Hopfen -> item(i,4) -> checkState();
    QDoubleSpinBox* spinBoxAlpha=(QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,1);
    QDoubleSpinBox* spinBoxMenge=(QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,2);
    QDoubleSpinBox* spinBoxPreis=(QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,3);
    QComboBox* comboTyp=(QComboBox*)tableWidget_Hopfen -> cellWidget(i,6);
    QDateEdit* deEinlagerung=(QDateEdit*)tableWidget_Hopfen -> cellWidget(i,8);
    QDateEdit* deMhd=(QDateEdit*)tableWidget_Hopfen -> cellWidget(i,9);
    sql = "INSERT INTO Hopfen(Beschreibung, Alpha, Menge, Preis, Pellets, Bemerkung, Eigenschaften, Typ, Eingelagert, Mindesthaltbar, Link) VALUES(\'" +
        tableWidget_Hopfen -> item(i,0) -> text().replace("'","''") +	"\'," +
        QString::number(spinBoxAlpha -> value()) + "," +
        QString::number(spinBoxMenge -> value()) + "," +
        QString::number(spinBoxPreis -> value()) + "," +
        QString::number(b) + ",\'" +
        tableWidget_Hopfen -> item(i,5) -> text().replace("'","''") + "\',\'" +
        tableWidget_Hopfen -> item(i,7) -> text().replace("'","''") + "\'," +
        QString::number(comboTyp -> currentIndex()) + ",'" +
        deEinlagerung -> date().toString(Qt::ISODate) + "','" +
        deMhd -> date().toString(Qt::ISODate) + "','" +
        tableWidget_Hopfen -> item(i,10) -> text().replace("'","''") + "')";
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
    QDoubleSpinBox* spinBoxMenge=(QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,1);
    QDoubleSpinBox* spinBoxWuerzemenge=(QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,2);
    QDoubleSpinBox* spinBoxPreis=(QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,3);
    QComboBox* comboTypOGUG=(QComboBox*)tableWidget_Hefe -> cellWidget(i,6);
    QComboBox* comboTypTrFl=(QComboBox*)tableWidget_Hefe -> cellWidget(i,7);
    QComboBox* comboSED=(QComboBox*)tableWidget_Hefe -> cellWidget(i,10);
    QDateEdit* deEinlagerung=(QDateEdit*)tableWidget_Hefe -> cellWidget(i,12);
    QDateEdit* deMhd=(QDateEdit*)tableWidget_Hefe -> cellWidget(i,13);
    sql = "INSERT INTO Hefe(Beschreibung, Menge, Wuerzemenge, Preis, Bemerkung, Verpackungsmenge, TypOGUG, TypTrFl, Temperatur, Eigenschaften, SED, EVG, Eingelagert, Mindesthaltbar, Link) VALUES(\'" +
        tableWidget_Hefe -> item(i,0) -> text().replace("'","''") +	"\'," +
        QString::number(spinBoxMenge -> value()) + "," +
        QString::number(spinBoxWuerzemenge -> value()) + "," +
        QString::number(spinBoxPreis -> value()) + ",\'" +
        tableWidget_Hefe -> item(i,4) -> text().replace("'","''") + "\',\'" +
        tableWidget_Hefe -> item(i,5) -> text().replace("'","''") + "\'," +
        QString::number(comboTypOGUG -> currentIndex()) + "," +
        QString::number(comboTypTrFl -> currentIndex()) + ",\'" +
        tableWidget_Hefe -> item(i,8) -> text().replace("'","''") + "\',\'" +
        tableWidget_Hefe -> item(i,9) -> text().replace("'","''") + "\'," +
        QString::number(comboSED -> currentIndex()) + ",\'" +
        tableWidget_Hefe -> item(i,11) -> text().replace("'","''") + "\','" +
        deEinlagerung -> date().toString(Qt::ISODate) + "','" +
        deMhd -> date().toString(Qt::ISODate) + "','" +
        tableWidget_Hefe -> item(i,14) -> text().replace("'","''") + "')";
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
    QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,1);
    QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(i,2);
    QComboBox* comboTyp=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(i,3);
    QDoubleSpinBox* dsbAusbeute=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,4);
    QDoubleSpinBox* dsbEBC=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,5);
    QDoubleSpinBox* dsbPreis=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,6);
    QDateEdit* deEinlagerung=(QDateEdit*)tableWidget_WeitereZutaten -> cellWidget(i,8);
    QDateEdit* deMhd=(QDateEdit*)tableWidget_WeitereZutaten -> cellWidget(i,9);
    sql = "INSERT INTO WeitereZutaten(Beschreibung, Menge, Einheiten, Typ, Ausbeute, EBC, Preis, Bemerkung, Eingelagert, Mindesthaltbar, Link) VALUES(\'" +
        tableWidget_WeitereZutaten -> item(i,0) -> text().replace("'","''") +	"\'," +
        QString::number(dsbMenge -> value()) + "," +
        QString::number(comboEinheit -> currentIndex()) + "," +
        QString::number(comboTyp -> currentIndex()) + "," +
        QString::number(dsbAusbeute -> value()) + "," +
        QString::number(dsbEBC -> value()) + "," +
        QString::number(dsbPreis -> value()) + "," +
        "\'" + tableWidget_WeitereZutaten -> item(i,7) -> text().replace("'","''") + "\','" +
        deEinlagerung -> date().toString(Qt::ISODate) + "','" +
        deMhd -> date().toString(Qt::ISODate) + "','" +
        tableWidget_WeitereZutaten -> item(i,10) -> text().replace("'","''") + "')";
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
  int FeldNr;

  //Sollte noch eine Zeile selectiert sein selection löschen
  tableWidget_Malz->clearSelection();
  Malz_Bezeichnung_Merker = "";

  //Malz einlesen
  QString sql = "SELECT * FROM Malz ORDER BY Beschreibung ASC";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int i=0;
    tableWidget_Malz -> clearContents();
    tableWidget_Malz -> setRowCount(0);
    while (query.next()){
      QTableWidgetItem *newItem1 = new QTableWidgetItem("");
      QTableWidgetItem *newItem2 = new QTableWidgetItem("");
      QTableWidgetItem *newItem3 = new QTableWidgetItem("");
      tableWidget_Malz -> setRowCount(tableWidget_Malz -> rowCount()+1);
      //Beschreibung
      FeldNr = query.record().indexOf("Beschreibung");
      newItem1 -> setText(query.value(FeldNr).toString());
      tableWidget_Malz -> setItem(i, 0, newItem1);
      //Farbe
      MyDoubleSpinBox *spinBoxFarbe = new MyDoubleSpinBox();
      spinBoxFarbe -> setMinimum(0);
      spinBoxFarbe -> setMaximum(10000);
      spinBoxFarbe -> setDecimals(1);
      FeldNr = query.record().indexOf("Farbe");
      spinBoxFarbe -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxFarbe, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      tableWidget_Malz -> setCellWidget(i, 1, spinBoxFarbe);

      //Maximaler Schüttungsanteil
      MyDoubleSpinBox *spinBoxMaxSchuettung = new MyDoubleSpinBox();
      spinBoxMaxSchuettung -> setMinimum(0);
      spinBoxMaxSchuettung -> setMaximum(100);
      spinBoxMaxSchuettung -> setDecimals(0);
      FeldNr = query.record().indexOf("MaxProzent");
      spinBoxMaxSchuettung -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxMaxSchuettung, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      tableWidget_Malz -> setCellWidget(i, 2, spinBoxMaxSchuettung);

      //Menge
      MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
      spinBoxMenge -> setMinimum(0);
      spinBoxMenge -> setMaximum(1000);
      spinBoxMenge -> setDecimals(3);
      spinBoxMenge -> setSingleStep(0.1);
      FeldNr = query.record().indexOf("Menge");
      spinBoxMenge -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      tableWidget_Malz -> setCellWidget(i, 3, spinBoxMenge);

      //Preis
      MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
      spinBoxPreis -> setMinimum(0);
      spinBoxPreis -> setMaximum(1000);
      spinBoxPreis -> setDecimals(2);
      spinBoxPreis -> setSingleStep(0.1);
      FeldNr = query.record().indexOf("Preis");
      spinBoxPreis -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      tableWidget_Malz -> setCellWidget(i, 4, spinBoxPreis);

      //Bemerkung
      FeldNr = query.record().indexOf("Bemerkung");
      newItem2 -> setText(query.value(FeldNr).toString());
      tableWidget_Malz -> setItem(i, 5, newItem2);

      //Anwendung
      FeldNr = query.record().indexOf("Anwendung");
      newItem3 -> setText(query.value(FeldNr).toString());
      tableWidget_Malz -> setItem(i, 6, newItem3);

      //Datum Eingelagert
      QDateEdit * deEinlagerung = new QDateEdit();
      deEinlagerung->setDisplayFormat("dd.MM.yyyy");
      deEinlagerung->setCalendarPopup(true);
      FeldNr = query.record().indexOf("Eingelagert");
      deEinlagerung -> setDate(QDate::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
      tableWidget_Malz -> setCellWidget(i, 7, deEinlagerung);

      //Mindesthaltbarkeitsdatum
      QDateEdit * deMhd = new QDateEdit();
      deMhd->setDisplayFormat("dd.MM.yyyy");
      deMhd->setCalendarPopup(true);
      FeldNr = query.record().indexOf("Mindesthaltbar");
      deMhd -> setDate(QDate::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
      tableWidget_Malz -> setCellWidget(i, 8, deMhd);

      //Link
      FeldNr = query.record().indexOf("Link");
      QTableWidgetItem *newItemLink = new QTableWidgetItem(query.value(FeldNr).toString());
      newItemLink->setTextColor(Qt::blue);
      tableWidget_Malz -> setItem(i, 9, newItemLink);

      i++;
    }
    tableWidget_Malz -> horizontalHeader() -> setSectionResizeMode(QHeaderView::ResizeToContents);
  }

  //Sollte noch eine Zeile selectiert sein selection löschen
  tableWidget_Hopfen->clearSelection();
  Hopfen_Bezeichnung_Merker = "";

  //Hopfen Einlesen
  sql = "SELECT * FROM Hopfen ORDER BY Beschreibung ASC";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int i=0;
    tableWidget_Hopfen -> clearContents();
    tableWidget_Hopfen -> setRowCount(0);
    while (query.next()){
      QTableWidgetItem *newItem1 = new QTableWidgetItem("");
      QTableWidgetItem *newItem5 = new QTableWidgetItem(trUtf8("Pellets"));
      QTableWidgetItem *newItem6 = new QTableWidgetItem("");
      QTableWidgetItem *newItem7 = new QTableWidgetItem("");
      tableWidget_Hopfen -> setRowCount(tableWidget_Hopfen -> rowCount()+1);

      //Beschreibung
      FeldNr = query.record().indexOf("Beschreibung");
      newItem1 -> setText(query.value(FeldNr).toString());
      tableWidget_Hopfen -> setItem(i, 0, newItem1);

      //Alpha
      MyDoubleSpinBox *spinBoxAlpha = new MyDoubleSpinBox();
      spinBoxAlpha -> setMinimum(0);
      spinBoxAlpha -> setMaximum(100);
      spinBoxAlpha -> setDecimals(1);
      spinBoxAlpha -> setSingleStep(0.1);
      FeldNr = query.record().indexOf("Alpha");
      spinBoxAlpha -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxAlpha, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      tableWidget_Hopfen -> setCellWidget(i, 1, spinBoxAlpha);

      //Menge
      MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
      spinBoxMenge -> setMinimum(0);
      spinBoxMenge -> setMaximum(999999);
      spinBoxMenge -> setDecimals(0);
      spinBoxMenge -> setSingleStep(10);
      FeldNr = query.record().indexOf("Menge");
      spinBoxMenge -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      tableWidget_Hopfen -> setCellWidget(i, 2, spinBoxMenge);

      //Preis
      MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
      spinBoxPreis -> setMinimum(0);
      spinBoxPreis -> setMaximum(999);
      spinBoxPreis -> setDecimals(2);
      spinBoxPreis -> setSingleStep(1);
      FeldNr = query.record().indexOf("Preis");
      spinBoxPreis -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      tableWidget_Hopfen -> setCellWidget(i, 3, spinBoxPreis);

      //Pellets
      FeldNr = query.record().indexOf("Pellets");
      bool b = query.value(FeldNr).toBool();
      if (b)
        newItem5 -> setCheckState(Qt::Checked);
      else
        newItem5 -> setCheckState(Qt::Unchecked);
      newItem5 -> setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
      tableWidget_Hopfen -> setItem(i, 4, newItem5);
      //Bemerkung
      FeldNr = query.record().indexOf("Bemerkung");
      newItem6 -> setText(query.value(FeldNr).toString());
      tableWidget_Hopfen -> setItem(i, 5, newItem6);

      //Combobox Typ
      MyComboBox *comboBoxTyp = new MyComboBox();
      comboBoxTyp -> addItems(HopfenTypListe);
      FeldNr = query.record().indexOf("Typ");
      comboBoxTyp -> setCurrentIndex(query.value(FeldNr).toInt());
      connect(comboBoxTyp, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
      tableWidget_Hopfen -> setCellWidget(i, 6, comboBoxTyp);

      //Eigenschaften
      FeldNr = query.record().indexOf("Eigenschaften");
      newItem7 -> setText(query.value(FeldNr).toString());
      tableWidget_Hopfen -> setItem(i, 7, newItem7);

      //Datum Eingelagert
      QDateEdit * deEinlagerung = new QDateEdit();
      deEinlagerung->setDisplayFormat("dd.MM.yyyy");
      deEinlagerung->setCalendarPopup(true);
      FeldNr = query.record().indexOf("Eingelagert");
      deEinlagerung -> setDate(QDate::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
      tableWidget_Hopfen -> setCellWidget(i, 8, deEinlagerung);

      //Mindesthaltbarkeitsdatum
      QDateEdit * deMhd = new QDateEdit();
      deMhd->setDisplayFormat("dd.MM.yyyy");
      deMhd->setCalendarPopup(true);
      FeldNr = query.record().indexOf("Mindesthaltbar");
      deMhd -> setDate(QDate::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
      tableWidget_Hopfen -> setCellWidget(i, 9, deMhd);

      //Link
      FeldNr = query.record().indexOf("Link");
      QTableWidgetItem *newItemLink = new QTableWidgetItem(query.value(FeldNr).toString());
      newItemLink->setTextColor(Qt::blue);
      tableWidget_Hopfen -> setItem(i, 10, newItemLink);

      i++;
    }
    tableWidget_Hopfen -> horizontalHeader() -> setSectionResizeMode(QHeaderView::ResizeToContents);
  }

  //Hefe Einlesen
  sql = "SELECT * FROM Hefe ORDER BY Beschreibung ASC";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int i=0;
    tableWidget_Hefe -> clearContents();
    tableWidget_Hefe -> setRowCount(0);
    while (query.next()){
      QTableWidgetItem *newItem1 = new QTableWidgetItem("");
      QTableWidgetItem *newItem4 = new QTableWidgetItem("");
      QTableWidgetItem *newItem5 = new QTableWidgetItem("");
      QTableWidgetItem *newItem8 = new QTableWidgetItem("");
      QTableWidgetItem *newItem9 = new QTableWidgetItem("");
      QTableWidgetItem *newItem11 = new QTableWidgetItem("");
      tableWidget_Hefe -> setRowCount(tableWidget_Hefe -> rowCount()+1);
      //Beschreibung
      FeldNr = query.record().indexOf("Beschreibung");
      newItem1 -> setText(query.value(FeldNr).toString());
      tableWidget_Hefe -> setItem(i, 0, newItem1);

      //Menge
      MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
      spinBoxMenge -> setMinimum(0);
      spinBoxMenge -> setMaximum(999999);
      spinBoxMenge -> setDecimals(0);
      spinBoxMenge -> setSingleStep(1);
      FeldNr = query.record().indexOf("Menge");
      spinBoxMenge -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      tableWidget_Hefe -> setCellWidget(i, 1, spinBoxMenge);

      //Benötigte einheiten wird nicht mehr gebraucht
      //			FeldNr = query.record().indexOf("Einheiten");
      //			d = query.value(FeldNr).toDouble();
      //			newItem3 -> setData(Qt::DisplayRole, d);
      //			tableWidget_Hefe -> setItem(i, 2, newItem3);

      //Würzemenge
      MyDoubleSpinBox *spinBoxWuerzemenge = new MyDoubleSpinBox();
      spinBoxWuerzemenge -> setMinimum(0);
      spinBoxWuerzemenge -> setMaximum(999999);
      spinBoxWuerzemenge -> setDecimals(0);
      spinBoxWuerzemenge -> setSingleStep(1);
      FeldNr = query.record().indexOf("Wuerzemenge");
      spinBoxWuerzemenge -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxWuerzemenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      tableWidget_Hefe -> setCellWidget(i, 2, spinBoxWuerzemenge);

      //Preis
      MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
      spinBoxPreis -> setMinimum(0);
      spinBoxPreis -> setMaximum(999);
      spinBoxPreis -> setDecimals(2);
      spinBoxPreis -> setSingleStep(0.1);
      FeldNr = query.record().indexOf("Preis");
      spinBoxPreis -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      tableWidget_Hefe -> setCellWidget(i, 3, spinBoxPreis);

      //Bemerkung
      FeldNr = query.record().indexOf("Bemerkung");
      newItem4 -> setText(query.value(FeldNr).toString());
      tableWidget_Hefe -> setItem(i, 4, newItem4);

      //Verpackungsmenge
      FeldNr = query.record().indexOf("Verpackungsmenge");
      newItem5 -> setText(query.value(FeldNr).toString());
      tableWidget_Hefe -> setItem(i, 5, newItem5);

      //Combobox Typ Obergärig Untergärig
      MyComboBox *comboBoxTypOGUG = new MyComboBox();
      comboBoxTypOGUG -> addItems(HefeTypOGUGListe);
      FeldNr = query.record().indexOf("TypOGUG");
      comboBoxTypOGUG -> setCurrentIndex(query.value(FeldNr).toInt());
      connect(comboBoxTypOGUG, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
      tableWidget_Hefe -> setCellWidget(i, 6, comboBoxTypOGUG);

      //Combobox Typ Flüssig Trocken
      MyComboBox *comboBoxTypTrFl = new MyComboBox();
      comboBoxTypTrFl -> addItems(HefeTypTrFlListe);
      FeldNr = query.record().indexOf("TypTrFl");
      comboBoxTypTrFl -> setCurrentIndex(query.value(FeldNr).toInt());
      connect(comboBoxTypTrFl, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
      tableWidget_Hefe -> setCellWidget(i, 7, comboBoxTypTrFl);

      //Temperaturbereich
      FeldNr = query.record().indexOf("Temperatur");
      newItem8 -> setText(query.value(FeldNr).toString());
      tableWidget_Hefe -> setItem(i, 8, newItem8);

      //Eigenschaften
      FeldNr = query.record().indexOf("Eigenschaften");
      newItem9 -> setText(query.value(FeldNr).toString());
      tableWidget_Hefe -> setItem(i, 9, newItem9);

      //Combobox Sedimentation
      MyComboBox *comboBoxSED = new MyComboBox();
      comboBoxSED -> addItems(HefeSedListe);
      FeldNr = query.record().indexOf("SED");
      comboBoxSED -> setCurrentIndex(query.value(FeldNr).toInt());
      connect(comboBoxSED, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
      tableWidget_Hefe -> setCellWidget(i, 10, comboBoxSED);

      //Endvergärungsgrad
      FeldNr = query.record().indexOf("EVG");
      newItem11 -> setText(query.value(FeldNr).toString());
      tableWidget_Hefe -> setItem(i, 11, newItem11);

      //Datum Eingelagert
      QDateEdit * deEinlagerung = new QDateEdit();
      deEinlagerung->setDisplayFormat("dd.MM.yyyy");
      deEinlagerung->setCalendarPopup(true);
      FeldNr = query.record().indexOf("Eingelagert");
      deEinlagerung -> setDate(QDate::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
      tableWidget_Hefe -> setCellWidget(i, 12, deEinlagerung);

      //Mindesthaltbarkeitsdatum
      QDateEdit * deMhd = new QDateEdit();
      deMhd->setDisplayFormat("dd.MM.yyyy");
      deMhd->setCalendarPopup(true);
      FeldNr = query.record().indexOf("Mindesthaltbar");
      deMhd -> setDate(QDate::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
      tableWidget_Hefe -> setCellWidget(i, 13, deMhd);

      //Link
      FeldNr = query.record().indexOf("Link");
      QTableWidgetItem *newItemLink = new QTableWidgetItem(query.value(FeldNr).toString());
      newItemLink->setTextColor(Qt::blue);
      tableWidget_Hefe -> setItem(i, 14, newItemLink);

      //Nächste Beschreibung aus Datei lesen
      i++;
    }
    tableWidget_Hefe -> horizontalHeader() -> setSectionResizeMode(QHeaderView::ResizeToContents);
  }

  //Weitere Zutaten einlesen
  sql = "SELECT * FROM WeitereZutaten ORDER BY Beschreibung ASC";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int i=0;
    tableWidget_WeitereZutaten -> clearContents();
    tableWidget_WeitereZutaten -> setRowCount(0);
    while (query.next()){
      QTableWidgetItem *newItem1 = new QTableWidgetItem("");
      QTableWidgetItem *newItem7 = new QTableWidgetItem("");
      tableWidget_WeitereZutaten -> setRowCount(tableWidget_WeitereZutaten -> rowCount() +1);
      //Beschreibung
      FeldNr = query.record().indexOf("Beschreibung");
      newItem1 -> setText(query.value(FeldNr).toString());
      tableWidget_WeitereZutaten -> setItem(i, 0, newItem1);

      //Menge
      MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
      spinBoxMenge -> setMinimum(0);
      spinBoxMenge -> setMaximum(10000);
      FeldNr = query.record().indexOf("Menge");
      spinBoxMenge -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
      tableWidget_WeitereZutaten -> setCellWidget(i, 1, spinBoxMenge);

      //Combobox Einheiten
      MyComboBox *comboBoxEinheiten = new MyComboBox();
      comboBoxEinheiten -> addItems(EinheitenListe);
      FeldNr = query.record().indexOf("Einheiten");
      comboBoxEinheiten -> setCurrentIndex(query.value(FeldNr).toInt());
      connect(comboBoxEinheiten, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
      connect(comboBoxEinheiten, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
      tableWidget_WeitereZutaten -> setCellWidget(i, 2, comboBoxEinheiten);

      //Combobox Typ
      MyComboBox *comboBoxTyp = new MyComboBox();
      comboBoxTyp -> addItems(ZutatenTypListe);
      FeldNr = query.record().indexOf("Typ");
      comboBoxTyp -> setCurrentIndex(query.value(FeldNr).toInt());
      connect(comboBoxTyp, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
      connect(comboBoxTyp, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
      tableWidget_WeitereZutaten -> setCellWidget(i, 3, comboBoxTyp);

      //Ausbeute
      MyDoubleSpinBox *spinBoxAusbeute = new MyDoubleSpinBox();
      spinBoxAusbeute -> setMinimum(0);
      spinBoxAusbeute -> setMaximum(100);
      spinBoxAusbeute -> setDecimals(0);
      spinBoxAusbeute -> setToolTip(trUtf8("Anteil an der Stamwürze in Prozent: 0=Wird bei der Berechnung der Stammwürze nicht berücksichtigt"));
      FeldNr = query.record().indexOf("Ausbeute");
      spinBoxAusbeute -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxAusbeute, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      connect(spinBoxAusbeute, SIGNAL( valueChanged(double) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
      tableWidget_WeitereZutaten -> setCellWidget(i, 4, spinBoxAusbeute);

      //EBC
      MyDoubleSpinBox *spinBoxEBC = new MyDoubleSpinBox();
      spinBoxEBC -> setMinimum(0);
      spinBoxEBC -> setMaximum(100000);
      spinBoxEBC -> setDecimals(1);
      spinBoxEBC -> setToolTip(trUtf8("Farbwert: 0=Wird bei der berechnung der Farbe nicht berücksichtigt"));
      FeldNr = query.record().indexOf("EBC");
      spinBoxEBC -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxEBC, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      connect(spinBoxEBC, SIGNAL( valueChanged(double) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
      tableWidget_WeitereZutaten -> setCellWidget(i, 5, spinBoxEBC);

      //Preis
      MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
      spinBoxPreis -> setMinimum(0);
      spinBoxPreis -> setMaximum(1000);
      spinBoxPreis -> setToolTip(trUtf8("Preis pro Kilogramm"));
      FeldNr = query.record().indexOf("Preis");
      spinBoxPreis -> setValue(query.value(FeldNr).toReal());
      connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
      connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
      tableWidget_WeitereZutaten -> setCellWidget(i, 6, spinBoxPreis);

      //Bemerkung
      FeldNr = query.record().indexOf("Bemerkung");
      newItem7 -> setText(query.value(FeldNr).toString());
      tableWidget_WeitereZutaten -> setItem(i, 7, newItem7);

      //Datum Eingelagert
      QDateEdit * deEinlagerung = new QDateEdit();
      deEinlagerung->setDisplayFormat("dd.MM.yyyy");
      deEinlagerung->setCalendarPopup(true);
      FeldNr = query.record().indexOf("Eingelagert");
      deEinlagerung -> setDate(QDate::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
      tableWidget_WeitereZutaten -> setCellWidget(i, 8, deEinlagerung);

      //Mindesthaltbarkeitsdatum
      QDateEdit * deMhd = new QDateEdit();
      deMhd->setDisplayFormat("dd.MM.yyyy");
      deMhd->setCalendarPopup(true);
      FeldNr = query.record().indexOf("Mindesthaltbar");
      deMhd -> setDate(QDate::fromString(query.value(FeldNr).toString(),Qt::ISODate));
      connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
      tableWidget_WeitereZutaten -> setCellWidget(i, 9, deMhd);

      //Link
      FeldNr = query.record().indexOf("Link");
      QTableWidgetItem *newItemLink = new QTableWidgetItem(query.value(FeldNr).toString());
      newItemLink->setTextColor(Qt::blue);
      tableWidget_WeitereZutaten -> setItem(i, 10, newItemLink);

      i++;
    }
    tableWidget_WeitereZutaten -> horizontalHeader() -> setSectionResizeMode(QHeaderView::ResizeToContents);
  }


  //Wasserwerte einlesen
  sql = "SELECT * FROM Wasser";
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    query.first();
    //Calcium
    FeldNr = query.record().indexOf("Calcium");
    SpinBox_wwCalcium_mg -> setValue(query.value(FeldNr).toDouble());
    //Magnesium
    FeldNr = query.record().indexOf("Magnesium");
    SpinBox_wwMagnesium_mg -> setValue(query.value(FeldNr).toDouble());
    //Säurekapazität
    FeldNr = query.record().indexOf("Saeurekapazitaet");
    SpinBox_wwSaeurekapazitaet_mmol -> setValue(query.value(FeldNr).toDouble());
  }
}


void MainWindowImpl::slot_pushButton_MalzDel()
{
  //Überprüfen ob bei nicht gebrauten Suden der Rohstoff verwendet wird.

  //Rohstoffname
  QString del_name = tableWidget_Malz -> item(tableWidget_Malz -> currentRow(),0) -> text();

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
              if (del_name != tableWidget_Malz -> item(i,0) -> text()){
                raDia.addAuswahlEintrag(tableWidget_Malz -> item(i,0) -> text() );
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

void MainWindowImpl::slot_pushButton_HopfenDel()
{
  //Überprüfen ob bei nicht gebrauten Suden der Rohstoff verwendet wird.

  //Rohstoffname
  QString del_name = tableWidget_Hopfen -> item(tableWidget_Hopfen -> currentRow(),0) -> text();

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
              if (del_name != tableWidget_Hopfen -> item(i,0) -> text()){
                raDia.addAuswahlEintrag(tableWidget_Hopfen -> item(i,0) -> text() );
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
              if (del_name != tableWidget_Hopfen -> item(i,0) -> text()){
                raDia.addAuswahlEintrag(tableWidget_Hopfen -> item(i,0) -> text() );
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

void MainWindowImpl::slot_pushButton_HefeDel()
{
  //Überprüfen ob bei nicht gebrauten Suden der Rohstoff verwendet wird.

  //Rohstoffname
  QString del_name = tableWidget_Hefe -> item(tableWidget_Hefe -> currentRow(),0) -> text();

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
            if (del_name != tableWidget_Hefe -> item(i,0) -> text()){
              raDia.addAuswahlEintrag(tableWidget_Hefe -> item(i,0) -> text() );
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
  settings.setValue("size", size());
  settings.setValue("pos", pos());
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
  resize(settings.value("size", QSize(600, 400)).toSize());
  move(settings.value("pos", QPoint(200, 200)).toPoint());
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

  //Sprache
  settings.beginGroup("Sprache");
  str = settings.value("sprachauswahl").toString();
  if (str == "") {
    settings.setValue("sprachauswahl","de");
    str = "de";
  }
  sprachauswahl = str;
  qDebug() << "sprachauswahl" << sprachauswahl;
  settings.endGroup();

  //Einstellungen Sonstiges
  settings.beginGroup("sonstiges");
  checkBox_MerklisteMengen->setChecked(settings.value("MerklisteMengenEinbeziehen").toBool());
  settings.endGroup();

}


void MainWindowImpl::createActions()
{

  saveAct = new QAction(trUtf8("&Speichern"), this);
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(trUtf8("Speichere die aktuellen Suddaten"));
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

  for (int i = 0; i < MaxRecentFiles; ++i) {
    recentFileActs[i] = new QAction(this);
    recentFileActs[i] -> setVisible(false);
    connect(recentFileActs[i], SIGNAL(triggered()),	this, SLOT(openRecentFile()));
  }

  //Aktionen in Menü Extras
  einstellungen = new QAction(trUtf8("&Einstellungen"), this);
  //einstellungen->setShortcuts(QKeySequence::Save);
  einstellungen -> setStatusTip(trUtf8("Öffnet einen Dialog mit Einstellungen"));
  connect(einstellungen, SIGNAL(triggered()), this, SLOT(slot_einstellungen()));

  //Assistent zum übernehmen von einem rezept
  schuettungProzent = new QAction(trUtf8("&Rezeptübernahme Schüttung"), this);
  //einstellungen->setShortcuts(QKeySequence::Save);
  schuettungProzent -> setStatusTip(trUtf8("Öffnet einen Dialog zur unterstützung für die Übernahme der Schüttung"));
  connect(schuettungProzent, SIGNAL(triggered()), this, SLOT(slot_schuettungProzent()));

  //Assistent zum berechnen de IBU Wertes eines Rezeptes
  berIBU = new QAction(trUtf8("&Rezeptübernahme Bittere"), this);
  //einstellungen->setShortcuts(QKeySequence::Save);
  berIBU -> setStatusTip(trUtf8("Öffnet einen Dialog zur Berechnung der Bittere"));
  connect(berIBU, SIGNAL(triggered()), this, SLOT(slot_berIBU()));

  //Hebt die Eingabesperre von einen als gebraut/Abgefüllt markierten Sud auf
  EntsperreEingabefelder = new QAction(trUtf8("&Entsperre Eingabefelder"), this);
  EntsperreEingabefelder -> setStatusTip(trUtf8("Hebt die Eingabesperre der Eingabefelder auf"));
  connect(EntsperreEingabefelder, SIGNAL(triggered()), this, SLOT(slot_EntsperreEingabefelder()));

  //Setzt das Bit BierGebraut zurück
  ResetBierGebraut = new QAction(trUtf8("Bier wurde &Gebraut zurücksetzten"), this);
  ResetBierGebraut -> setStatusTip(trUtf8("Setzt das Bit Bier wurde Gebraut von dem aktuellen Sud in der Datenbank zurück"));
  connect(ResetBierGebraut, SIGNAL(triggered()), this, SLOT(slot_ResetBierWurdeGebraut()));

  //Setzt das Bit Abgefuellt zurück
  ResetAbgefuellt = new QAction(trUtf8("Bier &Abgefüllt zurücksetzten"), this);
  ResetAbgefuellt -> setStatusTip(trUtf8("Setzt das Bit Abgefüllt von dem aktuellen Sud in der Datenbank zurück"));
  connect(ResetAbgefuellt, SIGNAL(triggered()), this, SLOT(slot_ResetAbgefuellt()));

  //Setzt das Bit Abgefuellt zurück
  ResetVerbraucht = new QAction(trUtf8("Bier &Verbraucht zurücksetzten"), this);
  ResetVerbraucht -> setStatusTip(trUtf8("Setzt das Bit Bier Verbraucht von dem aktuellen Sud in der Datenbank zurück"));
  connect(ResetVerbraucht, SIGNAL(triggered()), this, SLOT(slot_ResetBierVerbraucht()));

  //Setzt den Zugabestatus der Weiteren Zutaten zurück
  ResetZugabestatus = new QAction(trUtf8("&Reset Zugabestatus WZutaten"), this);
  ResetZugabestatus -> setStatusTip(trUtf8("setzt den Zugabestatus der Weiteren Zutaten zurück"));
  connect(ResetZugabestatus, SIGNAL(triggered()), this, SLOT(slot_ResetWZZugabestatus()));
}

void MainWindowImpl::ErstelleSprachMenu()
{
  sprachMenu = menuBar()->addMenu(trUtf8("&Sprache"));
  QActionGroup* langGroup = new QActionGroup(sprachMenu);
  langGroup->setExclusive(true);

  connect(langGroup, SIGNAL(triggered(QAction *)), this, SLOT(slot_SpracheWechselt(QAction *)));

  // format systems language
  //
  //QString defaultLocale = QLocale::system().name();       // e.g. "de_DE"
  //defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"
  //srachauswahl aus configdatei
  QString defaultLocale = sprachauswahl;
  qDebug() << "defaultLocale" << defaultLocale;

  m_langPath = QApplication::applicationDirPath();
  m_langPath.append("/languages");
  QDir dir(m_langPath);
  QStringList fileNames = dir.entryList(QStringList("kb_*.qm"));

  for (int i = 0; i < fileNames.size(); ++i)
  {
    // get locale extracted by filename
    QString locale;
    locale = fileNames[i];                  // "TranslationExample_de.qm"
    locale.truncate(locale.lastIndexOf('.'));   // "TranslationExample_de"
    locale.remove(0, locale.indexOf('_') + 1);   // "de"

    QString lang = QLocale::languageToString(QLocale(locale).language());
    QIcon ico(QString("%1/%2.png").arg(m_langPath).arg(locale));

    QAction *action = new QAction(ico, lang, this);
    action->setCheckable(true);
    action->setData(locale);

    sprachMenu->addAction(action);
    langGroup->addAction(action);

    // set default translators and language checked
    if (defaultLocale == locale)
    {
      action->setChecked(true);
    }
  }
  loadSprache(defaultLocale);
}

void MainWindowImpl::createMenus()
{
  //Menü geladener Sud
  geladenerSudMenu = menuBar()->addMenu(trUtf8("&geladener Sud"));
  geladenerSudMenu->addAction(saveAct);
  separatorAct = geladenerSudMenu->addSeparator();
  geladenerSudMenu->addAction(schuettungProzent);
  geladenerSudMenu->addAction(berIBU);
  geladenerSudMenu->addAction(EntsperreEingabefelder);
  geladenerSudMenu->addAction(ResetBierGebraut);
  geladenerSudMenu->addAction(ResetAbgefuellt);
  geladenerSudMenu->addAction(ResetVerbraucht);
  geladenerSudMenu->addAction(ResetZugabestatus);
  separatorAct = geladenerSudMenu->addSeparator();
  for (int i = 0; i < MaxRecentFiles; ++i)
    geladenerSudMenu->addAction(recentFileActs[i]);

  //Menü Extras
  extrasMenu = menuBar()->addMenu(trUtf8("&Extras"));
  extrasMenu->addAction(einstellungen);

  //Sprachauswahl Menü
  ErstelleSprachMenu();
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

void MainWindowImpl::LeseSuddatenDB(bool aktivateTab)
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
            connect(rast, SIGNAL( sig_nachOben(int) ), this, SLOT( on_pushButton_RastNachOben(int) ));
            connect(rast, SIGNAL( sig_nachUnten(int) ), this, SLOT( on_pushButton_RastNachUnten(int) ));
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
          //Ergebnisswidget ersetellen
          doubleEditLineImpl* berEwz = new doubleEditLineImpl(this);
          berEwz -> setAttribute(Qt::WA_DeleteOnClose);

          ewz -> ergWidget = berEwz;
          //Funktionen verknüpfen das das objekt die Daten holen kann
          connect(ewz, SIGNAL( sig_vorClose(int) ), this, SLOT( slot_ewzClose(int) ));
          connect(ewz, SIGNAL( sig_getEwzTyp(QString) ), this, SLOT( slot_getEwzTyp(QString) ));
          connect(ewz, SIGNAL( sig_getEwzEinheit(QString) ), this, SLOT( slot_getEwzEinheit(QString) ));
          connect(ewz, SIGNAL( sig_getEwzAusbeute(QString) ), this, SLOT( slot_getEwzAusbeute(QString) ));
          connect(ewz, SIGNAL( sig_getEwzFarbe(QString) ), this, SLOT( slot_getEwzFarbe(QString) ));
          connect(ewz, SIGNAL( sig_getEwzPreis(QString) ), this, SLOT( slot_getEwzPreis(QString) ));
          connect(ewz, SIGNAL( sig_getEwzPreisHopfen(QString) ), this, SLOT( slot_getEwzPreisHopfen(QString) ));
          connect(ewz, SIGNAL( sig_zugeben(QString, int, double) ), this, SLOT( slot_EwzZugegeben(QString, int, double) ));
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
          ewz -> setBierWurdeGebraut(BierWurdeGebraut);
          ewz -> setBierWurdeAbgefuellt(BierWurdeAbgefuellt);
          connect(ewz, SIGNAL( sig_Aenderung() ), this, SLOT( slot_EwzAenderung() ));

          //Ergebnisswidget dem Layout zuordnen
          verticalLayout_BerWeitereZutaten -> addWidget(berEwz);
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

      //Tabs enablen
      tab_Rezept -> setDisabled(false);
      tab_Braudaten -> setDisabled(false);

      //Gespeichertes Tab wiederherstellen
      //FeldNr = query_sud.record().indexOf("AktivTab");
      //tabWidged -> setCurrentIndex(query_sud.value(FeldNr).toInt());
      //Gespeichertes Tab im Gärverlauf wiederherstellen

      //Tab nach Status Einblenden
      if (aktivateTab) {
        // Wenn Bier Abgefüllt wurde Zusammenfassung anzeigen
        if (BierWurdeAbgefuellt) {
          tabWidged -> setCurrentIndex(TAB_ZUSAMMENFASSUNG);
        }
        //Wenn Bier gebraut wurde aber noch nicht abgefüllt ist den Gärverlauf anzeigen
        else if (BierWurdeGebraut) {
          tabWidged -> setCurrentIndex(TAB_GAERVERLAUF);
        }
        //Bei einem noch nicht gebrauten Sud wird das Rezept eingeblendet
        else {
          tabWidged -> setCurrentIndex(TAB_REZEPT);
        }
      }

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
    if (comboBox_AuswahlBrauanlage->count() <= aba) {
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
      comboBox_AuswahlHefe -> addItem(tableWidget_Hefe -> item(i,0) -> text() );
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


void MainWindowImpl::slot_dateChanged(QDate)
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

void MainWindowImpl::slot_EwzAenderungRohstoffe()
{
  if (Gestartet) {
    //Da sich die Rohstoffe für die Weiteren Zutaten geändert haben
    //müssen die Zutatenobjekte ihre Daten neu einholen
    for (int i=0; i < list_EwZutat.count(); i++){
      list_EwZutat[i] -> WerteNeuAusRohstoffeHolen();
    }
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
  double Alphaprozent[list_Hopfengaben.count()];
  QString s;
  //Pellets
  bool Pellets[list_Hopfengaben.count()];
  //Vorderwürzehopfung
  bool vwh[list_Hopfengaben.count()];
  //Kochzeiten
  int Kochzeiten[list_Hopfengaben.count()];
  //Mengen
  double MengenProzent[list_Hopfengaben.count()];
  //Ausbeute der einzelnen Hopfengaben
  double Ausbeute[list_Hopfengaben.count()];
  //Berechnung anschmeissen
  double HopfenMengen[list_Hopfengaben.count()];
  //IBU Anteil der einzelnen Gaben
  double IBUAnteil[list_Hopfengaben.count()];
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
      if (s == tableWidget_Hopfen -> item(i,0) -> text()){
        QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,1);
        Alphaprozent[o] = spinBox->value();
        Pellets[o] = tableWidget_Hopfen -> item(i,4) -> checkState();
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
}


void MainWindowImpl::ErstelleSpickzettelV2()
{
  //Überschrift vom Tab setzten
  tabWidged -> setTabText(5,trUtf8("Spickzettel"));
  // Seitenkopf
  QString seite, kopf, ende, style;
  QString s = "";

  kopf = "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'> <html><head><meta name='qrichtext' content='1' />";
  style = "<style type='text/css'>";
  //Style für Überschrift H1
  style += "p.h1{color:black;font-size:20pt;padding:0px;margin:0px;}";
  //Style für P
  style += "p{color:black;font-size:10pt;padding:0px;margin:0px;}";
  //Style für Variable
  style += ".value{color:blue;margin-left:5px;margin-right:5px;}";
  //Style für div Kommentar
  style += ".koment{}";
  style += "</style>";
  kopf += style;
  kopf += "</head><body align='center' style=' font-family:Ubuntu,Arial; font-size:10pt;font-style:normal;background-color:#fff;'>";
  seite = kopf;

  //Seiteninhalt
  //Name
  s += "<p class='h1'><b>" + lineEdit_Sudname -> text() + "</b></p>";

  seite += s;
  //Seitenende
  ende = "</body></html>";
  seite += ende;

  //textEdit -> setPlainText(seite);

  webView_Zusammenfassung -> setRenderHint(QPainter::TextAntialiasing, true);
  webView_Zusammenfassung -> setRenderHint(QPainter::SmoothPixmapTransform, true);
  webView_Zusammenfassung -> setRenderHint(QPainter::HighQualityAntialiasing, true);
  webView_Zusammenfassung -> setHtml(seite,QUrl::fromLocalFile(QCoreApplication::applicationDirPath()+"/"));
}


void MainWindowImpl::ErstelleSpickzettel()
{
  //Überschrift vom Tab setzten
  tabWidged -> setTabText(5,trUtf8("Spickzettel"));
  // Seitenkopf
  QString seite, kopf, ende, style;

  kopf = "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'> <html><head><meta name='qrichtext' content='1' />";
  style = "<style type='text/css'>";
  //Style für P
  style += "p{color:black;font-size:10pt;padding:0px;margin:0px;}";
  //Style für Variable
  style += ".value{color:blue;margin-left:5px;margin-right:5px;}";
  //Style für div Kommentar
  style += ".koment{}";
  //Style für ul
  style += "ul{color:black;font-size:10pt;}";
  //Style für Überschrift h1
  style += "p.h1{color:black;font-size:12pt;}";
  //Style für Überschrift h2
  style += "p.h2{color:black;font-size:11pt;margin-bottom:5px;}";
  style += "p.version{color:#999999;font-size:11pt;margin-top:5px;}";
  //Style für Div Box ohne Rahmen
  style += "div.r{border:0px solid #dddddd; border-radius: 10px; padding:5px;background-color:#dddddd;}";
  //Style für Div Box mit Rahmen
  style += "div.rm{border:2px solid #dddddd; border-radius: 10px; padding:5px;background-color:#ffffff;}";
  //Style für Hinweis
  style += ".hinweis{color:#d47209;}";
  //Style für Tabelle
  style += "td{padding:2px;margin:0px;}";
  style += "td.r{padding:2px;margin:0px;border-bottom-color:#dddddd;border-bottom-style:solid;border-width:1px;}";
  //Style für Hinweis Wert in Brau und Gärdaten eintragen
  style += "td.we{background-color: #eba328;}";
  style += "tr{padding:0px;margin:0px;}";
  style += "</style>";
  kopf += style;
  kopf += "</head><body align='center' style=' font-family:Ubuntu,Arial; font-size:10pt;font-style:normal;background-color:#fff;'>";
  seite = kopf;

  QString s = "";

  //Tabelle für Bild und Zutaten
  s += "<div class='' width='99%' style='' align='center'>";

  s += "<table width='50%' summary='testtabelle' border='0' cellspacing='5'>";
  s += "<tr >";
  s += "<td valign='middle' style=''>";
  //Solldaten des Rezeptes
  s += "<div class='rm' style='margin-top:10px;margin-bottom:5px;' align='center'>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Name
  s += "<tr style=''>";
  s += "<td>";
  s += "<p class='h1'><b>" + lineEdit_Sudname -> text() + "</b></p>";
  s += "</td>";
  s += "</tr>";
  //Menge
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Menge") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(spinBox_Menge -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Stammwürze
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Stammwürze") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(spinBox_SW -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("°P") + "</p>";
  s += "</td>";
  s += "</tr>";
  //High Gravity Faktor
  if (spinBox_High_Gravity->value() > 0) {
    s += "<tr style=''>";
    s += "<td>";
    s += "<p>" + trUtf8("High Gravity Faktor") + "</p>";
    s += "</td>";
    s += "<td align='right'>";
    s += "<p class='value'>" + QString::number(spinBox_High_Gravity -> value()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("%") + "</p>";
    s += "</td>";
    s += "</tr>";
  }
  //Bittere
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Bittere") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(spinBox_IBU -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("IBU") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Nachisomerisierungs-zeit
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Nachisomerisierungs-Zeit") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(spinBox_NachisomerisierungsZeit -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("min") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Farbe
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Farbe") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(doubleSpinBox_EBC -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("EBC") + "</p>";
  s += "</td>";
  s += "</tr>";
  //CO2 Gehalt
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("CO2 Gehalt") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(doubleSpinBox_CO2 -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("g/Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Brauanlage
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Brauanlage") + "</p>";
  s += "</td>";
  s += "<td colspan=2 align='right'>";
  s += "<p class='value'>" + comboBox_AuswahlBrauanlage->currentText() + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "</tbody></table>";
  s += "</div>";
  s += "</td>";
  s += "</tr>";
  s += "</table>";

  s += "<table width='90%' summary='tabelle' border='0' cellspacing='3'>";
  s += "<tr style=''>";
  s += "<td valign='bottom' style=''>";
  //Schüttung
  s += "<div class='rm' style='margin:0px;margin-bottom:5px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/getreide_300.png' alt='Getreide' width='300px' border=0>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Alle Malzgaben
  double fehlprozent = 0;
  if (list_Malzgaben.count()>0) {
    fehlprozent = list_Malzgaben[0]->getFehlProzent();
  }
  if (fehlprozent == 0) {
    for (int i=0; i < list_Malzgaben.count(); i++){
      s += "<tr style=''>";
      s += "<td>";
      s += "<p>" + list_Malzgaben[i]->getName() + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p class='value'>" + QString::number(list_Malzgaben[i]->getErgMenge()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("Kg") + "</p>";
      s += "</td>";
      s += "<td align='right'>";
      s += "<p class='value'>" + QString::number(list_Malzgaben[i]->getMengeProzent()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("%") + "</p>";
      s += "</td>";
      s += "<td align='right'>";
      s += "<p class='value'>" + QString::number(list_Malzgaben[i]->getFarbe()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("EBC") + "</p>";
      s += "</td>";
      s += "</tr>";
    }
  }
  //Wenn die Porzentuale aufteilung der schüttung nicht stimmt
  else {
    s += "<div class='hinweis'>" + trUtf8("Die einzelnen Schüttungen konnten nicht richtig berechnet werden da die aufteilung nicht 100% entspricht!")+"</div>";
  }
  //Gesamt
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Gesamt") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value' style='font-weight:bold;'>" + QString::number(doubleSpinBox_S_Gesammt -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Kg") + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "</tbody></table>";
  s += "</div>";
  s += "</td>";


  s += "<td valign='top' style=''>";
  //bild mit entsprechender Bierfarbe
  QColor farbe;
  farbe = Berechnungen.GetFarbwert(doubleSpinBox_EBC -> value());
  s += "<div class='' style='background-color:" + farbe.name() + ";width:210px;height:210px;margin:0px;padding:0px;'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/global/bier_420x420.png' alt='Bierfarbe' width='210px' height='210px' border=0>";
  s += "</div>";
  s += "</td>";
  s += "<td valign='top' style=''>";
  //Hopfen
  s += "<div class='rm' style='margin:0px;margin-bottom:5px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/hopfen_100.png' alt='Hopfen' width='100px' border=0>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Alle Hopfengaben
  fehlprozent = 0;
  if (list_Hopfengaben.count()>0) {
    fehlprozent = list_Hopfengaben[0]->getFehlProzent();
  }
  if (fehlprozent == 0) {
    for (int i=0; i < list_Hopfengaben.count(); i++){
      s += "<tr style=''>";
      s += "<td>";
      if (list_Hopfengaben[i]->getVWH())
        s += "<p>" + trUtf8("VWH ") + list_Hopfengaben[i]->getErgebnistext() + "</p>";
      else
        s += "<p>" + list_Hopfengaben[i]->getErgebnistext() + "</p>";
      s += "</td>";
      s += "<td align='right'>";
      s += "<p class='value'>" + QString::number(list_Hopfengaben[i]->getErgMenge()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("g") + "</p>";
      s += "</td>";
      s += "<td align='right'>";
      s += "<p class='value'>" + QString::number(list_Hopfengaben[i]->getKochzeit()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("min") + "</p>";
      s += "</td>";
      s += "</tr>";
    }
  }
  //Wenn die Porzentuale aufteilung der schüttung nicht stimmt
  else {
    s += "<div class='hinweis'>" + trUtf8("Die einzelnen Hopfenhaben konnten nicht richtig berechnet werden da die aufteilung nicht 100% entspricht!")+"</div>";
  }
  //Hopfengaben in den Weiteren Zutaten
  for (int i=0; i < list_EwZutat.count(); i++){
    //Nur Hopfengaben
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Hopfen){
      s += "<tr style=''>";
      s += "<td>";
      s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
      s += "</td>";
      s += "<td align='right'>";
      s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("g") + "</p>";
      s += "</td>";
      s += "<td align='right' colspan='2'>";
      if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
        s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
      else
        s += "<p class='value'>" + trUtf8("Anstellen") + "</p>";
      s += "</td>";
      s += "</tr>";
    }
  }
  s += "</tbody></table>";
  s += "</div>";
  s += "</td>";
  s += "</tr>";
  s += "</table>";

  s += "<table width='70%' summary='tabelle' border='0' cellspacing='3'>";
  s += "<tr style=''>";
  s += "<td valign='top' style=''>";
  //Wasser
  s += "<div class='rm' style='margin:0px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/wasser_100x107.png' alt='Hefe' width='50px' border=0>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Hauptguss
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Hauptguss") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(doubleSpinBox_WHauptguss -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "<tr>";
  s += "<td>";
  s += "<p>" + trUtf8("Milchsäure (80%)") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(SpinBox_waMilchsaeureHG_ml -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("ml") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Nachguss
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Nachguss") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(doubleSpinBox_WNachguss -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Milchsäure (80%)") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(SpinBox_waMilchsaeureNG_ml -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("ml") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Gesammt
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + label_37 -> text() + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value' style='font-weight:bold;'>" + QString::number(doubleSpinBox_W_Gesammt -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "</tbody></table>";
  s += "</div>";
  s += "</td>";
  //Hefe
  s += "<td valign='top' style=''>";
  s += "<div class='rm' style='margin:0px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/hefe_50.png' alt='Hefe' width='50px' border=0>";
  s += "<p>" + comboBox_AuswahlHefe -> currentText() + "</p>";
  QString sEinheiten;
  sEinheiten = trUtf8("Anzahl Einheiten:") + " <span class='value'>" + QString::number(spinBox_AnzahlHefeEinheiten->value()) +"</span>";
  //Verpackungsgrösse aus den Rohstoffdaten auslesen
  int AnzahlHefeEintraege = tableWidget_Hefe -> rowCount();
  QString HefeName = comboBox_AuswahlHefe -> currentText();
  QString verpMenge;
  if (HefeName != ""){
    //Würzemenge auslesen
    for (int i=0; i < AnzahlHefeEintraege; i++){
      //wenn Eintrag übereinstimmt
      if (tableWidget_Hefe -> item(i,0) -> text() == HefeName){
        verpMenge = tableWidget_Hefe -> item(i,5) -> text();
      }
    }
  }
  if (!verpMenge.isEmpty()) {
    sEinheiten += trUtf8(" zu ") + verpMenge;
  }
  s += "<p>" + sEinheiten +"</p>";
  s += "</div>";
  s += "</td>";
  s += "</tr>";

  s += "</table>";
  //s += "</div>";
  //s += "</div>";

  //Tabelle Weitere Zutaten
  s += "<table width='80%' summary='testtabelle' border='0' cellspacing='5'>";
  s += "<tr >";

  //Honig
  bool HonigVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Honig){
      HonigVorhanden = true;
    }
  }
  if (HonigVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_0_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Honig){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("Kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochbeginn") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  //Zucker
  bool ZuckerVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Zucker){
      ZuckerVorhanden = true;
    }
  }
  if (ZuckerVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_1_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Zucker){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("Kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochbeginn") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  //Gewürz
  bool GewuerzVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Gewuerz){
      GewuerzVorhanden = true;
    }
  }
  if (GewuerzVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_2_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Gewuerz){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("Kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochen") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  //Frucht
  bool FruchtVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Frucht){
      FruchtVorhanden = true;
    }
  }
  if (FruchtVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_3_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Frucht){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("Kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochen") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  //Sonstiges
  bool SonstigesVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Sonstiges){
      SonstigesVorhanden = true;
    }
  }
  if (SonstigesVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_4_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Sonstiges){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("Kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochen") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  s += "</tr >";
  s += "</table>";

  //s += "<div class='r' width='99%' style='' align='center'>";
  //Kommentar
  if (textEdit_Kommentar -> toPlainText() != "") {
    s += "<div class='rm' style='margin-top:10px;width:90%'>";
    s += textEdit_Kommentar -> toHtml();
    s += "</div>";
  }


  //Geräte und Zubehör
  //-------------------------------------------------------------
  QSqlQuery query;

  int id = getBrauanlagenIDRezept();
  QString sql = "SELECT Bezeichnung FROM Geraete WHERE AusruestungAnlagenID = " + QString::number(id);
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  else {
    int zaehler = 0;
    if (query.first()) {
      s += "<div align='center' class='rm' style='margin-top:10px;width:90%'>";
      s += "<p><b>";
      s += trUtf8("benötigte Gerätschaften");
      s += "</b></p>";
      s += "<table cellspacing=0 border=0><tbody>";
      if (zaehler == 0)
        s += "<tr>";
      s += "<td align=center><p>";
      s += query.value(0).toString();
      s += "</p></td>";
      if (zaehler == 2)
        s += "</tr>";
      zaehler ++;
      if (zaehler == 3)
        zaehler = 0;
      while (query.next()){
        if (zaehler == 0)
          s += "<tr>";
        s += "<td align=center><p>";
        s += query.value(0).toString();
        s += "</p></td>";
        if (zaehler == 2)
          s += "</tr>";
        zaehler ++;
        if (zaehler == 3)
          zaehler = 0;
      }
      if (zaehler != 0)
        s += "</tr>";
      s += "</tbody></table>";
      s += "</div>";
    }
  }

  //Brauablauf
  //-------------------------------------------------------------
  s += "<div align='center' class='rm' style='margin-top:10px;width:90%'>";
  s += "<p><b>";
  s += trUtf8("Maischen");
  s += "</b></p>";
  s += "<table cellspacing=0 border=0 width='90%'><tbody>";

  //Einmaischen
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Einmaischen: ");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += GetWertString(doubleSpinBox_WHauptguss -> value()) + trUtf8(" Liter Wasser auf ");
  s += GetWertString(spinBox_EinmaischenTemp -> value()) + trUtf8("°C erhitzen und Malzschüttung zugeben ");
  s += "</p></td>";
  s += "</tr>";

  //Rasten in Spickzettel eintragen
  for (int i=0; i < list_Rasten.count(); i++){
    s += "<tr>";
    s += "<td class='r' align=center><p>";
    s += list_Rasten[i]->getRastName();
    s += "</p></td>";
    s += "<td class='r' align=center><p>";
    s += trUtf8("Maische auf ");
    s += GetWertString(list_Rasten[i]->getRastTemp()) + trUtf8("°C erhitzen und ");
    s += GetWertString(list_Rasten[i]->getRastDauer()) + trUtf8(" min Rast einlegen.");
    s += "</p></td>";
    s += "</tr>";
  }

  //Weitere Zutaten beim Maischen
  bool MaischenVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen){
      MaischenVorhanden = true;
    }
  }
  if (MaischenVorhanden) {
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen){
        s += "<tr>";
        s += "<td class='r' align=center><p>";
        s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_"
            + QString::number(list_EwZutat[i] -> getTyp()) + "_50.png' alt='Honig' width='30px' border=0>";
        s += "</p></td>";
        s += "<td class='r' align=center><p>";
        s += list_EwZutat[i] -> getName() + " ";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<span class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</span>";
        else
          s += "<span class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</span>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<span>" + trUtf8(" Kg") + "</span>";
        else
          s += "<span>" + trUtf8(" g") + "</span>";
        s += "<br>";
        s += list_EwZutat[i] -> getBemerkung();
        s += "</p></td>";
        s += "</tr>";
      }
    }
  }


  //Läutereimer
  //s += "<tr>";
  //s += "<td align=center colspan=2><p>";
  //s += trUtf8("Wärend der Rast Läutereimer herichten");
  //s += "</p></td>";
  //s += "</tr>";

  //Jodprobe
  //s += "<tr>";
  //s += "<td align=center><p>";
  //s += trUtf8("Jodprobe");
  //s += "</p></td>";
  //s += "<td align=center><p>";
  //s += trUtf8("Überprüfen ob Jodprobe ohne verfärbung ist, ansonsten rast etwas verlängern");
  //s += "</p></td>";
  //s += "</tr>";

  s += "</tbody></table>";
  s += "</div>";

  s += "<div align='center' class='rm' style='margin-top:10px;width:90%'>";
  s += "<p><b>";
  s += trUtf8("Läutern");
  s += "</b></p>";
  s += "<table cellspacing=0 border=0 width='90%'><tbody>";

  //Maische in den Läutereimer
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Läutern vorbereiten");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Maische in den Läutereimer schöpfen");
  s += "</p></td>";
  s += "</tr>";

  //Nachguss erhitzen
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Nachguss vorbereiten: ");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += GetWertString(doubleSpinBox_WNachguss -> value()) + trUtf8(" Liter Wasser auf ") + GetWertString(78) + trUtf8("°C erhitzen");
  s += "</p></td>";
  s += "</tr>";

  //Vorderwürze Klarlaufen lassen
  s += "<tr>";
  s += "<td class='r' align=center colspan=2><p>";
  s += trUtf8("Nach etwa 10 - 20 min Wartezeit solange Vorderwürze ablaufen lassen und wieder zurück in den Läutereimer schütten, bis Würze klar läuft.");
  s += "</p></td>";
  s += "</tr>";

  //Läutern
  s += "<tr>";
  s += "<td class='' align=center><p>";
  s += trUtf8("Läutern");
  s += "</p></td>";
  s += "<td class='' align=center><p>";
  s += trUtf8("Würze langsam ablaufen lassen (Treber darf nicht trockenlaufen!!), bevor Treber trockenläuft immer etwas (ca. 2-3 Liter) Nachguss über Schaumlöffel nachgießen.");
  s += "</p></td>";
  s += "</tr>";


  s += "</tbody></table>";
  s += "</div>";

  s += "<div align='center' class='rm' style='margin-top:10px;width:90%'>";
  s += "<p><b>";
  s += trUtf8("Würze kochen");
  s += "</b></p>";
  s += "<table cellspacing=0 border=0 width='90%'><tbody>";

  //Vorderwürzehopfung
  for (int i=0; i < list_Hopfengaben.count(); i++){
    if (list_Hopfengaben[i]->getVWH()){
      s += "<tr>";
      s += "<td class='r' align=center><p>";
      s += trUtf8("Vorderwürzehopfung: ");
      s += "</p></td>";
      s += "<td class='r' align=center><p>";
      if ((spinBox_Gesammtkochdauer -> value() - list_Hopfengaben[i]->getKochzeit()) > 0){
        s += trUtf8("Nach ") + GetWertString(spinBox_Gesammtkochdauer -> value()
                                             - list_Hopfengaben[i]->getKochzeit()) + trUtf8(" min ");
      }
      s += GetWertString(list_Hopfengaben[i]->getErgMenge()) + trUtf8("g ");
      s += list_Hopfengaben[i]->getName() + trUtf8(" Hopfen vorlegen");
      s += "</p></td>";
      s += "</tr>";
    }
  }

  //Würze zum Kochen bringen
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Würze zum Kochen bringen");
  s += "</p></td>";
  s += "</tr>";

  //Gesammtkochdauer
  int kd = 0;
  //kleinste Kochzeit
  int kkd = spinBox_Gesammtkochdauer -> value();

  //Hopfengaben nach Kochdauer absteigend sortieren
  for (int i=0; i < list_Hopfengaben.count(); i++){
    for (int o=0; o < list_Hopfengaben.count()-1; o++){
      kd = list_Hopfengaben[o]->getKochzeit();
      if (kd < list_Hopfengaben[o+1]->getKochzeit()) {
        list_Hopfengaben.move(o,o+1);
      }
    }
  }
  kd = spinBox_Gesammtkochdauer -> value();
  for (int i=0; i < list_Hopfengaben.count(); i++){
    if (!list_Hopfengaben[i]->getVWH()){
      s += "<tr>";
      s += "<td class='r' align=center><p>";
      s += QString::number(i+1) + trUtf8(". Hopfengabe: ");
      s += "</p></td>";
      s += "<td class='r' align=center><p>";
      if ((spinBox_Gesammtkochdauer -> value() - list_Hopfengaben[i]->getKochzeit()) > 0){
        if (i == 0) {
          kd -= list_Hopfengaben[i]->getKochzeit();
        }
        else {
          kd = list_Hopfengaben[i-1]->getKochzeit() - list_Hopfengaben[i]->getKochzeit();
        }
        s += trUtf8("Nach ") + QString::number(kd) + trUtf8(" min ");
      }
      s += GetWertString(list_Hopfengaben[i]->getErgMenge()) + trUtf8("g ");
      s += list_Hopfengaben[i]->getName() + trUtf8(" Hopfen untermischen");
      s += "</p></td>";
      s += "</tr>";
      if (kkd > list_Hopfengaben[i]->getKochzeit()) {
        kkd = list_Hopfengaben[i]->getKochzeit();
      }
    }
  }
  kd = spinBox_Gesammtkochdauer -> value() - kkd;

  //Fertigkochen
  if (spinBox_Gesammtkochdauer -> value() - kd > 0) {
    s += "<tr>";
    s += "<td class='r' align=center><p>";
    s += trUtf8("Kochen");
    s += "</p></td>";
    s += "<td class='r' align=center><p>";
    s += trUtf8("Noch ") + GetWertString(spinBox_Gesammtkochdauer -> value() - kd) + trUtf8("min weiter kochen");;
    s += "</p></td>";
    s += "</tr>";
  }

  //Weitere Zutaten beim Kochen
  bool KochenVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn){
      KochenVorhanden = true;
    }
  }
  if (KochenVorhanden) {
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn){
        s += "<tr>";
        s += "<td class='r' align=center><p>";
        s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_"
            + QString::number(list_EwZutat[i] -> getTyp()) + "_50.png' alt='Honig' width='30px' border=0>";
        s += "</p></td>";
        s += "<td class='r' align=center><p>";
        s += list_EwZutat[i] -> getName() + " ";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<span class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</span>";
        else
          s += "<span class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</span>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<span>" + trUtf8(" Kg") + "</span>";
        else
          s += "<span>" + trUtf8(" g") + "</span>";
        s += "<br>";
        s += list_EwZutat[i] -> getBemerkung();
        s += "</p></td>";
        s += "</tr>";
      }
    }
  }

  //Menge Eintragen Würzemenge zur Berechnung der Sudhausausbeute
  s += "<tr>";
  s += "<td class='we' align=center><p>";
  s += trUtf8("Würzemenge eintragen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würzemenge ermitteln und im Tab (Brau & Gärdaten) in das Eingabefeld (Würzemenge vor dem Hopfenseihen) eintragen");
  s += "</p></td>";
  s += "</tr>";

  //Whirlpool
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Whirlpool");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Kocher abschalten, wenn thermische Bewegung abgeflaut ist einen Whirlpool erzeugen");
  s += "</p></td>";
  s += "</tr>";
  //Warten bis Trubkegel gebildet wurde
  s += "<tr>";
  s += "<td class='' align=center><p>";
  //s += trUtf8("");
  s += "</p></td>";
  s += "<td class='' align=center><p>";
  s += trUtf8("ca. 15 min warten bis sich Trubkegel gebildet hat");
  s += "</p></td>";
  s += "</tr>";

  s += "</tbody></table>";
  s += "</div>";

  s += "<div align='center' class='rm' style='margin-top:10px;width:90%'>";
  s += "<p><b>";
  s += trUtf8("Abseihen / Anstellen");
  s += "</b></p>";
  s += "<table cellspacing=0 border=0 width='90%'><tbody>";

  //Hopfenseihen
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Hopfenseihen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würze durch einen Filter in den Gärbehälter abfüllen");
  s += "</p></td>";
  s += "</tr>";

  //Würzemenge nach Kochende Eintragen
  s += "<tr>";
  s += "<td class='we' align=center><p>";
  s += trUtf8("Würzemenge Eintragen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würzemenge ermitteln und im Tab (Brau & Gärdaten) in das Eingabefeld (Würzemenge nach dem Hopfenseihen) eintragen");
  s += "</p></td>";
  s += "</tr>";

  //Stammwürze nach Kochende Eintragen
  s += "<tr>";
  s += "<td class='we' align=center><p>";
  s += trUtf8("Stammwürze Eintragen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Stammwürze messen und im Tab (Brau & Gärdaten) in das Eingabefeld (Stammwürze nach dem Hopfenseihen) eintragen");
  s += "</p></td>";
  s += "</tr>";

  //Verdünnen
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Verdünnen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Wenn gewünscht die Würze mit der berechneten Menge Wasser auf Sollstammwürze verdünnen");
  s += "</p></td>";
  s += "</tr>";

  //Speise abfüllen
  if (spinBox_Speisemenge -> value() > 0.04){
    s += "<tr>";
    s += "<td class='r' align=center><p>";
    s += trUtf8("Speise abfüllen");
    s += "</p></td>";
    s += "<td class='r' align=center><p>";
    s += QString::number(spinBox_Speisemenge -> value()) + trUtf8(" Liter Speise abfüllen");
    s += "</p></td>";
    s += "</tr>";
  }

  //Abkühlen
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Abkühlen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würze auf Anstelltemperatur bringen.");
  s += "</p></td>";
  s += "</tr>";

  //Würzemenge beim Anstellen Eintragen
  s += "<tr>";
  s += "<td class='we' align=center><p>";
  s += trUtf8("Würzemenge Eintragen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würzemenge ermitteln und im Tab (Brau & Gärdaten) in das Eingabefeld (Würzemenge beim Anstellen) eintragen");
  s += "</p></td>";
  s += "</tr>";

  //Stammwürze beim Anstellen Eintragen
  s += "<tr>";
  s += "<td class='we' align=center><p>";
  s += trUtf8("Stammwürze Eintragen");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Stammwürze messen und im Tab (Brau & Gärdaten) in das Eingabefeld (Stammwürze beim Anstellen) eintragen");
  s += "</p></td>";
  s += "</tr>";

  //Hefe zugeben
  s += "<tr>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Hefe zugeben");
  s += "</p></td>";
  s += "<td class='r' align=center><p>";
  s += trUtf8("Die Würze mit dem Rührer gut durchlüften und die Hefe unterrühren.");
  s += "</p></td>";
  s += "</tr>";

  //feddich
  s += "<tr>";
  s += "<td class='' align=center colspan=2><p>";
  s += trUtf8("Sud als gebraut markieren, zurücklehnen und ein Bier genießen (am besten ein Selbstgebrautes)");
  s += "</p></td>";
  s += "</tr>";

  s += "</tbody></table>";
  s += "</div>";

  //Weitere Zutaten bei der Gärung
  bool GaerungVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung){
      GaerungVorhanden = true;
    }
  }
  if (GaerungVorhanden) {
    s += "<div align='center' class='rm' style='margin-top:10px;width:90%'>";
    s += "<p><b>";
    s += trUtf8("Bei der Gärung");
    s += "</b></p>";
    s += "<table cellspacing=0 border=0 width='90%'><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung){
        s += "<tr>";
        s += "<td class='r' align=center><p>";
        s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_"
            + QString::number(list_EwZutat[i] -> getTyp()) + "_50.png' alt='Honig' width='30px' border=0>";
        s += "</p></td>";
        s += "<td class='r' align=center><p>";
        s += list_EwZutat[i] -> getName() + " ";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<span class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</span>";
        else
          s += "<span class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</span>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<span>" + trUtf8(" Kg") + "</span>";
        else
          s += "<span>" + trUtf8(" g") + "</span>";
        s += "<br>";
        s += list_EwZutat[i] -> getBemerkung();
        s += "</p></td>";
        s += "</tr>";
      }
    }
    s += "</tbody></table>";
    s += "</div>";
  }

  s += "<div><p class='version'>"APP_NAME" v";
  s += VERSION;
  s += "</p></div>";

  seite += s;
  //Seitenende
  ende = "</body></html>";
  seite += ende;

  //textEdit -> setPlainText(seite);

  webView_Zusammenfassung -> setRenderHint(QPainter::TextAntialiasing, true);
  webView_Zusammenfassung -> setRenderHint(QPainter::SmoothPixmapTransform, true);
  webView_Zusammenfassung -> setRenderHint(QPainter::HighQualityAntialiasing, true);
  webView_Zusammenfassung -> setHtml(seite,QUrl::fromLocalFile(QCoreApplication::applicationDirPath()+"/"));

}


void MainWindowImpl::ErstelleTabSpickzettel()
{
  if (BierWurdeGebraut){
    ErstelleZusammenfassung();
  }
  else {
    ErstelleSpickzettel();
  }
}


void MainWindowImpl::LeseGeraeteliste()
{
  QSettings daten("Geraete",QSettings::IniFormat);

  daten.beginGroup("Geraete");
  QString s;
  int i=0;
  s = daten.value("G" + QString::number(i+1)).toString();
  while (s != ""){
    QTableWidgetItem *newItem1 = new QTableWidgetItem("");
    tableWidget_Geraete -> setRowCount(tableWidget_Geraete -> rowCount()+1);
    //Beschreibung
    newItem1 -> setText(s);
    tableWidget_Geraete -> setItem(i, 0, newItem1);
    //Nächste Beschreibung aus Datei lesen
    i++;
    s = daten.value("G" + QString::number(i+1)).toString();
  }
  daten.endGroup();
  tableWidget_Geraete -> horizontalHeader() -> setSectionResizeMode(QHeaderView::ResizeToContents);
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
    tableWidget_Geraete -> horizontalHeader() -> setSectionResizeMode(QHeaderView::ResizeToContents);
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


void MainWindowImpl::slot_print()
{
  //allen nochmal durchrechnen
  BerAlles();
  //Zusammenfassung/Spickzettel neue erstellen
  ErstelleTabSpickzettel();

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  settings.beginGroup("PDF");

  //Zoomfaktor einlesen
  double zoom;
  QString s;
  if (BierWurdeGebraut){
    s = settings.value("zoomZusammenfassung").toString();
    if (s == ""){
      zoom = 1;
      settings.setValue("zoomZusammenfassung",zoom);
    }
    else
      zoom = s.toDouble();
  }
  else {
    s = settings.value("zoomSpickzettel").toString();
    if (s == ""){
      zoom = 1;
      settings.setValue("zoomSpickzettel",zoom);
    }
    else
      zoom = s.toDouble();
  }

  webView_Zusammenfassung->setTextSizeMultiplier(zoom);
  QPrinter printer(QPrinter::HighResolution);
  printer.setColorMode(QPrinter::Color);

  QPrintDialog *dialog = new QPrintDialog(&printer, this);
  dialog->setWindowTitle("Print");
  //if (webView_Zusammenfassung->hasSelection())
  //	dialog->addEnabledOption(QAbstractPrintDialog::PrintSelection);
  if (dialog->exec() != QDialog::Accepted){
    webView_Zusammenfassung->setZoomFactor(1);
    return;
  }
  //Drucken
  webView_Zusammenfassung->print(&printer);
  webView_Zusammenfassung->setTextSizeMultiplier(1);

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
  pushButton_SudAbgefuellt -> setDisabled(false);

  //Abfrage ob Rohstoffe vom Bestand abgezogen werden sollen
  QMessageBox msgBox;
  msgBox.setWindowTitle("kleine-frage");
  msgBox.setInformativeText("");
  msgBox.setText(trUtf8("Sollen die verwendeten Rohstoffe vom Bestand abgezogen werden?\n\nHinweis: Zutaten die bei der gärung hinzugegeben werden werden jetzt nicht verrechnet."));
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

  //lineEdit_Sudname -> setDisabled(status);
  spinBox_Menge -> setReadOnly(status);
  spinBox_Menge -> setButtonSymbols(bs);

  spinBox_SW -> setReadOnly(status);
  spinBox_SW -> setButtonSymbols(bs);

  spinBox_IBU -> setReadOnly(status);
  spinBox_IBU -> setButtonSymbols(bs);

  spinBox_EinmaischenTemp -> setReadOnly(status);
  spinBox_EinmaischenTemp -> setButtonSymbols(bs);

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

  pushButton_EWZ_Hinzufuegen -> setVisible(!status);

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

  //Tab Gärdaten ausblenden
  tab_Gaerverlauf->setEnabled(status);
  tabWidged->setTabEnabled(4,status);
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
      if (tableWidget_Malz -> item(o,0) -> text() == s){
        QDoubleSpinBox* dsbMenge = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(o,3);
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
      if (tableWidget_Hopfen -> item(o,0) -> text() == s){
        QDoubleSpinBox* dsbMenge = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(o,2);
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
    if (tableWidget_Hefe -> item(i,0) -> text() == s){
      QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,1);
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
    if (!list_EwZutat[i]->getZeitpunkt() == EWZ_Zeitpunkt_Gaerung) {
      //wenn Weiter Zutat kein Hopfen ist
      if (list_EwZutat[i]->getTyp() != EWZ_Typ_Hopfen){
        s = list_EwZutat[i]->getName();
        for (int o=0; o < tableWidget_WeitereZutaten -> rowCount(); o++){
          //wenn eintrag übereinstimmt
          if (tableWidget_WeitereZutaten -> item(o,0) -> text() == s){
            QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,1);
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
          if (tableWidget_Hopfen -> item(o,0) -> text() == s){
            QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(o,2);
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
      tabWidged -> setCurrentIndex(TAB_REZEPT);
      TabWidget_Zutaten -> setCurrentIndex(TAB_WeitereZutaten);
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

  pushButton_SudAbgefuellt -> setDisabled(status);

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

  //Tab Bewertung ausblenden
  tab_Bewertung->setEnabled(status);
  tabWidged->setTabEnabled(6,status);

}

void MainWindowImpl::SetDisabledVerbraucht(bool status)
{
  pushButton_SudVerbraucht -> setDisabled(status);
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
      if (tableWidget_Malz -> item(o,0) -> text() == s){
        gefunden = true;
        QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(o,3);
        ist = dsbMenge -> value();
        QDoubleSpinBox* dsbMaxprozent = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(o,2);
        maxprozent = dsbMaxprozent ->value();
      }
    }
    if (!gefunden){
      ist = 0;
    }
    //Anzeige Einfärben wenn Rohstoff nicht vorrätig wäre
    if (soll > ist){
      QString sf;
      sf = QString::number(soll - ist) + trUtf8(" Kg zu wenig vorhanden");
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
        list_Malzgaben[i] -> ergWidget -> setToolTip(trUtf8("Rest ") + QString::number(ist - soll) + "Kg");
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
      if ((list_EwZutat[o] -> getName() ==  s) && (list_EwZutat[o] -> getTyp() == 100)) {
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
      if (tableWidget_Hopfen -> item(o,0) -> text() == s){
        QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(o,2);
        ist = spinBox->value();
      }
    }
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
      if (tableWidget_Hefe -> item(i,0) -> text() == s){
        QSpinBox *spinBox = (QSpinBox*)tableWidget_Hefe -> cellWidget(i,1);
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
        if (tableWidget_Hopfen -> item(o,0) -> text() == s){
          QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(o,2);
          ist = spinBox->value();
        }
      }
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
        if (tableWidget_WeitereZutaten -> item(o,0) -> text() == s){
          gefunden = true;
          QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,1);
          ist = dsbMenge -> value();
        }
      }
      if (!gefunden){
        ist = 0;
      }
      if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
        soll = soll / 1000;
      //Anzeige Einfärben wenn Rohstoff nicht vorrätig wäre
      if (soll > ist){
        QString sf;
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg){
          sf = QString::number(soll - ist) + trUtf8(" Kg zu wenig vorhanden");
        }
        else{
          sf = QString::number(soll - ist) + trUtf8(" g zu wenig vorhanden");
        }
        list_EwZutat[i] -> ergWidget -> setToolTip(sf);
        list_EwZutat[i] -> ergWidget -> icon_achtung -> setVisible(true);
      }
      else {
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg){
          list_EwZutat[i] -> ergWidget -> setToolTip(trUtf8("Rest ") + QString::number(ist - soll) + "Kg");
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
  if (action){
    if (Aenderung){
      if (AbfrageSpeichern()){
      }
    }
    AktuelleSudID = action -> data().toInt();
    LadeSudDB(true);
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
    QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
    recentFileActs[i] -> setText(text);
    recentFileActs[i] -> setData(strippedID(files[i]));
    recentFileActs[i] -> setVisible(true);
    recentFileActs[i] -> setStatusTip(strippedName(files[i]));
  }
  for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
    recentFileActs[j]->setVisible(false);

  separatorAct->setVisible(numRecentFiles > 0);
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
  BierWurdeGebraut = false;
  ErstelleZutatenlisten();
  FuelleRezeptComboAuswahlen();
  LeseSuddatenDB(aktivateTab);
  if (BierWurdeGebraut) {
    SetStatusGebraut(true);
  }
  else {
    SetStatusGebraut(false);
  }

  if (BierWurdeAbgefuellt) {
    SetDisabledAbgefuellt(true);
  }
  else {
    SetDisabledAbgefuellt(false);
    if (!BierWurdeGebraut)
      pushButton_SudAbgefuellt -> setDisabled(true);
  }

  if (BierWurdeVerbraucht || !BierWurdeAbgefuellt) {
    SetDisabledVerbraucht(true);
  }
  else {
    SetDisabledVerbraucht(false);
  }
  setRecentFile(AktuelleSudID);
  AmLaden = false;
  BerAlles();
  ErstelleTabSpickzettel();
  setAenderung(false);
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
  msgBox.setWindowTitle(trUtf8("Speichern?"));
  msgBox.setText(trUtf8("Die Daten wurden verändert."));
  msgBox.setInformativeText(trUtf8("Sollen die Änderungen gespeichert werden?"));
  msgBox.setIcon(QMessageBox::Question);
  //msgBox.setDefaultButton(QMessageBox::Save);
  QPushButton *SpeichernButton = msgBox.addButton(trUtf8("Speichern"), QMessageBox::ActionRole);
  msgBox.addButton(trUtf8("Verwerfen"), QMessageBox::ActionRole);

  msgBox.exec();

  //Änderungen speichern
  if (msgBox.clickedButton() == SpeichernButton){
    save();
    setAenderung(false);
    return true;
  }
  //Änderungen nicht speichern
  else{
    //für den fall das sich die Rohstoffe geändert haben müssen sie neu geladen werden
    // in der Zeit des neu einlesens der Rohstoffe merker gestartet zurücksetzten
    // das hat den effekt das bei einer änderung an den Tabellen nicht neu berechnet wird
    // was zu einem absturz fürhen würde
    Gestartet = false;
    DatenEinlesenDB();
    Gestartet = true;
    return false;
  }

}


void MainWindowImpl::AddHopfengabe(bool vwh, QString Name, int Zeit, double Menge, double erg_Menge, double Alpha, int Pellets)
{
  //Hopfen hinzufügen
  //Zutatenobjekt hinzufügen
  hopfengabe* hopfen = new hopfengabe(this);
  hopfen -> setAttribute(Qt::WA_DeleteOnClose);
  //Ergebnisswidget ersetellen
  doubleEditLineImpl* berHopfen = new doubleEditLineImpl(this);
  berHopfen -> setAttribute(Qt::WA_DeleteOnClose);
  berHopfen -> setVisible(false);
  berHopfen -> spinBox_Wert->setDecimals(2);
  berHopfen -> label_Einheit->setText("g");

  //Zutatenliste füllen
  hopfen -> ergWidget = berHopfen;
  hopfen -> setBierWurdeGebraut(BierWurdeGebraut);
  hopfen -> setHopfenListe(HopfenListe);

  connect(hopfen, SIGNAL( sig_vorClose(int) ), this, SLOT( slot_hopfenClose(int) ));
  connect(hopfen, SIGNAL( sig_Aenderung() ), this, SLOT( slot_HopfenAenderung() ));

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
  verticalLayout_BerHopfengaben -> addWidget(berHopfen);

}


void MainWindowImpl::slot_spinBoxGesammtkochdauerChanged(int value)
{
  //Maximalen wert für Hopfenzeiten Setzten
  for (int i=0; i < list_Hopfengaben.count(); i++){
    list_Hopfengaben[i]->setMaxKochzeit(value);
  }
}


void MainWindowImpl::slot_spinBoxspinBox_ZeitGabe_1Changed(int )
{
  //Minimalwert für die nächste gabe setzten
  //spinBox_ZeitGabe_2 -> setMaximum(value);
}

void MainWindowImpl::slot_spinBoxspinBox_ZeitGabe_2Changed(int )
{
  //Minimalwert für die nächste gabe setzten
  //spinBox_ZeitGabe_3 -> setMaximum(value);
}

void MainWindowImpl::slot_spinBoxspinBox_ZeitGabe_3Changed(int )
{
  //Minimalwert für die nächste gabe setzten
  //spinBox_ZeitGabe_4 -> setMaximum(value);
}

void MainWindowImpl::slot_spinBoxspinBox_ZeitGabe_4Changed(int )
{
  //Minimalwert für die nächste gabe setzten
  //spinBox_ZeitGabe_5 -> setMaximum(value);
}

void MainWindowImpl::slot_spinBoxspinBox_ZeitGabe_5Changed(int )
{
  //Minimalwert für die nächste gabe setzten
  //spinBox_ZeitGabe_6 -> setMaximum(value);
}

QString MainWindowImpl::GetWertString(double value)
{
  return "<span class='value'>" + QString::number(value) + "</span>";
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


void MainWindowImpl::slot_pushButton_MalzKopie()
{
  int i = tableWidget_Malz -> currentRow();

  if ((i >= 0) && (i < tableWidget_Malz->rowCount()))  {
    QString s = Malz_Bezeichnung_Merker;
    Malz_Bezeichnung_Merker = "";
    tableWidget_Malz->setSortingEnabled(false);

    QString s1 = tableWidget_Malz -> item(i,0) -> text();
    QDoubleSpinBox* sbFarbe=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,1);
    QDoubleSpinBox* sbMaxSchuettung=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,2);
    QDoubleSpinBox* sbMenge=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,3);
    QDoubleSpinBox* sbPreis=(QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,4);
    QString s6 = tableWidget_Malz -> item(i,5) -> text();
    QString s7 = tableWidget_Malz -> item(i,6) -> text();
    QTableWidgetItem *newItem1 = new QTableWidgetItem(s1 + trUtf8(" Kopie"));
    QTableWidgetItem *newItem6 = new QTableWidgetItem(s6);
    QTableWidgetItem *newItem7 = new QTableWidgetItem(s7);

    i = tableWidget_Malz -> rowCount();
    tableWidget_Malz -> setRowCount(i+1);

    //Farbe
    MyDoubleSpinBox *spinBoxFarbe = new MyDoubleSpinBox();
    spinBoxFarbe -> setMinimum(0);
    spinBoxFarbe -> setMaximum(99999);
    spinBoxFarbe -> setDecimals(1);
    spinBoxFarbe -> setValue(sbFarbe->value());
    connect(spinBoxFarbe, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
    tableWidget_Malz -> setCellWidget(i, 1, spinBoxFarbe);

    //Maximaler Schüttungsanteil
    MyDoubleSpinBox *spinBoxMaxSchuettung = new MyDoubleSpinBox();
    spinBoxMaxSchuettung -> setMinimum(0);
    spinBoxMaxSchuettung -> setMaximum(100);
    spinBoxMaxSchuettung -> setDecimals(0);
    spinBoxMaxSchuettung -> setValue(sbMaxSchuettung->value());
    connect(spinBoxMaxSchuettung, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
    tableWidget_Malz -> setCellWidget(i, 2, spinBoxMaxSchuettung);

    //Menge
    MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
    spinBoxMenge -> setMinimum(0);
    spinBoxMenge -> setMaximum(9999);
    spinBoxMenge -> setDecimals(3);
    spinBoxMenge -> setSingleStep(0.1);
    spinBoxMenge -> setValue(sbMenge->value());
    connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
    tableWidget_Malz -> setCellWidget(i, 3, spinBoxMenge);

    //Preis
    MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
    spinBoxPreis -> setMinimum(0);
    spinBoxPreis -> setMaximum(9999);
    spinBoxPreis -> setDecimals(2);
    spinBoxPreis -> setSingleStep(0.1);
    spinBoxPreis -> setValue(sbPreis->value());
    connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
    tableWidget_Malz -> setCellWidget(i, 4, spinBoxPreis);

    //Datum Eingelagert
    QDateEdit * deEinlagerung = new QDateEdit(QDate::currentDate());
    deEinlagerung->setDisplayFormat("dd.MM.yyyy");
    deEinlagerung->setCalendarPopup(true);
    connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
    tableWidget_Malz -> setCellWidget(i, 7, deEinlagerung);

    //Mindesthaltbarkeitsdatum
    QDateEdit * deMhd = new QDateEdit(QDate::currentDate());
    deMhd->setDisplayFormat("dd.MM.yyyy");
    deMhd->setCalendarPopup(true);
    connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
    tableWidget_Malz -> setCellWidget(i, 8, deMhd);

    tableWidget_Malz -> setItem(i, 0, newItem1);
    tableWidget_Malz -> setItem(i, 5, newItem6);
    tableWidget_Malz -> setItem(i, 6, newItem7);

    setAenderung(true);
    AenderungRohstofftabelle = true;

    tableWidget_Malz->setSortingEnabled(true);
    Malz_Bezeichnung_Merker = s;
  }
}


void MainWindowImpl::slot_pushButton_HopfenKopie()
{
  int i = tableWidget_Hopfen -> currentRow();

  QString s = Hopfen_Bezeichnung_Merker;
  Hopfen_Bezeichnung_Merker = "";
  tableWidget_Hopfen->setSortingEnabled(false);

  QString s1 = tableWidget_Hopfen -> item(i,0) -> text();
  QDoubleSpinBox* sbAlpha=(QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,1);
  QDoubleSpinBox* sbMenge=(QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,2);
  QDoubleSpinBox* sbPreis=(QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,3);
  QString s6 = tableWidget_Hopfen -> item(i,5) -> text();
  QString s7 = tableWidget_Hopfen -> item(i,7) -> text();
  QTableWidgetItem *newItem1 = new QTableWidgetItem(s1 + trUtf8(" Kopie"));
  QTableWidgetItem *newItem5 = new QTableWidgetItem(trUtf8("Pellets"));
  QTableWidgetItem *newItem6 = new QTableWidgetItem(s6);
  QTableWidgetItem *newItem7 = new QTableWidgetItem(s7);
  newItem5 -> setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
  newItem5 -> setCheckState(tableWidget_Hopfen -> item(i,4) -> checkState());
  QComboBox* comboTyp=(QComboBox*)tableWidget_Hopfen -> cellWidget(i,6);

  i = tableWidget_Hopfen -> rowCount();
  tableWidget_Hopfen -> setRowCount(i+1);

  //Alpha
  MyDoubleSpinBox *spinBoxAlpha = new MyDoubleSpinBox();
  spinBoxAlpha -> setMinimum(0);
  spinBoxAlpha -> setMaximum(100);
  spinBoxAlpha -> setDecimals(1);
  spinBoxAlpha -> setSingleStep(0.1);
  spinBoxAlpha -> setValue(sbAlpha->value());
  connect(spinBoxAlpha, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Hopfen -> setCellWidget(i, 1, spinBoxAlpha);

  //Menge
  MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
  spinBoxMenge -> setMinimum(0);
  spinBoxMenge -> setMaximum(999999);
  spinBoxMenge -> setDecimals(0);
  spinBoxMenge -> setSingleStep(10);
  spinBoxMenge -> setValue(sbMenge->value());
  connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Hopfen -> setCellWidget(i, 2, spinBoxMenge);

  //Preis
  MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
  spinBoxPreis -> setMinimum(0);
  spinBoxPreis -> setMaximum(999);
  spinBoxPreis -> setDecimals(2);
  spinBoxPreis -> setSingleStep(1);
  spinBoxPreis -> setValue(sbPreis->value());
  connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Hopfen -> setCellWidget(i, 3, spinBoxPreis);

  //Combobox Typ
  MyComboBox *comboBoxTyp = new MyComboBox();
  comboBoxTyp -> addItems(HopfenTypListe);
  comboBoxTyp -> setCurrentIndex(comboTyp->currentIndex());
  connect(comboBoxTyp, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  tableWidget_Hopfen -> setCellWidget(i, 6, comboBoxTyp);

  //Datum Eingelagert
  QDateEdit * deEinlagerung = new QDateEdit(QDate::currentDate());
  deEinlagerung->setDisplayFormat("dd.MM.yyyy");
  deEinlagerung->setCalendarPopup(true);
  connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_Hopfen -> setCellWidget(i, 8, deEinlagerung);

  //Mindesthaltbarkeitsdatum
  QDateEdit * deMhd = new QDateEdit(QDate::currentDate());
  deMhd->setDisplayFormat("dd.MM.yyyy");
  deMhd->setCalendarPopup(true);
  connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_Hopfen -> setCellWidget(i, 9, deMhd);

  tableWidget_Hopfen -> setItem(i, 0, newItem1);
  tableWidget_Hopfen -> setItem(i, 4, newItem5);
  tableWidget_Hopfen -> setItem(i, 5, newItem6);
  tableWidget_Hopfen -> setItem(i, 7, newItem7);

  tableWidget_Hopfen->setSortingEnabled(true);
  Hopfen_Bezeichnung_Merker = s;
  setAenderung(true);
  AenderungRohstofftabelle = true;
}


void MainWindowImpl::slot_pushButton_HefeKopie()
{
  int i = tableWidget_Hefe -> currentRow();
  tableWidget_Hefe->setSortingEnabled(false);

  QString s1 = tableWidget_Hefe -> item(i,0) -> text();
  QDoubleSpinBox* sbMenge=(QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,1);
  QDoubleSpinBox* sbWuerzemenge=(QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,2);
  QDoubleSpinBox* sbPreis=(QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,3);
  QString s4 = tableWidget_Hefe -> item(i,4) -> text();
  QString s5 = tableWidget_Hefe -> item(i,5) -> text();
  QComboBox* comboTypOGUG=(QComboBox*)tableWidget_Hefe -> cellWidget(i,6);
  QComboBox* comboTypTrFl=(QComboBox*)tableWidget_Hefe -> cellWidget(i,7);
  QString s8 = tableWidget_Hefe -> item(i,8) -> text();
  QString s9 = tableWidget_Hefe -> item(i,9) -> text();
  QComboBox* comboSED=(QComboBox*)tableWidget_Hefe -> cellWidget(i,10);
  QString s11 = tableWidget_Hefe -> item(i,11) -> text();

  QString s = Hefe_Bezeichnung_Merker;
  Hefe_Bezeichnung_Merker = "";

  QTableWidgetItem *newItem1 = new QTableWidgetItem(s1 + trUtf8(" Kopie"));
  QTableWidgetItem *newItem4 = new QTableWidgetItem(s4);
  QTableWidgetItem *newItem5 = new QTableWidgetItem(s5);
  QTableWidgetItem *newItem8 = new QTableWidgetItem(s8);
  QTableWidgetItem *newItem9 = new QTableWidgetItem(s9);
  QTableWidgetItem *newItem11 = new QTableWidgetItem(s11);
  i = tableWidget_Hefe -> rowCount();
  tableWidget_Hefe -> setRowCount(i+1);

  //Beschreibung
  tableWidget_Hefe -> setItem(i, 0, newItem1);

  //Menge
  MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
  spinBoxMenge -> setMinimum(0);
  spinBoxMenge -> setMaximum(999999);
  spinBoxMenge -> setDecimals(0);
  spinBoxMenge -> setSingleStep(1);
  spinBoxMenge -> setValue(sbMenge->value());
  connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Hefe -> setCellWidget(i, 1, spinBoxMenge);

  //Würzemenge
  MyDoubleSpinBox *spinBoxWuerzemenge = new MyDoubleSpinBox();
  spinBoxWuerzemenge -> setMinimum(0);
  spinBoxWuerzemenge -> setMaximum(999999);
  spinBoxWuerzemenge -> setDecimals(0);
  spinBoxWuerzemenge -> setSingleStep(1);
  spinBoxWuerzemenge -> setValue(sbWuerzemenge->value());
  connect(spinBoxWuerzemenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Hefe -> setCellWidget(i, 2, spinBoxWuerzemenge);

  //Preis
  MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
  spinBoxPreis -> setMinimum(0);
  spinBoxPreis -> setMaximum(999);
  spinBoxPreis -> setDecimals(2);
  spinBoxPreis -> setSingleStep(0.1);
  spinBoxPreis -> setValue(sbPreis->value());
  connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  tableWidget_Hefe -> setCellWidget(i, 3, spinBoxPreis);

  //Bemerkung
  tableWidget_Hefe -> setItem(i, 4, newItem4);

  //Verpackungsmenge
  tableWidget_Hefe -> setItem(i, 5, newItem5);

  //Combobox Typ Obergärig Untergärig
  MyComboBox *comboBoxTypOGUG = new MyComboBox();
  comboBoxTypOGUG -> addItems(HefeTypOGUGListe);
  comboBoxTypOGUG -> setCurrentIndex(comboTypOGUG->currentIndex());
  connect(comboBoxTypOGUG, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  tableWidget_Hefe -> setCellWidget(i, 6, comboBoxTypOGUG);

  //Combobox Typ Flüssig Trocken
  MyComboBox *comboBoxTypTrFl = new MyComboBox();
  comboBoxTypTrFl -> addItems(HefeTypTrFlListe);
  comboBoxTypTrFl -> setCurrentIndex(comboTypTrFl->currentIndex());
  connect(comboBoxTypTrFl, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  tableWidget_Hefe -> setCellWidget(i, 7, comboBoxTypTrFl);

  //Temperaturbereich
  tableWidget_Hefe -> setItem(i, 8, newItem8);

  //Eigenschaften
  tableWidget_Hefe -> setItem(i, 9, newItem9);

  //Combobox Sedimentation
  MyComboBox *comboBoxSED = new MyComboBox();
  comboBoxSED -> addItems(HefeSedListe);
  comboBoxSED -> setCurrentIndex(comboSED->currentIndex());
  connect(comboBoxSED, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  tableWidget_Hefe -> setCellWidget(i, 10, comboBoxSED);

  //Endvergärungsgrad
  tableWidget_Hefe -> setItem(i, 11, newItem11);

  //Datum Eingelagert
  QDateEdit * deEinlagerung = new QDateEdit(QDate::currentDate());
  deEinlagerung->setDisplayFormat("dd.MM.yyyy");
  deEinlagerung->setCalendarPopup(true);
  connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_Hefe -> setCellWidget(i, 12, deEinlagerung);

  //Mindesthaltbarkeitsdatum
  QDateEdit * deMhd = new QDateEdit(QDate::currentDate());
  deMhd->setDisplayFormat("dd.MM.yyyy");
  deMhd->setCalendarPopup(true);
  connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_Hefe -> setCellWidget(i, 13, deMhd);

  tableWidget_Hefe->setSortingEnabled(true);
  Hefe_Bezeichnung_Merker = s;
  setAenderung(true);
  AenderungRohstofftabelle = true;
}


void MainWindowImpl::slot_makePdf()
{

  //allen nochmal durchrechnen
  BerAlles();
  //Zusammenfassung/Spickzettel neue erstellen
  ErstelleTabSpickzettel();
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  settings.beginGroup("PDF");

  //Zoomfaktor einlesen
  double zoom;
  QString s;
  if (BierWurdeGebraut){
    s = settings.value("zoomZusammenfassung").toString();
    if (s == ""){
      zoom = 1;
      settings.setValue("zoomZusammenfassung",zoom);
    }
    else
      zoom = s.toDouble();
  }
  else {
    s = settings.value("zoomSpickzettel").toString();
    if (s == ""){
      zoom = 1;
      settings.setValue("zoomSpickzettel",zoom);
    }
    else
      zoom = s.toDouble();
  }

  webView_Zusammenfassung->setTextSizeMultiplier(zoom);
  /*QPrintPreviewDialog dialog(this);
  connect(&dialog, SIGNAL(paintRequested(QPrinter *)),
                 webView_Zusammenfassung, SLOT(print(QPrinter *)));
  dialog.exec();
  */

  //letzten Pfad einlesen
  QString p;
  p = settings.value("recentPDFPath").toString();
  //wenn verzeichnis noch nicht gespeichert ist home verzeichnis nehmen
  if (p == "") {
    p = QDir::homePath();
  }

  //printer einstellungen
  QPrinter printer(QPrinter::HighResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setColorMode(QPrinter::Color);
  printer.setResolution(1200);
  QFileDialog fd(this);

  //QString fileName = fd.getSaveFileName(this, trUtf8("PDF Datei Speichern unter"), p, trUtf8("Suddateien (*.pdf)"),0,QFileDialog::DontUseNativeDialog);
  QString fileName = fd.getSaveFileName(this, trUtf8("PDF Datei Speichern unter"), p + "/" + lineEdit_Sudname->text()+".pdf", trUtf8("Suddateien (*.pdf)"),0);
  if (!fileName.isEmpty()) {
    printer.setOutputFileName(fileName);
    //pdf speichern
    webView_Zusammenfassung -> print(&printer);

    //Pfad abspeichern
    QFileInfo fi(fileName);
    settings.setValue("recentPDFPath",fi.absolutePath());

    //PDF Betrachter starten
    if (settings.value("startPDFBetrachter").toBool()) {
      QString prog = settings.value("PDFProg").toString();
      QFileInfo fi(prog);
      if (fi.exists()) {
        QStringList arguments;
        arguments << fileName;
        QProcess *myProcess = new QProcess();
        //qDebug() << "starte PDF Betrachter: " << prog << " " << arguments;
        myProcess->start(prog,arguments);
      }
    }
  }

  settings.endGroup();
  webView_Zusammenfassung->setTextSizeMultiplier(1);
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
    double schuettung[list_Malzgaben.count() + list_EwZutat.count()];
    double farbe[list_Malzgaben.count() + list_EwZutat.count()];
    QString s;
    int gefunden = 0;
    for (int z = 0; z < list_Malzgaben.count(); z++){
      s = list_Malzgaben[z]->getName();
      if (s != ""){
        schuettung[z] = list_Malzgaben[z]->getErgMenge();
        for (int i=0; i < tableWidget_Malz -> rowCount(); i++){
          if (s == tableWidget_Malz -> item(i,0) -> text()){
            QDoubleSpinBox* dsbFarbe = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,1);
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
    tableWidget_Sudauswahl -> horizontalHeader() -> setSectionResizeMode(QHeaderView::Interactive);
    tableWidget_Sudauswahl -> horizontalHeader() -> resizeSection(1,400);
    tableWidget_Sudauswahl -> horizontalHeader() -> setStretchLastSection(true);
  }
  tableWidget_Sudauswahl -> setSortingEnabled(true);
  tableWidget_Sudauswahl -> setCurrentCell(SelZeile,0);
  tableWidget_Sudauswahl -> setColumnHidden(0, true);
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
  pushButton_SudKopie -> setEnabled(false);
  QSqlQuery query;
  int row = tableWidget_Sudauswahl -> currentRow();
  QString SudIDFrom = tableWidget_Sudauswahl -> item(row,0) -> text();
  QString SudIDNeu;

  QSqlDatabase::database().transaction();

  //Datensatz Kopieren
  QString sql = "SELECT * FROM Sud WHERE ID=" + SudIDFrom;
  if (!query.exec(sql)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sql);
  }
  query.first();
  sql = "INSERT INTO Sud ('Sudname',";
  sql += "'Menge',";
  sql += "'SW',";
  sql += "'CO2',";
  sql += "'IBU',";
  sql += "'Kommentar' ,";
  sql += "'Braudatum',";
  sql += "'BierWurdeGebraut' ,";
  sql += "'Anstelldatum' ,";
  sql += "'WuerzemengeAnstellen'  ,";
  sql += "'SWAnstellen'  ,";
  sql += "'Abfuelldatum',";
  sql += "'BierWurdeAbgefuellt' ,";
  sql += "'SWSchnellgaerprobe' ,";
  sql += "'SWJungbier' ,";
  sql += "'TemperaturJungbier' ,";
  sql += "'WuerzemengeKochende' ,";
  sql += "'Speisemenge' ,";
  sql += "'SWKochende'  ,";
  sql += "'AuswahlHefe' ,";
  sql += "'FaktorHauptguss' ,";
  sql += "'KochdauerNachBitterhopfung' ,";
  sql += "'EinmaischenTemp' ,";
  sql += "'Erstellt' ,";
  sql += "'Gespeichert' ,";
  sql += "'AktivTab' ,";
  sql += "'erg_S_Gesammt'  ,";
  sql += "'erg_W_Gesammt' ,";
  sql += "'erg_WHauptguss'  ,";
  sql += "'erg_WNachguss'  ,";
  sql += "'erg_Sudhausausbeute' ,";
  sql += "'erg_Farbe'  ,";
  sql += "'erg_Preis' ,";
  sql += "'erg_Alkohol' ,";
  sql += "'erg_EffektiveAusbeute' ,";
  sql += "'KostenWasserStrom' ,";
  sql += "'Nachisomerisierungszeit' ,";
  sql += "'WuerzemengeVorHopfenseihen' ,";
  sql += "'SWVorHopfenseihen' ,";
  sql += "'RestalkalitaetSoll' ,";
  sql += "'SchnellgaerprobeAktiv' ,";
  sql += "'JungbiermengeAbfuellen' ,";
  sql += "'erg_AbgefuellteBiermenge' ,";
  sql += "'BewertungMaxSterne' ,";
  sql += "'NeuBerechnen' ,";
  sql += "'HefeAnzahlEinheiten' ,";
  sql += "'berechnungsArtHopfen' ,";
  sql += "'highGravityFaktor' ,";
  sql += "'AuswahlBrauanlage' ,";
  sql += "'AuswahlBrauanlageName' ,";
  sql += "'Reifezeit') ";
  sql += "Values(";
  sql += "'Kopie von " + query.value(1).toString().replace("'","''") + "',";
  sql += "'" + query.value(2).toString().replace("'","''") + "',";
  sql += "'" + query.value(3).toString().replace("'","''") + "',";
  sql += "'" + query.value(4).toString().replace("'","''") + "',";
  sql += "'" + query.value(5).toString().replace("'","''") + "',";
  sql += "'" + query.value(6).toString().replace("'","''") + "',";
  sql += "'" + query.value(7).toString().replace("'","''") + "',";
  sql += "'0',";
  sql += "'" + query.value(9).toString().replace("'","''") + "',";
  sql += "'" + query.value(10).toString().replace("'","''") + "',";
  sql += "'" + query.value(11).toString().replace("'","''") + "',";
  sql += "'" + query.value(12).toString().replace("'","''") + "',";
  sql += "'0',";
  sql += "'" + query.value(14).toString().replace("'","''") + "',";
  sql += "'" + query.value(15).toString().replace("'","''") + "',";
  sql += "'" + query.value(16).toString().replace("'","''") + "',";
  sql += "'" + query.value(17).toString().replace("'","''") + "',";
  sql += "'" + query.value(18).toString().replace("'","''") + "',";
  sql += "'" + query.value(19).toString().replace("'","''") + "',";
  sql += "'" + query.value(20).toString().replace("'","''") + "',";
  sql += "'" + query.value(21).toString().replace("'","''") + "',";
  sql += "'" + query.value(22).toString().replace("'","''") + "',";
  sql += "'" + query.value(23).toString().replace("'","''") + "',";
  sql += "'" + QDateTime::currentDateTime().toString(Qt::ISODate) + "',";
  sql += "'',";
  sql += "'" + query.value(26).toString().replace("'","''") + "',";
  sql += "'" + query.value(27).toString().replace("'","''") + "',";
  sql += "'" + query.value(28).toString().replace("'","''") + "',";
  sql += "'" + query.value(29).toString().replace("'","''") + "',";
  sql += "'" + query.value(30).toString().replace("'","''") + "',";
  sql += "'" + query.value(31).toString().replace("'","''") + "',";
  sql += "'" + query.value(32).toString().replace("'","''") + "',";
  sql += "'" + query.value(33).toString().replace("'","''") + "',";
  sql += "'" + query.value(34).toString().replace("'","''") + "',";
  sql += "'" + query.value(44).toString().replace("'","''") + "',";
  sql += "'" + query.value(35).toString().replace("'","''") + "',";
  sql += "'" + query.value(41).toString().replace("'","''") + "',";
  sql += "'" + query.value(42).toString().replace("'","''") + "',";
  sql += "'" + query.value(43).toString().replace("'","''") + "',";
  sql += "'" + query.value(45).toString().replace("'","''") + "',";
  sql += "'" + query.value(46).toString().replace("'","''") + "',";
  sql += "'" + query.value(47).toString().replace("'","''") + "',";
  sql += "'" + query.value(48).toString().replace("'","''") + "',";
  sql += "'" + query.value(49).toString().replace("'","''") + "',";
  sql += "'" + query.value(50).toString().replace("'","''") + "',";
  sql += "'" + query.value(51).toString().replace("'","''") + "',";
  sql += "'" + query.value(52).toString().replace("'","''") + "',";
  sql += "'" + query.value(53).toString().replace("'","''") + "',";
  sql += "'" + query.value(54).toString().replace("'","''") + "',";
  sql += "'" + query.value(55).toString().replace("'","''") + "',";
  sql += "'" + query.value(39).toString().replace("'","''") + "'";
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
      SudIDNeu = query.value(0).toString();
    }

    //Rastdatensätze Kopieren
    QSqlQuery query_rasten;
    sql = "SELECT * FROM Rasten WHERE SudID=" + SudIDFrom + ";";
    if (!query_rasten.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query_rasten.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    while (query_rasten.next()){
      sql = "INSERT INTO Rasten ";
      sql += "(";
      sql += "'SudID', ";
      sql += "'RastAktiv', ";
      sql += "'RastTemp', ";
      sql += "'RastDauer', ";
      sql += "'RastName' ";
      sql += ")Values(";
      sql += "'" + SudIDNeu + "',";
      sql += "'" + query_rasten.value(2).toString().replace("'","''") + "',";
      sql += "'" + query_rasten.value(3).toString().replace("'","''") + "',";
      sql += "'" + query_rasten.value(4).toString().replace("'","''") + "',";
      sql += "'" + query_rasten.value(5).toString().replace("'","''") + "'";
      sql += ");";
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
    }
    //Malz Schüttung Kopieren
    QSqlQuery query_Malz;
    sql = "SELECT * FROM Malzschuettung WHERE SudID=" + SudIDFrom + ";";
    if (!query_Malz.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query_Malz.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    while (query_Malz.next()){
      //überprüfen ob das Malz noch existiert
      QString name = query_Malz.value(2).toString();
      if (name !=""){
        bool gefunden = false;
        for (int i=0; i < tableWidget_Malz -> rowCount(); i++){
          if (tableWidget_Malz -> item(i,0) -> text() == name)
            gefunden = true;
        }
        //wenn Eintrag nicht gefunden wurde Dialogfeld zum Austauschen anzeigen
        if (!gefunden){
          RohstoffAustauschen raDia;
          raDia.setButtonCancelVisible(false);
          for (int i=0; i < tableWidget_Malz -> rowCount(); i++){
            raDia.addAuswahlEintrag(tableWidget_Malz -> item(i,0) -> text() );
          }
          raDia.SetText(trUtf8("Der Malzeintrag <b>") + name + trUtf8("</b> ist nicht mehr vorhanden.\n\n ersetzen durch?"));
          raDia.exec();
          name = raDia.GetAktAuswahl();
        }
      }

      //Malz eintragen
      sql = "INSERT INTO Malzschuettung ";
      sql += "(";
      sql += "'SudID', ";
      sql += "'Name', ";
      sql += "'Prozent', ";
      sql += "'erg_Menge', ";
      sql += "'Farbe' ";
      sql += ")Values(";
      sql += "'" + SudIDNeu + "',";
      sql += "'" + name.replace("'","''") + "',";
      sql += "'" + query_Malz.value(3).toString().replace("'","''") + "',";
      sql += "'" + query_Malz.value(4).toString().replace("'","''") + "',";
      sql += "'" + query_Malz.value(5).toString().replace("'","''") + "'";
      sql += ");";
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
    }
    //Hopfengaben Kopieren
    QSqlQuery query_Hopfen;
    sql = "SELECT * FROM Hopfengaben WHERE SudID=" + SudIDFrom + ";";
    if (!query_Hopfen.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query_Hopfen.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    while (query_Hopfen.next()){
      //überprüfen ob der Rohstoff noch existiert
      QString name = query_Hopfen.value(3).toString();
      if (name !=""){
        bool gefunden = false;
        for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
          if (tableWidget_Hopfen -> item(i,0) -> text() == name)
            gefunden = true;
        }
        //wenn Eintrag nicht gefunden wurde Dialogfeld zum Austauschen anzeigen
        if (!gefunden){
          RohstoffAustauschen raDia;
          raDia.setButtonCancelVisible(false);
          for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
            raDia.addAuswahlEintrag(tableWidget_Hopfen -> item(i,0) -> text() );
          }
          raDia.SetText(trUtf8("Der Hopfeneintrag <b>") + name + trUtf8("</b> ist nicht mehr vorhanden.\n\n ersetzen durch?"));
          raDia.exec();
          name = raDia.GetAktAuswahl();
        }
      }

      sql = "INSERT INTO Hopfengaben ";
      sql += "(";
      sql += "'SudID', ";
      sql += "'Aktiv', ";
      sql += "'Name', ";
      sql += "'Prozent', ";
      sql += "'Zeit', ";
      sql += "'erg_Menge', ";
      sql += "'erg_Hopfentext', ";
      sql += "'Alpha', ";
      sql += "'Pellets', ";
      sql += "'Vorderwuerze' ";
      sql += ")Values(";
      sql += "'" + SudIDNeu + "',";
      sql += "'" + query_Hopfen.value(2).toString().replace("'","''") + "',";
      sql += "'" + name.replace("'","''") + "',";
      sql += "'" + query_Hopfen.value(4).toString().replace("'","''") + "',";
      sql += "'" + query_Hopfen.value(5).toString().replace("'","''") + "',";
      sql += "'" + query_Hopfen.value(6).toString().replace("'","''") + "',";
      sql += "'" + query_Hopfen.value(7).toString().replace("'","''") + "',";
      sql += "'" + query_Hopfen.value(8).toString().replace("'","''") + "',";
      sql += "'" + query_Hopfen.value(9).toString().replace("'","''") + "',";
      sql += "'" + query_Hopfen.value(10).toString().replace("'","''") + "'";
      sql += ");";
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }

    }

    //Weitere Zutaten Kopieren
    QSqlQuery query_WeitereZutaten;
    sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + SudIDFrom + ";";
    if (!query_WeitereZutaten.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query_WeitereZutaten.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    while (query_WeitereZutaten.next()){
      QString name;
      //wenn typ = Hopfen ist
      if (query_WeitereZutaten.value(5).toString() == "100"){
        //überprüfen ob der Rohstoff noch existiert
        name = query_WeitereZutaten.value(2).toString();
        if (name !=""){
          bool gefunden = false;
          for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
            if (tableWidget_Hopfen -> item(i,0) -> text() == name)
              gefunden = true;
          }
          //wenn Eintrag nicht gefunden wurde Dialogfeld zum Austauschen anzeigen
          if (!gefunden){
            RohstoffAustauschen raDia;
            raDia.setButtonCancelVisible(false);
            for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
              raDia.addAuswahlEintrag(tableWidget_Hopfen -> item(i,0) -> text() );
            }
            raDia.SetText(trUtf8("Der Hopfeneintrag in den Weiteren Zutaten <b>") + name + trUtf8("</b> ist nicht mehr vorhanden.\n\n ersetzen durch?"));
            raDia.exec();
            name = raDia.GetAktAuswahl();
          }
        }
      }
      //wenn weitere Zutat
      else {
        //überprüfen ob der Rohstoff noch existiert
        name = query_WeitereZutaten.value(2).toString();
        if (name !=""){
          bool gefunden = false;
          for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
            if (tableWidget_WeitereZutaten -> item(i,0) -> text() == name)
              gefunden = true;
          }
          //wenn Eintrag nicht gefunden wurde Dialogfeld zum Austauschen anzeigen
          if (!gefunden){
            RohstoffAustauschen raDia;
            raDia.setButtonCancelVisible(false);
            for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
              raDia.addAuswahlEintrag(tableWidget_WeitereZutaten -> item(i,0) -> text() );
            }
            raDia.SetText(trUtf8("Der Rohstoffeintrag in den Weiteren Zutaten <b>") + name + trUtf8("</b> ist nicht mehr vorhanden.\n\n ersetzen durch?"));
            raDia.exec();
            name = raDia.GetAktAuswahl();
          }
        }
      }

      sql = "INSERT INTO WeitereZutatenGaben ";
      sql += "(";
      sql += "'SudID', ";
      sql += "'Name', ";
      sql += "'Menge', ";
      sql += "'Einheit', ";
      sql += "'Typ', ";
      sql += "'Zeitpunkt', ";
      sql += "'Bemerkung', ";
      sql += "'erg_Menge', ";
      sql += "'Ausbeute', ";
      sql += "'Farbe' ";
      sql += ")Values(";
      sql += "'" + SudIDNeu + "',";
      sql += "'" + name.replace("'","''") + "',";
      sql += "'" + query_WeitereZutaten.value(3).toString().replace("'","''") + "',";
      sql += "'" + query_WeitereZutaten.value(4).toString().replace("'","''") + "',";
      sql += "'" + query_WeitereZutaten.value(5).toString().replace("'","''") + "',";
      sql += "'" + query_WeitereZutaten.value(6).toString().replace("'","''") + "',";
      sql += "'" + query_WeitereZutaten.value(7).toString().replace("'","''") + "',";
      sql += "'" + query_WeitereZutaten.value(8).toString().replace("'","''") + "',";
      sql += "'" + query_WeitereZutaten.value(9).toString().replace("'","''") + "',";
      sql += "'" + query_WeitereZutaten.value(10).toString().replace("'","''") + "'";
      sql += ");";
      if (!query.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
      }
    }
  }

  QSqlDatabase::database().commit();
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
  if (Aenderung){
    if (AbfrageSpeichern()){
    }
  }
  pushButton_SudLaden -> setEnabled(false);
  int row = tableWidget_Sudauswahl -> currentRow();
  AktuelleSudID = tableWidget_Sudauswahl -> item(row,0) -> text().toInt();
  LadeSudDB(true);
  pushButton_SudLaden -> setEnabled(true);
}

void MainWindowImpl::on_pushButton_SudLadenBUebersicht_clicked()
{
  int row = tableWidget_Brauuebersicht -> currentRow();
  if (row >= 0){
    if (Aenderung){
      if (AbfrageSpeichern()){
      }
    }
    pushButton_SudLadenBUebersicht -> setEnabled(false);
    AktuelleSudID = tableWidget_Brauuebersicht -> item(row,0) -> text().toInt();
    LadeSudDB(true);
    pushButton_SudLadenBUebersicht -> setEnabled(true);
  }
}

void MainWindowImpl::AddMalzgabe(QString Name, double Prozent, double erg_Menge, double Farbe)
{

  //Malz hinzufügen
  //Zutatenobjekt hinzufügen
  malzgabe* malz = new malzgabe(this);
  malz -> setAttribute(Qt::WA_DeleteOnClose);
  //Ergebnisswidget ersetellen
  doubleEditLineImpl* berMalz = new doubleEditLineImpl(this);
  berMalz -> setAttribute(Qt::WA_DeleteOnClose);
  berMalz -> setVisible(false);

  malz -> ergWidget = berMalz;
  malz -> ergWidget->label_Einheit->setText("kg");

  connect(malz, SIGNAL( sig_vorClose(int) ), this, SLOT( slot_malzClose(int) ));
  connect(malz, SIGNAL( sig_Aenderung() ), this, SLOT( slot_MalzAenderung() ));
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
  verticalLayout_BerMalzgaben -> addWidget(berMalz);
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
        if (list_Hopfengaben[i]->getName() == tableWidget_Hopfen -> item(o,0) -> text()){
          QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(o,1);
          Alphaprozent = QString::number(spinBox->value());
          bool b = tableWidget_Hopfen -> item(o,4) -> checkState();
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
        if (Name == tableWidget_Malz -> item(i,0) -> text()){
          QDoubleSpinBox* dsbFarbe = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,1);
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
        if (s == tableWidget_Malz -> item(i,0) -> text()){
          QDoubleSpinBox* dsbPreis = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(i,4);
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
      if (s == tableWidget_Hopfen -> item(i,0) -> text()){
        QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,3);
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
      if (tableWidget_Hefe -> item(i,0) -> text() == s){
        QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hefe -> cellWidget(i,3);
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


void MainWindowImpl::on_pushButton_SpickzettelDrucken_clicked()
{
  slot_print();
}

void MainWindowImpl::on_pushButton_SpickzettelPDF_clicked()
{
  slot_makePdf();
}

void MainWindowImpl::on_spinBox_KostenWasserStrom_editingFinished()
{
}


void MainWindowImpl::on_spinBox_KostenWasserStrom_valueChanged(double )
{
  BerKosten();
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

  tableWidget_Brauuebersicht -> setColumnHidden(0, true);
  tableWidget_Brauuebersicht -> horizontalHeader() -> setSectionResizeMode(QHeaderView::Interactive);
  tableWidget_Brauuebersicht -> horizontalHeader() -> resizeSection(1,200);
  tableWidget_Brauuebersicht -> horizontalHeader() -> resizeSection(4,140);
  //tableWidget_Brauuebersicht -> horizontalHeader() -> setStretchLastSection(true);

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
      QTableWidgetItem *newItem5 = new QTableWidgetItem("");
      QTableWidgetItem *newItem6 = new QTableWidgetItem("");
      QTableWidgetItem *newItem7 = new QTableWidgetItem("");
      QTableWidgetItem *newItem8 = new QTableWidgetItem("");
      QTableWidgetItem *newItem9 = new QTableWidgetItem("");
      QTableWidgetItem *newItem10 = new QTableWidgetItem("");
      QTableWidgetItem *newItem11 = new QTableWidgetItem("");
      QTableWidgetItem *newItem12 = new QTableWidgetItem("");
      QTableWidgetItem *newItem13 = new QTableWidgetItem("");
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
        QString text = QString::number(w+1) + ". " + trUtf8("Woche");
        newItem11 -> setText(text + trUtf8("\n noch") + " " + QString::number(tage) + " "  + trUtf8("Tage"));
        newItem11 -> setTextColor(QColor::fromRgb(0,0,0));
        if (StyleDunkel)
          newItem11 -> setBackground(QColor::fromRgb(FARBE_BierReift_DARK));
        else
          newItem11 -> setBackground(QColor::fromRgb(FARBE_BierReift));
      }
      else {
        int w = tageReifung / 7;
        newItem11 -> setText(QString::number(w+1) + ". " + trUtf8("Woche"));
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
      newItem5 -> setText(QString::number(d));
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
      newItem6 -> setText(QString::number(sw));
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
      newItem7 -> setText(QString::number(sha));
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
      newItem8 -> setText(QString::number(msch));
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
      newItem9 -> setText(QString::number(kost));
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
      newItem10 -> setText(QString::number(abv));
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
      newItem12 -> setText(QString::number(evg));
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
      newItem13 -> setText(QString::number(d));
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
  NeueMessungWirdAngelegt = true;
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
  //Temperatur
  temperatur = doubleSpinBox_TempSchnellgaerprobe -> value();
  newItem4 -> setData(Qt::DisplayRole, temperatur);
  //Zeile anhand des Datums uhrzeit finden
  int i = tableWidget_Schnellgaerverlauf -> rowCount();
  for (int o=0; o < tableWidget_Schnellgaerverlauf -> rowCount(); o++){
    QDateTime dt2;
    dt2 = tableWidget_Schnellgaerverlauf -> item(o,0) -> data(Qt::DisplayRole).toDateTime();
    if (dt < dt2){
      i = o;
      o = tableWidget_Schnellgaerverlauf->rowCount();
    }
  }
  tableWidget_Schnellgaerverlauf -> insertRow(i);

  newItem1->setFlags(newItem1->flags() & (~Qt::ItemIsEditable));
  tableWidget_Schnellgaerverlauf -> setItem(i, 0, newItem1);
  tableWidget_Schnellgaerverlauf -> setItem(i, 1, newItem2);
  tableWidget_Schnellgaerverlauf -> setItem(i, 2, newItem3);
  tableWidget_Schnellgaerverlauf -> setItem(i, 3, newItem4);
  //Mittig ausrichten
  newItem1 -> setTextAlignment(Qt::AlignCenter);
  newItem2 -> setTextAlignment(Qt::AlignCenter);
  newItem3 -> setTextAlignment(Qt::AlignCenter);
  newItem4 -> setTextAlignment(Qt::AlignCenter);

  //Diagramm füllen
  FuelleDiagrammSchnellgaerverlauf();
  setAenderung(true);

  //Wenn Bier noch nicht abgefüllt wurde dann in Wert in Brau & Gärdaten übernehmen
  if (!BierWurdeAbgefuellt){
    spinBox_SWSchnellgaerprobe -> setValue(sw);
  }
  NeueMessungWirdAngelegt = false;
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
      newItem1->setFlags(newItem1->flags() & (~Qt::ItemIsEditable));
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
  NeueMessungWirdAngelegt = true;
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
  //Temperatur
  temperatur = doubleSpinBox_TempHauptgaerprobe -> value();
  newItem4 -> setData(Qt::DisplayRole, temperatur);

  //Zeile anhand des Datums uhrzeit finden
  int i = tableWidget_Hauptgaerverlauf -> rowCount();
  for (int o=0; o < tableWidget_Hauptgaerverlauf -> rowCount(); o++){
    QDateTime dt2;
    dt2 = tableWidget_Hauptgaerverlauf -> item(o,0) -> data(Qt::DisplayRole).toDateTime();
    if (dt < dt2){
      i = o;
      o = tableWidget_Hauptgaerverlauf->rowCount();
    }
  }
  tableWidget_Hauptgaerverlauf -> insertRow(i);
  newItem1->setFlags(newItem1->flags() & (~Qt::ItemIsEditable));
  tableWidget_Hauptgaerverlauf -> setItem(i, 0, newItem1);
  tableWidget_Hauptgaerverlauf -> setItem(i, 1, newItem2);
  tableWidget_Hauptgaerverlauf -> setItem(i, 2, newItem3);
  tableWidget_Hauptgaerverlauf -> setItem(i, 3, newItem4);
  //Mittig ausrichten
  newItem1 -> setTextAlignment(Qt::AlignCenter);
  newItem2 -> setTextAlignment(Qt::AlignCenter);
  newItem3 -> setTextAlignment(Qt::AlignCenter);
  newItem4 -> setTextAlignment(Qt::AlignCenter);

  //Diagramm füllen
  FuelleDiagrammHauptgaerverlauf();
  setAenderung(true);
  AenderungHauptgaerverlauf = true;

  //Wenn Bier noch nicht abgefüllt wurde dann in Wert in Brau & Gärdaten übernehmen
  if (!BierWurdeAbgefuellt){
    spinBox_SWJungbier -> setValue(sw);
    spinBox_TemperaturJungbier -> setValue(temperatur);
  }
  NeueMessungWirdAngelegt = false;
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
      newItem1->setFlags(newItem1->flags() & (~Qt::ItemIsEditable));
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
  NeueMessungWirdAngelegt = true;
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
  //Zeile anhand des Datums uhrzeit finden
  int i = tableWidget_Nachgaerverlauf -> rowCount();
  for (int o=0; o < tableWidget_Nachgaerverlauf -> rowCount(); o++){
    QDateTime dt2;
    dt2 = tableWidget_Nachgaerverlauf -> item(o,0) -> data(Qt::DisplayRole).toDateTime();
    if (dt < dt2){
      i = o;
      o = tableWidget_Nachgaerverlauf->rowCount();
    }
  }
  tableWidget_Nachgaerverlauf -> insertRow(i);

  newItem1->setFlags(newItem1->flags() & (~Qt::ItemIsEditable));
  tableWidget_Nachgaerverlauf -> setItem(i, 0, newItem1);
  tableWidget_Nachgaerverlauf -> setItem(i, 1, newItem2);
  tableWidget_Nachgaerverlauf -> setItem(i, 2, newItem3);
  tableWidget_Nachgaerverlauf -> setItem(i, 3, newItem4);
  //Mittig ausrichten
  newItem1 -> setTextAlignment(Qt::AlignCenter);
  newItem2 -> setTextAlignment(Qt::AlignCenter);
  newItem3 -> setTextAlignment(Qt::AlignCenter);
  newItem4 -> setTextAlignment(Qt::AlignCenter);

  //Diagramm füllen
  FuelleDiagrammNachgaerverlauf();
  setAenderung(true);
  NeueMessungWirdAngelegt = false;
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
      newItem1->setFlags(newItem1->flags() & (~Qt::ItemIsEditable));
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
      tableWidget_Nachgaerverlauf -> setItem(i, 3, newItem4);
      i++;
      //Mittig ausrichten
      newItem1 -> setTextAlignment(Qt::AlignCenter);
      newItem2 -> setTextAlignment(Qt::AlignCenter);
      newItem3 -> setTextAlignment(Qt::AlignCenter);
      newItem4 -> setTextAlignment(Qt::AlignCenter);
    }
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
  edia.exec();
  if (edia.NeuerDBPfad) {
    close();
  }
  //Farben für Diagramme neu einlesen
  if (edia.B_OK){
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
    rdia.comboBox_S_1 -> addItem(tableWidget_Malz -> item(i,0) -> text() );
    rdia.comboBox_S_2 -> addItem(tableWidget_Malz -> item(i,0) -> text() );
    rdia.comboBox_S_3 -> addItem(tableWidget_Malz -> item(i,0) -> text() );
    rdia.comboBox_S_4 -> addItem(tableWidget_Malz -> item(i,0) -> text() );
    rdia.comboBox_S_5 -> addItem(tableWidget_Malz -> item(i,0) -> text() );
    rdia.comboBox_S_6 -> addItem(tableWidget_Malz -> item(i,0) -> text() );
  }

  //Dialog öffnen mit ansicht Schüttungsübernahme
  rdia.exec();

  //Schüttung übernehmen
  if (rdia.b_OK  && !BierWurdeGebraut){
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
  //Dialog öffnen mit ansicht Berechnung Bittere
  rdia.exec();
  //IBU Wert übernehmen
  if (rdia.b_OK  && !BierWurdeGebraut){
    spinBox_IBU -> setValue(qRound(rdia.doubleSpinBox_IBU -> value()));
  }

}

void MainWindowImpl::on_tableWidget_Schnellgaerverlauf_itemChanged(QTableWidgetItem* )
{
  if (Gestartet && (!NeueMessungWirdAngelegt) && (!AmLaden)){
    // Alkoholgehalt dieser Zeile neu berechnen
    int zeile;
    double sw, alc;
    zeile = tableWidget_Schnellgaerverlauf -> currentRow();
    sw = tableWidget_Schnellgaerverlauf -> item(zeile,1) -> data(Qt::DisplayRole).toDouble();
    //Alkoholgehalt berechnen
    alc = Berechnungen.BerAlkohoVol(spinBox_SWSollGesammt -> value(), sw );
    alc = double(qRound(alc * 10)) / 10;
    tableWidget_Schnellgaerverlauf -> item(zeile,2) -> setData(Qt::DisplayRole, alc);
    setAenderung(true);
    FuelleDiagrammSchnellgaerverlauf();
  }
}

void MainWindowImpl::on_tableWidget_Hauptgaerverlauf_itemChanged(QTableWidgetItem* )
{
  if (Gestartet && (!NeueMessungWirdAngelegt) && (!AmLaden)){
    // Alkoholgehalt dieser Zeile neu berechnen
    int zeile;
    double sw, alc;
    zeile = tableWidget_Hauptgaerverlauf -> currentRow();
    sw = tableWidget_Hauptgaerverlauf -> item(zeile,1) -> data(Qt::DisplayRole).toDouble();
    //Alkoholgehalt berechnen
    alc = Berechnungen.BerAlkohoVol(spinBox_SWSollGesammt -> value(), sw );
    alc = double(qRound(alc * 10)) / 10;
    tableWidget_Hauptgaerverlauf -> item(zeile,2) -> setData(Qt::DisplayRole, alc);
    setAenderung(true);
    AenderungHauptgaerverlauf = true;
    FuelleDiagrammHauptgaerverlauf();
  }
}

void MainWindowImpl::on_tableWidget_Nachgaerverlauf_itemChanged(QTableWidgetItem* )
{
  if (Gestartet && (!NeueMessungWirdAngelegt) && (!AmLaden)){
    // CO2 Gehalt dieser Zeile Neu berechnen
    int zeile;
    double druck, temp, co2;
    zeile = tableWidget_Nachgaerverlauf -> currentRow();
    druck = tableWidget_Nachgaerverlauf -> item(zeile,1) -> data(Qt::DisplayRole).toDouble();
    temp = tableWidget_Nachgaerverlauf -> item(zeile,2) -> data(Qt::DisplayRole).toDouble();
    co2 = Berechnungen.BerCO2Gehalt(druck, temp);
    co2 = double(qRound(co2 * 100)) / 100;
    tableWidget_Nachgaerverlauf -> item(zeile,3) -> setData(Qt::DisplayRole, co2);
    setAenderung(true);
    FuelleDiagrammNachgaerverlauf();
  }
}

void MainWindowImpl::ErstelleZusammenfassung()
{
  //Überschrift vom Tab setzten
  tabWidged -> setTabText(5,trUtf8("Zusammenfassung"));
  // Seitenkopf
  QString seite, kopf, ende, style;

  kopf = "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'> <html><head><meta name='qrichtext' content='1' />";
  style = "<style type='text/css'>";
  //Style für P
  style += "p{color:black;font-size:10pt;padding:0px;margin:0px;}";
  //Style für Variable
  style += ".value{color:blue;margin-left:5px;margin-right:5px;}";
  //Style für div Kommentar
  style += ".koment{}";
  //Style für ul
  style += "ul{color:black;font-size:10pt;}";
  //Style für Überschrift h1
  style += "p.h1{color:black;font-size:12pt;}";
  //Style für Überschrift h2
  style += "p.h2{color:black;font-size:11pt;margin-bottom:5px;}";
  style += "p.version{color:#999999;font-size:11pt;margin-top:5px;}";
  //Style für Kommentar
  style += "p.kommentar{color:#555555;font-size:11pt;margin-bottom:5px;}";
  //Style für Div Box ohne Rahmen
  style += "div.r{border:0px solid #dddddd; border-radius: 10px; padding:5px;background-color:#dddddd;}";
  //Style für Div Box mit Rahmen
  style += "div.rm{border:2px solid #dddddd; border-radius: 10px; padding:5px;background-color:#ffffff;}";
  //Style für Tabelle
  style += "td{padding:2px;margin:0px;font-size:10pt;}";
  style += "td.r{padding:2px;margin:0px;border-bottom-color:#dddddd;border-bottom-style:solid;border-width:1px;}";
  //Style für Hinweis Wert in Brau und Gärdaten eintragen
  style += "td.we{background-color: #eba328;}";
  style += "tr{padding:0px;margin:0px;}";
  style += "</style>";
  kopf += style;
  kopf += "</head><body align='center' style='font-family:Ubuntu,Arial; font-size:10pt; font-style:normal;background-color:#fff;'>";
  seite = kopf;

  QString s = "";

  //Tabelle für Bild und Zutaten
  s += "<div class='' width='99%' style='' align='center'>";

  s += "<div class='rm' style='margin:0px;margin-bottom:5px;width:60%;' align='center'>";
  s += "<table style='width:99%;' cellspacing=0 border=0><tbody>";
  s += "<tr style=''>";
  s += "<td>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Name
  s += "<tr style=''>";
  s += "<td>";
  s += "<p class='h1'><b>" + lineEdit_Sudname -> text() + "</b></p>";
  s += "</td>";
  s += "</tr>";
  //Menge
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Menge") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + QString::number(spinBox_WuerzemengeAnstellen -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Stammwürze
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Stammwürze") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + QString::number(spinBox_SWSollGesammt -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("°P") + "</p>";
  s += "</td>";
  s += "</tr>";
  //High Gravity Faktor
  if (spinBox_High_Gravity->value() > 0) {
    s += "<tr style=''>";
    s += "<td>";
    s += "<p>" + trUtf8("High Gravity Faktor") + "</p>";
    s += "</td>";
    s += "<td align='right'>";
    s += "<p class='value'>" + QString::number(spinBox_High_Gravity -> value()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("%") + "</p>";
    s += "</td>";
    s += "</tr>";
  }
  //Alkoholgehalt
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Alkoholgehalt") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + QString::number(spinBox_AlkoholVol -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("%Vol") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Bittere
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Bittere") + "</p>";
  s += "</td>";
  s += "<td>";
  //Bittere anhand der Ausschlagmenge Berechnen
  double ibu = spinBox_IBU -> value()*highGravityFaktor;
  ibu = spinBox_MengeSollNachHopfenseihen -> value() / spinBox_WuerzemengeAnstellen -> value() * ibu;
  ibu = qRound(ibu*10);
  ibu = ibu/10;
  s += "<p class='value'>" + QString::number(ibu) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("IBU") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Farbe
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Farbe") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + QString::number(doubleSpinBox_EBC -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("EBC") + "</p>";
  s += "</td>";
  s += "</tr>";
  //CO2 Gehalt
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("CO2 Gehalt") + "</p>";
  s += "</td>";
  s += "<td>";
  //Bei mehr als einem Eintrag im Nachgärverlauf wird der CO2 Gehalt aus dem
  //Nachgärverlauf entnommen ansonsten der Teoretische
  double d = doubleSpinBox_CO2 -> value();
  QString sqlN = "SELECT * FROM Nachgaerverlauf WHERE SudID="
      + QString::number(AktuelleSudID) + " ORDER BY Zeitstempel DESC;";
  QSqlQuery queryN;
  if (!queryN.exec(sqlN)) {
    // Fehlermeldung Datenbankabfrage
    ErrorMessage *errorMessage = new ErrorMessage();
    errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                CANCEL_NO, trUtf8("Rückgabe:\n") + queryN.lastError().databaseText()
                                + trUtf8("\nSQL Befehl:\n") + sqlN);
  }
  else {
    if (queryN.first()){
      int FeldNr = queryN.record().indexOf("Druck");
      //Wert nur Übernehmen wenn der Druck größer 0 ist
      //Ansonsten ist davon auszugehen das der Eintrag der automatisch angelegete ist
      if (queryN.value(FeldNr).toDouble() > 0){
        FeldNr = queryN.record().indexOf("CO2");
        d = queryN.value(FeldNr).toDouble();
      }
    }
  }
  s += "<p class='value'>" + QString::number(d) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("g/Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  if (BierWurdeAbgefuellt){
    //Scheinbarer Endvergärungsgrad
    s += "<tr style=''>";
    s += "<td>";
    s += "<p>" + trUtf8("scheinbarer Endvergärungsgrad") + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p class='value'>" + QString::number(spinBox_SEVG -> value()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("%") + "</p>";
    s += "</td>";
    s += "</tr>";
  }
  //Effektive Sudhausausbeute
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("effektive Sudhausausbeute") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + QString::number(spinBox_AusbeuteEffektiv2 -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("%") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Brauanlage
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Brauanlage") + "</p>";
  s += "</td>";
  s += "<td colspan=2 align='right'>";
  s += "<p class='value'>" + comboBox_AuswahlBrauanlage->currentText() + "</p>";
  s += "</td>";
  s += "</tr>";
  //Kosten pro Liter
  //s += "<tr style=''>";
  //s += "<td>";
  //s += "<p>" + trUtf8("Kosten") + "</p>";
  //s += "</td>";
  //s += "<td>";
  //s += "<p class='value'>" + QString::number(spinBox_Preis -> value()) + "</p>";
  //s += "</td>";
  //s += "<td>";
  //s += "<p>" + trUtf8("€/Liter") + "</p>";
  //s += "</td>";
  //s += "</tr>";
  s += "</tbody></table>";
  s += "</td>";
  //Datumsangaben
  s += "<td style='vertical-align:top;' align=right>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Braudatum
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Braudatum") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + dateEdit_Braudatum -> date().toString("dd.MM.yyyy") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Abfülldatum
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Abfülldatum") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + dateEdit_Abfuelldatum -> date().toString("dd.MM.yyyy") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Datum der Angepeilten Reifezeit
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Angepeiltes Reifezeitende") + "</p>";
  s += "</td>";
  s += "<td>";
  //Start der Reifung ermitteln indem das letzte Datum vom
  //Nachgärverlauf benutzt wird
  sqlN = "SELECT * FROM Nachgaerverlauf WHERE SudID="
      + QString::number(AktuelleSudID) + " ORDER BY Zeitstempel DESC;";
  QDate date = dateEdit_Abfuelldatum -> date();
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
  date = date.addDays(spinBox_Reifezeit -> value() * 7);
  s += "<p class='value'>" + date.toString("dd.MM.yyyy") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Beste Bewertung
  int bew=0;
  QString bewtext="";
  if (list_Bewertung.count() > 0){
    for (int i=0; i<list_Bewertung.count(); i++){
      int b = list_Bewertung[i]->getSterne();
      if (b > bew){
        bew = b;
        bewtext = QString::number(list_Bewertung[i]->getWoche())+". Woche ";
      }
    }
    s += "<tr style=''>";
    s += "<td>";
    s += "<p>" + trUtf8("Beste Bewertung") + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p class='value'>" + bewtext + "</p>";
    s += "</td>";
    s += "</tr>";
    s += "<tr style=''>";
    s += "<td colspan=2>";
    s += "<div class='bew' style='' align='center'>";
    for (int i = 0; i<bew; i++){
      s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_24.png' width='24' border=0>";
    }
    for (int i = bew; i<MaxAnzahlSterne; i++){
      s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_gr_24.png' width='24' border=0>";
    }
    s += "</div>";
    s += "</td>";
    s += "</tr>";
  }

  s += "</tbody></table>";
  s += "</td>";
  s += "</tr>";
  s += "</tbody></table>";
  s += "</div>";

  s += "<table width='90%' summary='tabelle' border='0' cellspacing='3'>";
  s += "<tr style=''>";
  s += "<td valign='bottom' style=''>";
  //Schüttung
  s += "<div class='rm' style='margin:0px;margin-bottom:5px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/getreide_300.png' alt='Getreide' width='300px' border=0>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Alle Malzgaben
  for (int i=0; i < list_Malzgaben.count(); i++){
    s += "<tr style=''>";
    s += "<td>";
    s += "<p>" + list_Malzgaben[i]->getName() + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p class='value'>" + QString::number(list_Malzgaben[i]->getErgMenge()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("Kg") + "</p>";
    s += "</td>";
    s += "<td align='right'>";
    s += "<p class='value'>" + QString::number(list_Malzgaben[i]->getMengeProzent()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("%") + "</p>";
    s += "</td>";
    s += "<td align='right'>";
    s += "<p class='value'>" + QString::number(list_Malzgaben[i]->getFarbe()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("EBC") + "</p>";
    s += "</td>";
    s += "</tr>";
  }
  //Gesamt
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Gesamt") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value' style='font-weight:bold;'>" + QString::number(doubleSpinBox_S_Gesammt -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Kg") + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "</tbody></table>";
  s += "</div>";
  s += "</td>";
  s += "<td valign='top' style=''>";
  //bild mit entsprechender Bierfarbe
  QColor farbe;
  farbe = Berechnungen.GetFarbwert(doubleSpinBox_EBC -> value());
  s += "<div class='' style='background-color:" + farbe.name() + ";width:210px;height:210px;margin:0px;padding:0px;'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/global/bier_420x420.png' alt='Bierfarbe' width='210px' height='210px' border=0>";
  s += "</div>";
  s += "</td>";
  s += "<td valign='top' style=''>";
  //Hopfen
  s += "<div class='rm' style='margin:0px;margin-bottom:5px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/hopfen_100.png' alt='Hopfen' width='100px' border=0>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Alle Hopfengaben
  for (int i=0; i < list_Hopfengaben.count(); i++){
    s += "<tr style=''>";
    s += "<td>";
    if (list_Hopfengaben[i]->getVWH())
      s += "<p>" + trUtf8("VWH ") + list_Hopfengaben[i]->getErgebnistext() + "</p>";
    else
      s += "<p>" + list_Hopfengaben[i]->getErgebnistext() + "</p>";
    s += "</td>";
    s += "<td align='right'>";
    s += "<p class='value'>" + QString::number(list_Hopfengaben[i]->getErgMenge()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("g") + "</p>";
    s += "</td>";
    s += "<td align='right'>";
    s += "<p class='value'>" + QString::number(list_Hopfengaben[i]->getKochzeit()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("min") + "</p>";
    s += "</td>";
    s += "</tr>";
  }
  //Hopfengaben in den Weiteren Zutaten
  for (int i=0; i < list_EwZutat.count(); i++){
    //Nur Hopfengaben
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Hopfen){
      s += "<tr style=''>";
      s += "<td>";
      s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
      s += "</td>";
      s += "<td align='right'>";
      s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
      s += "</td>";
      s += "<td>";
      s += "<p>" + trUtf8("g") + "</p>";
      s += "</td>";
      s += "<td align='right' colspan='2'>";
      if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
        s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
      else
        s += "<p class='value'>" + trUtf8("Anstellen") + "</p>";
      s += "</td>";
      s += "</tr>";
      //wenn ein Kommentar vorhanden ist eine Zeile für den Komentar einfügen
      if (list_EwZutat[i]->getBemerkung() != "") {
        s += "<tr style=''>";
        s += "<td colspan='5'>";
        s += "<p class='kommentar'>"+ list_EwZutat[i]->getBemerkung().replace("\n","<br>")+"</p>";
        s += "</td>";
        s += "</tr>";
      }
    }
  }
  s += "</tbody></table>";
  s += "</div>";
  s += "</td>";
  s += "</tr>";
  s += "</table>";




  s += "<table width='70%' summary='tabelle' border='0' cellspacing='3'>";
  s += "<tr style=''>";
  s += "<td valign='top' style=''>";
  //Wasser
  s += "<div class='rm' style='margin:0px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/wasser_100x107.png' alt='Hefe' width='50px' border=0>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Hauptguss
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Hauptguss") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(doubleSpinBox_WHauptguss -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "<tr>";
  s += "<td>";
  s += "<p>" + trUtf8("Milchsäure (80%)") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(SpinBox_waMilchsaeureHG_ml -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("ml") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Nachguss
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Nachguss") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(doubleSpinBox_WNachguss -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Milchsäure (80%)") + "</p>";
  s += "</td>";
  s += "<td align='right'>";
  s += "<p class='value'>" + QString::number(SpinBox_waMilchsaeureNG_ml -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("ml") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Gesammt
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + label_37 -> text() + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value' style='font-weight:bold;'>" + QString::number(doubleSpinBox_W_Gesammt -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("Liter") + "</p>";
  s += "</td>";
  s += "</tr>";
  s += "</tbody></table>";
  s += "</div>";
  s += "</td>";
  //Hefe
  s += "<td valign='top' style=''>";
  s += "<div class='rm' style='margin:0px;' align='center'>";
  s += "<img style='padding:0px;margin:0px;' src='qrc:/zutaten/hefe_50.png' alt='Hefe' width='50px' border=0>";
  s += "<p>" + comboBox_AuswahlHefe -> currentText() + "</p>";
  QString sEinheiten;
  sEinheiten = trUtf8("Anzahl Einheiten:") + " <span class='value'>" + QString::number(spinBox_AnzahlHefeEinheiten->value()) +"</span>";
  //Verpackungsgrösse aus den Rohstoffdaten auslesen
  int AnzahlHefeEintraege = tableWidget_Hefe -> rowCount();
  QString HefeName = comboBox_AuswahlHefe -> currentText();
  QString verpMenge;
  if (HefeName != ""){
    //Würzemenge auslesen
    for (int i=0; i < AnzahlHefeEintraege; i++){
      //wenn Eintrag übereinstimmt
      if (tableWidget_Hefe -> item(i,0) -> text() == HefeName){
        verpMenge = tableWidget_Hefe -> item(i,5) -> text();
      }
    }
  }
  if (!verpMenge.isEmpty()) {
    sEinheiten += trUtf8(" zu ") + verpMenge;
  }
  s += "<p>" + sEinheiten +"</p>";
  s += "</div>";
  s += "</td>";
  s += "</tr>";

  s += "</table>";


  //Tabelle Weitere Zutaten
  s += "<table width='80%' summary='testtabelle' border='0' cellspacing='5'>";
  s += "<tr >";

  //Honig
  bool HonigVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Honig){
      HonigVorhanden = true;
    }
  }
  if (HonigVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_0_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Honig){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("Kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochbeginn") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
        //Wenn vorhanden Kommentar anzeigen
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4'>";
          s += "<p class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</p>";
          s += "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  //Zucker
  bool ZuckerVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Zucker){
      ZuckerVorhanden = true;
    }
  }
  if (ZuckerVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_1_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Zucker){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("Kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochbeginn") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
        //Wenn vorhanden Kommentar anzeigen
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4'>";
          s += "<p class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</p>";
          s += "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  //Gewürz
  bool GewuerzVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Gewuerz){
      GewuerzVorhanden = true;
    }
  }
  if (GewuerzVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_2_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Gewuerz){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("Kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochen") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
        //Wenn vorhanden Kommentar anzeigen
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4'>";
          s += "<p class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</p>";
          s += "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  //Frucht
  bool FruchtVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Frucht){
      FruchtVorhanden = true;
    }
  }
  if (FruchtVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_3_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Frucht){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("Kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochen") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
        //Wenn vorhanden Kommentar anzeigen
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4'>";
          s += "<p class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</p>";
          s += "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  //Sonstiges
  bool SonstigesVorhanden = false;
  for (int i=0; i < list_EwZutat.count(); i++){
    if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Sonstiges){
      SonstigesVorhanden = true;
    }
  }
  if (SonstigesVorhanden){
    s += "<td valign='middle' style=''>";
    s += "<div class='rm' style='margin:0px;' align='center'>";
    s += "<img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_4_50.png' alt='Honig' width='50px' border=0>";
    s += "<table cellspacing=0 border=0><tbody>";
    for (int i=0; i < list_EwZutat.count(); i++){
      if (list_EwZutat[i] -> getTyp() == EWZ_Typ_Sonstiges){
        s += "<tr style=''>";
        s += "<td>";
        s += "<p>" + list_EwZutat[i] -> getName() + "</p>";
        s += "</td>";
        s += "<td align='right'>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge() / 1000) + "</p>";
        else
          s += "<p class='value'>" + QString::number(list_EwZutat[i] -> getErg_Menge()) + "</p>";
        s += "</td>";
        s += "<td>";
        if (list_EwZutat[i] -> getEinheit() == EWZ_Einheit_Kg)
          s += "<p>" + trUtf8("Kg") + "</p>";
        else
          s += "<p>" + trUtf8("g") + "</p>";
        s += "</td>";
        s += "<td align='right' colspan='2'>";
        if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Gaerung)
          s += "<p class='value'>" + trUtf8("Gärung") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Kochbeginn)
          s += "<p class='value'>" + trUtf8("Kochen") + "</p>";
        else if (list_EwZutat[i] -> getZeitpunkt() == EWZ_Zeitpunkt_Maischen)
          s += "<p class='value'>" + trUtf8("Maischen") + "</p>";
        s += "</td>";
        s += "</tr>";
        //Wenn vorhanden Kommentar anzeigen
        if (list_EwZutat[i] -> getBemerkung() != ""){
          s += "<tr>";
          s += "<td colspan='4'>";
          s += "<p class='kommentar'>" + list_EwZutat[i] -> getBemerkung().replace("\n","<br>") + "</p>";
          s += "</td>";
          s += "</tr>";
        }
      }
    }
    s += "</table>";
    s += "</div>";
    s += "</td>";
  }

  s += "</tr >";
  s += "</table>";

  //Kommentar
  if (textEdit_Kommentar -> toPlainText() != ""){
    s += "<div class='rm' style='margin:10px;width:80%;'>";
    s += textEdit_Kommentar -> toHtml();
    s += "</div>";
  }


  //Maischen
  s += "<div div class='rm' style='width:80%;'>";
  s += "<p class='h2'>" + trUtf8("Maischen:") + "</p>";
  s += "<table cellspacing=0 border=0><tbody>";
  //Einmaischen
  s += "<tr style=''>";
  s += "<td>";
  s += "<p>" + trUtf8("Einmaischen bei") + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p class='value'>" + QString::number(spinBox_EinmaischenTemp -> value()) + "</p>";
  s += "</td>";
  s += "<td>";
  s += "<p>" + trUtf8("°C") + "</p>";
  s += "</td>";
  s += "</tr>";
  //Rasten in Zusammenfassung eintragen
  for (int i=0; i<list_Rasten.count();i++){
    s += "<tr style=''>";
    s += "<td>";
    s += "<p>" + list_Rasten[i]->getRastName() + trUtf8(" bei ") + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p class='value'>" + QString::number(list_Rasten[i]->getRastTemp()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("°C") + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p class='value'>" + QString::number(list_Rasten[i]->getRastDauer()) + "</p>";
    s += "</td>";
    s += "<td>";
    s += "<p>" + trUtf8("min") + "</p>";
    s += "</td>";
    s += "</tr>";
  }
  s += "</tbody></table>";
  s += "</div>";
  //Bewertungen
  //asdf

  //Gärverlauf
  //FuelleGaerverlauf();
  //widget_DiaSchnellgaerverlauf -> BildSpeichern("svg");
  //s += "<div div class='r' style='width:99%;float:left;margin-top:10px;'>";
  //s += "<p class='h2'>" + trUtf8("Gärverlauf:") + "</p>";
  //s += "<p>" + trUtf8("Schnellvergärprobe:") + "</p>";
  //s += "<img src='svg.png' style='width:98%'>";
  //s += "</div>";
  s += "";
  s += "";
  s += "";
  s += "";
  s += "";
  s += "";
  s += "";
  s += "";
  s += "";
  //s += QUrl::fromLocalFile(QCoreApplication::applicationDirPath()).toString();

  //Anhänge
  if (list_Anhang.count() > 0) {
    s += "</br>";
    s += "<div div class='rm' style='width:80%;'>";
    s += "<p class='h2'>" + trUtf8("Anhänge:") + "</p>";
    for (int i=0; i<list_Anhang.count();i++){
      if (AnhangWidget::isImage(list_Anhang[i]->getPfad()))
        s += "<img style=\"max-width:80%;\" src=\"file:///" + list_Anhang[i]->getFullPfad() + "\"></br>";
      else
        s += "<a href=\"file:///" + list_Anhang[i]->getFullPfad() + "\" target=\"_blank\">" + list_Anhang[i]->getPfad() + "</a></br>";
    }
    s += "</div>";
  }

  s += "<div><p class='version'>"APP_NAME" v";
  s += VERSION;
  s += "</p></div>";

  seite += s;
  //Seitenende
  ende = "</body></html>";
  seite += ende;

  webView_Zusammenfassung -> setRenderHint(QPainter::TextAntialiasing, true);
  webView_Zusammenfassung -> setRenderHint(QPainter::SmoothPixmapTransform, true);
  webView_Zusammenfassung -> setRenderHint(QPainter::HighQualityAntialiasing, true);

  webView_Zusammenfassung -> setHtml(seite,QUrl::fromLocalFile(QCoreApplication::applicationDirPath()+"/"));
  //webView_Zusammenfassung -> reload();
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
  EingabeHVolumenImpl dia;
  dia.setHoehe(getSudpfanneHoehe());
  dia.setDurchmesser(getSudpfanneDurchmesser());
  dia.setLiter(spinBox_WuerzemengeVorHopfenseihen -> value());
  dia.setWindowTitle(trUtf8("Eingabehilfe für Volumen Sudpfanne"));
  dia.exec();
  if (!dia.abgebrochen){
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
  EingabeHVolumenImpl dia;
  dia.setLiter(spinBox_WuerzemengeKochende -> value());
  dia.setWindowTitle(trUtf8("Eingabehilfe für Volumen nach dem Hopfenseihen"));
  dia.setVisibleVonOben(false);
  dia.setVisibleVonUnten(false);
  dia.exec();
  if (!dia.abgebrochen){
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
      if (tableWidget_Hefe -> item(i,0) -> text() == s){
        QSpinBox *spinBox = (QSpinBox*)tableWidget_Hefe -> cellWidget(i,2);
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
  s = QFileDialog::getSaveFileName(this, trUtf8("Export Sud"), p, trUtf8("Sud Export Dateien (*.xsud)"),0);
  if (!s.isEmpty()) {
    QFileInfo fileinfo(s);
    settings.setValue("recentExportPath",fileinfo.path());
    Export.ExportSudXML(SudID, s);
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
  s = QFileDialog::getOpenFileName(this, trUtf8("Suddatei öffnen"), p, trUtf8("Sud Export Dateien (*.xsud)"),0);
  if (!s.isEmpty()) {
    QFileInfo fileinfo(s);
    settings.setValue("recentExportPath",fileinfo.path());
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
}

void MainWindowImpl::ErstelleSudInfo()
{

  // Seitenkopf
  QString seite, kopf, ende, style, s, SudnameFehler;
  int NeuBerechnen = 0;

  kopf = "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'> <html><head><meta name='qrichtext' content='1' />";
  if (StyleDunkel){
    style = "<style type='text/css'>";
    //Style für P
    style += "p{color:#fff;font-size:10pt;padding:0px;margin:0px;}";
    //Style für Variable
    style += "p.value{color:#eee;margin-left:5px;margin-right:5px;}";
    //Style für div Kommentar
    style += ".koment{}";
    //Style für ul
    style += "ul{color:#fff;font-size:10pt;}";
    //Style für Überschrift h1
    style += "p.h1{color:#fff;font-size:12pt;}";
    //Style für Überschrift h2
    style += "p.h2{color:#fff;font-size:11pt;margin-bottom:5px;}";
    //Style für Div Box bewertung
    style += "div.bew{border:0px solid #fff; border-radius: 5px; padding:0px;background-color:#222222;}";
    //Style für Div Box ohne Rahmen
    style += "div.r{border:0px solid #444444; border-radius: 10px; padding:5px;background-color:#444444;}";
    //Style für Div Box mit Rahmen
    style += "div.rm{border:2px solid #444444; border-radius: 10px; padding:5px;background-color:#222222;font-size:12pt;}";
    //Style für Div Box mit Rahmen für Hinweis
    style += "div.rmh{border:2px solid #ff0000; border-radius: 10px; padding:5px;background-color:#222222;font-size:12pt;}";
    //Style für Tabelle
    style += "td{padding:2px;margin:0px;font-size:10pt;}";
    style += "td.r{padding:2px;margin:0px;border-bottom-color:#aaaaaa;border-bottom-style:solid;border-width:1px;}";
    style += "tr{padding:0px;margin:0px;}";
    style += "body{font-family:Ubuntu,Arial; font-size:10pt; font-style:normal; background-color:#222222; color:#fff;}";
    style += "</style>";
  }
  else {
    style = "<style type='text/css'>";
    //Style für P
    style += "p{color:black;font-size:10pt;padding:0px;margin:0px;}";
    //Style für Variable
    style += "p.value{color:blue;margin-left:5px;margin-right:5px;}";
    //Style für div Kommentar
    style += ".koment{}";
    //Style für ul
    style += "ul{color:black;font-size:10pt;}";
    //Style für Überschrift h1
    style += "p.h1{color:black;font-size:12pt;}";
    //Style für Überschrift h2
    style += "p.h2{color:black;font-size:11pt;margin-bottom:5px;}";
    //Style für Div Box bewertung
    style += "div.bew{border:0px solid #fff; border-radius: 5px; padding:0px;background-color:#fff;}";
    //Style für Div Box ohne Rahmen
    style += "div.r{border:0px solid #dddddd; border-radius: 10px; padding:5px;background-color:#dddddd;}";
    //Style für Div Box mit Rahmen
    style += "div.rm{border:2px solid #dddddd; border-radius: 10px; padding:5px;background-color:#ffffff;font-size:12pt;}";
    //Style für Div Box mit Rahmen für Hinweis
    style += "div.rmh{border:2px solid #ff0000; border-radius: 10px; padding:5px;background-color:#ffffff;font-size:12pt;}";
    //Style für Tabelle
    style += "td{padding:2px;margin:0px;font-size:10pt;}";
    style += "td.r{padding:2px;margin:0px;border-bottom-color:#aaaaaa;border-bottom-style:solid;border-width:1px;}";
    style += "tr{padding:0px;margin:0px;}";
    style += "body{font-family:Ubuntu,Arial; font-size:10pt; font-style:normal; background-color:#fff;}";
    style += "</style>";
  }
  kopf += style;
  kopf += "</head><body>";
  seite = kopf;

  //Liste der SudIds
  QList<int> ListSudID;

  //Bei einer Singleauswahl Datensatz abfragen und Eckdaten anzeigen
  QList<QTableWidgetItem *> sList;
  sList = tableWidget_Sudauswahl -> selectedItems();

  if (sList.count() == 4 || sList.count() == 5) {

    //Sud ID ermitteln
    int row = sList.first() -> row();
    int SudID = tableWidget_Sudauswahl -> item(row,0) -> text().toInt();
    ListSudID.append(SudID);

    //Datensatz abfragen
    QSqlQuery query_sud;
    int FeldNr;
    QString sql = "SELECT * FROM Sud WHERE ID=" + QString::number(SudID) + ";";
    if (!query_sud.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query_sud.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
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
          seite += "<div class='r' style='margin-bottom:10px;' align='center'><p class='h1'><b>" + query_sud.value(FeldNr).toString() + "</b></p></div>";

          //bild mit entsprechender Bierfarbe
          QColor farbe;
          FeldNr = query_sud.record().indexOf("erg_Farbe");
          farbe = Berechnungen.GetFarbwert(query_sud.value(FeldNr).toDouble());
          FeldNr = query_sud.record().indexOf("Bewertung");
          int bewertung = query_sud.value(FeldNr).toInt();

          //Solldaten des Rezeptes
          s += "<div class='rm' style='margin-top:10px;margin-bottom:5px;' align='center'>";
          //Bewertung
          if (bewertung > 0){
            if (bewertung > MaxAnzahlSterne)
              bewertung = MaxAnzahlSterne;
            s += "<div class='bew' style='' align='center'>";
            for (int i = 0; i<bewertung; i++){
              s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_24.png' width='24' border=0>";
            }
            for (int i = bewertung; i<MaxAnzahlSterne; i++){
              if (StyleDunkel)
                s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_gr_dark_24.png' width='24' border=0>";
              else
                s += "<img style='padding:0px;margin:0px;' src='qrc:/global/star_gr_24.png' width='24' border=0>";
            }
            s += "</div>";
          }
          s += "<table cellspacing=0 border=0><tbody>";
          //Menge
          FeldNr = query_sud.record().indexOf("Menge");
          s += "<tr style=''>";
          s += "<td rowspan=5>";
          s += "<div class='r' style='background-color:" + farbe.name() +
              ";width:100px;height:100px;margin:0px;padding:0px;'>";
          if (StyleDunkel)
            s += "<img style='padding:0px;margin:0px;' src='qrc:/global/bier_dark_200x200.png' alt='Bierfarbe' width='100' height='100' border=0>";
          else
            s += "<img style='padding:0px;margin:0px;' src='qrc:/global/bier_200x200.png' alt='Bierfarbe' width='100' height='100' border=0>";
          s += "</div>";
          s += "</td>";
          s += "<td>";
          s += "<p>" + trUtf8("Menge") + "</p>";
          s += "</td>";
          s += "<td align='right'>";
          s += "<p class='value'>" + QString::number(query_sud.value(FeldNr).toInt()) + "</p>";
          s += "</td>";
          s += "<td>";
          s += "<p>" + trUtf8("Liter") + "</p>";
          s += "</td>";
          s += "</tr>";
          //Stammwürze
          FeldNr = query_sud.record().indexOf("SW");
          s += "<tr style=''>";
          s += "<td>";
          s += "<p>" + trUtf8("Stammwürze") + "</p>";
          s += "</td>";
          s += "<td align='right'>";
          s += "<p class='value'>" + QString::number(query_sud.value(FeldNr).toDouble()) + "</p>";
          s += "</td>";
          s += "<td>";
          s += "<p>" + trUtf8("°P") + "</p>";
          s += "</td>";
          s += "</tr>";
          //Bittere
          FeldNr = query_sud.record().indexOf("IBU");
          s += "<tr style=''>";
          s += "<td>";
          s += "<p>" + trUtf8("Bittere") + "</p>";
          s += "</td>";
          s += "<td align='right'>";
          s += "<p class='value'>" + QString::number(query_sud.value(FeldNr).toInt()) + "</p>";
          s += "</td>";
          s += "<td>";
          s += "<p>" + trUtf8("IBU") + "</p>";
          s += "</td>";
          s += "</tr>";
          //Farbe
          FeldNr = query_sud.record().indexOf("erg_Farbe");
          s += "<tr style=''>";
          s += "<td>";
          s += "<p>" + trUtf8("Farbe") + "</p>";
          s += "</td>";
          s += "<td align='right'>";
          s += "<p class='value'>" + QString::number(query_sud.value(FeldNr).toDouble()) + "</p>";
          s += "</td>";
          s += "<td>";
          s += "<p>" + trUtf8("EBC") + "</p>";
          s += "</td>";
          s += "</tr>";
          //CO2 Gehalt
          FeldNr = query_sud.record().indexOf("CO2");
          s += "<tr style=''>";
          s += "<td>";
          s += "<p>" + trUtf8("CO2 Gehalt") + "</p>";
          s += "</td>";
          s += "<td align='right'>";
          s += "<p class='value'>" + QString::number(query_sud.value(FeldNr).toDouble()) + "</p>";
          s += "</td>";
          s += "<td>";
          s += "<p>" + trUtf8("g/Liter") + "</p>";
          s += "</td>";
          s += "</tr>";
          s += "</tbody></table>";
          s += "</div>";
        }
      }
    }
  }
  else {
    //Überschrift Auswahl
    //s += "<div class='r' style='margin-bottom:10px;' align='center'><p class='h1'><b>" + trUtf8("Auswahl") + "</b></p></div>";
    //s += "<div class='rm' style='margin-bottom:10px;'>";
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
        QString sql = "SELECT Sudname,NeuBerechnen FROM Sud WHERE ID=" + QString::number(SudID) + ";";
        if (!query_sud.exec(sql)) {
          // Fehlermeldung Datenbankabfrage
          ErrorMessage *errorMessage = new ErrorMessage();
          errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                      CANCEL_NO, trUtf8("Rückgabe:\n") + query_sud.lastError().databaseText()
                                      + trUtf8("\nSQL Befehl:\n") + sql);
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
    //s += "</div>";
  }

  if (NeuBerechnen > 0){
    //Meldung ausgeben das der Sud zum Neu Berechnen geladen werden muss.
    seite += "<div class='rmh' style='margin-bottom:10px;' align='center'>";
    seite += trUtf8("Bei dem Sud >") + SudnameFehler + trUtf8("< wurde ein Rohstoff verändert und die Berechneten Werte stimmen nun nicht mehr. Zum neu Berechnen bitte den Sud Laden und wieder Speichern.");
    seite += "</div>";
  }
  else {
    //Benötigte Rohstoffe mit Vorhandenen verechnen

    //Alle Malzeinträge abrfuen
    QList<Rohstoff> ListMalz;
    for (int sid = 0; sid < ListSudID.size(); ++sid){
      //Schüttung Abfragen
      QSqlQuery query_Malz;
      QString sql = "SELECT * FROM Malzschuettung WHERE SudID=" + QString::number(ListSudID.at(sid)) + ";";
      if (!query_Malz.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_Malz.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
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
      QString sql = "SELECT * FROM Hopfengaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + ";";
      if (!query_Hopfen.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_Hopfen.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
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
      sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + " AND Typ=100;";
      if (!query_Hopfen.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_Hopfen.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
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
      QString sql = "SELECT AuswahlHefe,HefeAnzahlEinheiten FROM Sud WHERE ID=" + QString::number(ListSudID.at(sid)) + ";";
      if (!query_Hefe.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_Hefe.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
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
              if (tableWidget_Hefe -> item(i,0) -> text() == eHefe.Name){
                QSpinBox *spinBox = (QSpinBox*)tableWidget_Hefe -> cellWidget(i,1);
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
      QString sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + " AND Typ=0;";
      QSqlQuery query_weitereZutaten;
      if (!query_weitereZutaten.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
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
      QString sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + " AND Typ=1;";
      QSqlQuery query_weitereZutaten;
      if (!query_weitereZutaten.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
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
      QString sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + " AND Typ=2;";
      QSqlQuery query_weitereZutaten;
      if (!query_weitereZutaten.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
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
      QString sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + " AND Typ=3;";
      QSqlQuery query_weitereZutaten;
      if (!query_weitereZutaten.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
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
      QString sql = "SELECT * FROM WeitereZutatenGaben WHERE SudID=" + QString::number(ListSudID.at(sid)) + " AND Typ=4;";
      QSqlQuery query_weitereZutaten;
      if (!query_weitereZutaten.exec(sql)) {
        // Fehlermeldung Datenbankabfrage
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                    CANCEL_NO, trUtf8("Rückgabe:\n") + query_weitereZutaten.lastError().databaseText()
                                    + trUtf8("\nSQL Befehl:\n") + sql);
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
    s += "<div class='r' style='margin-bottom:10px;' align='center'><p class='h1'><b>" + trUtf8("benötigte Rohstoffe") + "</b></p></div>";

    //Malz Mengen anzeigen
    //Bild für getreide anzeigen
    s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/zutaten/getreide_300.png' alt='Getreide' width='300px' border=0></div>";
    s += "<div align='center' style='font-size:12pt;'>";
    s += "<table border=0 cellspacing=0 >";
    s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt (kg)") + "</td><td align='center'>" + trUtf8("übrig (kg)") + "</td>";
    for (int i = 0; i < ListMalz.size(); ++i){
      double ist = 0;
      //Vorhandene Menge von diesem Malz
      bool gefunden = false;
      for (int o=0; o < tableWidget_Malz -> rowCount(); o++){
        //wenn Eintrag übereinstimmt
        if (tableWidget_Malz -> item(o,0) -> text() == ListMalz.at(i).Name){
          QDoubleSpinBox* dsbMenge = (QDoubleSpinBox*)tableWidget_Malz -> cellWidget(o,3);
          ist = dsbMenge ->value();
          gefunden = true;
        }
      }
      double rest = ist - ListMalz.at(i).Menge;
      s += "<tr valign='middle'>";
      if (rest < 0){
        s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
      }
      else {
        s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
      }
      //Rohstoff ist aufgeführt
      if (gefunden){
        s += "<td align='left'>" + ListMalz.at(i).Name + "</td>";
      }
      //Rohstoff ist nicht vorhanden
      else {
        s += "<td align='left' style='color: grey;'>" + ListMalz.at(i).Name + "</td>";
      }
      s += "<td align='center'>" + QString::number(ListMalz.at(i).Menge) + "</td>";
      if (rest < 0)
        s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b></td>";
      else
        s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b></td>";
      s += "</tr>";
    }
    s += "</table>";
    s += "</div>";


    //Hopfen Mengen anzeigen
    //Bild für Hopfen anzeigen
    s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/zutaten/hopfen_100.png' alt='Hopfen' width='100px' border=0></div>";
    s += "<div align='center' style='font-size:12pt;'>";
    s += "<table border=0 cellspacing=0 >";
    s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt (g)") + "</td><td align='center'>" + trUtf8("übrig (g)") + "</td>";
    for (int i = 0; i < ListHopfen.size(); ++i){
      double ist = 0;
      bool gefunden = false;
      //Vorhandene Menge von diesem Hopfen
      for (int o=0; o < tableWidget_Hopfen -> rowCount(); o++){
        //wenn Eintrag übereinstimmt
        if (tableWidget_Hopfen -> item(o,0) -> text() == ListHopfen.at(i).Name){
          QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(o,2);
          ist = spinBox->value();
          gefunden = true;
        }
      }
      double rest = ist - ListHopfen.at(i).Menge;
      s += "<tr valign='middle'>";
      if (rest < 0)
        s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
      else
        s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
      //Rohstoff ist aufgeführt
      if (gefunden){
        s += "<td align='left'>" + ListHopfen.at(i).Name + "</td>";
      }
      //Rohstoff ist nicht vorhanden
      else {
        s += "<td align='left' style='color: grey;'>" + ListHopfen.at(i).Name + "</td>";
      }
      s += "<td align='center'>" + QString::number(ListHopfen.at(i).Menge) + "</td>";
      if (rest < 0)
        s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b></td>";
      else
        s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b></td>";
      s += "</tr>";
    }
    s += "</table>";
    s += "</div>";


    //Hefe Mengen anzeigen
    //Bild für Hefe anzeigen
    s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/zutaten/hefe_50.png' alt='Hefe' width='50px' border=0></div>";
    s += "<div align='center' style='font-size:12pt;'>";
    s += "<table border=0 cellspacing=0 >";
    s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("übrig") + "</td>";
    for (int i = 0; i < ListHefe.size(); ++i){
      double ist;
      bool gefunden = false;
      int AnzahlHefeEintraege = tableWidget_Hefe -> rowCount();
      for (int o=0; o < AnzahlHefeEintraege; o++){
        //wenn Eintrag übereinstimmt
        if (tableWidget_Hefe -> item(o,0) -> text() == ListHefe.at(i).Name){
          gefunden = true;
        }
      }
      //Vorhandene Menge von diesem Hopfen
      ist = ListHefe.at(i).MengeIst;
      double rest = ist - ListHefe.at(i).Menge;
      s += "<tr valign='middle'>";
      if (rest < 0)
        s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
      else
        s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
      //Rohstoff ist aufgeführt
      if (gefunden){
        s += "<td align='left'>" + ListHefe.at(i).Name + "</td>";
      }
      //Rohstoff ist nicht vorhanden
      else {
        s += "<td align='left' style='color: grey;'>" + ListHefe.at(i).Name + "</td>";
      }
      s += "<td align='center'>" + QString::number(ListHefe.at(i).Menge) + "</td>";
      if (rest < 0)
        s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b></td>";
      else
        s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b></td>";
      s += "</tr>";
    }
    s += "</table>";
    s += "</div>";

    //WeitereZutaten Honig Mengen anzeigen
    //Bild für Honig anzeigen
    if (ListWeitereZutatenHonig.count() > 0){
      int Einheit=0;
      s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_0_50.png' alt='Honig' width='50px' border=0></div>";
      s += "<div align='center' style='font-size:12pt;'>";
      s += "<table border=0 cellspacing=0 >";
      s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("übrig") + "</td>";
      for (int i = 0; i < ListWeitereZutatenHonig.size(); ++i){
        double ist=0;
        bool gefunden = false;
        //Vorhandene Menge von diesem Honig
        for (int o=0; o < tableWidget_WeitereZutaten -> rowCount(); o++){
          //wenn Eintrag übereinstimmt
          if (tableWidget_WeitereZutaten -> item(o,0) -> text() == ListWeitereZutatenHonig.at(i).Name){
            QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,1);
            ist = dsbMenge -> value();
            QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(o,2);
            Einheit = comboEinheit -> currentIndex();
            if (Einheit == EWZ_Einheit_Kg){
              ist = ist * 1000;
            }
            gefunden = true;
          }
        }
        double rest = ist - ListWeitereZutatenHonig.at(i).Menge;
        s += "<tr valign='middle'>";
        if (rest < 0)
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
        else
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
        //Rohstoff ist aufgeführt
        if (gefunden){
          s += "<td align='left'>" + ListWeitereZutatenHonig.at(i).Name + "</td>";
        }
        //Rohstoff ist nicht vorhanden
        else {
          s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenHonig.at(i).Name + "</td>";
        }
        if (Einheit == EWZ_Einheit_Kg){
          s += "<td align='center'>" + QString::number(ListWeitereZutatenHonig.at(i).Menge / 1000) + " kg</td>";
          if (rest < 0)
            s += "<td align='center' style='color: red;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
          else
            s += "<td align='center' style='color: green;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
        }
        else {
          s += "<td align='center'>" + QString::number(ListWeitereZutatenHonig.at(i).Menge) + " g</td>";
          if (rest < 0)
            s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b> g</td>";
          else
            s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b> g</td>";
        }

        s += "</tr>";
      }
      s += "</table>";
      s += "</div>";
    }
    //WeitereZutaten Zucker Mengen anzeigen
    //Bild für Zucker anzeigen
    if (ListWeitereZutatenZucker.count() > 0){
      int Einheit=0;
      s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_1_50.png' alt='Honig' width='50px' border=0></div>";
      s += "<div align='center' style='font-size:12pt;'>";
      s += "<table border=0 cellspacing=0 >";
      s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("übrig") + "</td>";
      for (int i = 0; i < ListWeitereZutatenZucker.size(); ++i){
        double ist=0;
        bool gefunden = false;
        //Vorhandene Menge von diesem Honig
        for (int o=0; o < tableWidget_WeitereZutaten -> rowCount(); o++){
          //wenn Eintrag übereinstimmt
          if (tableWidget_WeitereZutaten -> item(o,0) -> text() == ListWeitereZutatenZucker.at(i).Name){
            QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,1);
            ist = dsbMenge -> value();
            QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(o,2);
            Einheit = comboEinheit -> currentIndex();
            if (Einheit == 0){
              ist = ist * 1000;
            }
            gefunden = true;
          }
        }
        double rest = ist - ListWeitereZutatenZucker.at(i).Menge;
        s += "<tr valign='middle'>";
        if (rest < 0)
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
        else
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
        //Rohstoff ist aufgeführt
        if (gefunden){
          s += "<td align='left'>" + ListWeitereZutatenZucker.at(i).Name + "</td>";
        }
        //Rohstoff ist nicht vorhanden
        else {
          s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenZucker.at(i).Name + "</td>";
        }
        if (Einheit == 0){
          s += "<td align='center'>" + QString::number(ListWeitereZutatenZucker.at(i).Menge / 1000) + " kg</td>";
          if (rest < 0)
            s += "<td align='center' style='color: red;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
          else
            s += "<td align='center' style='color: green;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
        }
        else {
          s += "<td align='center'>" + QString::number(ListWeitereZutatenZucker.at(i).Menge) + " g</td>";
          if (rest < 0)
            s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b> g</td>";
          else
            s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b> g</td>";
        }

        s += "</tr>";
      }
      s += "</table>";
      s += "</div>";
    }
    //WeitereZutaten Gewuerz Mengen anzeigen
    //Bild für Gewuerz anzeigen
    if (ListWeitereZutatenGewuerz.count() > 0){
      int Einheit=0;
      s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_2_50.png' alt='Honig' width='50px' border=0></div>";
      s += "<div align='center' style='font-size:12pt;'>";
      s += "<table border=0 cellspacing=0 >";
      s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("übrig") + "</td>";
      for (int i = 0; i < ListWeitereZutatenGewuerz.size(); ++i){
        double ist=0;
        bool gefunden = false;
        //Vorhandene Menge von diesem Honig
        for (int o=0; o < tableWidget_WeitereZutaten -> rowCount(); o++){
          //wenn Eintrag übereinstimmt
          if (tableWidget_WeitereZutaten -> item(o,0) -> text() == ListWeitereZutatenGewuerz.at(i).Name){
            QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,1);
            ist = dsbMenge -> value();
            QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(o,2);
            Einheit = comboEinheit -> currentIndex();
            if (Einheit == 0){
              ist = ist * 1000;
            }
            gefunden = true;
          }
        }
        double rest = ist - ListWeitereZutatenGewuerz.at(i).Menge;
        s += "<tr valign='middle'>";
        if (rest < 0)
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
        else
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
        //Rohstoff ist aufgeführt
        if (gefunden){
          s += "<td align='left'>" + ListWeitereZutatenGewuerz.at(i).Name + "</td>";
        }
        //Rohstoff ist nicht vorhanden
        else {
          s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenGewuerz.at(i).Name + "</td>";
        }
        if (Einheit == 0){
          s += "<td align='center'>" + QString::number(ListWeitereZutatenGewuerz.at(i).Menge / 1000) + " kg</td>";
          if (rest < 0)
            s += "<td align='center' style='color: red;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
          else
            s += "<td align='center' style='color: green;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
        }
        else {
          s += "<td align='center'>" + QString::number(ListWeitereZutatenGewuerz.at(i).Menge) + " g</td>";
          if (rest < 0)
            s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b> g</td>";
          else
            s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b> g</td>";
        }

        s += "</tr>";
      }
      s += "</table>";
      s += "</div>";
    }
    //WeitereZutaten Frucht Mengen anzeigen
    //Bild für Frucht anzeigen
    if (ListWeitereZutatenFrucht.count() > 0){
      int Einheit=0;
      s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_3_50.png' alt='Honig' width='50px' border=0></div>";
      s += "<div align='center' style='font-size:12pt;'>";
      s += "<table border=0 cellspacing=0 >";
      s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("übrig") + "</td>";
      for (int i = 0; i < ListWeitereZutatenFrucht.size(); ++i){
        double ist=0;
        bool gefunden = false;
        //Vorhandene Menge von diesem Honig
        for (int o=0; o < tableWidget_WeitereZutaten -> rowCount(); o++){
          //wenn Eintrag übereinstimmt
          if (tableWidget_WeitereZutaten -> item(o,0) -> text() == ListWeitereZutatenFrucht.at(i).Name){
            QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,1);
            ist = dsbMenge -> value();
            QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(o,2);
            Einheit = comboEinheit -> currentIndex();
            if (Einheit == 0){
              ist = ist * 1000;
            }
            gefunden = true;
          }
        }
        double rest = ist - ListWeitereZutatenFrucht.at(i).Menge;
        s += "<tr valign='middle'>";
        if (rest < 0)
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
        else
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
        //Rohstoff ist aufgeführt
        if (gefunden){
          s += "<td align='left'>" + ListWeitereZutatenFrucht.at(i).Name + "</td>";
        }
        //Rohstoff ist nicht vorhanden
        else {
          s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenFrucht.at(i).Name + "</td>";
        }
        if (Einheit == 0){
          s += "<td align='center'>" + QString::number(ListWeitereZutatenFrucht.at(i).Menge / 1000) + " kg</td>";
          if (rest < 0)
            s += "<td align='center' style='color: red;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
          else
            s += "<td align='center' style='color: green;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
        }
        else {
          s += "<td align='center'>" + QString::number(ListWeitereZutatenFrucht.at(i).Menge) + " g</td>";
          if (rest < 0)
            s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b> g</td>";
          else
            s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b> g</td>";
        }

        s += "</tr>";
      }
      s += "</table>";
      s += "</div>";
    }
    //WeitereZutaten Sonstiges Mengen anzeigen
    //Bild für Sonstiges anzeigen
    if (ListWeitereZutatenSonstiges.count() > 0){
      int Einheit=0;
      s += "<div align='center'><img style='padding:0px;margin:0px;' src='qrc:/ewz/ewz_typ_4_50.png' alt='Honig' width='50px' border=0></div>";
      s += "<div align='center' style='font-size:12pt;'>";
      s += "<table border=0 cellspacing=0 >";
      s += "<td></td><td></td><td align='center'>" + trUtf8("benötigt") + "</td><td align='center'>" + trUtf8("übrig") + "</td>";
      for (int i = 0; i < ListWeitereZutatenSonstiges.size(); ++i){
        double ist=0;
        bool gefunden = false;
        //Vorhandene Menge von diesem Honig
        for (int o=0; o < tableWidget_WeitereZutaten -> rowCount(); o++){
          //wenn Eintrag übereinstimmt
          if (tableWidget_WeitereZutaten -> item(o,0) -> text() == ListWeitereZutatenSonstiges.at(i).Name){
            QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(o,1);
            ist = dsbMenge -> value();
            QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(o,2);
            Einheit = comboEinheit -> currentIndex();
            if (Einheit == 0){
              ist = ist * 1000;
            }
            gefunden = true;
          }
        }
        double rest = ist - ListWeitereZutatenSonstiges.at(i).Menge;
        s += "<tr valign='middle'>";
        if (rest < 0)
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/nio_32x32.png' alt='IO' width='16px' border=0></td>";
        else
          s += "<td><img style='padding:0px;margin:0px;' src='qrc:/global/io_32x32.png' alt='IO' width='16px' border=0></td>";
        //Rohstoff ist aufgeführt
        if (gefunden){
          s += "<td align='left'>" + ListWeitereZutatenSonstiges.at(i).Name + "</td>";
        }
        //Rohstoff ist nicht vorhanden
        else {
          s += "<td align='left' style='color: grey;'>" + ListWeitereZutatenSonstiges.at(i).Name + "</td>";
        }
        if (Einheit == 0){
          s += "<td align='center'>" + QString::number(ListWeitereZutatenSonstiges.at(i).Menge / 1000) + " kg</td>";
          if (rest < 0)
            s += "<td align='center' style='color: red;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
          else
            s += "<td align='center' style='color: green;'><b>" + QString::number(rest/1000) + "</b> kg</td>";
        }
        else {
          s += "<td align='center'>" + QString::number(ListWeitereZutatenSonstiges.at(i).Menge) + " g</td>";
          if (rest < 0)
            s += "<td align='center' style='color: red;'><b>" + QString::number(rest) + "</b> g</td>";
          else
            s += "<td align='center' style='color: green;'><b>" + QString::number(rest) + "</b> g</td>";
        }

        s += "</tr>";
      }
      s += "</table>";
      s += "</div>";
    }
  }

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  settings.beginGroup("DB");
  QDir dbpfad = QDir(settings.value("DB_Pfad").toString());
  settings.endGroup();
  bool kopzeile = false;
  for (int sid = 0; sid < ListSudID.size(); ++sid){
    QString sql = "SELECT * FROM Anhang WHERE SudID=" + QString::number(ListSudID.at(sid));
    QSqlQuery query_anhang;
    if (!query_anhang.exec(sql)) {
      // Fehlermeldung Datenbankabfrage
      ErrorMessage *errorMessage = new ErrorMessage();
      errorMessage -> showMessage(ERR_SQL_DB_ABFRAGE, TYPE_WARNUNG,
                                  CANCEL_NO, trUtf8("Rückgabe:\n") + query_anhang.lastError().databaseText()
                                  + trUtf8("\nSQL Befehl:\n") + sql);
    }
    else {
      while (query_anhang.next()){
        if (!kopzeile) {
          s += "<div class='r' style='margin-bottom:10px;' align='center'><p class='h1'><b>" + trUtf8("Anhänge") + "</b></p></div>";
          kopzeile = true;
        }
        int FeldNr = query_anhang.record().indexOf("Pfad");
        QString pfad = query_anhang.value(FeldNr).toString();
        if (QDir::isRelativePath(pfad))
          pfad = dbpfad.filePath(pfad);
        if (AnhangWidget::isImage(pfad))
          s += "<img style=\"max-width:" + QString::number(webView_Info->width() - 10) + "px;\" src=\"file:///" + pfad + "\"></br>";
        else
          s += "<a href=\"file:///" + pfad + "\" target=\"_blank\">" + pfad + "</a></br>";
      }
    }
  }

  seite += s;
  //Seitenende
  ende = "</body></html>";
  seite += ende;

  webView_Info -> setRenderHint(QPainter::TextAntialiasing, true);
  webView_Info -> setRenderHint(QPainter::SmoothPixmapTransform, true);
  webView_Info -> setRenderHint(QPainter::HighQualityAntialiasing, true);
  webView_Info -> setHtml(seite,QUrl::fromLocalFile(QCoreApplication::applicationDirPath()+"/"));
}

void MainWindowImpl::slot_urlClicked(const QUrl &url)
{
  QDesktopServices::openUrl(url);
}

void MainWindowImpl::on_TabWidget_Zutaten_currentChanged(int index)
{
  TabWidget_RezeptErgebnisse -> setCurrentIndex(index);
}

void MainWindowImpl::on_pushButton_WeitereZutatenNeu_clicked()
{
  QString s = WZutaten_Bezeichnung_Merker;
  WZutaten_Bezeichnung_Merker = "";
  tableWidget_WeitereZutaten->setSortingEnabled(false);

  QTableWidgetItem *newItem1 = new QTableWidgetItem("Neuer Eintrag");
  QTableWidgetItem *newItem7 = new QTableWidgetItem(" ");

  int i = tableWidget_WeitereZutaten -> rowCount();
  tableWidget_WeitereZutaten -> setRowCount(i+1);

  //Beschreibung
  tableWidget_WeitereZutaten -> setItem(i, 0, newItem1);

  //Menge
  MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
  spinBoxMenge -> setMinimum(0);
  spinBoxMenge -> setMaximum(10000);
  connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 1, spinBoxMenge);

  //Combobox Einheiten
  MyComboBox *comboBoxEinheiten = new MyComboBox();
  comboBoxEinheiten -> addItems(EinheitenListe);
  connect(comboBoxEinheiten, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  connect(comboBoxEinheiten, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 2, comboBoxEinheiten);

  //Combobox Typ
  MyComboBox *comboBoxTyp = new MyComboBox();
  comboBoxTyp -> addItems(ZutatenTypListe);
  connect(comboBoxTyp, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  connect(comboBoxTyp, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 3, comboBoxTyp);

  //Ausbeute
  MyDoubleSpinBox *spinBoxAusbeute = new MyDoubleSpinBox();
  spinBoxAusbeute -> setMinimum(0);
  spinBoxAusbeute -> setMaximum(100);
  spinBoxAusbeute -> setDecimals(0);
  spinBoxAusbeute -> setToolTip(trUtf8("Anteil an der Stamwürze in Prozent: 0=Wird bei der Berechnung der Stammwürze nicht berücksichtigt"));
  connect(spinBoxAusbeute, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBoxAusbeute, SIGNAL( valueChanged(double) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 4, spinBoxAusbeute);

  //EBC
  MyDoubleSpinBox *spinBoxEBC = new MyDoubleSpinBox();
  spinBoxEBC -> setMinimum(0);
  spinBoxEBC -> setMaximum(100000);
  spinBoxEBC -> setDecimals(1);
  spinBoxEBC -> setToolTip(trUtf8("Farbwert: 0=Wird bei der berechnung der Farbe nicht berücksichtigt"));
  connect(spinBoxEBC, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBoxEBC, SIGNAL( valueChanged(double) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 5, spinBoxEBC);

  //Preis
  MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
  spinBoxPreis -> setMinimum(0);
  spinBoxPreis -> setMaximum(1000);
  spinBoxPreis -> setToolTip(trUtf8("Preis pro Kilogramm"));
  connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 6, spinBoxPreis);

  tableWidget_WeitereZutaten -> setItem(i, 7, newItem7);

  //Datum Eingelagert
  QDateEdit * deEinlagerung = new QDateEdit(QDate::currentDate());
  deEinlagerung->setDisplayFormat("dd.MM.yyyy");
  deEinlagerung->setCalendarPopup(true);
  connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 8, deEinlagerung);

  //Mindesthaltbarkeitsdatum
  QDateEdit * deMhd = new QDateEdit(QDate::currentDate());
  deMhd->setDisplayFormat("dd.MM.yyyy");
  deMhd->setCalendarPopup(true);
  connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 9, deMhd);

  //Link
  QTableWidgetItem *newItemLink = new QTableWidgetItem("");
  newItemLink->setTextColor(Qt::blue);
  tableWidget_WeitereZutaten -> setItem(i, 10, newItemLink);

  tableWidget_WeitereZutaten->setSortingEnabled(true);
  WZutaten_Bezeichnung_Merker = s;
  setAenderung(true);
  AenderungRohstofftabelle = true;
}

void MainWindowImpl::on_pushButton_WeitereZutatenKopie_clicked()
{
  //Aktuelle Zeile
  int i = tableWidget_WeitereZutaten -> currentRow();
  QString s = WZutaten_Bezeichnung_Merker;
  WZutaten_Bezeichnung_Merker = "";
  tableWidget_WeitereZutaten->setSortingEnabled(false);

  //Aktuelle zeile auslesen
  QDoubleSpinBox* dsbMenge=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,1);
  QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(i,2);
  QComboBox* comboTyp=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(i,3);
  QDoubleSpinBox* dsbAusbeute=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,4);
  QDoubleSpinBox* dsbEBC=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,5);
  QDoubleSpinBox* dsbPreis=(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,6);
  QTableWidgetItem *newItem1 = new QTableWidgetItem(tableWidget_WeitereZutaten -> item(i,0) -> text());
  QTableWidgetItem *newItem7 = new QTableWidgetItem(tableWidget_WeitereZutaten -> item(i,7) -> text());


  i = tableWidget_WeitereZutaten -> rowCount();
  tableWidget_WeitereZutaten -> setRowCount(i+1);

  //Beschreibung
  tableWidget_WeitereZutaten -> setItem(i, 0, newItem1);

  //Menge
  MyDoubleSpinBox *spinBoxMenge = new MyDoubleSpinBox();
  spinBoxMenge -> setMinimum(0);
  spinBoxMenge -> setMaximum(10000);
  spinBoxMenge -> setValue(dsbMenge -> value());
  connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBoxMenge, SIGNAL( valueChanged(double) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 1, spinBoxMenge);

  //Combobox Einheiten
  MyComboBox *comboBoxEinheiten = new MyComboBox();
  comboBoxEinheiten -> addItems(EinheitenListe);
  comboBoxEinheiten -> setCurrentIndex(comboEinheit -> currentIndex());
  connect(comboBoxEinheiten, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  connect(comboBoxEinheiten, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 2, comboBoxEinheiten);

  //Combobox Typ
  MyComboBox *comboBoxTyp = new MyComboBox();
  comboBoxTyp -> addItems(ZutatenTypListe);
  comboBoxTyp -> setCurrentIndex(comboTyp -> currentIndex());
  connect(comboBoxTyp, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_ComboBoxIndexChanged(int) ));
  connect(comboBoxTyp, SIGNAL( currentIndexChanged(int) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 3, comboBoxTyp);

  //Ausbeute
  MyDoubleSpinBox *spinBoxAusbeute = new MyDoubleSpinBox();
  spinBoxAusbeute -> setMinimum(0);
  spinBoxAusbeute -> setMaximum(100);
  spinBoxAusbeute -> setDecimals(0);
  spinBoxAusbeute -> setValue(dsbAusbeute -> value());
  spinBoxAusbeute -> setToolTip(trUtf8("Anteil an der Stamwürze in Prozent: 0=Wird bei der Berechnung der Stammwürze nicht berücksichtigt"));
  connect(spinBoxAusbeute, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBoxAusbeute, SIGNAL( valueChanged(double) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 4, spinBoxAusbeute);

  //EBC
  MyDoubleSpinBox *spinBoxEBC = new MyDoubleSpinBox();
  spinBoxEBC -> setMinimum(0);
  spinBoxEBC -> setMaximum(100000);
  spinBoxEBC -> setDecimals(1);
  spinBoxEBC -> setValue(dsbEBC -> value());
  spinBoxEBC -> setToolTip(trUtf8("Farbwert: 0=Wird bei der berechnung der Farbe nicht berücksichtigt"));
  connect(spinBoxEBC, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBoxEBC, SIGNAL( valueChanged(double) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 5, spinBoxEBC);

  //Preis
  MyDoubleSpinBox *spinBoxPreis = new MyDoubleSpinBox();
  spinBoxPreis -> setMinimum(0);
  spinBoxPreis -> setMaximum(1000);
  spinBoxPreis -> setValue(dsbPreis -> value());
  spinBoxPreis -> setToolTip(trUtf8("Preis pro Kilogramm"));
  connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_spinBoxValueChanged(double) ));
  connect(spinBoxPreis, SIGNAL( valueChanged(double) ), this, SLOT( slot_EwzAenderungRohstoffe() ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 6, spinBoxPreis);

  //Bemerkung
  tableWidget_WeitereZutaten -> setItem(i, 7, newItem7);

  //Datum Eingelagert
  QDateEdit * deEinlagerung = new QDateEdit(QDate::currentDate());
  deEinlagerung->setDisplayFormat("dd.MM.yyyy");
  deEinlagerung->setCalendarPopup(true);
  connect(deEinlagerung, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 8, deEinlagerung);

  //Mindesthaltbarkeitsdatum
  QDateEdit * deMhd = new QDateEdit(QDate::currentDate());
  deMhd->setDisplayFormat("dd.MM.yyyy");
  deMhd->setCalendarPopup(true);
  connect(deMhd, SIGNAL( dateChanged(QDate) ), this, SLOT( slot_dateChanged(QDate) ));
  tableWidget_WeitereZutaten -> setCellWidget(i, 9, deMhd);

  WZutaten_Bezeichnung_Merker = s;
  setAenderung(true);
  AenderungRohstofftabelle = true;
  tableWidget_WeitereZutaten->setSortingEnabled(true);

}

void MainWindowImpl::on_pushButton_WeitereZutatenDel_clicked()
{
  //Überprüfen ob bei nicht gebrauten Suden der Rohstoff verwendet wird.

  //Rohstoffname
  QString del_name = tableWidget_WeitereZutaten -> item(tableWidget_WeitereZutaten -> currentRow(),0) -> text();

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
              if (del_name != tableWidget_WeitereZutaten -> item(i,0) -> text()){
                raDia.addAuswahlEintrag(tableWidget_WeitereZutaten -> item(i,0) -> text() );
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
  ewz -> setAttribute(Qt::WA_DeleteOnClose);
  //Ergebnisswidget ersetellen
  doubleEditLineImpl* berEwz = new doubleEditLineImpl(this);
  berEwz -> setAttribute(Qt::WA_DeleteOnClose);

  ewz -> ergWidget = berEwz;
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
  //Zutatenliste füllen
  ewz -> setEwListe(ewzListe);
  ewz -> setHopfenListe(HopfenListe);

  verticalLayout_WeitereZutaten -> addWidget(ewz);
  list_EwZutat.append(ewz);
  ewz -> setID((int)time(NULL)+rand());

  //Ergebnisswidget dem Layout zuordnen
  verticalLayout_BerWeitereZutaten -> addWidget(berEwz);

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
    ewzListe.append(tableWidget_WeitereZutaten -> item(i,0) -> text());
  }
  //Alle Hopfeneintäge hinzufügen
  HopfenListe.clear();
  for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
    HopfenListe.append(tableWidget_Hopfen -> item(i,0) -> text());
  }
  //Liste für Malzeinträge
  MalzListe.clear();
  for (int i=0; i < tableWidget_Malz -> rowCount(); i++){
    MalzListe.append(tableWidget_Malz -> item(i,0) -> text());
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
    if (tableWidget_WeitereZutaten -> item(i,0) -> text() == zutat){
      QComboBox* comboTyp=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(i,3);
      return comboTyp -> currentIndex();
    }
  }
  return -1;
}


int MainWindowImpl::slot_getEwzEinheit(QString zutat)
{
  for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
    if (tableWidget_WeitereZutaten -> item(i,0) -> text() == zutat){
      QComboBox* comboEinheit=(QComboBox*)tableWidget_WeitereZutaten -> cellWidget(i,2);
      return comboEinheit -> currentIndex();
    }
  }
  return -1;
}


int MainWindowImpl::slot_getEwzAusbeute(QString zutat)
{
  for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
    if (tableWidget_WeitereZutaten -> item(i,0) -> text() == zutat){
      QDoubleSpinBox *spinBoxAusbeute =(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,4);
      return spinBoxAusbeute -> value();
    }
  }
  return -1;
}


double MainWindowImpl::slot_getEwzFarbe(QString zutat)
{
  for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
    if (tableWidget_WeitereZutaten -> item(i,0) -> text() == zutat){
      QDoubleSpinBox *spinBoxFarbe =(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,5);
      return spinBoxFarbe -> value();
    }
  }
  return -1;
}


double MainWindowImpl::slot_getEwzPreis(QString zutat)
{
  for (int i=0; i < tableWidget_WeitereZutaten -> rowCount(); i++){
    if (tableWidget_WeitereZutaten -> item(i,0) -> text() == zutat){
      QDoubleSpinBox *spinBoxPreis =(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,6);
      return spinBoxPreis -> value();
    }
  }
  return -1;
}


double MainWindowImpl::slot_getEwzPreisHopfen(QString zutat)
{
  for (int i=0; i < tableWidget_Hopfen -> rowCount(); i++){
    if (tableWidget_Hopfen -> item(i,0) -> text() == zutat){
      QDoubleSpinBox *spinBox = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,3);
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
        if (tableWidget_Hopfen -> item(i,0) -> text() == zutat){
          QDoubleSpinBox* dsbMenge = (QDoubleSpinBox*)tableWidget_Hopfen -> cellWidget(i,2);
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
        if (tableWidget_WeitereZutaten -> item(i,0) -> text() == zutat){
          QDoubleSpinBox *dsbMenge =(QDoubleSpinBox*)tableWidget_WeitereZutaten -> cellWidget(i,1);
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
    sql += "Bemerkung, erg_Menge, Ausbeute, Zeitpunkt_von, Zeitpunkt_bis, Entnahmeindex, Zugabestatus, Farbe) VALUES(" +
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
  connect(rast, SIGNAL( sig_nachOben(int) ), this, SLOT( on_pushButton_RastNachOben(int) ));
  connect(rast, SIGNAL( sig_nachUnten(int) ), this, SLOT( on_pushButton_RastNachUnten(int) ));
}

void MainWindowImpl::on_pushButton_RastNachOben(int id)
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

void MainWindowImpl::on_pushButton_RastNachUnten(int id)
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
  if (Gestartet){
    spinBox_JungbiermengeAbfuellen -> setValue(arg1);
  }
}

void MainWindowImpl::on_spinBox_SWKochende_valueChanged(double arg1)
{
  spinBox_SWAnstellen -> setValue(arg1);
}

void MainWindowImpl::on_spinBox_WuerzemengeKochende_valueChanged(double arg1)
{
  spinBox_WuerzemengeAnstellen -> setValue(arg1 - spinBox_Speisemenge -> value());
}

void MainWindowImpl::on_spinBox_Speisemenge_valueChanged(double arg1)
{
  spinBox_WuerzemengeAnstellen -> setValue(spinBox_WuerzemengeKochende->value() - arg1);
}

void MainWindowImpl::on_spinBox_SW_valueChanged(double arg1)
{
  spinBox_SWVorHopfenseihen -> setValue(arg1);
  spinBox_SWKochende -> setValue(arg1);
}

void MainWindowImpl::on_spinBox_WuerzemengeVorHopfenseihen_valueChanged(double arg1)
{
  spinBox_WuerzemengeKochende -> setValue(arg1);
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
    QTableWidgetItem *newItem = tableWidget_Malz -> item(row,0);

    if (newItem->text() == ""){
      newItem->setText(trUtf8("Bitte eine Bezeichnung angeben"));
    }

    //Hier eine Überprüfung vornehmen ob der Name doppelt vorkommt
    bool doppelt = true;
    while (doppelt){
      doppelt = false;
      for (int i=0; i < tableWidget_Malz->rowCount(); i++){
        if (i != row){
          if (tableWidget_Malz->item(i,0)->text() == newItem->text()){
            doppelt = true;
            qDebug() << "Malzeintrag ist doppelt: " << newItem->text();
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
    QTableWidgetItem *newItem = tableWidget_Hopfen -> item(row,0);

    if (newItem->text() == ""){
      newItem->setText(trUtf8("Bitte eine Bezeichnung angeben"));
    }

    //Hier eine Überprüfung vornehmen ob der Name doppelt vorkommt
    bool doppelt = true;
    while (doppelt){
      doppelt = false;
      for (int i=0; i < tableWidget_Hopfen->rowCount(); i++){
        if (i != row){
          if (tableWidget_Hopfen->item(i,0)->text() == newItem->text()){
            doppelt = true;
            qDebug() << "Hopfeneintrag ist doppelt: " << newItem->text();
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
    QTableWidgetItem *newItem = tableWidget_Hefe -> item(row,0);

    if (newItem->text() == ""){
      newItem->setText(trUtf8("Bitte eine Bezeichnung angeben"));
    }

    //Hier eine Überprüfung vornehmen ob der Name doppelt vorkommt
    bool doppelt = true;
    while (doppelt){
      doppelt = false;
      for (int i=0; i < tableWidget_Hefe->rowCount(); i++){
        if (i != row){
          if (tableWidget_Hefe->item(i,0)->text() == newItem->text()){
            doppelt = true;
            qDebug() << "Hefeeintrag ist doppelt: " << newItem->text();
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
    QTableWidgetItem *newItem = tableWidget_WeitereZutaten -> item(row,0);

    if (newItem->text() == ""){
      newItem->setText(trUtf8("Bitte eine Bezeichnung angeben"));
    }

    //Hier eine Überprüfung vornehmen ob der Name doppelt vorkommt
    bool doppelt = true;
    while (doppelt){
      doppelt = false;
      for (int i=0; i < tableWidget_WeitereZutaten->rowCount(); i++){
        if (i != row){
          if (tableWidget_WeitereZutaten->item(i,0)->text() == newItem->text()){
            doppelt = true;
            qDebug() << "Eintrag in den Weiteren Zutaten ist doppelt: " << newItem->text();
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
    Malz_Bezeichnung_Merker = newItem->text();
  }
}

void MainWindowImpl::on_tableWidget_Hopfen_currentCellChanged(int currentRow, int currentColumn, int , int )
{
  //wenn eine Zelle von der Beschreibung angewählt wurde Text merken für späteren vergleich ob sich Beschreibungstext geändert hat
  //und Sude angepasst werden müssen
  if (currentColumn == 0) {
    QTableWidgetItem *newItem = tableWidget_Hopfen -> item(currentRow,currentColumn);
    Hopfen_Bezeichnung_Merker = newItem->text();
  }
}


void MainWindowImpl::on_tableWidget_Hefe_currentCellChanged(int currentRow, int currentColumn, int , int )
{
  //wenn eine Zelle von der Beschreibung angewählt wurde Text merken für späteren vergleich ob sich Beschreibungstext geändert hat
  //und Sude angepasst werden müssen
  if (currentColumn == 0) {
    QTableWidgetItem *newItem = tableWidget_Hefe -> item(currentRow,currentColumn);
    Hefe_Bezeichnung_Merker = newItem->text();
  }
}

void MainWindowImpl::on_tableWidget_WeitereZutaten_currentCellChanged(int currentRow, int currentColumn, int , int )
{
  //wenn eine Zelle von der Beschreibung angewählt wurde Text merken für späteren vergleich ob sich Beschreibungstext geändert hat
  //und Sude angepasst werden müssen
  if (currentColumn == 0) {
    QTableWidgetItem *newItem = tableWidget_WeitereZutaten -> item(currentRow,currentColumn);
    WZutaten_Bezeichnung_Merker = newItem->text();
  }
}

void MainWindowImpl::on_tableWidget_Malz_itemSelectionChanged()
{
  //Buttons zum Laden etc. ein/Ausblenden
  if (tableWidget_Malz -> selectedItems().count() == 3) {
    //Alle Buttons enablen
    pushButton_MalzKopie -> setDisabled(false);
    pushButton_MalzDel -> setDisabled(false);
  }
  else {
    pushButton_MalzKopie -> setDisabled(true);
    pushButton_MalzDel -> setDisabled(true);
  }

}

void MainWindowImpl::on_tableWidget_Hopfen_itemSelectionChanged()
{
  //Buttons zum Laden etc. ein/Ausblenden
  //qDebug() << "count: " << tableWidget_Hopfen -> selectedItems().count();
  if (tableWidget_Hopfen -> selectedItems().count() == 3) {
    //Alle Buttons enablen
    pushButton_HopfenKopie -> setDisabled(false);
    pushButton_HopfenDel -> setDisabled(false);
  }
  else {
    pushButton_HopfenKopie -> setDisabled(true);
    pushButton_HopfenDel -> setDisabled(true);
  }
}

void MainWindowImpl::on_tableWidget_Hefe_itemSelectionChanged()
{
  //Buttons zum Laden etc. ein/Ausblenden
  //qDebug() << "count: " << tableWidget_Hopfen -> selectedItems().count();
  if (tableWidget_WeitereZutaten -> selectedItems().count() == 3) {
    //Alle Buttons enablen
    pushButton_WeitereZutatenKopie -> setDisabled(false);
    pushButton_WeitereZutatenDel -> setDisabled(false);
  }
  else {
    pushButton_WeitereZutatenKopie -> setDisabled(true);
    pushButton_WeitereZutatenDel -> setDisabled(true);
  }
}

void MainWindowImpl::on_tableWidget_WeitereZutaten_itemSelectionChanged()
{
  //Buttons zum Laden etc. ein/Ausblenden
  if (tableWidget_WeitereZutaten -> selectedItems().count() == 2) {
    //Alle Buttons enablen
    pushButton_WeitereZutatenDel -> setDisabled(false);
    pushButton_WeitereZutatenKopie -> setDisabled(false);
  }
  else {
    pushButton_WeitereZutatenDel -> setDisabled(true);
    pushButton_WeitereZutatenKopie -> setDisabled(true);
  }
}



void MainWindowImpl::on_spinBox_Menge_valueChanged(double arg1)
{
  spinBox_WuerzemengeVorHopfenseihen -> setValue(arg1);
  spinBox_WuerzemengeKochende -> setValue(arg1);

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

  //Brauübersicht
  if (index == 7){
    FuelleBrauuebersicht();
  }
  //Ausrüstung
  else if (index == 8){
    BerEffektiveAusbeuteMittel();
  }
  //Gärverlauf
  else if (index == 4){
    FuelleGaerverlauf();
  }
  //Zusammenfassung
  if (index == 5){
    //Seite Spickzettel erstellen
    ErstelleTabSpickzettel();
  }
  //Spickzettel
  if (index == 5){
    //Seite Spickzettel erstellen
    ErstelleTabSpickzettel();
  }
  //Brau && Gärdaten
  else if (index == 3){
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
  else if (index == 10){
    if (!keinInternet)
      webView_Anleitung -> setUrl(QUrl(URL_ANLEITUNG));
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
  bver.exec();
  if (!bver.abgebrochen) {
    doubleSpinBox_Verdampfung->setValue(bver.getVerdampfungsziffer());
  }
}

void MainWindowImpl::on_pushButton_SudinfoDrucken_clicked()
{
  bool merker = StyleDunkel;
  if (StyleDunkel) {
    StyleDunkel = false;
    ErstelleSudInfo();
  }

  QPrinter printer(QPrinter::HighResolution);
  printer.setColorMode(QPrinter::Color);

  QPrintDialog *dialog = new QPrintDialog(&printer, this);
  dialog->setWindowTitle("Print");
  if (dialog->exec() != QDialog::Accepted){
    //webView_Info->setZoomFactor(1);
  }
  else {
    //Drucken
    webView_Info->print(&printer);
    webView_Info->setTextSizeMultiplier(1);
  }

  if (merker) {
    StyleDunkel = merker;
    ErstelleSudInfo();
  }
}

void MainWindowImpl::on_pushButton_SudinfoPDF_clicked()
{
  bool merker = StyleDunkel;
  if (StyleDunkel) {
    StyleDunkel = false;
    ErstelleSudInfo();
  }

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  settings.beginGroup("PDF");
  //letzten Pfad einlesen
  QString p;
  p = settings.value("recentPDFPath").toString();
  if (p == "") {
    p = QDir::homePath();
  }

  //printer einstellungen
  QPrinter printer(QPrinter::HighResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setColorMode(QPrinter::Color);
  printer.setResolution(1200);
  QFileDialog fd(this);

  //QString fileName = fd.getSaveFileName(this, trUtf8("PDF Datei Speichern unter"), p, trUtf8("Suddateien (*.pdf)"),0,QFileDialog::DontUseNativeDialog);
  QString fileName = fd.getSaveFileName(this, trUtf8("PDF Datei Speichern unter"), p + "/Rohstoffliste.pdf", trUtf8("Suddateien (*.pdf)"),0);
  if (!fileName.isEmpty()) {
    printer.setOutputFileName(fileName);
    //pdf speichern
    webView_Info -> print(&printer);

    //Pfad abspeichern
    QFileInfo fi(fileName);
    settings.setValue("recentPDFPath",fi.absolutePath());

    //PDF Betrachter starten
    if (settings.value("startPDFBetrachter").toBool()) {
      QString prog = settings.value("PDFProg").toString();
      QFileInfo fi(prog);
      if (fi.exists()) {
        QStringList arguments;
        arguments << fileName;
        QProcess *myProcess = new QProcess();
        qDebug() << "starte PDF Betrachter: " << prog << " " << arguments;
        myProcess->start(prog,arguments);
      }
    }
  }

  settings.endGroup();
  if (merker) {
    StyleDunkel = merker;
    ErstelleSudInfo();
  }
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
    QString text = trUtf8("alle vergessen");
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

void MainWindowImpl::on_listWidget_Brauanlagen_currentRowChanged(int)
{
}


void MainWindowImpl::on_pushButton_VerschneidungZumischen_clicked()
{
  spinBox_WuerzemengeAnstellen->setValue(spinBox_WuerzemengeKochende->value() + spinBox_WasserVerschneidung->value()-spinBox_Speisemenge->value());
}

void MainWindowImpl::on_pushButton_MalzNeu_clicked()
{
  tableWidget_Malz->setSortingEnabled(false);
  MalzNeueZeile();
  tableWidget_Malz->setSortingEnabled(true);
}

void MainWindowImpl::on_pushButton_HopfenNeu_clicked()
{
  tableWidget_Hopfen->setSortingEnabled(false);
  HopfenNeueZeile();
  tableWidget_Hopfen->setSortingEnabled(true);
}

void MainWindowImpl::on_pushButton_HefeNeu_clicked()
{
  tableWidget_Hefe->setSortingEnabled(false);
  HefeNeueZeile();
  tableWidget_Hefe->setSortingEnabled(true);
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
  if (column == 9) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
      QDesktopServices::openUrl(QUrl(tableWidget_Malz->item(row, column)->text()));
  }
}

void MainWindowImpl::on_tableWidget_Hopfen_cellClicked(int row, int column)
{
  if (column == 10) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
      QDesktopServices::openUrl(QUrl(tableWidget_Hopfen->item(row, column)->text()));
  }
}

void MainWindowImpl::on_tableWidget_Hefe_cellClicked(int row, int column)
{
  if (column == 14) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
      QDesktopServices::openUrl(QUrl(tableWidget_Hefe->item(row, column)->text()));
  }
}

void MainWindowImpl::on_tableWidget_WeitereZutaten_cellClicked(int row, int column)
{
  if (column == 10) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
      QDesktopServices::openUrl(QUrl(tableWidget_WeitereZutaten->item(row, column)->text()));
  }
}

void MainWindowImpl::on_pushButton_CO2_Info_clicked()
{
  DialogInfo::Info(this, INFO_CO2_TITLE, INFO_CO2_TEXT);
}

void MainWindowImpl::on_pushButton_IBU_Info_clicked()
{
  DialogInfo::Info(this, INFO_IBU_TITLE, INFO_IBU_TEXT);
}

void MainWindowImpl::on_pushButton_SW_Info_clicked()
{
  DialogInfo::Info(this, INFO_SW_TITLE, INFO_SW_TEXT);
}

void MainWindowImpl::on_pushButton_High_Gravity_Info_clicked()
{
  DialogInfo::Info(this, INFO_HIGHGRAVITY_TITLE, INFO_HIGHGRAVITY_TEXT);
}

void MainWindowImpl::on_pushButton_NeuerAnhang_clicked()
{
  AddAnhang("");
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
  anhang->setPfad(pfad);
  anhang->setID((int)time(NULL)+rand());

  verticalLayout_Anhang -> addWidget(anhang);
  list_Anhang.append(anhang);

  connect(anhang, SIGNAL( sig_vorClose(int) ), this, SLOT( slot_anhangClose(int) ));
  connect(anhang, SIGNAL( sig_Aenderung() ), this, SLOT( slot_anhangAenderung() ));
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
