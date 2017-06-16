#include <Mirf.h>
#include <MirfHardwareSpiDriver.h>
#include <Mirf_nRF24L01.h>
#include "PortsDefine.h"

Mirf nRF24L01(nRF24L01_CE, nRF24L01_CSN);
byte value = 0;

void setup()
{
  Serial.begin(115200);
  nRF24L01.spi = &MirfHardwareSpi;
  nRF24L01.init();
  nRF24L01.setRADDR((byte *)"_SERV");
  nRF24L01.payload = sizeof(value);
  nRF24L01.channel = 9 + 2 + 4 + 8;
  nRF24L01.config();
}


void loop()
{
  nRF24L01.setTADDR((byte *)"_CLNT");
  value = random(255);
  nRF24L01.send((byte *)&value);
  Serial.println("Instruction sent.");
  while (nRF24L01.isSending()) delay(1);
  Serial.println("Data sent.");
  delay(1000);
}

