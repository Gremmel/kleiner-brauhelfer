#include "einstellungsdialogimpl.h"
#include "definitionen.h"
#include "errormessage.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QSettings>
#include <QDir>
#include <QMessageBox>

//
EinstellungsdialogImpl::EinstellungsdialogImpl( QWidget * parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  NeuerDBPfad = false;
  setupUi(this);

  label_Datenbankpfad -> setText(trUtf8("Pfad zur Datenbank: ") + DB_USER_NAME);
  ErstelleIcons();

  LeseKonfig();
  on_checkBox_PDFProgStarten_clicked();
}
//


void EinstellungsdialogImpl::ErstelleIcons()
{
  QListWidgetItem *DatenbankButton = new QListWidgetItem(listWidget);
  DatenbankButton->setIcon(QIcon(":/dlg_einstellungen/datenbank.png"));
  DatenbankButton->setText(trUtf8("Datenbank"));
  DatenbankButton->setTextAlignment(Qt::AlignHCenter);
  DatenbankButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  QListWidgetItem *FarbauswahlButton = new QListWidgetItem(listWidget);
  FarbauswahlButton->setIcon(QIcon(":/dlg_einstellungen/farbauswahl.png"));
  FarbauswahlButton->setText(trUtf8("Farbauswahl"));
  FarbauswahlButton->setTextAlignment(Qt::AlignHCenter);
  FarbauswahlButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  QListWidgetItem *KonfigButton = new QListWidgetItem(listWidget);
  KonfigButton->setIcon(QIcon(":/dlg_einstellungen/erweitert.png"));
  KonfigButton->setText(trUtf8("Erweitert"));
  KonfigButton->setTextAlignment(Qt::AlignHCenter);
  KonfigButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  QListWidgetItem *AnsichtButton = new QListWidgetItem(listWidget);
  AnsichtButton->setIcon(QIcon(":/dlg_einstellungen/auge.png"));
  AnsichtButton->setText(trUtf8("Ansicht"));
  AnsichtButton->setTextAlignment(Qt::AlignHCenter);
  AnsichtButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  connect(listWidget,
          SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
          this, SLOT(SeiteWechseln(QListWidgetItem*,QListWidgetItem*)));
}

void EinstellungsdialogImpl::StyleFelderEinausblenden()
{
  comboBox_Farbgebung->setVisible(!checkBox_NativStyle->isChecked());
  if (checkBox_NativStyle->isChecked()){
    widget_FarbeStyleAuswahl->setVisible(false);
  }
  else {
    if (comboBox_Farbgebung->currentIndex() == 0)
      widget_FarbeStyleAuswahl->setVisible(false);
    else
      widget_FarbeStyleAuswahl->setVisible(true);
  }
}


void EinstellungsdialogImpl::SeiteWechseln(QListWidgetItem *current, QListWidgetItem *previous)
{
  if (!current)
    current = previous;

  stackedWidget->setCurrentIndex(listWidget->row(current));
}


void EinstellungsdialogImpl::on_pushButton_GetColorDia_S_L1_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorDia_S_L1 -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorDia_S_L1->setText(color.name());
    label_ColorDia_S_L1->setPalette(QPalette(color));
    label_ColorDia_S_L1->setAutoFillBackground(true);
  }
}

