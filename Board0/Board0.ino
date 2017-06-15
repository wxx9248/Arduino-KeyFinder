#include <avr/wdt.h>
#include <avr/pgmspace.h>
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
// #define DEBUG_SYM

#ifdef DEBUG_SYM
#include <stdio.h>
#else
#define printf(format, ...)
#endif

extern "C"
{
#include <string.h>
}

#define MAX_LENGTH 64
static char *pstr = new char[MAX_LENGTH];

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

#ifdef DEBUG_SYM

PROGMEM const char strWDTReset[] = "[%u] %s%s(): Watchdog reseted.\r\n";
PROGMEM const char strRFPMin[] = "[%u] %s%s(): RF: Min output power.\r\n";
PROGMEM const char strRFPMax[] = "[%u] %s%s(): RF: Max output power.\r\n";
PROGMEM const char strLCDInit[] = "[%u] %s%s(): LCD Initialized.\r\n";
PROGMEM const char strBuzzerInit[] = "[%u] %s%s(): Buzzer Initialized.\r\n";
PROGMEM const char strSysInitScr[] = "[%u] %s%s(): Showing initializing screen...\r\n";
PROGMEM const char strLCDClear[] = "[%u] %s%s(): LCD cleared.\r\n";
PROGMEM const char strUsrPrmpt[] = "[%u] %s%s(): Showing user prompt...\r\n";
PROGMEM const char strLEDInit[] = "[%u] %s%s(): LED of statement lit.\r\n";
PROGMEM const char strSysDfltScr[] = "[%u] %s%s(): Showing default screen & refreshing screen...\r\n";
PROGMEM const char strLEDWarnRST[] = "[%u] %s%s(): LED of warning reseted.\r\n";
PROGMEM const char strLEDOKRST[] = "[%u] %s%s(): LED of OK reseted.\r\n";
PROGMEM const char strLEDWarn[] = "[%u] %s%s(): LED of warn lit.\r\n";
PROGMEM const char strBuzz[] = "[%u] %s%s(): Buzzing...\r\n";
PROGMEM const char strLEDOK[] = "[%u] %s%s(): LED of OK lit.\r\n";
PROGMEM const char strSysInit[] = "[%u] %s%s(): System initializing...\r\n";
PROGMEM const char strWDTEnable[] = "[%u] %s%s(): Watchdog enabled.\r\n";
PROGMEM const char strRFInit[] = "[%u] %s%s(): RF initializing...\r\n";
PROGMEM const char strRFCn[] = "[%u] %s%s(): Channel = %u.   :P\r\n";
PROGMEM const char strRFCfg[] = "[%u] %s%s(): RF moduled configured.\r\n";
PROGMEM const char strSetTAddr[] = "[%u] %s%s(): Set target address.\r\n";
PROGMEM const char strSndMsg[] = "[%u] %s%s(): Sending message...\r\n";
PROGMEM const char strWtRsp[] = "[%u] %s%s(): Instruction sent. Waiting for response...\r\n";
PROGMEM const char strOver[] = "[%u] %s%s(): Complete.\r\n";
PROGMEM const char strMsgRecv[] = "[%u] %s%s(): Message received.\r\n";

#endif

void judge();
void find();
void showState(E_State);
void init();
void Delay(uint16_t);
char *pgm2str(char *);

#ifdef DEBUG_SYM
int sputchar(char, struct __file *);
void printf_begin();
#endif

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
    printf(System::pgm2str(System::strWDTReset), millis(), __func__);
  }

  if (digitalRead(HC_SR501) == HIGH)
  {
    System::judge();
    wdt_reset();
    printf(System::pgm2str(System::strWDTReset), millis(), __func__);
  }

  System::State = System::_DEFAULT;
  System::showState(System::State);
  wdt_reset();
  printf(System::pgm2str(System::strWDTReset), millis(), __func__);
  delay(200);
}

