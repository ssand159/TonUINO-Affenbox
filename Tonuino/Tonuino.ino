#include <DFMiniMp3.h>
#include <EEPROM.h>
#include <JC_Button.h>
#include <MFRC522.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <avr/sleep.h>
#include <TimerOne.h>
#include <ClickEncoder.h>
/*
   _____         _____ _____ _____ _____
  |_   _|___ ___|  |  |     |   | |     |
    | | | . |   |  |  |-   -| | | |  |  |
    |_| |___|_|_|_____|_____|_|___|_____|
    TonUINO Version 2.1

    created by Thorsten Voß and licensed under GNU/GPL.
    Information and contribution at https://tonuino.de.
    Fork by Marco Schulz
*/

///////// uncomment the below line to enable the function ////////////////
//#define FIVEBUTTONS
//#define CHECK_BATTERY
#define DEBUG
//#define PUSH_ON_OFF
//#define STARTUP_SOUND
//#define SPEAKER_SWITCH
//#define ROTARY_ENCODER
//#define ROTARY_SWITCH
//////////////////////////////////////////////////////////////////////////

///////// conifguration of the input and output pin //////////////////////
#define buttonPause A0 //Default A0; 
#define buttonUp A1 //Default A1; 
#define buttonDown A2 //Default A2; 
#define busyPin 4

#define shutdownPin 7 //Default 7

#define openAnalogPin A7 //Default A7

#ifdef FIVEBUTTONS
#define buttonFourPin A3
#define buttonFivePin A4
#endif

#define LONG_PRESS 1000
#define LONGER_PRESS 2000

#ifdef SPEAKER_SWITCH
#define SpeakerOnPin 8
#endif

//#ifdef CHECK_BATTERY
//#define LEDgreenPin 6
//#define LEDredPin 5
//#define batMeasPin A7
//#endif

#ifdef ROTARY_ENCODER
#define ROTARY_ENCODER_PIN_A 5 //Default 5; 
#define ROTARY_ENCODER_PIN_B 6 //Default 6; 
//#define ROTARY_ENCODER_PIN_SUPPLY 8 //uncomment if you want to use an IO pin as supply
#endif

#ifdef ROTARY_SWITCH
#define ROTARY_SWITCH_PIN  A5
#endif
//////////////////////////////////////////////////////////////////////////

///////// conifguration of the battery measurement ///////////////////////
//#ifdef CHECK_BATTERY
//#define refVoltage 1.08
//#define pinSupply 1024.0
//
//#define batLevel_LEDyellowOn 3.5
//#define batLevel_LEDyellowOff 3.7
//
//#define batLevel_LEDredOn 3.1
//#define batLevel_LEDredOff 3.2
//
//#define batLevel_Empty 2.9
//
//#define Rone  9920.0 //6680.0
//#define Rtwo  990.0 //987.0
//#endif
//////////////////////////////////////////////////////////////////////////

///////// conifguration of the rotary encoder ////////////////////////////
#ifdef ROTARY_ENCODER
#define ROTARY_ENCODER_STEPS 4
#endif
//////////////////////////////////////////////////////////////////////////

///////// conifguration of the rotary switch ////////////////////////////
#ifdef ROTARY_SWITCH
#define ROTARY_SWITCH_POSITIONS 12
#define ROTARY_SWITCH_TOLERNACE 0.15
#define ROTARY_SWITCH_REF_VOLTAGE 5.0
#define ROTARY_SWITCH_STEP_MIN ((ROTARY_SWITCH_REF_VOLTAGE/ROTARY_SWITCH_POSITIONS) - ((ROTARY_SWITCH_REF_VOLTAGE/ROTARY_SWITCH_POSITIONS)*ROTARY_SWITCH_TOLERNACE))
#define ROTARY_SWITCH_STEP_MAX ((ROTARY_SWITCH_REF_VOLTAGE/ROTARY_SWITCH_POSITIONS) + ((ROTARY_SWITCH_REF_VOLTAGE/ROTARY_SWITCH_POSITIONS)*ROTARY_SWITCH_TOLERNACE))
#define ROTARY_SWITCH_TRIGGER_TIME 2000
#endif
//////////////////////////////////////////////////////////////////////////

///////// MFRC522 ////////////////////////////////////////////////////////
#define RST_PIN 9                 // Configurable, see typical pin layout above
#define SS_PIN 10                 // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522
MFRC522::MIFARE_Key key;
bool successRead;
byte sector = 1;
byte blockAddr = 4;
byte trailerBlock = 7;
MFRC522::StatusCode status;
//////////////////////////////////////////////////////////////////////////

///////// setup buttons //////////////////////////////////////////////////
Button pauseButton(buttonPause);
Button upButton(buttonUp);
Button downButton(buttonDown);
#ifdef FIVEBUTTONS
Button buttonFour(buttonFourPin);
Button buttonFive(buttonFivePin);
#endif
bool ignorePauseButton = false;
bool ignoreUpButton = false;
bool ignoreDownButton = false;
#ifdef FIVEBUTTONS
bool ignoreButtonFour = false;
bool ignoreButtonFive = false;
#endif
//////////////////////////////////////////////////////////////////////////

//////// rotary encoder /////////////////////////////////////////////////
#ifdef ROTARY_ENCODER
int16_t RotEncOldEncPos = -1 ;
int16_t RotEncPos = 15;
ClickEncoder encoder(ROTARY_ENCODER_PIN_A, ROTARY_ENCODER_PIN_B, ROTARY_ENCODER_STEPS);
#endif
//////////////////////////////////////////////////////////////////////////

//////// rotary encoder /////////////////////////////////////////////////
#ifdef ROTARY_SWITCH
uint8_t RotSwCurrentPos = 0;
unsigned long RotSwMillis = 0;
uint8_t RotSwNewPos = 0;
uint8_t RotSwActivePos = 0;
//Array Slot [x,0] = Folder No., Array[x,1] = Play Mode, Array[x,2] = Special, Array[x,3] = Special2. If Folder = 0 then Mode = Modifier
int8_t RotSwMap [ROTARY_SWITCH_POSITIONS][4];
#endif

typedef enum Enum_PlayMode
{
  ModifierMode = 0,
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
  SleepTimerMod = 1,
  FreezeDanceMod = 2,
  LockedMod = 3,
  ToddlerModeMod = 4,
  KindergardenModeMod = 5,
  RepeatSingleMod = 6,
  FeedbackMod = 7,
  PuzzleGameMod = 8,
  QuizGameMod = 9,
  ButtonSmashMod = 10,
  AdminMenuMod = 255
};
typedef enum Enum_SystemControl
{
  PauseCont = 1,
  VolumeCont = 2,
  ForwardCont = 3,
  BackwardCont = 4,
  ShutDownCont = 5,
  RemoveModifierCont_6
};


//////////////////////////////////////////////////////////////////////////

//////// battery check //////////////////////////////////////////////////
//#ifdef CHECK_BATTERY
//float Bat_Mean = 0.0;
//float Bat_SqrMean = 0.0;
//float Bat_VarMean = 0.0;
//uint8_t Bat_N = 0;
//uint8_t Bat_BatteryLoad = 0;
//uint8_t batLevel_EmptyCounter = 0;
//uint8_t batLevel_LEDyellowCounter = 0;
//uint8_t batLevel_LEDredCounter = 0;
//float batLevel_LEDyellow;
//float batLevel_LEDred;
//#endif
//////////////////////////////////////////////////////////////////////////

///////// card cookie ////////////////////////////////////////////////////
static const uint32_t cardCookie = 322417479;
//////////////////////////////////////////////////////////////////////////

///////// DFPlayer Mini //////////////////////////////////////////////////
SoftwareSerial mySoftwareSerial(2, 3); // RX, TX
uint16_t numTracksInFolder;
uint16_t currentTrack;
uint16_t firstTrack;
uint8_t queue[255];
uint8_t volume;
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

struct nfcTagObject {
  uint32_t cookie;
  uint8_t version;
  folderSettings nfcFolderSettings;
};
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
  long standbyTimer;
  bool invertVolumeButtons;
  folderSettings shortCuts[4];
  uint8_t adminMenuLocked;
  uint8_t adminMenuPin[4];
};

adminSettings mySettings;
nfcTagObject myCard;
folderSettings *myFolder;
unsigned long sleepAtMillis = 0;
static uint16_t _lastTrackFinished;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
#ifdef ROTARY_ENCODER
void timerIsr();
#endif
#ifdef ROTARY_SWITCH
bool SetModifier (uint8_t tmpMode, uint8_t tmpSpecial1, uint8_t tmpSpecial2);
void RotSwloop(uint8_t TriggerTime = 0);
#endif
void shutDown ();
static void nextTrack(uint16_t track);
uint8_t voiceMenu(int numberOfOptions, int startMessage, int messageOffset,
                  bool preview = false, int previewFromFolder = 0, int defaultValue = 0, bool exitWithLongPress = false);
bool isPlaying();
bool checkTwo ( uint8_t a[], uint8_t b[] );
void writeCard(nfcTagObject nfcTag);
void dump_byte_array(byte * buffer, byte bufferSize);
void adminMenu(bool fromCard = false);
bool knownCard = false;
void readButtons();
void playFolder();
//////////////////////////////////////////////////////////////////////////

// implement a notification class,
// its member methods will get called
//
class Mp3Notify {
  public:
    static void OnError(uint16_t errorCode) {
      // see DfMp3_Error for code meaning
      Serial.println();
      Serial.print("Com Error ");
      Serial.println(errorCode);
    }
    static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action) {
      if (source & DfMp3_PlaySources_Sd) Serial.print("SD ");
      if (source & DfMp3_PlaySources_Usb) Serial.print("USB ");
      if (source & DfMp3_PlaySources_Flash) Serial.print("Flash ");
      Serial.println(action);
    }
    static void OnPlayFinished(DfMp3_PlaySources source, uint16_t track) {
      nextTrack(track);
    }
    static void OnPlaySourceOnline(DfMp3_PlaySources source) {
      PrintlnSourceAction(source, "online");
    }
    static void OnPlaySourceInserted(DfMp3_PlaySources source) {
      PrintlnSourceAction(source, "ready");
    }
    static void OnPlaySourceRemoved(DfMp3_PlaySources source) {
      PrintlnSourceAction(source, "remove");
    }
};

static DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(mySoftwareSerial);
//////////////////////////////////////////////////////////////////////////
void shuffleQueue() {
  // Queue für die Zufallswiedergabe erstellen
  for (uint8_t x = 0; x < numTracksInFolder - firstTrack + 1; x++)
    queue[x] = x + firstTrack;
  // Rest mit 0 auffüllen
  for (uint8_t x = numTracksInFolder - firstTrack + 1; x < 255; x++)
    queue[x] = 0;
  // Queue mischen
  for (uint8_t i = 0; i < numTracksInFolder - firstTrack + 1; i++)
  {
    uint8_t j = random (0, numTracksInFolder - firstTrack + 1);
    uint8_t t = queue[i];
    queue[i] = queue[j];
    queue[j] = t;
  }
  //#ifdef DEBUG
  //  Serial.println(F("Queue :"));
  //  for (uint8_t x = 0; x < numTracksInFolder - firstTrack + 1 ; x++)
  //    Serial.println(queue[x]);
  //#endif
}
//////////////////////////////////////////////////////////////////////////
void writeSettingsToFlash() {
#ifdef DEBUG
  Serial.println(F("writeSettings"));
#endif
  int address = sizeof(myFolder->folder) * 100;
  EEPROM.put(address, mySettings);
}
//////////////////////////////////////////////////////////////////////////
void resetSettings() {
#ifdef DEBUG
  Serial.println(F("resetSettings"));
#endif
  mySettings.cookie = cardCookie;
  mySettings.version = 2;
  mySettings.maxVolume = 25;
  mySettings.minVolume = 1;
  mySettings.initVolume = 1;
  mySettings.eq = 1;
  mySettings.locked = false;
  mySettings.standbyTimer = 0;
  mySettings.invertVolumeButtons = true;
  mySettings.shortCuts[0].folder = 0;
  mySettings.shortCuts[1].folder = 0;
  mySettings.shortCuts[2].folder = 0;
  mySettings.shortCuts[3].folder = 0;
  mySettings.adminMenuLocked = 0;
  mySettings.adminMenuPin[0] = 1;
  mySettings.adminMenuPin[1] = 1;
  mySettings.adminMenuPin[2] = 1;
  mySettings.adminMenuPin[3] = 1;

  writeSettingsToFlash();
}
//////////////////////////////////////////////////////////////////////////
void migrateSettings(int oldVersion) {
  if (oldVersion == 1) {
#ifdef DEBUG
    Serial.println(F("resetSettings"));
    Serial.println(F("1 > 2"));
#endif
    mySettings.version = 2;
    mySettings.adminMenuLocked = 0;
    mySettings.adminMenuPin[0] = 1;
    mySettings.adminMenuPin[1] = 1;
    mySettings.adminMenuPin[2] = 1;
    mySettings.adminMenuPin[3] = 1;
    writeSettingsToFlash();
  }
}
//////////////////////////////////////////////////////////////////////////
void loadSettingsFromFlash() {
#ifdef DEBUG
  Serial.println(F("loadSettings"));
#endif
  int address = sizeof(myFolder->folder) * 100;
  EEPROM.get(address, mySettings);
  if (mySettings.cookie != cardCookie)
    resetSettings();
  migrateSettings(mySettings.version);

#ifdef DEBUG
  Serial.print(F("Version: "));
  Serial.println(mySettings.version);

  Serial.print(F("Max Vol: "));
  Serial.println(mySettings.maxVolume);

  Serial.print(F("Min Vole: "));
  Serial.println(mySettings.minVolume);

  Serial.print(F("Init Vol: "));
  Serial.println(mySettings.initVolume);

  Serial.print(F("EQ: "));
  Serial.println(mySettings.eq);

  Serial.print(F("Locked: "));
  Serial.println(mySettings.locked);

  Serial.print(F("Sleep Timer: "));
  Serial.println(mySettings.standbyTimer);

  Serial.print(F("Inverted Vol Buttons: "));
  Serial.println(mySettings.invertVolumeButtons);

  Serial.print(F("Admin Menu locked: "));
  Serial.println(mySettings.adminMenuLocked);

  Serial.print(F("Admin Menu Pin: "));
  Serial.print(mySettings.adminMenuPin[0]);
  Serial.print(mySettings.adminMenuPin[1]);
  Serial.print(mySettings.adminMenuPin[2]);
  Serial.println(mySettings.adminMenuPin[3]);
#endif
}

/// Funktionen für den Standby Timer (z.B. über Pololu-Switch oder Mosfet)

void setstandbyTimer() {

#ifdef DEBUG
  Serial.println(F("setstandbyTimer"));
#endif
  if (mySettings.standbyTimer != 0)
    sleepAtMillis = millis() + (mySettings.standbyTimer * 60 * 1000);
  else
    sleepAtMillis = 0;
#ifdef DEBUG
  Serial.print(F("milis: "));
  Serial.println(sleepAtMillis);
#endif
}
//////////////////////////////////////////////////////////////////////////
void disablestandbyTimer() {
#ifdef DEBUG
  Serial.println(F("disablestandby"));
#endif
  sleepAtMillis = 0;
}
//////////////////////////////////////////////////////////////////////////
void checkStandbyAtMillis() {
  if (sleepAtMillis != 0 && millis > sleepAtMillis) {
#ifdef DEBUG
    Serial.println(F("power off"));
#endif
    // enter sleep state
    digitalWrite(shutdownPin, HIGH);
    delay(500);

    // http://discourse.voss.earth/t/intenso-s10000-powerbank-automatische-abschaltung-software-only/805
    // powerdown to 27mA (powerbank switches off after 30-60s)
    mfrc522.PCD_AntennaOff();
    mfrc522.PCD_SoftPowerDown();
    mp3.sleep();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();  // Disable interrupts
    sleep_mode();
  }
}
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
    virtual bool handleNextButton() {
      return false;
    }
    virtual bool handlePreviousButton() {
      return false;
    }
    virtual bool handleVolumeUp() {
      return false;
    }
    virtual bool handleVolumeDown() {
      return false;
    }
    virtual bool handleShutDown() {
      return false;
    }
    virtual bool handleRFID(nfcTagObject *newCard) {
      return false;
    }
    virtual uint8_t getActive() {
      return 0;
    }
    Modifier() {

    }
};

Modifier *activeModifier = NULL;
//////////////////////////////////////////////////////////////////////////
class SleepTimer: public Modifier {
  private:
    unsigned long sleepAtMillis = 0;

  public:
    void loop() {
      if (this->sleepAtMillis != 0 && millis > this->sleepAtMillis) {
#ifdef DEBUG
        Serial.println(F("SleepTimer:loop > SLEEP"));
#endif
        mp3.pause();
        setstandbyTimer();
        activeModifier = NULL;
        delete this;
      }
    }

    SleepTimer(uint8_t minutes) {
#ifdef DEBUG
      Serial.println(F("SleepTimer"));
      Serial.println(minutes);
#endif
      this->sleepAtMillis = millis() + minutes * 60000;
      //      if (isPlaying())
      //        mp3.playAdvertisement(302);
      //      delay(500);
    }
    uint8_t getActive() {
#ifdef DEBUG
      Serial.println(F("SleepTimer:getActive"));
#endif
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
      uint16_t seconds = random(this->minSecondsBetweenStops, this->maxSecondsBetweenStops + 1);
#ifdef DEBUG
      Serial.println(F("=FreezeDance:setNextStopAtMillis "));
      Serial.println(seconds);
#endif
      this->nextStopAtMillis = millis() + seconds * 1000;
    }

  public:
    void loop() {
      if (this->nextStopAtMillis != 0 && millis > this->nextStopAtMillis) {
#ifdef DEBUG
        Serial.println(F("FreezeDance:loop > FREEZE"));
#endif
        if (isPlaying()) {
          mp3.playAdvertisement(301);
          delay(500);
        }
        setNextStopAtMillis();
      }
    }
    FreezeDance(void) {
#ifdef DEBUG
      Serial.println(F("= FreezeDance"));
#endif
      if (isPlaying()) {
        delay(1000);
        mp3.playAdvertisement(300);
        delay(500);
      }
      setNextStopAtMillis();
    }
    uint8_t getActive() {
#ifdef DEBUG
      Serial.println(F("FreezeDance:getActive"));
#endif
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

    nfcTagObject tmpCard;

    void Success ()
    {
      PartOneSaved = false;
      PartTwoSaved = false;
#ifdef DEBUG
      Serial.println(F("PuzzleGame:Success"));
#endif

      mp3.playMp3FolderTrack(297); //Toll gemacht! Das ist richtig.
      waitForTrackToFinish();
    }

    void Failure ()
    {
      if (mode == PuzzlePart) {
        PartOneSaved = false;
      }
      PartTwoSaved = false;

#ifdef DEBUG
      Serial.println(F("PuzzleGame:Failure"));
#endif

      mp3.playMp3FolderTrack(296); //Bist du dir sicher das diese Karte richtig ist? Versuche es doch noch einmal.
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
      if (upButton.pressedFor(LONG_PRESS) && downButton.pressedFor(LONG_PRESS)) {
        do {
          readButtons();
        } while (upButton.isPressed() || downButton.isPressed());
#ifdef DEBUG
        Serial.println(F("PuzzleGame:loop > Del part"));
#endif
        mp3.pause();
        delay(100);
        mp3.playMp3FolderTrack(298); //Okay, lass uns was anderes probieren.
        PartOneSaved = false;
        PartTwoSaved = false;
      }
    }

    PuzzleGame(uint8_t special)
    {
      mp3.loop();
      mp3.pause();
#ifdef DEBUG
      Serial.println(F("= PuzzleGame"));
#endif

      mode = special;
    }

    virtual bool handlePause() {
      return false;
    }

    virtual bool handleNext() {
#ifdef DEBUG
      Serial.println(F("PuzzleGame:Next > Lock"));
#endif
      return true;
    }

    virtual bool handleNextButton()       {
#ifdef DEBUG
      Serial.println(F("PuzzleGame:NextButton > Lock"));
#endif
      return true;
    }

    virtual bool handlePreviousButton() {
#ifdef DEBUG
      Serial.println(F("PuzzleGame:PrevButton > Lock"));
#endif
      return true;
    }

    virtual bool handleRFID(nfcTagObject *newCard) {

      this->tmpCard = *newCard;
      if (tmpCard.nfcFolderSettings.mode != PuzzlePart) {
#ifdef DEBUG
        Serial.println(F("PuzzleGame:RFID > No Valid Part"));
#endif
        mp3.pause();
        delay(100);
        mp3.playMp3FolderTrack(299); //Diese Karte ist nicht Teil des Spiels. Probiere eine Andere.
        return true;
      }
      else {
#ifdef DEBUG
        Serial.println(F("PuzzleGame:RFID > Valid Part"));
#endif
        if (!PartOneSaved)
        {
          PartOneSpecial = tmpCard.nfcFolderSettings.special;
          PartOneFolder = tmpCard.nfcFolderSettings.folder;
          PartOneSpecial2 = tmpCard.nfcFolderSettings.special2;
          PartOneSaved = true;
        }
        else if (PartOneSaved && !PartTwoSaved) {
          PartTwoSpecial = tmpCard.nfcFolderSettings.special;
          PartTwoFolder = tmpCard.nfcFolderSettings.folder;
          PartTwoSpecial2 = tmpCard.nfcFolderSettings.special2;
          PartTwoSaved = true;
        }
        return false;
      }
    }

    uint8_t getActive() {
#ifdef DEBUG
      Serial.println(F("PuzzleGame:getActive"));
#endif
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

    nfcTagObject tmpCard;

    void Success () {
      PartOneSaved = false;
      PartTwoSaved = false;
#ifdef DEBUG
      Serial.println(F("QuizGame:Success"));
#endif
      mp3.playMp3FolderTrack(297);
      waitForTrackToFinish();
      next();
    }

    void Failure () {
      PartTwoSaved = false;
#ifdef DEBUG
      Serial.println(F("QuizGame:Failure"));
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
#ifdef DEBUG
          Serial.println(F("QuizGame:loop > Compare"));
#endif
          CompareParts();
        }
      }

      if (upButton.pressedFor(LONG_PRESS) && downButton.pressedFor(LONG_PRESS)) {
        do {
          readButtons();
        } while (upButton.isPressed() || downButton.isPressed());
#ifdef DEBUG
        Serial.println(F("QuizGame:loop > Del part"));
#endif
        mp3.pause();
        delay(100);
        mp3.playMp3FolderTrack(298); //Okay, lass uns was anderes probieren.
        waitForTrackToFinish();
        PartOneSaved = false;
        PartTwoSaved = false;
        next();
      }
    }

