#include "mainwindowimpl.h"

#include <QFile>
#include <QSettings>
#include <QFileDialog>
#include <QSvgRenderer>
#include <QPrinter>
#include <QDesktopServices>
#include <QtMath>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>


#include "errormessage.h"
#include "mustache.h"
#include "definitionen.h"

#include "svgview.h"

void MainWindowImpl::ErstelleFlaschenlabel()
{
  QString current = comboBox_FLabelAuswahl->currentText();
  comboBox_FLabelAuswahl->clear();
  comboBox_FLabelAuswahl->addItem("");
  for (int i = 0; i < list_Anhang.count(); i++) {
    //wenn SVG dann auch der Flaschenlabel auswahl hinzufügen
    if (list_Anhang[i]->getPfad().indexOf(".svg") > 0) {
      comboBox_FLabelAuswahl->addItem(list_Anhang[i]->getPfad());
    }
  }
  comboBox_FLabelAuswahl->setCurrentText(current);

}

void MainWindowImpl::LadeFlaschenlabel()
{
  QString FullPfad;
  for (int i = 0; i < list_Anhang.count(); i++) {
    if (list_Anhang[i]->getPfad() == comboBox_FLabelAuswahl->currentText()) {
      FullPfad = list_Anhang[i]->getFullPfad();
    }
  }

  QVariantHash contextVariables;
  contextVariables["Sudname"] = lineEdit_Sudname->text();

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
  QString settingsPath = QFileInfo(settings.fileName()).absolutePath() + "/";

  QFile file(FullPfad);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  QString svg_template = file.readAll();

  Mustache::Renderer renderer;
  Mustache::QtVariantContext context(contextVariables);
  QString svg = renderer.render(svg_template, &context);
  file.close();

  QFile filenew(settingsPath + "streifen.svg");
  if (filenew.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&filenew);
    out << svg << endl;
    filenew.close();
  }

  FLabel_svgView->openFile(settingsPath + "streifen.svg");

  if (checkBox_FLabelSVGViewKontur->isChecked()) {
    FLabel_svgView->setViewOutline(true);
  }
  else {
    FLabel_svgView->setViewOutline(false);
  }

}

void MainWindowImpl::on_pushButton_FlaschenlabelPDF_clicked()
{
  QString Sudname = lineEdit_Sudname->text() + "_label";

  // letzten Pfad einlesen
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER,
                     APP_KONFIG);
  settings.beginGroup("PDF");
  QString p = settings.value("recentPDFPath", QDir::homePath()).toString();

  QString fileName =
      QFileDialog::getSaveFileName(this, trUtf8("PDF speichern unter"),
                                   p + "/" + Sudname + ".pdf", "PDF (*.pdf)");
  if (!fileName.isEmpty()) {
    // pdf speichern
    QPrinter printer(QPrinter::HighResolution); //create your QPrinter (don't need to be high resolution, anyway)
    printer.setPageSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Portrait);
    printer.setPageMargins (spinBox_FLabel_RandLinks->value(),spinBox_FLabel_RandOben->value(),spinBox_FLabel_RandRechts->value(),spinBox_FLabel_RandUnten->value(),QPrinter::Millimeter);
    printer.setFullPage(false);
    printer.setOutputFileName(fileName);
    printer.setOutputFormat(QPrinter::PdfFormat); //you can use native format of system usin QPrinter::NativeFormat

    const QSize sizeSVG = FLabel_svgView->svgSize();

    qreal seitenverhaeltnisSVG = qreal(sizeSVG.width()) / qreal(sizeSVG.height());
    qreal widthPageMM = printer.widthMM();
    qreal heightPageMM = printer.heightMM();
    qreal widthPagePx = printer.width();
    qreal faktorPxPerMM = widthPagePx / widthPageMM;

    qreal breiteMM = spinBox_BreiteLabel->value();
    qreal SVGhoeheMM = (breiteMM / seitenverhaeltnisSVG);
    qreal hoehePx = SVGhoeheMM * faktorPxPerMM;
    qreal abstandMM = spinBox_AbstandLabel->value();
    qreal abstandPx = abstandMM * faktorPxPerMM;

    //Gewünschte Anzahl
    int totalCount = spinBox_AnzahlLabels->value();

    //Anzahl der Streifen pro Seite
    int countPerPage = int(heightPageMM / (SVGhoeheMM + abstandMM));

    QPainter painter(&printer); // create a painter which will paint 'on printer'.
    int zaehler = 0;
    //Anzahl Seiten
    int pageCount = int(round(double(totalCount)/double(countPerPage) + double(0.5)));

    for (int seite = 0; seite < pageCount; seite++) {
      for( int i = 0 ; i < countPerPage; i++) {
        if (zaehler >= totalCount) {
          i = countPerPage;
        }
        else {
          FLabel_svgView->renderer()->render(&painter,QRectF(0,hoehePx*i+abstandPx*i,breiteMM * faktorPxPerMM,hoehePx));
          zaehler++;
        }
      }
      if (zaehler < totalCount) {
        printer.newPage();
      }
    }
    painter.end();

    // Pfad abspeichern
    QFileInfo fi(fileName);
    settings.setValue("recentPDFPath", fi.absolutePath());

    // open PDF
    QDesktopServices::openUrl(QUrl("file:///" + fileName));
  }

  settings.endGroup();
}


void MainWindowImpl::on_checkBox_FLabelSVGViewKontur_stateChanged(int arg1)
{
  FLabel_svgView->setViewOutline(arg1);
}


void MainWindowImpl::on_comboBox_FLabelAuswahl_activated(const QString &arg1)
{
  if (arg1.isEmpty()) {
    FLabel_svgView->scene()->clear();
    checkBox_FLabelSVGViewKontur->setDisabled(true);
  }
  else {
    LadeFlaschenlabel();
    checkBox_FLabelSVGViewKontur->setDisabled(false);
  }
}
