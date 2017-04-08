#include <avr/wdt.h>
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
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PSTATE, OUTPUT);
  pinMode(LED_WARN, OUTPUT);
  pinMode(LED_PASS, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(HC_SR501, INPUT_PULLUP);

  System::init();
  RF::init();
}

void loop()
{
  if (digitalRead(BUTTON) == HIGH)
  {
    System::find();
    wdt_reset();
  }

  if (digitalRead(HC_SR501) == HIGH)
  {
    System::judge();
    wdt_reset();
  }

  System::State = System::_DEFAULT;
  System::showState(System::State);
  Serial.println(digitalRead(HC_SR501));
  wdt_reset();
  delay(200);
}

void System::judge()
{
  using RF::nRF24L01;
  using RF::send;
  using RF::receive;
  using RF::setPower;

  byte tmp = 0xff;

  setPower(0x00);
  send(REQ_DETECT);
  setPower(0x01);

  wdt_reset();

  for (uint8_t i = 0; i < 5; i++)
  {
    if ((tmp = receive()) == RSP_DETECT)
    {
      System::State = System::_PASS;
      System::showState(System::State);
      wdt_reset();
      break;
    }
    else if (i == 4)
    {
      System::State = System::_WARN;
      System::showState(System::State);
      wdt_reset();
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
  send(REQ_FIND);
  wdt_reset();

  State = _FIND;
  showState(State);
  System::Delay(3000);
}

void System::showState(E_State State)
{
  LCD12864RSPI LCD12864(LCD12864_E, LCD12864_RS, LCD12864_RW);
  Buzzer myBuzzer(BUZZER);

  const char GBK_str_init[] = { 0x20, 0x20, 0xD5, 0xFD, 0xD4, 0xDA, 0xB3, 0xF5, 0xCA, 0xBC, 0xBB, 0xAF, '\0'};
  const char GBK_str_default[] = { 0xD4, 0xBF, 0xB3, 0xD7, 0xCD, 0xFC, 0xB4, 0xF8, 0xCC, 0xE1, 0xD0, 0xD1, 0xCF, 0xB5, 0xCD, 0xB3, 0x20, 0x20, 0x20, 0x20, 0x20, 0x56, 0x31, 0x2E, 0x30, '\0' };
  const char GBK_str_warn[] = { 0x20, 0x20, 0xD4, 0xBF, 0xB3, 0xD7, 0xCD, 0xFC, 0xB4, 0xF8, 0xC0, 0xB2, 0xA3, 0xA1, '\0' };
  const char GBK_str_pass[] = { 0x20, 0x20, 0xBC, 0xC7, 0xD0, 0xD4, 0xB2, 0xBB, 0xB4, 0xED, 0xC5, 0xB6, 0xA1, 0xAB, '\0' };
  const char GBK_str_find[] = { 0xD5, 0xFD, 0xD4, 0xDA, 0xBA, 0xF4, 0xBD, 0xD0, 0xD4, 0xBF, 0xB3, 0xD7, 0xA1, 0xAD, 0xA1, 0xAD, '\0' };
  Note warn;

  warn.Freq = NOTE_E4;
  warn.Duration = 3000;

  wdt_reset();

  switch (State)
  {
    case _INIT:
      LCD12864.clear();
      LCD12864.displayString(0, 0, GBK_str_init);
      digitalWrite(LED_PSTATE, HIGH);
      break;

    case _DEFAULT:
      digitalWrite(LED_WARN, LOW);
      digitalWrite(LED_PASS, LOW);
      LCD12864.clear();
      LCD12864.displayString(0, 0, GBK_str_default);
      break;

    case _WARN:
      digitalWrite(LED_WARN, HIGH);
      myBuzzer.buzz(&warn);
      LCD12864.clear();
      LCD12864.displayString(0, 0, GBK_str_warn);
      break;

    case _PASS:
      digitalWrite(LED_PASS, HIGH);
      LCD12864.clear();
      LCD12864.displayString(0, 0, GBK_str_pass);
      break;

    case _FIND:
      LCD12864.clear();
      LCD12864.displayString(0, 0, GBK_str_find);
      break;

    default:
      break;
  }
}

void System::init()
{
  State = _INIT;
  showState(State);
  wdt_enable(WDTO_1S);
  System::Delay(1000);
}

void System::Delay(uint16_t ms)
{
  for (uint8_t i = 0; i < ms / 500; i++)
  {
    delay(500);
    wdt_reset();
  }
}


void RF::init()
{
  nRF24L01.spi = &MirfHardwareSpi;
  nRF24L01.init();
  nRF24L01.setTADDR(CLIENT_ID);
  nRF24L01.setRADDR(SERVER_ID);
  nRF24L01.payload = sizeof (char);
  nRF24L01.config();
  wdt_reset();
}

void RF::send(byte msg)
{
  nRF24L01.send(&msg);
  while (nRF24L01.isSending())
    wdt_reset();
}

byte RF::receive()
{
  byte msg = 0xff;

  if (!nRF24L01.isSending() && nRF24L01.dataReady())
  {
    nRF24L01.getData(&msg);
    wdt_reset();
  }
  return msg;
}

NOT_FINISHED void RF::setPower(uint8_t mode)
{
  switch (mode)
  {
    case 0x00:          // Low
      //      nRF24L01.writeRegister();
      wdt_reset();
      break;

    case 0x01:          // High
      //      nRF24L01.writeRegister();
      wdt_reset();
      break;

    default:
      wdt_reset();
      break;
  }
}

