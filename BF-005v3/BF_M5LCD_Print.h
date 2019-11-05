// Copyright 2019 BotanicFields, Inc.
// Arduino core for ESP32
// Print information to M5Stack LCD
//
#ifndef _BF_M5LCD_PRINT_INCLUDED
#define _BF_M5LCD_PRINT_INCLUDED

void BF_M5LCD_init(uint8_t msg_max, const char* msg[]);
void BF_M5LCD_update(uint8_t msg_now);
void BF_M5LCD_start();
void BF_M5LCD_task(void * pvParameters);
void BF_M5LCD_Print(uint16_t f_color, uint16_t b_color, uint16_t x0, uint16_t y0, const char* s);

#endif // #ifndef _BF_M5LCD_PRINT_INCLUDED

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
