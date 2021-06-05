/*!
 * @file Adafruit_TLC59711.cpp
 *
 * @mainpage Adafruit TLC59711 PWM/LED driver
 *
 * @section intro_sec Introduction
 *
 * This is a library for our Adafruit 12-channel PWM/LED driver
 *
 * Pick one up today in the adafruit shop!
 * ------> http://www.adafruit.com/products/1455
 *
 * Two SPI Pins are required to send data: clock and data pin.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text above must be included in any redistribution
 */

#include <Adafruit_TLC59711.h>

/*!
 *  @brief  Instantiates a new Adafruit_TLC59711 class
 *  @param  n
 *          number of connected drivers
 *  @param  c
 *          clock pin
 *  @param  d
 *          data pin
 */
Adafruit_TLC59711::Adafruit_TLC59711(uint8_t n, uint8_t c, uint8_t d) {
  numdrivers = n;

  BCr = BCg = BCb = 0x7F; // default 100% brigthness

  pwmbuffer = (uint16_t *)calloc(2, 12 * n);

  _spi_dev = new Adafruit_SPIDevice(-1, c, -1, d, 1000000);
}

/*!
 *  @brief  Instantiates a new Adafruit_TLC59711 class using provided SPI
 *  @param  n
 *          number of connected drivers
 *  @param  *theSPI
 *          spi object
 */
Adafruit_TLC59711::Adafruit_TLC59711(uint8_t n, SPIClass *theSPI) {
  numdrivers = n;

  BCr = BCg = BCb = 0x7F; // default 100% brigthness

  pwmbuffer = (uint16_t *)calloc(2, 12 * n);

  _spi_dev = new Adafruit_SPIDevice(-1, 1000000, SPI_BITORDER_MSBFIRST,
                                    SPI_MODE0, theSPI);
}

/*!
 *  @brief  Writes PWM buffer to board
 */
void Adafruit_TLC59711::write() {
  uint32_t command;

  // Magic word for write
  command = 0x25;

  command <<= 5;
  // OUTTMG = 1, EXTGCK = 0, TMGRST = 1, DSPRPT = 1, BLANK = 0 -> 0x16
  command |= 0x16;

  command <<= 7;
  command |= BCr;

  command <<= 7;
  command |= BCg;

  command <<= 7;
  command |= BCb;

  noInterrupts();

  _spi_dev->beginTransaction();
  for (uint8_t n = 0; n < numdrivers; n++) {
    _spi_dev->transfer(command >> 24);
    _spi_dev->transfer(command >> 16);
    _spi_dev->transfer(command >> 8);
    _spi_dev->transfer(command);

    // 12 channels per TLC59711
    for (int8_t c = 11; c >= 0; c--) {
      // 16 bits per channel, send MSB first
      _spi_dev->transfer(pwmbuffer[n * 12 + c] >> 8);
      _spi_dev->transfer(pwmbuffer[n * 12 + c]);
    }
  }

  delayMicroseconds(200);
  _spi_dev->endTransaction();

  interrupts();
}

/*!
 *  @brief  Set PWM value on selected channel
 *  @param  chan
 *          one from 12 channel (per driver) so there is 12 * number of drivers
 *  @param  pwm
 *          pwm value
 */
void Adafruit_TLC59711::setPWM(uint8_t chan, uint16_t pwm) {
  if (chan > 12 * numdrivers)
    return;
  pwmbuffer[chan] = pwm;
}

/*!
 *  @brief  Set RGB value for selected LED
 *  @param  lednum
 *          selected LED number that for which value will be set
 *  @param  r
 *          red value
 *  @param g
 *          green value
 *  @param b
 *          blue value
 */
void Adafruit_TLC59711::setLED(uint8_t lednum, uint16_t r, uint16_t g,
                               uint16_t b) {
  setPWM(lednum * 3, r);
  setPWM(lednum * 3 + 1, g);
  setPWM(lednum * 3 + 2, b);
}

/*!
 *  @brief  Get RGB value for selected LED
 *  @param  lednum
 *          selected LED number that for which value will be set
 *  @param  r
 *          red value
 *  @param g
 *          green value
 *  @param b
 *          blue value
 */
void Adafruit_TLC59711::getLED(uint8_t lednum, uint16_t &r, uint16_t &g,
                               uint16_t &b) {
  r = pwmbuffer[lednum * 3];
  g = pwmbuffer[lednum * 3 + 1];
  b = pwmbuffer[lednum * 3 + 2];
}

/*!
 *  @brief  Set the brightness of LED channels to same value
 *  @param  BC
 *          Brightness Control value
 */
void Adafruit_TLC59711::simpleSetBrightness(uint8_t BC) {
  if (BC > 127) {
    BC = 127; // maximum possible value since BC can only be 7 bit
  } else if (BC < 0) {
    BC = 0;
  }

  BCr = BCg = BCb = BC;
}

/*!
 *  @brief  Set the brightness of LED channels to specific value
 *  @param  bcr
 *          Brightness Control Red value
 *  @param  bcg
 *          Brightness Control Green value
 *  @param  bcb
 *          Brightness Control Blue value
 */
void Adafruit_TLC59711::setBrightness(uint8_t bcr, uint8_t bcg, uint8_t bcb) {
  if (bcr > 127) {
    bcr = 127; // maximum possible value since BC can only be 7 bit
  } else if (bcr < 0) {
    bcr = 0;
  }

  BCr = bcr;

  if (bcg > 127) {
    bcg = 127; // maximum possible value since BC can only be 7 bit
  } else if (bcg < 0) {
    bcg = 0;
  }

  BCg = bcg;

  if (bcb > 127) {
    bcb = 127; // maximum possible value since BC can only be 7 bit
  } else if (bcb < 0) {
    bcb = 0;
  }

  BCb = bcb;
}

/*!
 *  @brief  Begins SPI connection if there is not empty PWM buffer
 *  @return If successful returns true, otherwise false
 */
bool Adafruit_TLC59711::begin() {
  if (!pwmbuffer)
    return false;

  return _spi_dev->begin();
}