void EinstellungsdialogImpl::LeseKonfigFarben()
{
  //Farben aus Konfigdatei auslesen und im Dialogfeld setzten
  QColor color;
  QString HtmlColor = "";

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  //Styleeinstellung einlesen
  settings.beginGroup("Style");

  checkBox_NativStyle->setChecked(settings.value("NativStyle").toBool());
  StyleFelderEinausblenden();
  comboBox_Farbgebung->setCurrentIndex(settings.value("Farbgebung").toInt());

  //Farbe Highlight
  HtmlColor = settings.value("FARBE_HIGHLIGHT").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_STYLE_HIGHLIGHT);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    label_ColorStyleAuswahl -> setText(color.name());
    label_ColorStyleAuswahl -> setPalette(QPalette(color));
    label_ColorStyleAuswahl -> setAutoFillBackground(true);
  }

  settings.endGroup();

  //Diagrammfarben einlesen
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
    label_ColorDia_S_L1 -> setText(color.name());
    label_ColorDia_S_L1 -> setPalette(QPalette(color));
    label_ColorDia_S_L1 -> setAutoFillBackground(true);
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
    label_ColorDia_S_L2 -> setText(color.name());
    label_ColorDia_S_L2 -> setPalette(QPalette(color));
    label_ColorDia_S_L2 -> setAutoFillBackground(true);
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
    label_ColorDia_S_L3 -> setText(color.name());
    label_ColorDia_S_L3 -> setPalette(QPalette(color));
    label_ColorDia_S_L3 -> setAutoFillBackground(true);
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
    label_ColorDia_H_L1 -> setText(color.name());
    label_ColorDia_H_L1 -> setPalette(QPalette(color));
    label_ColorDia_H_L1 -> setAutoFillBackground(true);
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
    label_ColorDia_H_L2 -> setText(color.name());
    label_ColorDia_H_L2 -> setPalette(QPalette(color));
    label_ColorDia_H_L2 -> setAutoFillBackground(true);
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
    label_ColorDia_H_L3 -> setText(color.name());
    label_ColorDia_H_L3 -> setPalette(QPalette(color));
    label_ColorDia_H_L3 -> setAutoFillBackground(true);
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
    label_ColorDia_N_L1 -> setText(color.name());
    label_ColorDia_N_L1 -> setPalette(QPalette(color));
    label_ColorDia_N_L1 -> setAutoFillBackground(true);
  }

  //Farbe Linie 2 Nachgärverlauf
  HtmlColor = settings.value("FARBE_GAERVERLAUF_DIAGRAMM_N_L2").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_N_L2);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    label_ColorDia_N_L2 -> setText(color.name());
    label_ColorDia_N_L2 -> setPalette(QPalette(color));
    label_ColorDia_N_L2 -> setAutoFillBackground(true);
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
    label_ColorDia_B_L1 -> setText(color.name());
    label_ColorDia_B_L1 -> setPalette(QPalette(color));
    label_ColorDia_B_L1 -> setAutoFillBackground(true);
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
    label_ColorDia_B_L2 -> setText(color.name());
    label_ColorDia_B_L2 -> setPalette(QPalette(color));
    label_ColorDia_B_L2 -> setAutoFillBackground(true);
  }

  //Farbe Spalte 1 Brauübersicht
  HtmlColor = settings.value("FARBE_BRAUUEBERSICHT_SPALTE_L1").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_BRAUUEBERSICHT_SPALTE_L1);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    label_ColorDia_B_SpL1 -> setText(color.name());
    label_ColorDia_B_SpL1 -> setPalette(QPalette(color));
    label_ColorDia_B_SpL1 -> setAutoFillBackground(true);
  }

  //Farbe Spalte 2 Brauübersicht
  HtmlColor = settings.value("FARBE_BRAUUEBERSICHT_SPALTE_L2").toString();
  if (HtmlColor == ""){
    color = QColor::fromRgb(FARBE_BRAUUEBERSICHT_SPALTE_L2);
  }
  else {
    color.setNamedColor(HtmlColor);
  }
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    label_ColorDia_B_SpL2 -> setText(color.name());
    label_ColorDia_B_SpL2 -> setPalette(QPalette(color));
    label_ColorDia_B_SpL2 -> setAutoFillBackground(true);
  }
  settings.endGroup();
}


void EinstellungsdialogImpl::LeseKonfig()
{
  LeseKonfigDB();
  LeseKonfigFarben();
  LeseKonfigErweitert();
  LeseKonfigAnsicht();
}


void EinstellungsdialogImpl::on_pushButton_GetColorDia_S_L2_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorDia_S_L2 -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorDia_S_L2->setText(color.name());
    label_ColorDia_S_L2->setPalette(QPalette(color));
    label_ColorDia_S_L2->setAutoFillBackground(true);
  }
}

void EinstellungsdialogImpl::on_pushButton_GetColorDia_S_L3_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorDia_S_L3 -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorDia_S_L3->setText(color.name());
    label_ColorDia_S_L3->setPalette(QPalette(color));
    label_ColorDia_S_L3->setAutoFillBackground(true);
  }
}

void EinstellungsdialogImpl::on_pushButton_GetColorDia_H_L1_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorDia_H_L1 -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorDia_H_L1->setText(color.name());
    label_ColorDia_H_L1->setPalette(QPalette(color));
    label_ColorDia_H_L1->setAutoFillBackground(true);
  }
}

void EinstellungsdialogImpl::on_pushButton_GetColorDia_H_L2_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorDia_H_L2 -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorDia_H_L2->setText(color.name());
    label_ColorDia_H_L2->setPalette(QPalette(color));
    label_ColorDia_H_L2->setAutoFillBackground(true);
  }
}


