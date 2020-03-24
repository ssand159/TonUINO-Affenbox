# TonUINO-GoogleTTS

Ein Powershell-Skript zur automatischen Generierung von Text-to-Speech-Mediendateien.


# Ersteller:
Einen herzlichen Dank geht an [ceear](https://github.com/ceear), der dieses Skript ursprünglich geschrieben hat.:sunglasses:


# Hinweis:
Dieses Powershell-Skript ist ausschließlich für Windows Benutzer geeignet.

Empfohlen wird Windows 10.


# Vorwort:
Für Google TTS wird ein entsprechender API-Key benötigt.
Dieser Skript kommt mit einem API-Key und es wird kein eigener benötigt.

Wer dennoch seinen eigenen API-Key verwenden möchte, der kann dies in der Zeile 2 ersetzen.
Hier eine Anleitung wie ein API-Key erstellt wird:
https://discourse.voss.earth/t/professionelle-stimme-gesucht/19/35

Die Sprechgeschwindigkeit kann unter Zeile 3 eingestellt werden.
Der Bereich liegt zwischen 0.25 bis 4.00. Der Standard liegt bei 1.00.

Die Stimmhöhe kann unter Zeile 4 eingestellt werden.
Der Bereich liegt zwischen -20.00 bis 20.00. Der Standard liegt bei 0.

Das Skript erwartet die Datei "soundfiles.txt" als Eingabedatei für die zu erzeugenden Texte. 
Diese liegt bei und kann nach Belieben umgeändert oder ersetzt werden.

Die erstellten Dateien findet ihr unter "C:\TonUINO-TTS\GoogleTTS".

Hier könnt ihr die verschiedenen Stimmen Probehören und eure Powershell-Datei am Ende auswählen:
https://cloud.google.com/text-to-speech/


# Anleitung:
1. Entsprechende Änderungen in der soundfiles.txt machen. (Optional)

2. Start-*.ps1 starten. (Rechtsklick auf die Datei und „Mit PowerShell ausführen“ anwählen.)

3. Mit "Ja" bestätigen.

4. Warten bis alles fertig ist.

5. Mit Enter am Ende bestätigen.

6. Der Ziel-Ordner mit den TTS-Dateien öffnet sich automatisch.