void System::judge()
{
  using RF::nRF24L01;
  using RF::send;
  using RF::receive;
  using RF::setPower;

  setPower(0x00);

  send(REQ_DETECT);
  setPower(0x01);


  wdt_reset();
  printf(System::pgm2str(System::strWDTReset), millis(), __func__);

  for (uint8_t i = 0; i < 5; i++)
  {
    if (receive() == RSP_DETECT)
    {
      System::State = System::_PASS;
      System::showState(System::State);
      wdt_reset();
      printf(System::pgm2str(System::strWDTReset), millis(), __func__);
      break;
    }
    else if (i == 4)
    {
      System::State = System::_WARN;
      System::showState(System::State);
      wdt_reset();
      printf(System::pgm2str(System::strWDTReset), millis(), __func__);
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
  printf(System::pgm2str(System::strWDTReset), millis(), __func__);

  State = _FIND;
  showState(State);
  System::Delay(3000);
}

void System::showState(E_State State)
{
  LCD12864RSPI LCD12864(LCD12864_E, LCD12864_RS, LCD12864_RW);
  printf(System::pgm2str(System::strLCDInit), millis(), __func__);
  Buzzer myBuzzer(BUZZER);
  printf(System::pgm2str(System::strBuzzerInit), millis(), __func__);

  const char GBK_str_init[] = { 0x20, 0x20, 0xD5, 0xFD, 0xD4, 0xDA, 0xB3, 0xF5, 0xCA, 0xBC, 0xBB, 0xAF, '\0'};
  const char GBK_str_default[] = { 0xD4, 0xBF, 0xB3, 0xD7, 0xCD, 0xFC, 0xB4, 0xF8, 0xCC, 0xE1, 0xD0, 0xD1, 0xCF, 0xB5, 0xCD, 0xB3, 0x20, 0x20, 0x20, 0x20, 0x20, 0x56, 0x31, 0x2E, 0x30, '\0' };
  const char GBK_str_warn[] = { 0x20, 0x20, 0xD4, 0xBF, 0xB3, 0xD7, 0xCD, 0xFC, 0xB4, 0xF8, 0xC0, 0xB2, 0xA3, 0xA1, '\0' };
  const char GBK_str_pass[] = { 0x20, 0x20, 0xBC, 0xC7, 0xD0, 0xD4, 0xB2, 0xBB, 0xB4, 0xED, 0xC5, 0xB6, 0xA1, 0xAB, '\0' };
  const char GBK_str_find[] = { 0xD5, 0xFD, 0xD4, 0xDA, 0xBA, 0xF4, 0xBD, 0xD0, 0xD4, 0xBF, 0xB3, 0xD7, 0xA1, 0xAD, 0xA1, 0xAD, '\0' };
  Note warn;

  warn.Freq = NOTE_E4;
  warn.Duration = 3000;

  wdt_reset();
  printf(System::pgm2str(System::strWDTReset), millis(), __func__);

  switch (State)
  {
    case _INIT:
      printf(System::pgm2str(System::strSysInitScr), millis(), __func__);
      LCD12864.clear();
      printf(System::pgm2str(System::strLCDClear), millis(), __func__);
      LCD12864.displayString(0, 0, GBK_str_init);
      printf(System::pgm2str(System::strUsrPrmpt), millis(), __func__);
      digitalWrite(LED_PSTATE, HIGH);
      printf(System::pgm2str(System::strLEDInit), millis(), __func__);
      break;

    case _DEFAULT:
      printf(System::pgm2str(System::strSysDfltScr), millis(), __func__);
      digitalWrite(LED_WARN, LOW);
      printf(System::pgm2str(System::strLEDWarnRST), millis(), __func__);
      digitalWrite(LED_PASS, LOW);
      printf(System::pgm2str(System::strLEDOKRST), millis(), __func__);
      LCD12864.clear();
      printf(System::pgm2str(System::strLCDClear), millis(), __func__);
      LCD12864.displayString(0, 0, GBK_str_default);
      printf(System::pgm2str(System::strUsrPrmpt), millis(), __func__);
      break;

    case _WARN:
      digitalWrite(LED_WARN, HIGH);
      printf(System::pgm2str(System::strLEDWarn), millis(), __func__);
      myBuzzer.buzz(&warn);
      printf(System::pgm2str(System::strBuzz), millis(), __func__);
      LCD12864.clear();
      printf(System::pgm2str(System::strLCDClear), millis(), __func__);
      LCD12864.displayString(0, 0, GBK_str_warn);
      printf(System::pgm2str(System::strUsrPrmpt), millis(), __func__);
      break;

    case _PASS:
      digitalWrite(LED_PASS, HIGH);
      printf(System::pgm2str(System::strLEDOK), millis(), __func__);
      LCD12864.clear();
      printf(System::pgm2str(System::strLCDClear), millis(), __func__);
      LCD12864.displayString(0, 0, GBK_str_pass);
      printf(System::pgm2str(System::strUsrPrmpt), millis(), __func__);
      break;

    case _FIND:
      LCD12864.clear();
      printf(System::pgm2str(System::strLCDClear), millis(), __func__);
      LCD12864.displayString(0, 0, GBK_str_find);
      printf(System::pgm2str(System::strUsrPrmpt), millis(), __func__);
      break;

    default:
      break;
  }
}

void System::init()
{
  State = _INIT;
  showState(State);

#ifdef DEBUG_SYM
  printf_begin();
#endif

  printf(System::pgm2str(System::strSysInit), millis(), __func__);

  pinMode(LED_PSTATE, OUTPUT);
  pinMode(LED_WARN, OUTPUT);
  pinMode(LED_PASS, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(HC_SR501, INPUT_PULLUP);

  wdt_enable(WDTO_1S);
  printf(System::pgm2str(System::strWDTEnable), millis(), __func__);
  System::Delay(1000);
}

void System::Delay(uint16_t ms)
{
  for (uint8_t i = 0; i < ms / 500; i++)
  {
    delay(500);
    wdt_reset();
    printf(System::pgm2str(System::strWDTReset), millis(), __func__);
  }
}

char *System::pgm2str(char *pgmIn)
{
  for (uint16_t i = 0; i < MAX_LENGTH; i++)
    *(pstr + i) = pgm_read_byte(&pgmIn[i]);

  return pstr;
}

#ifdef DEBUG_SYM

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
#endif

void RF::init()
{
  printf(System::pgm2str(System::strRFInit), millis(), __func__);

  nRF24L01.spi = &MirfHardwareSpi;
  nRF24L01.init();
  nRF24L01.setRADDR(SERVER_ID);
  nRF24L01.payload = sizeof (char);
  nRF24L01.channel = 9;
  printf(System::pgm2str(System::strRFCn), millis(), __func__, "", nRF24L01.channel);
  nRF24L01.config();
  printf(System::pgm2str(System::strRFCfg), millis(), __func__);
  wdt_reset();
  printf(System::pgm2str(System::strWDTReset), millis(), __func__);
}

void RF::send(byte msg)
{
  nRF24L01.setTADDR(CLIENT_ID);
  printf(System::pgm2str(System::strSetTAddr), millis(), __func__);

  printf(System::pgm2str(System::strSndMsg), millis(), __func__);
  nRF24L01.send(&msg);
  printf(System::pgm2str(System::strWtRsp), millis(), __func__);
  while (nRF24L01.isSending())
    continue;
  printf(System::pgm2str(System::strOver), millis(), __func__);
  wdt_reset();
  printf(System::pgm2str(System::strWDTReset), millis(), __func__);
}

byte RF::receive()
{
  byte msg = 0xff;

  if (!nRF24L01.isSending() && nRF24L01.dataReady())
  {
    nRF24L01.getData(&msg);
    printf(System::pgm2str(System::strMsgRecv), millis(), __func__);
  }

  wdt_reset();
  printf(System::pgm2str(System::strWDTReset), millis(), __func__);
  return msg;
}

NOT_FINISHED void RF::setPower(uint8_t mode)
{
  switch (mode)
  {
    case 0x00:          // Low
      //      nRF24L01.writeRegister();
      printf(System::pgm2str(System::strRFPMin), millis(), __func__);
      wdt_reset();
      printf(System::pgm2str(System::strWDTReset), millis(), __func__);
      break;

    case 0x01:          // High
      //      nRF24L01.writeRegister();
      printf(System::pgm2str(System::strRFPMax), millis(), __func__);
      wdt_reset();
      printf(System::pgm2str(System::strWDTReset), millis(), __func__);
      break;

    default:
      wdt_reset();
      printf(System::pgm2str(System::strWDTReset), millis(), __func__);
      break;
  }
}