void EinstellungsdialogImpl::on_pushButton_GetColorDia_H_L3_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorDia_H_L3 -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorDia_H_L3->setText(color.name());
    label_ColorDia_H_L3->setPalette(QPalette(color));
    label_ColorDia_H_L3->setAutoFillBackground(true);
  }
}

void EinstellungsdialogImpl::on_pushButton_GetColorDia_N_L1_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorDia_N_L1 -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorDia_N_L1->setText(color.name());
    label_ColorDia_N_L1->setPalette(QPalette(color));
    label_ColorDia_N_L1->setAutoFillBackground(true);
  }
}

void EinstellungsdialogImpl::on_pushButton_GetColorDia_N_L2_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorDia_N_L2 -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorDia_N_L2->setText(color.name());
    label_ColorDia_N_L2->setPalette(QPalette(color));
    label_ColorDia_N_L2->setAutoFillBackground(true);
  }
}

void EinstellungsdialogImpl::on_pushButton_GetColorDia_B_L1_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorDia_B_L1 -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorDia_B_L1->setText(color.name());
    label_ColorDia_B_L1->setPalette(QPalette(color));
    label_ColorDia_B_L1->setAutoFillBackground(true);
  }
}

void EinstellungsdialogImpl::on_pushButton_GetColorDia_B_L2_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorDia_B_L2 -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorDia_B_L2->setText(color.name());
    label_ColorDia_B_L2->setPalette(QPalette(color));
    label_ColorDia_B_L2->setAutoFillBackground(true);
  }
}


void EinstellungsdialogImpl::LeseKonfigDB()
{
  QString s;
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("DB");
  s = settings.value("DB_Pfad").toString();
  settings.endGroup();

  if (s == ""){
    s = QDir::homePath() + "/" + APP_VERZEICHNIS;
  }
  else {

  }
  lineEdit_Datenbankpfad -> setText(s);
}


void EinstellungsdialogImpl::on_pushButton_GetPfad_clicked()
{


  DBDirVorher = lineEdit_Datenbankpfad -> text();
  //QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly		| QFileDialog::DontUseNativeDialog;
  QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
  QString directory = QFileDialog::getExistingDirectory(this,
                      trUtf8("Datenbankpfad Auswählen"),
                      lineEdit_Datenbankpfad -> text(),
                      options);

  if (directory.isEmpty()){

  }
  else {
    lineEdit_Datenbankpfad -> setText(directory);
    //Wenn sich Verzeichniss geänder hat Merker setzten das DB umplatziert werden muss
    if (DBDirVorher != directory){
      NeuerDBPfad = true;
    }
  }
}

void EinstellungsdialogImpl::on_buttonBox_accepted()
{
  //Einstellungen Speichern
  SchreibeKonfigDB();
  SchreibeKonfigFarben();
  SchreibeKonfigErweitert();
  SchreibeKonfigAnsicht();
  accept();
}

void EinstellungsdialogImpl::on_buttonBox_rejected()
{
  NeuerDBPfad = false;
  reject();
}

void EinstellungsdialogImpl::SchreibeKonfigDB()
{
  if (NeuerDBPfad) {
    bool b = true;
    //Abfrage ob Datenbank zu dem Neuen Pfad verschoben werden soll
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, trUtf8("Datenbank verschieben?"),
              trUtf8("Der Datenbankpfad wurde geändert!\nSoll die Datenbank an den neuen Ort Kopiert werden?\n\nDer kleine-brauhelfer wird nach dem Kopieren beendet und muss neu gestartet werden."),
              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes){
      //Datenbank wird an den Neuen Ort verschoben
      QFile file(DBDirVorher + "/" + DB_USER_NAME);
      if (file.copy(lineEdit_Datenbankpfad -> text() + "/" + DB_USER_NAME)) {
        //if (!file.remove()){
          //Fehlermeldung Kann alte Datenbankdatei nicht entfernen
          //ErrorMessage *errorMessage = new ErrorMessage();
          //errorMessage -> showMessage(ERR_DB_DEL_ALT, TYPE_WARNUNG,
            //CANCEL_NO, trUtf8("Betroffene Datei:\n") + file.fileName());
        //}
      }
      else {
        NeuerDBPfad = false;
        b = false;
        // Fehlermeldung Kann Datenbankdatei nicht an neuen Ort Kopieren
        ErrorMessage *errorMessage = new ErrorMessage();
        errorMessage -> showMessage(ERR_DB_KOPIE_NEU_ORT, TYPE_WARNUNG,
          CANCEL_NO, trUtf8("Betroffene Datei:\n") + file.fileName());
      }
    }

    if (b){
      //Neuen Pfad Speichern
      QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
      settings.beginGroup("DB");
      settings.setValue("DB_Pfad", lineEdit_Datenbankpfad -> text());
      settings.endGroup();
    }
  }
}


