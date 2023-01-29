
#include "LcdShield.h"

// ------------------------------------------------------------------------------------ //
//                                Utilisation Broches                                   //
// ------------------------------------------------------------------------------------ //
// 
// D00 - Rx  - Midi In
// D01 - Tx  - Midi out
// D02 -     - Rot 1
// D03 ~     - Rot 2
// D04 - LCD - Db4
// D05 ~ LCD - Db5
// D06 ~ LCD - Db6
// D07 - LCD - Db7
// D08 - LCD - RS
// D09 ~ LCD - E
// D10 ~     - Retro eclairage ?
// D11 ~     - Rot Btn
// D12 -     - Btn
// D13 -     - Led Carte
// 
// A0  - LCD - Btn
// A1  -
// A2  -
// A3  -
// A4  - SDA (I2C)
// A5  - SCL (I2C)
// 

// --------------------------------------------------------------------------------- //
// Parametres Boutons                                                                //
// --------------------------------------------------------------------------------- //

#define ROT1 2
#define ROT2 3
#define ROTB 11   // Rotary Button
#define BTN1 12
#define LED1 13

// --------------------------------------------------------------------------------- //
//                                 Boutons                                           //
// --------------------------------------------------------------------------------- //

boolean Btn1Down =false;
boolean Btn1Press=false;

boolean ROTBDown =false;
boolean ROTBPress=false;

void CheckButtons () {  
  // ------------------  test bouton 1 ------------------
  if ( Btn1Down ) {
    if (digitalRead(BTN1)==true) { // bouton plus enfoncé
      Btn1Down=false;
      delay(10);
    }
  } else {
    if (digitalRead(BTN1)==false) { // bouton enfoncé
      Btn1Down=true;
      Btn1Press=true;
    }    
  }
   // -------------  test bouton 2 ( Rotary ) ------------
  if ( ROTBDown ) {
    if (digitalRead(ROTB)==true) { // bouton plus enfoncé
      ROTBDown=false;
      delay(10);
    }
  } else {
    if (digitalRead(ROTB)==false) { // bouton enfoncé
      ROTBDown=true;
      ROTBPress=true;
    }    
  }
}

boolean keyPress1() {
  boolean tmp;
  tmp=Btn1Press;
  Btn1Press=false;
  return tmp; 
}

boolean keyPress2() {
  boolean tmp;
  tmp=ROTBPress;
  ROTBPress=false;
  return tmp; 
}

/* ---------------------------------------------------------------------- */
/*                          Buttons Functions                             */
/* ---------------------------------------------------------------------- */

#define BtnPin     0

#define btnRIGHT   1
#define btnUP      2
#define btnDOWN    3
#define btnLEFT    4
#define btnSELECT  5
#define btnNONE    0

boolean keypressed = false;
boolean keyarmed   = false;
int     keywait    = btnNONE;
int     akey       = btnNONE;

int keyvalue(int value) {
  if (value > 1000) return btnNONE; 
  if (value < 50)   return btnRIGHT;  
  if (value < 177)  return btnUP; 
  if (value < 332)  return btnDOWN; 
  if (value < 524)  return btnLEFT; 
  if (value < 831)  return btnSELECT;    
  return btnNONE;
}

void Updatekey(void) {
  int temp;
  
  temp = keyvalue(analogRead(BtnPin));
  if ( keyarmed ) {
    if ( temp==btnNONE ) {
      keyarmed   = false;
      keypressed = true;
      akey       = keywait;
      keywait    = btnNONE;
      delay(100);
    }
  } else {
    if ( temp!=btnNONE ) {
      keyarmed   = true;
      keypressed = false;

      keywait    = temp;
      delay(50);
    }
  }
}

int readkey(void) {
  int temp;
  
  keypressed = false;
  temp       = akey;  
  key        = btnNONE;
  return  temp;
}

// --------------------------------------------------------------------------------- //
//  Rotary Encoder                                                                   //
// --------------------------------------------------------------------------------- //

unsigned long ltime;
int Compteur = 0;

void RotaryChange() {
  unsigned long ntime;
  int increment=0;
  
  ntime=millis();
  if ((ntime-ltime) < 100 ) {
    if (digitalRead(ROT1) == digitalRead(ROT2)) {
      increment= -1;   
     } else {
      increment= +1;   
     }
  }
  Compteur   = Compteur   + increment;           
  ltime=ntime;
}

// --------------------------------------------------------------------------------- //
//                                  Send MIDI Commands                               //
// --------------------------------------------------------------------------------- //

