#include <QApplication>
#include <QTranslator>
#include <QDebug>
#include <QStyleFactory>
#include "mainwindowimpl.h"
#include "connection.h"
#include "definitionen.h"

//
int main(int argc, char ** argv)
{

  QApplication app( argc, argv );

  QString str;
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

  settings.beginGroup("Style");

  //Nativen Betriebssystem Style verwenden
  str = settings.value("NativStyle").toString();
  if (str == ""){
    settings.setValue("NativStyle", false);
  }

  bool nativStyle = settings.value("NativStyle").toBool();

  //Farbgebung - 0 = Systemfarben - 1 = Hell - 2 = Dunkel
  str = settings.value("Farbgebung").toString();
  if (str == ""){
    settings.setValue("Farbgebung", 1);
  }

  int farbgebung = settings.value("Farbgebung").toInt();

  //Farbe für Auswahl
  str = settings.value("FARBE_HIGHLIGHT").toString();
  if (str == ""){
    settings.setValue("FARBE_HIGHLIGHT", QColor::fromRgb(FARBE_STYLE_HIGHLIGHT).name());
  }
  QColor color;
  color.setNamedColor(settings.value("FARBE_HIGHLIGHT").toString());

  settings.endGroup();

  //Schrift für die Anwendung setzten wenn nicht die Systemschrift benutzt werden soll
  settings.beginGroup("Ansicht");

  //System Einstellung benutzen
  str = settings.value("checkBox_FontSystem").toString();
  if (str == "") {
    settings.setValue("checkBox_FontSystem", true);
  }
  bool systemfont = settings.value("checkBox_FontSystem").toBool();

  //Schriftname
  str = settings.value("fontComboBox_Schriftname").toString();
  if (str == "") {
    QFont f;
    settings.setValue("fontComboBox_Schriftname", f.family());
  }

  //Schriftgröße
  str = settings.value("spinBox_Schriftgroesse").toString();
  if (str == "") {
    QFont f;
    settings.setValue("spinBox_Schriftgroesse", f.pointSize());
  }

  if (!systemfont) {
    QFont f;
    f.setFamily(settings.value("fontComboBox_Schriftname").toString());
    f.setPointSize(settings.value("spinBox_Schriftgroesse").toInt());
    app.setFont(f);
  }

  settings.endGroup();

  //Fusion style setzten
  if (!nativStyle){
    app.setStyle(QStyleFactory::create("fusion"));

    //Dunkel
    if (farbgebung == 2){
      QPalette palette;
//			palette.setColor(QPalette::Window, QColor(53,53,53));
//			palette.setColor(QPalette::WindowText, Qt::white);
//			palette.setColor(QPalette::Base, QColor(34,34,34));
//			palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
//			palette.setColor(QPalette::Text, Qt::white);
//			palette.setColor(QPalette::Button, QColor(53,53,53));
//			palette.setColor(QPalette::ButtonText, Qt::white);
//			palette.setColor(QPalette::BrightText, Qt::red);


      palette.setColorGroup(QPalette::Disabled,
                            Qt::gray,//windowText
                            QColor(53,53,53),//button
                            QColor(255,255,255),//light
                            QColor(255,255,255),//dark
                            QColor(255,255,255),//mid
                            Qt::gray,//text
                            Qt::red,//bright_text
                            QColor(34,34,34),//base
                            QColor(53,53,53));//window
      palette.setColorGroup(QPalette::Inactive,
                            Qt::white,//windowText
                            QColor(53,53,53),//button
                            QColor(255,255,255),//light
                            QColor(255,255,255),//dark
                            QColor(255,255,255),//mid
                            Qt::white,//text
                            Qt::red,//bright_text
                            QColor(34,34,34),//base
                            QColor(53,53,53));//window
      palette.setColorGroup(QPalette::Active,
                            Qt::white,//windowText
                            QColor(53,53,53),//button
                            QColor(255,255,255),//light
                            QColor(255,255,255),//dark
                            QColor(255,255,255),//mid
                            Qt::white,//text
                            Qt::red,//bright_text
                            QColor(34,34,34),//base
                            QColor(53,53,53));//window

      palette.setColor(QPalette::Highlight, color);
      palette.setColor(QPalette::HighlightedText, Qt::black);
      palette.setColor(QPalette::ToolTipBase, QColor(55,55,55));
      palette.setColor(QPalette::ToolTipText, QColor(255,255,255));

      app.setPalette(palette);
    }
    //Hell
    else if (farbgebung == 1){
      QPalette palette;
//			palette.setColor(QPalette::Window, QColor(226,226,226));
//			palette.setColor(QPalette::WindowText, Qt::black);
//			palette.setColor(QPalette::Base, QColor(255,255,255));
//			palette.setColor(QPalette::AlternateBase, QColor(206,206,206));
//			palette.setColor(QPalette::Text, Qt::black);
//			palette.setColor(QPalette::Button, QColor(226,226,226));
//			palette.setColor(QPalette::ButtonText, Qt::black);
//			palette.setColor(QPalette::BrightText, Qt::red);

//			app.setPalette(palette);
      palette.setColorGroup(QPalette::Disabled,
                            Qt::gray,//windowText
                            QColor(226,226,226),//button
                            QColor(255,255,255),//light
                            QColor(255,255,255),//dark
                            QColor(255,255,255),//mid
                            Qt::darkGray,//text
                            Qt::red,//bright_text
                            QColor(255,255,255),//base
                            QColor(226,226,226));//window
      palette.setColorGroup(QPalette::Active,
                            QColor(40,40,40),//windowText
                            QColor(226,226,226),//button
                            QColor(255,255,255),//light
                            QColor(255,255,255),//dark
                            QColor(255,255,255),//mid
                            QColor(40,40,40),//text
                            Qt::red,//bright_text
                            QColor(255,255,255),//base
                            QColor(226,226,226));//window

      palette.setColor(QPalette::ToolTipBase, QColor(34,34,34));
      palette.setColor(QPalette::ToolTipText, QColor(255,255,255));
      palette.setColor(QPalette::Highlight, color);
      palette.setColor(QPalette::HighlightedText, Qt::white);

      app.setPalette(palette);
    }
  }

  //Wenn die Datenbanksicherung noch vorhanden ist ist bei einem vorherigen start
  //etwas schiefgelaufen, also erst die Sicherung wieder zurücksichern
  RestorDatenbank();

  //Mit Datenbank verbinden
  if (!ErstelleVerbindung()) {
    return 1;
  }

  //Das verbinden/Updaten der Datenbank war erfolgreich somit kann die Sicherungsdatei
  //gelöscht werden
  RemoveDatenbanksicherung();

  MainWindowImpl win;
  win.show();

  app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
  return app.exec();
}