void EinstellungsdialogImpl::SchreibeKonfigFarben()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  //Style Einstellungen in Konfigdatei schreiben
  settings.beginGroup("Style");

  //checkbox Nativ Style
  settings.setValue("NativStyle", checkBox_NativStyle->isChecked());

  //Auswahl Farbgebung
  settings.setValue("Farbgebung", comboBox_Farbgebung->currentIndex());

  //Farbe Highlight
  settings.setValue("FARBE_HIGHLIGHT", label_ColorStyleAuswahl -> text());

  settings.endGroup();

  //Farben in Konfigdatei schreiben

  settings.beginGroup("Farben");

  //Farbe Linie 1 Schnellgärverlauf
  settings.setValue("FARBE_GAERVERLAUF_DIAGRAMM_S_L1", label_ColorDia_S_L1 -> text());

  //Farbe Linie 2 Schnellgärverlauf
  settings.setValue("FARBE_GAERVERLAUF_DIAGRAMM_S_L2", label_ColorDia_S_L2 -> text());

  //Farbe Linie 3 Schnellgärverlauf
  settings.setValue("FARBE_GAERVERLAUF_DIAGRAMM_S_L3", label_ColorDia_S_L3 -> text());

  //Farbe Linie 1 Hauptgärverlauf
  settings.setValue("FARBE_GAERVERLAUF_DIAGRAMM_H_L1", label_ColorDia_H_L1 -> text());

  //Farbe Linie 2 Hauptgärverlauf
  settings.setValue("FARBE_GAERVERLAUF_DIAGRAMM_H_L2", label_ColorDia_H_L2 -> text());

  //Farbe Linie 3 Hauptgärverlauf
  settings.setValue("FARBE_GAERVERLAUF_DIAGRAMM_H_L3", label_ColorDia_H_L3 -> text());

  //Farbe Linie 1 Nachgärverlauf
  settings.setValue("FARBE_GAERVERLAUF_DIAGRAMM_N_L1", label_ColorDia_N_L1 -> text());

  //Farbe Linie 2 Nachgärverlauf
  settings.setValue("FARBE_GAERVERLAUF_DIAGRAMM_N_L2", label_ColorDia_N_L2 -> text());

  //Farbe Linie 1 Brauübersicht
  settings.setValue("FARBE_BRAUUEBERSICHT_DIAGRAMM_L1", label_ColorDia_B_L1 -> text());

  //Farbe Linie 2 Brauübersicht
  settings.setValue("FARBE_BRAUUEBERSICHT_DIAGRAMM_L2", label_ColorDia_B_L2 -> text());

  //Farbe Spalte 1 Brauübersicht
  settings.setValue("FARBE_BRAUUEBERSICHT_SPALTE_L1", label_ColorDia_B_SpL1 -> text());

  //Farbe Spalte 2 Brauübersicht
  settings.setValue("FARBE_BRAUUEBERSICHT_SPALTE_L2", label_ColorDia_B_SpL2 -> text());

  settings.endGroup();
}


void EinstellungsdialogImpl::on_pushButton_GetColorDia_SPB_L1_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorDia_B_SpL1 -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorDia_B_SpL1->setText(color.name());
    label_ColorDia_B_SpL1->setPalette(QPalette(color));
    label_ColorDia_B_SpL1->setAutoFillBackground(true);
  }
}

void EinstellungsdialogImpl::on_pushButton_GetColorDia_SpB_L2_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorDia_B_SpL2 -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorDia_B_SpL2->setText(color.name());
    label_ColorDia_B_SpL2->setPalette(QPalette(color));
    label_ColorDia_B_SpL2->setAutoFillBackground(true);
  }
}

void EinstellungsdialogImpl::on_pushButton_GetColorStyleAuswahl_clicked()
{
  QColor color,acolor;
  acolor.setNamedColor(label_ColorStyleAuswahl -> text());
  color = QColorDialog::getColor(acolor, this,"", QColorDialog::DontUseNativeDialog);

  if (color.isValid()) {
    label_ColorStyleAuswahl->setText(color.name());
    label_ColorStyleAuswahl->setPalette(QPalette(color));
    label_ColorStyleAuswahl->setAutoFillBackground(true);
  }
}


