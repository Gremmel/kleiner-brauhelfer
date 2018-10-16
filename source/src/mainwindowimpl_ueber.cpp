#include "mainwindowimpl.h"

#include <QFile>
#include <QSettings>
#include "mustache.h"
#include "definitionen.h"

void MainWindowImpl::ErstelleUeber()
{
    QVariantHash contextVariables;
    contextVariables["AppName"] = APP_NAME;
    contextVariables["AppVersion"] = VERSION;
    contextVariables["Style"] = StyleDunkel ? "style_dunkel.css" : "style_hell.css";
    contextVariables["Entwickler"] = trUtf8("Entwickler");
    contextVariables["SourceCode"] = trUtf8("Source code");
    contextVariables["License1"] = trUtf8("Dieses Programm ist lizenziert unter den Bedingungen der GNU General Public License Version 3.");
    contextVariables["License2"] = trUtf8("Verfügbar online unter");
    contextVariables["Kompiliert"] = trUtf8("Kompiliert mit");
    contextVariables["QtVersion"] = QT_VERSION_STR;
    contextVariables["hobbybrauer1"] = trUtf8("Besonderer Dank geht an");
    contextVariables["hobbybrauer2"] = trUtf8("für die Fülle an Informationen und Hilfe");

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);
    QString settingsPath = QFileInfo(settings.fileName()).absolutePath() + "/";
    QFile file(":/data/ueber.html");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString html_template = file.readAll();
    file.close();

    Mustache::Renderer renderer;
    Mustache::QtVariantContext context(contextVariables);
    QString seite = renderer.render(html_template, &context);
    if (webView_Ueber->url().isEmpty())
        MyWebView::clearMemoryCaches();
    webView_Ueber->setHtml(seite, QUrl::fromLocalFile(settingsPath));
    webView_Ueber->setLinksExternal(true);
}