    QuizGame(uint8_t special) {
      mp3.loop();
#ifdef DEBUG
      Serial.println(F("= QuizGame"));
#endif
      mp3.pause();


      PartOneFolder = special;
      numTracksInFolder = mp3.getFolderTrackCount(PartOneFolder);
      firstTrack = 1;
      shuffleQueue();
      currentTrack = 0;

#ifdef DEBUG
      Serial.println(F("= QuizGame > Queue set"));
#endif
      next();
    }

    void  next() {
      numTracksInFolder = mp3.getFolderTrackCount(PartOneFolder);
#ifdef DEBUG
      Serial.println(F("QuizGame:next()"));
      Serial.println(currentTrack);
      Serial.println(numTracksInFolder - firstTrack + 1);
#endif
      if (currentTrack != numTracksInFolder - firstTrack + 1) {
#ifdef DEBUG
        Serial.println(F("QuizGame:next()-> Queue next"));
#endif
        currentTrack++;
      } else {
#ifdef DEBUG
        Serial.println(F("QuizGame:next > repeat"));
#endif
        currentTrack = 1;
      }
      PartOneSaved = true;
      PartOneSpecial = queue[currentTrack - 1];
      PartOneSpecial2 = PartOneSpecial;
#ifdef DEBUG
      Serial.println(currentTrack);
      Serial.println(PartOneSpecial);
#endif
      mp3.playFolderTrack(PartOneFolder, PartOneSpecial);
    }

    virtual bool handleNext() {
#ifdef DEBUG
      Serial.println(F("QuizGame:Next > Lock"));
#endif
      return true;
    }

    virtual bool handlePause() {
#ifdef DEBUG
      Serial.println(F("QuizGame:Pause > Pause and repeat "));
#endif
      mp3.pause();
      delay(100);
      mp3.playFolderTrack(PartOneFolder, PartOneSpecial);
      return true;
    }

    virtual bool handleNextButton()       {
#ifdef DEBUG
      Serial.println(F("QuizGame:NextButton > Lock"));
#endif
      return true;
    }

    virtual bool handlePreviousButton() {
#ifdef DEBUG
      Serial.println(F("QuizGame:PrevButton > Lock"));
#endif
      return true;
    }

    virtual bool handleRFID(nfcTagObject * newCard) {

      this->tmpCard = *newCard;
      if (tmpCard.nfcFolderSettings.mode != PuzzlePart) {
#ifdef DEBUG
        Serial.println(F("QuizGame:RFID > No Valid Part"));
#endif
        mp3.pause();
        delay(100);
        mp3.playMp3FolderTrack(299); //Diese Karte ist nicht Teil des Spiels. Probiere eine Andere.
        return true;
      }
      else {
#ifdef DEBUG
        Serial.println(F("QuizGame:RFID > Valid Part"));
#endif
        if (PartOneSaved && !PartTwoSaved) {
          PartTwoSpecial = tmpCard.nfcFolderSettings.special;
          PartTwoFolder = tmpCard.nfcFolderSettings.folder;
          PartTwoSpecial2 = tmpCard.nfcFolderSettings.special2;
          PartTwoSaved = true;
        }
        return false;
      }
    }

    uint8_t getActive() {
#ifdef DEBUG
      Serial.println(F("QuizGame:getActive"));
#endif
      return QuizGameMod;
    }
};
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
#ifdef DEBUG
      Serial.println(F("= ButtonSmash"));
#endif
      mp3.pause();
      delay(200);
#ifdef DEBUG
      Serial.println(F("= ButtonSmash > Set Vol"));
#endif
      mp3.setVolume(special2);
      delay(200);
#ifdef DEBUG
      Serial.println(F("= ButtonSmash > Set Folder"));
#endif
      Folder = special;
      numTracksInFolder = mp3.getFolderTrackCount(Folder);
#ifdef DEBUG
      Serial.println(F("= ButtonSmash > queue set"));
#endif
      firstTrack = 1;
      shuffleQueue();
      currentTrack = 0;

#ifdef DEBUG
      Serial.println(F("= ButtonSmash > Init End"));