void EinstellungsdialogImpl::on_pushButton_SetDefaultFarbe_clicked()
{
  QColor color;
  //Farbwerte zurücksetzten

  color = QColor::fromRgb(FARBE_STYLE_HIGHLIGHT);
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    label_ColorStyleAuswahl -> setText(color.name());
    label_ColorStyleAuswahl -> setPalette(QPalette(color));
    label_ColorStyleAuswahl -> setAutoFillBackground(true);
  }

  color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_S_L1);
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    label_ColorDia_S_L1 -> setText(color.name());
    label_ColorDia_S_L1 -> setPalette(QPalette(color));
    label_ColorDia_S_L1 -> setAutoFillBackground(true);
  }

  //Farbe Linie 2 Schnellgärverlauf
  color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_S_L2);
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    label_ColorDia_S_L2 -> setText(color.name());
    label_ColorDia_S_L2 -> setPalette(QPalette(color));
    label_ColorDia_S_L2 -> setAutoFillBackground(true);
  }

  //Farbe Linie 3 Schnellgärverlauf
  color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_S_L3);
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    label_ColorDia_S_L3 -> setText(color.name());
    label_ColorDia_S_L3 -> setPalette(QPalette(color));
    label_ColorDia_S_L3 -> setAutoFillBackground(true);
  }

  //Farbe Linie 1 Hauptgärverlauf
  color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_H_L1);
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    label_ColorDia_H_L1 -> setText(color.name());
    label_ColorDia_H_L1 -> setPalette(QPalette(color));
    label_ColorDia_H_L1 -> setAutoFillBackground(true);
  }

  //Farbe Linie 2 Hauptgärverlauf
  color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_H_L2);
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    label_ColorDia_H_L2 -> setText(color.name());
    label_ColorDia_H_L2 -> setPalette(QPalette(color));
    label_ColorDia_H_L2 -> setAutoFillBackground(true);
  }

  //Farbe Linie 3 Hauptgärverlauf
  color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_H_L3);
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    label_ColorDia_H_L3 -> setText(color.name());
    label_ColorDia_H_L3 -> setPalette(QPalette(color));
    label_ColorDia_H_L3 -> setAutoFillBackground(true);
  }

  //Farbe Linie 1 Nachgärverlauf
  color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_N_L1);
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    label_ColorDia_N_L1 -> setText(color.name());
    label_ColorDia_N_L1 -> setPalette(QPalette(color));
    label_ColorDia_N_L1 -> setAutoFillBackground(true);
  }

  //Farbe Linie 2 Nachgärverlauf
  color = QColor::fromRgb(FARBE_GAERVERLAUF_DIAGRAMM_N_L2);
  //wenn Farbwert gültig ist Farbe setzen
  if (color.isValid()){
    label_ColorDia_N_L2 -> setText(color.name());
    label_ColorDia_N_L2 -> setPalette(QPalette(color));
    label_ColorDia_N_L2 -> setAutoFillBackground(true);
  }

  //Farbe Linie 1 Brauübersicht
  color = QColor::fromRgb(FARBE_BRAUUEBERSICHT_DIAGRAMM_L1);
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    label_ColorDia_B_L1 -> setText(color.name());
    label_ColorDia_B_L1 -> setPalette(QPalette(color));
    label_ColorDia_B_L1 -> setAutoFillBackground(true);
  }

  //Farbe Linie 2 Brauübersicht
  color = QColor::fromRgb(FARBE_BRAUUEBERSICHT_DIAGRAMM_L2);
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    label_ColorDia_B_L2 -> setText(color.name());
    label_ColorDia_B_L2 -> setPalette(QPalette(color));
    label_ColorDia_B_L2 -> setAutoFillBackground(true);
  }

  //Farbe Spalte 1 Brauübersicht
  color = QColor::fromRgb(FARBE_BRAUUEBERSICHT_SPALTE_L1);
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    label_ColorDia_B_SpL1 -> setText(color.name());
    label_ColorDia_B_SpL1 -> setPalette(QPalette(color));
    label_ColorDia_B_SpL1 -> setAutoFillBackground(true);
  }

  //Farbe Spalte 2 Brauübersicht
  color = QColor::fromRgb(FARBE_BRAUUEBERSICHT_SPALTE_L2);
  //wenn Farbwert nicht gültig ist dann defaultfarbe setzen
  if (color.isValid()){
    label_ColorDia_B_SpL2 -> setText(color.name());
    label_ColorDia_B_SpL2 -> setPalette(QPalette(color));
    label_ColorDia_B_SpL2 -> setAutoFillBackground(true);
  }
}

