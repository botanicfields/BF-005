// copyright 2019 BotanicFields, Inc.
// NTP process
//
#include <Arduino.h>
#include "BF_ESP32_NTP.h"

const char* NTP_SERVER = "pool.ntp.org";

// get time from NTP
void ntp_get() {
  configTime(GMT_OFFSET * 3600, DAYLIGHT_OFFSET * 3600, NTP_SERVER);

  struct tm timeinfo;
  if(getLocalTime(&timeinfo))
    Serial.println(&timeinfo, "[NTP] %A, %B %d %Y %T %z");
  else
    Serial.println("[NTP] failed to obtain time..");
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