#endif
    }

    void  next() {
      mp3.pause();
      delay(100);
      numTracksInFolder = mp3.getFolderTrackCount(Folder);
#ifdef DEBUG
      Serial.println(F("ButtonSmash:next()"));
      Serial.println(currentTrack);
      Serial.println(numTracksInFolder - firstTrack + 1);
#endif
      if (currentTrack != numTracksInFolder - firstTrack + 1) {
#ifdef DEBUG
        Serial.println(F("ButtonSmash:next()-> queue next"));
#endif
        currentTrack++;
      } else {
#ifdef DEBUG
        Serial.println(F("ButtonSmash:next > repeat"));
#endif
        firstTrack = 1;
        shuffleQueue();
        currentTrack = 1;
      }
      mp3.playFolderTrack(Folder, queue[currentTrack - 1]);
      waitForTrackToFinish();
    }

    virtual bool handleNext() {
#ifdef DEBUG
      Serial.println(F("ButtonSmash:Next"));
#endif
      return true;
    }

    virtual bool handlePause() {
#ifdef DEBUG
      Serial.println(F("ButtonSmash:Pause"));
#endif
      next();
      return true;
    }

    virtual bool handleNextButton()       {
#ifdef DEBUG
      Serial.println(F("ButtonSmash:NextButton"));
#endif
      next();
      return true;
    }

    virtual bool handlePreviousButton() {
#ifdef DEBUG
      Serial.println(F("ButtonSmash:PrevButton"));
#endif
      next();
      return true;
    }

    virtual bool handleVolumeUp() {
#ifdef DEBUG
      Serial.println(F("ButtonSmash:VolUp"));
#endif
      next();
      return true;
    }
    virtual bool handleVolumeDown() {
#ifdef DEBUG
      Serial.println(F("ButtonSmash:VolDown"));
#endif
      next();
      return true;
    }

    virtual bool handleShutDown() {
#ifdef DEBUG
      Serial.println(F("ButtonSmash:ShutDown"));
#endif
      next();
      return true;
    }
    virtual bool handleRFID(nfcTagObject * newCard) {
#ifdef DEBUG
      Serial.println(F("ButtonSmash:RFID"));
#endif
      next();
      return true;
    }

    uint8_t getActive() {
#ifdef DEBUG
      Serial.println(F("ButtonSmash:getActive"));
#endif
      return ButtonSmashMod ;
    }
};
//////////////////////////////////////////////////////////////////////////
class Locked: public Modifier {
  public:
    virtual bool handlePause()     {
#ifdef DEBUG
      Serial.println(F("Locked:Pause > Lock"));
#endif
      return true;
    }
    virtual bool handleNextButton()       {
#ifdef DEBUG
      Serial.println(F("Locked:NextButton > Lock"));
#endif
      return true;
    }
    virtual bool handlePreviousButton() {
#ifdef DEBUG
      Serial.println(F("Locked:PrevButton > Lock"));
#endif
      return true;
    }
    virtual bool handleVolumeUp()   {
#ifdef DEBUG
      Serial.println(F("Locked:VolUp > Lock"));
#endif
      return true;
    }
    virtual bool handleVolumeDown() {
#ifdef DEBUG
      Serial.println(F("Locked:VolDown > Lock"));
#endif
      return true;
    }
    virtual bool handleShutDown() {
#ifdef DEBUG
      Serial.println(F("Locked:ShutDown > Lock"));
#endif
      return true;
    }
    virtual bool handleRFID(nfcTagObject *newCard) {
#ifdef DEBUG
      Serial.println(F("Locked:RFID > Lock"));
#endif
      return true;
    }
    Locked(void) {
#ifdef DEBUG
      Serial.println(F("= Locked"));
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
#ifdef DEBUG
      Serial.println(F("ToddlerMode:Pause > Lock"));
#endif
      return true;
    }
    virtual bool handleNextButton()       {
#ifdef DEBUG
      Serial.println(F("ToddlerMode:NextButton > Lock"));
#endif
      return true;
    }
    virtual bool handlePreviousButton() {
#ifdef DEBUG
      Serial.println(F("ToddlerMode:PrevButton > Lock"));
#endif
      return true;
    }
    virtual bool handleVolumeUp()   {
#ifdef DEBUG
      Serial.println(F("ToddlerMode:VolUp > Lock"));
#endif
      return true;
    }
    virtual bool handleVolumeDown() {
#ifdef DEBUG
      Serial.println(F("ToddlerMode:VolDown > Lock"));
#endif
      return true;
    }
    virtual bool handleShutDown() {
#ifdef DEBUG
      Serial.println(F("Locked:ShutDown > Lock"));
#endif
      return true;
    }
    ToddlerMode(void) {
#ifdef DEBUG
      Serial.println(F("= ToddlerMode"));
#endif
    }
    uint8_t getActive() {
#ifdef DEBUG
      Serial.println(F("ToddlerMode:getActive"));
#endif
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
#ifdef DEBUG
      Serial.println(F("KindergardenMode:Next"));
#endif
      if (this->cardQueued == true) {
        this->cardQueued = false;

        myCard = nextCard;
        if (&myCard.nfcFolderSettings > 255)
          myFolder = &myCard.nfcFolderSettings - 255;
        else
          myFolder = &myCard.nfcFolderSettings;
#ifdef DEBUG
        Serial.println(myFolder->folder);
        Serial.println(myFolder->mode);
#endif
        playFolder();
        return true;
      }
      return false;
    }
    virtual bool handleNextButton()       {
#ifdef DEBUG
      Serial.println(F("KindergardenMode:NextButton > Lock"));
#endif
      return true;
    }
    virtual bool handlePreviousButton() {
#ifdef DEBUG
      Serial.println(F("KindergardenMode:PrevButton > Lock"));
#endif
      return true;
    }
    virtual bool handleShutDown() {
#ifdef DEBUG
      Serial.println(F("KindergardenMode:ShutDown > Lock"));
#endif
      return true;
    }
    virtual bool handleRFID(nfcTagObject * newCard) { // lot of work to do!
#ifdef DEBUG
      Serial.println(F("KindergardenMode:RFID > queued"));
#endif
      this->nextCard = *newCard;
      this->cardQueued = true;
      if (!isPlaying()) {
        handleNext();
      }
      return true;
    }
    KindergardenMode() {
#ifdef DEBUG
      Serial.println(F("KindergardenMode"));
#endif
    }
    uint8_t getActive() {
#ifdef DEBUG
      Serial.println(F("KindergardenMode:getActive"));
#endif
      return KindergardenModeMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class RepeatSingleModifier: public Modifier {
  public:
    virtual bool handleNext() {
#ifdef DEBUG
      Serial.println(F("RepeatSingle:Next > repeat track"));
#endif
      delay(50);
      if (isPlaying()) return true;
      mp3.playFolderTrack(myFolder->folder, currentTrack);
      _lastTrackFinished = 0;
      return true;
    }
    RepeatSingleModifier() {
#ifdef DEBUG
      Serial.println(F("RepeatSingle"));
#endif
    }
    uint8_t getActive() {
#ifdef DEBUG
      Serial.println(F("RepeatSingle:getActive"));
#endif
      return RepeatSingleMod;
    }
};
//////////////////////////////////////////////////////////////////////////
// An modifier can also do somethings in addition to the modified action
// by returning false (not handled) at the end
// This simple FeedbackModifier will tell the volume before changing it and
// give some feedback once a RFID card is detected.
class FeedbackModifier: public Modifier {
  public:
    virtual bool handleVolumeDown() {
      if (volume > mySettings.minVolume) {
        mp3.playAdvertisement(volume - 1);
      }
      else {
        mp3.playAdvertisement(volume);
      }
      delay(500);
#ifdef DEBUG
      Serial.println(F("Feedback:VolDown"));
#endif
      return false;
    }
    virtual bool handleVolumeUp() {
      if (volume < mySettings.maxVolume) {
        mp3.playAdvertisement(volume + 1);
      }
      else {
        mp3.playAdvertisement(volume);
      }
      delay(500);
#ifdef DEBUG
      Serial.println(F("Feedback:VolUp"));
#endif
      return false;
    }
    virtual bool handleRFID(nfcTagObject *newCard) {
#ifdef DEBUG
      Serial.println(F("Feedback:RFID"));
#endif
      return false;
    }
    uint8_t getActive() {
#ifdef DEBUG
      Serial.println(F("Feedback:getActive"));
#endif
      return FeedbackMod;
    }
};
//////////////////////////////////////////////////////////////////////////
// Leider kann das Modul selbst keine Queue abspielen, daher müssen wir selbst die Queue verwalten
static void nextTrack(uint16_t track) {
  uint16_t tmpNextTrack = 65536;
 
#ifdef DEBUG
  Serial.println(F("nextTrack"));
#endif

  if (activeModifier != NULL)
    if (activeModifier->handleNext() == true)
      return;

if (track == _lastTrackFinished) {
    return;
  }
  _lastTrackFinished = track;
  
  if (knownCard == false)
    // Wenn eine neue Karte angelernt wird soll das Ende eines Tracks nicht
    // verarbeitet werden
    return;

  switch (myFolder->mode) {
    case Album:
    case Album_Section:
      if (currentTrack != numTracksInFolder)
        tmpNextTrack = currentTrack + 1;
      else
        setstandbyTimer();
      break;

    case Party:
    case Party_Section:
      if (currentTrack != numTracksInFolder - firstTrack + 1)
        tmpNextTrack = currentTrack + 1;
      else {
#ifdef DEBUG
        Serial.println(F("queue End"));
#endif
        currentTrack = 1;
        //// Wenn am Ende der Queue neu gemischt werden soll bitte die Zeilen wieder aktivieren
        //     Serial.println(F("Ende der Queue > mische neu"));
        //     shuffleQueue();
      }
      tmpNextTrack = queue[tmpNextTrack - 1];
      break;

    case AudioBook:
    case AudioBook_Section:
      if (currentTrack != numTracksInFolder) {
        tmpNextTrack = currentTrack + 1;

        // Fortschritt im EEPROM abspeichern
        writeAudiobookMemory (myFolder->folder, myFolder->special3, tmpNextTrack);
      } else {
        // Fortschritt zurücksetzen (wenn vorhanden: auf das nächste Hörspiel im Ordner. sonst: ganz auf den Anfang)
        if (currentTrack < mp3.getFolderTrackCount(myFolder->folder)) {
          writeAudiobookMemory (myFolder->folder, myFolder->special3, firstTrack + numTracksInFolder);
        } else {
          writeAudiobookMemory (myFolder->folder, myFolder->special3, 1);
        }
        setstandbyTimer();
      }
      break;

    default:
#ifdef DEBUG
      Serial.println(F("No next Track"));
#endif
      setstandbyTimer();
      delay(500);
      return;
      break;
  }

#ifdef DEBUG
  Serial.print("next track: ");
  Serial.println(tmpNextTrack);
#endif

  mp3.playFolderTrack(myFolder->folder, tmpNextTrack);
  currentTrack = tmpNextTrack;
  delay(500);
}
//////////////////////////////////////////////////////////////////////////
static void previousTrack() {
  uint16_t tmpPreviousTrack = currentTrack;

  switch (myFolder->mode) {
    case Album:
    case Album_Section:
      if (currentTrack != firstTrack)
        tmpPreviousTrack = currentTrack - 1;
      break;

    case Party:
    case Party_Section:
      if (currentTrack != 1)
        tmpPreviousTrack = currentTrack - 1;
      else
      {
#ifdef DEBUG
        Serial.print(F("beginn queue"));
#endif
        tmpPreviousTrack = numTracksInFolder;
      }
      tmpPreviousTrack = queue[tmpPreviousTrack - 1];
      break;

    case AudioBook:
    case AudioBook_Section:
      if (currentTrack != firstTrack)
        tmpPreviousTrack = currentTrack - 1;

      // Fortschritt im EEPROM abspeichern
      writeAudiobookMemory (myFolder->folder, myFolder->special3, tmpPreviousTrack);
      break;

    default:
#ifdef DEBUG
      Serial.println(F("No previous Track"));
#endif
      setstandbyTimer();
      break;
  }

#ifdef DEBUG
  Serial.print("previous track: ");
  Serial.println(tmpPreviousTrack);
#endif

  mp3.playFolderTrack(myFolder->folder, tmpPreviousTrack);
  currentTrack = tmpPreviousTrack;
  delay(1000);
}
//////////////////////////////////////////////////////////////////////////
bool isPlaying() {
  return !digitalRead(busyPin);
}
//////////////////////////////////////////////////////////////////////////
void waitForTrackToFinish() {
  long currentTime = millis();
#define TIMEOUT 1000
  do {
    mp3.loop();
  } while (!isPlaying() && millis() < currentTime + TIMEOUT);
  delay(100);
  do {
    mp3.loop();
  } while (isPlaying());
}
//////////////////////////////////////////////////////////////////////////
//#ifdef CHECK_BATTERY
//void setColor(int redValue, int greenValue, int blueValue) {
//
//  analogWrite(LEDredPin, redValue);
//  analogWrite(LEDgreenPin, greenValue);
//  //analogWrite(LEDbluePin, blueValue);
//}
//#endif
//////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200); // Es gibt ein paar Debug Ausgaben über die serielle Schnittstelle
  // Dieser Hinweis darf nicht entfernt werden
  Serial.println(F("\n _____         _____ _____ _____ _____"));
  Serial.println(F("|_   _|___ ___|  |  |     |   | |     |"));
  Serial.println(F(" | | | . |   |  |  |-   -| | | |  |  |"));
  Serial.println(F(" |_| |___|_|_|_____|_____|_|___|_____|\n"));
  Serial.println(F("TonUINO Version 2.1"));
  Serial.println(F("created by Thorsten Voß and licensed under GNU/GPL."));
  Serial.println(F("Information and contribution at https://tonuino.de.\n"));
  Serial.println(F("Fork by Marco Schulz"));

  analogReference(DEFAULT);

  // Busy Pin
  pinMode(busyPin, INPUT);
  mp3.begin();
  delay(2000);
  mp3.loop();

  // Wert für randomSeed() erzeugen durch das mehrfache Sammeln von rauschenden LSBs eines offenen Analogeingangs
  uint32_t ADC_LSB;
  uint32_t ADCSeed;
  for (uint8_t i = 0; i < 128; i++) {
    ADC_LSB = analogRead(openAnalogPin) & 0x1;
    ADCSeed ^= ADC_LSB << (i % 32);
  }
  randomSeed(ADCSeed); // Zufallsgenerator initialisieren

#ifdef ROTARY_ENCODER
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  encoder.setAccelerationEnabled(false);
#ifdef ROTARY_ENCODER_PIN_SUPPLY
  pinMode(ROTARY_ENCODER_PIN_SUPPLY, OUTPUT);
  digitalWrite(ROTARY_ENCODER_PIN_SUPPLY, HIGH);
#endif
#endif

#ifdef ROTARY_SWITCH
  // Enum_PlayMode
  //        Hoerspiel_1 = 1,
  //        Album_2 = 2,
  //        Party_3 = 3,
  //        Einzel_4 = 4,
  //        Hoerbuch_5 = 5,
  //        Hoerspiel_von_bis_7 = 7,
  //        Album_von_bis_8 = 8,
  //        Party_von_bis_9 = 9,
  //        Puzzle_10 = 10,
  //        Hoerbuch_von_bis_11 = 11
  // Enum_Modifier
  //        SleepTimer_1 = 1,
  //        FreezeDance_2 = 2,
  //        Locked_3 = 3,
  //        ToddlerMode_4 = 4,
  //        KindergardenMode_5 = 5,
  //        RepeatSingleModifier_6 = 6,
  //        FeedbackModifier_7 = 7,
  //        PuzzleGame_8 = 8,
  //        QuizGame_9 = 9,
  //        ButtonSmash_10 = 10
  // Enum_SystemControl
  //         Pause_1 = 1,
  //         Volume_2 = 2,
  //         Forward_3 = 3,
  //         Backward_4 = 4,
  //         ShutDown_5 = 5,
  //         Remove_Modifier_6

  //              |Folder No.|                 |Mode|                               |Special|                  |Special2|
  RotSwMap[0][0] =     -1;     RotSwMap[0][1] =  Volume_2;             RotSwMap[0][2] = 5;         RotSwMap[0][3] = 0;
  RotSwMap[1][0] =     -1;     RotSwMap[1][1] =  Volume_2;             RotSwMap[1][2] = 15;         RotSwMap[1][3] = 0;
  RotSwMap[2][0] =     3;     RotSwMap[2][1] =  Party_3;             RotSwMap[2][2] = 0;         RotSwMap[2][3] = 0;
  RotSwMap[3][0] =     4;     RotSwMap[3][1] =  Party_3;             RotSwMap[3][2] = 0;         RotSwMap[3][3] = 0;
  RotSwMap[4][0] =     5;     RotSwMap[4][1] =  Party_3;             RotSwMap[4][2] = 0;         RotSwMap[4][3] = 0;
  RotSwMap[5][0] =     6;     RotSwMap[5][1] =  Party_3;             RotSwMap[5][2] = 0;         RotSwMap[5][3] = 0;
  RotSwMap[6][0] =     7;     RotSwMap[6][1] =  Party_3;             RotSwMap[6][2] = 0;         RotSwMap[6][3] = 0;
  RotSwMap[7][0] =     8;     RotSwMap[7][1] =  Party_3;             RotSwMap[7][2] = 0;         RotSwMap[7][3] = 0;
  RotSwMap[8][0] =     0;     RotSwMap[8][1] =  2;       RotSwMap[8][2] = 0;         RotSwMap[8][3] = 0;
  RotSwMap[9][0] =     0;     RotSwMap[9][1] =  8;        RotSwMap[9][2] = 1;         RotSwMap[9][3] = 0;
  RotSwMap[10][0] =    -1;     RotSwMap[10][1] = Remove_Modifier_6;          RotSwMap[10][2] = 1;        RotSwMap[10][3] = 0;
  RotSwMap[11][0] =    -1;     RotSwMap[11][1] = Pause_1;      RotSwMap[11][2] = 5;        RotSwMap[11][3] = 5;

  RotSwCurrentPos = RotSwGetPosition();
  RotSwActivePos = RotSwCurrentPos;
  RotSwMillis = millis();
#endif

#ifdef SPEAKER_SWITCH
  pinMode(SpeakerOnPin, OUTPUT);
  digitalWrite(SpeakerOnPin, LOW);
#endif


  //
  //#ifdef CHECK_BATTERY
  //  pinMode(LEDredPin, OUTPUT);
  //  pinMode(LEDgreenPin, OUTPUT);
  //
  //  setColor(0, 10, 0); // White Color
  //
  //  batLevel_LEDyellow = batLevel_LEDyellowOn;
  //  batLevel_LEDred = batLevel_LEDredOn;
  //#endif

  // load Settings from EEPROM
  loadSettingsFromFlash();

  // activate standby timer
  setstandbyTimer();

  // DFPlayer Mini initialisieren
  //  mp3.begin();
  //  // Zwei Sekunden warten bis der DFPlayer Mini initialisiert ist
  //  delay(2000);
  volume = mySettings.initVolume;
  mp3.setVolume(volume);
  mp3.setEq(mySettings.eq - 1);

  // NFC Leser initialisieren
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
#ifdef DEBUG
  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader
#endif
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  pinMode(buttonPause, INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
#ifdef FIVEBUTTONS
  pinMode(buttonFourPin, INPUT_PULLUP);
  pinMode(buttonFivePin, INPUT_PULLUP);
#endif
  pinMode(shutdownPin, OUTPUT);
  digitalWrite(shutdownPin, LOW);


  // RESET --- ALLE DREI KNÖPFE BEIM STARTEN GEDRÜCKT HALTEN > alle EINSTELLUNGEN werden gelöscht
  if (digitalRead(buttonPause) == LOW && digitalRead(buttonUp) == LOW &&
      digitalRead(buttonDown) == LOW) {
#ifdef DEBUG
    Serial.println(F("Reset > EEPROM wird gelöscht"));
#endif
    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.update(i, 0);
    }
    loadSettingsFromFlash();
  }
#ifdef SPEAKER_SWITCH
  digitalWrite(SpeakerOnPin, HIGH);
  delay(100);
#endif

  // Start Shortcut "at Startup"- e.g. Welcome Sound
  playShortCut(3);

#ifdef STARTUP_SOUND
  mp3.playMp3FolderTrack(264);
  delay(500);
  waitForTrackToFinish();
#endif
}
//////////////////////////////////////////////////////////////////////////
void readButtons() {
  pauseButton.read();
  upButton.read();
  downButton.read();
#ifdef FIVEBUTTONS
  buttonFour.read();
  buttonFive.read();
#endif
}
//////////////////////////////////////////////////////////////////////////
#ifndef ROTARY_ENCODER
void volumeUpButton() {
  if (activeModifier != NULL)
    if (activeModifier->handleVolumeUp() == true)
      return;
#ifdef DEBUG
  Serial.println(F("= volumeUp()"));
#endif
  if (volume < mySettings.maxVolume) {
    mp3.increaseVolume();
    volume++;
  }
#ifdef DEBUG
  Serial.println(volume);
#endif
}
//////////////////////////////////////////////////////////////////////////
void volumeDownButton() {
  if (activeModifier != NULL)
    if (activeModifier->handleVolumeDown() == true)
      return;

#ifdef DEBUG
  Serial.println(F("= volumeDown()"));
#endif
  if (volume > mySettings.minVolume) {
    mp3.decreaseVolume();
    volume--;
  }
#ifdef DEBUG
  Serial.println(volume);
#endif
}
#endif
//////////////////////////////////////////////////////////////////////////
void nextButton() {
  if (activeModifier != NULL)
    if (activeModifier->handleNextButton() == true)
      return;

  nextTrack(random(65536));
  delay(300);
}
//////////////////////////////////////////////////////////////////////////
void previousButton() {


  if (activeModifier != NULL)
    if (activeModifier->handlePreviousButton() == true)
      return;

  previousTrack();
  delay(300);
}
//////////////////////////////////////////////////////////////////////////
void playFolder() {
  uint8_t counter = 0;
  uint16_t playTrack;
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
      Serial.println(F("Hörspiel"));
#endif
      playTrack = random(1, numTracksInFolder + 1);
      break;
    case AudioDrama_Section:
#ifdef DEBUG
      Serial.println(F("Von-Bis Hörspiel"));
      Serial.print(myFolder->special);
      Serial.print(F(" to "));
      Serial.println(myFolder->special2);
#endif
      numTracksInFolder = myFolder->special2;
      playTrack = random(myFolder->special, numTracksInFolder + 1);
      break;

    case Album:
#ifdef DEBUG
      Serial.println(F("Album"));
#endif
      playTrack = 1;
      break;
    case Album_Section:
#ifdef DEBUG
      Serial.println(F("Von-Bis Album"));
      Serial.print(myFolder->special);
      Serial.print(F(" to "));
      Serial.println(myFolder->special2);
#endif
      numTracksInFolder = myFolder->special2;
      playTrack = myFolder->special;
      break;

    case Party:
#ifdef DEBUG
      Serial.println(
        F("Party"));
#endif
      shuffleQueue();
      playTrack = queue[0];
      break;
    case Party_Section:
#ifdef DEBUG
      Serial.println(
        F("Von-Bis Party"));
#endif
      firstTrack = myFolder->special;
      numTracksInFolder = myFolder->special2;
      shuffleQueue();
      playTrack = queue[0];
      break;

    case AudioBook:
#ifdef DEBUG
      Serial.println(F("Hörbuch"));
#endif
      playTrack = readAudiobookMemory(myFolder->folder, myFolder->special3);
      if (playTrack == 0 || playTrack > numTracksInFolder) {
        playTrack = 1;
      }
      break;
    case AudioBook_Section:
#ifdef DEBUG
      Serial.println(
        F("Von-Bis Hörbuch"));
#endif
      firstTrack = myFolder->special;
      numTracksInFolder = myFolder->special2;
      playTrack = readAudiobookMemory(myFolder->folder, myFolder->special3);
      if (playTrack < firstTrack || playTrack >= firstTrack + numTracksInFolder) {
        playTrack = firstTrack;
#ifdef DEBUG
        Serial.println(F("not correct tag, start from beginn"));
#endif
      }
      break;

    case Single:
#ifdef DEBUG
      Serial.println(
        F("Einzel"));
#endif
      playTrack = myFolder->special;
      break;

    case PuzzlePart:
#ifdef DEBUG
      Serial.println(
        F("Puzzle"));
#endif
      playTrack = myFolder->special;
      break;
  }

#ifdef DEBUG
  Serial.print(F("play track: "));
  Serial.println(playTrack);
#endif

