# TonUINO
Die DIY Musikbox (nicht nur) für Kinder


# Change Log

## Version 2.01 (01.11.2018)
- kleiner Fix um die Probleme beim Anlernen von Karten zu reduzieren

## Version 2.0 (26.08.2018)

- Lautstärke wird nun über einen langen Tastendruck geändert
- bei kurzem Tastendruck wird der nächste / vorherige Track abgespielt (je nach Wiedergabemodus nicht verfügbar)
- Während der Wiedergabe wird bei langem Tastendruck auf Play/Pause die Nummer des aktuellen Tracks angesagt
- Neuer Wiedergabemodus: **Einzelmodus**
  Eine Karte kann mit einer einzelnen Datei aus einem Ordner verknüpft werden. Dadurch sind theoretisch 25000 verschiedene Karten für je eine Datei möglich
- Neuer Wiedergabemodus: **Hörbuch-Modus**
  Funktioniert genau wie der Album-Modus. Zusätzlich wir der Fortschritt im EEPROM des Arduinos gespeichert und beim nächsten mal wird bei der jeweils letzten Datei neu gestartet. Leider kann nur der Track, nicht die Stelle im Track gespeichert werden
- Um mehr als 100 Karten zu unterstützen wird die Konfiguration der Karten nicht mehr im EEPROM gespeichert sondern direkt auf den Karten - die Karte muss daher beim Anlernen aufgelegt bleiben!
- Durch einen langen Druck auf Play/Pause kann **eine Karte neu konfiguriert** werden
- In den Auswahldialogen kann durch langen Druck auf die Lautstärketasten jeweils um 10 Ordner oder Dateien vor und zurück gesprungen werden
- Reset des MP3 Moduls beim Start entfernt - war nicht nötig und hat "Krach" gemacht



# TonUINO_Fork-Affenbox

Own TonUINO Fork

Mein eigener Fork der TonUINO DEV 2.1

Funktionsumfang/ Änderungen:

- Unterstützung der AiO Baugruppe von Leiterkartenpiraten.de
- Start Up Sound und ShutDown Sound eingefügt. Die Sounds sind im mp3 Ordner abgelegt: Start = 264; Shut Down = 265
- Zu-/Abschalten des Lautsprechers um das Störgeräusch des DF Players beim Einschalten zu entfernen. Setzt eine zusätzliche Schaltung voraus. AiO Baugruppe bring Funktion Standardmäßig mit.
- Ausschalten über langen druck auf Pause Taste, setzt geeignete Hardware vorraus (z.B. Pololu Switch). AiO Baugruppe bring Funktion Standardmäßig mit.
- Serielle Ausgaben können über mehrere #defines zu/abgeschaltete werden um Programmspeicher zu sparen.
- Anpassung Abspielmodus Album. Hier kann ein Speicher eingestellt werden um den Fortschritt wie beim Hörbuch zu speichern. Anders als beim Hörbuch, wird allerdings immer der Track nach dem Track im Speicher geladen.
- Ergänzung um den Abspielmodus: Hörbuchmodus Von-Bis 
- Hörbuchspeicher wird nun auf der RFID Karte abgelegt und nicht mehr im EEPROM. Außnahme sind die Shortcuts, die im EEPROM gespeichert werden.
- Neue Modifier: Puzzle, Quiz, Button Smash, Calculate
- Pause bei entfernen des RFID Tags, über das Adminmenü auswählbar
- Modifier können im EEPROM abgespeichert werden, um sie bei einem Reset zu reaktivieren. 
- Mehr Shortcuts. Es gibt weitere Shortcuts, insgesamt 12. Shortcut 1 & 2 sind Standardmäßig auf langem Druck der Up & Down Tasten. Es können alle Shortcuts mit analogen Eingangswerten oder einer IR Fernbedienung, verknüpft werden.
- Neue Shortcuts Funktionen. Shortcuts können jetzt auch Modifier annehmen.
- Neue Eingabemethode: Analoger Eingang. Es kann ein Anlaoger Eingang verwendet werden um den TonUINO zu steuern oder Shortcuts zu starten. Die Eingangspegel können über das Adminmenü einglernt werden. Somit ist es egal welches System zur generierung der Pegel verwendet wird.
- Neue Eingabemethode: IR Fernbedienung. Es kann eine Infrarot Fernbedienung über das Admin Menü angelernt werden, um den TonUINO zu steuern oder Shortcuts zu starten.
- Menüs sind jetzt umlaufend und das Admin Menü beendet sich nicht automatisch, sodass man mehrere Einstellungen hintereinander vornehmen kann.
- #defines sind in der Configuration.h ausgelagert.

