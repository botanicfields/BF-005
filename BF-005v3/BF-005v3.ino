// copyright 2019 BotanicFields, Inc.
// BF-005 
// M5Stack/ESP32 + TLC5940 + 16 segment LED display
//
#include <M5Stack.h>
#include "BF_ESP32_TLC5940_SEG16.h"
#include "BF_ESP32_NTP.h"
#include "BF_M5Wifi_Connect.h"
#include "BF_M5LCD_Print.h"

// LCD demo control
const char* BF_005_MSG[] = {" 16 Segment LED / TLC5940 ",  // 0
                            " lamp test   ",               // 1
                            " dot flow    ",               // 2
                            " font show   ",               // 3
                            " string flow ",               // 4
                            " aliens      ",               // 5
                            " clock       ",               // 6
                           };

void setup() {
  M5.begin();
  BF_WifiScan();
  BF_WifiInit();
  ntp_get();

  TLC5940_init();
  BF_M5LCD_init(6, BF_005_MSG);

  xTaskCreatePinnedToCore(
                    task_TLC5940_demo,   // Function to implement the task
                   "task_TLC5940_demo",  // Name of the task
                    4096,                // Stack size in words
                    NULL,                // Task input parameter
                    2,                   // Priority of the task
                    NULL,                // Task handle
                    1);                  // Core where the task should run
}

