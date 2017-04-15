#include <avr/wdt.h>
#include <stdio.h>
#include "Mirf.h"
#include "Mirf_nRF24L01.h"
#include "MirfHardwareSpiDriver.h"
#include "LCD12864RSPI.h"
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
void setPower(uint8_t);
void send(byte);
byte receive();
}

namespace System
{
enum E_State { _INIT, _DEFAULT, _WARN, _PASS, _FIND } State;

void judge();
void find();
void showState(E_State);
void init();
void Delay(uint16_t);
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
  if (digitalRead(BUTTON) == HIGH)
  {
    System::find();
    wdt_reset();
    printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
  }

  if (digitalRead(HC_SR501) == HIGH)
  {
    System::judge();
    wdt_reset();
    printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
  }

  System::State = System::_DEFAULT;
  System::showState(System::State);
  wdt_reset();
  printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
  delay(200);
}

void System::judge()
{
  using RF::nRF24L01;
  using RF::send;
  using RF::receive;
  using RF::setPower;

  setPower(0x00);
  printf("[%u] %s%s(): RF: Min output power.\r\n", millis(), __func__);
  send(REQ_DETECT);
  setPower(0x01);
  printf("[%u] %s%s(): RF: Max output power.\r\n", millis(), __func__);

  wdt_reset();
  printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);

  for (uint8_t i = 0; i < 5; i++)
  {
    if (receive() == RSP_DETECT)
    {
      System::State = System::_PASS;
      System::showState(System::State);
      wdt_reset();
      printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
      break;
    }
    else if (i == 4)
    {
      System::State = System::_WARN;
      System::showState(System::State);
      wdt_reset();
      printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
      break;
    }
    else
    {
      wdt_reset();
      continue;
    }
  }

  System::Delay(3000);
}

void System::find()
{
  using RF::nRF24L01;
  using RF::send;
  using RF::setPower;

  setPower(0x01);
  printf("[%u] %s%s(): RF: Max output power.\r\n", millis(), __func__);
  send(REQ_FIND);
  wdt_reset();
  printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);

  State = _FIND;
  showState(State);
  System::Delay(3000);
}

