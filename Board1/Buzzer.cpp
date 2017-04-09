/*
	Buzzer driver library for Arduino
	by wxx9248
*/


#include "Buzzer.h"
#include "Pitches.h"

Buzzer::Buzzer(uint8_t pin)
{
  init(pin);
}

void Buzzer::init(uint8_t pin_in)
{
  pin = pin_in;
  pinMode(pin, OUTPUT);
}

void Buzzer::buzz(pNote note)
{
  tone(pin, note->Freq, note->Duration);
}

void Buzzer::play(pNotes notes)
{
  for (uint16_t i = 0; i < sizeof(*notes) / sizeof(Note); i++)
    buzz(notes[i]);
}
