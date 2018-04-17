#ifndef MAINWINDOWIMPL_H
#define MAINWINDOWIMPL_H
//
#include <QMainWindow>
#include <QCloseEvent>
#include <QMenu>
#include <QAction>
#include <QList>
#include <QTranslator>
#include "ui_mainwindow.h"
#include "berechnungen.h"
#include "qexport.h"
#include "eingabehvolumenimpl.h"
#include "erweitertezutatimpl.h"
#include "rastwidget.h"
#include "anhangwidget.h"
#include "doubleeditlineimpl.h"
#include "bewertung.h"
#include "hopfengabe.h"
#include "malzgabe.h"
#include "msgdialog.h"

//
class MainWindowImpl : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

    enum TableMalzCol
    {
        TableMalzColName,
        TableMalzColMenge,
        TableMalzColFarbe,
        TableMalzColMaxSchuettung,
        TableMalzColAnwendung,
        TableMalzColBemerkung,
        TableMalzColPreis,
        TableMalzColEinlagerung,
        TableMalzColMindesthalbat,
        TableMalzColLink
    };
    enum TableHopfenCol
    {
        TableHopfenColName,
        TableHopfenColMenge,
        TableHopfenColAlpha,
        TableHopfenColPellets,
        TableHopfenColTyp,
        TableHopfenColEigenschaften,
        TableHopfenColBemerkung,
        TableHopfenColPreis,
        TableHopfenColEinlagerung,
        TableHopfenColMindesthalbat,
        TableHopfenColLink
    };
    enum TableHefeCol
    {
        TableHefeColName,
        TableHefeColMenge,
        TableHefeColWuerzemenge,
        TableHefeColVerpackungsmenge,
        TableHefeColTypUgOg,
        TableHefeColTypTrFlg,
        TableHefeColTemperatur,
        TableHefeColEigenschaften,
        TableHefeColSedimentation,
        TableHefeColEVG,
        TableHefeColBemerkung,
        TableHefeColPreis,
        TableHefeColEinlagerung,
        TableHefeColMindesthalbat,
        TableHefeColLink
    };
    enum TableWZutatCol
    {
        TableWZutatColName,
        TableWZutatColMenge,
        TableWZutatColEinheit,
        TableWZutatColTyp,
        TableWZutatColAusbeute,
        TableWZutatColFarbe,
        TableWZutatColBemerkung,
        TableWZutatColPreis,
        TableWZutatColEinlagerung,
        TableWZutatColMindesthalbat,
        TableWZutatColLink
    };

protected:
  void closeEvent(QCloseEvent *event);
  void changeEvent(QEvent *event);
