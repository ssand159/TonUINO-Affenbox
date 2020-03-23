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

Funktionsumfang/ Änderungen

- Start Up Sound und ShutDown Sound über ein define eingefügt.

- Zu-/Abschalten des Lautsprechers um das Störgeräusch des DF Players beim Einschalten zu entfernen. Setzt eine zusätzliche Schaltung voraus.

- Ausschalten über langen druck auf Pause Taste, setzt geeignete Hardware vorraus (z.B. Pololu Switch)

- Serielle Ausgaben können über ein #define zu/abgeschaltete werden um Programmspeicher zu sparen.

- Rotary Encoder KY 040 unterstützung zur Lautstärkeregelung

- Drehschalter unterstützung um Ordner abzuspielen , Modifier auszuwählen oder für Systembefehle (Pause, Volume, ...)

- Hörbuchmodus Von-Bis 

- erweiterter Hörbuchspeicher um zusammen mit dem Hörbuchmodus Von-Bis mehrere Hörbücher in einem Ordner abzulegen

- Spiele: Puzzle, Quiz, Button Smash



HINWEIS: Bitte den mp3 Ordner erneuern!

Viele Funktionen, sind über #defines aktivierbar und/pder konfigurierbar 




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



# Quelle für MP3s

Eine Datenbank mit Piktogrammen und MP3 mit Passenden Wörtern in verschiednen Sprachen findet ihr hier

http://www.arasaac.org/descargas.php


## Feature: Drehschalter

Der Drehschalter ist alternatives/ergänzendes Bedienkonzept für den TonUINO.
Ihr benötigt dazu einen einpoligen Drehschalter, und Widerstände der selben Größe (z.B. 1k Ohm) in der Anzahl der Drehschalterstellungen -1.

Die Widerstände werden zwischen die Schalterstellungen gelötet und bilden einen Spannungsteiler, der für jede Stellung eine Spannung definiert. Der erste Schalterkontakt wird mit GND verbunden, der Letzte mit 5V. Der Mittelkontakt muss mit einem Analogen Input verbunden werden.

Über das #define ROTARY_SWITCH wird der Drehschalter im Programm ergänzt
über folgende #defines wird die Hardware konfiguriert:


*#define ROTARY_SWITCH_POSITIONS 12*  //Anzahl Schalterstellungen

*#define ROTARY_SWITCH_TOLERNACE 0.15* //Toleranz der Schwellspannung des Spannungsteiler

*#define ROTARY_SWITCH_TRIGGER_TIME 2000* //Zeit die vergehen muss damit die gewählte Schalterstellung angenommen wird in ms


Die einzelnen Postitionen des Drehschalter werden in einem Array definiert. Diese Array ist in void setup () deklariert.
Hier ein Beispiel:




 //              |Folder No.|                 |Mode|                               |Special|                  |Special2|
 
  RotSwMap[0][0] =     -1;     RotSwMap[0][1] =  Volume_2;            RotSwMap[0][2] = 5;         RotSwMap[0][3] = 0;
  
  RotSwMap[1][0] =     -1;     RotSwMap[1][1] =  Volume_2;            RotSwMap[1][2] = 15;        RotSwMap[1][3] = 0;
  
  RotSwMap[2][0] =     3;      RotSwMap[2][1] =  Party_3;             RotSwMap[2][2] = 0;         RotSwMap[2][3] = 0;
  
  RotSwMap[3][0] =     4;      RotSwMap[3][1] =  Party_3;             RotSwMap[3][2] = 0;         RotSwMap[3][3] = 0;
  
  RotSwMap[4][0] =     5;      RotSwMap[4][1] =  Party_3;             RotSwMap[4][2] = 0;         RotSwMap[4][3] = 0;
  
  RotSwMap[5][0] =     6;      RotSwMap[5][1] =  Party_3;             RotSwMap[5][2] = 0;         RotSwMap[5][3] = 0;
  
  RotSwMap[6][0] =     7;      RotSwMap[6][1] =  Party_3;             RotSwMap[6][2] = 0;         RotSwMap[6][3] = 0;
  
  RotSwMap[7][0] =     8;      RotSwMap[7][1] =  Party_3;             RotSwMap[7][2] = 0;         RotSwMap[7][3] = 0;
  
  RotSwMap[8][0] =     0;      RotSwMap[8][1] =  2;                   RotSwMap[8][2] = 0;         RotSwMap[8][3] = 0;
  
  RotSwMap[9][0] =     0;      RotSwMap[9][1] =  8;                   RotSwMap[9][2] = 1;         RotSwMap[9][3] = 0;
  
  RotSwMap[10][0] =    -1;     RotSwMap[10][1] = Remove_Modifier_6;   RotSwMap[10][2] = 1;        RotSwMap[10][3] = 0;
  
  RotSwMap[11][0] =    -1;     RotSwMap[11][1] = Pause_1;             RotSwMap[11][2] = 5;        RotSwMap[11][3] = 5;
  
  
  

Das Array ist genau wie die RFID Tags aufgebaut.

Array Slot [x][0] enthält etweder die Ordnernummer (1-99), die Modifierkennung (0) oder eine Kennung für Saystembefehle (-1)
Array Slot [x][1] enthält den Mode und kann über die Enums auch als Wort eingetragen werden:


**für [x][0] = 1-99:**

 - Hoerspiel_1 = 1,
  
 - Album_2 = 2,
  
-  Party_3 = 3,
  
-  Einzel_4 = 4,
  
-  Hoerbuch_5 = 5,
  
-  Hoerspiel_von_bis_7 = 7,
  
-  Album_von_bis_8 = 8,
  
-  Party_von_bis_9 = 9,
  
-  Hoerbuch_von_bis_10 = 10,
  
 - Puzzle_11 = 11




**für [x][0] = 0:**

  - SleepTimer_1 = 1,
  
-  FreezeDance_2 = 2,
  
-  Locked_3 = 3,
  
 - ToddlerMode_4 = 4,
  
-  KindergardenMode_5 = 5,
  
-  RepeatSingleModifier_6 = 6,
  
-  FeedbackModifier_7 = 7,
  
-  PuzzleGame_8 = 8,
  
-  QuizGame_9 = 9,
  
-  ButtonSmash_10 = 10




 **für [x][0] = -1:**

-  Pause_1 = 1,
  
 - Volume_2 = 2,
  
-  Forward_3 = 3,
  
 - Backward_4 = 4,
  
-  ShutDown_5 = 5,
  
-  Remove_Modifier_6





Array Slot [x][1] & Array Slot [x][2], stehen für die Specialwerte die je nach Mode eine andere Bedeutung haben:


- Von-Bis Modis: [x][1] = Von, [x][2] = bis

- Einzel oder Puzzleteil: [x][1] = Tracknummer

- Quiz: [x][1] = Ordner Nr. mit den Fragen

- Button Smash: [x][1] = Ordner Nr. mit den Sounds, [x][2] = Lauttstärke




## Feature: Rotary Encoder

Mit dem Rotary Encoder kann die Lautstärke eingestellt werden.
Um diesen nutzen zu können muss die Bibliothen im data Ordner eingebunden werden.


## Feature: Lautsprecherschalter

Mit einer antiseriellen verschaltung zweier MOSFET, kann man über den Arduino gesteuert,den Lautsprecher Ein- und Ausschalten.
In meinem Fork setzte ich das ein um zu Beginn den Lautsprecher auszuschalten. 
Denkbar wäre auch den Lautsprecher für die Verwendung eines Kopfhörers.


Bei der Wahl der MOSFET muss man auf einen möglichst geringen Widerstand achten.

