/*
 * ht16k33.cpp - used to talk to the htk1633 chip to do things like turn on LEDs or scan keys
 * Copyright:  Peter Sjoberg <peters-alib AT techwiz.ca>
 * License: GPLv3
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3 as 
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * History:
 * 2015-10-04  Peter Sjoberg <peters-alib AT techwiz.ca>
 *             Created using https://www.arduino.cc/en/Hacking/LibraryTutorial and ht16k33 datasheet
 * 2015-11-25  Peter Sjoberg <peters-alib AT techwiz DOT ca>
 *             first check in to github
 * 2016-08-09  René Wennekes <rene.wennekes AT gmail.com>
 *             Contribution of 7-segment & 16-segment display support
 *             Added clearAll() function
 * 2023-09-11  Nomake Wan <nomake_wan AT yahoo.co.jp>
 *             Fix for compiler warnings in IDE
 * 2024-01-14  Nomake Wan <nomake_wan AT yahoo.co.jp>
 *             Fix to allow full brightness in setBrightness() function
 *
 */

#include "Arduino.h"
#include "ht16k33.h"
#include <Wire.h>

// "address" is base address 0-7 which becomes 11100xxx = E0-E7
#define BASEHTADDR 0x70

//Commands
#define HT16K33_DDAP          0b00000000 // Display data address pointer: 0000xxxx
#define HT16K33_SS            0b00100000 // System setup register
#define HT16K33_SS_STANDBY    0b00000000 // System setup - oscillator in standby mode
#define HT16K33_SS_NORMAL     0b00000001 // System setup - oscillator in normal mode
#define HT16K33_KDAP          0b01000000 // Key Address Data Pointer
#define HT16K33_IFAP          0b01100000 // Read status of INT flag
#define HT16K33_DSP           0b10000000 // Display setup
#define HT16K33_DSP_OFF       0b00000000 // Display setup - display off
#define HT16K33_DSP_ON        0b00000001 // Display setup - display on
#define HT16K33_DSP_NOBLINK   0b00000000 // Display setup - no blink
#define HT16K33_DSP_BLINK2HZ  0b00000010 // Display setup - 2hz blink
#define HT16K33_DSP_BLINK1HZ  0b00000100 // Display setup - 1hz blink
#define HT16K33_DSP_BLINK05HZ 0b00000110 // Display setup - 0.5hz blink
#define HT16K33_RIS           0b10100000 // ROW/INT Set
#define HT16K33_RIS_OUT       0b00000000 // Set INT as row driver output
#define HT16K33_RIS_INTL      0b00000001 // Set INT as int active low
#define HT16K33_RIS_INTH      0b00000011 // Set INT as int active high
#define HT16K33_DIM           0b11100000 // Dimming set
#define HT16K33_DIM_1         0b00000000 // Dimming set - 1/16
#define HT16K33_DIM_2         0b00000001 // Dimming set - 2/16
#define HT16K33_DIM_3         0b00000010 // Dimming set - 3/16
#define HT16K33_DIM_4         0b00000011 // Dimming set - 4/16
#define HT16K33_DIM_5         0b00000100 // Dimming set - 5/16
#define HT16K33_DIM_6         0b00000101 // Dimming set - 6/16
#define HT16K33_DIM_7         0b00000110 // Dimming set - 7/16
#define HT16K33_DIM_8         0b00000111 // Dimming set - 8/16
#define HT16K33_DIM_9         0b00001000 // Dimming set - 9/16
#define HT16K33_DIM_10        0b00001001 // Dimming set - 10/16
#define HT16K33_DIM_11        0b00001010 // Dimming set - 11/16
#define HT16K33_DIM_12        0b00001011 // Dimming set - 12/16
#define HT16K33_DIM_13        0b00001100 // Dimming set - 13/16
#define HT16K33_DIM_14        0b00001101 // Dimming set - 14/16
#define HT16K33_DIM_15        0b00001110 // Dimming set - 15/16
#define HT16K33_DIM_16        0b00001111 // Dimming set - 16/16

// Constructor
HT16K33::HT16K33(){
}

/****************************************************************/
// Setup the env
//
void HT16K33::begin(uint8_t address){
  _address=address | BASEHTADDR;
  Wire.begin();
  i2c_write(HT16K33_SS  | HT16K33_SS_NORMAL); // Wakeup
  i2c_write(HT16K33_DSP | HT16K33_DSP_ON | HT16K33_DSP_NOBLINK); // Display on and no blinking
  i2c_write(HT16K33_RIS | HT16K33_RIS_OUT); // INT pin works as row output 
  i2c_write(HT16K33_DIM | HT16K33_DIM_16);  // Brightness set to max
  //Clear all lights
  //  memset(displayRam,0,sizeof(displayRam));
  //  i2c_write(HT16K33_DDAP, displayRam,sizeof(displayRam),true);
  clearAll();
} // begin

