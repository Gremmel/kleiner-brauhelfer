# kleiner-brauhelfer
Brauplanungs-software
http://www.joerum.de/kleiner-brauhelfer/
## Download letzte Version
- [Version 1.4.4.5](https://github.com/Gremmel/kleiner-brauhelfer/releases/tag/v1.4.4.5)
- [macOS](https://github.com/realholgi/kleiner-brauhelfer/releases) (Dank an **realholgi**)
## Änderungen & Erweiterungen
### Version 1.4.4.6 (in Entwicklung)
- Fix: Sonderzeichenproblem bei Sud kopieren
- Fix: Berechnung der Farbe mit weiteren Zutaten
- Fix: Darstellungsproblem bei langen Texte in Sudauswahl

### Version 1.4.4.5
- Fix: Umlaute in Flaschenlabel
- Flaschenlabel Aussenkontur erscheint jetzt auch im PDF
- "x" Button zum leeren der Sudinfo Filter auswahl
- Sortierung und Auswahl in der Sudauswahl merken
- url für meldungsabfrage auf https geändert.

### Version 1.4.4.4
- Fix: Datenbank update bei Erstinstallation
- HTML Tags Glas-hell und Glas-dunkel
- HTML Tags Malz-rest, Hopfen-rest, etc.
- HTML Templates direkt unter Einstellungen editierbar
- Textfilter für Sudauswahl sucht auch in Malzschüttungen, Hopfengaben, weitere Zutaten Gaben, Hefeauswahl und Kommentar
- Erstellen von Flaschenlabels integriert. 
  - Es können SVG Bilder aus den Anhängen ausgewählt werden. 
  - Tags werden wie beim Spickzettel etc. ersetzt.
- Fix: Berechnung Stammwürze bei einer änderung der Temperatur im Dialog "Berechnung Stammwürze"
### Version 1.4.4.3
[Download](https://github.com/Gremmel/kleiner-brauhelfer/releases/tag/v1.4.4.3)
- Deutsch überarbeitet (grosser Dank an **Radulph**)
- Fix: Windows SSL Bibliotheken für Anleitung integriert
- Fix: Absturz in "Bewertung" bei Sprachumschaltung
- Min Max werte gesamte Kochdauer auf 1 - 999 min geändert
- Möglichkeit eine Sicherungskopie beim Start anzulegen
- Formatierung von Spickzettel & Zusammenfassung lässt sich über HTML Datei anpassen
  - Windows: C:\\Users\\&lt;benutzer&gt;\\AppData\\Roaming\\Gremmelsoft
  - Linux: /home/&lt;benutzer&gt;/.config/Gremmelsoft/
  - macOS: /Users/&lt;benutzer&gt;/.kleiner-brauhelfer
  - Templatesystem von Robert Knight (https://github.com/robertknight/qt-mustache)
- Zusammenfassung enthält auch Rezeptwerte
- Spalte Bewertung in Sudauswahl
- Import von "Maische Malz und Mehr" Rezepte unterstütz "Weitere Zutaten" und VorderwürzeHopfung
### Version 1.4.4.2
[Download](https://github.com/Gremmel/kleiner-brauhelfer/releases/tag/v1.4.4.2)
- ~~Umstellung auf Qt 5.10~~ (weiterhin Qt5.3)
- ~~Windows Versionen für 32bit und 64bit Architektur~~
- Rohstofftabellen neu organisiert
- Direktes Drucken wird nicht mehr unterstützt (PDF erstellen bleibt wie gewohnt)
- EBC Wert bis 2000
- Weitere Zutaten können auch nach dem Zustand "gebraut" hinzugefügt werden
- Sud kann wärend des Brauens oder Gärung in zwei Teile geteilt werden 
- Überarbeitung des Dialogs für die Stammwürzeeingabe
- Terrill Formel für die Berechnung der Stammwürze mit einem Refraktometer
- Fix: falsche Dropdown Werte (Hopfengabe Gewicht statt IBU, Weitere Zutaten Zugabezeitpunkt)
- Fix: Falsche Werte bei Spickzettel / Zusammenfassung
- Datum in Brau&Gärdaten wird beim laden eines noch nicht gebrauten Sudes auf das heutige Datum gesetzt
- Neue Spalte "Hefe" in der Brauübersicht
- Tabelle "Rohstoffvorlage" lässt sich sortieren (keine numerische Sortierung)
- Textfilter für den Sudname in der Sudauswahl
### Version 1.4.4.1
[Download](https://github.com/Gremmel/kleiner-brauhelfer/releases/tag/v1.4.4.1)
- Zustand der Tabellen werden gespeichert
- Gewünschte Restalkalität kann auch negativ sein
- Rohstoffvorlagen für "Weitere Zutaten"
- Farbliche Kennung der Verfügbarkeit auch bei der Hefeauswahl
- Rohstofftabellen: farbliche Kennung, wenn Menge verbraucht
- Rohstofftabellen: farbliche Kennung des EBC Wertes
- Rohstofftabellen: farbliche Kennung, wenn Haltbarkeitsdatum vorbei
- Fix: Brauanlage bei Sudimport
### Version 1.4.4.0
[Download](https://github.com/Gremmel/kleiner-brauhelfer/releases/tag/v1.4.4.0)
- Übersetzung ins Englische (Danke an **freibadschwimmer** & **stikkx**)
- Importieren von "Maische Malz und Mehr" Rezepte (Danke an **bitwave**)
- Bessere Anzeige, ob ein Rohstoff noch vorhanden ist
- Die meisten Tabellen lassen sich sortieren
- Rohstoffvorlagen lassen sich editieren, importieren & exportieren
- Hilfe für die Berechnung der Einmaischetemperatur