HINWEIS: Bitte den mp3 Ordner erneuern und den EEPROM inital löschen!


# Software Features


## Feature: Puzzle Spiel


Ein simples Spiel für den TonUINO, dass zwei zueinander gehörende Karten erwartet.

HINWEIS: Die Sounds sollten in einem Ordner mit möglichst niedriger Nummer liegen, sonst kann es zu starker Verzögerung beim abspielen kommen.

### Funktion:

1. Als erstes müsst Ihr die Modifikationskarte "Puzzlespiel" über das Admin Menü erstellen.


2. Danach müsst ihr zwei Puzzleteile über die Normale Kartenkonfiguration erstellen. 

Hierbei wählt ihr wie gewohnt den Ordner in dem eure MP3s für das Spiel liegen.

Wählt nun den Modus Puzzle.

Jetzt wählt ihr das MP3.

Zum Schluss müsst ihr dem Puzzleteil eine Nummer geben. Teile mit gleicher Nummer gehören zusammen. Die beiden Teile MÜSSEN aber mindestens unterschiedliche MP3 Nummern haben oder in verschieden Ordnern liegen!



Die Puzzleteile können im Normalen Betrieb wie ein Einzeltrack abgespielt werden.



3. Aktiviert das Spiel durch Auflegen der Modifikationskarte.



4. Jetzt seit Ihr im Spiel. 

Das Spiel akzeptiert nur zum Spiel gehörende Karten alle anderen Karten werden mit einem Signalton abgelehnt.

Ihr könnt nun ein beliebiges Puzzleteil auflegen, und das MP3 dazu wird abgespielt. Anschließend kann folgendes passieren:

- Ihr legt ein Puzzleteil mit der gleichen Nummer auf (ausgenommen das gerade aufgelgte) und bekommt ein bestätigendes Signal. Teil gefunden! Jetzt kann man ein neues Teil auflegen und weiter spielen.

- Ihr legt ein Teil mit einer anderen Nummer auf. Ein Fehlerton ertönt und Ihr müsst weiter nach dem richtigen Teil suchen.

- Ihr haltet den Up & Down Button gleichzeitig gedrückt und last ihn nach ca 1s los. Das aktuell gespeicherte Teil wird gelöscht und man kann eine neue Runde starten.

- Ihr könnt den Pausetaster drücken und hört die letzte Ausgabe erneut oder stoppt eine laufende Ausgabe



5. Das Spiel wird durch erneutes Auflegen der Modifikationskarte beendet.



## Feature: Quiz 



Ein weiteres simples Spiel auf Basis des Puzzles. Im Unterschied zum Puzzle wird hier ein Sound, aus einem im Modifire Tag hinterlegten Ordner, zufällig abgespielt. Das Ziel ist es das dazugehörige Puzzle- /Antwortteil aufzulegen.



Ihr könnt so eure schon erstellten Puzzleteile und die dazugehörigen MP3s nutzen oder neue MP3s mit Fragen erstellen, die als Antwort die Puzzleteile erwarten.