private:
  void initUi();
  void retranslate();
  //gibt die ID der im Rezept ausgewählten Brauanlage zurück
  int getBrauanlagenIDRezept();
  //gibt die ID der in der Ausrüstung ausgewählten Brauanlage zurück
  int getBrauanlagenIDAusruestung();
  //gibt die Angenommene sudhausausbeute der Ausgewählten Brauanlage zurück
  double getAngenommeneSudhausausbeute();
  //setzt die Angenommene sudhausausbeute der Ausgewählten Brauanlage
  void setAngenommeneSudhausausbeute(int value);
  //gibt die Korrektur Wassermenge der Ausgewählten Brauanlage zurück
  double getKorrekturWassermenge();
  //setzt die Korrektur Wassermenge der Ausgewählten Brauanlage
  void setKorrekturWassermenge(double value);
  //gibt die Korrektur Wassermenge der Ausgewählten Brauanlage zurück
  int getKorrekturFarbe();
  //setzt die Korrektur Wassermenge der Ausgewählten Brauanlage
  void setKorrekturFarbe(int value);
  //gibt die Verdampfungsziffer der Ausgewählten Brauanlage zurück
  double getVerdampfungsziffer();
  //setzt die Verdampfungsziffer der Ausgewählten Brauanlage
  void setVerdampfungsziffer(double value);
  //gibt die Kosten der Ausgewählten Brauanlage zurück
  double getBrauanlageKosten();
  //setzt die Kosten der Ausgewählten Brauanlage
  void setBrauanlageKosten(double value);
  //gibt die MaischebottichHoehe der Ausgewählten Brauanlage zurück
  double getMaischebottichHoehe();
  //setzt die MaischebottichHoehe der Ausgewählten Brauanlage
  void setMaischebottichHoehe(double value);
  //gibt die MaischebottichHoehe der Ausgewählten Brauanlage zurück
  double getMaischebottichDurchmesser();
  //setzt die MaischebottichHoehe der Ausgewählten Brauanlage
  void setMaischebottichDurchmesser(double value);
  //gibt die MaischebottichHoehe der Ausgewählten Brauanlage zurück
  double getMaischebottichMaxFuellhoehe();
  //setzt die MaischebottichHoehe der Ausgewählten Brauanlage
  void setMaischebottichMaxFuellhoehe(double value);

  //gibt die MaischebottichHoehe der Ausgewählten Brauanlage zurück
  double getSudpfanneHoehe();
  //setzt die MaischebottichHoehe der Ausgewählten Brauanlage
  void setSudpfanneHoehe(double value);
  //gibt die MaischebottichHoehe der Ausgewählten Brauanlage zurück
  double getSudpfanneDurchmesser();
  //setzt die MaischebottichHoehe der Ausgewählten Brauanlage
  void setSudpfanneDurchmesser(double value);
  //gibt die MaischebottichHoehe der Ausgewählten Brauanlage zurück
  double getSudpfanneMaxFuellhoehe();
  //setzt die MaischebottichHoehe der Ausgewählten Brauanlage
  void setSudpfanneMaxFuellhoehe(double value);

  //gibt das MaischenMaxNutzvolumen der Ausgewählten Brauanlage zurück
  double getMaischenMaxNutzvolumen();
  //gibt das SudpfanneMaxNutzvolumen der Ausgewählten Brauanlage zurück
  double getSudpfanneMaxNutzvolumen();

  //Ladet die Sprache
  void switchTranslator(QTranslator& translator, const QString& filename);
  void loadSprache(const QString& rLanguage);
	QTranslator m_translator;   /*< contains the translations for this application */
	QTranslator m_translatorQt; /*< contains the translations for qt */
	QString m_currLang;     /*< contains the currently loaded language */
  double highGravityFaktor;
  bool keinInternet;
  int MaxAnzahlSterne;
  void LeseMaxAnzahlSterne();
	void ErstelleSprachMenu();
  int NeuBerechnen;
  //Dialog zum Anzeigen der Messages
  MsgDialog msgdlg;
  //Ruft den Message Dialog auf
  void checkMsg();
  //Stammwürzenanteil die durch die Erweiterten Zutaten erreicht wird
  double sw_ewz;
  //Stammwürzeanteil der durch die Erweiterten Zutaten beim gären erreicht wird
  double sw_ewz_gaerung;
  //Stammwürze beim Kochen
  double sw_kochen;
  //Stammwürze Schüttung
  double sw_schuettung;
  //Stammwürze gesammt
  double sw_gesammt;
  void setAenderung(bool value);
  void BerWeitereZutaten();
  //füllt die comboauswahl für Weitere Zutaten zugeben
  void fuelleComboEwzZugeben();
  void fuelleComboEwzEntnehmen();
  void SchreibeErweiterteZutatenDB();
  QIcon appIcon;
  void ErstelleZutatenlisten();
  QList<ErweiterteZutatImpl *> list_EwZutat;
  QList<hopfengabe *> list_Hopfengaben;
  QList<malzgabe *> list_Malzgaben;
  //Ermittelt den Bewertungsindex in der Liste
  int getBewertungsIndex();
  //Diese Liste enhält alle Bewertungen
  QList<Bewertung *> list_Bewertung;
  QList<Rastwidget *> list_Rasten;
  QList<AnhangWidget *> list_Anhang;
  //Liste Auswählbare Erweiterte Zutaten
  QStringList ewzListe;
  QStringList HopfenListe;
  QStringList MalzListe;
  void ErstelleSudInfo();
  bool AenderungAusruestung;
  bool AenderungGeraeteliste;
  bool AenderungHauptgaerverlauf;
  void BerRestalkalitaet();
  void BerWasserwerte();
  void CheckPfannevoll();
  void CheckGesammtMaischeMenge();
  void BerPfanneVoll();
  void BerVolumenMaischen();
  void BerEmpfehlungFaktorHauptguss();
  void BerEmpfehlungHefeEinheiten();
  void setHefeAuswahlListeFarbe();
  void BerAusruestung();
  void DBErgebnisseNeuBerechnen();
  bool CheckDBNeuBerechnen();
  void SetDisabledVerbraucht(bool status);
  void ErstelleZusammenfassung();
  QColor ColorBrauUebersicht_Spalte1;
  QColor ColorBrauUebersicht_Spalte2;
  QColor ColorBierReift;
  QColor ColorBierFertig;
  QColor ColorBierLeer;
  void SetDiagrammFarben();
  void SetAnsicht();
  void SetMaxAnzahlSterne();
  void FuelleDiagrammNachgaerverlauf();
  void LeseNachgaerverlaufDB();
  void SchreibeNachgaerverlaufDB();
  void FuelleDiagrammHauptgaerverlauf();
  void LeseHauptgaerverlaufDB();
  void SchreibeHauptgaerverlaufDB();
  void FuelleDiagrammSchnellgaerverlauf();
  void LeseSchnellgaerverlaufDB();
  void SchreibeSchnellgaerverlaufDB();
  void FuelleGaerverlauf();
  void BerEffektiveAusbeuteMittel();
  void FuelleBrauuebersicht();
  void BerKosten();
  void ImportSudDatei(QString Name);
  void SchreibeMalzschuettungDB();
  void SchreibeHopfengabenDB();
  void SchreibeRastenDB();
  void SchreibeSuddatenDB();
  void SchreibeBewertungenDB();
  void AddMalzgabe(QString Name, double Prozent, double erg_Menge, double Farbe);
  void FuelleSudauswahl();
  void LeseSuddatenDB();
  void LeseRohstoffeDB();
  void setDatumsfelder();
  void LadeSudDB(bool aktivateTab);
  void LeseRohstoffeDB_test();
  void SchreibeRohstoffeDB();
  void LeseGeraetelisteDB(int id);
  void SchreibeGeraetelisteDB(int id = 0);
  void DatenEinlesenDB();
  void LeseAusruestungDB();
  void BerFarbe(double cEBC = 0);
  void LadeBild();
  bool ComboboxWirdGefuellt;
  bool AenderungRohstofftabelle;
  QString GetWertString(double value);
  void CheckHopfenProzent();
  void AddHopfengabe(bool vwh, QString Name, int Zeit, double Menge, double erg_Menge, double Alpha, int Pellets);
  void SetStatusHopfengaben();
  bool AbfrageSpeichern();
  bool Aenderung;
  QString strippedName(const QString &fullFileName);
  int strippedID(const QString &fullFileName);
  void updateRecentFileActions();
  void CheckJungbierSW();
  void CheckRohstoffeVorhanden();
  void CheckMalzProzent();
  void CheckFehler();
  void SetDisabledAbgefuellt(bool status);
  void RohstoffeAbziehen();
  void AddAnhang(QString pfad);
  void SetStatusGebraut(bool status);
  void BerBraudaten();
  bool BierWurdeGebraut;
  bool BierWurdeAbgefuellt;
  bool BierWurdeVerbraucht;
  void ErstelleSpickzettel();
  void ErstelleTabSpickzettel();
  void ErstelleUeber();
  void BerHopfen();
  void BerWasser();
  void BerSchuettung();
  void BerAlles();
  void FuelleRezeptComboAuswahlen();
  void createMenus();
  void createActions();
  void retranslateMenus();
  void LeseKonfig();
  void SchreibeKonfig();
  void LeseRohstoffe();
  void DatenSchreibenDB();
  void SchreibeAusruestungDB();
  void setRecentFile(int ID);
	void setFensterTitel();
  void SchreibeAnhangDB();
  void LeseAnhangDB();
  QMenu *geladenerSudMenu;
  QMenu *extrasMenu;
  QMenu *sprachMenu;
  QMenu *recentFilesMenu;
  QAction *saveAct;
  QAction *exitAct;
  QAction *einstellungen;
  QAction *schuettungProzent;
  QAction *berIBU;
  QAction *EntsperreEingabefelder;
  QAction *ResetBierGebraut;
  QAction *ResetAbgefuellt;
  QAction *ResetVerbraucht;
  QAction *ResetZugabestatus;
  enum { MaxRecentFiles = 5 };
  QAction *recentFileActs[MaxRecentFiles];
  //QString AktuelleSuddatei;
  int AktuelleSudID;
  QBerechnungen Berechnungen;
  QExport Export;
  bool Gestartet;
  bool AmLaden;
  bool NichtBerechnen;
  bool fuelleGeraeteliste;
  //Beschreibung für Malzeintrag merken in Rohstoffliste
  QString Malz_Bezeichnung_Merker;
  QString Hopfen_Bezeichnung_Merker;
  QString Hefe_Bezeichnung_Merker;
  QString WZutaten_Bezeichnung_Merker;

  enum RendererType { Native, OpenGL, Image };
  //Die Scene
  QGraphicsScene *scene;
  //Das Bierglasbild
  QGraphicsItem *m_svgItem;
  //Das Rechteck im Hintergrund mit der Entsprechenden Bierfarbe
  QGraphicsItem *rect;
  QGraphicsRectItem *m_backgroundItem;
  QGraphicsRectItem *m_outlineItem;
  //Setzt den Farbeinstellungswert in den Bewertungen
  void setBewertungFarbe();
  //Setzt die Schaumeinstellung in den Bewertungen
  void setBewertungSchaum();
  //Setzt die Gerucheinstellungen in den Bewertungen
  void setBewertungGeruch();
  //Setzt die Geschmackseinstellungen in den Bewertungen
  void setBewertungGeschmack();
  //Setzt die Antrunkseinstellungen in den Bewertungen
  void setBewertungAntrunk();
  //Setzt die Haupttrunkeinstellungen in den Bewertungen
  void setBewertungHaupttrunk();
  //Setzt die Nachtrunkeinstellungen in den Bewertungen
  void setBewertungNachtrunk();
  //Setzt die Gesamteindrucksbewertung in den Bewertungen
  void setBewertungGesamteindruck();
  struct Rohstoff{
    int ID;
    QString Name;
    double Menge;
    double MengeIst;
  };

