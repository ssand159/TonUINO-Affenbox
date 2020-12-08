///////// uncomment the below line to enable the function ////////////////
//#define FIVEBUTTONS
#define DEBUG //Debug Ausgaben in der Konsole
//#define DEBUG_QUEUE //Debug Ausgabe der Queue
//#define PUSH_ON_OFF //Ein Ausschalten des TonUINO
//#define STARTUP_SOUND
//#define SPEAKER_SWITCH
//#define ROTARY_ENCODER
//#define ANALOG_INPUT //old ROTARY_SWITCH
//#define ROBOTDYN_3X4 //Ersetzt die Auswertung des ANALOG_INPUT, durch eine für die Robotdyn 3x4 Matrixtastatur angepasste. ANALOG_INPUT muss zusätzlich aktiviert sein!
//#define POWER_ON_LED
//#define FADING_LED //Experimentell, nur in Verbindung mit POWER_ON_LED
//#define DEVELOPER_MODE //Löscht den EEPROM bei jedem Start
//////////////////////////////////////////////////////////////////////////

///////// conifguration of the input and output pins //////////////////////
#define buttonPause A0 //Default A0; Pocket A2
#define buttonUp A1 //Default A1; Pocket A0
#define buttonDown A2 //Default A2; Pocket A1
#define busyPin 4

#define shutdownPin 7 //Default 7

#define openAnalogPin A7 //Default A7, muss ein unbelegeter, analoger Eingang sein

#ifdef FIVEBUTTONS
#define buttonFourPin A3
#define buttonFivePin A4
#endif

#define RST_PIN 9                 // Configurable, see typical pin layout above
#define SS_PIN 10                 // Configurable, see typical pin layout above

#ifdef SPEAKER_SWITCH
#define SpeakerOnPin 8
#endif

#ifdef POWER_ON_LED
#define PowerOnLEDPin 6
#endif

#ifdef ANALOG_INPUT
#define ANALOG_INPUT_PIN  A6
//#define ANALOG_INPUT_SUPPLY_PIN 6 //Der Referenzpegel kann auch von einem freien Output Pin kommen
#endif

#ifdef ROTARY_ENCODER
#define ROTARY_ENCODER_PIN_A 5 //Default 5; 
#define ROTARY_ENCODER_PIN_B 6 //Default 6; 
#define ROTARY_ENCODER_PIN_SUPPLY 8 //uncomment if you want to use an IO pin as supply
#endif
//////////////////////////////////////////////////////////////////////////

////////// Button timings ////////////////////////////////////////////////
#define LONG_PRESS 1000
#define LONGER_PRESS 2000
#define LONGEST_PRESS 5000
//////////////////////////////////////////////////////////////////////////

////////// NFC Gain //////////////////////////////////////////////////////
//#define NFCgain_max   // Maximale Empfindlichkeit
#define NFCgain_avg   // Mittlere Empfindlichkeit
//#define NFCgain_min   // Minimale Empfindlichkeit
//////////////////////////////////////////////////////////////////////////

///////// conifguration of the rotary encoder ////////////////////////////
#ifdef ROTARY_ENCODER
#define ROTARY_ENCODER_STEPS 4
#endif
//////////////////////////////////////////////////////////////////////////

///////// conifguration of the analog input ////////////////////////////
#ifdef ANALOG_INPUT
#define ANALOG_INPUT_POSITIONS 10
#define ANALOG_INPUT_TOLERNACE 0.15
#define ANALOG_INPUT_REF_VOLTAGE 5.0
#define ANALOG_INPUT_RES_TO_GND 1 //Anzahl Widerstände zwischen der erten Stufe und GND
#define ANALOG_INPUT_RES_TO_VCC 1 //Anzahl Widerstände zwischen der letzten Stufe und VCC
#define ANALOG_INPUT_TRIGGER_TIME 2000
#endif
//////////////////////////////////////////////////////////////////////////
