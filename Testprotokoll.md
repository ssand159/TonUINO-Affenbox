# Testprotokol (WIP!!)


In diesem Protokoll möchte ich festhalten, welche Funktionen getestet wurden, ob es Auffälligkeiten gab und ob diese behoben wurden.
Jeder Punkt, der mit "ok" gekennzeichnet ist wurde validiert.


## Auffälligkeiten


- Optionsnummer wird nicht wieder gegeben; ok
- Falsche Ansage bei schnellem Optionswechsel; ok


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


## Modifier


1. Sleep Timer
	- Tag konfiguerieren; ok
	- Tag lesen; ok
	- Stop nach eingestellter Zeit; ok

2. Stopp Tanz
	- Tag konfiguerieren
	- Tag lesen
	- Stopp nach angegebener Zeit

3. Sperre
	- Tag konfiguerieren
	- Tag lesen
	- Funktionen gesperrt
		- Pause Taste
		- Up Taste
		- Down Taste
		- RFID leser
		- Taste 4
		- Taste 5
		- Rotary Encoder
		- Drehschalter

4. Kleinkind Modus
	- Tag konfiguerieren
	- Tag lesen
	- Funktionen gesperrt
		- Pause Taste
		- Up Taste
		- Down Taste
		- Taste 4
		- Taste 5
		- Rotary Encoder
		- Drehschalter

5. Wiederholen
	- Tag konfiguerieren
	- Tag lesen
	- Titel wird wiederholt

6. Puzzlespiel
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