public:
  MainWindowImpl( QWidget * parent = 0,  Qt::WindowFlags f = 0 );
  bool NativStyle;
  bool StyleDunkel;
  bool reconnect;
private slots:
  //wird aufgerufen wenn Messeges aufruf abgeschlossen ist
  void on_MsgCheckFertig(int count);
  void slot_pushButton_gebraut();
  double slot_getEwzFarbe(QString zutat);
  double slot_getEwzPreis(QString zutat);
  double slot_getEwzPreisHopfen(QString zutat);
  void slot_EwzZugegeben(QString zutat, int typ, double menge);
  int slot_getEwzAusbeute(QString zutat);
  void slot_EwzAenderung();
  void slot_MalzAenderung();
  void slot_HopfenAenderung();
  double slot_MalzGetMenge(QString name);
  double slot_EwzGetMenge(QString name);
  double slot_HopfenGetMenge(QString name);
  double slot_HefeGetMenge(QString name);
  int slot_getEwzEinheit(QString zutat);
  int slot_getEwzTyp(QString zutat);
  void slot_ewzClose(int id);
  void slot_malzClose(int id);
  void slot_hopfenClose(int id);
  void slot_rastClose(int id);
  void slot_anhangClose(int id);
  void slot_bewClose(int id);
  void slot_rastAenderung(int id);
  void slot_anhangAenderung();
  void on_pushButton_EWZ_Hinzufuegen_clicked();
  void on_tableWidget_WeitereZutaten_itemSelectionChanged();
  void on_pushButton_WeitereZutatenDel_clicked();
  void on_pushButton_WeitereZutatenKopie_clicked();
  void on_pushButton_WeitereZutatenNeu_clicked();
  void on_TabWidget_Zutaten_currentChanged(int index);
  void on_TabWidget_RezeptErgebnisse_currentChanged(int index);
  void on_tableWidget_Sudauswahl_itemSelectionChanged();
  void on_tableWidget_Brauuebersicht_cellDoubleClicked(int row, int column);
  void on_tableWidget_Sudauswahl_cellDoubleClicked(int row, int column);
  void slot_AenderungAusruestung(double d);
  void slot_AenderungAusruestung(int d);
  void slot_dateChanged(QDate);
  void on_spinBox_SWVorHopfenseihen_valueChanged(double );
  void on_SpinBox_wwSaeurekapazitaet_mmol_valueChanged(double );
  void on_SpinBox_wwCarbonathaerte_dh_valueChanged(double );
  void on_SpinBox_wwMagnesium_mg_valueChanged(double );
  void on_SpinBox_wwMagnesium_mmol_valueChanged(double );
  void on_SpinBox_wwCalcium_mmol_valueChanged(double );
  void on_SpinBox_wwCalcium_mg_valueChanged(double );
  void on_tableWidget_Brauuebersicht_itemSelectionChanged();
  void slot_diagram_Brauuebersicht_selectionChanged(int id);
  void on_pushButton_SudImport_clicked();
  void on_pushButton_SudExport_clicked();
  void slot_EntsperreEingabefelder();
  void slot_ResetBierWurdeGebraut();
  void slot_ResetBierVerbraucht();
  void slot_ResetAbgefuellt();
  //Zugabestatus der Weiteren Zutaten wird zurückgesetzt
  void slot_ResetWZZugabestatus();
  void on_pushButton_EingabeHMengeNHopfenseihen_clicked();
  void on_pushButton_EingabeHMengeVHopfenseihen_clicked();
  void on_pushButton_EingabeHSWVorHopfenseihen_clicked();
  void slot_berIBU();
  void slot_schuettungProzent();
  void on_lineEdit_Sudname_textEdited(QString );
  void on_lineEdit_Sudname_editingFinished();
  void on_pushButton_SudLadenBUebersicht_clicked();
  void on_pushButton_SudVerbraucht_clicked();
  void on_tableWidget_Hauptgaerverlauf_itemChanged(QTableWidgetItem* item);
  void on_tableWidget_Nachgaerverlauf_itemChanged(QTableWidgetItem* item);
  void on_tableWidget_Schnellgaerverlauf_itemChanged(QTableWidgetItem* item);
  void slot_einstellungen();
  void on_pushButton_DelNachgaerMessung_clicked();
  void on_pushButton_AddNachgaerMessung_clicked();
  void on_pushButton_DelHauptgaerMessung_clicked();
  void on_pushButton_EingabeSWHauptgaerverlauf_clicked();
  void on_pushButton_AddHauptgaerMessung_clicked();
  void on_pushButton_EingabeSWSchnellgaerverlauf_clicked();
  void on_pushButton_AddSchnellgaerMessung_clicked();
    void on_pushButton_DelSchnellgaerMessung_clicked();
  void on_comboBox_AuswahlL1_currentIndexChanged(int index);
  void on_comboBox_AuswahlL2_currentIndexChanged(int index);
  void on_pushButton_SpickzettelPDF_clicked();
  void slot_pushButton_SudKopie();
  void slot_FilterClicked(bool checked);
  void on_lineEdit_FilterText_textChanged(const QString &arg1);
  void slot_pushButton_SudLaden();
  void slot_pushButton_SudDel();
  int slot_pushButton_SudNeu();
  void slot_lineEdit_editingFinished();
  void on_pushButton_HefeKopie_clicked();
  void on_pushButton_HopfenKopie_clicked();
  void on_pushButton_MalzKopie_clicked();
  void slot_tabWidgetChanged(int);
  void slot_spinBoxGesammtkochdauerChanged(int value);
  void slot_TableWidget_cellChanged(int row, int column);
  void openRecentFile();
  void slot_pushButton_DatenUebertragen();
  void slot_Changed();
  void slot_pushButton_SudAbgefuellt();
  void slot_pushButton_EingabeHSWJungbier();
  void slot_pushButton_EingabeHSWSchnellgaerprobe();
  void slot_pushButton_EingabeHSWAnstellen();
  void slot_pushButton_EingabeHSWKochende();
  void slot_pushButton_AbfuelldatumHeute();
  void slot_pushButton_AnstelldatumHeute();
  void slot_pushButton_BraudatumHeute();
  void slot_pushButton_GeraeteNeu();
  void slot_pushButton_GeraeteDel();
  void slot_ComboBoxIndexChanged(int i);
  void slot_spinBoxValueChanged(int i);
  void save();
  void on_pushButton_MalzDel_clicked();
  void MalzNeueZeile(const QString& name, double ebc, double schuettung, double menge, double preis,
                     const QString& bemerkung, const QString& anwendung,
                     const QDate& einlagerung, const QDate& haltbar, const QString& link);
  void on_pushButton_MalzNeuVorlage_clicked();
  void on_pushButton_HopfenDel_clicked();
  void HopfenNeueZeile(const QString& name, double alpha, double menge, double preis, bool pellets,
                       const QString& bemerkung, int typ, const QString& eigenschaft,
                       const QDate& einlagerung, const QDate& haltbar, const QString& link);
  void on_pushButton_HefeDel_clicked();
  void HefeNeueZeile(const QString& name, double menge, double wuerzemenge, double preis,
                     const QString& bemerkung, const QString& verpackung, int typ1, int typ2,
                     const QString& temperatur, const QString& eigenschaft,
                     int sedimentation, const QString &evg,
                     const QDate& einlagerung, const QDate& haltbar, const QString& link);
  void WeitereZutatNeueZeile(const QString& name, double menge, int einheit, int typ,
                             double ausbeute, double ebc, double preis, const QString& bemerkung,
                             const QDate& einlagerung, const QDate& haltbar, const QString& link);
  void on_pushButton_WeitereZutatenNeuVorlage_clicked();
  void slot_spinBoxValueChanged(double d);
  void slot_RohstoffNonZeroValueChanged(double menge);
  void slot_RohstoffMengeValueChanged(double menge);
  void slot_RohstoffHaltbarValueChanged(const QDate &date);
  void slot_RohstoffFarbeValueChanged(double ebc);
  void slot_tableSyncValueChanged(double value);
  void slot_tableSyncValueChanged(const QString &value);
  void slot_tableSyncValueChanged(const QDate &date);
  void on_pushButton_NeueRast_clicked();
  void slot_pushButton_RastNachOben(int id);
  void slot_pushButton_RastNachUnten(int id);
  void on_spinBox_WuerzemengeAnstellen_valueChanged(double arg1);
  void on_spinBox_SWKochende_valueChanged(double arg1);
  void on_spinBox_WuerzemengeKochende_valueChanged(double arg1);
  void on_spinBox_Speisemenge_valueChanged(double arg1);
  void on_spinBox_SW_valueChanged(double arg1);
  void on_spinBox_WuerzemengeVorHopfenseihen_valueChanged(double arg1);
  void on_pushButton_BewertungNeu_clicked();
  void slot_BewertungWoche_clicked(int id);
  void slot_GraphicsView_AnzahlSterneChanged(int value);
  void on_dateEdit_Bewertungsdatum_dateChanged(const QDate &date);
  void on_textEdit_bewBemerkung_textChanged();
  void on_radioButton_farbe_0_clicked();
  void on_radioButton_farbe_1_clicked();
  void on_radioButton_farbe_2_clicked();
  void on_radioButton_farbe_3_clicked();
  void on_radioButton_farbe_4_clicked();
  void on_radioButton_farbe_5_clicked();
  void on_radioButton_farbe_6_clicked();
  void on_radioButton_farbe_7_clicked();
  void on_radioButton_farbe_8_clicked();
  void on_radioButton_farbe_9_clicked();
  void on_radioButton_farbe_10_clicked();
  void on_radioButton_schaum_0_clicked();
  void on_radioButton_schaum_1_clicked();
  void on_radioButton_schaum_2_clicked();
  void on_radioButton_schaum_3_clicked();
  void on_radioButton_schaum_4_clicked();
  void on_radioButton_schaum_5_clicked();
  void on_radioButton_schaum_6_clicked();
  void on_radioButton_schaum_7_clicked();
  void on_radioButton_schaum_8_clicked();
  void on_radioButton_schaum_9_clicked();
  void on_radioButton_schaum_10_clicked();
  void on_checkBox_geruch_0_clicked();
  void on_checkBox_geruch_1_clicked();
  void on_checkBox_geruch_2_clicked();
  void on_checkBox_geruch_3_clicked();
  void on_checkBox_geruch_4_clicked();
  void on_checkBox_geruch_5_clicked();
  void on_checkBox_geruch_6_clicked();
  void on_checkBox_geruch_7_clicked();
  void on_checkBox_geruch_8_clicked();
  void on_checkBox_geruch_9_clicked();
  void on_checkBox_geruch_10_clicked();
  void on_checkBox_geruch_11_clicked();
  void on_checkBox_geschmack_0_clicked();
  void on_checkBox_geschmack_1_clicked();
  void on_checkBox_geschmack_2_clicked();
  void on_checkBox_geschmack_3_clicked();
  void on_checkBox_geschmack_4_clicked();
  void on_checkBox_geschmack_5_clicked();
  void on_checkBox_geschmack_6_clicked();
  void on_checkBox_geschmack_7_clicked();
  void on_checkBox_geschmack_8_clicked();
  void on_checkBox_geschmack_9_clicked();
  void on_checkBox_geschmack_10_clicked();
  void on_checkBox_geschmack_11_clicked();
  void on_checkBox_geschmack_12_clicked();
  void on_radioButton_antrunk_0_clicked();
  void on_radioButton_antrunk_1_clicked();
  void on_radioButton_antrunk_2_clicked();
  void on_radioButton_antrunk_3_clicked();
  void on_radioButton_antrunk_4_clicked();
  void on_radioButton_antrunk_5_clicked();
  void on_radioButton_antrunk_6_clicked();
  void on_radioButton_antrunk_7_clicked();
  void on_radioButton_haupttrunk_0_clicked();
  void on_radioButton_haupttrunk_1_clicked();
  void on_radioButton_haupttrunk_2_clicked();
  void on_radioButton_haupttrunk_3_clicked();
  void on_radioButton_haupttrunk_4_clicked();
  void on_radioButton_nachtrunk_0_clicked();
  void on_radioButton_nachtrunk_1_clicked();
  void on_radioButton_nachtrunk_2_clicked();
  void on_radioButton_nachtrunk_3_clicked();
  void on_radioButton_nachtrunk_4_clicked();
  void on_radioButton_nachtrunk_5_clicked();
  void on_radioButton_nachtrunk_6_clicked();
  void on_radioButton_nachtrunk_7_clicked();
  void on_radioButton_nachtrunk_8_clicked();
  void on_radioButton_gesamteindruck_0_clicked();
  void on_radioButton_gesamteindruck_1_clicked();
  void on_radioButton_gesamteindruck_2_clicked();
  void on_radioButton_gesamteindruck_3_clicked();
  void on_radioButton_gesamteindruck_4_clicked();
  void on_radioButton_gesamteindruck_5_clicked();
  void on_radioButton_gesamteindruck_6_clicked();
  void on_radioButton_gesamteindruck_7_clicked();
  void on_textEdit_farbe_textChanged();
  void on_textEdit_schaum_textChanged();
  void on_textEdit_geruch_textChanged();
  void on_textEdit_geschmack_textChanged();
  void on_textEdit_antrunk_textChanged();
  void on_textEdit_haupttrunk_textChanged();
  void on_textEdit_nachtrunk_textChanged();
  void on_textEdit_gesamteindruck_textChanged();
  void on_pushButton_addStar_clicked();
  void on_pushButton_remStar_clicked();
  void on_tableWidget_Malz_cellChanged(int row, int column);
  void on_tableWidget_Malz_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_tableWidget_Malz_itemSelectionChanged();
  void on_pushButton_HopfenNeuVorlage_clicked();
  void on_tableWidget_Hopfen_itemSelectionChanged();
  void on_tableWidget_Hopfen_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_tableWidget_Hopfen_cellChanged(int row, int column);
  void on_tableWidget_Hefe_cellChanged(int row, int column);
  void on_tableWidget_Hefe_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_pushButton_HefeNeuVorlage_clicked();
  void on_tableWidget_Hefe_itemSelectionChanged();
  void on_tableWidget_WeitereZutaten_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_tableWidget_WeitereZutaten_cellChanged(int row, int column);
  void on_pushButton_HopfenHinzufuegen_clicked();
  void on_pushButton_MalzHinzufuegen_clicked();
  void on_spinBox_Menge_valueChanged(double arg1);
  void on_comboBox_BerechnungsArtHopfen_currentIndexChanged(int index);
  void on_tabWidged_currentChanged(int index);
  void on_spinBox_High_Gravity_valueChanged(int arg1);
	//sprache wird umgeschalten
	void slot_SpracheWechselt(QAction* action);
	void on_pushButton_EingabeHVerdampfungsziffer_clicked();
  void on_pushButton_SudinfoPDF_clicked();
  void on_pushButton_NeueBrauanlage_clicked();
  void on_pushButton_loescheBrauanlage_clicked();
  void on_listWidget_Brauanlagen_itemSelectionChanged();
  void on_listWidget_Brauanlagen_itemChanged(QListWidgetItem *item);
  void on_spinBox_AngenommeneAusbeute_valueChanged(int arg1);
  void on_dSpinBox_KorrekturNachguss_valueChanged(double arg1);
  void on_doubleSpinBox_Verdampfung_valueChanged(double arg1);
  void on_spinBox_MaischebottichHoehe_valueChanged(double arg1);
  void on_spinBox_MaischebottichDurchmesser_valueChanged(double arg1);
  void on_spinBox_MaischebottichMaxFuellhoehe_valueChanged(double arg1);
  void on_spinBox_SudpfanneHoehe_valueChanged(double arg1);
  void on_spinBox_SudpfanneDurchmesser_valueChanged(double arg1);
  void on_spinBox_SudpfanneMaxFuellhoehe_valueChanged(double arg1);
  void on_hSlider_dAusbeuteSude_valueChanged(int value);
  void on_spinBox_KorrekturFarbe_valueChanged(int arg1);
  void setButtonsTextMerken();
  void on_pushButton_merken_clicked();
  void on_pushButton_vergessen_clicked();
  void on_pushButton_alleVergessen_clicked();
  void on_checkBox_MerklisteMengen_clicked();
  void on_spinBox_NachisomerisierungsZeit_valueChanged(int arg1);
  void on_dspinBox_KostenAusruestung_valueChanged(double arg1);
  void on_pushButton_MalzNeu_clicked();
  void on_pushButton_HopfenNeu_clicked();
  void on_pushButton_HefeNeu_clicked();
  void on_listWidget_Brauanlagen_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
  void on_tableWidget_Geraete_itemChanged(QTableWidgetItem *item);
  void on_tableWidget_Malz_cellClicked(int row, int column);
  void on_tableWidget_Hopfen_cellClicked(int row, int column);
  void on_tableWidget_Hefe_cellClicked(int row, int column);
  void on_tableWidget_WeitereZutaten_cellClicked(int row, int column);
  void on_pushButton_CO2_Info_clicked();
  void on_pushButton_IBU_Info_clicked();
  void on_pushButton_SW_Info_clicked();
  void on_pushButton_High_Gravity_Info_clicked();
  void on_pushButton_NeuerAnhang_clicked();
  void on_comboBox_GaerungEwzAuswahl_currentIndexChanged(const QString &arg1);
  void on_pushButton_GaerungEwzZugeben_clicked();
  void on_pushButton_GaerungEwzEntnehmen_clicked();
  void on_pushButton_CalcEinmaischeTemp_clicked();
  void on_spinBox_AnzahlHefeEinheiten_valueChanged(int);
  void on_pushButton_SudTeilen_clicked();
  void on_dateEdit_AuswahlVon_userDateChanged(const QDate &date);
  void on_dateEdit_AuswahlBis_userDateChanged(const QDate &date);
  void on_checkBox_BrauuebersichtAuswahlAlle_stateChanged(int);
  void on_horizontalSlider_ScalePDF_valueChanged(int);
};
#endif



















