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