#define NOTEOFF B10000000 // note off
#define NOTEON  B10010000 // note on
#define AFTERT  B10100000 // aftertouch
#define CCONTRL B10110000 // continuous controller
#define PATCHCH B11000000 // patch change
#define CHANELP B11010000 // channel pressure
#define PITCHBE B11100000 // pitch bend
#define OTHERCD B11110000 // non-musical commands


void NoteOff(byte chanel, byte note, byte velocity) {
  Serial.write(NOTEOFF + chanel);
  Serial.write(note);
  Serial.write(velocity);
}

void NoteOn(byte chanel, byte note, byte velocity) {
  Serial.write(NOTEON + chanel);
  Serial.write(note);
  Serial.write(velocity);
}

void PitchBend(byte chanel,word value) {
  Serial.write(PITCHBE + chanel);
  Serial.write(value && 0x7F);
  Serial.write(value >> 7);  
}

// --------------------------------------------------------------------------------- //
//                                  Get MIDI Commands                               //
// --------------------------------------------------------------------------------- //

byte    MidiCmd;
byte    MidiChanel;
byte    MidiNote;
byte    MidiVelocity;

void getMidi(){
  byte Tmp;
  
  MidiCmd      = 0;
  MidiChanel   = 0;
  MidiNote     = 0;
  MidiVelocity = 0;
  
  if (Serial.available() > 1) {
    Tmp = Serial.read();   // read first byte
    if ( (Tmp & B11110000 ) == B11110000 ) {       // Sysex
      MidiCmd = Tmp & B11110000;
      if ( Tmp == 0xF0 ) {                         // Sysex start
        while ( Tmp != 0xF7 ) {
          Tmp = Serial.read();
        }
      }
    } else if ( (Tmp & B10000000 ) == B10000000 ) { // commande
      digitalWrite(LED1,true);
      MidiCmd        = Tmp & B11110000;
      MidiChanel     = Tmp & B00001111;
      MidiNote       = Serial.read();

      if ( ( Tmp & B11100000 ) != B11000000 ){ // Program Change or After Touch chanel have only 2 bytes
        while ( Serial.available() == 0 ) { }
        MidiVelocity   = Serial.read();  
      } 
      digitalWrite(LED1,false);
    }
  }
}

// --------------------------------------------------------------------------------- //
//                               Display MIDI Commands                               //
// --------------------------------------------------------------------------------- //

char line0[16];
char line1[16];

void DisplayMidi() {
  char MCmd[5];

  getMidi();
  if ( MidiCmd != 0 ) {
    switch (MidiCmd) {
      case NOTEOFF:
        strcpy(MCmd,"OFF ");
        break;
      case NOTEON:
        strcpy(MCmd,"ON  ");
        break;
      case AFTERT:
        strcpy(MCmd,"Aftr");
        break;
      case CCONTRL:
        strcpy(MCmd,"CCrt");
        break;
      case PATCHCH:
        strcpy(MCmd,"PATC");
        break;
      case CHANELP:
        strcpy(MCmd,"CHaP");
        break;
      case PITCHBE:
        strcpy(MCmd,"Pith");
        break;
      default:
        sprintf(MCmd,"%03d ",MidiCmd);
    }    
    sprintf(line1,"%4s %03d %03d %03d",MCmd , MidiChanel, MidiNote, MidiVelocity);
//    sprintf(line1,"%3d %03d %03d %03d",MidiCmd , MidiChanel, MidiNote, MidiVelocity);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(line0);
    
    lcd.setCursor(0,1);    
    lcd.print(line1);
    
    strcpy(line0,line1);
  }  
}

// --------------------------------------------------------------------------------- //
//                                   M E N U S                                       //
// --------------------------------------------------------------------------------- //

void Display() {
  boolean Run=true;   
  lcd.clear();
  lcd.print("any key to stop");
  while (Run) {      
    DisplayMidi();
    
    CheckButtons ();
    if (keyPress1()) {
      NoteOn(3,0x40,0x70);
      digitalWrite(LED1,true);
      delay(100);
      digitalWrite(LED1,false);  
    }

    if (keyPress2()) {
      NoteOff(3,0x40,0x00);
      digitalWrite(LED1,true);
      delay(100);
      digitalWrite(LED1,false);    
    }
   
    Updatekey();
    if (keypressed) {    
      readkey();
      Run=false;   
    }
  }
}

// --------------------------------------------------------------------------------- //

