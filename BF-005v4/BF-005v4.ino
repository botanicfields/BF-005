// copyright 2019 BotanicFields, Inc.
// BF-005 
// M5Stack/ESP32 + TLC5940 + 16 segment LED display
//
#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "BF_Pcf8563.h"
#include "BF_RtcxNtp.h"
#include "BF_ESP32_TLC5940_SEG16.h"

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// for NTP
const char* time_zone  = "JST-9";
const char* ntp_server = "pool.ntp.org";
bool localtime_valid(false);
struct tm   local_tm;
bool rtcx_exist(false);

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// for WiFi
const int wifi_config_portal_timeout_sec(60);
const unsigned int wifi_retry_interval_ms(60000);
      unsigned int wifi_retry_last_ms(0);
const int wifi_retry_max_times(3);
      int wifi_retry_times(0);
wl_status_t wifi_status(WL_NO_SHIELD);

const char* wl_status_str[] = {
  "WL_IDLE_STATUS",      // 0
  "WL_NO_SSID_AVAIL",    // 1
  "WL_SCAN_COMPLETED",   // 2
  "WL_CONNECTED",        // 3
  "WL_CONNECT_FAILED",   // 4
  "WL_CONNECTION_LOST",  // 5
  "WL_DISCONNECTED",     // 6
  "WL_NO_SHIELD",        // 7 <-- 255
  "wl_status invalid",   // 8
};

const char* WlStatus(wl_status_t wl_status)
{
  if (wl_status >= 0 && wl_status <= 6) {
    return wl_status_str[wl_status];
  }
  if (wl_status == 255) {
    return wl_status_str[7];
  }
  return wl_status_str[8];
}

void WifiCheck()
{
  wl_status_t wifi_status_new = WiFi.status();
  if (wifi_status != wifi_status_new) {
    wifi_status = wifi_status_new;
    Serial.printf("[WiFi]%s\n", WlStatus(wifi_status));
  }

  // retry interval
  if (millis() - wifi_retry_last_ms < wifi_retry_interval_ms) {
    return;
  }
  wifi_retry_last_ms = millis();

  // reboot if wifi connection fails
  if (wifi_status == WL_CONNECT_FAILED) {
    Serial.print("[WiFi]connect failed: rebooting..\n");
    ESP.restart();
    return;
  }

  // let the wifi process do if wifi is not disconnected
  if (wifi_status != WL_DISCONNECTED) {
    wifi_retry_times = 0;
    return;
  }

  // reboot if wifi is disconnected for a long time
  if (++wifi_retry_times > wifi_retry_max_times) {
    Serial.print("[WiFi]disconnect timeout: rebooting..\n");
    ESP.restart();
    return;
  }

  // reconnect, and reboot if reconnection fails
  Serial.printf("[WiFi]reconnect %d\n", wifi_retry_times);
  if (!WiFi.reconnect()) {
    Serial.print("[WiFi]reconnect failed: rebooting..\n");
    ESP.restart();
    return;
  };
}

