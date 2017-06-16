#include <Mirf.h>
#include <MirfHardwareSpiDriver.h>
#include <Mirf_nRF24L01.h>
#include "PortsDefine.h"

Mirf nRF24L01(nRF24L01_CE, nRF24L01_CSN);
byte value = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("Program started.");
  nRF24L01.spi = &MirfHardwareSpi;
  nRF24L01.init();
  nRF24L01.setRADDR((byte *)"_CLNT");
  nRF24L01.payload = sizeof(value);
  nRF24L01.channel = 9 + 2 + 4 + 8;
  nRF24L01.config();
}


void loop()
{
  if (nRF24L01.dataReady())
  {
    nRF24L01.getData((byte *)&value);
    Serial.print("Got data: ");
    Serial.println(value);
  }
}