void Send() {
  boolean Run=true;   
  lcd.clear();
  lcd.print("any key to stop");
  Compteur   = 0;
  byte oNote = 0;
  byte Note  = 0x40;
  
  lcd.setCursor(0,1);
  lcd.print("Note : ");
      
  while (Run) {      
//    noInterrupts ();
    Note     = 0x40 + Compteur;
//    Compteur = 0;
//    interrupts ();

    if (oNote != Note ) { 
      oNote = Note;
      lcd.setCursor(8,1);
      lcd.print(Note); 
    }
        
    CheckButtons ();
    if (keyPress1()) {
      NoteOn(3,Note,0x70);
      digitalWrite(LED1,true);
      delay(250);
      digitalWrite(LED1,false);    
      NoteOff(3,Note,0x00);      
    }

    if (keyPress2()) {
      NoteOn(3,Note+2,0x70);
      digitalWrite(LED1,true);
      delay(100);
      digitalWrite(LED1,false);    
    }
   
    Updatekey();
    if (keypressed) {    
      readkey();
      Run=false;   
    }
  }
}

// --------------------------------------------------------------------------------- //

#define NbNotes 144 
byte  Note [NbNotes] =  
{ 62, 60, 62, 0,  62, 0,  62, 64, 65, 0,  65, 0,  65, 67, 64, 0,  64, 0,  62, 60, 60, 62, 0,  0,
  57, 60, 62, 0,  62, 0,  62, 64, 65, 0,  65, 0,  65, 67, 64, 0,  64, 0,  62, 60, 62, 0,  0,  0,
  57, 60, 62, 0,  62, 0,  62, 65, 67, 0,  67, 0,  67, 69, 70, 0,  70, 0,  69, 0,  69, 62, 0,  0,
  62, 65, 64, 0,  64, 0,  65, 0,  64, 0,  0,  0,  69, 72, 74, 0,  74, 0,  74, 76, 77, 0,  77, 0,
  77, 79, 76, 0,  76, 0,  74, 72, 72, 74, 0,  0,  69, 72, 74, 0,  74, 0,  74, 76, 77, 0,  77, 0,
  77, 79, 76, 0,  76, 0,  74, 72, 74, 0,  0,  0,  69, 72, 74, 0,  74, 0,  69, 72, 64, 0,  64, 0 };
  
#define Tempo  140
#define TempoL  80

void Demo() {
  lcd.clear();
  lcd.print("Demo");
  for (int i=0; i<NbNotes; i++ ) {
      byte Nt = Note[i];
      if ( Nt>0 ) { NoteOn(3, Nt ,0x70); }
      digitalWrite(LED1,true);
      delay(TempoL);
      digitalWrite(LED1,false);    
      delay(Tempo-TempoL-10);
      if ( Nt>0 ) { NoteOff(3, Nt ,0x00); }   
  }
  delay(500);
}

// --------------------------------------------------------------------------------- //
//                                  S E T U P                                        //
// --------------------------------------------------------------------------------- //

void setup() {
  pinMode(ROT1,INPUT_PULLUP);
  pinMode(ROT2,INPUT_PULLUP);
  pinMode(ROTB,INPUT_PULLUP);
  attachInterrupt(0, RotaryChange, CHANGE);

  pinMode(BTN1,INPUT_PULLUP);
  pinMode(LED1,OUTPUT);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("MidiBox     In <");
  lcd.setCursor(0,1);
  lcd.print("V0.20a     Out >");
  delay(1500);
    
  Serial.begin(31250);
  Serial.setTimeout(500);
  strcpy(line0," ");
}

// --------------------------------------------------------------------------------- //
//                                  L O O P                                          //
// --------------------------------------------------------------------------------- //

boolean change   = true;
int     menu     = 0;

void loop(){
  float v = 0;

  // display menu
  if ( change ) {
    change=false;
    lcd.clear ();
    lcd.setCursor(0,0);
    switch (menu) {
      case 0 :  lcd.print("Ecoute");
                lcd.setCursor(0,1);
                lcd.print("<Sel> to Start");
                break; 
      case 1 :  lcd.print("Send");
                lcd.setCursor(0,1);
                lcd.print("<Sel> to Start");
                break;
      case 2 :  lcd.print("Demo");
                lcd.setCursor(0,1);
                lcd.print("<Sel> to Start");
                break;
    }
  }

  // traitement touches
  Updatekey();
  if (keypressed) {
    change=true;
    switch (readkey()) {
      case btnDOWN :   menu++;
                       break; 
      case btnUP :     menu--;
                       break;
      case btnLEFT :   
                       break;
      case btnRIGHT :  
                       break;
      case btnSELECT:  if (menu==0) {
                          Display();
                          change=true;
                       }
                       if (menu==1) {
                          Send();
                          change=true;
                       }
                       if (menu==2) {
                          Demo();
                          change=true;
                       }
                       break;    
    }
  if (menu<0) menu=0;
  if (menu>2) menu=2;  
  }
}