void WifiConfigModeCallback(WiFiManager *wm)
{
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Lcd.println("WiFi config portal:");
  M5.Lcd.println(wm->getConfigPortalSSID().c_str());
  M5.Lcd.println(WiFi.softAPIP().toString().c_str());
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// for WiFi
void setup()
{
  const bool lcd_enable(true);
  const bool sd_enable(true);
  const bool serial_enable(true);
  const bool i2c_enable(true);
  M5.begin(lcd_enable, sd_enable, serial_enable, i2c_enable);

  M5.Lcd.wakeup();
  M5.Lcd.setBrightness(100);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);

  // RTCx PCF8563: connect and start
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);  M5.Lcd.print("I2C, RTCx:");
  if (rtcx.Begin(Wire) == 0) {
    rtcx_exist = true;
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);  M5.Lcd.println("OK");
    if (SetTimeFromRtcx(time_zone)) {
      localtime_valid = true;
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);  M5.Lcd.println("RTCx valid");
    }
  }
  else {
    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);  M5.Lcd.println("NG");
  }
  if (!localtime_valid) {
    Serial.print("RTC not valid: set the localtime temporarily\n");
    local_tm.tm_year = 117;  // 2017 > 2016, getLocalTime() returns true
    local_tm.tm_mon  = 0;    // January
    local_tm.tm_mday = 1;
    local_tm.tm_hour = 0;
    local_tm.tm_min  = 0;
    local_tm.tm_sec  = 0;
    struct timeval tv = { mktime(&local_tm), 0 };
    settimeofday(&tv, NULL);
  }
  getLocalTime(&local_tm);
  Serial.print(&local_tm, "localtime: %A, %B %d %Y %H:%M:%S\n");
  // print sample: must be < 64
  //....:....1....:....2....:....3....:....4....:....5....:....6....
  //localtime: Wednesday, September 11 2021 11:10:46

  // WiFi start
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);  M5.Lcd.println("WiFi connecting.. ");
  WiFiManager wm;  // blocking mode only

  // erase SSID/Key to force rewrite
  if (digitalRead(BUTTON_A_PIN) == LOW) {
    wm.resetSettings();
    M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);  M5.Lcd.println("SSID/Key erased");
    delay(3000);
  }

  // WiFi connect
  wm.setConfigPortalTimeout(wifi_config_portal_timeout_sec);
  wm.setAPCallback(WifiConfigModeCallback);
  if (wm.autoConnect()) {
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);  M5.Lcd.println("OK");
  }
  else {
    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);  M5.Lcd.println("NG");
  }
  WiFi.setSleep(false);  // https://macsbug.wordpress.com/2021/05/02/buttona-on-m5stack-does-not-work-properly/
  wifi_retry_last_ms = millis() - wifi_retry_interval_ms;

  // NTP start
  NtpBegin(time_zone, ntp_server);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);  M5.Lcd.println("NTP start");
  delay(3000);

  // wait button-C to proceed into loop
  M5.Lcd.setTextColor(TFT_CYAN, TFT_BLACK);  M5.Lcd.println("Button-C to continue:");
  while (!M5.BtnC.wasReleased()) {
    M5.update();
    WifiCheck();
    if (RtcxUpdate()) {
      localtime_valid = true;  // SNTP sync completed
    }
  }

  // RTCx PCF8563: disable CLKO and INT
  const bool enable_clock_out(true);
  rtcx.ClockOutForTrimmer(!enable_clock_out);
  rtcx.DisableTimer();
  rtcx.DisableTimerInterrupt();

  // for TLC5940
  TLC5940_init();
}

