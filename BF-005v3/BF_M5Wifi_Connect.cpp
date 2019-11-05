// Copyright 2019 BotanicFields, Inc.
// Arduino core for ESP32
// Wifi Connection
//
#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "BF_M5Wifi_Connect.h"
#include "BF_M5LCD_Print.h"

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7....:....8
// SSIDs and Keys
const char BF_WIFI_SSID1[] = "xxxxxxxxxxxxx";
const char BF_WIFI_PASS1[] = "xxxxxxxxxxxxx";
const char BF_WIFI_SSID2[] = "xxxxxxxxxxxxx";
const char BF_WIFI_PASS2[] = "xxxxxxxxxxxxx";

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7....:....8
// constants
const int   BF_WIFI_TCK = 1000;                       // time-out delay(ms) 
const int   BF_WIFI_TMO = 20;                         // time-out count
const char* BF_WIFI_MSG[] = {"     Wifi Connection",  // 0
                             " Wifi Previous..   ",   // 1
                             " Wifi-Single..     ",   // 2
                             " Wifi-Multi..      ",   // 3
                             " Smart-Config..    ",   // 4
                             " Connected.        ",   // 5
                             " Configuration Set.",   // 6
                             " Connection Failed.",   // 7
                           };

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7....:....8
// wifi Initialize .. select method
void BF_WifiInit() {
  BF_M5LCD_start();
  BF_M5LCD_init(7, BF_WIFI_MSG);

  // try the previous connection 
  BF_M5LCD_update(1);  
  BF_WifiConnect(NULL);

  if(WiFi.status() != WL_CONNECTED) {
//    BF_M5LCD_update(2);  
//    BF_WifiConnect(BF_WIFI_SSID1, BF_WIFI_PASS1);
    BF_M5LCD_update(3);  
    BF_WifiMulti();
  }

  if(WiFi.status() != WL_CONNECTED) {
    BF_M5LCD_update(4);  
    BF_WifiSmartConfig();
  }

  if(WiFi.status() != WL_CONNECTED) {
    // Connection failed
    BF_M5LCD_update(7);  
    WiFi.disconnect();
//    WiFi.disconnect(true);        // wifi off
//    WiFi.disconnect(true, true);  // erase AP
    return;
  }

  // Connected
  BF_M5LCD_update(5);

  // Set IP manually
  // in the case of a DHCP may not work, such as tethering by iPhone.  
  if(WiFi.status() == WL_CONNECTED && WiFi.dnsIP() == IPAddress(0, 0, 0, 0)) {
    BF_M5LCD_update(6);
    WiFi.config(IPAddress(172,  20,  10,   6),  // local IP
                IPAddress(172,  20,  10,   1),  // gateway
                IPAddress(255, 255, 255, 240),  // subnet
                IPAddress(172,  20,  10,   1),  // DNS 0
                IPAddress(  0,   0,   0,   0)   // DNS 1
               );
    BF_WifiPrint(true);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7....:....8
// wifi connect
void BF_WifiConnect(const char* ssid, const char* pass) {
  if(ssid == NULL) {
    Serial.printf("\n[Wifi] Previous: Establishing connection ");
    WiFi.begin();  // try previous connection
  } else {
    Serial.printf("\n[Wifi] Normal: Establishing connection ");
    WiFi.begin(ssid, pass);
  }
  for(int i = 0; i < BF_WIFI_TMO; i++) {
    if(WiFi.status() == WL_CONNECTED) {
      BF_WifiPrint(true);
      break;
    }      
    Serial.printf(".");
    delay(BF_WIFI_TCK);
  }
  if(WiFi.status() != WL_CONNECTED) {
    BF_WifiPrint(false);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7....:....8
// Wifi Multi
WiFiMulti WFM;
void BF_WifiMulti() {
  Serial.printf("\n[Wifi]Multi: Establishing connection ");
  WFM.addAP(BF_WIFI_SSID1, BF_WIFI_PASS1);
  WFM.addAP(BF_WIFI_SSID2, BF_WIFI_PASS2);
  for(int i = 0; i < 30; i++) {
    if(WFM.run() == WL_CONNECTED) {
      BF_WifiPrint(true);
      break;
    }      
    Serial.printf(".");
    delay(100);
  }
  if(WiFi.status() != WL_CONNECTED) {
    BF_WifiPrint(false);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7....:....8
// Wifi Smart Config
void BF_WifiSmartConfig() {
  Serial.printf("\n[Wifi]SmartConfig: Waiting smart device ");
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();
  for(int i = 0; i < BF_WIFI_TMO; i++) {
    if(WiFi.smartConfigDone()) {
      Serial.printf("\n  SmartConfig received.\n");
      break;
    }      
    Serial.printf(".");
    delay(BF_WIFI_TCK);
  }
  if(!WiFi.smartConfigDone()) {
    Serial.printf("\n  SmartConfig time-out.\n");
    return;
  }

  Serial.printf("\n[Wifi]SmartConfig: Establishing connection ");
  for(int i = 0; i < BF_WIFI_TMO; i++) {
    if(WiFi.status() == WL_CONNECTED) {
      BF_WifiPrint(true);
      break;
    }      
    Serial.printf(".");
    delay(BF_WIFI_TCK);
  }
  if(WiFi.status() != WL_CONNECTED) {
    BF_WifiPrint(false);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7....:....8
// Wifi information 
void BF_WifiPrint(boolean connect_success) {
  if(connect_success) {
    Serial.printf("\n  Connected to\n");
    Serial.printf("\tNetwork:     %s\n", WiFi.SSID().c_str());
    Serial.printf("\tLocal MAC:   %s\n", WiFi.macAddress().c_str());   
    Serial.printf("\tSubnet mask: %s\n", WiFi.subnetMask().toString().c_str());
    Serial.printf("\tLocal IP:    %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("\tGateway IP:  %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("\tDNS IP 0:    %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("\tDNS IP 1:    %s\n", WiFi.dnsIP(1).toString().c_str());
  }
  else {
    Serial.printf("\n  Connection failed.\n");
  }
}

void BF_LCD_Print(uint16_t f_color, uint16_t b_color, uint16_t x0, uint16_t y0, const char* s) {
  M5.Lcd.setTextColor(f_color, b_color);
  M5.Lcd.setCursor(x0, y0);
  M5.Lcd.print(s);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7....:....8
// wifi scan
String enctype_str(wifi_auth_mode_t e) {
  switch(e) {
    case WIFI_AUTH_OPEN:            return "Open";
    case WIFI_AUTH_WEP:             return "WEP";
    case WIFI_AUTH_WPA_PSK:         return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:        return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA_WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2_ENTERPRISE";
  }
}
void BF_WifiScan() {
  Serial.printf("\n[Wifi] Scan \n");
  int n = WiFi.scanNetworks();
  Serial.printf("  Networks found: %d\n", n);
  Serial.printf("     Network name, Channel, Signal strength, MAC address, Encryption type\n");
  for (int i = 0; i < n; i++) {
    Serial.printf("(%2d) %s \t%2d \t%3ddB/m \t%s \t%s\n",
                    i + 1,
                    WiFi.SSID(i).c_str(),
                    WiFi.channel(i),
                    WiFi.RSSI(i), 
                    WiFi.BSSIDstr(i).c_str(),
                    enctype_str(WiFi.encryptionType(i)).c_str()
                 );
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
