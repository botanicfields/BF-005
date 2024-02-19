// copyright 2019 BotanicFields, Inc.
// M5Stack/ESP32 + TLC5940 + 16 segment LED display
//
#include "BF_ESP32_TLC5940_SEG16.h"
#include <SPI.h>

// internal parameters
const char* TLC5940_text  = NULL;  // text data to display
const char* TLC5940_attr  = NULL;  // attribute to display
const char* TLC5940_dot   = NULL;  // dot to display in attribute code
int16_t     TLC5940_pos   = 0;     // start position to display
int16_t     TLC5940_len   = 1;     // length of the text data
int16_t     TLC5940_fonts = 0;     // select font 
int16_t     TLC5940_decay = 2;     // decay parameter for half-life 

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// initialize main
void TLC5940_init() {
  Serial.printf("[TLC5940] initialize start\n");

  // internal parameters, images
  TLC5940_image_clear();
  TLC5940_intensity_init();

  // user parameters, images
  TLC5940_font_set();
  TLC5940_decay_set();
  TLC5940_dot_set(TLC5940_SAMPLE_DOT);
  TLC5940_text_set(TLC5940_SAMPLE_TEXT, TLC5940_SAMPLE_ATTR, 0, strlen(TLC5940_SAMPLE_TEXT));

  // hardware controls
  TLC5940_pin_init();
  TLC5940_ledc_init();
  TLC5940_spi_init();
  TLC5940_dc_init();
  TLC5940_timer_init();
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// pin definition
const uint8_t TLC5940_PIN_MOSI  = 23;  // GPIO23
const uint8_t TLC5940_PIN_MISO  = 19;  // GPIO19 .. dummy
const uint8_t TLC5940_PIN_SCLK  = 18;  // GPIO18
const uint8_t TLC5940_PIN_SS    =  5;  // GPIO5
const uint8_t TLC5940_PIN_VPRG  = 13;  // GPIO13
const uint8_t TLC5940_PIN_XLAT  = 12;  // GPIO12
const uint8_t TLC5940_PIN_BLANK = 15;  // GPIO15
const uint8_t TLC5940_PIN_GSCLK =  2;  // GPIO2

// assign pins
void TLC5940_pin_init() {
  pinMode(TLC5940_PIN_VPRG,  OUTPUT);   
  pinMode(TLC5940_PIN_XLAT,  OUTPUT);   
  pinMode(TLC5940_PIN_BLANK, OUTPUT);   
  digitalWrite(TLC5940_PIN_VPRG,  LOW);   // LOW: GS-reg, HIGH: DC-reg
  digitalWrite(TLC5940_PIN_XLAT,  LOW);
  digitalWrite(TLC5940_PIN_BLANK, HIGH);  // start with BLANK
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// LEDC for GS(Gray-scale) clock
const uint8_t  TLC5940_LEDC_CHN = 0;    // LEDC channel
const double   TLC5940_LEDC_FRQ = 2e6;  // LEDC frequency 2MHz
const uint8_t  TLC5940_LEDC_RSL = 5;    // LEDC resolution 5bit = 32
const uint32_t TLC5940_LEDC_DTY = 8;    // LEDC duty 25% .. 8 / 32

// start GSCLK
void TLC5940_ledc_init() {
  ledcSetup(TLC5940_LEDC_CHN, TLC5940_LEDC_FRQ, TLC5940_LEDC_RSL);
  ledcAttachPin(TLC5940_PIN_GSCLK, TLC5940_LEDC_CHN);
  ledcWrite(TLC5940_LEDC_CHN, TLC5940_LEDC_DTY);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// timer for BLANK, XLAT task
const uint8_t  TLC5940_TIMER_NUM = 0;     // hardware timer number
const uint16_t TLC5940_TIMER_DIV = 80;    // timer divider to make 1MHz from 80MHz
const uint64_t TLC5940_TIMER_ALM = 2050;  // interrupt every 2.050ms 

// timer interruption
hw_timer_t* TLC5940_timer = NULL;
volatile SemaphoreHandle_t TLC5940_timerSemaphore;
portMUX_TYPE TLC5940_timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t TLC5940_count = 0;

// set up timer
void TLC5940_timer_init() {

  // create semaphore
  TLC5940_timerSemaphore = xSemaphoreCreateBinary();

  // standby TLC5940 task
  xTaskCreatePinnedToCore(TLC5940_task,   // Function to implement the task
                         "TLC5940_task",  // Name of the task
                          4096,           // Stack size in words
                          NULL,           // Task input parameter
                          3,              // Priority of the task
                          NULL,           // Task handle.
                          1);             // Core where the task should run

  // start timer
  TLC5940_timer = timerBegin(TLC5940_TIMER_NUM, TLC5940_TIMER_DIV, true);  // increment mode
  timerAttachInterrupt(TLC5940_timer, &TLC5940_onTimer, true);             // edge mode
  timerAlarmWrite(TLC5940_timer, TLC5940_TIMER_ALM, true);                 // auto-reload mode
  timerAlarmEnable(TLC5940_timer);
}

// handle timer interruption
void IRAM_ATTR TLC5940_onTimer(){

  // BLANK, XLAT
  digitalWrite(TLC5940_PIN_BLANK, HIGH);
  digitalWrite(TLC5940_PIN_XLAT,  HIGH);
  digitalWrite(TLC5940_PIN_XLAT,  LOW);
  digitalWrite(TLC5940_PIN_BLANK, LOW);

  // increment counter & check task status
  portENTER_CRITICAL_ISR(&TLC5940_timerMux);
  TLC5940_count++;
  portEXIT_CRITICAL_ISR(&TLC5940_timerMux);

  // wake TLC5940-task up
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(TLC5940_timerSemaphore, &xHigherPriorityTaskWoken);
}

// send GS data through SPI
void TLC5940_task(void * pvParameters) {
  for(;;) {

    // wait for semaphore
    xSemaphoreTake(TLC5940_timerSemaphore, portMAX_DELAY);

    // get counter
    portENTER_CRITICAL(&TLC5940_timerMux);
    uint16_t count = TLC5940_count;
    portEXIT_CRITICAL(&TLC5940_timerMux);

    // update gs
    TLC5940_update(count);
    TLC5940_spi_gs_send(count);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// SPI to send gray-scale data
const uint32_t TLC5940_SPI_CLK = 8000000;  // 8MHz
SPIClass* TLC5940_vspi = NULL;

// initialize SPI
void TLC5940_spi_init() {
  TLC5940_vspi = new SPIClass(VSPI);
  TLC5940_vspi->begin(TLC5940_PIN_SCLK, TLC5940_PIN_MISO, TLC5940_PIN_MOSI, TLC5940_PIN_SS);
  pinMode(TLC5940_PIN_SS, OUTPUT);
}

// send dot correction(DC) data
void TLC5940_dc_init() {
  uint8_t dc_reg[TLC5940_SEG * 3 / 4];

  // TLC5940_DC: aaaaaa-- bbbbbb-- cccccc-- dddddd--
  //             i*4      +1       +2       +3
  //     dc_reg: aaaaaabb bbbbcccc ccdddddd
  //             i*3      +1       +2
  for(int i = 0; i < TLC5940_SEG / 4; i++) {
    dc_reg[i * 3    ] =  (TLC5940_DC[i * 4    ] & 0xfc)       | (TLC5940_DC[i * 4 + 1] >> 6);
    dc_reg[i * 3 + 1] = ((TLC5940_DC[i * 4 + 1] & 0xfc) << 2) | (TLC5940_DC[i * 4 + 2] >> 4);
    dc_reg[i * 3 + 2] = ((TLC5940_DC[i * 4 + 2] & 0xfc) << 4) | (TLC5940_DC[i * 4 + 3] >> 2);
  }
  digitalWrite(TLC5940_PIN_VPRG, HIGH);  // select DC
  TLC5940_vspi->beginTransaction(SPISettings(TLC5940_SPI_CLK, MSBFIRST, SPI_MODE0));
  digitalWrite(TLC5940_PIN_SS, LOW); // active
  for(int k = TLC5940_GRP - 1; k >= 0; k--) {
    TLC5940_vspi->transfer(0);  // dummy, column select
    TLC5940_vspi->transfer(0);  // dummy, GS of dot
    for(int i = 0; i < TLC5940_SEG * 3 / 4; i++)
      TLC5940_vspi->transfer(dc_reg[i]);
  }  
  digitalWrite(TLC5940_PIN_SS, HIGH); // inactive
  TLC5940_vspi->endTransaction();
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// image buffer
uint16_t TLC5940_image_seg[TLC5940_GRP][TLC5940_COL][TLC5940_SEG];
uint16_t TLC5940_image_dot[TLC5940_GRP][TLC5940_COL];

// clear image buffer
void TLC5940_image_clear(const uint16_t c) {
  for(int k = 0; k < TLC5940_GRP; k++)
    for(int j = 0; j < TLC5940_COL; j++) {
      for(int i = 0; i < TLC5940_SEG; i++)
        TLC5940_image_seg[k][j][i] = c;
      TLC5940_image_dot[k][j] = c;
    }
}

// make and send GS data through SPI
void TLC5940_spi_gs_send(uint16_t count) {
  uint8_t gs_reg[TLC5940_SEG * 3 / 2];
  uint8_t col = count % 8;  //select column

  digitalWrite(TLC5940_PIN_VPRG, LOW);  // select GS
  TLC5940_vspi->beginTransaction(SPISettings(TLC5940_SPI_CLK, MSBFIRST, SPI_MODE0));
  digitalWrite(TLC5940_PIN_SS, LOW); // active

  // for each groups
  for(int k = TLC5940_GRP - 1; k >= 0; k--) {
    TLC5940_vspi->transfer(0x80 >> col);                     // column selected
    TLC5940_vspi->transfer(TLC5940_image_dot[k][col] >> 8);  // dot
  
    // images    16bit: aaaaaaaaaaaa----    bbbbbbbbbbbb----
    //                  2*i                 +1
    // registers 12bit: aaaaaaaa    aaaabbbb    bbbbbbbb
    //                  3*i         +1          +2
    for(int i = 0; i < TLC5940_SEG / 2; i++) {
      uint16_t s0 = TLC5940_image_seg[k][col][i * 2    ];
      uint16_t s1 = TLC5940_image_seg[k][col][i * 2 + 1];
      gs_reg[i * 3    ] =  s0 >> 8;
      gs_reg[i * 3 + 1] = (s0 & 0x00f0) | (s1 >> 12);
      gs_reg[i * 3 + 2] =  s1 >> 4;
    }
    for(int i = 0; i < TLC5940_SEG * 3 / 2; i++)
      TLC5940_vspi->transfer(gs_reg[i]);
  }
  digitalWrite(TLC5940_PIN_SS, HIGH); // inactive
  TLC5940_vspi->endTransaction();
}

// update image buffer
void TLC5940_update(const uint16_t count) {
  uint8_t col = count % 8;  //select column

  // for each groups
  for(int k = 0; k < TLC5940_GRP; k++) {

    // update dot
    uint16_t* p = &TLC5940_image_dot[k][col];
    *p = *p - *p / TLC5940_decay;              // after image
    uint16_t q = TLC5940_dot_new(k, col);
    if(*p < q) *p = q;                         // refresh by dot data

    // update segment
    for(int i = 0; i < TLC5940_SEG; i++) {
      p = &TLC5940_image_seg[k][col][i];
      *p = *p - *p / TLC5940_decay;       // after image
      q = TLC5940_seg_new(k, col, i);
      if(*p < q) *p = q;                  // refresh by font
    }
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// intensity table
uint16_t TLC5940_intensity[TLC5940_INTENSITY_LENGTH];

// set intensity table
//   0(max) .. 9(min)
//   Anext = A - A / decay
void TLC5940_intensity_init(const uint16_t decay) {
  TLC5940_intensity[0] = 0xffff;
  for(int i = 1; i < TLC5940_INTENSITY_LENGTH; i++) {
    uint16_t* p = &TLC5940_intensity[i];
    *p = *(p - 1) - *(p - 1) / decay;
  }
  Serial.printf("[TLC5940] -intensity initialized: ");
  for(int i = 0; i < TLC5940_INTENSITY_LENGTH; i++)
    Serial.printf("%d, ", TLC5940_intensity[i]);
  Serial.printf("\n");
}

// refresh data of segment
uint16_t TLC5940_seg_new(const uint8_t grp, const uint8_t col, const uint8_t seg) {
  uint16_t gs = 0;
  uint16_t tp = TLC5940_pos + grp * TLC5940_COL + col;  // check range of text
  if((tp >= 0) && (tp < TLC5940_len)) {                 
    uint8_t fp = TLC5940_text[tp] + TLC5940_fonts;      // check font
    if((TLC5940_FONT[fp] & (0x8000 >> seg)) != 0) {     
      uint8_t attr = TLC5940_attr[tp];                  // check attribute

      // simple intensity
      if(attr >= TLC5940_INTENSITY_FROM)
        if((attr -= TLC5940_INTENSITY_FROM) < TLC5940_INTENSITY_LENGTH)
          gs = TLC5940_intensity[attr];
    }
  }
  return gs;
}

// refresh data of dot
uint16_t TLC5940_dot_new(const uint8_t grp, const uint8_t col) {
  uint16_t gs = 0;
  uint8_t attr = TLC5940_dot[grp * TLC5940_COL + col];  // check attribute

  // simple intensity
  if(attr >= TLC5940_INTENSITY_FROM)
    if((attr -= TLC5940_INTENSITY_FROM) < TLC5940_INTENSITY_LENGTH)
      gs = TLC5940_intensity[attr];

  return gs;
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// set font
void TLC5940_font_set(const uint8_t font) {
  TLC5940_fonts = font;
  Serial.printf("[TLC5940] -font set to %d\n", TLC5940_fonts);
}

// half-life time(ms) to decay parameter
//   after image: Anew <-- Aold - Aold / decay
//   decay = half-life:ms / update:2050us / columns:8 / ln(2):0.693
void TLC5940_decay_set(const uint16_t half_ms) {
  TLC5940_decay = (uint64_t)half_ms * 1000000 / TLC5940_TIMER_ALM / TLC5940_COL / 693;
  Serial.printf("[TLC5940] -decay set to %d\n", TLC5940_decay);
}

// set text and attribute
void TLC5940_text_set(const char* text, const char* attr, const int16_t pos, const uint16_t len) {
  TLC5940_text = text;
  TLC5940_attr = attr;
  TLC5940_pos  = pos;
  TLC5940_len  = len;
}

// set text and attribute
void TLC5940_dot_set(const char* dot) {
  TLC5940_dot = dot;
}

// set and flow text
void TLC5940_flow(const char* text, const char* attr, const uint16_t delay_ms) {
  TLC5940_text = text;
  TLC5940_attr = attr;
  TLC5940_len  = strlen(text);
  for(int i = - TLC5940_GRP * TLC5940_COL; i <= TLC5940_len; i++) {
    TLC5940_pos = i;
    delay(delay_ms);
  }
}
