#ifndef EINSTELLUNGSDIALOGIMPL_H
#define EINSTELLUNGSDIALOGIMPL_H
//
#include <QDialog>
#include "ui_einstellungen.h"
#include "htmlhighlighter.h"
//
class EinstellungsdialogImpl : public QDialog, public Ui::Dialog
{
Q_OBJECT
private:
  void SchreibeKonfigErweitert();
  void LeseKonfigErweitert();
  void LeseKonfigAnsicht();
  void SchreibeKonfigAnsicht();
  void SchreibeKonfigFarben();
  void SchreibeKonfigDB();
  void LeseKonfigDB();
  void LeseKonfig();
  void LeseKonfigFarben();
  void LeseKonfigHtmlTemplates();
  void ErstelleIcons();
  QString DBDirVorher;
  void StyleFelderEinausblenden();
public:
  bool NeuerDBPfad;
  HtmlHighLighter* htmlHightLighter;
  EinstellungsdialogImpl( QWidget * parent = 0,  Qt::WindowFlags f = 0 );
  ~EinstellungsdialogImpl();
private slots:
  void on_pushButton_SetDefaultErweitert_clicked();
  void on_comboBox_FormelBrixPlato_currentIndexChanged(QString );
  void on_comboBox_FormelBrixPlato_textChanged(QString );
  void on_pushButton_SetDefaultFarbe_clicked();
  void on_pushButton_GetColorDia_SPB_L1_clicked();
  void on_pushButton_GetColorDia_SpB_L2_clicked();
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();
  void on_pushButton_GetPfad_clicked();
  void on_pushButton_GetColorDia_S_L1_clicked();
  void on_pushButton_GetColorDia_S_L2_clicked();
  void on_pushButton_GetColorDia_S_L3_clicked();
  void on_pushButton_GetColorDia_H_L1_clicked();
  void on_pushButton_GetColorDia_H_L2_clicked();
  void on_pushButton_GetColorDia_H_L3_clicked();
  void on_pushButton_GetColorDia_N_L1_clicked();
  void on_pushButton_GetColorDia_N_L2_clicked();
  void on_pushButton_GetColorDia_B_L1_clicked();
  void on_pushButton_GetColorDia_B_L2_clicked();
  void SeiteWechseln(QListWidgetItem *current, QListWidgetItem *previous);
  void on_checkBox_NativStyle_clicked();
  void on_comboBox_Farbgebung_currentIndexChanged(int index);
  void on_pushButton_GetColorStyleAuswahl_clicked();
  void on_checkBox_FontSystem_clicked();
  void on_pushButton_GetBackupPfad_clicked();
  void on_comboBox_htmltemplate_currentIndexChanged(const QString &filename);
  void on_pushButton_htmltemplate_save_clicked();
  void on_textEdit_htmltemplate_textChanged();
  void on_pushButton_htmltemplate_restore_clicked();
};
#endif