void EinstellungsdialogImpl::LeseKonfigErweitert()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  double d;
  settings.beginGroup("Erweitert");
  //Foktor Brix nach Plato
  d = settings.value("FaktorBrixPlato").toDouble();

  if (d == 0){
    d = 1.03;
  }
  SpinBox_FaktorBrixPlato -> setValue(d);

  //Formel zur Umrechnung von Brix nach Plato im Gärverlauf
  QString s;
  s = settings.value("FormelBrixPlato").toString();
  bool gefunden = false;
  for (int i=0; i < comboBox_FormelBrixPlato -> count(); i++){
    if (comboBox_FormelBrixPlato -> itemText(i) == s){
      comboBox_FormelBrixPlato -> setCurrentIndex(i);
      gefunden = true;
    }
  }
  if (!gefunden){
    comboBox_FormelBrixPlato -> setCurrentIndex(0);
  }

  if (s == "Kleier"){
    widget_Gaerungskorrektur -> show();
  }
  else {
    widget_Gaerungskorrektur -> hide();
  }

  //Gärungskorrektur
  d = settings.value("Gaerungskorrektur").toDouble();

  if (d == 0){
    d = 0.44552;
  }
  SpinBox_Gaerungskorrektur -> setValue(d);

  spinBox_MaxBewertungSterne->setValue(settings.value("MaxAnzahlSterne").toInt());

  settings.endGroup();

  //PDF Einstellungen
  settings.beginGroup("PDF");
  s = settings.value("zoomSpickzettel").toString();
  if (s == ""){
    settings.setValue("zoomSpickzettel",1);
    d = 1;
  }
  else {
    d = s.toDouble();
  }
  doubleSpinBox_zoomSpickzettel->setValue(d);

  s = settings.value("zoomZusammenfassung").toString();
  if (s == ""){
    settings.setValue("zoomZusammenfassung",1);
    d = 1;
  }
  else {
    d = s.toDouble();
  }
  doubleSpinBox_zoomZusammenfassung->setValue(d);

  s = settings.value("startPDFBetrachter").toString();
  bool b;
  if (s == ""){
    settings.setValue("startPDFBetrachter",false);
    b = false;
  }
  else {
    b = settings.value("startPDFBetrachter").toBool();
  }
  checkBox_PDFProgStarten->setChecked(b);

  lineEdit_PDFBetrachter->setText(settings.value("PDFProg").toString());

  settings.endGroup();

}

