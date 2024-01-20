//
//    FILE: ADS_read.ino
//  AUTHOR: Rob.Tillaart
// PURPOSE: read analog inputs - straightforward.
//     URL: https://github.com/RobTillaart/ADS1X15

#include "ADS1X15.h"

ADS1115 ALPHA_ADS(0x48);
ADS1115 BRAVO_ADS(0x49);

int16_t analogValues[8];
float converted[8];

float cellVoltage = 3.3;

void setup() 
{
  Serial.begin(9600);
  Serial.println(__FILE__);
  Serial.print("ADS1X15_LIB_VERSION: ");
  Serial.println(ADS1X15_LIB_VERSION);

  Wire.begin();
  ALPHA_ADS.begin();
  BRAVO_ADS.begin();
}


void loop() 
{
  ALPHA_ADS.setGain(0);
  BRAVO_ADS.setGain(0);

  int masterIdx = 0;

  for(int i=0; i<4; i++)
  {
    analogValues[masterIdx] = ALPHA_ADS.readADC(i);
    masterIdx ++;
  }

  
  for(int i=0; i<4; i++)
  {
    analogValues[masterIdx] = BRAVO_ADS.readADC(i);
    masterIdx ++;
  }

  float f = ALPHA_ADS.toVoltage(1);  //  voltage factor

  Serial.println("RAW VALUES");

  for(int i=0; i<8; i++)
  {
    Serial.println(analogValues[i]);
  }
  delay(2000);

  Serial.println();

  Serial.println("CONVERTED VALUES");

  for(int i=0; i<8; i++)
  {
    float v = analogValues[i]*f;
    float _converted = (23*v)/3;
    converted[i] = _converted;
    Serial.println(_converted);
  }
  delay(2000);

  Serial.println();

  for(int i=0; i<8; i++)
  {
    float targetVoltage = (i+1)*cellVoltage;
    if(converted[i] < 1.06*targetVoltage && converted[i] > 0.94*targetVoltage)
    {
      Serial.print("Cell ");
      Serial.print(i+1);
      Serial.print(" in tolerance\n");
    } 
    else
    {
      Serial.println("VOLTAGE CHECK FAILED!!!");
    }
  }
  Serial.println("HIT NANO RESET BUTTON TO REDO TEST");
  Serial.println("RESETS AUTOMATICALLY IN 20 SECONDS");
  delay(20000);
  
}