void loop() {
  M5.update();
  delay(100);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// main task of TLC5940 demonstrations
void task_TLC5940_demo(void * pvParameters) {
  BF_M5LCD_update(0);  delay(1500);
  for(;;) {
    BF_M5LCD_update(1);  demo_lamp();   delay(1500);
    BF_M5LCD_update(2);  demo_dot();    delay(1500);
    BF_M5LCD_update(3);  demo_char();   delay(1500);
    BF_M5LCD_update(4);  demo_flow();   delay(1500);
    BF_M5LCD_update(5);  demo_alien();  delay(1500);
    BF_M5LCD_update(6);  demo_clock();  delay(1500);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// text, attrbute, dot
const uint16_t TXT_MAX = 256;
const uint16_t DOT_MAX =  16;

char Txt[TXT_MAX + 1];
char Atr[TXT_MAX + 1];
char Dot[DOT_MAX + 1];

// clear text
void fill_txt(const char t, const char a) {
  for(int i = 0; i < TXT_MAX; i++) {
    Txt[i] = t;
    Atr[i] = a;
  }
  Txt[TXT_MAX] = '\0';
  Atr[TXT_MAX] = '\0';
}

// clear dot
void fill_dot(const char d) {
  for(int i = 0; i < DOT_MAX; i++)
    Dot[i] = d;
  Dot[DOT_MAX] = '\0';
}

// initialize common for demonstration
void demo_init(const uint8_t font, const uint16_t half_ms) {
  TLC5940_font_set(font);
  TLC5940_decay_set(half_ms);
  fill_txt(' ', '0');
  TLC5940_text_set(Txt, Atr, 0, TXT_MAX);
  fill_dot(' ');
  TLC5940_dot_set(Dot);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// demo all segment
void demo_lamp() {
  Serial.printf("[TLC5940] demo_lamp start\n");
  demo_init(TLC5940_FNT, 500);

  fill_dot('0');
  fill_txt(0x7f, '0');
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// demo dot
void demo_dot() {
  Serial.printf("[TLC5940] demo_dot start\n");
  demo_init(TLC5940_FNT, 300);

  for(int k = 0; k < 3; k++) {
    for(int j = 0; j < 16; j++) {
      for(int i = 0; i < 16; i++)
        Dot[i] = (i == j)? '0': '9';
      delay(100);
    }
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// demo character
void demo_char() {
  Serial.printf("[TLC5940] demo_char start\n");
  demo_init(TLC5940_FNT, 100);

  for(int i = 0; i < 256; i++)
    Txt[i] = i;
  Txt[256] = '\0';

  for(int i = 0; i < 256; i += 16) {
    TLC5940_text_set(Txt, Atr, i, 256);
    delay(1500);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// demo flow string
void demo_flow() {
  const char s1[] = "16-Segment LED with TLC5940+M5Stack/ESP32 by BotanicFields, Inc.";
  const char a1[] = "1111111111111111111100000001000000010000011111111111111111111111";
  const char s2[] = "A QUICK FOX JUMPED OVER THE LAZY BROWN DOG.";
  const char a2[] = "0000000000000000000000000000000000000000000";
  const char s3[] = "An old saying tells us that if you want something done well, you should do it yourself.";
  const char a3[] = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";

  Serial.printf("[TLC5940] demo_flow start\n");
  demo_init(TLC5940_FNT, 30);

  TLC5940_font_set(TLC5940_FNT);
  strcpy(Txt, s1);  strcpy(Atr, a1);  TLC5940_flow(Txt, Atr, 300);
  strcpy(Txt, s2);  strcpy(Atr, a2);  TLC5940_flow(Txt, Atr, 300);
  strcpy(Txt, s3);  strcpy(Atr, a3);  TLC5940_flow(Txt, Atr, 200);

  TLC5940_font_set(TLC5940_ALT);
  strcpy(Txt, s2);  strcpy(Atr, a2);  TLC5940_flow(Txt, Atr, 200);
  strcpy(Txt, s3);  strcpy(Atr, a3);  TLC5940_flow(Txt, Atr, 200);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// demo alien
void demo_alien() {
  Serial.printf("[TLC5940] demo_alien start\n");
  demo_init(TLC5940_FNT, 30);  demo_alien_sub();  delay(1000);
//  demo_init(TLC5940_ALT, 30);  demo_alien_sub();
}

void demo_alien_sub() {
  int x[5] = {200, 300, 400, 500, 600, };  // location * 100
  int v[5] = {  5,   5,  10,  10,  20, };  // velocity * 100
  
  for(int t = 0; t < 200; t++) {  // time

    // clear canvas
    fill_txt(' ', '0');

    // dot
    for(int i = 0; i < 16; i++)
      Dot[i] = (t % 10 == 0)? '0': '9';
    
    // clock, spoke
    Txt[ 0] = 0x00 + t % 8;
    Txt[15] = 0x08 + t % 8;
    
    // aliens
    for(int i = 0; i < 5; i++) {
      x[i] += v[i];
      if(x[i] < 100 || x[i] > 1400)
        v[i] *= -1;
      int xt = x[i] / 100;
      switch(i) {
        case 0: Txt[xt] = 0x10 + (t / 4) % 4;  break;
        case 1: Txt[xt] = 0x14 + (t / 4) % 4;  break;
        case 2: Txt[xt] = 0x18 + (t / 4) % 2;  break;
        case 3: Txt[xt] = 0x1a + (t / 4) % 2;  break;
        case 4: Txt[xt] = 0x1c + (t    ) % 4;  break;
        default:  break;
      }
    }
    delay(150);  
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// demo - clock
void demo_clock() {
  Serial.printf("[TLC5940] demo_clock start\n");
  struct tm timeinfo;

  demo_init(TLC5940_FNT, 100);
  Dot[ 3] = '0';
  Dot[ 5] = '0';
  Dot[10] = '0';
  Dot[12] = '0';
  for(int i = 0; i < 40; i++) {
    getLocalTime(&timeinfo);
    strftime(Txt, 17, "%Y%m%d %H%M%S ", &timeinfo);  // "yyyymmdd hhmmss*"
    Txt[15] = 0x08 + i % 8; 
    delay(125);
  }

  demo_init(TLC5940_FNT, 100);
  Dot[ 4] = '0';
  for(int i = 0; i < 40; i++) {
    getLocalTime(&timeinfo);
    strftime(Txt, 17, "  %a%e-%b-%Y", &timeinfo);  // "*DDDdd-MMM-yyyy"
    Txt[ 0] = 0x00 + i % 8; 
    delay(125);
  }

  demo_init(TLC5940_FNT, 25);  demo_clock_sub();
//  demo_init(TLC5940_ALT, 25);  demo_clock_sub();
}

void demo_clock_sub() {
  struct tm timeinfo;
  uint16_t l = TXT_MAX;  // temporary

  for(int i = - TLC5940_GRP * TLC5940_COL; i <= l; i++) {
    getLocalTime(&timeinfo);
    strftime(Txt, 128, "%A, %B %e %Y, %H:%M:%S", &timeinfo);  // "DDDD, MMMM dd yyyy, hh:mm:dd"
    l = strlen(Txt);
    TLC5940_text_set(Txt, Atr, i, l);
    delay(250);
  }
}
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
