QT = gui \
 core \
 svg \
 sql \
 network \
 xml

lessThan(QT_MAJOR_VERSION, 5) | lessThan(QT_MINOR_VERSION, 5) {
 QT += webkitwidgets printsupport
} else {
 QT += webenginewidgets
}



TEMPLATE = app
TARGET = kleiner-brauhelfer
VERSION = 1.4.4.6
VERSION_SUFFIX =
VERSION_INT = 1040406

DEFINES += TARGET=\\\"$${TARGET}\\\" \
           VERSION=\\\"$${VERSION}$${VERSION_SUFFIX}\\\" \
           VERSION_INT=$${VERSION_INT}

win32:RC_ICONS += res/logo.ico
macx:ICON = res/AppIcon.icns

CONFIG += qt warn_on
DESTDIR = bin
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
FORMS = ui/mainwindow.ui \
 ui/einstellungen.ui \
 ui/ber_schuettung.ui \
 ui/ber_ibu.ui \
 ui/EingabeHVolumen.ui \
 ui/ErweiterteZutat.ui \
 ui/doubleEditLine.ui \
 ui/rastwidget.ui \
 ui/bewertung.ui \
 ui/getrohstoffvorlage.ui \
 ui/rohstoffaustauschen.ui \
 ui/malzgabe.ui \
 ui/msgdialog.ui \
 ui/hopfengabe.ui \
 ui/dialogberverdampfung.ui \
 ui/dialoginfo.ui \
 ui/anhangwidget.ui \
 ui/dialogdatum.ui \
 ui/dialogeinmaischetemp.ui \
 ui/dialogsudteilen.ui \
 ui/dialogstammwuerze.ui
HEADERS = src/mainwindowimpl.h \
 src/berechnungen.h \
 src/bierfarbe.h \
 src/connection.h \
 src/definitionen.h \
 src/errormessage.h \
 src/mydiagrammview.h \
 src/einstellungsdialogimpl.h \
 src/dialog_berschuettungimpl.h \
 src/dialog_berechne_ibuimpl.h \
 src/eingabehvolumenimpl.h \
 src/qexport.h \
 src/erweitertezutatimpl.h \
 src/doubleeditlineimpl.h \
 src/rastwidget.h \
 src/mycombobox.h \
 src/mydoublespinbox.h \
 src/myspinbox.h \
 src/faderwidget.h \
 src/bewertung.h \
 src/starview.h \
 src/getrohstoffvorlage.h \
 src/rohstoffaustauschen.h \
 src/malzgabe.h \
 src/msgdialog.h \
 src/hopfengabe.h \
 src/dialogberverdampfung.h \
 src/brauanlage.h \
 src/dialoginfo.h \
 src/anhangwidget.h \
 src/dialogdatum.h \
 src/mywebview.h \
 src/mytablewidget.h \
 src/mytablewidgetitemnumeric.h \
 src/mydsvtablemodel.h \
 src/dialogeinmaischetemp.h \
 src/dialogsudteilen.h \
 src/database.h \
 src/dialogstammwuerze.h \
 src/mustache.h \
 src/htmlhighlighter.h \
   src/svgview.h
SOURCES = src/mainwindowimpl.cpp \
 src/main.cpp \
 src/berechnungen.cpp \
 src/bierfarbe.cpp \
 src/errormessage.cpp \
 src/mydiagrammview.cpp \
 src/einstellungsdialogimpl.cpp \
 src/dialog_berschuettungimpl.cpp \
 src/dialog_berechne_ibuimpl.cpp \
 src/eingabehvolumenimpl.cpp \
 src/qexport.cpp \
 src/erweitertezutatimpl.cpp \
 src/doubleeditlineimpl.cpp \
 src/rastwidget.cpp \
 src/mycombobox.cpp \
 src/mydoublespinbox.cpp \
 src/myspinbox.cpp \
 src/faderwidget.cpp \
 src/bewertung.cpp \
 src/starview.cpp \
 src/getrohstoffvorlage.cpp \
 src/rohstoffaustauschen.cpp \
 src/malzgabe.cpp \
 src/msgdialog.cpp \
 src/hopfengabe.cpp \
 src/dialogberverdampfung.cpp \
 src/brauanlage.cpp \
 src/dialoginfo.cpp \
 src/anhangwidget.cpp \
 src/dialogdatum.cpp \
 src/mywebview.cpp \
 src/mytablewidget.cpp \
 src/mytablewidgetitemnumeric.cpp \
 src/mainwindowimpl_sudinfo.cpp \
 src/mainwindowimpl_spickzettel.cpp \
 src/mainwindowimpl_zusammenfassung.cpp \
 src/mydsvtablemodel.cpp \
 src/dialogeinmaischetemp.cpp \
 src/mainwindowimpl_ueber.cpp \
 src/dialogsudteilen.cpp \
 src/database.cpp \
 src/dialogstammwuerze.cpp \
 src/mustache.cpp \
 src/htmlhighlighter.cpp \
 src/mainwindowimpl_flaschenlabel.cpp \
   src/svgview.cpp
TRANSLATIONS += languages/kb_de.ts languages/kb_pl.ts languages/kb_de_CH.ts languages/kb_en.ts
RESOURCES += res/grafiken.qrc data/data.qrc
