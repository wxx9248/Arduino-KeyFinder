/*
  @Author:TONYLABS
  @Date:2012/12/21
  @Function: Driver 12864 LCD Module
*/

#include "LCD12864RSPI.h"

extern "C"
{
#include <inttypes.h>
#include <string.h>
}


LCD12864RSPI::LCD12864RSPI(const uint8_t clockPin, const uint8_t latchPin, const uint8_t dataPin)
{
  init(clockPin, latchPin, dataPin);
}

void LCD12864RSPI::delay()
{
  delayMicroseconds(DELAYTIME);
}


void LCD12864RSPI::writeByte(const uint8_t data)
{
  digitalWrite(latchPin, HIGH);
  delay();
  shiftOut(dataPin, clockPin, MSBFIRST, data);
  digitalWrite(latchPin, LOW);
}


void LCD12864RSPI::writeCommand(const uint8_t command)
{
  int H_data, L_data;
  H_data = command;
  H_data &= 0xf0;
  L_data = command;
  L_data &= 0x0f;
  L_data <<= 4;
  writeByte(0xf8);
  writeByte(H_data);
  writeByte(L_data);
}


void LCD12864RSPI::writeData(const uint8_t command)
{
  int H_data, L_data;
  H_data = command;
  H_data &= 0xf0;
  L_data = command;
  L_data &= 0x0f;
  L_data <<= 4;
  writeByte(0xfa);
  writeByte(H_data);
  writeByte(L_data);
}



void LCD12864RSPI::init(const uint8_t clockPin_in, const uint8_t latchPin_in, const uint8_t dataPin_in)
{
  clockPin = clockPin_in;
  latchPin = latchPin_in;
  dataPin = dataPin_in;

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  digitalWrite(latchPin, LOW);
  delay();

  writeCommand(0x30);
  writeCommand(0x0c);
  writeCommand(0x01);
  writeCommand(0x06);
}


void LCD12864RSPI::clear(void)
{
  writeCommand(0x30);
  writeCommand(0x01);
}


void LCD12864RSPI::displayString(const uint8_t X, uint8_t Y, const char *pstr)
{
  switch (X)
  {
    case 0:  Y |= 0x80; break;
    case 1:  Y |= 0x90; break;
    case 2:  Y |= 0x88; break;
    case 3:  Y |= 0x98; break;
    default: break;
  }
  writeCommand(Y);

  for (uint8_t i = 0; i < strlen(pstr); i++)
    writeData(*(pstr + i));
}

void LCD12864RSPI::displayString(const uint8_t X, uint8_t Y, const char *pstr, const uint8_t len)
{
  switch (X)
  {
    case 0:  Y |= 0x80; break;
    case 1:  Y |= 0x90; break;
    case 2:  Y |= 0x88; break;
    case 3:  Y |= 0x98; break;
    default: break;
  }
  writeCommand(Y);

  for (uint8_t i = 0; i < len || pstr + i; i++)
    writeData(*(pstr + i));
}


void LCD12864RSPI::displaySig(const uint8_t M, uint8_t N, const uint8_t sig)
{
  switch (M)
  {
    case 0:  N |= 0x80; break;
    case 1:  N |= 0x90; break;
    case 2:  N |= 0x88; break;
    case 3:  N |= 0x98; break;
    default: break;
  }
  writeCommand(N);
  writeData(sig);
}




void LCD12864RSPI::drawFullScreen(const uint8_t *p)
{
  uint8_t ygroup, x, y;
  uint16_t tmp;

  for (ygroup = 0; ygroup < 64; ygroup++)
  {
    if (ygroup < 32)
    {
      x = 0x80;
      y = ygroup + 0x80;
    }
    else
    {
      x = 0x88;
      y = ygroup - 32 + 0x80;
    }

    writeCommand(0x34);
    writeCommand(y);
    writeCommand(x);
    writeCommand(0x30);
    tmp = ygroup * 16;

    for (uint8_t i = 0; i < 16; i++)
      writeData(p[tmp++]);
  }

  writeCommand(0x34);
  writeCommand(0x36);
}
