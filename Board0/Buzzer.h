/*
*	Joystick driver library for Arduino
*	by wxx9248
*/

#ifndef _BUZZER_H
#define _BUZZER_H

#include <Arduino.h>

extern "C"
{
#include <inttypes.h>
}

struct Note
{
  uint16_t Freq;      // Hertz (Hz)
  uint32_t Duration;  // Millisecond (ms)
};

typedef Note *pNote;
typedef Note *(pNotes[]);

class Buzzer
{
  private:
    uint8_t pin;

  public:
    Buzzer(uint8_t);
    void init(uint8_t);
    void buzz(pNote);
    void play(pNotes);
};


#endif
