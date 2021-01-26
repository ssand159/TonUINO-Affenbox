/* ToDo:
   Fehlerbehandlung Speichern auf Karte
   Shortcut: Modifier entfernen falls einer aktiv
   Shortcut: Speicher für Hörbuch/Album?
   Modifier: Konfiguration falsch
   Modifier: Caluclate Spiel fehlerhaft
   IR: Anlernen falsche Sprachausgaben
   IR: Anlernen fehlerhaft, alle Buttons = pause
   voiceMenu: Abbruchausgabe wird nicht gespielt
   AdminMenu abbruch Menu = shutdown
*/
#include "Configuration.h"
#include <DFMiniMp3.h>
#include <EEPROM.h>
#include <JC_Button.h>
#include <MFRC522.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#if not defined AiO
#include <avr/sleep.h>
#endif
#if defined ROTARY_ENCODER
#include <TimerOne.h>
#include <ClickEncoder.h>
#endif
#if defined IRREMOTE
#include <IRremote.h>
#endif
#if defined AiO
#include "Emulated_EEPROM.h"
#endif

/*
   _____         _____ _____ _____ _____
  |_   _|___ ___|  |  |     |   | |     |
    | | | . |   |  |  |-   -| | | |  |  |
    |_| |___|_|_|_____|_____|_|___|_____|
    TonUINO Version 2.2
    created by Thorsten Voß and licensed under GNU/GPL.
    Information and contribution at https://tonuino.de.
    Fork by Marco Schulz DEVELOPER
*/
///////// General globals ////////////////////////////////////////////////
unsigned long sleepAtMillis = 0;

///////// MFRC522 ////////////////////////////////////////////////////////
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522
MFRC522::MIFARE_Key key;
bool successRead;
uint8_t sector = 1;
uint8_t blockAddr = 4;
uint8_t trailerBlock = 7;
MFRC522::StatusCode status;
//////////////////////////////////////////////////////////////////////////

///////// card cookie ////////////////////////////////////////////////////
static const uint32_t cardCookie = 322417479;
//////////////////////////////////////////////////////////////////////////

static const uint16_t EEPROM_size = 512;
static const uint16_t EEPROM_settingsStartAdress = 1;

static const uint8_t availableShortCuts = 12; //Die Anzahl Inputs einer 3x4 Matrix oder eines 0-9,#,* Tastenfeldes
///////// setup buttons //////////////////////////////////////////////////
Button pauseButton(buttonPause);
Button upButton(buttonUp);
Button downButton(buttonDown);
#if defined FIVEBUTTONS
Button buttonFour(buttonFourPin);
Button buttonFive(buttonFivePin);
#endif
//////////////////////////////////////////////////////////////////////////

typedef enum Enum_Trigger {
  NoTrigger = 0,
  PauseTrackTrigger,
  NextTrigger,
  NextPlusTenTrigger,
  PreviousTrigger,
  PreviousPlusTenTrigger,
  VolumeUpTrigger,
  VolumeDownTrigger,
  ShutDownTrigger,
  AdminMenuTrigger,
  ResetTrackTrigger,
  ShortcutTrigger //muss an letzter Stelle stehen bleiben!
};

struct inputTrigger {
  bool noTrigger;
  bool pauseTrack;
  bool next;
  bool nextPlusTen;
  bool previous;
  bool previousPlusTen;
  bool volumeUp;
  bool volumeDown;
  bool shutDown;
  bool adminMenu;
  bool resetTrack;
  bool shortCutNo [availableShortCuts]; //muss an letzter Stelle stehen bleiben!
};

static inputTrigger myTrigger;
static inputTrigger myTriggerEnable;

static const uint8_t inputTriggerCount = sizeof(myTrigger);

#if defined AiO
const uint8_t folderMemoryCount = 3;
#else
const uint8_t folderMemoryCount = 8;
#endif

//////// IR Remote ////////////////////////////////////////////////////
#if defined IRREMOTE
const uint8_t irRemoteCodeCount = inputTriggerCount;
IRrecv irReceiver(IRREMOTE_PIN); // create IRrecv instance
decode_results irReading;
#endif
//////////////////////////////////////////////////////////////////////////

///////// DFPlayer Mini //////////////////////////////////////////////////
SoftwareSerial mySoftwareSerial(2, 3); // RX, TX
uint8_t numTracksInFolder;
uint8_t currentTrack;
uint8_t firstTrack;
uint8_t queue[255];
uint8_t volume;
static uint8_t _lastTrackFinished;
//////////////////////////////////////////////////////////////////////////

//////// rotary encoder /////////////////////////////////////////////////
#if defined ROTARY_ENCODER
int8_t RotEncOldEncPos = -1 ;
int8_t RotEncPos = 15;
ClickEncoder encoder(ROTARY_ENCODER_PIN_A, ROTARY_ENCODER_PIN_B, ROTARY_ENCODER_STEPS);
#endif
//////////////////////////////////////////////////////////////////////////

//////// analog input /////////////////////////////////////////////////
#if defined ANALOG_INPUT
int8_t analogInLastPosition = -1;
unsigned long analogInMillis = 0;
#endif
typedef enum analogTriggerType
{
  NoType = 0,
  Impuls,
  Permament
};
//////////////////////////////////////////////////////////////////////////

///////// Enums //////////////////////////////////////////////////////////
typedef enum Enum_PlayMode
{
  AudioDrama = 1,
  Album = 2,
  Party = 3,
  Single = 4,
  AudioBook = 5,
  AdminMenu = 6,
  AudioDrama_Section = 7,
  Album_Section = 8,
  Party_Section = 9,
  AudioBook_Section = 10,
  PuzzlePart = 11
};
typedef enum Enum_Modifier
{
  ModifierMode = 0,
  SleepTimerMod = 1,
  FreezeDanceMod = 2,
  LockedMod = 3,
  ToddlerModeMod = 4,
  KindergardenModeMod = 5,
  RepeatSingleMod = 6,
  PuzzleGameMod = 7,
  QuizGameMod = 8,
  ButtonSmashMod = 9,
  //TheQuestMod,
  CalculateMod = 10,
  AdminMenuMod = 255
};

typedef enum Enum_AdminMenuOptions
{
  Exit = 0,
  ResetCard,
  MaxVolume,
  MinVolume,
  InitVolume,
  EQ,
  SetupShortCuts,
  SetupStandbyTimer,
  CreateFolderCards,
  InvertButtons,
  StopWhenCardAway,
  SetupAnalogInputLeraning,
  SetupAnalogInputType,
  SetupIRRemote,
  ResetEEPROM,
  LockAdminMenu
};
typedef enum Enum_PCS
{
  PCS_NO_CHANGE     = 0, // no change detected since last pollCard() call
  PCS_NEW_CARD      = 1, // card with new UID detected (had no card or other card before)
  PCS_CARD_GONE     = 2, // card is not reachable anymore
  PCS_CARD_IS_BACK  = 3 // card was gone, and is now back again
};

typedef enum Enum_NfcGain
{
  minimum = 0,
  average = 1,
  maximum = 2
};
//////////////////////////////////////////////////////////////////////////

///////// this object stores nfc tag data ///////////////////////////////
struct folderSettings {
  uint8_t folder;
  uint8_t mode;
  uint8_t special;
  uint8_t special2;
  uint8_t special3;
  uint8_t special4;
};
folderSettings *myFolder;

struct nfcTagObject {
  uint32_t cookie;
  uint8_t version;
  folderSettings nfcFolderSettings;
};
nfcTagObject myCard;
//////////////////////////////////////////////////////////////////////////

///////// admin settings stored in eeprom ///////////////////////////////
struct adminSettings {
  uint32_t cookie;
  byte version;
  uint8_t maxVolume;
  uint8_t minVolume;
  uint8_t initVolume;
  uint8_t eq;
  bool locked;
  unsigned long standbyTimer;
  bool invertVolumeButtons;
  folderSettings shortCuts[availableShortCuts];
  uint8_t adminMenuLocked;
  folderSettings savedModifier;
  bool stopWhenCardAway;
  Enum_NfcGain nfcGain;
  uint8_t userAge;
  uint16_t irRemoteUserCodes[inputTriggerCount];//Ein Slot pro möglichem Trigger
  uint16_t analogInUserValues[inputTriggerCount]; //Ein Slot pro möglichem Trigger
  uint8_t analogInputTolerance;
  uint16_t analogInputTriggerTime;
  uint8_t analogInputTriggerType;
};
adminSettings mySettings;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
static bool hasCard = false;
static byte lastCardUid[4];
static byte retries;
static bool lastCardWasUL;
static bool forgetLastCard = false;
static bool knownCard = false;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void shuffleQueue(uint8_t startTrack = firstTrack, uint8_t endTrack = numTracksInFolder);
void writeSettingsToFlash();
void printSettings();
void resetSettings();
void playFolder();
void activateShortCut (uint8_t shortCutNo);
static void nextTrack(uint8_t track);
static void previousTrack() ;
bool isPlaying();
void mp3Pause(uint16_t delayTime = 100);
void waitForTrackToFinish (bool interruptByTrigger = true, uint16_t timeOut = 5000);
void waitForTrackToStart(uint16_t timeOut = 5000);
void setup();
void readButtons();
#if defined IRREMOTE
void readIr ();
#endif
#if defined ANALOG_INPUT
void readAnalogIn();
#endif
#if defined ROTARY_ENCODER
void RotEncSetVolume ();
void timerIsr() ;
#endif
void checkNoTrigger ();
void resetTrigger();
void resetTriggerEnable();
void volumeUpAction();
void volumeDownAction();
void nextAction() ;
void previousAction();
void pauseAction();
void loop();
void adminMenu(bool fromCard = false);
uint8_t voiceMenu(int16_t numberOfOptions, uint16_t startMessage, int messageOffset,
                  bool preview = false, uint8_t previewFromFolder = 0, int defaultValue = 0,
                  bool enableSkipTen = false);
bool setupFolder(folderSettings * theFolder) ;
void setupShortCut(uint8_t shortCutNumber = 0);
void setupCard();
bool readCard(nfcTagObject * nfcTag);
void resetCard() ;
void writeCard(nfcTagObject nfcTag, int8_t writeBlock = -1, bool feedback = true) ;
void dump_byte_array(byte * buffer, byte bufferSize) ;
bool setupModifier(folderSettings * tmpFolderSettings);
bool RemoveModifier();
void writeCardMemory (uint8_t track) ;
byte pollCard();
void handleCardReader();
void onNewCard();
void setstandbyTimer();
void disablestandbyTimer();
void checkStandbyAtMillis();
void shutDown();
#if defined FADING_LED
void fadeStatusLed(bool isPlaying);
#endif
//////////////////////////////////////////////////////////////////////////

// implement a notification class,
// its member methods will get called
//
class Mp3Notify {
  public:

    static void OnError(uint16_t errorCode) {
#if defined DEBUG
      switch (errorCode) {
        case DfMp3_Error_Busy: {
            Serial.print(F("busy"));
            break;
          }
        case DfMp3_Error_Sleeping: {
            Serial.print(F("sleep"));
            break;
          }
        case DfMp3_Error_SerialWrongStack: {
            Serial.print(F("serial stack"));
            break;
          }
        case DfMp3_Error_CheckSumNotMatch: {
            Serial.print(F("checksum"));
            break;
          }
        case DfMp3_Error_FileIndexOut: {
            Serial.print(F("file index"));
            break;
          }
        case DfMp3_Error_FileMismatch: {
            Serial.print(F("file mismatch"));
            break;
          }
        case DfMp3_Error_Advertise: {
            Serial.print(F("advertise"));
            break;
          }
        case DfMp3_Error_RxTimeout: {
            Serial.print(F("rx timeout"));
            break;
          }
        case DfMp3_Error_PacketSize: {
            Serial.print(F("packet size"));
            break;
          }
        case DfMp3_Error_PacketHeader: {
            Serial.print(F("packet header"));
            break;
          }
        case DfMp3_Error_PacketChecksum: {
            Serial.print(F("packet checksum"));
            break;
          }
        case DfMp3_Error_General: {
            Serial.print(F("general"));
            break;
          }
        default: {
            Serial.print(F("unknown"));
            break;
          }
      }
      #endif
    }

    static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action) {
#if defined DEBUG
      if (source & DfMp3_PlaySources_Sd) Serial.print("SD ");
      if (source & DfMp3_PlaySources_Usb) Serial.print("USB ");
      if (source & DfMp3_PlaySources_Flash) Serial.print("Flash ");
      Serial.println(action);
#endif

    }

    static void OnPlaySourceOnline(DfMp3_PlaySources source) {
#if defined DEBUG
      PrintlnSourceAction(source, "online");
#endif

    }
    static void OnPlaySourceInserted(DfMp3_PlaySources source) {
#if defined DEBUG
      PrintlnSourceAction(source, "ready");
#endif
    }
    static void OnPlaySourceRemoved(DfMp3_PlaySources source) {
#if defined DEBUG
      PrintlnSourceAction(source, "removed");
#endif
    }
    static void OnPlayFinished(DfMp3_PlaySources source, uint8_t track) {
      nextTrack(track);
    }
};

static DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(mySoftwareSerial);
//////////////////////////////////////////////////////////////////////////
void shuffleQueue(uint8_t startTrack = firstTrack, uint8_t endTrack = numTracksInFolder) {
#ifdef DEBUG
  Serial.println(F("shuffle Queue"));
#endif
  // Queue für die Zufallswiedergabe erstellen
  for (uint8_t x = 0; x < endTrack - startTrack + 1; x++)
    queue[x] = x + startTrack;
  // Rest mit 0 auffüllen
  for (uint8_t x = endTrack - startTrack + 1; x < 255; x++)
    queue[x] = 0;
  // Queue mischen
  for (uint8_t i = 0; i < endTrack - startTrack + 1; i++)
  {
    uint8_t j = random (0, endTrack - startTrack + 1);
    uint8_t t = queue[i];
    queue[i] = queue[j];
    queue[j] = t;
  }
#ifdef QUEUE_PRINT
  Serial.println(F("Queue :"));
  for (uint8_t x = 0; x < endTrack - startTrack + 1 ; x++) {
    Serial.print(x + 1);
    Serial.print(". : ");
    Serial.println(queue[x]);
  }
#endif
}
//////////////////////////////////////////////////////////////////////////
void writeSettingsToFlash() {

#if defined DEBUG
  Serial.print(F("write to EEPROM"));
#endif
#if defined AiO
  EEPROM_put(EEPROM_settingsStartAdress, mySettings);
#else
  EEPROM.put(EEPROM_settingsStartAdress, mySettings);
#endif

}
//////////////////////////////////////////////////////////////////////////
void printSettings() {

#if defined AiO
  EEPROM_get(EEPROM_settingsStartAdress, mySettings);
#else
  EEPROM.get(EEPROM_settingsStartAdress, mySettings);
#endif

  if (mySettings.version != 3 || mySettings.cookie != cardCookie) {
    resetSettings();
  }
#if defined DEBUG

  Serial.print(F("Version "));
  Serial.println(mySettings.version);

  Serial.print(F("Max Vol "));
  Serial.println(mySettings.maxVolume);

  Serial.print(F("Min Vol "));
  Serial.println(mySettings.minVolume);

  Serial.print(F("Init Vol "));
  Serial.println(mySettings.initVolume);

  Serial.print(F("EQ "));
  Serial.println(mySettings.eq);

  Serial.print(F("Locked "));
  Serial.println(mySettings.locked);

  Serial.print(F("Sleep Timer "));
  Serial.println(mySettings.standbyTimer);

  Serial.print(F("Inverted Buttons "));
  Serial.println(mySettings.invertVolumeButtons);

  Serial.print(F("Stop when card away "));
  Serial.println(mySettings.stopWhenCardAway);

  Serial.print(F("Admin Menu locked "));
  Serial.println(mySettings.adminMenuLocked);

  Serial.print(F("User Age "));
  Serial.println(mySettings.userAge);

  Serial.print(F("NFC Gain "));
  switch (mySettings.nfcGain) {
    case 0:
      Serial.println(F("minimum"));
      break;
    case 1:
      Serial.println(F("average"));
      break;
    case 2:
      Serial.println(F("maximum"));
      break;
  }
  Serial.print(F("Saved Modifier "));
  Serial.println(mySettings.savedModifier.mode);
#endif
#if defined ANALOG_INPUT_PRINT
  Serial.print(F("Analog input tolerance "));
  Serial.println(mySettings.analogInputTolerance);

  Serial.print(F("Analog input trigger time "));
  Serial.println(mySettings.analogInputTriggerTime);

  for (uint8_t i = 0; i < inputTriggerCount; i++) {
    Serial.print(F("Analog input value "));
    Serial.print(i);
    Serial.print(F(": "));
    Serial.println(mySettings.analogInUserValues[i]);
  }
  Serial.print(F("Analog input trigger type "));
  Serial.println(mySettings.analogInputTriggerType);
#endif
}
//////////////////////////////////////////////////////////////////////////
void resetSettings() {
  Serial.println(F("reset settings"));
  mySettings.cookie = cardCookie;
  mySettings.version = 3;
  mySettings.maxVolume = 25;
  mySettings.minVolume = 1;
  mySettings.initVolume = 18;
  mySettings.eq = 1;
  mySettings.locked = false;
  mySettings.standbyTimer = 0;
#if defined AiO
  mySettings.invertVolumeButtons = false;
#else
  mySettings.invertVolumeButtons = true;
#endif
  mySettings.adminMenuLocked = 0;
  mySettings.savedModifier.folder = 0;
  mySettings.savedModifier.special = 0;
  mySettings.savedModifier.special2 = 0;
  mySettings.savedModifier.mode = 0;
  mySettings.stopWhenCardAway = false;
  mySettings.userAge = 0;
  mySettings.nfcGain = 1;
  for (uint8_t i = 0; i < availableShortCuts; i++) {
    mySettings.shortCuts[i].folder = 0;
    mySettings.shortCuts[i].mode = 0;
    mySettings.shortCuts[i].special = 0;
    mySettings.shortCuts[i].special2 = 0;
    mySettings.shortCuts[i].special3 = 0;
    mySettings.shortCuts[i].special4 = 0;
  }
  mySettings.analogInputTolerance = 2;
  mySettings.analogInputTriggerTime = 35;
  mySettings.analogInputTriggerType = NoType;
  writeSettingsToFlash();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class Modifier {
  public:
    virtual void loop() {}
    virtual bool handlePause() {
      return false;
    }
    virtual bool handleNext() {
      return false;
    }
    virtual bool handlePrevious() {
      return false;
    }
    virtual bool handlenextAction() {
      return false;
    }
    virtual bool handlepreviousAction() {
      return false;
    }
    virtual bool handleVolume() {
      return false;
    }
    virtual bool handleShortCut() {
      return false;
    }
    virtual bool handleShutDown() {
      return false;
    }
    virtual bool handleRFID(nfcTagObject *newCard) {
      return false;
    }
    virtual bool handleStopWhenCardAway() {
      return false;
    }
    virtual bool handleSaveModifier() {
      return false;
    }
    virtual bool handleAdminMenu() {
      return false;
    }
    virtual uint8_t getActive() {
      return 0;
    }
    Modifier() {

    }
};

Modifier *activeModifier = NULL;
/*///////////////////////////////////////////////////////////////////////////
  //// An modifier can also do somethings in addition to the modified action
  //// by returning false (not handled) at the end
  //// This simple FeedbackModifier will tell the volume before changing it and
  //// give some feedback once a RFID card is detected.
  //class FeedbackModifier: public Modifier {
  //  public:
  //    virtual bool handleRFID(nfcTagObject *newCard) {
  //#if defined DEBUG
  //      Serial.println(F("Feedback > RFID"));
  //#endif
  //      return false;
  //    }
  //    uint8_t getActive() {
  //      return FeedbackMod;
  //    }
  //};
  /////////////////////////////////////////////////////////////////////////*/
//////////////////////////////////////////////////////////////////////////
class SleepTimer: public Modifier {
  private:
    unsigned long sleepAtMillis = 0;

  public:
    void loop() {
      if (this->sleepAtMillis != 0 && millis() > this->sleepAtMillis) {
#if defined DEBUG
        Serial.println(F("SleepTimer > sleep"));
#endif
        mp3Pause();
        setstandbyTimer();
        activeModifier = NULL;
        delete this;
      }
    }

    SleepTimer(uint8_t minutes) {
#if defined DEBUG
      Serial.print(F("SleepTimer minutes: "));
      Serial.println(minutes);
#endif
      this->sleepAtMillis = millis() + minutes * 60000;
    }
    uint8_t getActive() {
      return SleepTimerMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class FreezeDance: public Modifier {
  private:
    unsigned long nextStopAtMillis = 0;
    const uint8_t minSecondsBetweenStops = 5;
    const uint8_t maxSecondsBetweenStops = 30;

    void setNextStopAtMillis() {
      uint8_t seconds = random(this->minSecondsBetweenStops, this->maxSecondsBetweenStops + 1);
#if defined DEBUG
      Serial.print(F("FreezeDance set next stop at "));
      Serial.print(seconds);
      Serial.println(F(" s."));
#endif
      this->nextStopAtMillis = millis() + seconds * 1000;
    }

  public:
    void loop() {
      if (this->nextStopAtMillis != 0 && millis() > this->nextStopAtMillis) {
#if defined DEBUG
        Serial.println(F("FreezeDance > freeze"));
#endif
        if (isPlaying()) {
          mp3.playAdvertisement(301);
          delay(500);
        }
        setNextStopAtMillis();
      }
    }
    FreezeDance(void) {
#if defined DEBUG
      Serial.println(F("FreezeDance"));
#endif
      if (isPlaying()) {
        delay(1000);
        mp3.playAdvertisement(300);
        delay(500);
      }
      setNextStopAtMillis();
    }
    uint8_t getActive() {
      return FreezeDanceMod ;
    }
};
//////////////////////////////////////////////////////////////////////////
class PuzzleGame: public Modifier {
  private:
    uint8_t PartOneSpecial = 0;
    uint8_t PartOneFolder = 0;
    uint8_t PartOneSpecial2 = 0;

    uint8_t PartTwoSpecial = 0;
    uint8_t PartTwoFolder = 0;
    uint8_t PartTwoSpecial2 = 0;

    uint8_t mode = 0;

    bool PartOneSaved = false;
    bool PartTwoSaved = false;

    void Success ()
    {
      PartOneSaved = false;
      PartTwoSaved = false;
#if defined DEBUG
      Serial.println(F("PuzzleGame > success"));
#endif

      mp3.playMp3FolderTrack(297); //Toll gemacht! Das ist richtig.
      waitForTrackToFinish();
    }

    void Failure ()
    {
      if (mode == 1) {
        PartOneSaved = false;
      }
      PartTwoSaved = false;

#if defined DEBUG
      Serial.println(F("PuzzleGame > Failure"));
#endif

      mp3.playMp3FolderTrack(296);
      waitForTrackToFinish();
    }

    void CompareParts()
    {
      if ((PartOneSpecial == PartTwoSpecial) && (PartOneFolder == PartTwoFolder) && (PartOneSpecial2 == PartTwoSpecial2)) {
        PartTwoSaved = false;
        return;
      }
      else if (((PartOneSpecial !=  PartTwoSpecial) || (PartOneFolder != PartTwoFolder)) && (PartOneSpecial2 == PartTwoSpecial2)) {
        Success();
      }
      else {
        Failure();
      }
    }

  public:
    void loop()
    {
      if (PartOneSaved && PartTwoSaved) {
        if (!isPlaying()) {
          CompareParts();
        }
      }
      if (myTrigger.resetTrack) {
#if defined DEBUG
        Serial.println(F("PuzzleGame > delete part"));
#endif
        mp3Pause();
        mp3.playMp3FolderTrack(298); //Okay, lass uns was anderes probieren.
        PartOneSaved = false;
        PartTwoSaved = false;
      }
    }

    PuzzleGame(uint8_t special)
    {
      mp3.loop();
      mp3Pause();
#if defined DEBUG
      Serial.println(F("PuzzleGame"));
#endif

      mode = special;
    }

    virtual bool handlePause() {
      return false;
    }

    virtual bool handleNext() {
      return true;
    }

    virtual bool handlePrevious() {
      return true;
    }

    virtual bool handlenextAction()       {
      return true;
    }

    virtual bool handlepreviousAction() {
      return true;
    }

    virtual bool handleShutDown() {
      return true;
    }

    virtual bool handleShortCut() {
      return true;
    }

    virtual bool handleRFID(nfcTagObject *newCard) {

      //this->tmpCard = *newCard;
      if (newCard->nfcFolderSettings.mode != PuzzlePart) {
#if defined DEBUG
        Serial.println(F("PuzzleGame > no valid part"));
#endif
        mp3Pause();
        mp3.playMp3FolderTrack(299); //Diese Karte ist nicht Teil des Spiels. Probiere eine Andere.
        return true;
      }
      else {
#if defined DEBUG
        Serial.println(F("PuzzleGame > valid part"));
#endif
        if (!PartOneSaved)
        {
          PartOneSpecial = newCard->nfcFolderSettings.special;
          PartOneFolder = newCard->nfcFolderSettings.folder;
          PartOneSpecial2 = newCard->nfcFolderSettings.special2;
          PartOneSaved = true;
        }
        else if (PartOneSaved && !PartTwoSaved) {
          PartTwoSpecial = newCard->nfcFolderSettings.special;
          PartTwoFolder = newCard->nfcFolderSettings.folder;
          PartTwoSpecial2 = newCard->nfcFolderSettings.special2;
          PartTwoSaved = true;
        }
        return false;
      }
    }
    virtual bool handleStopWhenCardAway() {
      return true;
    }
    uint8_t getActive() {
      return PuzzleGameMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class QuizGame: public Modifier {
  private:
    uint8_t PartOneSpecial = 0;
    uint8_t PartOneFolder = 0;
    uint8_t PartOneSpecial2 = 0;

    uint8_t PartTwoSpecial = 0;
    uint8_t PartTwoFolder = 0;
    uint8_t PartTwoSpecial2 = 0;

    bool PartOneSaved = false;
    bool PartTwoSaved = false;

    void Success () {
      PartOneSaved = false;
      PartTwoSaved = false;
#if defined DEBUG
      Serial.println(F("QuizGame > success"));
#endif
      mp3.playMp3FolderTrack(297);
      waitForTrackToFinish();
      next();
    }

    void Failure () {
      PartTwoSaved = false;
#if defined DEBUG
      Serial.println(F("QuizGame > failure"));
#endif
      mp3.playMp3FolderTrack(296);
      waitForTrackToFinish();
    }

    void CompareParts() {
      if ((PartOneSpecial == PartTwoSpecial) && (PartOneFolder == PartTwoFolder) && (PartOneSpecial2 == PartTwoSpecial2)) {
        PartTwoSaved = false;
        return;
      }
      else if (((PartOneSpecial !=  PartTwoSpecial) || (PartOneFolder != PartTwoFolder)) && (PartOneSpecial2 == PartTwoSpecial2)) {
        Success();
      }
      else {
        Failure();
      }
    }

  public:
    void loop() {
      if (PartOneSaved && PartTwoSaved) {
        if (!isPlaying()) {
          CompareParts();
        }
      }

      if (myTrigger.resetTrack) {
#if defined DEBUG
        Serial.println(F("QuizGame > delete part"));
#endif
        mp3Pause();
        mp3.playMp3FolderTrack(298); //Okay, lass uns was anderes probieren.
        waitForTrackToFinish();
        PartOneSaved = false;
        PartTwoSaved = false;
        next();
      }
    }

    QuizGame(uint8_t special) {
      mp3.loop();
#if defined DEBUG
      Serial.println(F("QuizGame"));
#endif
      mp3Pause();

      PartOneFolder = special;
      numTracksInFolder = mp3.getFolderTrackCount(PartOneFolder);
      firstTrack = 1;
      shuffleQueue();
      currentTrack = 0;

#if defined DEBUG
      Serial.println(F("QuizGame > queue set"));
#endif
      next();
    }

    void  next() {
      numTracksInFolder = mp3.getFolderTrackCount(PartOneFolder);

      if (currentTrack != numTracksInFolder - firstTrack + 1) {
#if defined DEBUG
        Serial.println(F("QuizGame > queue next"));
#endif
        currentTrack++;
      } else {
#if defined DEBUG
        Serial.println(F("QuizGame > queue repeat"));
#endif
        currentTrack = 1;
      }
      PartOneSaved = true;
      PartOneSpecial = queue[currentTrack - 1];
      PartOneSpecial2 = PartOneSpecial;
#if defined DEBUG
      Serial.println(currentTrack);
      Serial.println(PartOneSpecial);
#endif
      mp3.playFolderTrack(PartOneFolder, PartOneSpecial);
    }

    virtual bool handleNext() {
      return true;
    }

    virtual bool handlePrevious() {
      return true;
    }
    virtual bool handlePause() {
#if defined DEBUG
      Serial.println(F("QuizGame > pause and repeat "));
#endif
      mp3Pause();
      mp3.playFolderTrack(PartOneFolder, PartOneSpecial);
      return true;
    }

    virtual bool handlenextAction()       {
      return true;
    }

    virtual bool handlepreviousAction() {
      return true;
    }

    virtual bool handleShutDown() {
      return true;
    }

    virtual bool handleShortCut() {
      return true;
    }

    virtual bool handleRFID(nfcTagObject * newCard) {

      if (newCard->nfcFolderSettings.mode != PuzzlePart) {
#if defined DEBUG
        Serial.println(F("QuizGame > no valid part"));
#endif
        mp3Pause();
        mp3.playMp3FolderTrack(299); //Diese Karte ist nicht Teil des Spiels. Probiere eine Andere.
        return true;
      }
      else {
#if defined DEBUG
        Serial.println(F("QuizGame > valid part"));
#endif
        if (PartOneSaved && !PartTwoSaved) {
          PartTwoSpecial = newCard->nfcFolderSettings.special;
          PartTwoFolder = newCard->nfcFolderSettings.folder;
          PartTwoSpecial2 = newCard->nfcFolderSettings.special2;
          PartTwoSaved = true;
        }
        return false;
      }
    }
    virtual bool handleStopWhenCardAway() {
      return true;
    }
    uint8_t getActive() {
      return QuizGameMod;
    }
};
//////////////////////////////////////////////////////////////////////////
///////////WIP!!//////////////////////////////////////////////////////////
/*class TheQuest: public Modifier {
  private:
    uint8_t PartOneSpecial = 0;
    uint8_t PartOneFolder = 0;
    uint8_t PartOneSpecial2 = 0;

    bool PartOneSaved = false;

    unsigned long NextAnnouncementAtMillis = 0;
    uint8_t TimeOut = 1;
    uint8_t AnnouncementTrack = 0;
    uint8_t AnnouncementFolder = 0;

    void setNextAnnouncementAtMillis() {
  #if defined DEBUG
      Serial.print(F("TheQuest > next announcement in "));
      Serial.print(this->TimeOut);
      Serial.println(F("min"));
  #endif
      this->NextAnnouncementAtMillis = millis() + (this->TimeOut * 60000);
  #if defined DEBUG
      Serial.print(this->NextAnnouncementAtMillis);
      Serial.println(F("ms"));
  #endif
    }

    void Success () {
      this->PartOneSaved = false;
  #if defined DEBUG
      Serial.println(F("TheQuest > success"));
  #endif
      mp3.playMp3FolderTrack(297);
      waitForTrackToFinish();
      next();
    }
  public:
    void loop() {
      if (this->NextAnnouncementAtMillis != 0 && millis() > this->NextAnnouncementAtMillis) {
  #if defined DEBUG
        Serial.print(millis());
        Serial.println(F("ms"));
        Serial.print(this->NextAnnouncementAtMillis);
        Serial.println(F("ms"));
  #endif
        this->AnnouncementTrack++;
        if (this->AnnouncementTrack <= 3) {
  #if defined DEBUG
          Serial.println(F("TheQuest > Time out, play announcement"));
  #endif
          setNextAnnouncementAtMillis();
  mp3Pause();
          mp3.playFolderTrack(this->AnnouncementFolder, this->AnnouncementTrack);
          waitForTrackToFinish();
        } else {
          this->AnnouncementTrack = 0;
          next();
        }
      }

      // Abbruch der Aktuellen Questaufgabe
      if (upButton.pressedFor(LONG_PRESS) && downButton.pressedFor(LONG_PRESS)) {
        do {
          readTrigger();
        } while (myTrigger.resetTrack);
  #if defined DEBUG
        Serial.println(F("TheQuest > delete part"));
  #endif
  mp3Pause();
        mp3.playMp3FolderTrack(298); //Okay, lass uns was anderes probieren.
        waitForTrackToFinish();
        this->PartOneSaved = false;
        next();
      }
      //Steuerung über Pause Button
      //2s Druck: Wiederholen der Questaufgabe
      if (pauseButton.pressedFor(LONGER_PRESS)) {
        if (pauseButton.wasReleased()) {
  #if defined DEBUG
          Serial.println(F("QuizGame > pause and repeat "));
  #endif
  mp3Pause();
          mp3.playFolderTrack(PartOneFolder, PartOneSpecial);
        }
        //Druck <1s: Questaufgabe erfüllt
      } else if (pauseButton.wasReleased()) {
        Success();
      }
    }

    TheQuest(uint8_t special, uint8_t special2) {
      mp3.loop();
  #if defined DEBUG
      Serial.println(F("TheQuest"));
  #endif
  mp3Pause();

      this->AnnouncementFolder = special2;
      this->PartOneFolder = special;
      numTracksInFolder = mp3.getFolderTrackCount(this->PartOneFolder);
      firstTrack = 1;
      shuffleQueue();
      currentTrack = 0;

  #if defined DEBUG
      Serial.println(F("TheQuest > queue set"));
  #endif
      next();
    }

    void  next() {
      numTracksInFolder = mp3.getFolderTrackCount(this->PartOneFolder);

      if (currentTrack != numTracksInFolder - firstTrack + 1) {
  #if defined DEBUG
        Serial.println(F("TheQuest > queue next"));
  #endif
        currentTrack++;
      } else {
  #if defined DEBUG
        Serial.println(F("TheQuest > queue repeat"));
  #endif
        currentTrack = 1;
      }
      this->PartOneSaved = true;
      this->PartOneSpecial = queue[currentTrack - 1];
      this->PartOneSpecial2 = PartOneSpecial;
  #if defined DEBUG
      Serial.println(currentTrack);
      Serial.println(this->PartOneSpecial);
  #endif
      mp3.playFolderTrack(this->PartOneFolder, this->PartOneSpecial);

      setNextAnnouncementAtMillis();
    }

    virtual bool handleNext() {
      return true;
    }

    virtual bool handlePrevious() {
      return true;
    }

    virtual bool handlePause() {
      return true;
    }

    virtual bool handlenextAction() {
      if (this->TimeOut < 10) {
        this->TimeOut++;
        setNextAnnouncementAtMillis();
      }
      return true;
    }

    virtual bool handlepreviousAction() {
      if (this->TimeOut > 1) {
        this->TimeOut--;
        setNextAnnouncementAtMillis();
      }
      return true;
    }

    virtual bool handleShutDown() {
      return true;
    }

    virtual bool handleShortCut() {
      return true;
    }

    virtual bool handleRFID(nfcTagObject * newCard) {
      return true;
    }
    virtual bool handleStopWhenCardAway() {
      return true;
    }
    uint8_t getActive() {
      return TheQuestMod;
    }
  };*/
//////////////////////////////////////////////////////////////////////////
class ButtonSmash: public Modifier {
  private:
    uint8_t Folder;

    nfcTagObject tmpCard;

  public:
    void loop() {
    }

    ButtonSmash(uint8_t special, uint8_t special2) {
      mp3.loop();
#if defined DEBUG
      Serial.println(F("ButtonSmash"));
#endif
      mp3Pause();

      mp3.setVolume(special2);
      delay(100);

      this->Folder = special;
      numTracksInFolder = mp3.getFolderTrackCount(Folder);

      firstTrack = 1;
      shuffleQueue();
      currentTrack = 0;
    }

    void  next() {
      mp3Pause();
      numTracksInFolder = mp3.getFolderTrackCount(this->Folder);

      if (currentTrack != numTracksInFolder - firstTrack + 1) {
#if defined DEBUG
        Serial.println(F("ButtonSmash > queue next"));
#endif
        currentTrack++;
      } else {
#if defined DEBUG
        Serial.println(F("ButtonSmash > repeat queue"));
#endif
        firstTrack = 1;
        shuffleQueue();
        currentTrack = 1;
      }
      mp3.playFolderTrack(this->Folder, queue[currentTrack - 1]);
      waitForTrackToFinish();
    }

    virtual bool handleNext() {
      return true;
    }

    virtual bool handlePrevious() {
      return true;
    }

    virtual bool handlePause() {
      next();
      return true;
    }

    virtual bool handlenextAction() {
      next();
      return true;
    }

    virtual bool handlepreviousAction() {
      next();
      return true;
    }

    virtual bool handleVolume() {
      next();
      return true;
    }

    virtual bool handleShortCut() {
      next();
      return true;
    }

    virtual bool handleShutDown() {
      next();
      return true;
    }

    virtual bool handleAdminMenu() {
      next();
      return true;
    }
    virtual bool handleRFID(nfcTagObject * newCard) {
      next();
      return true;
    }
    virtual bool handleStopWhenCardAway() {
      return true;
    }
    uint8_t getActive() {
      return ButtonSmashMod ;
    }
};
//////////////////////////////////////////////////////////////////////////
class Calculate: public Modifier {
  private:
    uint8_t mode = 1; // learn mode (1 = addition, 2 = subtraction, 3 = multiplication, 4 = division, 5 = mixed)
    uint8_t upperBound = 0; // max number (can not exceed 254)
    uint8_t opA = 0;
    uint8_t opB = 0;
    int16_t answer = 0;
    uint8_t answerOld = 0;
    uint8_t result = 0;
    uint8_t opr = 0; // operator, see mode (0..3)
    bool ignUpButton = false;
    bool ignDownButton = false;
    bool ignPauseButton = false;

    void nextQuestion(bool repeat = false) {
#if defined DEBUG
      Serial.println(F("Calculate > next Question"));
#endif
      this->answer = 0;
      this->answerOld = 0;

      if (!repeat) {
        if (this->mode == 5)
        {
          this->opr = random(1, 5);
        }
        else {
          this->opr = this->mode;
        }
        this->result = 0;

        switch (this->opr) {
          case 2: // subtraction
            {
              this->opA = random(2, this->upperBound);
              this->opB = random(1, this->opA);
            }
            break;
          case 3: // multiplication
            {
              this->opA = random(1, this->upperBound);
              this->opB = random(1, floor(this->upperBound / this->opA));

            }
            break;
          case 4: // division
            {
              this->opA = random(2, this->upperBound);
              do {
                this->opB = random(1, this->upperBound);
              } while (this->opA % this->opB  != 0);
            }
            break;
          default: // addition
            {
              this->opA = random(1, this->upperBound - 1);
              this->opB = random(1, this->upperBound - this->opA);
            }
            break;
        }
      }

      switch (this->opr) {
        case 1:
          {
            this->result = this->opA + this->opB;
          }
          break;
        case 2:
          {
            this->result = this->opA - this->opB;
          }
          break;
        case 3:
          {
            this->result = this->opA * this->opB;
          }
          break;
        case 4:
          {
            this->result = this->opA / this->opB;
          }
          break;
      }
#if defined DEBUG
      Serial.print(F("Calculate > correct result: "));
      Serial.println(this->result);
#endif
      mp3.playMp3FolderTrack(411); // question "how much is"
      waitForTrackToFinish();
      mp3.playMp3FolderTrack(this->opA); // 1..254
      waitForTrackToFinish();
      mp3.playMp3FolderTrack(411 + this->opr); // 402, 403, 404, 405
      waitForTrackToFinish();
      mp3.playMp3FolderTrack(this->opB); // 1..254
      waitForTrackToFinish();
    }

    uint8_t readAnswer() {

      if (myTrigger.shutDown) {
#if defined DEBUG
        Serial.println(F("Calculate > long pause "));
#endif
        this->ignPauseButton = true;
        return -1;
      }
      if (myTrigger.pauseTrack) {
#if defined DEBUG
        Serial.println(F("Calculate > release pause "));
#endif
        if (!this->ignPauseButton) {

          if (this->answer != 0) {
#if defined DEBUG
            Serial.print(F("= "));
            Serial.println(this->answer);
#endif
            mp3Pause();
            return 1;
          }
          else {
            this->ignPauseButton = false;
          }

        }
      }
      if (myTrigger.nextPlusTen) {
        this->answer = min(this->answer + 10, this->upperBound);
        this->ignUpButton = true;
      } else if (myTrigger.next) {
        if (!this->ignUpButton) {
          this->answer = min(this->answer + 1, this->upperBound) ;
        } else {
          this->ignUpButton = false;
        }
      }

      if (myTrigger.previousPlusTen) {
        this->answer = max(this->answer - 10, 1);
        this->ignDownButton = true;
      } else if (myTrigger.previous) {
        if (!this->ignDownButton) {
          this->answer = max(this->answer - 1, 1);
        } else {
          this->ignDownButton = false;
        }
      }

      if (this->answer != this->answerOld) {
        this->answerOld = this->answer;
#if defined DEBUG
        Serial.println(this->answer);
#endif
        mp3.playMp3FolderTrack(this->answer);
        waitForTrackToFinish();
      }

      return 0;
    }

  public:
    virtual bool handlePause() {
      return true;
    }
    virtual bool handleNext() {
      return true;
    }
    virtual bool handlePrevious() {
      return true;
    }
    virtual bool handlenextAction() {
      return true;
    }
    virtual bool handlepreviousAction() {
      return true;
    }
    virtual bool handleShortCut() {
      return true;
    }
    virtual bool handleVolume() {
      return true;
    }
    virtual bool handleShutDown() {
      return true;
    }
    virtual bool handleRFID(nfcTagObject *newCard) {
      return true;
    }
    virtual bool handleAdminMenu() {
      return true;
    }
    virtual void loop() {
      int8_t value = readAnswer();

      if (value == -1) {
        this->answer = 0;
        this->answerOld = 0;
        mp3.playMp3FolderTrack(298); // next Question
        waitForTrackToFinish();
        this->nextQuestion();
        return;
      } else if (value == 1) {
        if (this->result == this->answer) {
#if defined DEBUG
          Serial.println(F("Calculate > right"));
#endif

          mp3.playMp3FolderTrack(420); // richtig
          waitForTrackToFinish();
          this->nextQuestion();
        }
        else {
#if defined DEBUG
          Serial.println(F("Calculate > wrong"));
#endif

          mp3.playMp3FolderTrack(421); // falsch
          waitForTrackToFinish();
          this->nextQuestion(true); // repeat question
        }
      }
      return;
    }

    Calculate(uint8_t special, uint8_t special2) {
#if defined DEBUG
      Serial.println(F("Calculate"));
#endif
      mp3.loop();
      mp3Pause();

      this->mode = special;
      this->upperBound = special2;

      if (this->mode == 5) {
        this->opr = random(1, 5);
      }
      else {
        this->opr = this->mode;
      }
      mp3.playMp3FolderTrack(410); // intro
      waitForTrackToFinish();
      this->nextQuestion();
    }

    uint8_t getActive() {
      return CalculateMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class Locked: public Modifier {
  public:
    virtual bool handlePause()     {
      return true;
    }
    virtual bool handlenextAction()       {
      return true;
    }
    virtual bool handlepreviousAction() {
      return true;
    }
    virtual bool handleVolume()   {
      return true;
    }
    virtual bool handleShutDown() {
      return true;
    }

    virtual bool handleShortCut() {
      return true;
    }
    virtual bool handleAdminMenu() {
      return true;
    }

    virtual bool handleRFID(nfcTagObject *newCard) {
      return true;
    }
    Locked(void) {
#if defined DEBUG
      Serial.println(F("Locked Moifier"));
#endif
    }
    uint8_t getActive() {
      return LockedMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class ToddlerMode: public Modifier {
  public:
    virtual bool handlePause()     {
      return true;
    }
    virtual bool handlenextAction()       {
      return true;
    }
    virtual bool handlepreviousAction() {
      return true;
    }
    virtual bool handleVolume()   {
      return true;
    }
    virtual bool handleShutDown() {
      return true;
    }
    virtual bool handleShortCut() {
      return true;
    }
    virtual bool handleAdminMenu() {
      return true;
    }

    ToddlerMode() {
#if defined DEBUG
      Serial.println(F("ToddlerMode"));
#endif
    }

    uint8_t getActive() {
      return ToddlerModeMod ;
    }
};
//////////////////////////////////////////////////////////////////////////
class KindergardenMode: public Modifier {
  private:
    nfcTagObject nextCard;
    bool cardQueued = false;

  public:
    virtual bool handleNext() {
#if defined DEBUG
      Serial.println(F("KindergardenMode > next"));
#endif
      if (this->cardQueued == true) {
        this->cardQueued = false;

        myCard = nextCard;
        *myFolder = myCard.nfcFolderSettings;
#if defined DEBUG
        Serial.println(myFolder->folder);
        Serial.println(myFolder->mode);
#endif
        playFolder();
        return true;
      }
      return false;
    }
    virtual bool handlenextAction()       {
      return true;
    }
    virtual bool handlepreviousAction() {
      return true;
    }
    virtual bool handleShutDown() {
      return true;
    }
    virtual bool handleAdminMenu() {
      return true;
    }

    virtual bool handleShortCut() {
      if (isPlaying())
        return true;
      else
        return false;
    }

    virtual bool handleRFID(nfcTagObject * newCard) { // lot of work to do!
#if defined DEBUG
      Serial.println(F("KindergardenMode > RFID queued"));
#endif
      this->nextCard = *newCard;
      this->cardQueued = true;
      if (!isPlaying()) {
        handleNext();
      }
      return true;
    }
    KindergardenMode() {
#if defined DEBUG
      Serial.println(F("KindergardenMode"));
#endif
    }
    uint8_t getActive() {
      return KindergardenModeMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class RepeatSingleModifier: public Modifier {
  public:
    virtual bool handleNext() {
#if defined DEBUG
      Serial.println(F("RepeatSingle > repeat track"));
#endif
      delay(50);
      if (isPlaying())
        return true;
      if (myFolder->mode == Party || myFolder->mode == Party_Section) {
        mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
      }
      else {
        mp3.playFolderTrack(myFolder->folder, currentTrack);
      }

      _lastTrackFinished = 0;
      return true;
    }
    RepeatSingleModifier() {
#if defined DEBUG
      Serial.println(F("RepeatSingle"));
#endif
    }
    uint8_t getActive() {
      return RepeatSingleMod;
    }
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void playFolder() {
  uint8_t counter = 0;
  bool queueTrack = false;

#ifdef DEBUG
  Serial.println(F("playFolder")) ;
#endif

  mp3.loop();
  disablestandbyTimer();
  knownCard = true;
  _lastTrackFinished = 0;
  numTracksInFolder = mp3.getFolderTrackCount(myFolder->folder);
  firstTrack = 1;

#ifdef DEBUG
  Serial.print(numTracksInFolder);
  Serial.print(F(" tracks in folder "));
  Serial.println(myFolder->folder);
#endif

  switch (myFolder->mode) {
    case AudioDrama:
#ifdef DEBUG
      Serial.println(F("Audio Drama"));
#endif
      currentTrack = random(1, numTracksInFolder + 1);
      break;
    case AudioDrama_Section:
#ifdef DEBUG
      Serial.println(F("Audio Drama section"));
      Serial.print(myFolder->special);
      Serial.print(F(" to "));
      Serial.println(myFolder->special2);
#endif
      //numTracksInFolder = myFolder->special2;
      firstTrack = myFolder->special;
      currentTrack = random(myFolder->special, myFolder->special2 + 1);
      break;

    case Album:
#ifdef DEBUG
      Serial.println(F("Album"));
#endif
      currentTrack = 1;
      break;
    case Album_Section:
#ifdef DEBUG
      Serial.println(F("Album section"));
      Serial.print(myFolder->special);
      Serial.print(F(" to "));
      Serial.println(myFolder->special2);
#endif
      //numTracksInFolder = myFolder->special2;
      firstTrack = myFolder->special;
      currentTrack = myFolder->special;
      break;

    case Party:
#ifdef DEBUG
      Serial.println(F("Party"));
#endif
      currentTrack = 1;
      shuffleQueue();
      queueTrack = true;
      break;
    case Party_Section:
#ifdef DEBUG
      Serial.println(F("Party section"));
#endif
      currentTrack = 1;
      firstTrack = myFolder->special;
      //numTracksInFolder = myFolder->special2;
      shuffleQueue(firstTrack, myFolder->special2);
      queueTrack = true;
      break;

    case AudioBook:
#ifdef DEBUG
      Serial.println(F("Audio Book"));
#endif
      currentTrack = myFolder->special3; //readAudiobookMemory(myFolder->folder, myFolder->special3);
      if (currentTrack == 0 || currentTrack > numTracksInFolder) {
        currentTrack = 1;
      }
      break;
    case AudioBook_Section:
#ifdef DEBUG
      Serial.println(F("Audio Book section"));
#endif
      firstTrack = myFolder->special;
      //numTracksInFolder = myFolder->special2;
      currentTrack = myFolder->special3;//readAudiobookMemory(myFolder->folder, myFolder->special3);
      if (currentTrack < firstTrack || currentTrack >= firstTrack + numTracksInFolder) {
        currentTrack = firstTrack;
#ifdef DEBUG
        Serial.println(F("not correct tag, start from beginn"));
#endif
      }
      break;

    case Single:
#ifdef DEBUG
      Serial.println(F("Single"));
#endif
      currentTrack = myFolder->special;
      break;

    case PuzzlePart:
#ifdef DEBUG
      Serial.println(F("Puzzle Part"));
#endif
      currentTrack = myFolder->special;
      break;
  }

#ifdef DEBUG
  Serial.print(F("play track: "));
#endif

  if (queueTrack) {
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
#ifdef DEBUG
    Serial.println(queue[currentTrack - 1]);
#endif
  }
  else {
    mp3.playFolderTrack(myFolder->folder, currentTrack);
#ifdef DEBUG
    Serial.println(currentTrack);
#endif
  }

  waitForTrackToStart();

}

void activateShortCut (uint8_t shortCutNo) {
  if (!isPlaying()) {
#if defined DEBUG
    Serial.print("play short cut ");
#endif
    if (!(mySettings.shortCuts[shortCutNo - 1].folder == 0 && mySettings.shortCuts[shortCutNo - 1].mode == 0)) {
      Serial.println(shortCutNo);
      switch (mySettings.shortCuts[shortCutNo - 1].folder) {
        case ModifierMode:
          SetModifier (&mySettings.shortCuts[shortCutNo - 1]);
          break;
        default:
          if (activeModifier != NULL) {
            if (activeModifier->handleShortCut() == true) {
#if defined DEBUG
              Serial.println(F("locked"));
#endif
              return;
            }
          }
          myFolder = &mySettings.shortCuts[shortCutNo - 1];
          playFolder();
          break;
      }
    } else {
      setupShortCut(shortCutNo);
    }
  }
}
//////////////////////////////////////////////////////////////////////////
// Leider kann das Modul selbst keine Queue abspielen, daher müssen wir selbst die Queue verwalten
//////////////////////////////////////////////////////////////////////////
static void nextTrack(uint8_t track) {
  bool queueTrack = false;

  if (track == _lastTrackFinished || knownCard == false) {
    return;
  }
  _lastTrackFinished = track;
#ifdef DEBUG
  Serial.println(F("Next track"));
#endif

  if (activeModifier != NULL) {
    if (activeModifier->handleNext() == true) {
#ifdef DEBUG
      Serial.println(F("locked"));
#endif
      return;
    }
  }

  switch (myFolder->mode) {
    case AudioDrama:
      //currentTrack = random(1, numTracksInFolder + 1);
      break;
    case Album:
      if (currentTrack < numTracksInFolder)
        currentTrack = currentTrack + 1;
      else {
        currentTrack = 0;
        mp3Pause();
        setstandbyTimer();
        return;
      }
      break;
    case Album_Section:
      if (currentTrack < myFolder->special2)
        currentTrack = currentTrack + 1;
      else {
        currentTrack = firstTrack - 1;
        mp3Pause();
        setstandbyTimer();
        return;
      }
      break;

    case Party:
      if (currentTrack < numTracksInFolder) {
        currentTrack = currentTrack + 1;
      }
      else {
        currentTrack = 1;
        //// Wenn am Ende der Queue neu gemischt werden soll bitte die Zeilen wieder aktivieren
        //     Serial.println(F("Ende der Queue > mische neu"));
        //     shuffleQueue();
      }
      queueTrack = true;
      break;
    case Party_Section:
      if (currentTrack < (myFolder->special2 - firstTrack + 1)) {
        currentTrack = currentTrack + 1;
      }
      else {
        currentTrack = 1;
        //// Wenn am Ende der Queue neu gemischt werden soll bitte die Zeilen wieder aktivieren
        //     Serial.println(F("Ende der Queue > mische neu"));
        //     shuffleQueue(firstTrack,myFolder->special2);
      }
      queueTrack = true;
      break;

    case AudioBook:
      if (currentTrack < numTracksInFolder) {
        currentTrack = currentTrack + 1;
        writeCardMemory (currentTrack);
      }
      else {
        writeCardMemory (firstTrack);
        mp3Pause();
        setstandbyTimer();
        return;
      }

      break;
    case AudioBook_Section:
      if (currentTrack < myFolder->special2) {
        currentTrack = currentTrack + 1;
        writeCardMemory (currentTrack);
      }
      else {
        writeCardMemory (firstTrack);
        mp3Pause();
        setstandbyTimer();
        return;
      }
      break;

    default:
#ifdef DEBUG
      Serial.println(F("no next Track"));
#endif
      setstandbyTimer();
      //delay(500);
      return;
      break;
  }

  disablestandbyTimer();

  Serial.print("next track: ");

  if (queueTrack) {
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
    Serial.println(queue[currentTrack - 1]);
  }
  else {
    mp3.playFolderTrack(myFolder->folder, currentTrack);
    Serial.println(currentTrack);
  }
}

static void previousTrack() {
  bool queueTrack = false;

  if (activeModifier != NULL) {
    if (activeModifier->handlePrevious() == true) {
#ifdef DEBUG
      Serial.println(F("locked"));
#endif
      return;
    }
  }

  _lastTrackFinished = 0;

  switch (myFolder->mode) {
    case Album:
      if (currentTrack > 1) {
        currentTrack = currentTrack - 1;
      }
      else {
        currentTrack = numTracksInFolder;
      }
      break;
    case Album_Section:
      if (currentTrack > firstTrack) {
        currentTrack = currentTrack - 1;
      }
      else {
        currentTrack = myFolder->special2 - firstTrack + 1;
      }
      break;

    case Party:
      if (currentTrack > 1) {
        currentTrack = currentTrack - 1;
      }
      else {
        currentTrack = numTracksInFolder;
      }
      queueTrack = true;
      break;
    case Party_Section:
      if (currentTrack > 1) {
        currentTrack = currentTrack - 1;
      }
      else {
        currentTrack = myFolder->special2 - firstTrack + 1;
      }
      queueTrack = true;
      break;

    case AudioBook:
      if (currentTrack > 1) {
        currentTrack = currentTrack - 1;
      } else {
        currentTrack = 1;
      }
      // Fortschritt im EEPROM abspeichern
      writeCardMemory (currentTrack);
      break;

    case AudioBook_Section:
      if (currentTrack > firstTrack) {
        currentTrack = currentTrack - 1;

      } else {
        currentTrack = 1;
      }
      // Fortschritt im EEPROM abspeichern
      writeCardMemory (currentTrack);
      break;

    default:
      Serial.println(F("no previous Track"));
      setstandbyTimer();
      return;
      break;
  }

  disablestandbyTimer();

  Serial.print("previous track: ");

  if (queueTrack) {
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
    Serial.println(queue[currentTrack - 1]);
  }
  else {
    mp3.playFolderTrack(myFolder->folder, currentTrack);
    Serial.println(currentTrack);
  }
}
//////////////////////////////////////////////////////////////////////////
bool isPlaying() {
  return !digitalRead(busyPin);
}

void mp3Pause(uint16_t delayTime = 100) {
  mp3.pause();
  delay(delayTime);
}

void waitForTrackToFinish (bool interruptByTrigger = true, uint16_t timeOut = 5000) {
  waitForTrackToStart(timeOut);
  while (isPlaying()) {
    mp3.loop();
    readTrigger();
    if (!myTrigger.noTrigger) {
      break;
    }
    resetTrigger();
  }
  resetTrigger();
}

void waitForTrackToStart(uint16_t timeOut = 5000) {
  unsigned long waitForTrackToStartMillis = millis();
  delay(200);
  while (!isPlaying()) {
    if (millis() - waitForTrackToStartMillis >= timeOut) {
#ifdef DEBUG
      Serial.println(F("Track not starting")) ;
#endif
      break;
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void setup() {
#if defined AiO
  // spannung einschalten
  pinMode(shutdownPin, OUTPUT);
  digitalWrite(shutdownPin, HIGH);

  // sd karten zugang aus
  pinMode(A5, OUTPUT);
  digitalWrite(A5, LOW);
#else
  pinMode(shutdownPin, OUTPUT);
  digitalWrite(shutdownPin, LOW);
#endif

#if defined AiO
  // verstärker aus
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  delay(100);
#else if defined SPEAKER_SWITCH
  pinMode(SpeakerOnPin, OUTPUT);
  digitalWrite(SpeakerOnPin, LOW);
#endif

  Serial.begin(115200); // Es gibt ein paar Debug Ausgaben über die serielle Schnittstelle
  // Dieser Hinweis darf nicht entfernt werden
  //  Serial.println(F("\n _____         _____ _____ _____ _____"));
  //  Serial.println(F("|_   _|___ ___|  |  |     |   | |     |"));
  //  Serial.println(F(" | | | . |   |  |  |-   -| | | |  |  |"));
  //  Serial.println(F(" |_| |___|_|_|_____|_____|_|___|_____|\n"));

  Serial.print(F("TonUINO Version "));
  Serial.println(VERSION_TONUINO);
#if defined AiO
  Serial.println(F("AiO"));
#endif
  Serial.println(F("created by Thorsten Voß and licensed under GNU/GPL."));
  Serial.println(F("Information and contribution at https://tonuino.de.\n"));
  Serial.println(F("Fork by Marco Schulz DEVELOP"));


#if defined AiO
  analogReference(INTERNAL4V096);
  analogReadResolution(12);
#else
  analogReference(DEFAULT);
#endif

  // Busy Pin
  pinMode(busyPin, INPUT);
  mp3.begin();


  // Wert für randomSeed() erzeugen durch das mehrfache Sammeln von rauschenden LSBs eines offenen Analogeingangs
  uint32_t ADC_LSB;
  uint32_t ADCSeed;
  for (uint8_t i = 0; i < 128; i++) {
    ADC_LSB = analogRead(openAnalogPin) & 0x1;
    ADCSeed ^= ADC_LSB << (i % 32);
  }
  randomSeed(ADCSeed); // Zufallsgenerator initialisieren

#if defined ROTARY_ENCODER
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  encoder.setAccelerationEnabled(false);
#if defined ROTARY_ENCODER_PIN_SUPPLY
  pinMode(ROTARY_ENCODER_PIN_SUPPLY, OUTPUT);
  digitalWrite(ROTARY_ENCODER_PIN_SUPPLY, HIGH);
#endif
#endif

#if defined IRREMOTE
  irReceiver.enableIRIn();
#endif

#if defined POWER_ON_LED
  pinMode(PowerOnLEDPin, OUTPUT);
  digitalWrite(PowerOnLEDPin, LOW);
#endif

  // activate standby timer
  setstandbyTimer();


  // NFC Leser initialisieren
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
#if defined DEBUG
  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader
#endif
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

#if defined NFCgain_min
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_min);
#endif
#if defined NFCgain_avg
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_avg);
#endif
#if defined NFCgain_max
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
#endif

  pinMode(buttonPause, INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
#if defined FIVEBUTTONS
  pinMode(buttonFourPin, INPUT_PULLUP);
  pinMode(buttonFivePin, INPUT_PULLUP);
#endif

  resetTrigger();
  resetTriggerEnable();


  // RESET --- ALLE DREI KNÖPFE BEIM STARTEN GEDRÜCKT HALTEN > alle EINSTELLUNGEN werden gelöscht
  if (digitalRead(buttonPause) == LOW && digitalRead(buttonUp) == LOW &&
      digitalRead(buttonDown) == LOW) {
    Serial.println(F("delete EEPROM"));
#if defined AiO
    for (uint16_t i = 0; i < EEPROM_size; i++) {
      EEPROM_update(i, 0);
    }
#else
    for (uint16_t i = 0; i < EEPROM.length(); i++) {
      EEPROM.update(i, 0);
    }
#endif
    resetSettings();
  }
  // load Settings from EEPROM
  printSettings();

#if defined AiO
  // verstärker an
  digitalWrite(8, LOW);
  delay(500);
#else if defined SPEAKER_SWITCH
  digitalWrite(SpeakerOnPin, HIGH);
  delay(500);
#endif

#if defined POWER_ON_LED
  digitalWrite(PowerOnLEDPin, HIGH);
#endif

  delay(2000);
  mp3.loop();
  volume = mySettings.initVolume;
  mp3.setVolume(volume);
  mp3.setEq(mySettings.eq - 1);

  mp3.playMp3FolderTrack(264);
  waitForTrackToFinish();

  // Set saved Modifier
  if (mySettings.savedModifier.mode != 0)
    SetModifier(&mySettings.savedModifier);
}
//////////////////////////////////////////////////////////////////////////
void readButtons() {
  inputTrigger copyTrigger;

  pauseButton.read();
  upButton.read();
  downButton.read();
#if defined FIVEBUTTONS
  buttonFour.read();
  buttonFive.read();
#endif
  myTrigger.pauseTrack |= pauseButton.wasReleased();
  myTrigger.next |= upButton.wasReleased();
  myTrigger.nextPlusTen |= upButton.pressedFor(LONG_PRESS);
  myTrigger.previous |= downButton.wasReleased();
  myTrigger.previousPlusTen |= downButton.pressedFor(LONG_PRESS);
#if defined FIVEBUTTONS
  myTrigger.volumeUp |= buttonFour.wasReleased();
  myTrigger.volumeDown |= buttonFive.wasReleased();
  myTrigger.shortCutNo[0] |= buttonFour.pressedFor(LONG_PRESS);
  myTrigger.shortCutNo[1] |= buttonFive.pressedFor(LONG_PRESS);
#else
  myTrigger.volumeUp |= upButton.pressedFor(LONG_PRESS);
 myTrigger.volumeDown |= downButton.pressedFor(LONG_PRESS);
   myTrigger.shortCutNo[0] |= upButton.pressedFor(LONG_PRESS);
  myTrigger.shortCutNo[1] |= downButton.pressedFor(LONG_PRESS);
#endif
  myTrigger.shutDown |= pauseButton.pressedFor(LONGER_PRESS) &&
                        !upButton.isPressed() && !downButton.isPressed();
  myTrigger.adminMenu |=  (pauseButton.pressedFor(LONG_PRESS) || upButton.pressedFor(LONG_PRESS) || downButton.pressedFor(LONG_PRESS)) &&
                          pauseButton.isPressed() && upButton.isPressed() && downButton.isPressed();
  myTrigger.resetTrack |= (upButton.pressedFor(LONGER_PRESS) || downButton.pressedFor(LONGER_PRESS)) && !pauseButton.isPressed() && upButton.isPressed() && downButton.isPressed();

  if (mySettings.invertVolumeButtons) {
    copyTrigger.next = myTrigger.next;
    copyTrigger.previous = myTrigger.previous;
    copyTrigger.volumeUp = myTrigger.volumeUp;
    copyTrigger.volumeDown = myTrigger.volumeDown;

    myTrigger.volumeUp = copyTrigger.next;
    myTrigger.volumeDown = copyTrigger.previous;
    myTrigger.next = copyTrigger.volumeUp;
    myTrigger.previous = copyTrigger.volumeDown;
  }
}

#if defined IRREMOTE
void readIr () {
  Enum_Trigger irRemoteEvent = NoTrigger;
  uint16_t irRemoteCode = 0;
  static unsigned long irRemoteOldMillis;

  // poll ir receiver, has precedence over (overwrites) physical buttons
  if (irReceiver.decode(&irReading)) {
    // on NEC encoding 0xFFFFFFFF means the button is held down, we ignore this
    if (!(irReading.decode_type == NEC && irReading.value == 0xFFFFFFFF)) {
      // convert irReading.value from 32bit to 16bit
      irRemoteCode = (irReading.value & 0xFFFF);
      for (uint8_t j = 0; j < irRemoteCodeCount; j++) {
        //if we have a match, temporally populate irRemoteEvent and break
        if (irRemoteCode == mySettings.irRemoteUserCodes[j]) {
          irRemoteEvent = j + 1;
          break;
        }
      }
      // if the inner loop had a match, populate inputEvent and break
      // ir remote key presses are debounced by 250ms
      if (irRemoteEvent != 0 && millis() - irRemoteOldMillis >= 250) {
        irRemoteOldMillis = millis();
        switch (irRemoteEvent) {
          case PauseTrackTrigger:
            myTrigger.pauseTrack |= true;
            break;
          case NextTrigger:
            myTrigger.next |= true;
            break;
          case NextPlusTenTrigger:
            myTrigger.nextPlusTen |= true;
            break;
          case PreviousTrigger:
            myTrigger.previous |= true;
            break;
          case PreviousPlusTenTrigger:
            myTrigger.previousPlusTen |= true;
            break;
          case VolumeUpTrigger:
            myTrigger.volumeUp |= true;
            break;
          case VolumeDownTrigger:
            myTrigger.volumeDown |= true;
            break;
          case ShutDownTrigger:
            myTrigger.shutDown |= true;
            break;
          case AdminMenuTrigger:
            myTrigger.adminMenu |= true;
            break;
          case ResetTrackTrigger:
            myTrigger.resetTrack |= true;
            break;
          default:
            if (irRemoteEvent >= ShortcutTrigger) {
              myTrigger.shortCutNo[irRemoteEvent - ShortcutTrigger] |= true;
            }
            break;
        }
      }
    }
    irReceiver.resume();
  }
}
#endif

#if defined ANALOG_INPUT
void readAnalogIn() {
  uint16_t currentAnalogValue = analogRead(ANALOG_INPUT_PIN) ;
  int8_t analogInCurrentPosition = -1;

  //Abfrage der Position zu diesem Wert
  for (uint8_t i = 0; i < inputTriggerCount; i++) {
    if (currentAnalogValue > (mySettings.analogInUserValues[i] - ((mySettings.analogInUserValues[i] * mySettings.analogInputTolerance) / 100)) && currentAnalogValue < (mySettings.analogInUserValues[i] + ((mySettings.analogInUserValues[i] * mySettings.analogInputTolerance) / 100))) {
      //analoger Wert in Tabelle hinterlegt
      analogInCurrentPosition = i;
      break;
    }
  }

  if ((millis() - analogInMillis >= mySettings.analogInputTriggerTime) && (analogInLastPosition == analogInCurrentPosition)) {
    analogInLastPosition = analogInCurrentPosition; //aktuelle aktivierte Position speichern. Falls ein Drehschalter verwendet wird soll die Position nicht nach jedem Abaluf der Triggertime aktiviert werden.
    switch (analogInCurrentPosition) {
      case PauseTrackTrigger:
        myTrigger.pauseTrack |= true;
        break;
      case NextTrigger:
        myTrigger.next |= true;
        break;
      case NextPlusTenTrigger:
        myTrigger.nextPlusTen |= true;
        break;
      case PreviousTrigger:
        myTrigger.previous |= true;
        break;
      case PreviousPlusTenTrigger:
        myTrigger.previousPlusTen |= true;
        break;
      case VolumeUpTrigger:
        myTrigger.volumeUp |= true;
        break;
      case VolumeDownTrigger:
        myTrigger.volumeDown |= true;
        break;
      case ShutDownTrigger:
        myTrigger.shutDown |= true;
        break;
      case AdminMenuTrigger:
        myTrigger.adminMenu |= true;
        break;
      case ResetTrackTrigger:
        myTrigger.resetTrack |= true;
        break;
      default:
        if (analogInCurrentPosition >= ShortcutTrigger) {
          myTrigger.shortCutNo[analogInCurrentPosition - ShortcutTrigger] |= true;
        }
        break;
    }
  } else if ( analogInCurrentPosition != analogInLastPosition) {
    analogInLastPosition = analogInCurrentPosition;
#if defined ANALOG_INPUT_PRINT
    Serial.print(F("new analog position: "));
    Serial.println(analogInCurrentPosition);
#endif
    //Wenn aktueller analoger Wert +/- des Toleranzwertes vom letzten erkannten Wert abweicht
    if (mySettings.analogInputTriggerTime > 0) {
      analogInMillis = millis(); //Rücksetzen der Triggerzeit
    }
  }
}
#endif

#if defined ROTARY_ENCODER
void RotEncSetVolume () {

  RotEncPos += encoder.getValue();

  if (RotEncPos != RotEncOldEncPos)  {
#if defined ROTARY_ENCODER_PRINT
    Serial.print(F("RotEncPos: "));
    Serial.println(RotEncPos);
    Serial.print(F("RotEncOldEncPos: "));
    Serial.println(RotEncOldEncPos);
#endif

    RotEncOldEncPos = RotEncPos;

    if (RotEncPos > (mySettings.maxVolume)) {
#if defined ROTARY_ENCODER_PRINT
      Serial.println(F("RotEncPos corrected to max"));
#endif
      RotEncPos  = mySettings.maxVolume;
    }
    else if (RotEncPos < (mySettings.minVolume)) {
#if defined ROTARY_ENCODER_PRINT
      Serial.println(F("RotEncPos corrected to min"));
#endif
      RotEncPos  = mySettings.minVolume;
    }

    if (activeModifier != NULL) {
      if (activeModifier->handleVolume() == true) {
        return;
      }
    }
    volume = RotEncPos;
    mp3.setVolume(volume);
#if defined DEBUG || defined ROTARY_ENCODER_PRINT
    Serial.print(F("volume: "));
    Serial.println(volume);
#endif
  }
}
void timerIsr() {
  encoder.service();
}
#endif

void readTrigger() {
  readButtons();

#if defined IRREMOTE
  readIr();
#endif

#if defined ANALOG_INPUT
  readAnalogIn();
#endif

  checkNoTrigger ();

  if (myTrigger.noTrigger == true) {
    resetTriggerEnable();
  }
}

void checkNoTrigger () {
  myTrigger.noTrigger = !(myTrigger.pauseTrack || myTrigger.next || myTrigger.nextPlusTen || myTrigger.previous || myTrigger.previousPlusTen
                          || myTrigger.volumeUp || myTrigger.volumeDown || myTrigger.pauseTrack || myTrigger.shutDown || myTrigger.adminMenu || myTrigger.resetTrack);

  for (uint8_t i = 0; i < availableShortCuts; i++) {
    myTrigger.noTrigger &= !myTrigger.shortCutNo[i];
    }
}

void resetTrigger() {
  myTrigger.noTrigger = false;
  myTrigger.pauseTrack = false;
  myTrigger.next = false;
  myTrigger.nextPlusTen = false;
  myTrigger.previous = false;
  myTrigger.previousPlusTen = false;
  myTrigger.volumeUp = false;
  myTrigger.volumeDown = false;
  for (uint8_t i = 0; i < availableShortCuts; i++) {
    myTrigger.shortCutNo[i] = false;
    }
  myTrigger.shutDown = false;
  myTrigger.adminMenu = false;
  myTrigger.resetTrack = false;
}

void resetTriggerEnable() {
  myTriggerEnable.noTrigger = true;
  myTriggerEnable.pauseTrack = true;
  myTriggerEnable.next = true;
  myTriggerEnable.nextPlusTen = true;
  myTriggerEnable.previous = true;
  myTriggerEnable.previousPlusTen = true;
  myTriggerEnable.volumeUp = true;
  myTriggerEnable.volumeDown = true;
  for (uint8_t i = 0; i < availableShortCuts; i++) {
    myTriggerEnable.shortCutNo[i] = true;
    }
  myTriggerEnable.shutDown = true;
  myTriggerEnable.adminMenu = true;
  myTriggerEnable.resetTrack = true;
}
//////////////////////////////////////////////////////////////////////////
void volumeUpAction() {
  if (activeModifier != NULL) {
    if (activeModifier->handleVolume() == true) {
#if defined DEBUG
      Serial.println(F("Volume locked"));
#endif
      return;
    }
  }
  if (isPlaying()) {
    if (volume < mySettings.maxVolume) {
      mp3.increaseVolume();
      volume++;
    }
#if defined DEBUG
    Serial.print(F("volume Up: "));
    Serial.println(volume);
#endif
  }
  delay(75);
}
//////////////////////////////////////////////////////////////////////////
void volumeDownAction() {
  if (activeModifier != NULL) {
    if (activeModifier->handleVolume() == true) {
#if defined DEBUG
      Serial.println(F("volume command locked"));
#endif
      return;
    }
  }

  if (isPlaying()) {
    if (volume > mySettings.minVolume) {
      mp3.decreaseVolume();
      volume--;
    }
#if defined DEBUG
    Serial.print(F("volume Down "));
    Serial.println(volume);
#endif
  }
  delay(75);
}
//////////////////////////////////////////////////////////////////////////
void nextAction() {
  if (activeModifier != NULL) {
    if (activeModifier->handlenextAction() == true) {
#if defined DEBUG
      Serial.println(F("next button locked"));
#endif
      return;
    }
  }
  if (myTriggerEnable.next == true) {
    myTriggerEnable.next = false;
    nextTrack(random(65536));
  }
}
//////////////////////////////////////////////////////////////////////////
void previousAction() {
  if (activeModifier != NULL) {
    if (activeModifier->handlepreviousAction() == true) {
#if defined DEBUG
      Serial.println(F("previous command locked"));
#endif
      return;
    }
  }
  if (myTriggerEnable.previous == true) {
    myTriggerEnable.previous = false;
    previousTrack();
  }
}
//////////////////////////////////////////////////////////////////////////
void pauseAction() {
  if (activeModifier != NULL) {
    if (activeModifier->handlePause() == true) {
#if defined DEBUG
      Serial.println(F("Pause command locked"));
#endif
      return;
    }
  }
  if (myTriggerEnable.pauseTrack == true) {
    myTriggerEnable.pauseTrack = false;
    if (isPlaying()) {
      Serial.println(F("pause"));
      mp3Pause();
      setstandbyTimer();
    }
    else if (knownCard) {
      Serial.println(F("play"));
      mp3.start();
      disablestandbyTimer();
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void loop() {
  checkStandbyAtMillis();
  mp3.loop();

#if defined ROTARY_ENCODER
  RotEncSetVolume();
#endif

#if defined FADING_LED
  fadeStatusLed(isPlaying());
#endif

  readTrigger();

  if (activeModifier != NULL) {
    activeModifier->loop();
  }

  // admin menu
  if (myTrigger.adminMenu) {
    mp3Pause();
    do {
      readTrigger();
      if (myTrigger.noTrigger == true) {
        break;
      }
      resetTrigger();
    } while (true);
    resetTrigger();
    adminMenu();
    return;
  }

  //Springe zum ersten Titel zurück
  if (myTrigger.resetTrack) {
    mp3Pause();
    do {
      readTrigger();
      if (myTrigger.noTrigger == true) {
        break;
      }
      resetTrigger();
    } while (true);
    resetTrigger();
    readTrigger();
#if defined DEBUG
    Serial.println(F("back to first track"));
#endif
    if (currentTrack != firstTrack) {
      currentTrack = firstTrack;
      if (myFolder->special3 != 0) {
#if defined DEBUG
        Serial.println(F("reset memory"));
#endif
        writeCardMemory (currentTrack);
      }
    }
    playFolder();
    return;
  }

  if (myTrigger.pauseTrack) {
    pauseAction();
  }

  else if (myTrigger.shutDown) {
    shutDown();
  }

  if (myTrigger.volumeUp) {
    volumeUpAction();
  }

  if (myTrigger.volumeDown) {
    volumeDownAction();
  }

  if (myTrigger.next) {
    nextAction();
  }

  if (myTrigger.previous) {
    previousAction();
  }

  for (uint8_t i = 0; i < availableShortCuts; i++) {
    if (myTrigger.shortCutNo[i]) {
      activateShortCut(i);
    }
  }
  resetTrigger();
  handleCardReader();

}
//////////////////////////////////////////////////////////////////////////
void adminMenu(bool fromCard = false) {
  if (activeModifier != NULL) {
    if (activeModifier->handleAdminMenu() == true) {
#if defined DEBUG
      Serial.println(F("Admin Menu locked"));
#endif
      return;
    }
  }
  forgetLastCard = true;
  disablestandbyTimer();
  mp3Pause();
#if defined DEBUG
  Serial.println(F("adminMenu"));
#endif
  knownCard = false;
  if (fromCard == false) {
    // Admin menu has been locked - it still can be trigged via admin card
    if (mySettings.adminMenuLocked == 1) {
      return;
    }
  }

  int subMenu = voiceMenu(15, 900, 900, false, false, 0);
  if (subMenu == Exit) {
    return;
  }
  else if (subMenu == ResetCard) {
    resetCard();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
  else if (subMenu == MaxVolume) {
    // Maximum Volume
    mySettings.maxVolume = voiceMenu(30 - mySettings.minVolume, 930, mySettings.minVolume, false, false, mySettings.maxVolume - mySettings.minVolume) + mySettings.minVolume;
  }
  else if (subMenu == MinVolume) {
    // Minimum Volume
    mySettings.minVolume = voiceMenu(mySettings.maxVolume - 1, 931, 0, false, false, mySettings.minVolume);
  }
  else if (subMenu == InitVolume) {
    // Initial Volume
    mySettings.initVolume = voiceMenu(mySettings.maxVolume - mySettings.minVolume + 1, 932, mySettings.minVolume - 1, false, false, mySettings.initVolume - mySettings.minVolume + 1) + mySettings.minVolume - 1;
  }
  else if (subMenu == EQ) {
    // EQ
    mySettings.eq = voiceMenu(6, 920, 920);
    mp3.setEq(mySettings.eq - 1);
  }
  else if (subMenu == SetupShortCuts) {
    setupShortCut();
  }
  else if (subMenu == SetupStandbyTimer) {
    switch (voiceMenu(5, 960, 960)) {
      case 1: mySettings.standbyTimer = 5; break;
      case 2: mySettings.standbyTimer = 15; break;
      case 3: mySettings.standbyTimer = 30; break;
      case 4: mySettings.standbyTimer = 60; break;
      case 5: mySettings.standbyTimer = 0; break;
    }
  }
  else if (subMenu == CreateFolderCards) {
    // Create Cards for Folder
    // Ordner abfragen
    nfcTagObject tempCard;
    tempCard.cookie = cardCookie;
    tempCard.version = 1;
    tempCard.nfcFolderSettings.mode = Single;
    tempCard.nfcFolderSettings.folder = voiceMenu(99, 301, 0, true);
    uint8_t special = voiceMenu(mp3.getFolderTrackCount(tempCard.nfcFolderSettings.folder), 321, 0,
                                true, tempCard.nfcFolderSettings.folder, true);
    uint8_t special2 = voiceMenu(mp3.getFolderTrackCount(tempCard.nfcFolderSettings.folder), 322, 0,
                                 true, tempCard.nfcFolderSettings.folder, special, true);

    mp3.playMp3FolderTrack(936);
    waitForTrackToFinish();
    for (uint8_t x = special; x <= special2; x++) {
      mp3.playMp3FolderTrack(x);
      tempCard.nfcFolderSettings.special = x;
#if defined DEBUG
      Serial.print(x);
      Serial.println(F(" place Tag"));
#endif
      do {
        readTrigger();
        if (myTrigger.shutDown) {
#if defined DEBUG
          Serial.println(F("abort!"));
#endif
          mp3.playMp3FolderTrack(802);
          return;
        }
      } while (!mfrc522.PICC_IsNewCardPresent());

      // RFID Karte wurde aufgelegt
      if (mfrc522.PICC_ReadCardSerial()) {
#if defined DEBUG
        Serial.println(F("write Tag"));
#endif
        writeCard(tempCard);
        delay(50);
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        waitForTrackToFinish();
      }
    }
  }
  else if (subMenu == InvertButtons) {
    // Invert Functions for Up/Down Buttons
    switch (voiceMenu(2, 933, 933, false)) {
      case 1:
        mySettings.invertVolumeButtons = false;
        break;
      case 2:
        mySettings.invertVolumeButtons = true;
        break;
    }
  }
  else if (subMenu == StopWhenCardAway) {
    switch (voiceMenu(2, 937, 933, false)) {
      case 1:
        mySettings.stopWhenCardAway = false;
        break;
      case 2:
        mySettings.stopWhenCardAway = true;
        break;
    }
  }
  else if (subMenu == SetupAnalogInputLeraning) {
#if defined ANALOG_INPUT
#if defined ANALOG_INPUT_PRINT
    Serial.println(F("--- learning analog input ---"));
#endif
    mp3Pause();
    mp3.playMp3FolderTrack(828);
    waitForTrackToFinish();

    bool exitWhile = true;
    uint8_t functionNr = 0;
    uint32_t currentAnalogValue = 0;
    uint16_t newAnalogValue = 0;
    uint16_t upperTolerance = 0;
    uint16_t lowerTolerance = 0;
    static const uint8_t minValue = 3;
    static const uint8_t analogValuesCount = 15;
    unsigned long millisTriggerTime = 0;

    do {
      mp3.loop();
      functionNr = voiceMenu((inputTriggerCount - availableShortCuts)+1, 834, 834, false, false, 0);
      if (functionNr == 0) {
        exitWhile = false;
        break;
      } else if (functionNr >= (inputTriggerCount - availableShortCuts)+1) {
        functionNr += voiceMenu(availableShortCuts, 940, 0) - 1;
        if (functionNr < availableShortCuts) {
          exitWhile = false;
          break;
        }
      }
#if defined ANALOG_INPUT_PRINT
      Serial.print(F("setup position no: "));
      Serial.println(functionNr);
#endif
      mySettings.analogInUserValues[functionNr] = 0;
      //Mittelwert des Leerlaufs bilden
      for (uint8_t j = 0; j < analogValuesCount ; j++) {
        currentAnalogValue += analogRead(ANALOG_INPUT_PIN);
      }
      currentAnalogValue = currentAnalogValue / analogValuesCount;      
      currentAnalogValue = max(currentAnalogValue, minValue);

      upperTolerance = (currentAnalogValue + ((currentAnalogValue * mySettings.analogInputTolerance) / 100));
      upperTolerance = max(upperTolerance, minValue + 1);
      lowerTolerance = (currentAnalogValue - ((currentAnalogValue * mySettings.analogInputTolerance) / 100));
      if (lowerTolerance <= 3) {
        lowerTolerance = 0;
      }

#if defined ANALOG_INPUT_PRINT
      Serial.print(F("analog input compare value: "));
      Serial.println(currentAnalogValue);
      Serial.print(F("lower tolerance: "));
      Serial.println(lowerTolerance);
      Serial.print(F("upper tolerance:"));
      Serial.println(upperTolerance);
#endif
      mp3Pause();
      mp3.playMp3FolderTrack(829);
      waitForTrackToStart();

#if defined ANALOG_INPUT_PRINT
      Serial.println(F("wait for user..."));
#endif
      millisTriggerTime = millis();
      while (millis() - millisTriggerTime <= mySettings.analogInputTriggerTime/*currentAnalogValue >= lowerTolerance && currentAnalogValue <= upperTolerance*/) {
        currentAnalogValue = analogRead(ANALOG_INPUT_PIN);
        currentAnalogValue = max(currentAnalogValue, minValue);
        if (currentAnalogValue >= lowerTolerance && currentAnalogValue <= upperTolerance) {
          millisTriggerTime = millis();
        }
        readTrigger();
        if (myTrigger.shutDown) {
          break;
        }
        resetTrigger();
      };

      if (!myTrigger.shutDown) {

#if defined ANALOG_INPUT_PRINT
        Serial.println(F("input detected, measure average value"));
        Serial.println(F("!!hold the input!!"));
#endif
        currentAnalogValue = 0;
        //Mittelwert des Leerlaufs bilden
        for (uint8_t j = 0; j < analogValuesCount ; j++) {
          currentAnalogValue += analogRead(ANALOG_INPUT_PIN);
        }
        newAnalogValue = currentAnalogValue / analogValuesCount;

        resetTrigger();
        mp3Pause();

        if (mySettings.analogInputTriggerType != Permament) {
          mp3.playMp3FolderTrack(830);
          waitForTrackToStart();

#if defined ANALOG_INPUT_PRINT
          Serial.println(F("wait for release..."));
#endif
          while (currentAnalogValue < lowerTolerance || currentAnalogValue > upperTolerance) {
            readTrigger();
            currentAnalogValue = analogRead(ANALOG_INPUT_PIN);
            currentAnalogValue = max(currentAnalogValue, minValue);
            if (myTrigger.shutDown) {
              break;
            }
            resetTrigger();
          };
#if defined ANALOG_INPUT_PRINT
          Serial.println(F("input released"));
#endif
        }
        if (!myTrigger.shutDown) {
          mySettings.analogInUserValues[functionNr] = newAnalogValue;

#if defined ANALOG_INPUT_PRINT
          Serial.print(F("finish, new analog input value: "));
          Serial.println(mySettings.analogInUserValues[functionNr]);
#endif
          mp3Pause();
          mp3.playMp3FolderTrack(400);
          waitForTrackToFinish();
        }
      }
      resetTrigger();
    } while (exitWhile);

#if defined ANALOG_INPUT_PRINT
    Serial.println(F("--- end learning ---"));
#endif

#else
    mp3.playMp3FolderTrack(827);
    waitForTrackToFinish();
#endif
  }
  else if (subMenu == SetupAnalogInputType) {
#if defined ANALOG_INPUT
    mp3Pause();
    mp3.playMp3FolderTrack(847);
    waitForTrackToFinish();

    mySettings.analogInputTriggerType = voiceMenu(2, 848, 848);
    switch (mySettings.analogInputTriggerType) {
      case NoType:
        break;
      case Impuls:
        mySettings.analogInputTriggerTime = 35;
        break;
      case Permament:
        mySettings.analogInputTriggerTime = 1500;
        break;
    }
#else
    mp3.playMp3FolderTrack(827);
    waitForTrackToFinish();
#endif
  }
  else if (subMenu == SetupIRRemote) {
#if defined IRREMOTE
    mp3Pause();
    mp3.playMp3FolderTrack(832);
    waitForTrackToFinish();
    bool exitWhile = true;
    uint8_t functionNr = 0;
    do {
      mp3.loop();
      functionNr = voiceMenu(ShortcutTrigger, 834, 834, false, false, 0);
      if (functionNr == 0) {
        exitWhile = false;
        break;
      } else if (functionNr >= ShortcutTrigger) {
        functionNr += voiceMenu(sizeof(myTrigger) - ShortcutTrigger, 940, 0) - 1;
        Serial.println(sizeof(myTrigger) - ShortcutTrigger);
        if (functionNr < (sizeof(myTrigger) - ShortcutTrigger)) {
          exitWhile = false;
          break;
        }
      }
      mp3Pause();
      mp3.playMp3FolderTrack(833);
      // clear ir receive buffer
      irReceiver.resume();
      // wait for ir signal
      while (!irReceiver.decode(&irReading)) {
      }
      mp3Pause();
      // on NEC encoding 0xFFFFFFFF means the button is held down, we ignore this
      if (!(irReading.decode_type == NEC && irReading.value == 0xFFFFFFFF)) {
        // convert irReading.value from 32bit to 16bit
        uint16_t irRemoteCode = (irReading.value & 0xFFFF);
#if defined DEBUG
        Serial.print(F("ir code: 0x"));
        Serial.print(irRemoteCode <= 0x0010 ? "0" : "");
        Serial.print(irRemoteCode <= 0x0100 ? "0" : "");
        Serial.print(irRemoteCode <= 0x1000 ? "0" : "");
        Serial.println(irRemoteCode, HEX);
#endif
        mySettings.irRemoteUserCodes[functionNr - 1] = irRemoteCode;
        irReceiver.resume();

      }
    } while (exitWhile);

#else
    mp3.playMp3FolderTrack(831);
    waitForTrackToFinish();
#endif
  }
  else if (subMenu == ResetEEPROM) {
#if defined AiO
    for (int i = 0; i < EEPROM_size; i++) {
      EEPROM_update(i, 0);
    }
#else
    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.update(i, 0);
    }
#endif
    resetSettings();
    mp3.playMp3FolderTrack(999);
  }
  else if (subMenu == LockAdminMenu) {
    switch (voiceMenu(2, 980, 980)) {
      case 1:
        mySettings.adminMenuLocked = 0;
        break;
      case 2:
        mySettings.adminMenuLocked = 1;
        break;
    }
  }
  resetTrigger();
  writeSettingsToFlash();
  setstandbyTimer();
}
//////////////////////////////////////////////////////////////////////////
uint8_t voiceMenu(int16_t numberOfOptions, uint16_t startMessage, int messageOffset,
                  bool preview = false, uint8_t previewFromFolder = 0, int defaultValue = 0,
                  bool enableSkipTen = false) {
  int16_t returnValue = -1;
  int16_t currentValue = defaultValue;
  int16_t currentValueOld = currentValue;


  mp3Pause();

  resetTrigger();
  resetTriggerEnable();

  if (startMessage != 0) {
    mp3.playMp3FolderTrack(startMessage);
    waitForTrackToStart();
  }

#if defined DEBUG
  Serial.print(F("voiceMenu "));
  Serial.print(numberOfOptions);
  Serial.println(F(" Options"));
#endif

  do {
    readTrigger();
    mp3.loop();

    if (myTrigger.nextPlusTen && numberOfOptions > 10 && enableSkipTen) {
      myTriggerEnable.nextPlusTen = false;
      currentValue += 10;
      if (currentValue == 0) {
        currentValue = 1;
      }
      myTriggerEnable.next = true;
    } else if (myTrigger.next) {
      myTriggerEnable.next = false;
      //if (!myTriggerEnable.next) {
      currentValue++;
      if (currentValue == 0) {
        currentValue = 1;
      }
    }
    if (currentValue > numberOfOptions) {
      currentValue = currentValue - numberOfOptions;
    }

    if (myTrigger.previousPlusTen && numberOfOptions > 10 && enableSkipTen) {
      myTriggerEnable.previousPlusTen = false;
      currentValue -= 10;
      myTriggerEnable.previous = true;
      if (currentValue == 0) {
        currentValue = -1;
      }
    } else if (myTrigger.previous) {
      myTriggerEnable.previous = false;
      //if (!myTriggerEnable.previous) {
      currentValue--;
      if (currentValue == 0) {
        currentValue = -1;
      }
    }
    if (currentValue == -1) {
      currentValue = numberOfOptions;
    } else if (currentValue < -1) {
      currentValue = numberOfOptions + currentValue;
    }

    if (currentValue != currentValueOld) {
      currentValueOld = currentValue;
#if defined DEBUG
      Serial.println(currentValue);
#endif
      mp3.playMp3FolderTrack(messageOffset + currentValue);
      waitForTrackToFinish();
      if (preview) {        
        waitForTrackToStart();
        if (previewFromFolder == 0) {
          mp3.playFolderTrack(currentValue, 1);
        } else {
          mp3.playFolderTrack(previewFromFolder, currentValue);
        }
      }
      resetTrigger();
    }

    if (myTrigger.shutDown) {
      mp3.playMp3FolderTrack(802);
      waitForTrackToFinish();
      myTriggerEnable.pauseTrack = true;
      checkStandbyAtMillis();
      returnValue = defaultValue;
    }
    if (myTrigger.pauseTrack) {
      if (currentValue != 0) {
        returnValue = currentValue;
      }
    }

#if defined DEBUG
    if (Serial.available() > 0) {
      int optionSerial = Serial.parseInt();
      if (optionSerial != 0 && optionSerial <= numberOfOptions)
        returnValue = optionSerial;
    }
#endif

    resetTrigger();
  } while (returnValue == -1);

#if defined DEBUG
  Serial.print(F("= "));
  Serial.println(returnValue);
#endif
  mp3Pause();
  return returnValue;

}
//////////////////////////////////////////////////////////////////////////
bool setupFolder(folderSettings * theFolder) {
  uint8_t enableMemory = 0;
  // Ordner abfragen
  theFolder->folder = voiceMenu(99, 301, 0, true, 0, 0, true);
  if (theFolder->folder == 0) return false;

  // Wiedergabemodus abfragen
  theFolder->mode = voiceMenu(11, 305, 305);
  if (theFolder->mode == 0) return false;
#if defined DEBUG
  Serial.println(F("Setup folder"));
  Serial.print(F("Folder: "));
  Serial.println(theFolder->folder);
  Serial.print(F("Mode: "));
  Serial.println(theFolder->mode);
#endif
  //  // Hörbuchmodus > Fortschritt im EEPROM auf 1 setzen
  //  writeCardMemory (myFolder->folder,myFolder->special3, 1);

  switch (theFolder->mode) {
    case Single:
#if defined DEBUG
      Serial.println(F("Single"));
#endif
      //Einzeltrack in special speichern
      theFolder->special = voiceMenu(mp3.getFolderTrackCount(theFolder->folder), 320, 0,
                                     true, theFolder->folder, 0, true);
      break;
    case AudioDrama_Section:
    case Album_Section:
    case Party_Section:
    case AudioBook_Section:
#if defined DEBUG
      Serial.println(F("Section"));
#endif
      //Von (special), Bis (special2) speichern
      theFolder->special = voiceMenu(mp3.getFolderTrackCount(theFolder->folder), 321, 0,
                                     true, theFolder->folder, true);
      theFolder->special2 = voiceMenu(mp3.getFolderTrackCount(theFolder->folder), 322, 0,
                                      true, theFolder->folder, theFolder->special, true);

    case AudioBook:
    case Album:
#if defined DEBUG
      Serial.println(F("Memory"));
#endif
      //Speicherplatz für Alben? Ja/Nein
      if (theFolder->mode == Album || theFolder->mode == Album_Section)
        enableMemory = voiceMenu(2, 978, 933, false, false, 0) - 1;
      //Speicherplatz wählen
      if (theFolder->mode == AudioBook || theFolder->mode == AudioBook_Section || enableMemory == 1)
        theFolder->special3 = voiceMenu(folderMemoryCount, 325, 0);
      else
        theFolder->special3 = 0;
      break;
    case AdminMenu:
#if defined DEBUG
      Serial.println(F("AdminMenu"));
#endif
      theFolder->folder = 0;
      theFolder->mode = 255;
      break;
    case PuzzlePart:
#if defined DEBUG
      Serial.println(F("PuzzlePart"));
#endif
      theFolder->special = voiceMenu(mp3.getFolderTrackCount(theFolder->folder), 323, 0,
                                     true, theFolder->folder, 0, true);
      theFolder->special2 = voiceMenu(255, 324, 0,
                                      false, theFolder->folder, 0, true);
      break;
    default:
      break;
  }
#if defined DEBUG
  Serial.println(F("Setup folder ok"));
#endif
  return true;
}
//////////////////////////////////////////////////////////////////////////
void setupShortCut(uint8_t shortCutNumber = 0) {
  bool returnValue = true;
  mp3Pause();
#if defined DEBUG
  Serial.println(F("setupShortCut"));
#endif
  if (shortCutNumber == 0) {
  shortCutNumber = voiceMenu(availableShortCuts, 940, 0);
  }
  switch (voiceMenu(2, 941, 941)) {
    case 1:
      returnValue &= setupFolder(&mySettings.shortCuts[shortCutNumber - 1]);
      break;
    case 2:
      returnValue &= setupModifier(&mySettings.shortCuts[shortCutNumber - 1]);
      break;
    default:
      returnValue &= false;
      break;
  }
  if (returnValue) {
    mp3.playMp3FolderTrack(400);
    waitForTrackToStart();
    writeSettingsToFlash();
  }
}

void setupCard() {  
  bool returnValue = true;
  mp3Pause();
#if defined DEBUG
  Serial.println(F("setupCard"));
#endif
  nfcTagObject newCard;

  switch (voiceMenu(2, 941, 941)) {
    case 1:
      returnValue &= setupFolder(&newCard.nfcFolderSettings);
      break;
    case 2:
      returnValue &= setupModifier(&newCard.nfcFolderSettings);
      break;
    default:
      returnValue &= false;
      break;
  }
  if (returnValue) {
    // Karte ist konfiguriert > speichern
    mp3Pause();
    forgetLastCard = true;
    writeCard(newCard);
  }
}

bool readCard(nfcTagObject * nfcTag) {
  nfcTagObject tempCard;
  // Show some details of the PICC (that is: the tag/card)

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

#if defined DEBUG
  Serial.println(F("Card UID "));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.print(F("PICC type "));
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
#endif

  byte buffer[18];
  byte size = sizeof(buffer);

  // Authenticate using key A
  if ((piccType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
#if defined DEBUG
    Serial.println(F("Authenticating Classic using key A..."));
    dump_byte_array(key.keyByte, 6);
#endif
    status = mfrc522.PCD_Authenticate(
               MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    Serial.println(status);
  }
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the tempCard

    // Authenticate using key A
#if defined DEBUG
    Serial.println(F("Authenticating MIFARE UL"));
#endif
    status = mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
  }

  if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
    Serial.print(F("PCD_Authenticate failed "));
    Serial.println(mfrc522.GetStatusCodeName(status));
#endif
    return false;
  }

  // Show the whole sector as it currently is
  // Serial.println(F("Current data in sector:"));
  // mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  // Serial.println();

  // Read data from the block
  if ((piccType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
#if defined DEBUG
    Serial.print(F("Reading data block"));
    Serial.println(blockAddr);
#endif
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
      Serial.print(F("MIFARE_Read failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
  }
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte buffer2[18];
    byte size2 = sizeof(buffer2);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(8, buffer2, &size2);
    if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
      Serial.print(F("MIFARE_Read_1 failed "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer, buffer2, 4);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(9, buffer2, &size2);
    if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
      Serial.print(F("MIFARE_Read_2 failed "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer + 4, buffer2, 4);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(10, buffer2, &size2);
    if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
      Serial.print(F("MIFARE_Read_3 failed "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer + 8, buffer2, 4);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(11, buffer2, &size2);
    if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
      Serial.print(F("MIFARE_Read_4 failed "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer + 12, buffer2, 4);
  }

#if defined DEBUG
  Serial.print(F("Data on Card: "));
  dump_byte_array(buffer, 16);
#endif
  uint32_t tempCookie;
  tempCookie = (uint32_t)buffer[0] << 24;
  tempCookie += (uint32_t)buffer[1] << 16;
  tempCookie += (uint32_t)buffer[2] << 8;
  tempCookie += (uint32_t)buffer[3];

  tempCard.cookie = tempCookie;
  tempCard.version = buffer[4];
  tempCard.nfcFolderSettings.folder = buffer[5];
  tempCard.nfcFolderSettings.mode = buffer[6];
  tempCard.nfcFolderSettings.special = buffer[7];
  tempCard.nfcFolderSettings.special2 = buffer[8];
  tempCard.nfcFolderSettings.special3 = buffer[9];
  tempCard.nfcFolderSettings.special4 = buffer[10];

  if (tempCard.cookie == cardCookie) {

    if (activeModifier != NULL && tempCard.nfcFolderSettings.folder != 0) {
      if (activeModifier->handleRFID(&tempCard) == true) {
#if defined DEBUG
        Serial.println(F("RFID locked"));
#endif
        return false;
      }
    }
    if (tempCard.nfcFolderSettings.folder == 0)
      return SetModifier (&tempCard.nfcFolderSettings);
    else {
      memcpy(nfcTag, &tempCard, sizeof(nfcTagObject));
#if defined DEBUG
      Serial.println( nfcTag->nfcFolderSettings.folder);
#endif
      myFolder = &nfcTag->nfcFolderSettings;
#if defined DEBUG
      Serial.println( myFolder->folder);
#endif
    }
    return true;
  }
  else {
    memcpy(nfcTag, &tempCard, sizeof(nfcTagObject));
    return true;
  }
}

void resetCard() {
  mp3.playMp3FolderTrack(800);
  do {
    readTrigger();
    if (myTrigger.shutDown) {
#if defined DEBUG
      Serial.print(F("Abgebrochen!"));
#endif
      mp3.playMp3FolderTrack(802);
      return;
    }
  } while (!mfrc522.PICC_IsNewCardPresent());

  if (!mfrc522.PICC_ReadCardSerial())
    return;

#if defined DEBUG
  Serial.print(F("reset Tag"));
#endif
  setupCard();
}
//////////////////////////////////////////////////////////////////////////
void writeCard(nfcTagObject nfcTag, int8_t writeBlock = -1, bool feedback = true) {
  MFRC522::PICC_Type mifareType;
  byte buffer[16] = {0x13, 0x37, 0xb3, 0x47, // 0x1337 0xb347 magic cookie to
                     // identify our nfc tags
                     0x02,                   // version 1
                     nfcTag.nfcFolderSettings.folder,          // the folder picked by the user
                     nfcTag.nfcFolderSettings.mode,    // the playback mode picked by the user
                     nfcTag.nfcFolderSettings.special, // track or function for admin cards
                     nfcTag.nfcFolderSettings.special2,
                     nfcTag.nfcFolderSettings.special3,
                     nfcTag.nfcFolderSettings.special4,
                     0x00, 0x00, 0x00, 0x00, 0x00
                    };

  byte size = sizeof(buffer);

#if defined DEBUG
  Serial.println(F("write Card"));
#endif
  mifareType = mfrc522.PICC_GetType(mfrc522.uid.sak);

  // Authenticate using key B
  //authentificate with the card and set card specific parameters
  if ((mifareType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
#if defined DEBUG
    Serial.println(F("Authenticating again using key A..."));
#endif
    status = mfrc522.PCD_Authenticate(
               MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  }
  else if (mifareType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the NFCtag

    // Authenticate using key A
#if defined DEBUG
    Serial.println(F("Authenticating UL..."));
#endif
    status = mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
  }

  if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
    Serial.print(F("PCD_Authenticate failed "));
    Serial.println(mfrc522.GetStatusCodeName(status));
#endif
    if (feedback) {
      mp3.playMp3FolderTrack(401);
      waitForTrackToFinish();
    }
    return;
  }

  // Write data to the block
#if defined DEBUG
  Serial.print(F("Writing data "));
  Serial.print(blockAddr);
  Serial.println(F("..."));
  dump_byte_array(buffer, 16);
  Serial.println();
#endif

  if ((mifareType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(blockAddr, buffer, 16);
  }
  else if (mifareType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte buffer2[16];
    byte size2 = sizeof(buffer2);
    if (writeBlock == -1) {
      memset(buffer2, 0, size2);
      memcpy(buffer2, buffer, 4);
      status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(8, buffer2, 16);

      memset(buffer2, 0, size2);
      memcpy(buffer2, buffer + 4, 4);
      status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(9, buffer2, 16);

      memset(buffer2, 0, size2);
      memcpy(buffer2, buffer + 8, 4);
      status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(10, buffer2, 16);

      memset(buffer2, 0, size2);
      memcpy(buffer2, buffer + 12, 4);
      status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(11, buffer2, 16);
    } else if (writeBlock <= 11 && writeBlock >= 8) {
      memset(buffer2, 0, size2);
      memcpy(buffer2, buffer, 4);
      status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(writeBlock, buffer2, 16);
    }
  }

  if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
    Serial.print(F("MIFARE_Write failed "));
    Serial.println(mfrc522.GetStatusCodeName(status));
#endif
    if (feedback) {
      mp3.playMp3FolderTrack(401);
    }
  }
  else if (feedback) {
    mp3.playMp3FolderTrack(400);
    waitForTrackToFinish();
  }
#if defined DEBUG
  Serial.println();
#endif
  //delay(1000);
}
//////////////////////////////////////////////////////////////////////////
/**
  Helper routine to dump a byte array as hex values to Serial.
*/
#if defined DEBUG
void dump_byte_array(byte * buffer, byte bufferSize) {

  for (byte i = 0; i < bufferSize; i++) {

    Serial.print(buffer[i] < 0x10 ? "0" : " ");
    Serial.print(buffer[i], HEX );
    Serial.print(" ");

  }
  Serial.println();
}
#endif
///////////////////////////////////////// Check Bytes   ///////////////////////////////////
/*bool checkTwo ( uint8_t a[], uint8_t b[] ) {
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
      return false;
    }
  }
  return true;
  }*/
//////////////////////////////////////////////////////////////////////////

bool setupModifier(folderSettings * tmpFolderSettings) {

  tmpFolderSettings->folder = ModifierMode;
  tmpFolderSettings->mode = voiceMenu(12, 966, 966, false, 0, 0, true);

  if (tmpFolderSettings->mode != ModifierMode) {
    if (tmpFolderSettings->mode == SleepTimerMod) {
      switch (voiceMenu(4, 960, 960)) {
        case 1: tmpFolderSettings->special = 5; break;
        case 2: tmpFolderSettings->special = 15; break;
        case 3: tmpFolderSettings->special = 30; break;
        case 4: tmpFolderSettings->special = 60; break;
      }
      tmpFolderSettings->special2 = 0x00;
    } else if (tmpFolderSettings->mode == PuzzleGameMod) {
      //Save first part?
      tmpFolderSettings->special = voiceMenu(2, 979, 933) - 1;
      tmpFolderSettings->special2 = 0x00;
    }
    else if (tmpFolderSettings->mode == QuizGameMod) {
      //Set Folder
      tmpFolderSettings->special =  voiceMenu(99, 301, 0, true, 0, 0, true);
      tmpFolderSettings->special2 = 0x00;
    }
    else if (tmpFolderSettings->mode == ButtonSmashMod) {
      //Set Folder
      tmpFolderSettings->special =  voiceMenu(99, 301, 0, true, 0, 0, true);
      //Set Volume
      tmpFolderSettings->special2 =  voiceMenu(30, 904, false, false, 0);
    }
    else if (tmpFolderSettings->mode == CalculateMod) {
      //Set Oprator
      tmpFolderSettings->special =  voiceMenu(5, 423, 423, false, 0, 0, true);
      //Set max number
      tmpFolderSettings->special2 =  voiceMenu(255, 429, 0, false, 0, 0, true);
    }

    //Save Modifier in EEPROM?
    tmpFolderSettings->special3 = voiceMenu(2, 980, 933, false, false, 0) - 1;
    tmpFolderSettings->special4 = 0x00;
    return true;
  }
  return false;
}

bool SetModifier (folderSettings * tmpFolderSettings) {
  if (activeModifier != NULL) {
    if (activeModifier->getActive() == tmpFolderSettings->mode) {
      return RemoveModifier();
    }
  }

#if defined DEBUG
  Serial.print(F("set modifier: "));
  Serial.println(tmpFolderSettings->mode);
#endif

  if (tmpFolderSettings->mode != 0 && tmpFolderSettings->mode != AdminMenuMod) {
    if (isPlaying()) {
      mp3.playAdvertisement(260);
    }
    else {
      mp3.playMp3FolderTrack(260);
      waitForTrackToFinish();
    }
  }

  switch (tmpFolderSettings->mode) {
    case 0:
    case AdminMenuMod:
      mfrc522.PICC_HaltA(); mfrc522.PCD_StopCrypto1(); adminMenu(true);  break;
    case SleepTimerMod: activeModifier = new SleepTimer(tmpFolderSettings->special); break;
    case FreezeDanceMod: activeModifier = new FreezeDance(); break;
    case LockedMod: activeModifier = new Locked(); break;
    case ToddlerModeMod: activeModifier = new ToddlerMode(); break;
    case KindergardenModeMod: activeModifier = new KindergardenMode(); break;
    case RepeatSingleMod: activeModifier = new RepeatSingleModifier(); break;
    case PuzzleGameMod: activeModifier = new PuzzleGame(tmpFolderSettings->special); break;
    case QuizGameMod: activeModifier = new QuizGame(tmpFolderSettings->special); break;
    case ButtonSmashMod: activeModifier = new ButtonSmash(tmpFolderSettings->special, tmpFolderSettings->special2); break;
    //case TheQuestMod: activeModifier = new TheQuest(tmpFolderSettings->special, tmpFolderSettings->special2); break;
    case CalculateMod: activeModifier = new Calculate (tmpFolderSettings->special, tmpFolderSettings->special2); break;
  }
  if (tmpFolderSettings->special3 == 1) {
    mySettings.savedModifier = *tmpFolderSettings;
    writeSettingsToFlash();
  }
  return false;
}

bool RemoveModifier() {
   delete activeModifier;
  activeModifier = NULL;

  _lastTrackFinished = 0;

  mySettings.savedModifier.folder = 0;
  mySettings.savedModifier.mode = 0;
  writeSettingsToFlash();

#if defined DEBUG
  Serial.println(F("modifier removed"));
#endif
  if (isPlaying()) {
    mp3.playAdvertisement(261);
  }
  else {
    mp3Pause();
    mp3.playMp3FolderTrack(261);
    waitForTrackToFinish();
  }
  return false;
}

void writeCardMemory (uint8_t track) {
  myCard.nfcFolderSettings.special3 = track;
  writeCard(myCard, 10, false);
}

//Um festzustellen ob eine Karte entfernt wurde, muss der MFRC regelmäßig ausgelesen werden
byte pollCard() {
  const byte maxRetries = 2;

  if (!hasCard)  {
    if (mfrc522.PICC_IsNewCardPresent()) {
      if (mfrc522.PICC_ReadCardSerial()) {
#if defined DEBUG
        Serial.println(F("ReadCardSerial finished"));
#endif
        if (readCard(&myCard))  {
          bool bSameUID = !memcmp(lastCardUid, mfrc522.uid.uidByte, 4);
          // store info about current card
          memcpy(lastCardUid, mfrc522.uid.uidByte, 4);
          lastCardWasUL = mfrc522.PICC_GetType(mfrc522.uid.sak) == MFRC522::PICC_TYPE_MIFARE_UL;

          retries = maxRetries;
          hasCard = true;
          return bSameUID ? PCS_CARD_IS_BACK : PCS_NEW_CARD;
        }
        else {//readCard war nicht erfolgreich
          mfrc522.PICC_HaltA();
          mfrc522.PCD_StopCrypto1();
#if defined DEBUG
          Serial.println(F("cant read card"));
#endif
        }
      }
    }
    return PCS_NO_CHANGE;
  }
  else // hasCard
  {
    // perform a dummy read command just to see whether the card is in range
    byte buffer[18];
    byte size = sizeof(buffer);

    if (mfrc522.MIFARE_Read(lastCardWasUL ? 8 : blockAddr, buffer, &size) != MFRC522::STATUS_OK) {
      if (retries > 0)
        retries--;
      else {
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        hasCard = false;
        return PCS_CARD_GONE;
      }
    }
    else
      retries = maxRetries;
  }
  return PCS_NO_CHANGE;
}

void handleCardReader() {
  bool tmpStopWhenCardAway = true;

  // poll card only every 100ms
  static unsigned long lastCardPoll = 0;
  unsigned long now = millis();

  if (static_cast<unsigned long>(now - lastCardPoll) > 100)  {
    if (activeModifier != NULL)
      tmpStopWhenCardAway = mySettings.stopWhenCardAway && !activeModifier->handleStopWhenCardAway();
    else
      tmpStopWhenCardAway = mySettings.stopWhenCardAway;

    lastCardPoll = now;
    switch (pollCard()) {
      case PCS_NEW_CARD:
#if defined DEBUG
        Serial.println(F("new card"));
#endif
        onNewCard();
        break;
      case PCS_CARD_GONE:
#if defined DEBUG
        Serial.println(F("card gone"));
#endif
        if (tmpStopWhenCardAway) {
          knownCard = false;
          mp3Pause();
          setstandbyTimer();
        }
        break;
      case PCS_CARD_IS_BACK:
#if defined DEBUG
        Serial.println(F("same card"));
#endif
        if (tmpStopWhenCardAway) {

          //nur weiterspielen wenn vorher nicht konfiguriert wurde
          if (!forgetLastCard) {
            knownCard = true;
            mp3.start();
            disablestandbyTimer();
          }
          else
            onNewCard();
        } else {
          onNewCard();
        }
        break;
    }
  }
}

void onNewCard() {
  forgetLastCard = false;
  //make random a little bit more "random"
  //randomSeed(millis() + random(1000));
  if (myCard.cookie == cardCookie && myCard.nfcFolderSettings.folder != 0 && myCard.nfcFolderSettings.mode != 0) {
    Serial.println(F("onNewCard()->playFolder"));
    playFolder();
  }

  // Neue Karte konfigurieren
  else if (myCard.cookie != cardCookie) {
    mp3.playMp3FolderTrack(300);
    waitForTrackToFinish();
    setupCard();
  }
}

/// Funktionen für den Standby Timer (z.B. über Pololu-Switch oder Mosfet)

void setstandbyTimer() {
#if defined DEBUG
  Serial.println(F("set standby timer"));
#endif
#if defined AiO
  // verstärker aus
  digitalWrite(8, HIGH);
  delay(100);
#else if defined SPEAKER_SWITCH
  digitalWrite(SpeakerOnPin, LOW);
  delay(100);
#endif
  if (mySettings.standbyTimer != 0)
    sleepAtMillis = millis() + (mySettings.standbyTimer * 60 * 1000);
  else
    sleepAtMillis = 0;
#if defined DEBUG
  Serial.print(F("milis: "));
  Serial.println(sleepAtMillis);
#endif
}
//////////////////////////////////////////////////////////////////////////
void disablestandbyTimer() {
#if defined DEBUG
  Serial.println(F("disable standby timer"));
#endif
#if defined AiO
  // verstärker an
  digitalWrite(8, LOW);
  delay(100);
#else if defined SPEAKER_SWITCH
  digitalWrite(SpeakerOnPin, HIGH);
  delay(100);
#endif
  sleepAtMillis = 0;
}
//////////////////////////////////////////////////////////////////////////
void checkStandbyAtMillis() {
  if (sleepAtMillis != 0 && millis() > sleepAtMillis) {
#if defined DEBUG
    Serial.println(F("standby active"));
#endif
    shutDown();
  }
}
//////////////////////////////////////////////////////////////////////////
void shutDown() {
  if (activeModifier != NULL) {
    if (activeModifier->handleShutDown() == true) {
#if defined DEBUG
      Serial.println(F("Shut down locked"));
#endif
      return;
    }
  }

#if defined DEBUG
  Serial.println("Shut Down");
#endif

  mp3Pause();

#if defined AiO
  // verstärker an
  digitalWrite(8, LOW);

#else if defined SPEAKER_SWITCH
  digitalWrite(SpeakerOnPin, HIGH);
#endif

#if defined POWER_ON_LED
  digitalWrite(PowerOnLEDPin, LOW);
#endif

  delay(100);

  if (volume > mySettings.initVolume)
    volume = mySettings.initVolume;

  mp3.setVolume(volume);
  delay(100);

  knownCard = false;
  mp3.playMp3FolderTrack(265);
  waitForTrackToFinish();

#if defined AiO
  // verstärker aus
  digitalWrite(8, HIGH);

#else if defined SPEAKER_SWITCH
  digitalWrite(SpeakerOnPin, LOW);
#endif

#if not defined AiO
  digitalWrite(shutdownPin, HIGH);

  // enter sleep state
  // http://discourse.voss.earth/t/intenso-s10000-powerbank-automatische-abschaltung-software-only/805
  // powerdown to 27mA (powerbank switches off after 30-60s)
  mfrc522.PCD_AntennaOff();
  mfrc522.PCD_SoftPowerDown();
  mp3.sleep();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();  // Disable interrupts
  sleep_mode();
#else
  digitalWrite(shutdownPin, LOW);
#endif
}

//////////////////////////////////////////////////////////////////////////
#if defined FADING_LED
// fade in/out status led while beeing idle, during playback set to full brightness
void fadeStatusLed(bool isPlaying) {
  static bool statusLedDirection = false;
  static int8_t statusLedValue = 255;
  static unsigned long statusLedOldMillis;
  static int8_t statusLedDeltaValuePause = 500;
  static int8_t statusLedDeltaValuePlay = 1;
  static int8_t statusLedDeltaValue = 10;

  if (isPlaying) {
    statusLedDeltaValue = statusLedDeltaValuePlay;
  }
  else {
    statusLedDeltaValue = statusLedDeltaValuePause;
  }

  if ((millis() - statusLedOldMillis) >= 100) {
    statusLedOldMillis = millis();
    if (statusLedDirection) {
      statusLedValue += statusLedDeltaValue;
      if (statusLedValue >= 255) {
        statusLedValue = 255;
        statusLedDirection = !statusLedDirection;
      }
    }
    else {
      statusLedValue -= statusLedDeltaValue;
      if (statusLedValue <= 0) {
        statusLedValue = 0;
        statusLedDirection = !statusLedDirection;
      }
    }
    analogWrite(POWER_ON_LED_PIN, statusLedValue);
  }
}
#endif
//////////////////////////////////////////////////////////////////////////
