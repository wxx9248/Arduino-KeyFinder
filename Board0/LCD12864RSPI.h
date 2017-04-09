/*
  @Author:TONYLABS
  @Date:2012/12/21
  @Function: Driver 12864 LCD Module
*/

#ifndef _LCD12864RSPI_H
#define _LCD12864RSPI_H

#include <Arduino.h>
#include <inttypes.h>

class LCD12864RSPI
{
  private:
    void delay();
    static const uint8_t DELAYTIME = 80; // 80 ��s
    uint8_t clockPin, latchPin, dataPin;

  public:
    LCD12864RSPI(const uint8_t, const uint8_t, const uint8_t);
    void init(const uint8_t, const uint8_t, const uint8_t);
    void writeByte(const uint8_t);
    void writeCommand(const uint8_t);
    void writeData(const uint8_t);
    void clear();
    void displayString(const uint8_t, uint8_t, const char *);
    void displayString(const uint8_t, uint8_t, const char *, const uint8_t);
    void displaySig(const uint8_t, const uint8_t, const uint8_t);
    void drawFullScreen(const uint8_t *);

};


#endif
