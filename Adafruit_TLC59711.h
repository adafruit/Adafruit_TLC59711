/*!
 *  @file Adafruit_TLC59711.h
 *
 *  This is a library for the Adafruit 12-channel PWM/LED driver
 *
 *  Designed specifically to work with the Adafruit 12-channel PWM/LED driver
 *
 *  Pick one up today in the adafruit shop!
 *  ------> https://www.adafruit.com/product/1455
 *
 *  Two SPI Pins are required to send data: clock and data pin.
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit andopen-source hardware by purchasing products
 *  from Adafruit!
 *
 *  Limor Fried/Ladyada (Adafruit Industries).
 *
 *  BSD license, all text above must be included in any redistribution
 */

#ifndef _ADAFRUIT_TLC59711_H
#define _ADAFRUIT_TLC59711_H

#include <Adafruit_SPIDevice.h>
#include <Arduino.h>

/*!
 *  @brief  Class that stores state and functions for interacting with
 *          TLC59711 Senor
 */
class Adafruit_TLC59711 {
public:
  Adafruit_TLC59711(uint8_t n, uint8_t c, uint8_t d);
  Adafruit_TLC59711(uint8_t n, SPIClass *theSPI = &SPI);

  bool begin();

  void setPWM(uint8_t chan, uint16_t pwm);
  void setLED(uint8_t lednum, uint16_t r, uint16_t g, uint16_t b);
  void getLED(uint8_t lednum, uint16_t &r, uint16_t &g, uint16_t &b);
  void write();
  void setBrightness(uint8_t bcr, uint8_t bcg, uint8_t bcb);
  void simpleSetBrightness(uint8_t BC);

private:
  uint16_t *pwmbuffer = NULL;

  uint8_t BCr = 0, BCg = 0, BCb = 0;
  int8_t numdrivers = 0;
  Adafruit_SPIDevice *_spi_dev = NULL;
};

#endif
