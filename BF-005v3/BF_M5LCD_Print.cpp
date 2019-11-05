// Copyright 2019 BotanicFields, Inc.
// Arduino core for ESP32
// Print information to M5Stack LCD
//
#include <M5Stack.h>
#include "BF_M5LCD_Print.h"

uint8_t     BF_M5LCD_now = 0;  // the line to highlight
uint8_t     BF_M5LCD_old = 0;  // the line changed last
uint8_t     BF_M5LCD_max = 0;  // lines 0..max
const char* BF_M5LCD_msg[11];  // line 0..10

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7....:....8
// initialize
void BF_M5LCD_init(uint8_t msg_max, const char* msg[]) {
  BF_M5LCD_now = 0;
  BF_M5LCD_old = 0;
  BF_M5LCD_max = msg_max;
  for(int i = 0; i <= BF_M5LCD_max; i++)  
    BF_M5LCD_msg[i] = msg[i];  // copy pointers
  delay(1000);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7....:....8
// update LCD
void BF_M5LCD_update(uint8_t msg_now) {
  BF_M5LCD_now = msg_now;
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7....:....8
// start LCD
void BF_M5LCD_start() {
  xTaskCreatePinnedToCore(BF_M5LCD_task,   // Function to implement the task
                         "BF_M5LCD_task",  // Name of the task
                          4096,             // Stack size in words
                          NULL,             // Task input parameter
                          1,                // Priority of the task
                          NULL,             // Task handle
                          1);               // Core where the task should run
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// LCD task 
void BF_M5LCD_task(void * pvParameters) {
  M5.Lcd.wakeup();
  M5.Lcd.setBrightness(100);  
  M5.Lcd.setTextSize(2);

  for(;;) {
    if(BF_M5LCD_now == 0) {
      M5.Lcd.fillScreen(TFT_BLACK);
      BF_M5LCD_Print(TFT_BLACK, TFT_GREEN, 0, 0, " // BotanicFields, Inc.// ");
    }
    if(BF_M5LCD_now != BF_M5LCD_old) {
      BF_M5LCD_old = BF_M5LCD_now;
      BF_M5LCD_Print(TFT_YELLOW, TFT_BLACK, 0, 16, BF_M5LCD_msg[0]);
      for(int i = 1; i <= BF_M5LCD_max; i++) {
        if(i == BF_M5LCD_now) {
          BF_M5LCD_Print(TFT_BLACK, TFT_CYAN,  0, 32 + 16 * i, ">");
          BF_M5LCD_Print(TFT_BLACK, TFT_CYAN, 12, 32 + 16 * i, BF_M5LCD_msg[i]);
        }
        else
          BF_M5LCD_Print(TFT_CYAN, TFT_BLACK, 12, 32 + 16 * i, BF_M5LCD_msg[i]);
      }
    }
    delay(500);
  }
}

void BF_M5LCD_Print(uint16_t f_color, uint16_t b_color, uint16_t x0, uint16_t y0, const char* s) {
  M5.Lcd.setTextColor(f_color, b_color); 
  M5.Lcd.setCursor(x0, y0);  
  M5.Lcd.print(s);
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
