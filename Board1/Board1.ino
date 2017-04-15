#include <avr/wdt.h>
#include <stdio.h>
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
void init();
void warn();
int sputchar(char, struct __file *);
void printf_begin();
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
      {
        printf("[%u] %s%s(): Responding message...\r\n", millis(), __func__);
        send(RSP_DETECT);

        wdt_reset();
        printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
      }
      delayMicroseconds(50);
      break;

    case REQ_FIND:
      printf("[%u] %s%s(): Received finding message, keeping buzzing...\r\n", millis(), __func__);
      warn();
      break;

    default:
      break;
  }

  wdt_reset();
  printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
}

void System::init()
{
  printf_begin();
  printf("[%u] %s%s(): System initializing...\r\n", millis(), __func__);
  wdt_enable(WDTO_1S);
  printf("[%u] %s%s(): Watchdog enabled.\r\n", millis(), __func__);
}

void System::warn()
{
  Buzzer myBuzzer(BUZZER);
  printf("[%u] %s%s(): Buzzer Initialized.\r\n", millis(), __func__);

  Note warn;
  warn.Freq = NOTE_E4;
  warn.Duration = 0;

  wdt_disable();
  printf("[%u] %s%s(): Watchdog disabled.\r\n", millis(), __func__);

  printf("[%u] %s%s(): Buzzing...\r\n", millis(), __func__);
  for (;;)
    myBuzzer.buzz(&warn);
}

int System::sputchar(char c, struct __file *)
{
  Serial.write(c);
  return c;
}

void System::printf_begin()
{
  Serial.begin(115200);
  fdevopen(&sputchar, 0);
}

void RF::init()
{
  printf("[%u] %s%s(): RF initializing...\r\n", millis(), __func__);
  nRF24L01.spi = &MirfHardwareSpi;
  printf("[%u] %s%s(): SPI initialized.\r\n", millis(), __func__);
  nRF24L01.init();
  printf("[%u] %s%s(): CE & CSN pin initialized.\r\n", millis(), __func__);
  nRF24L01.setRADDR(CLIENT_ID);
  printf("[%u] %s%s(): Set local address.\r\n", millis(), __func__);
  nRF24L01.payload = sizeof (char);
  printf("[%u] %s%s(): Data payload = 1 bytes.\r\n", millis(), __func__);
  nRF24L01.channel = 9 + 2 + 4 + 8;
  printf("[%u] %s%s(): Channel = 9 + 2 + 4 + 8.   :P\r\n", millis(), __func__);
  nRF24L01.config();
  printf("[%u] %s%s(): RF moduled configured.\r\n", millis(), __func__);
  wdt_reset();
  printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
}

void RF::send(byte msg)
{
  nRF24L01.setTADDR(SERVER_ID);
  printf("[%u] %s%s(): Set target address.\r\n", millis(), __func__);

  printf("[%u] %s%s(): Sending message...\r\n", millis(), __func__);
  nRF24L01.send(&msg);
  printf("[%u] %s%s(): Instruction sent. Waiting for response...\r\n", millis(), __func__);
  while (nRF24L01.isSending())
    continue;
  printf("[%u] %s%s(): Complete.\r\n", millis(), __func__);
  wdt_reset();
  printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
}

byte RF::receive()
{
  byte msg = 0xff;

  if (!nRF24L01.isSending() && nRF24L01.dataReady())
  {
    nRF24L01.getData(&msg);
    printf("[%u] %s%s(): Message received.\r\n", millis(), __func__);
  }

  wdt_reset();
  printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
  return msg;
}