void EinstellungsdialogImpl::LeseKonfigAnsicht()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("Ansicht");
  //Systemfont benutzen
  checkBox_FontSystem->setChecked(settings.value("checkBox_FontSystem").toBool());
  on_checkBox_FontSystem_clicked();

  //Schriftart
  fontComboBox->setCurrentText(settings.value("fontComboBox_Schriftname").toString());

  //Schriftgröße
  spinBox_fontSize->setValue(settings.value("spinBox_Schriftgroesse").toInt());

  //Keine Meldungen beim start anzeigen
  checkBox_MsgNichtAktiv->setChecked(settings.value("checkBox_MsgNichtAnzeigen").toBool());

  //tatsächlicher Endvergärungsgrad (EVG)
  checkBox_Ansicht_BuG_tevg->setChecked(settings.value("checkBox_Ansicht_BuG_tevg").toBool());

  //Scheinbarer Endvergärungsgrad (EVG)
  checkBox_Ansicht_BuG_sevg->setChecked(settings.value("checkBox_Ansicht_BuG_sevg").toBool());

  //Sudhausausbeute
  checkBox_Ansicht_BuG_shab->setChecked(settings.value("checkBox_Ansicht_BuG_shab").toBool());

  //effektive Sudhausausbeute
  checkBox_Ansicht_BuG_eshab->setChecked(settings.value("checkBox_Ansicht_BuG_eshab").toBool());

  //Akloholgehalt
  checkBox_Ansicht_BuG_alc->setChecked(settings.value("checkBox_Ansicht_BuG_alc").toBool());

  //Kosten pro Liter
  checkBox_Ansicht_BuG_kpl->setChecked(settings.value("checkBox_Ansicht_BuG_kpl").toBool());

  //Grünschlauchzeitpunkt
  checkBox_Ansicht_BuG_gszp->setChecked(settings.value("checkBox_Ansicht_BuG_gszp").toBool());

  //Spundungsdruck
  checkBox_Ansicht_BuG_sd->setChecked(settings.value("checkBox_Ansicht_BuG_sd").toBool());

  //Speisemenge gesamt
  checkBox_Ansicht_BuG_smg->setChecked(settings.value("checkBox_Ansicht_BuG_smg").toBool());

  //Haushaltszuckergabe gesamt
  checkBox_Ansicht_BuG_hzmg->setChecked(settings.value("checkBox_Ansicht_BuG_hzmg").toBool());

  //Traubenzuckergabe gesamt
  checkBox_Ansicht_BuG_tzmg->setChecked(settings.value("checkBox_Ansicht_BuG_tzmg").toBool());

  //Speisemenge pro 0,5 Liter Flasche
  checkBox_Ansicht_BuG_s05->setChecked(settings.value("checkBox_Ansicht_BuG_s05").toBool());

  //Haushaltszuckergabe pro Flasche 0,5 Liter
  checkBox_Ansicht_BuG_hz05->setChecked(settings.value("checkBox_Ansicht_BuG_hz05").toBool());

  //Traubenzuckergabe pro Flasche 0,5 Liter
  checkBox_Ansicht_BuG_tz05->setChecked(settings.value("checkBox_Ansicht_BuG_tz05").toBool());

  //Speisemenge pro 0,33 Liter Flasche
  checkBox_Ansicht_BuG_s033->setChecked(settings.value("checkBox_Ansicht_BuG_s033").toBool());

  //Haushaltszuckergabe pro Flasche 0,33 Liter
  checkBox_Ansicht_BuG_hz033->setChecked(settings.value("checkBox_Ansicht_BuG_hz033").toBool());

  //Traubenzuckergabe pro Flasche 0,33 Liter
  checkBox_Ansicht_BuG_tz033->setChecked(settings.value("checkBox_Ansicht_BuG_tz033").toBool());

  settings.endGroup();
}

void EinstellungsdialogImpl::SchreibeKonfigAnsicht()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("Ansicht");

  //Systemfont benutzen
  settings.setValue("checkBox_FontSystem", checkBox_FontSystem->isChecked());

  //Schriftart
  settings.setValue("fontComboBox_Schriftname", fontComboBox->currentText());

  //Schriftgröße
  settings.setValue("spinBox_Schriftgroesse", spinBox_fontSize->value());

  //Beim Programmstart keine Meldungen anzeigen
  settings.setValue("checkBox_MsgNichtAnzeigen", checkBox_MsgNichtAktiv -> isChecked());

  //tatsächlicher Endvergärungsgrad (EVG)
  settings.setValue("checkBox_Ansicht_BuG_tevg", checkBox_Ansicht_BuG_tevg -> isChecked());

  //Scheinbarer Endvergärungsgrad (EVG)
  settings.setValue("checkBox_Ansicht_BuG_sevg", checkBox_Ansicht_BuG_sevg -> isChecked());

  //Sudhausausbeute
  settings.setValue("checkBox_Ansicht_BuG_shab", checkBox_Ansicht_BuG_shab -> isChecked());

  //effektive Sudhausausbeute
  settings.setValue("checkBox_Ansicht_BuG_eshab", checkBox_Ansicht_BuG_eshab -> isChecked());

  //Akloholgehalt
  settings.setValue("checkBox_Ansicht_BuG_alc", checkBox_Ansicht_BuG_alc -> isChecked());

  //Kosten pro Liter
  settings.setValue("checkBox_Ansicht_BuG_kpl", checkBox_Ansicht_BuG_kpl -> isChecked());

  //Grünschlauchzeitpunkt
  settings.setValue("checkBox_Ansicht_BuG_gszp", checkBox_Ansicht_BuG_gszp -> isChecked());

  //Spundungsdruck
  settings.setValue("checkBox_Ansicht_BuG_sd", checkBox_Ansicht_BuG_sd -> isChecked());

  //Speisemenge gesamt
  settings.setValue("checkBox_Ansicht_BuG_smg", checkBox_Ansicht_BuG_smg -> isChecked());

  //Haushaltszuckergabe gesamt
  settings.setValue("checkBox_Ansicht_BuG_hzmg", checkBox_Ansicht_BuG_hzmg -> isChecked());

  //Traubenzuckergabe gesamt
  settings.setValue("checkBox_Ansicht_BuG_tzmg", checkBox_Ansicht_BuG_tzmg -> isChecked());

  //Speisemenge pro 0,5 Liter Flasche
  settings.setValue("checkBox_Ansicht_BuG_s05", checkBox_Ansicht_BuG_s05 -> isChecked());

  //Haushaltszuckergabe pro Flasche 0,5 Liter
  settings.setValue("checkBox_Ansicht_BuG_hz05", checkBox_Ansicht_BuG_hz05 -> isChecked());

  //Traubenzuckergabe pro Flasche 0,5 Liter
  settings.setValue("checkBox_Ansicht_BuG_tz05", checkBox_Ansicht_BuG_tz05 -> isChecked());

  //Speisemenge pro 0,33 Liter Flasche
  settings.setValue("checkBox_Ansicht_BuG_s033", checkBox_Ansicht_BuG_s033 -> isChecked());

  //Haushaltszuckergabe pro Flasche 0,33 Liter
  settings.setValue("checkBox_Ansicht_BuG_hz033", checkBox_Ansicht_BuG_hz033 -> isChecked());

  //Traubenzuckergabe pro Flasche 0,33 Liter
  settings.setValue("checkBox_Ansicht_BuG_tz033", checkBox_Ansicht_BuG_tz033 -> isChecked());

  settings.endGroup();
}


