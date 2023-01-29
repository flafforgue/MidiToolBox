
#ifndef _LCDSHIELD_H_
#define _LCDSHIELD_H_

#include <Arduino.h>
#include <inttypes.h>

#include <LiquidCrystal.h>

// Définir les pins utilisées par le LCD
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// ------------------------------------------------------------------------------------ //
//                                Retro Eclairage                                       //
// ------------------------------------------------------------------------------------ //

void RetoOn() {
  digitalWrite(10,true);
}

void RetoOff() {
  digitalWrite(10,false);
}

// ------------------------------------------------------------------------------------ //
//                                      Buttons                                         //
// ------------------------------------------------------------------------------------ //

#define LCD_BTN_PIN 0

#define KEY_NONE   0
#define KEY_RIGHT  1
#define KEY_UP     2
#define KEY_DOWN   3
#define KEY_LEFT   4
#define KEY_SELECT 5

int LCDKeyWait  = KEY_NONE;
int LCDKeyPress = KEY_NONE;

// ------------------------------------------------------------------------------------ //

int key;

// ------------------------------------------------------------------------------------ //
//                                Fonctions Buttons                                     //
// ------------------------------------------------------------------------------------ //

int LcdButtonValue() {
  int temp = analogRead(LCD_BTN_PIN);
  if (temp > 1000) return KEY_NONE;   // 
  if (temp < 50)   return KEY_RIGHT;  //   0 
  if (temp < 180)  return KEY_UP;     //  98
  if (temp < 330)  return KEY_DOWN;   // 255
  if (temp < 525)  return KEY_LEFT;   // 409
  if (temp < 830)  return KEY_SELECT; // 640
  return KEY_NONE;
}

int LcdButtonUpdate() {
  int temp=LcdButtonValue();

  if ( LCDKeyWait != temp ) {
    if (LCDKeyWait == KEY_NONE ) {
      LCDKeyWait=temp;
    } else {
      LCDKeyPress=LCDKeyWait;
      LCDKeyWait =KEY_NONE;
    }
    delay(20);
  }
}

bool LcdKeyPressed () {
  return (LCDKeyPress != KEY_NONE);
}

int LcdReadKey() {
  int temp=LCDKeyPress;
  LCDKeyWait =KEY_NONE;
  LCDKeyPress=KEY_NONE;
  return temp;
}

#endif