/****************************************************************/
// internal function - Write a single byte
//
uint8_t HT16K33::i2c_write(uint8_t val){
  Wire.beginTransmission(_address);
  Wire.write(val);
  return Wire.endTransmission();
} // i2c_write

/****************************************************************/
// internal function - Write several bytes
// "size" is amount of data to send excluding the first command byte
// if LSB is true then swap high and low byte to send LSB MSB
// NOTE: Don't send odd amount of data if using LSB, then it will send one to much
//
uint8_t HT16K33::i2c_write(uint8_t cmd,uint8_t *data,uint8_t size,boolean LSB){
  uint8_t i;
  Wire.beginTransmission(_address);
  Wire.write(cmd);
  i=0;
  while (i<size){
    if (LSB){
      Wire.write(data[i+1]);
      Wire.write(data[i++]);
      i++;
    } else {
      Wire.write(data[i++]);
    }
  }
  return Wire.endTransmission(); // Send out the data
} // i2c_write

/****************************************************************/
// internal function - read a byte from specific address (send one byte(address to read) and read a byte)
//
uint8_t HT16K33::i2c_read(uint8_t addr){
  i2c_write(addr);
  Wire.requestFrom(_address,(uint8_t) 1);
  return Wire.read();    // read one byte
} // i2c_read

/****************************************************************/
// read an array from specific address (send a byte and read several bytes back)
// return value is how many bytes that where really read
//
uint8_t HT16K33::i2c_read(uint8_t addr,uint8_t *data,uint8_t size){
  uint8_t i,retcnt;
  
  i2c_write(addr);
  retcnt=Wire.requestFrom(_address, size);
  i=0;
  while(Wire.available() && i<size)    // slave may send less than requested
  {
    data[i++] = Wire.read();    // receive a byte as character
  }

  return retcnt;
} // i2c_read

/****************************************************************/
// Clear all leds and displays
//
void HT16K33::clearAll(){
  memset(displayRam,0,sizeof(displayRam));
  i2c_write(HT16K33_DDAP,displayRam,sizeof(displayRam));
} // clearAll

/****************************************************************/
// define seg7Font table
//
void HT16K33::define7segFont(uint8_t *ptr){
  _seg7Font=ptr;
} // define7segFont

/****************************************************************/
// define seg16Font table
//
void HT16K33::define16segFont(uint16_t *ptr){
  _seg16Font=ptr;
#ifdef PSDEBUG
  Serial.print("DEBUG1: ");Serial.println((uint16_t)ptr,HEX);
  Serial.println();
  Serial.print("DEBUG2: ");Serial.println((uint16_t)_seg16Font,HEX);
  Serial.println();
  Serial.print("DEBUG3: ");Serial.println(ptr[0],HEX);
  Serial.println(ptr[1],HEX);
  Serial.println(ptr[2],HEX);
  Serial.println();
  Serial.print("DEBUG4: ");Serial.println(_seg16Font[0],HEX);
  Serial.println(_seg16Font[1],HEX);
  Serial.println(_seg16Font[2],HEX);
#endif
} // define16segFont

/****************************************************************/
// Put the chip to sleep
//
uint8_t HT16K33::sleep(){
  return i2c_write(HT16K33_SS|HT16K33_SS_STANDBY); // Stop oscillator
} // sleep

/****************************************************************/
// Wake up the chip (after it been a sleep )
//
uint8_t HT16K33::normal(){
  return i2c_write(HT16K33_SS|HT16K33_SS_NORMAL); // Start oscillator
} // normal

/****************************************************************/
// Turn off one led but only in memory
// To do it on chip a call to "sendLed" is needed
//
uint8_t HT16K33::clearLed(uint8_t ledno){ // 16x8 = 128 LEDs to turn on, 0-127
  if (ledno<128){
    bitClear(displayRam[int(ledno/8)],(ledno % 8));
    return 0;
  } else {
    return 1;
  }
} // clearLed

