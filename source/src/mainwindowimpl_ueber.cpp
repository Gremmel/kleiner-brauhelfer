#include "mainwindowimpl.h"

#include <QtGlobal>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include "definitionen.h"
#include "errormessage.h"

void MainWindowImpl::ErstelleUeber()
{
    QString seite, kopf, style;

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

    seite = "<h1 style=\"text-align: center;\">" + QString(TARGET) + "</h1>\
            <h3 style=\"text-align: center;\">v" + QString(VERSION) + "</h3>\
            <p style=\"text-align: center;\"><a href=\"http://www.joerum.de/kleiner-brauhelfer\">http://www.joerum.de/kleiner-brauhelfer</a></p>\
            <p style=\"text-align: center;\">&nbsp;</p>\
            <h3 style=\"text-align: center;\">" + trUtf8("Entwickler") + "</h3>\
            <p style=\"text-align: center;\"><em>Gremmel<br/></em><a href=\"mailto:kleiner-brauhelfer@soseies.de\">kleiner-brauhelfer@soseies.de</a></p>\
            <p style=\"text-align: center;\">&</p>\
            <p style=\"text-align: center;\"><em>Frédéric<br/></em><a href=\"mailto:bourgeoislab@gmail.com\">bourgeoislab@gmail.com</a></p>\
            <p style=\"text-align: center;\">&nbsp;</p>\
            <h3 style=\"text-align: center;\">" + trUtf8("Source code") + "</h3>\
            <p style=\"text-align: center;\"><a href=\"http://github.com/Gremmel/kleiner-brauhelfer/\">http://github.com/Gremmel/kleiner-brauhelfer</a></p>\
            <p style=\"text-align: center;\">&nbsp;</p>\
            <p style=\"text-align: center;\">&nbsp;</p>\
            <p style=\"text-align: center;\">" + trUtf8("Dieses Programm ist lizenziert unter den Bedingungen der GNU General Public License Version 3.") + "</p>\
            <p style=\"text-align: center;\">" + trUtf8("Verfügbar online unter") + " <a href=\"http://www.gnu.org/licenses/gpl.html\">http://www.gnu.org/licenses/gpl.html</a></p>\
            <p style=\"text-align: center;\">&nbsp;</p>\
            <p style=\"text-align: center;\">" + trUtf8("Kompiliert mit") + " Qt " + QString(QT_VERSION_STR) + "</p>\
            <p style=\"text-align: center;\">&nbsp;</p>\
            <p style=\"text-align: center;\">&nbsp;</p>\
            <p style=\"text-align: center;\"><strong>" + trUtf8("Besonderer Dank geht an") + "</strong></p>\
            <p style=\"text-align: center;\"><a href=\"http://www.hobbybrauer.de\">http://www.hobbybrauer.de</a></p>\
            <p style=\"text-align: center;\">(" + trUtf8("für die Fülle an Informationen und Hilfe") + ")</p>";

    webView_Ueber->setHtml(kopf + style + "</head><body>" + seite + "</body></html>");
    webView_Ueber->setLinksExternal(true);
}