void EinstellungsdialogImpl::SchreibeKonfigErweitert()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("Erweitert");

  //Korrekturfaktor zur umrechnung von Brix nach Plato
  settings.setValue("FaktorBrixPlato", SpinBox_FaktorBrixPlato -> value());
  //Formel zur Umrechnung von Brix nach Plato im Gärverlauf
  settings.setValue("FormelBrixPlato", comboBox_FormelBrixPlato -> currentText());
  //Gärungskorrektur
  settings.setValue("Gaerungskorrektur", SpinBox_Gaerungskorrektur -> value());
  //Maximale Anzahl Sterne
  settings.setValue("MaxAnzahlSterne", spinBox_MaxBewertungSterne -> value());

  settings.endGroup();

  //PDF Einstellungen schreiben
  settings.beginGroup("PDF");

  settings.setValue("zoomSpickzettel",doubleSpinBox_zoomSpickzettel->value());
  settings.setValue("zoomZusammenfassung",doubleSpinBox_zoomZusammenfassung->value());
  settings.setValue("startPDFBetrachter",checkBox_PDFProgStarten->isChecked());
  settings.setValue("PDFProg", lineEdit_PDFBetrachter->text());
  settings.endGroup();

}


void EinstellungsdialogImpl::on_comboBox_FormelBrixPlato_textChanged(QString )
{
}

void EinstellungsdialogImpl::on_comboBox_FormelBrixPlato_currentIndexChanged(QString )
{
  QString s = comboBox_FormelBrixPlato -> currentText();
  if (s == "Kleier"){
    widget_Gaerungskorrektur -> show();
  }
  else {
    widget_Gaerungskorrektur -> hide();
  }
}

void EinstellungsdialogImpl::on_pushButton_SetDefaultErweitert_clicked()
{
  SpinBox_FaktorBrixPlato -> setValue(1.03);
  comboBox_FormelBrixPlato -> setCurrentIndex(0);
  SpinBox_Gaerungskorrektur -> setValue(0.44552);
}

void EinstellungsdialogImpl::on_checkBox_NativStyle_clicked()
{
  StyleFelderEinausblenden();
}

void EinstellungsdialogImpl::on_comboBox_Farbgebung_currentIndexChanged(int )
{
  StyleFelderEinausblenden();
}


void EinstellungsdialogImpl::on_checkBox_PDFProgStarten_clicked()
{
  if (checkBox_PDFProgStarten->isChecked()){
    widget_PDFProg->setVisible(true);
  }
  else {
    widget_PDFProg->setVisible(false);
  }
}

void EinstellungsdialogImpl::on_pushButton_ProgPDF_clicked()
{
  QFileDialog fd(this);
  QString fileName = fd.getOpenFileName(this, trUtf8("Bitte wähle einen PDF Betrachter aus"), "", "*");
  if (fileName != ""){
    lineEdit_PDFBetrachter->setText(fileName);
  }
}

void EinstellungsdialogImpl::on_checkBox_FontSystem_clicked()
{
  if (checkBox_FontSystem->isChecked()) {
    widget_font->setEnabled(false);
    QFont f;
    spinBox_fontSize->setValue(f.pointSize());
    fontComboBox->setCurrentFont(f);
  }
  else {
    widget_font->setEnabled(true);
  }
}