void System::showState(E_State State)
{
  LCD12864RSPI LCD12864(LCD12864_E, LCD12864_RS, LCD12864_RW);
  printf("[%u] %s%s(): LCD Initialized.\r\n", millis(), __func__);
  Buzzer myBuzzer(BUZZER);
  printf("[%u] %s%s(): Buzzer Initialized.\r\n", millis(), __func__);

  const char GBK_str_init[] = { 0x20, 0x20, 0xD5, 0xFD, 0xD4, 0xDA, 0xB3, 0xF5, 0xCA, 0xBC, 0xBB, 0xAF, '\0'};
  const char GBK_str_default[] = { 0xD4, 0xBF, 0xB3, 0xD7, 0xCD, 0xFC, 0xB4, 0xF8, 0xCC, 0xE1, 0xD0, 0xD1, 0xCF, 0xB5, 0xCD, 0xB3, 0x20, 0x20, 0x20, 0x20, 0x20, 0x56, 0x31, 0x2E, 0x30, '\0' };
  const char GBK_str_warn[] = { 0x20, 0x20, 0xD4, 0xBF, 0xB3, 0xD7, 0xCD, 0xFC, 0xB4, 0xF8, 0xC0, 0xB2, 0xA3, 0xA1, '\0' };
  const char GBK_str_pass[] = { 0x20, 0x20, 0xBC, 0xC7, 0xD0, 0xD4, 0xB2, 0xBB, 0xB4, 0xED, 0xC5, 0xB6, 0xA1, 0xAB, '\0' };
  const char GBK_str_find[] = { 0xD5, 0xFD, 0xD4, 0xDA, 0xBA, 0xF4, 0xBD, 0xD0, 0xD4, 0xBF, 0xB3, 0xD7, 0xA1, 0xAD, 0xA1, 0xAD, '\0' };
  Note warn;

  warn.Freq = NOTE_E4;
  warn.Duration = 3000;

  wdt_reset();
  printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);

  switch (State)
  {
    case _INIT:
      printf("[%u] %s%s(): Showing initializing screen...\r\n", millis(), __func__);
      LCD12864.clear();
      printf("[%u] %s%s(): LCD cleared.\r\n", millis(), __func__);
      LCD12864.displayString(0, 0, GBK_str_init);
      printf("[%u] %s%s(): Showing user prompt...\r\n", millis(), __func__);
      digitalWrite(LED_PSTATE, HIGH);
      printf("[%u] %s%s(): LED of statement lit.\r\n", millis(), __func__);
      break;

    case _DEFAULT:
      printf("[%u] %s%s(): Showing default screen & refreshing screen...\r\n", millis(), __func__);
      digitalWrite(LED_WARN, LOW);
      printf("[%u] %s%s(): LED of warning reseted.\r\n", millis(), __func__);
      digitalWrite(LED_PASS, LOW);
      printf("[%u] %s%s(): LED of OK reseted.\r\n", millis(), __func__);
      LCD12864.clear();
      printf("[%u] %s%s(): LCD cleared.\r\n", millis(), __func__);
      LCD12864.displayString(0, 0, GBK_str_default);
      printf("[%u] %s%s(): Showing user prompt...\r\n", millis(), __func__);
      break;

    case _WARN:
      digitalWrite(LED_WARN, HIGH);
      printf("[%u] %s%s(): LED of warn lit.\r\n", millis(), __func__);
      myBuzzer.buzz(&warn);
      printf("[%u] %s%s(): Buzzing...\r\n", millis(), __func__);
      LCD12864.clear();
      printf("[%u] %s%s(): LCD cleared.\r\n", millis(), __func__);
      LCD12864.displayString(0, 0, GBK_str_warn);
      printf("[%u] %s%s(): Showing user prompt...\r\n", millis(), __func__);
      break;

    case _PASS:
      digitalWrite(LED_PASS, HIGH);
      printf("[%u] %s%s(): LED of OK lit.\r\n", millis(), __func__);
      LCD12864.clear();
      printf("[%u] %s%s(): LCD cleared.\r\n", millis(), __func__);
      LCD12864.displayString(0, 0, GBK_str_pass);
      printf("[%u] %s%s(): Showing user prompt...\r\n", millis(), __func__);
      break;

    case _FIND:
      LCD12864.clear();
      printf("[%u] %s%s(): LCD cleared.\r\n", millis(), __func__);
      LCD12864.displayString(0, 0, GBK_str_find);
      printf("[%u] %s%s(): Show user prompt...\r\n", millis(), __func__);
      break;

    default:
      break;
  }
}

void System::init()
{
  State = _INIT;
  showState(State);
  printf_begin();

  printf("[%u] %s%s(): System initializing...\r\n", millis(), __func__);

  pinMode(LED_PSTATE, OUTPUT);
  pinMode(LED_WARN, OUTPUT);
  pinMode(LED_PASS, OUTPUT);
  printf("[%u] %s%s(): LED pinMode = OUTPUT\r\n", millis(), __func__);
  pinMode(BUTTON, INPUT_PULLUP);
  printf("[%u] %s%s(): BUTTON pinMode = INPUT_PULLUP\r\n", millis(), __func__);
  pinMode(HC_SR501, INPUT_PULLUP);
  printf("[%u] %s%s(): HC_SR501 pinMode = INPUT_PULLUP\r\n", millis(), __func__);

  wdt_enable(WDTO_1S);
  printf("[%u] %s%s(): Watchdog enabled.\r\n", millis(), __func__);
  System::Delay(1000);
}

void System::Delay(uint16_t ms)
{
  for (uint8_t i = 0; i < ms / 500; i++)
  {
    delay(500);
    wdt_reset();
    printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
  }
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
  nRF24L01.setRADDR(SERVER_ID);
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
  nRF24L01.setTADDR(CLIENT_ID);
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

NOT_FINISHED void RF::setPower(uint8_t mode)
{
  switch (mode)
  {
    case 0x00:          // Low
      //      nRF24L01.writeRegister();
      wdt_reset();
      printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
      break;

    case 0x01:          // High
      //      nRF24L01.writeRegister();
      wdt_reset();
      printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
      break;

    default:
      wdt_reset();
      printf("[%u] %s%s(): Watchdog reseted.\r\n", millis(), __func__);
      break;
  }
}