Zu Beachten ist, das die MP3 Nr der Frage mit der Nummer des Puzzleteils übereinstimmen muss. Falls das Puzzleteil direkt auf die Frage zeigt (gleiche Ordner & MP3 Nr.) wird das Antwortteil nicht aktzeptiert.



## Feature: Button Smash



Besonders kleine Kinder verstehen die Funktionen des TonUINO noch nicht so ganz und neigen dazu einfach alles aus zu probieren. Wild auf Knöpfe drücken oder wahllos Karten auflegen scheint ihnen besondere Freude zu bereiten.

Um die Kinder nicht mit der Tastensperre zu frustrieren, weil nichts passiert, könnt ihr stattdessen das Button Smash Spiel verwenden.

Hier werden aus einem Ordner, der im Modifire Tag hinterlegt ist, beliebig MP3s abgespielt.



Um die Ohren der Eltern zu schonen, müsst iht im Modifire Tag auch eine feste Lautstärke hinterlegen. Im Spiel sind alle üblichen Funktionen deaktiviert. 

Außerdem wird jede MP3 zuende gespielt, deshalb sollten die MP3s nicht zu lang sein.


# Spiel Idee



Die Idee hinter diesen Spielen ist, mit nur einem Datensatz an MP3s, ein große Funktionsvielfalt zu erhalten.



Ihr habt z.B. 4 Ordner



01 : Tiergeräusche



02 : Tiernamen deutsch



03 : Tiernamen englisch



04 : Fragen zu Tieren



In jedem dieser Ordner sind z.B. 100 MP3s, wobei immer die erste MP3 Beispielswiese "Hund" ist.

Sprich:



in Ordner 01 ist die MP3 001, bellen



in Ordner 02 ist die MP3 001, Hund ausgesprochen



in Ordner 03 ist die MP3 001, Dog ausgesprochen



in Ordner 04 die MP3 001, "Was ist des Menschen bester Freund?"



Jetzt legt Ihr zu den MP3 in den ersten 3 Ordnern je ein Puzzle-/Antwort Tag an.

Ordner 04_ hinterlegt Ihr im Quiz Modifier Tag und Ordner 01 im Button Smash Tag.



Die Puzzle-/Antwort Tags bedruckt Ihr dann entsprechend mit Bildern oder Wörtern.



Wenn ihr zusätzlich noch eine Auswahl an Puzzle-/Antwort Tags unbedruckt lasst, könnt ihr daraus ein akustisches Memory erzeugen.



## Quelle für MP3s

Eine Datenbank mit Piktogrammen und MP3 mit Passenden Wörtern in verschiednen Sprachen findet ihr hier

http://www.arasaac.org/descargas.php


## Hörbuch von bis

Es gibt nun auch die Möglichkeit einen Bereich von MP3s in einem Ordner als Hörbuch zu definieren.

Um mit diesem neuen Abspielmodus mehere Hörbücher innerhalb eines Ordner wie gehabt nutzen zu können, wurde die Möglichkeit eingebaut mehrere Speicherplätze in einem Ordner zu vergeben.

Beim Knofigurieren einer neue Karte gibt es entsprechend neue Menüpunkte.


# Hardware Features


Alle Hardware bezognen Features sind über #defines zu-/abwählbar und konfigurierbar, wenn nötig.


## Feature: Lautsprecherschalter

Mit einer antiseriellen verschaltung zweier MOSFET, kann man über den Arduino gesteuert,den Lautsprecher Ein- und Ausschalten.
In meinem Fork setzte ich das ein um zu Beginn den Lautsprecher auszuschalten. 
Denkbar wäre auch den Lautsprecher für die Verwendung eines Kopfhörers.


Bei der Wahl der MOSFET muss man auf einen möglichst geringen Widerstand achten.


Über das **#define SPEAKER_SWITCH** wird der Lautsprecherschalter im Programm ergänzt.
Über die folgenden #defines wird die Hardware konfiguriert:


- *#define SpeakerOnPin 8* //digital IO Pin
