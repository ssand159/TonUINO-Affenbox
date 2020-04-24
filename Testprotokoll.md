# Testprotokol (WIP!!)


In diesem Protokoll möchte ich festhalten, welche Funktionen getestet wurden, ob es Auffälligkeiten gab und ob diese behoben wurden.
Jeder Punkt, der mit "ok" gekennzeichnet ist wurde validiert.


## Auffälligkeiten


- Optionsnummer wird nicht wieder gegeben; ok
- Falsche Ansage bei schnellem Optionswechsel; ok
- Voice menü: sporadisch kein Menüführung bei angespielten Tracks


## Steurung


1. Pause
	- Track pausiert; ok
	- Track spielt wieder an, wenn dieser pausiert ist; ok 

2. Up
	- nächster Track wird gespielt; ok
	- Lautstärke wird erhöht
	
3. Down
	- vorheriger Track wird gespielt; ok
	- Lautstärke wird verringert

4. dritter Button
	- nächster Track wird gespielt
	- Lautstärke wird erhöht

4. vierter Button
	- nächster Track wird gespielt
	- Lautstärke wird erhöht

5. Rotary Encoder
	- Lautstärker wird erhöht; ok
	- Lautstärke wird verringert; ok
	- Up & Down verändern die Lautstärke nicht, wenn Rotary Encoder eingebundenist; ok

6. RFID
	- konfigurierte Karte wird richtig erkannt; ok
	- neue Karte mit ungültigem Cookie wird erkannt und Konfigurationsmenü wird gestartet; ok
	- Konfigurationsmenü funktionsweise; ok

## Abspielmodis


1. Hörspielmodus
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- korrektes abspielen; ok

2. Album
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- korrektes abspielen; ok

3. Party
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- korrektes abspielen; ok

4. Single
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- korrektes abspielen; ok

5. Hörbuch
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- korrektes abspielen; ok
	- letzter Track wurde gespeichert und gelesen; ok

6. Hörspielmodus von bis
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- korrektes abspielen; ok
	- Titel vor und zurück sind gesperrt; ok 

7. Album von bis
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- korrektes abspielen; ok
	- Grenzen werden eingehalten; ok

8. Party von bis
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- korrektes abspielen; ok
	- Grenzen werden eingehalten; ok

9. Hörbuch von bis
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- korrektes abspielen; ok
	- Grenzen werden eingehalten; ok
	- letzter Track wurde gespeichert und gelesen; ok

10. Puzzleteil
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- korrektes abspielen; ok

## Admin Menü

- Aufruf des Admin Menü über drei Taste; ok
- Aufruf über Tag; ok 

1. Karte neu konfigurieren
	- erkennt Karte; ok
	- Karte wird konfiguriert; ok

2. Maximale Lautstärke
	- einstellen der Lautstärke; ok
	- Lautstärke wird übernommen; ok

3. Minimale Lautstärke
	- einstellen der Lautstärke; ok
        - Lautstärke wird übernommen; ok
	
4. Lautstärke bei Start
	- einstellen der Lautstärke; ok
        - Lautstärke wird übernommen; ok

5. EQ Konfiguration 
        - einstellen des EQ; ok
        - EQ wird übernommen; ok

6. Modifikationskarte erstellen
	- erkennt Karte; ok
        - Karte wird konfiguriert; ok
Bemerkung: Funktion einzelener Modifier unter "Modifier"

7. Shortcut erstellen
	- Pause Taste; n.ok für Pause Taste ist kein Shortcut vorgesehen
	- Up Taste; ok
	- Down Taste; ok

8. Timer
	- Standby nach eingestellter Zeit; ok

9. Einzelkarten konfigurieren
	- erkennt Karte; ok
	- Karte wird konfiguriert; ok

10. Funktion Lautstärketaster umkehren
	- Funktion wird umgekehrt; ok

11. Stopp bei entfernter Karte
	- Pause wenn karte weg; n.ok sporadisch wird dennoch next Track ausgeführt, vor allem bei Trackwechsel problematisch
	- Fortsetzen wenn Karte wieder da; ok

12. Konfigurieren des Drehschalters
	- Nur festgelegte Positionen auswählbar; ok
	- Auswahl der drei Modis: Wiedergabe, Modifikation, Systemsteuerung; ok
		- Wiedergabe konfiugurieren; ok
		- Wiedergabe abspielen; ok
		- Modifikation konfigurieren; ok
		- Modifikation ausführen; ok
		- Systemsteuerung konfigureiern; ok
		- Systembefehl Lautstärke ausführen; ok
		- Systembefehl Pause ausführen; ok
		- Systembefehl Modifikation entfernen; ok
		- Systembefehl nächster Titel; n.ok Mode wird auf 0 gesetzt und blockiert so den Titelwechsel
		- Systembefehl vorheriger Titel; n.ok Mode wird auf 0 gesetzt und blockiert so den Titelwechsel
		- Systembefehl Shut Down; ok
	- Möglichkeit mehrere Positionen zu konfigureiern; ok
	- Unkonfigurierte Position wird abgelehnt; ok
	- Annehmen der Position, erst nach Trigger Zeit; ok

## Modifier


- Speichern und Laden der Modifier, wenn konfiguriert; ok


1. Sleep Timer
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- Stop nach eingestellter Zeit; ok

2. Stopp Tanz
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- Stopp nach angegebener Zeit; ok

3. Sperre
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- Funktionen gesperrt:
		- Pause Taste; ok
		- Up Taste; ok
		- Down Taste; ok
		- RFID leser; ok
		- Taste 4
		- Taste 5
		- Rotary Encoder; n.ok
		- Drehschalter; n.ok

4. Kleinkind Modus
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- Funktionen gesperrt:
                - Pause Taste; ok
                - Up Taste; ok
                - Down Taste; ok                
                - Taste 4
                - Taste 5
                - Rotary Encoder; n.ok
                - Drehschalter; n.ok
	- Funktionen freigegeben:
		- RFID leser; ok

7. Kita Modus
	- Tag konfiguerieren; ok
        - Tag lesen; ok
        - Funktionen gesperrt:
                - Up Taste; ok
                - Down Taste; ok
	- Nächste Karte wird in Queue eingereiht und erst nach abspielen des aktuellen Tracks gesepielt; n.ok Bemerkung: neue Karte wird nicht zuverlässig in die Queue aufgenommen.
	- ShortCut wird nur gespielt und aktiviert, wenn kein Track läuft; n.ok


6. Wiederholen
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- Titel wird wiederholt; ok

7. Puzzlespiel
	- Tag konfiguerieren
	- Tag lesen
	- Spielablauf
		- unterscheiden von Puzzleteilen und normalen Tags
		- einzelnes abspielen eines Puzzleteils
		- zweites inkorrektes Teil wird erkannt und Fehlersound wird gespielt
		- zweites korrektes Teil wird erkannt und Erfolgssound wird gespielt
		- neues erstes Teil wird abgespielt
		- Pause wiederholt erstes Teil
		- Up Taste iste gesperrt
		- Down Taste ist gesperrt
		- Taste 4 ist gesperrt
		- Taste 5 ist gesperrt
		- Modifier & Playback Shortcuts sind gesperrt


