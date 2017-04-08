#include "Mirf.h"
#include "Mirf_nRF24L01.h"
#include "MirfHardwareSpiDriver.h"
#include "PortsDefine.h"
#include "Buzzer.h"
#include "Pitches.h"
#include "ID.h"
#include "msg.h"

#define NOT_FINISHED

extern "C"
{
#include <string.h>
}

namespace RF
{
Mirf nRF24L01(nRF24L01_CE, nRF24L01_CSN);

void init();
void send(byte);
byte receive();
}

namespace System
{
Buzzer myBuzzer(BUZZER);
void init();
void warn();
}


void setup()
{
  System::init();
  RF::init();
}


void loop()
{
  using System::warn;
  using RF::send;
  using RF::receive;

  switch (receive())
  {
    case REQ_DETECT:
      for (uint8_t i = 0; i < 5; i++)
        send(RSP_DETECT);
      delayMicroseconds(50);
      break;

    case REQ_FIND:
      warn();
      break;

    default:
      break;
  }
}

void System::init()
{
  // Dummy.
}

void System::warn()
{
  using System::myBuzzer;

  Note warn;

  warn.Freq = NOTE_E4;
  warn.Duration = 0;

  for (;;)
    myBuzzer.buzz(&warn);
}


void RF::init()
{
  nRF24L01.spi = &MirfHardwareSpi;
  nRF24L01.init();
  nRF24L01.setTADDR(SERVER_ID);
  nRF24L01.setRADDR(CLIENT_ID);
  nRF24L01.payload = sizeof (char);
  nRF24L01.config();
}

void RF::send(byte msg)
{
  nRF24L01.send(&msg);
  while (nRF24L01.isSending())
    continue;
}

byte RF::receive()
{
  byte msg = 0xff;

  if (!nRF24L01.isSending() && nRF24L01.dataReady())
    nRF24L01.getData(&msg);
  return msg;
}


