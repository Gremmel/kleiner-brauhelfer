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

    QFile file(settingsPath + "flaschenlabel.html");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString html_template = file.readAll();
    file.close();

    Mustache::Renderer renderer;
    Mustache::QtVariantContext context(contextVariables);
    QString seite = renderer.render(html_template, &context);
    if (webView_Flaschenlabel->url().isEmpty())
        MyWebView::clearMemoryCaches();
    webView_Flaschenlabel->setHtml(seite, QUrl::fromLocalFile(settingsPath));
    webView_Flaschenlabel->setLinksExternal(true);
}
