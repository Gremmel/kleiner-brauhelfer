#include "mainwindowimpl.h"

#include <QFile>
#include <QSettings>
#include "mustache.h"
#include "definitionen.h"

void MainWindowImpl::ErstelleFlaschenlabel()
{
    QVariantHash contextVariables;
    contextVariables["Sudname"] = lineEdit_Sudname->text();

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
    QString settingsPath = QFileInfo(settings.fileName()).absolutePath() + "/";

    QFile file(settingsPath + "streifen_vorlage.svg");
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

//    svg_template.remove(0, svg_template.indexOf("<svg"));

    QFile fileHTML(settingsPath + "flaschenlabel.html");
    if (!fileHTML.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString html_template = fileHTML.readAll();
    fileHTML.close();



    QString Seite = renderer.render(html_template, &context);
//    Seite.replace(QString("##svgLabel##"), svg);

//    if (webView_Flaschenlabel->url().isEmpty())
    MyWebView::clearMemoryCaches();
    webView_Flaschenlabel->setHtml(Seite, QUrl::fromLocalFile(settingsPath));
    webView_Flaschenlabel->setLinksExternal(true);
}
