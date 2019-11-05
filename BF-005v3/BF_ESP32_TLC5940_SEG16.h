// copyright 2019 BotanicFields, Inc.
// M5Stack/ESP32 + TLC5940 + 16 segment LED display
//
#ifndef _BF_ESP32_TLC5940_SEG16_INCLUDED
#define _BF_ESP32_TLC5940_SEG16_INCLUDED
#include <Arduino.h>

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// LED display configuration
const uint8_t  TLC5940_SEG = 16;  // segments per LED display
const uint8_t  TLC5940_COL =  8;  // columns per dynamic display group
const uint8_t  TLC5940_GRP =  2;  // groups of dynamic display 

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// effect
const uint16_t TLC5940_HLF =   50;  // default half-life of after image(ms)
const uint16_t TLC5940_FNT =    0;  // default font
const uint16_t TLC5940_ALT = 0x80;  // alternate font

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// attribute
const uint8_t TLC5940_INTENSITY_FROM   = 0x30;  // intensity code from '0'
const uint8_t TLC5940_INTENSITY_LENGTH = 10;    //                  to '9'
const uint8_t TLC5940_INTENSITY_DECAY  = 2;     // decay parameter Anew = Aold - Aold / decay

// samples
//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
const char TLC5940_SAMPLE_TEXT[] = "by BotanicFields"; 
const char TLC5940_SAMPLE_ATTR[] = "2222222222222222";
const char TLC5940_SAMPLE_DOT[]  = "4444444444444444";

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// functions for user
void TLC5940_init();
void TLC5940_decay_set(const uint16_t half_ms = TLC5940_HLF);
void TLC5940_font_set(const uint8_t font = TLC5940_FNT);
void TLC5940_text_set(const char* text, const char* attr, const int16_t pos, const uint16_t len);
void TLC5940_dot_set(const char* dot);
void TLC5940_flow(const char* text, const char* attr, const uint16_t delay_ms);

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// dot correction
//                  segment:     c     l    d2     m     n    d1     e    g1    g2     b    a2     k     j    a1     h     f
const uint8_t TLC5940_DC[] = {0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f, 0xff, 0x7f, 0x7f, 0xff, 0x7f, 0xff, 0xff, 0x7f, 0xff, 0xff,};

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// font pattern
const uint16_t TLC5940_FONT[] = {
  0xA66D, 0xA67D, 0xA6ED, 0xE66D, 0xB66D, 0xAE6D, 0xA76D, 0xA66F, 0x0018, 0x0090, 0x4080, 0x5000, 0x1800, 0x0900, 0x0102, 0x000A, 
  0x59E5, 0x51E5, 0x49E5, 0x19E5, 0xA79A, 0xA798, 0xA792, 0xA78A, 0x6DC9, 0x6C5B, 0x6DD3, 0xCBB6, 0x0870, 0xE002, 0x0E10, 0x4007, 
  0x0000, 0x0410, 0x0048, 0xB5C8, 0xB5AD, 0xB99D, 0xE736, 0x0010, 0x7038, 0x1C0E, 0x581A, 0x1188, 0x1400, 0x0180, 0x0400, 0x0810, 
  0xA665, 0x8040, 0x27E4, 0xA5E4, 0x81C1, 0xA5A5, 0xA7A5, 0x8065, 0xA7E5, 0xA5E5, 0x0404, 0x1404, 0x4010, 0x2580, 0x0802, 0x10E5, 
  0xD764, 0x83E5, 0xB4EC, 0x2625, 0xB46C, 0x27A5, 0x03A5, 0xA6A5, 0x83C1, 0x342C, 0xA440, 0x4311, 0x2601, 0x8253, 0xC243, 0xA675, 
  0x03E5, 0xE665, 0x43E5, 0x6426, 0x102C, 0xA641, 0x0A11, 0xCA41, 0x4812, 0x1012, 0x2C34, 0x3028, 0x1192, 0x140C, 0x0050, 0x2400, 
  0x0002, 0xE700, 0xA781, 0x2780, 0xA7C0, 0x2F00, 0x11A8, 0xE480, 0x8381, 0x3504, 0xA4A0, 0x4381, 0x340C, 0x9380, 0x8380, 0xA780, 
  0x030D, 0x310D, 0x0380, 0x6480, 0x3188, 0xA600, 0x0A80, 0xCA00, 0x4980, 0xE500, 0x2D00, 0x3128, 0x1008, 0x148C, 0x0013, 0xFFFF, 
  0x0020, 0x0040, 0x8000, 0x2000, 0x0400, 0x0200, 0x0001, 0x0004, 0x000C, 0x0030, 0x00C0, 0xC000, 0x3000, 0x0C00, 0x0300, 0x0003, 
  0x6D88, 0x6D92, 0x6C1A, 0x6C08, 0x09B6, 0x11B6, 0x41B6, 0x49B6, 0x6C00, 0x0036, 0x4980, 0x0192, 0x19E8, 0xB18A, 0x1798, 0x518D, 
  0x2000, 0x0838, 0x0009, 0x13AD, 0xB4AE, 0xC993, 0xEC0E, 0x0008, 0x6635, 0xAC66, 0x4992, 0x33CC, 0x1100, 0x4182, 0x1700, 0x0990, 
  0xAE75, 0x8050, 0x2CE0, 0xA4E0, 0x1189, 0x6505, 0x6705, 0x1034, 0xA7B6, 0xA0E6, 0x0104, 0x1104, 0x4110, 0x01A4, 0x0882, 0x08E4, 
  0xB760, 0x83E8, 0xA78D, 0xA60D, 0xB56C, 0x2CA6, 0x08A6, 0x6685, 0x89C8, 0xB42C, 0xAC60, 0x8892, 0xAC28, 0x926D, 0xB24D, 0xA677, 
  0x0335, 0xEE65, 0x4335, 0xA6A6, 0x106D, 0xAC42, 0xC042, 0xB649, 0xC813, 0x1053, 0xAC35, 0x0C06, 0x6006, 0x6030, 0x0064, 0x2600, 
  0x002C, 0x5700, 0xB488, 0x2700, 0x3708, 0xB700, 0x35A8, 0xA4E8, 0x3301, 0x3104, 0x1504, 0x508C, 0x100C, 0x9A80, 0x8A80, 0xAC80, 
  0x08E8, 0x410D, 0x0880, 0x6450, 0x4988, 0xB100, 0xAD00, 0xB600, 0x3580, 0xA4C8, 0xA4B0, 0x2D26, 0x9249, 0x64B4, 0x00CD, 0x599A, 
};

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// functions for internal use
void TLC5940_pin_init();
void TLC5940_ledc_init();
void TLC5940_timer_init();
void IRAM_ATTR TLC5940_onTimer();
void TLC5940_task(void* pvParameters);
void TLC5940_spi_init();
void TLC5940_dc_init();
void TLC5940_image_clear(const uint16_t c = 0);
void TLC5940_spi_gs_send(const uint16_t count);
void TLC5940_update(const uint16_t count);
void TLC5940_intensity_init(const uint16_t decay = TLC5940_INTENSITY_DECAY);
uint16_t TLC5940_seg_new(const uint8_t grp, const uint8_t col, const uint8_t seg);
uint16_t TLC5940_dot_new(const uint8_t grp, const uint8_t col);

#endif // #ifndef _BF_ESP32_TLC5940_SEG16_INCLUDED

/* 
The MIT License
SPDX short identifier: MIT

Copyright 2019 BotanicFields, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.
*/