/****************************************************************/
// Turn on one led but only in memory
// To do it on chip a call to "sendLed" is needed
//
uint8_t HT16K33::setLed(uint8_t ledno){ // 16x8 = 128 LEDs to turn on, 0-127
  if (ledno<128){
    bitSet(displayRam[int(ledno/8)],(ledno % 8));
    return 0;
  } else {
    return 1;
  }
} // setLed

/****************************************************************/
// check if a specific led is on(true) or off(false)
//
boolean HT16K33::getLed(uint8_t ledno,boolean Fresh){ 

  // get the current state from chip
  if (Fresh) {
    i2c_read(HT16K33_DDAP,displayRam,sizeof(displayRam));
  }

  if (ledno<128){
    return bitRead(displayRam[int(ledno/8)],ledno % 8) != 0;
  }
  return false;
} // getLed

/****************************************************************/
uint8_t HT16K33::setDisplayRaw(uint8_t pos, uint8_t val) {
  if (pos < sizeof(displayRam)) {
    displayRam[pos] = val;
    return 0;
  } else {
    return 1;
  }
} // setDisplayRaw

/****************************************************************/
// Send the display ram info to chip - kind of commit all changes to the outside world
//
uint8_t HT16K33::sendLed(){
  return i2c_write(HT16K33_DDAP, displayRam,sizeof(displayRam));
} // sendLed

/****************************************************************/
// set a single LED and update NOW
//
uint8_t HT16K33::setLedNow(uint8_t ledno){
  uint8_t rc;
  rc=setLed(ledno);
  if (rc==0){
    return sendLed();
  } else {
    return rc;
  }
} // setLedNow

/****************************************************************/
// clear a single LED and update NOW
//
uint8_t HT16K33::clearLedNow(uint8_t ledno){
  uint8_t rc;
  rc=clearLed(ledno);
  if (rc==0){
    return sendLed();
  } else {
    return rc;
  }
} // clearLedNow

/****************************************************************/
// Turn on one 7-segment but only in memory
// To do it on chip a call to "sendLed" is needed
//
uint8_t HT16K33::set7Seg(uint8_t dig, uint8_t cha, boolean dp){ // position 0-15, 0-15 (0-F Hexadecimal), decimal point on|off
  if (cha<16 && dig<16){
    if (dig<=7) {dig = dig*2;} else {dig = ((dig-8)*2)+1;} //re-arrange digit positions
    uint8_t num = _seg7Font[cha];
    if (dp) {bitSet(num,(7));} else {bitClear(num,(7));} //Set decimal point on 7th bit
    displayRam[dig] = num;
    return 0;
  } else {
    return 1;
  }
} // set7Seg

/****************************************************************/
uint8_t HT16K33::set7SegRaw(uint8_t dig, uint8_t val) {
  if (dig<16) {
    if (dig<=7) {dig = dig*2;} else {dig = ((dig-8)*2)+1;} //re-arrange digit positions
    displayRam[dig] = val;
    return 0;
  } else {
    return 1;
  }
} // set7SegRaw

/****************************************************************/
// Turn on one 16-segment but only in memory
// To do it on chip a call to "sendLed" is needed
//
uint8_t HT16K33::set16Seg(uint8_t dig, uint8_t cha){ // position 0-15, 0-15 (0-F Hexadecimal)
  if (cha<128 && dig<8){
    dig = dig*2;
    // reverse the bits using a lookup table
    //    displayRam[dig+1] = BitReverseTable256[lowByte(seg16Chartable[cha])];
    //    displayRam[dig] = BitReverseTable256[highByte(seg16Chartable[cha])];
    //    displayRam[dig] = lowByte(seg16Chartable[cha]);
    //    displayRam[dig+1] = highByte(seg16Chartable[cha]);
    displayRam[dig] = lowByte(_seg16Font[cha]);
    displayRam[dig+1] = highByte(_seg16Font[cha]);
    //    Serial.print(lowByte(_seg16Font[cha]),HEX);Serial.print(F(" "));Serial.println(highByte(&_seg16Font[cha]),HEX);
#ifdef PSDEBUG
    Serial.print((uint16_t)_seg16Font,HEX);Serial.print(F(":"));Serial.print(_seg16Font[cha] & 0xff,HEX);Serial.print(F(" "));Serial.println(_seg16Font[cha] >> 8,HEX);
#endif
    return 0;
  } else {
    return 1;
  }
} // set16Seg

/****************************************************************/
// Change brightness of the whole display
// level 0-15, 0 means display off
//
uint8_t HT16K33::setBrightness(uint8_t level){
  if (level<=HT16K33_DIM_16){
    return i2c_write(HT16K33_DIM|level);
  } else {
    return 1;
  }
} // setBrightness

