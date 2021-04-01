///////// uncomment the below line to enable the function ////////////////

//#define AiO

/////!! NICHT ENTFERNEN!!/////
#if not defined AiO
#define VERSION_TONUINO 2.13
#else
#define VERSION_TONUINO 2.23
#endif
//////////////////////////////

//#define FIVEBUTTONS  //Die AiO verwendet Standardmäßig fünf Buttons, vier Analoge eingänge und einen separaten für Pause. Falls ANALOG_INPUT verwendet werden soll muss FIVEBUTTONS deaktiviert werden um einen analogen Eingang frei zu machen. 
#define DEBUG        //Erweiterte Debug Ausgaben in der Konsole

//#define PUSH_ON_OFF     //Ein Ausschalten des TonUINO //mit AiO nicht mehr nötig, da standardmäßig vorhanden
//#define SPEAKER_SWITCH  //mit AiO nicht mehr als externe Hardware nötig, da standardmäßig vorhanden
//#define POWER_ON_LED
//#define FADING_LED      //nur in Verbindung mit POWER_ON_LED
//#define ANALOG_INPUT    //Programmierbarer Analoger Eingang, setzt entsprechende Hardware vorraus
//#define IRREMOTE
#define ROTARY_ENCODER    //

//////////////////////////////////////////////////////////////////////////

///////// conifguration of the input and output pins //////////////////////
#define buttonPause A0 //Default A0; Pocket A2
#define buttonUp A1 //Default A1; AiO A4; Pocket A0
#define buttonDown A2//Default A2; AiO A3; Pocket A1
#define busyPin 4

#define shutdownPin 7 //Default 7; AiO 7

#define openAnalogPin A7 //Default A7, muss ein unbelegeter, analoger Eingang sein

#ifdef FIVEBUTTONS
#define buttonFourPin A3 //Default A3; AiO A2
#define buttonFivePin A4 //Default A4; AiO A1
#endif

#define RST_PIN 9                 // Configurable, see typical pin layout above
#define SS_PIN 10                 // Configurable, see typical pin layout above

#if defined SPEAKER_SWITCH || defined AiO
#define SpeakerOnPin 8
#endif

#ifdef POWER_ON_LED
#define POWER_ON_LED_PIN 5
#endif

#ifdef ANALOG_INPUT
#define ANALOG_INPUT_PIN  A4
//#define ANALOG_INPUT_SUPPLY_PIN 6 //Nur verwenden, wenn es unbedingt nötig ist!Der Referenzpegel kann auch von einem IO Pin kommen.
#endif

#if defined IRREMOTE
#define IRREMOTE_PIN 6                    // pin used for the ir receiver
#endif

#ifdef ROTARY_ENCODER
#define ROTARY_ENCODER_PIN_A A3  
#define ROTARY_ENCODER_PIN_B A4  
//#define ROTARY_ENCODER_PIN_SUPPLY 8 //Nur verwenden, wenn es unbedingt nötig ist! Die Versorgung des Rotary Encoder kann auch von einem IO Pin kommen-
#endif
//////////////////////////////////////////////////////////////////////////

////////// Button timings ////////////////////////////////////////////////
#define SHORT_PRESS 50
#define LONG_PRESS 750
#define LONGER_PRESS 1500 
#define LONGEST_PRESS 3000
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
///////// conifguration of the analog input ////////////////////////////

//#define ANALOG_INPUT_PRINT //Aktiviert zusätzliche Debug Ausgaben im seriellen Moitor
#define ROTARY_ENCODER_PRINT
//#define SHORTCUTS_PRINT
//#define QUEUE_PRINT  //Debug Ausgabe der Queue
//#define DFPLAYER_PRINT
//#define IRREMOTE_PRINT

//////////////////////////////////////////////////////////////////////////
