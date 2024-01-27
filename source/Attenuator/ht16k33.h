/*
  ht16k33.h - used to talk to the htk1633 chip to do things like turn on LEDs or scan keys
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
 *	       first check in to github
 * 2015-12-05  Peter Sjoberg <peters-alib AT techwiz.ca>
 *	       moved displayram to public section
 * 2016-08-09  René Wennekes <rene.wennekes AT gmail.com>
 *             Contribution of 7-segment & 16-segment display support
 *             Added clearAll() function
 */



#ifndef ht16k33_h
#define ht16k33_h

#include "Arduino.h"

class HT16K33
{
 public:
  typedef uint16_t KEYDATA[3];
  typedef uint8_t  DisplayRam_t[16];

  DisplayRam_t displayRam;

  HT16K33(); // the class itself

  void    begin(uint8_t address);
  void    end();
  void    clearAll(); // clear all LEDs
  uint8_t sleep();  // stop oscillator to put the chip to sleep
  uint8_t normal(); // wake up chip and start ocillator
  uint8_t clearLed(uint8_t ledno); // 16x8 = 128 LEDs to turn on, 0-127
  uint8_t setLed(uint8_t ledno); // 16x8 = 128 LEDs to turn on, 0-127
  void    define7segFont(uint8_t *ptr); // Pass a pointer to a font table for 7seg
  void    define16segFont(uint16_t *ptr); // Pass a pointer to a font table for 16seg
  uint8_t set7Seg(uint8_t dig, uint8_t cha, boolean dp); // position 0-15, 0-15 (0-F Hexadecimal), decimal point
  uint8_t set16Seg(uint8_t dig, uint8_t cha); // position 0-7, see asciifont.h
  boolean getLed(uint8_t ledno,boolean Fresh=false); // check if a specific led is on(true) or off(false)
  uint8_t setDisplayRaw(uint8_t pos, uint8_t val); // load byte "pos" with value "val"
  uint8_t sendLed(); // send whatever led patter you set
  uint8_t set7SegNow(uint8_t dig, uint8_t cha, boolean dp); // position 0-15, 0-15 (0-F Hexadecimal), decimal point and send led in one function
  uint8_t set7SegRaw(uint8_t dig, uint8_t val); // load byte "pos" with value "val"
  uint8_t set16SegNow(uint8_t dig, uint8_t cha); // position 0-17, see asciifont.h and send led in one function
  uint8_t setLedNow(uint8_t ledno); //Set a single led and send led in one function
  uint8_t clearLedNow(uint8_t ledno); //Clear a single led and send led in one function
  uint8_t setBrightness(uint8_t level); // level 0-16, 0 means display off
  uint8_t keyINTflag(); // INTerrupt flag value, set when a key is pressed
  uint8_t keysPressed(); // report how many keys that are pressed, clear means report as if new
  int8_t  readKey(boolean clear=false);  // read what key was pressed, Fresh=false to go from cache
  void    readKeyRaw(KEYDATA keydata,boolean Fresh=true); //read the raw key info, bitmapped info of all key(s) pressed
  uint8_t setBlinkRate(uint8_t rate); // HT16K33_DSP_{NOBLINK,BLINK2HZ,BLINK1HZ,BLINK05HZ}
  void    displayOn();
  void    displayOff();
  // Some helper functions that can be useful in other parts of the code that use this library
  uint8_t i2c_write(uint8_t val);
  uint8_t i2c_write(uint8_t cmd,uint8_t *data,uint8_t size,boolean LSB=false);
  uint8_t i2c_read(uint8_t addr);
  uint8_t i2c_read(uint8_t addr,uint8_t *data,uint8_t size);

 private:
  void _updateKeyram();

  KEYDATA _keyram;
  uint8_t _address;
  uint8_t *_seg7Font;
  uint16_t *_seg16Font;
};


#endif