  mp3.playFolderTrack(myFolder->folder, playTrack);
  currentTrack = playTrack;

  while (!isPlaying() && counter <= 100) {
    delay(50);
    counter ++;
    if (counter >= 100) {
#ifdef DEBUG
      Serial.println(F("Track not starting.")) ;
#endif
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void playShortCut(uint8_t shortCut) {
#ifdef DEBUG
  Serial.print(F("ShortCut "));
  Serial.println(shortCut);
#endif
  if (mySettings.shortCuts[shortCut].folder != 0) {
    myFolder = &mySettings.shortCuts[shortCut];
    playFolder();
    disablestandbyTimer();
    //delay(1000);
  }
  else
  {
#ifdef DEBUG
    Serial.println(F("not configured"));
#endif
  }
}
//////////////////////////////////////////////////////////////////////////
//#ifdef CHECK_BATTERY
//void batteryCheck () {
//  float physValue;
//
//  physValue =  (((analogRead(batMeasPin) * refVoltage) / pinSupply) / (Rtwo / (Rone + Rtwo)));
//
//  Bat_Mean = Bat_Mean * Bat_N / (Bat_N + 1) + physValue / (Bat_N + 1);
//  if ((Bat_N < 5) || (Bat_VarMean > 0.002)) {
//    Bat_SqrMean = Bat_SqrMean * Bat_N / (Bat_N + 1) + physValue * physValue / (Bat_N + 1);
//    Bat_VarMean = (Bat_SqrMean - Bat_Mean * Bat_Mean) / (Bat_N + 1);
//    Bat_N = Bat_N + 1;
//  }
//  if (Bat_N > 10)  {
//    if (Bat_Mean > batLevel_LEDyellow)    {
//      batLevel_LEDyellowCounter = 0;
//      batLevel_LEDyellow = batLevel_LEDyellowOn;
//      setColor(0, 10, 0); // Green Color
//      //#ifdef DEBUG
//      //  Serial.println(F("= batteryCheck > Battery High"));
//      //#endif
//    }
//    else if (Bat_Mean < batLevel_LEDyellow && Bat_Mean > batLevel_LEDred )    {
//      if (batLevel_LEDyellowCounter >= 10)      {
//        batLevel_LEDredCounter = 0;
//        batLevel_LEDyellowCounter = 0;
//        batLevel_LEDyellow = batLevel_LEDyellowOff;
//        batLevel_LEDred = batLevel_LEDredOn;
//        setColor(20, 10, 0); // Yellow Color
//#ifdef DEBUG
//        Serial.println(F("batteryCheck > Mid"));
//#endif
//      }
//      else
//        batLevel_LEDyellowCounter ++;
//    }
//
//    else if (Bat_Mean < batLevel_LEDred && Bat_Mean > batLevel_Empty)    {
//      if (batLevel_LEDredCounter >= 10)      {
//        batLevel_EmptyCounter = 0;
//        batLevel_LEDredCounter = 0;
//        batLevel_LEDred = batLevel_LEDredOff;
//        setColor(20, 0, 0); // Red Color
//#ifdef DEBUG
//        Serial.println(F("batteryCheck > Low"));
//#endif
//      }
//      else
//        batLevel_LEDredCounter ++;
//    }
//    else if (Bat_Mean <= batLevel_Empty)    {
//      if (batLevel_EmptyCounter >= 10)      {
//#ifdef DEBUG
//        Serial.println(F("batteryCheck > Empty"));
//#endif
//        batLevel_EmptyCounter = 0;
//        //shutDown();
//      }
//      else
//        batLevel_EmptyCounter++;
//    }
//  }
//}
//#endif
//////////////////////////////////////////////////////////////////////////
void shutDown () {
  if (activeModifier != NULL)
    if (activeModifier->handleShutDown() == true)
      return;

#ifdef DEBUG
  Serial.println("Shut Down");
#endif

  mp3.pause();
  delay(500);

#ifdef STARTUP_SOUND
  mp3.setVolume(mySettings.initVolume);
  delay(500);
#ifdef DEBUG
  Serial.println("Shut Down Sound");
#endif
  mp3.playMp3FolderTrack(265);
  waitForTrackToFinish();
#endif

  digitalWrite(shutdownPin, HIGH);
}
//////////////////////////////////////////////////////////////////////////
void loop() {
  do {
    checkStandbyAtMillis();
    mp3.loop();

#ifdef ROTARY_ENCODER
    RotEncSetVolume();
#endif

    //#ifdef CHECK_BATTERY
    //    batteryCheck();
    //#endif

#ifdef ROTARY_SWITCH
    RotSwloop(ROTARY_SWITCH_TRIGGER_TIME);
#endif

    // Buttons werden nun über JS_Button gehandelt, dadurch kann jede Taste
    // doppelt belegt werden
    readButtons();

    // Modifier : WIP!
    if (activeModifier != NULL) {
      activeModifier->loop();
    }

    // admin menu
    if ((pauseButton.pressedFor(LONG_PRESS) || upButton.pressedFor(LONG_PRESS) || downButton.pressedFor(LONG_PRESS)) && pauseButton.isPressed() && upButton.isPressed() && downButton.isPressed()) {
      mp3.pause();
      do {
        readButtons();
      } while (pauseButton.isPressed() || upButton.isPressed() || downButton.isPressed());
      readButtons();
      adminMenu();
      break;
    }

    if (pauseButton.wasReleased()) {

      if (activeModifier != NULL)
        if (activeModifier->handlePause() == true)
          return;
      if (ignorePauseButton == false)

        if (isPlaying()) {
          mp3.pause();
          setstandbyTimer();
        }
        else if (knownCard) {
          mp3.start();
          disablestandbyTimer();
        }
      ignorePauseButton = false;
    }
#ifdef PUSH_ON_OFF
    else if (pauseButton.pressedFor(LONGER_PRESS) &&
             ignorePauseButton == false) {
      ignorePauseButton = true;
      shutDown();
    }
#endif

#ifndef ROTARY_ENCODER
    if (upButton.pressedFor(LONG_PRESS)) {

#ifndef FIVEBUTTONS
      if (isPlaying()) {
        if (!mySettings.invertVolumeButtons) {
          volumeUpButton();
        }
        else {
          nextButton();
        }
      }
      else {
        playShortCut(1);
      }
      ignoreUpButton = true;
#endif
    } else if (upButton.wasReleased()) {
      if (!ignoreUpButton)
        if (!mySettings.invertVolumeButtons) {
          nextButton();
        }
        else {
          volumeUpButton();
        }
      ignoreUpButton = false;
    }
    if (downButton.pressedFor(LONG_PRESS)) {
#ifndef FIVEBUTTONS
      if (isPlaying()) {
        if (!mySettings.invertVolumeButtons) {
          volumeDownButton();
        }
        else {
          previousButton();
        }
      }
      else {
        playShortCut(2);
      }
      ignoreDownButton = true;
#endif
    } else if (downButton.wasReleased()) {
      if (!ignoreDownButton) {
        if (!mySettings.invertVolumeButtons) {
          previousButton();
        }
        else {
          volumeDownButton();
        }
      }
      ignoreDownButton = false;
    }


#ifdef FIVEBUTTONS
    if (buttonFour.wasReleased()) {
      if (isPlaying()) {
        if (!mySettings.invertVolumeButtons) {
          volumeUpButton();
        }
        else {
          nextButton();
        }
      }
      else {
        playShortCut(1);
      }
    }
    if (buttonFive.wasReleased()) {
      if (isPlaying()) {
        if (!mySettings.invertVolumeButtons) {
          volumeDownButton();
        }
        else {
          previousButton();
        }
      }
      else {
        playShortCut(2);
      }
    }
#endif

#endif

#ifdef ROTARY_ENCODER
    if (upButton.pressedFor(LONG_PRESS)) {
      if (isPlaying()) {
        nextButton();
      }

      else {
        playShortCut(1);
      }
      ignoreUpButton = true;
    }


    else if (upButton.wasReleased()) {
      if (!ignoreUpButton)
        if (isPlaying()) {
          nextButton();
        }
      ignoreUpButton = false;
    }

    if (downButton.pressedFor(LONG_PRESS)) {
      if (isPlaying()) {
        previousButton();
      }

      else {
        playShortCut(2);
      }
      ignoreDownButton = true;
    }


    else if (downButton.wasReleased()) {
      if (!ignoreDownButton)
        if (isPlaying()) {
          previousButton();
        }
      ignoreDownButton = false;
    }
#endif

    // Ende der Buttons
  } while (!mfrc522.PICC_IsNewCardPresent());

  // RFID Karte wurde aufgelegt

  if (!mfrc522.PICC_ReadCardSerial())
    return;

  if (readCard(&myCard) == true) {
    if (myCard.cookie == cardCookie && myCard.nfcFolderSettings.folder != 0 && myCard.nfcFolderSettings.mode != ModifierMode) {
      playFolder();
    }

    // Neue Karte konfigurieren
    else if (myCard.cookie != cardCookie) {
      knownCard = false;
      mp3.playMp3FolderTrack(300);
      waitForTrackToFinish();
      setupCard();
    }
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
//////////////////////////////////////////////////////////////////////////
void adminMenu(bool fromCard = false) {
  disablestandbyTimer();
  mp3.pause();
#ifdef DEBUG
  Serial.println(F("adminMenu"));
#endif
  knownCard = false;
  if (fromCard == false) {
    // Admin menu has been locked - it still can be trigged via admin card
    if (mySettings.adminMenuLocked == 1) {
      return;
    }
    // Pin check
    else if (mySettings.adminMenuLocked == 2) {
      uint8_t pin[4];
      mp3.playMp3FolderTrack(991);
      if (askCode(pin) == true) {
        if (checkTwo(pin, mySettings.adminMenuPin) == false) {
          return;
        }
      } else {
        return;
      }
    }
    // Match check
    else if (mySettings.adminMenuLocked == 3) {
      uint8_t a = random(10, 20);
      uint8_t b = random(1, 10);
      uint8_t c;
      mp3.playMp3FolderTrack(992);
      waitForTrackToFinish();
      mp3.playMp3FolderTrack(a);

      if (random(1, 3) == 2) {
        // a + b
        c = a + b;
        waitForTrackToFinish();
        mp3.playMp3FolderTrack(993);
      } else {
        // a - b
        b = random(1, a);
        c = a - b;
        waitForTrackToFinish();
        mp3.playMp3FolderTrack(994);
      }
      waitForTrackToFinish();
      mp3.playMp3FolderTrack(b);
#ifdef DEBUG
      Serial.println(c);
#endif
      uint8_t temp = voiceMenu(255, 0, 0, false);
      if (temp != c) {
        return;
      }
    }
  }
  int subMenu = voiceMenu(12, 900, 900, false, false, 0, true);
  if (subMenu == 0) {
    return;
  }
  if (subMenu == 1) {
    resetCard();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
  else if (subMenu == 2) {
    // Maximum Volume
    mySettings.maxVolume = voiceMenu(30 - mySettings.minVolume, 930, mySettings.minVolume, false, false, mySettings.maxVolume - mySettings.minVolume) + mySettings.minVolume;
  }
  else if (subMenu == 3) {
    // Minimum Volume
    mySettings.minVolume = voiceMenu(mySettings.maxVolume - 1, 931, 0, false, false, mySettings.minVolume);
  }
  else if (subMenu == 4) {
    // Initial Volume
    mySettings.initVolume = voiceMenu(mySettings.maxVolume - mySettings.minVolume + 1, 932, mySettings.minVolume - 1, false, false, mySettings.initVolume - mySettings.minVolume + 1) + mySettings.minVolume - 1;
  }
  else if (subMenu == 5) {
    // EQ
    mySettings.eq = voiceMenu(6, 920, 920, false, false, mySettings.eq);
    mp3.setEq(mySettings.eq - 1);
  }
  else if (subMenu == 6) {
    // create modifier card
    nfcTagObject tempCard;
    tempCard.cookie = cardCookie;
    tempCard.version = 1;
    tempCard.nfcFolderSettings.folder = 0;
    tempCard.nfcFolderSettings.special = 0;
    tempCard.nfcFolderSettings.special2 = 0;
    tempCard.nfcFolderSettings.mode = voiceMenu(10, 966, 966, false, false, 0, true);

    if (tempCard.nfcFolderSettings.mode != ModifierMode) {
      if (tempCard.nfcFolderSettings.mode == AudioDrama) {
        switch (voiceMenu(4, 960, 960)) {
          case 1: tempCard.nfcFolderSettings.special = 5; break;
          case 2: tempCard.nfcFolderSettings.special = 15; break;
          case 3: tempCard.nfcFolderSettings.special = 30; break;
          case 4: tempCard.nfcFolderSettings.special = 60; break;
        }
      } else if (tempCard.nfcFolderSettings.mode == Album_Section) {
        switch (voiceMenu(2, 1, 1)) {
          case 1: tempCard.nfcFolderSettings.special = 0; break;
          case 2: tempCard.nfcFolderSettings.special2 = 1; break;
        }
      }
      else if (tempCard.nfcFolderSettings.mode == Party_Section) {
        tempCard.nfcFolderSettings.special =  voiceMenu(99, 301, 0, true, 0, 0, true);
      }
      else if (tempCard.nfcFolderSettings.mode == AudioBook_Section) {
        tempCard.nfcFolderSettings.special =  voiceMenu(99, 301, 0, true, 0, 0, true);
        tempCard.nfcFolderSettings.special2 =  voiceMenu(30, 904, 0, true, 0, 0, true);
      }
      mp3.playMp3FolderTrack(800);
      do {
        readButtons();
        if (upButton.wasReleased() || downButton.wasReleased()) {
#ifdef DEBUG
          Serial.println(F("abort"));
#endif
          mp3.playMp3FolderTrack(802);
          return;
        }
      } while (!mfrc522.PICC_IsNewCardPresent());

      // RFID Karte wurde aufgelegt
      if (mfrc522.PICC_ReadCardSerial()) {
#ifdef DEBUG
        Serial.println(F("write Tag"));
#endif
        writeCard(tempCard);
        delay(100);
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        waitForTrackToFinish();
      }
    }
  }
  else if (subMenu == 7) {
    uint8_t shortcut = voiceMenu(4, 940, 940);
    setupFolder(&mySettings.shortCuts[shortcut - 1]);
    mp3.playMp3FolderTrack(400);
  }
  else if (subMenu == 8) {
    switch (voiceMenu(5, 960, 960)) {
      case 1: mySettings.standbyTimer = 5; break;
      case 2: mySettings.standbyTimer = 15; break;
      case 3: mySettings.standbyTimer = 30; break;
      case 4: mySettings.standbyTimer = 60; break;
      case 5: mySettings.standbyTimer = 0; break;
    }
  }
  else if (subMenu == 9) {
    // Create Cards for Folder
    // Ordner abfragen
    nfcTagObject tempCard;
    tempCard.cookie = cardCookie;
    tempCard.version = 1;
    tempCard.nfcFolderSettings.mode = 4;
    tempCard.nfcFolderSettings.folder = voiceMenu(99, 301, 0, true);
    uint8_t special = voiceMenu(mp3.getFolderTrackCount(tempCard.nfcFolderSettings.folder), 321, 0,
                                true, tempCard.nfcFolderSettings.folder);
    uint8_t special2 = voiceMenu(mp3.getFolderTrackCount(tempCard.nfcFolderSettings.folder), 322, 0,
                                 true, tempCard.nfcFolderSettings.folder, special);

    mp3.playMp3FolderTrack(936);
    waitForTrackToFinish();
    for (uint8_t x = special; x <= special2; x++) {
      mp3.playMp3FolderTrack(x);
      tempCard.nfcFolderSettings.special = x;
#ifdef DEBUG
      Serial.print(x);
      Serial.println(F(" place Tag"));
#endif
      do {
        readButtons();
        if (upButton.wasReleased() || downButton.wasReleased()) {
#ifdef DEBUG
          Serial.println(F("abort!"));
#endif
          mp3.playMp3FolderTrack(802);
          return;
        }
      } while (!mfrc522.PICC_IsNewCardPresent());

      // RFID Karte wurde aufgelegt
      if (mfrc522.PICC_ReadCardSerial()) {
#ifdef DEBUG
        Serial.println(F("write Tag"));
#endif
        writeCard(tempCard);
        delay(100);
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        waitForTrackToFinish();
      }
    }
  }
  else if (subMenu == 10) {
    // Invert Functions for Up/Down Buttons
    int temp = voiceMenu(2, 933, 933, false);
    if (temp == 2) {
      mySettings.invertVolumeButtons = true;
    }
    else {
      mySettings.invertVolumeButtons = false;
    }
  }
  else if (subMenu == 11) {
#ifdef DEBUG
    Serial.println(F("Reset > EEPROM wird gelöscht"));
#endif
    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.update(i, 0);
    }
    resetSettings();
    mp3.playMp3FolderTrack(999);
  }
  // lock admin menu
  else if (subMenu == 12) {
    int temp = voiceMenu(4, 980, 980, false);
    if (temp == 1) {
      mySettings.adminMenuLocked = 0;
    }
    else if (temp == 2) {
      mySettings.adminMenuLocked = 1;
    }
    else if (temp == 3) {
      int8_t pin[4];
      mp3.playMp3FolderTrack(991);
      if (askCode(pin)) {
        memcpy(mySettings.adminMenuPin, pin, 4);
        mySettings.adminMenuLocked = 2;
      }
    }
    else if (temp == 4) {
      mySettings.adminMenuLocked = 3;
    }

  }
  writeSettingsToFlash();
  setstandbyTimer();
}
//////////////////////////////////////////////////////////////////////////
bool askCode(uint8_t *code) {
  uint8_t x = 0;
  while (x < 4) {
    readButtons();
    if (pauseButton.pressedFor(LONG_PRESS))
      break;
    if (pauseButton.wasReleased())
      code[x++] = 1;
    if (upButton.wasReleased())
      code[x++] = 2;
    if (downButton.wasReleased())
      code[x++] = 3;
  }
  return true;
}
//////////////////////////////////////////////////////////////////////////
uint8_t voiceMenu(int numberOfOptions, int startMessage, int messageOffset,
                  bool preview = false, int previewFromFolder = 0, int defaultValue = 0, bool exitWithLongPress = false) {
  uint8_t returnValue = defaultValue;
  if (startMessage != 0)
    mp3.playMp3FolderTrack(startMessage);
#ifdef DEBUG
  Serial.print(F("voiceMenu "));
  Serial.print(numberOfOptions);
  Serial.println(F("Options"));
#endif
  do {
#ifdef DEBUG
    if (Serial.available() > 0) {
      int optionSerial = Serial.parseInt();
      if (optionSerial != 0 && optionSerial <= numberOfOptions)
        return optionSerial;
    }
#endif
    readButtons();
    mp3.loop();
    if (pauseButton.pressedFor(LONG_PRESS)) {
      mp3.playMp3FolderTrack(802);
      ignorePauseButton = true;
      return defaultValue;
    }
    if (pauseButton.wasReleased()) {
      if (returnValue != 0) {
#ifdef DEBUG
        Serial.print(F("= "));
        Serial.print(returnValue);
        Serial.println(F("=="));
#endif
        return returnValue;
      }
      delay(500);
    }

    if (upButton.pressedFor(LONG_PRESS)) {
      returnValue = min(returnValue + 10, numberOfOptions);
#ifdef DEBUG
      Serial.println(returnValue);
#endif
      //mp3.pause();
      mp3.playMp3FolderTrack(messageOffset + returnValue);
      waitForTrackToFinish();
      /*if (preview) {
        if (previewFromFolder == 0)
          mp3.playFolderTrack(returnValue, 1);
        else
          mp3.playFolderTrack(previewFromFolder, returnValue);
        }*/
      ignoreUpButton = true;
    } else if (upButton.wasReleased()) {
      if (!ignoreUpButton) {
        returnValue = min(returnValue + 1, numberOfOptions);
#ifdef DEBUG
        Serial.println(returnValue);
#endif
        //mp3.pause();
        mp3.playMp3FolderTrack(messageOffset + returnValue);
        if (preview) {
          waitForTrackToFinish();
          if (previewFromFolder == 0) {
            mp3.playFolderTrack(returnValue, 1);
          } else {
            mp3.playFolderTrack(previewFromFolder, returnValue);
          }
          delay(500);
        }
      } else {
        ignoreUpButton = false;
      }
    }

    if (downButton.pressedFor(LONG_PRESS)) {
      returnValue = max(returnValue - 10, 1);
#ifdef DEBUG
      Serial.println(returnValue);
#endif
      //mp3.pause();
      mp3.playMp3FolderTrack(messageOffset + returnValue);
      waitForTrackToFinish();
      /*if (preview) {
        if (previewFromFolder == 0)
          mp3.playFolderTrack(returnValue, 1);
        else
          mp3.playFolderTrack(previewFromFolder, returnValue);
        }*/
      ignoreDownButton = true;
    } else if (downButton.wasReleased()) {
      if (!ignoreDownButton) {
        returnValue = max(returnValue - 1, 1);
#ifdef DEBUG
        Serial.println(returnValue);
#endif
        //mp3.pause();
        mp3.playMp3FolderTrack(messageOffset + returnValue);
        if (preview) {
          waitForTrackToFinish();
          if (previewFromFolder == 0) {
            mp3.playFolderTrack(returnValue, 1);
          }
          else {
            mp3.playFolderTrack(previewFromFolder, returnValue);
          }
          delay(500);
        }
      } else {
        ignoreDownButton = false;
      }
    }
  } while (true);
}
//////////////////////////////////////////////////////////////////////////
void resetCard() {
  mp3.playMp3FolderTrack(800);
  do {
    pauseButton.read();
    upButton.read();
    downButton.read();

    if (upButton.wasReleased() || downButton.wasReleased()) {
#ifdef DEBUG
      Serial.print(F("Abgebrochen!"));
#endif
      mp3.playMp3FolderTrack(802);
      return;
    }
  } while (!mfrc522.PICC_IsNewCardPresent());

  if (!mfrc522.PICC_ReadCardSerial())
    return;

#ifdef DEBUG
  Serial.print(F("reset Tag"));
#endif
  setupCard();
}
//////////////////////////////////////////////////////////////////////////
bool setupFolder(folderSettings * theFolder) {
  // Ordner abfragen
  theFolder->folder = voiceMenu(99, 301, 0, true, 0, 0, true);
  if (theFolder->folder == 0) return false;

  // Wiedergabemodus abfragen
  theFolder->mode = voiceMenu(11, 305, 305, false, 0, 0, true);
  if (theFolder->mode == 0) return false;

  //  // Hörbuchmodus > Fortschritt im EEPROM auf 1 setzen
  //  writeAudiobookMemory (myFolder->folder,myFolder->special3, 1);

  // Einzelmodus > Datei abfragen
  if (theFolder->mode == Single)
    theFolder->special = voiceMenu(mp3.getFolderTrackCount(theFolder->folder), 320, 0,
                                   true, theFolder->folder);
  // Admin Funktionen
  if (theFolder->mode == AdminMenu) {
    theFolder->folder = 0;
    theFolder->mode = 255;
  }
  // Spezialmodus Von-Bis
  if (theFolder->mode == AudioDrama_Section || theFolder->mode == Album_Section || theFolder->mode == Party_Section || theFolder->mode == AudioBook_Section) {
    theFolder->special = voiceMenu(mp3.getFolderTrackCount(theFolder->folder), 321, 0,
                                   true, theFolder->folder);
    theFolder->special2 = voiceMenu(mp3.getFolderTrackCount(theFolder->folder), 322, 0,
                                    true, theFolder->folder, theFolder->special);
  }
  // Speicherplatz Hörbuch
  if (theFolder->mode == AudioBook || theFolder->mode == AudioBook_Section) {
    theFolder->special3 = voiceMenu(255, 325, 0,
                                    false, 0, 0, true);
  }

  //Puzzle oder Quiz Karte
  if (theFolder->mode == PuzzlePart ) {
    theFolder->special = voiceMenu(mp3.getFolderTrackCount(theFolder->folder), 323, 0,
                                   true, theFolder->folder);
    theFolder->special2 = voiceMenu(255, 324, 0,
                                    false, theFolder->folder);

  }
  return true;
}
//////////////////////////////////////////////////////////////////////////
void setupCard() {
  mp3.pause();
#ifdef DEBUG
  Serial.println(F("setupCard"));
#endif
  nfcTagObject newCard;
  if (setupFolder(&newCard.nfcFolderSettings) == true)
  {
    // Karte ist konfiguriert > speichern
    mp3.pause();
    do {
    } while (isPlaying());
    writeCard(newCard);
  }
  delay(1000);
}

bool readCard(nfcTagObject * nfcTag) {
  nfcTagObject tempCard;
  // Show some details of the PICC (that is: the tag/card)
#ifdef DEBUG
  Serial.println(F("Card UID "));
#endif
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
#ifdef DEBUG
  Serial.print(F("PICC type "));
#endif
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
#ifdef DEBUG
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
#endif

  byte buffer[18];
  byte size = sizeof(buffer);

  // Authenticate using key A
  if ((piccType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
#ifdef DEBUG
    Serial.println(F("Authenticating Classic using key A..."));
#endif
    status = mfrc522.PCD_Authenticate(
               MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  }
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the tempCard

    // Authenticate using key A
#ifdef DEBUG
    Serial.println(F("Authenticating MIFARE UL..."));
#endif
    status = mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
  }

  if (status != MFRC522::STATUS_OK) {
#ifdef DEBUG
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
#ifdef DEBUG
    Serial.print(F("Reading data block"));
    Serial.print(blockAddr);
    Serial.println(F("..."));
#endif
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
#ifdef DEBUG
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
#ifdef DEBUG
      Serial.print(F("MIFARE_Read_1 failed "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer, buffer2, 4);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(9, buffer2, &size2);
    if (status != MFRC522::STATUS_OK) {
#ifdef DEBUG
      Serial.print(F("MIFARE_Read_2 failed "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer + 4, buffer2, 4);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(10, buffer2, &size2);
    if (status != MFRC522::STATUS_OK) {
#ifdef DEBUG
      Serial.print(F("MIFARE_Read_3 failed "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer + 8, buffer2, 4);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(11, buffer2, &size2);
    if (status != MFRC522::STATUS_OK) {
#ifdef DEBUG
      Serial.print(F("MIFARE_Read_4 failed "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer + 12, buffer2, 4);
  }

#ifdef DEBUG
  Serial.print(F("Data on Card : "));
#endif
  dump_byte_array(buffer, 16);

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
        return false;
      }
    }
    if (tempCard.nfcFolderSettings.folder == 0) {

      return SetModifier (tempCard.nfcFolderSettings.mode, tempCard.nfcFolderSettings.special, tempCard.nfcFolderSettings.special2);

      /*if (activeModifier != NULL) {
        if (activeModifier->getActive() == tempCard.nfcFolderSettings.mode) {
          activeModifier = NULL;
        #ifdef DEBUG
          Serial.println(F("modifier removed"));
        #endif
          if (isPlaying()) {
            mp3.playAdvertisement(261);
          }
          else {
            //            mp3.start();
            //            delay(100);
            //            mp3.playAdvertisement(261);
            //            delay(100);
            //            mp3.pause();
            mp3.pause();
            delay(500);
            mp3.playMp3FolderTrack(261);
            waitForTrackToFinish();
          }
          //delay(2000);
          return false;
        }
        }
        if (tempCard.nfcFolderSettings.mode != Modifier && tempCard.nfcFolderSettings.mode != 255) {
        if (isPlaying()) {
          mp3.playAdvertisement(260);
        }
        else {
          //          mp3.start();
          //          delay(100);
          //          mp3.playAdvertisement(260);
          //          delay(100);
          //          mp3.pause();
          //mp3.pause();
          //delay(500);
          mp3.playMp3FolderTrack(260);
          //delay(1000);
          waitForTrackToFinish();
        }
        }
        delay(2000);
        switch (tempCard.nfcFolderSettings.mode ) {
        case 0:
        case 255:
          mfrc522.PICC_HaltA(); mfrc522.PCD_StopCrypto1(); adminMenu(true);  break;
        case 1: activeModifier = new SleepTimer(tempCard.nfcFolderSettings.special); break;
        case 2: activeModifier = new FreezeDance(); break;
        case 3: activeModifier = new Locked(); break;
        case 4: activeModifier = new ToddlerMode(); break;
        case 5: activeModifier = new KindergardenMode(); break;
        case 6: activeModifier = new RepeatSingleModifier(); break;
        case 7: activeModifier = new FeedbackModifier(); break;
        case 8: activeModifier = new PuzzleGame(tempCard.nfcFolderSettings.special); break;
        case 9: activeModifier = new QuizGame(tempCard.nfcFolderSettings.special); break;
        case 10: activeModifier = new ButtonSmash(tempCard.nfcFolderSettings.special, tempCard.nfcFolderSettings.special2); break;
        }
        //delay(2000);
        return false;*/
    }
    else {
      memcpy(nfcTag, &tempCard, sizeof(nfcTagObject));
#ifdef DEBUG
      Serial.println( nfcTag->nfcFolderSettings.folder);
#endif
      myFolder = &nfcTag->nfcFolderSettings;
#ifdef DEBUG
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
//////////////////////////////////////////////////////////////////////////
void writeCard(nfcTagObject nfcTag) {
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

  mifareType = mfrc522.PICC_GetType(mfrc522.uid.sak);

  // Authenticate using key B
  //authentificate with the card and set card specific parameters
  if ((mifareType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
#ifdef DEBUG
    Serial.println(F("Authenticating again using key A..."));
#endif
    status = mfrc522.PCD_Authenticate(
               MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  }
  else if (mifareType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the NFCtag

    // Authenticate using key A
#ifdef DEBUG
    Serial.println(F("Authenticating UL..."));
#endif
    status = mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
  }

  if (status != MFRC522::STATUS_OK) {
#ifdef DEBUG
    Serial.print(F("PCD_Authenticate failed "));
    Serial.println(mfrc522.GetStatusCodeName(status));
#endif
    mp3.playMp3FolderTrack(401);
    waitForTrackToFinish();
    return;
  }

  // Write data to the block
#ifdef DEBUG
  Serial.print(F("Writing data "));
  Serial.print(blockAddr);
  Serial.println(F("..."));
#endif
  dump_byte_array(buffer, 16);
#ifdef DEBUG
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
  }

  if (status != MFRC522::STATUS_OK) {
#ifdef DEBUG
    Serial.print(F("MIFARE_Write failed "));
    Serial.println(mfrc522.GetStatusCodeName(status));
#endif
    mp3.playMp3FolderTrack(401);
  }
  else
    mp3.playMp3FolderTrack(400);
#ifdef DEBUG
  Serial.println();
#endif
  waitForTrackToFinish();
  //delay(2000);
}
//////////////////////////////////////////////////////////////////////////
/**
  Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
#ifdef DEBUG
    Serial.print(buffer[i] < 0x10 ? "0" : " ");
    Serial.print(buffer[i], HEX );
    Serial.print(" ");
#endif
  }
#ifdef DEBUG
  Serial.println();
#endif
}
///////////////////////////////////////// Check Bytes   ///////////////////////////////////
bool checkTwo ( uint8_t a[], uint8_t b[] ) {
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
      return false;
    }
  }
  return true;
}
//////////////////////////////////////////////////////////////////////////
#ifdef ROTARY_ENCODER
void RotEncSetVolume () {
  RotEncPos += encoder.getValue();

  if ((RotEncPos >= RotEncOldEncPos + 2) || (RotEncPos <= RotEncOldEncPos - 2))  {
    if (RotEncPos >= RotEncOldEncPos + 2) {
      RotEncPos = RotEncPos - 1;
      RotEncOldEncPos = RotEncPos;
    }
    else if (RotEncPos <= RotEncOldEncPos - 2) {
      RotEncPos = RotEncPos + 1;
      RotEncOldEncPos = RotEncPos;
    }
    if (RotEncPos > (mySettings.maxVolume)) {
      volume  = mySettings.maxVolume;
      RotEncPos  = mySettings.maxVolume;
    }
    else if (RotEncPos < (mySettings.minVolume)) {
      volume  = mySettings.minVolume;
      RotEncPos  = mySettings.minVolume;
    }
    else   {
      volume = RotEncPos;
    }
    mp3.setVolume(volume);
  }
}

void timerIsr() {
  encoder.service();
}
#endif

#ifdef ROTARY_SWITCH
uint8_t RotSwGetPosition () {
  float analogValue = (analogRead(ROTARY_SWITCH_PIN) * ROTARY_SWITCH_REF_VOLTAGE) / 1024.0;

  for (uint8_t x = 0; x <= ROTARY_SWITCH_POSITIONS; x++)
  {
    if (analogValue >= (ROTARY_SWITCH_STEP_MIN * x) && analogValue <= (ROTARY_SWITCH_STEP_MAX * x)) {
      return x + 1;
    }
  }
}

void RotSwloop(int TriggerTime) {

  if (RotSwCurrentPos != RotSwGetPosition()) {
    RotSwCurrentPos = RotSwGetPosition();

    if (TriggerTime > 0)
      RotSwMillis = millis();

  } else if ((millis() > (RotSwMillis + TriggerTime))  && (RotSwCurrentPos != RotSwActivePos)) {
    RotSwSet(RotSwCurrentPos);
  }
}

void RotSwSet (uint8_t RotarySwitchPosition) {
  folderSettings myRotSw;
  RotSwActivePos = RotarySwitchPosition;
#ifdef DEBUG
  Serial.print("RotSw active Pos ");
  Serial.println(RotSwActivePos);
#endif

  if (RotSwMap[RotarySwitchPosition - 1][0] > 0) {

    myRotSw.folder = RotSwMap[RotarySwitchPosition - 1][0];
    myRotSw.mode = RotSwMap[RotarySwitchPosition - 1][1];
    myRotSw.special = RotSwMap[RotarySwitchPosition - 1][2];
    myRotSw.special2 = RotSwMap[RotarySwitchPosition - 1][3];
    myFolder = &myRotSw;
    playFolder();

  } else if (RotSwMap[RotarySwitchPosition - 1][0] == 0) {
    SetModifier (RotSwMap[RotarySwitchPosition - 1][ 1], RotSwMap[RotarySwitchPosition - 1][2], RotSwMap[RotarySwitchPosition - 1][3]);
  } else if (RotSwMap[RotarySwitchPosition - 1][0] == -1) {
    switch (RotSwMap[RotarySwitchPosition - 1][1]) {
      case 1: //Pause
        if (activeModifier != NULL)
          if (activeModifier->handlePause() == true)
            return;
        if (ignorePauseButton == false)

          if (isPlaying()) {
            mp3.pause();
            setstandbyTimer();
          }
        break;
      case 2: //Volume
        volume = RotSwMap[RotarySwitchPosition - 1][2];
        mp3.setVolume(volume);
        break;
      case 3: //Forward
        nextButton();
        break;
      case 4: //Backward
        previousButton();
        break;
      case 5: //ShutDown
        shutDown();
      case 6: //Remove Modifier
        if (activeModifier != NULL)
          RemoveModifier();
        break;
    }
  }
}
#endif
//////////////////////////////////////////////////////////////////////////

bool SetModifier (uint8_t tmpMode, uint8_t tmpSpecial1, uint8_t tmpSpecial2) {
  if (activeModifier != NULL) {
    if (activeModifier->getActive() == tmpMode) {
      return RemoveModifier();
    }
  }
  if (tmpMode != ModifierMode && tmpMode != AdminMenuMod) {
    if (isPlaying()) {
      mp3.playAdvertisement(260);
    }
    else {
      //          mp3.start();
      //          delay(100);
      //          mp3.playAdvertisement(260);
      //          delay(100);
      //          mp3.pause();
      //mp3.pause();
      //delay(500);
      mp3.playMp3FolderTrack(260);
      //delay(1000);
      waitForTrackToFinish();
    }
  }
  delay(2000);
  switch (tmpMode) {
    case 0:
    case 255:
      mfrc522.PICC_HaltA(); mfrc522.PCD_StopCrypto1(); adminMenu(true);  break;
    case 1: activeModifier = new SleepTimer(tmpSpecial1); break;
    case 2: activeModifier = new FreezeDance(); break;
    case 3: activeModifier = new Locked(); break;
    case 4: activeModifier = new ToddlerMode(); break;
    case 5: activeModifier = new KindergardenMode(); break;
    case 6: activeModifier = new RepeatSingleModifier(); break;
    case 7: activeModifier = new FeedbackModifier(); break;
    case 8: activeModifier = new PuzzleGame(tmpSpecial1); break;
    case 9: activeModifier = new QuizGame(tmpSpecial1); break;
    case 10: activeModifier = new ButtonSmash(tmpSpecial1, tmpSpecial2); break;
  }
  //delay(2000);
  return false;
}

bool RemoveModifier() {
  activeModifier = NULL;
#ifdef DEBUG
  Serial.println(F("modifier removed"));
#endif
  if (isPlaying()) {
    mp3.playAdvertisement(261);
  }
  else {
    //            mp3.start();
    //            delay(100);
    //            mp3.playAdvertisement(261);
    //            delay(100);
    //            mp3.pause();
    mp3.pause();
    delay(500);
    mp3.playMp3FolderTrack(261);
    waitForTrackToFinish();
  }
  //delay(2000);
  return false;
}

uint8_t readAudiobookMemory (uint8_t folder, uint8_t memoryNumber) {
  return EEPROM.read(folder + (99 * memoryNumber));
}

void writeAudiobookMemory (uint8_t folder, uint8_t memoryNumber, uint8_t track) {
  EEPROM.update(folder + (99 * memoryNumber), track);
}
