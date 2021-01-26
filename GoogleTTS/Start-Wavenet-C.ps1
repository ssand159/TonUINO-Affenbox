# Benutzer Konfiguration
$GoogleAPIToken ="AIzaSyBm7NinfOuybtq-WWkcFMoiHHxMz60qxj0"
$GoogleTTSspeakingRate="1.2" # 0.25 - 4.00
$GoogleTTSpitch="-2" # -20.00 - 20.00

# Unterdrücke Fehlermeldungen
$ErrorActionpreference = "silentlycontinue"

# Abfrage
Write-Host "TTS-Dateien erstellen? (Ja / Nein)"
Write-Host ""
Write-Host -nonewline "Eingabe: "
$response = read-host
if ( $response -ne "Ja" ) { exit }
clear

# Textdatei
$MySpeech = Import-Csv .\soundfiles.txt -Delimiter "|" -Header file, text -Encoding UTF8

# Zielverzeichnis
$Basepath = "C:\TonUINO-TTS"

# Konfiguriere Google Text2Speech Engine
$GenerateGoogleTTS = $true
$GenerateGoogleTTSVoice="de-DE-Wavenet-C"

# Initialisiere Speech Engines
if ($GenerateGoogleTTS) {
    $GoogleTTSPath = $Basepath + "\"
    if (!(test-path($GoogleTTSPath))) { New-Item -Path $GoogleTTSPath -ItemType Directory }
}

foreach ($ToSpeak in $MySpeech) {

	write-host ""
    write-host $ToSpeak.text

    if ($GenerateGoogleTTS) {
        foreach ($voice in $GenerateGoogleTTSVoice) {
            $mytmppath=$GoogleTTSPath + "\" + $voice
            if (!(Test-Path($mytmppath))){new-item $mytmppath -ItemType Directory}
            $mytmppath += "\" + $ToSpeak.file
            if (!(Test-Path($mytmppath))) {
                $tmp=@{input=@{text="$($ToSpeak.text)"};voice=@{languageCode="de-DE";name="$($voice)"};audioConfig=@{audioEncoding="MP3";speakingRate="$GoogleTTSspeakingRate";pitch="$GoogleTTSPitch";'sampleRateHertz'="44100"}}
                $tmp = $tmp | ConvertTo-Json
                $Myrequest=Invoke-WebRequest -Uri "https://texttospeech.googleapis.com/v1beta1/text:synthesize?key=$($GoogleAPIToken)" -ContentType "application/json; charset=utf-8" -Method Post -Body $tmp
                if ($Myrequest) {
                    $tmp=($Myrequest.content | ConvertFrom-Json).audiocontent
                    $tmp = [Convert]::FromBase64String($tmp)
                    [IO.File]::WriteAllBytes($mytmppath, $tmp)
					
                    write-host " - GoogleTTS - $Voice"
					
                } else {
                    write-host " - API-Fehler: GoogleTTS - $Voice"
                }
				
			} else {
				write-host " - Bereits erstellt: GoogleTTS - $Voice"
			}
			
        }
		
    }
	
}

# Fertig
write-host ""
write-host ""
write-host -nonewline "Druecken Sie eine beliebige Taste, um fortzufahren..."
[void][System.Console]::ReadKey($true)
explorer.exe C:\TonUINO-TTS\GoogleTTS\de-DE-Wavenet-C
