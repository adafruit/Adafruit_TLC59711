/*************************************************** 
  This is a library for our Adafruit 24-channel PWM/LED driver

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1455

  These drivers uses SPI to communicate, 3 pins are required to  
  interface: Data, Clock and Latch. The boards are chainable

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_TLC59711.h>
#include <SPI.h>

Adafruit_TLC59711::Adafruit_TLC59711(uint8_t n, uint8_t c, uint8_t d) {
  numdrivers = n;
  _clk = c;
  _dat = d;

  BCr = BCg = BCb = 0x7F; //default 100% brigthness

  pwmbuffer = (uint16_t *)calloc(2, 12*n);
}

Adafruit_TLC59711::Adafruit_TLC59711(uint8_t n) {
  numdrivers = n;
  _clk = -1;
  _dat = -1;

  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.setDataMode(SPI_MODE0);
  BCr = BCg = BCb = 0x7F; //default 100% brigthness

  pwmbuffer = (uint16_t *)calloc(2, 12*n);
}

void  Adafruit_TLC59711::spiwriteMSB(uint32_t d) {

  if (_clk >= 0) {
    uint32_t b = 0x80;
    //  b <<= (bits-1);
    for (; b!=0; b>>=1) {
      digitalWrite(_clk, LOW);
      if (d & b)  
	digitalWrite(_dat, HIGH);
      else
	digitalWrite(_dat, LOW);
      digitalWrite(_clk, HIGH);
    }
  } else {
    SPI.transfer(d);
  }
}

void Adafruit_TLC59711::write(void) {
  uint32_t command;

  // Magic word for write
  command = 0x25;

  command <<= 5;
  //OUTTMG = 1, EXTGCK = 0, TMGRST = 1, DSPRPT = 1, BLANK = 0 -> 0x16
  command |= 0x16;

  command <<= 7;
  command |= BCr;

  command <<= 7;
  command |= BCg;

  command <<= 7;
  command |= BCb;

  cli();
  for (uint8_t n=0; n<numdrivers; n++) {
    spiwriteMSB(command >> 24);
    spiwriteMSB(command >> 16);
    spiwriteMSB(command >> 8);
    spiwriteMSB(command);

    // 12 channels per TLC59711
    for (int8_t c=11; c >= 0 ; c--) {
      // 16 bits per channel, send MSB first
      spiwriteMSB(pwmbuffer[n*12+c]>>8);
      spiwriteMSB(pwmbuffer[n*12+c]);
    }
  }

  if (_clk >= 0)
    delayMicroseconds(200);
  else
    delayMicroseconds(2);
  sei();
}



void Adafruit_TLC59711::setPWM(uint8_t chan, uint16_t pwm) {
  if (chan > 12*numdrivers) return;
  pwmbuffer[chan] = pwm;  
}


void Adafruit_TLC59711::setLED(uint8_t lednum, uint16_t r, uint16_t g, uint16_t b) {
  setPWM(lednum*3, r);
  setPWM(lednum*3+1, g);
  setPWM(lednum*3+2, b);
}

void Adafruit_TLC59711::simpleSetBrightness(uint8_t BC){
//sets the brightness of all LED channels to  same value
  if(BC > 127){
    BC = 127; //maximum possible value since BC can only be 7 bit
  } else if(BC < 0){
    BC = 0;
  }
  
  BCr = BCg = BCb = BC;
}

void Adafruit_TLC59711::setBrightness(uint8_t BCr, uint8_t BCg, uint8_t BCb){
  if(bcr > 127){
    bcr = 127; //maximum possible value since BC can only be 7 bit
  } else if(bcr < 0){
    bcr = 0;
  }
  
  BCr = bcr;
  
  if(bcg > 127){
    bcg = 127; //maximum possible value since BC can only be 7 bit
  } else if(bcg < 0){
    bcg = 0;
  }
  
  BCg = bcg;
  
  if(bcb > 127){
    bcb = 127; //maximum possible value since BC can only be 7 bit
  } else if(bcb < 0){
    bcb = 0;
  }
  
  BCb = bcb;
}


boolean Adafruit_TLC59711::begin() {
  if (!pwmbuffer) return false;

  if (_clk >= 0) {
    pinMode(_clk, OUTPUT);
    pinMode(_dat, OUTPUT);
  } else {
    SPI.begin();
  }
  return true;
}
