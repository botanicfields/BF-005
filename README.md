# BF-005
16 Segment LED x 16  driven by TLC5940 and M5Stack/ESP32

https://youtu.be/Hr57Gi8mDxo

https://github.com/botanicfields/kicad

(1) configuration

Dynamic display of 8 columns a TLC5940 driver, controlled by a 74HC595(Shift Register).
The 17th segments(dots) are controlled by a 74HC595 and 8 resistors.
An M5Stack controls 2 TLC5940s for 16 columns of LED display modules.
Enable after-image effect by full PWM capability of TC5940.

(2) printed circuit boards

BF-005 LED board:

Mount 2 LED display modules.
2 LEDs are able to mount instead of a LED display module.
An TLC5940 and 2 74HC595's are installed on the master board.
No ICs are installed on the slave board.
A dynamic display group of 8 LED display modules is consist of 1 master and 3 slave board.
2 groups are connected for 16 columns of LED.

BF-010 interface board:

Handle SS(slave select) of VSPI and convert 3.3V to 5V.
LED indicater for XERR.
LED power output(5-12V).
Make logic power(5V).

BF-002 terminater board:

To reduce reflection of GSCLK, SLCK, ..

(3) tools

"esp32gpio.xlsx" is to consider pin assignments

"seg16font.xlsm" is to design font patterns

(4) signals for TLC5940

Reflesh rate: > 60Hz = 16.6ms.
Period per LED display = 2ms = 16ms / 8 columns.

GSCLK(gray-scale clock):

Generated by LEDC of ESP32.
For resolution of 12bit, GSCLK = 2MHz = 4,096 levels / 2ms.

BLANK, XLAT(latch):

Generated by timer interuption of ESP32 in every 2ms
The digitalWrite() functions are used in the interrput handler

DATA, SCLK(serial clock):

The VSPI is used to send serial data, with the clock speed of 8MHz.
Assign a unique SS(Slave Select) line, to share VSPI with LCD and microSD.
The MISO(Master In Slave Out) is not used.

XERR(error):

The lamp and thermal checks are indicated by an LED.
The detail information of error status is NOT treated.

(5) others

SmartConfig:

To configure SSID/PSK of Wifi, the SmartConfig is used.
The Wifi is used for NTP

NTP:

To show date and time, NTP is used