/****************************************************************/
// Check the chips interrupt flag
// 0 if no new key is pressed
// !0 if some key is pressed and not yet read
//
uint8_t HT16K33::keyINTflag(){ 
  return i2c_read(HT16K33_IFAP);
} // keyINTflag

/****************************************************************/
// Check if any key is pressed
// returns how many keys that are currently pressed
// 

//From http://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
#ifdef __GNUC__
  uint16_t _popcount(uint16_t x) {
    return __builtin_popcount(x);
  }
#else
  uint16_t _popcount(uint16_t i) {
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
  }
#endif

uint8_t HT16K33::keysPressed(){ 
  //  Serial.println(_keyram[0]|_keyram[1]|_keyram[2],HEX);
  return (_popcount(_keyram[0])+_popcount(_keyram[1])+_popcount(_keyram[2]));
} // keysPressed

/****************************************************************/
// Internal function - update cached key array
//
void HT16K33::_updateKeyram(){
  uint8_t curkeyram[6];

  i2c_read(HT16K33_KDAP, curkeyram, 6);
  _keyram[0]=curkeyram[1]<<8 | curkeyram[0]; // datasheet page 21, 41H is high 40H is low
  _keyram[1]=curkeyram[3]<<8 | curkeyram[2]; // or LSB MSB
  _keyram[2]=curkeyram[5]<<8 | curkeyram[4];
  return;
} // _updateKeyram

/****************************************************************/
// return the key status
//
void HT16K33::readKeyRaw(HT16K33::KEYDATA keydata,boolean Fresh){
  int8_t i;

  // get the current state
  if (Fresh) {_updateKeyram();}

  for (i=0;i<3;i++){
    keydata[i]=_keyram[i];
  }

  return;
} // readKeyRaw

/****************************************************************
 * read the keys and return the key that changed state
 * if more than one is pressed (compared to last scan) 
 * only one is returned, the first one found
 * 0 means no key pressed.
 * "1" means the key #1 is pressed
 * "-1" means the key #1 is released 
 * "clear"=true means it will only look keys currently pressed down.
 *     this is so you can detect what key is still pressed down after
 *     several keys are pressed down and then all but one is released
 *     (without keeping track of up/down separately)
 *
 *Observations:
 * As long as the key is pressed the keyram bit is set
 * the flag is set when key is pressed down but then cleared at first
 * read of key ram.
 * When released the key corresponding bit is cleared but the flag is NOT set
 * This means that the only way a key release can be detected is
 * by only polling readKey and ignoring flag
 * 
 */

int8_t HT16K33::readKey(boolean clear){
  static HT16K33::KEYDATA oldKeyData;
  uint16_t diff;
  uint8_t key;
  int8_t i,j;

    // save the current state
  for (i=0;i<3;i++){
    if (clear){
      oldKeyData[i]=0;
    } else {
      oldKeyData[i]=_keyram[i];
    }
  }
    
  _updateKeyram();

  key=0; //the key that changed state
  for (i=0;i<3;i++){
    diff=_keyram[i] ^ oldKeyData[i]; //XOR old and new, any changed bit is set.
    if ( diff !=0 ){ // something did change
      for (j=0;j<13;j++){
	key++;
	if (((diff >> j) & 1) == 1){
	  if (((_keyram[i] >> j) & 1)==0){
	    return -key;
	  }else{
	    return key;
	  }
	} // if keyram differs
      } // for j in bits
    } else {
      key+=13;
    } // if diff
  } // for i
  return 0; //apparently no new key was pressed - old might still be held down, pass clear=true to see it
} // readKey

/****************************************************************/
// Make the display blink
//
uint8_t HT16K33::setBlinkRate(uint8_t rate){
  switch (rate) {
    case HT16K33_DSP_NOBLINK:
    case HT16K33_DSP_BLINK2HZ:
    case HT16K33_DSP_BLINK1HZ:
    case HT16K33_DSP_BLINK05HZ:
      i2c_write(HT16K33_DSP |rate);
      return 0;
      ;;
  default:
    return 1;
  }
} //setBlinkRate

/****************************************************************/
// turn on the display
//
void HT16K33::displayOn(){
  i2c_write(HT16K33_DSP |HT16K33_DSP_ON);
} // displayOn

/****************************************************************/
// turn off the display
//
void HT16K33::displayOff(){
  i2c_write(HT16K33_DSP |HT16K33_DSP_OFF);
} // displayOff
