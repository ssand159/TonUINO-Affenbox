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
	a. Lautstärker wird erhöht; ok
	b. Lautstärke wird verringert; ok
	c. Up & Down verändern die Lautstärke nicht, wenn Rotary Encoder eingebundenist; ok

6. RFID
	a. konfigurierte Karte wird richtig erkannt; ok
	b. neue Karte mit ungültigem Cookie wird erkannt und Konfigurationsmenü wird gestartet; ok
	c. Konfigurationsmenü funktionsweise; ok

## Abspielmodis


1. Hörspielmodus
	a. Tag konfiguerieren; ok
	b. Tag lesen; ok
	c. korrektes abspielen; ok

2. Album
	a. Tag konfiguerieren; ok
	b. Tag lesen; ok
	c. korrektes abspielen; ok

3. Party
	a. Tag konfiguerieren; ok
	b. Tag lesen; ok
	c. korrektes abspielen; ok

4. Single
	a. Tag konfiguerieren; ok
	b. Tag lesen; ok
	c. korrektes abspielen; ok

5. Hörbuch
	a. Tag konfiguerieren; ok
	b. Tag lesen; ok
	c. korrektes abspielen; ok
	d. letzter Track wurde gespeichert und gelesen; ok

6. Hörspielmodus von bis
	a. Tag konfiguerieren; ok
	b. Tag lesen; ok
	c. korrektes abspielen; ok
	d. Titel vor und zurück sind gesperrt; ok 

7. Album von bis
	a. Tag konfiguerieren; ok
	b. Tag lesen; ok
	c. korrektes abspielen; ok
	d. Grenzen werden eingehalten; ok

8. Party von bis
	a. Tag konfiguerieren; ok
	b. Tag lesen; ok
	c. korrektes abspielen; ok
	d. Grenzen werden eingehalten; ok

9. Hörbuch von bis
	a. Tag konfiguerieren; ok
	b. Tag lesen; ok
	c. korrektes abspielen; ok
	d. Grenzen werden eingehalten; ok
	d. letzter Track wurde gespeichert und gelesen; ok

10. Puzzleteil
	a. Tag konfiguerieren; ok
	b. Tag lesen; ok
	c. korrektes abspielen; ok


## Modifier


1. Sleep Timer
	a. Tag konfiguerieren
	b. Tag lesen
	c. Stop nach eingestellter Zeit
		- 5min
		- 15min	
		- 30min
		- 60min

2. Stopp Tanz
	a. Tag konfiguerieren
	b. Tag lesen
	c. Stopp nach angegebener Zeit

3. Sperre
	a. Tag konfiguerieren
	b. Tag lesen
	c. Funktionen gesperrt
		- Pause Taste
		- Up Taste
		- Down Taste
		- RFID leser
		- Taste 4
		- Taste 5
		- Rotary Encoder
		- Drehschalter

4. Kleinkind Modus
	a. Tag konfiguerieren
	b. Tag lesen
	c. Funktionen gesperrt
		- Pause Taste
		- Up Taste
		- Down Taste
		- Taste 4
		- Taste 5
		- Rotary Encoder
		- Drehschalter

5. Wiederholen
	a. Tag konfiguerieren
	b. Tag lesen
	c. Titel wird wiederholt

6. Puzzlespiel
	a. Tag konfiguerieren
	b. Tag lesen
	c. Spielablauf
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