void loop()
{
  // WiFi recovery
  WifiCheck();

  // NTP sync
  if (RtcxUpdate()) {
    localtime_valid = true;  // SNTP sync completed
  }

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  M5.Lcd.println("demo_lamp");   demo_lamp();   delay(1500);
  M5.Lcd.println("demo_dot");    demo_dot();    delay(1500);
  M5.Lcd.println("demo_char");   demo_char();   delay(1500);
  M5.Lcd.println("demo_flow");   demo_flow();   delay(1500);
  M5.Lcd.println("demo_alien");  demo_alien();  delay(1500);
  M5.Lcd.println("demo_clock");  demo_clock();  delay(1500);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// text, attrbute, dot
const uint16_t TXT_MAX = 256;
const uint16_t DOT_MAX =  16;

char Txt[TXT_MAX + 1];
char Atr[TXT_MAX + 1];
char Dot[DOT_MAX + 1];

// clear text
void fill_txt(const char t, const char a)
{
  for(int i = 0; i < TXT_MAX; ++i) {
    Txt[i] = t;
    Atr[i] = a;
  }
  Txt[TXT_MAX] = '\0';
  Atr[TXT_MAX] = '\0';
}

// clear dot
void fill_dot(const char d)
{
  for(int i = 0; i < DOT_MAX; ++i)
    Dot[i] = d;
  Dot[DOT_MAX] = '\0';
}

// initialize common for demonstration
void demo_init(const uint8_t font, const uint16_t half_ms)
{
  TLC5940_font_set(font);
  TLC5940_decay_set(half_ms);
  fill_txt(' ', '0');
  TLC5940_text_set(Txt, Atr, 0, TXT_MAX);
  fill_dot(' ');
  TLC5940_dot_set(Dot);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// demo all segment
void demo_lamp()
{
  Serial.printf("[TLC5940] demo_lamp start\n");
  demo_init(TLC5940_FNT, 500);

  fill_dot('0');
  fill_txt(0x7f, '0');
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// demo dot
void demo_dot()
{
  Serial.printf("[TLC5940] demo_dot start\n");
  demo_init(TLC5940_FNT, 300);

  for(int k = 0; k < 3; ++k) {
    for(int j = 0; j < 16; ++j) {
      for(int i = 0; i < 16; ++i)
        Dot[i] = (i == j)? '0': '9';
      delay(100);
    }
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// demo character
void demo_char()
{
  Serial.printf("[TLC5940] demo_char start\n");
  demo_init(TLC5940_FNT, 100);

  for(int i = 0; i < 256; ++i)
    Txt[i] = i;
  Txt[256] = '\0';

  for(int i = 0; i < 256; i += 16) {
    TLC5940_text_set(Txt, Atr, i, 256);
    delay(1500);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// demo flow string
void demo_flow()
{
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
void demo_alien()
{
  Serial.printf("[TLC5940] demo_alien start\n");
  demo_init(TLC5940_FNT, 30);  demo_alien_sub();  delay(1000);
//  demo_init(TLC5940_ALT, 30);  demo_alien_sub();
}

void demo_alien_sub()
{
  int x[5] = {200, 300, 400, 500, 600, };  // location * 100
  int v[5] = {  5,   5,  10,  10,  20, };  // velocity * 100
  
  for(int t = 0; t < 200; ++t) {  // time

    // clear canvas
    fill_txt(' ', '0');

    // dot
    for(int i = 0; i < 16; ++i)
      Dot[i] = (t % 10 == 0)? '0': '9';
    
    // clock, spoke
    Txt[ 0] = 0x00 + t % 8;
    Txt[15] = 0x08 + t % 8;
    
    // aliens
    for(int i = 0; i < 5; ++i) {
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
void demo_clock()
{
  Serial.printf("[TLC5940] demo_clock start\n");
  struct tm timeinfo;

  demo_init(TLC5940_FNT, 100);
  Dot[ 3] = '0';
  Dot[ 5] = '0';
  Dot[10] = '0';
  Dot[12] = '0';
  for(int i = 0; i < 40; ++i) {
    getLocalTime(&timeinfo);
    strftime(Txt, 17, "%Y%m%d %H%M%S ", &timeinfo);  // "yyyymmdd hhmmss*"
    Txt[15] = 0x08 + i % 8; 
    delay(125);
  }

  demo_init(TLC5940_FNT, 100);
  Dot[ 4] = '0';
  for(int i = 0; i < 40; ++i) {
    getLocalTime(&timeinfo);
    strftime(Txt, 17, "  %a%e-%b-%Y", &timeinfo);  // "*DDDdd-MMM-yyyy"
    Txt[ 0] = 0x00 + i % 8; 
    delay(125);
  }

  demo_init(TLC5940_FNT, 25);  demo_clock_sub();
//  demo_init(TLC5940_ALT, 25);  demo_clock_sub();
}

void demo_clock_sub()
{
  struct tm timeinfo;
  uint16_t l = TXT_MAX;  // temporary

  for(int i = - TLC5940_GRP * TLC5940_COL; i <= l; ++i) {
    getLocalTime(&timeinfo);
    strftime(Txt, 128, "%A, %B %e %Y, %H:%M:%S", &timeinfo);  // "DDDD, MMMM dd yyyy, hh:mm:dd"
    l = strlen(Txt);
    TLC5940_text_set(Txt, Atr, i, l);
    delay(250);
  }
}
