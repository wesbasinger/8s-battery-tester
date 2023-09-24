//
//    FILE: ADS_1114_four.ino
//  AUTHOR: Rob.Tillaart
// PURPOSE: demo reading four ADS1114 modules in parallel
//     URL: https://github.com/RobTillaart/ADS1X15


// Note all IO with the sensors are guarded by an isConnected()
// this is max robust, in non critical application one may either
// cache the value or only verify it in setup (least robust).
// Less robust may cause the application to hang - watchdog reset ?


#include "ADS1X15.h"


ADS1114 ADS[4];

float CELL_VOLTAGE = 3.0;
float CELL_TOLERANCE = 0.05;
int R1 = 10000;
int R2 = 1500;

int ratio = R2 / (R1 + R2);


bool results[16];
int resultsIdx = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("ADS1X15_LIB_VERSION: ");
  Serial.println(ADS1X15_LIB_VERSION);

  for (uint8_t i = 0; i < 4; i++)
  {
    uint8_t address = 0x48 + i;
    ADS[i] = ADS1114(address);

    Serial.print(address, HEX);
    Serial.print("  ");
    Serial.println(ADS[i].begin() ? "connected" : "not connected");

    ADS[i].setDataRate(4);        // 0 = slow   4 = medium   7 = fast, but more noise

    // relay pins
    for (int i=2; i<10; i++)
    { 
      pinMode(i, OUTPUT);
    }
    
  }
}


void loop()
{
  // read individual cell voltages using the first two ADC modules
  for (int i=0; i<2; i++)
  {
    // now read from each ADC
    for (int j=0; j<4; j++)
    {
      // relay pin started at 2, adc could be 0 or 1
      int relayPin = i*4 + j + 2;
      bool goodVoltage = checkCellVoltage(relayPin, i, j);
      if (goodVoltage) 
      {
        results[resultsIdx] = true;
      } else 
      {
        results[resultsIdx] = false;
      }
      resultsIdx ++;
    }
  }
  // check voltage divider circuits, 2nd two ADC modules
  // actual test specifies to start with measuring 1 cell, and then ensuring that each successive cell adds a specified amount of voltage
  int numCells = 1;
  for (int i=2; i<4; i++)
  {
    for (int j=0; i<4; j++)
    {
       float result = ADS[i].readADC(j)*(5/1023);
       float sourceVoltage = convertDividedVoltage(result);
       if(sourceVoltage*numCells >  (1-CELL_TOLERANCE) * CELL_VOLTAGE * numCells && sourceVoltage*numCells < (1+CELL_TOLERANCE)*CELL_VOLTAGE * numCells)
       {
        results[resultsIdx] = true;
       } else 
       {
        results[resultsIdx] = false;
       }
       resultsIdx ++;
       numCells ++;
    }
  }
  for (int i=0; i<8; i++)
  {
    if(!results[i])
    {
      Serial.print("Result idx ");
      Serial.print(i);
      Serial.print(" failed\n");
      Serial.println("FAILED");
      delay(20000);
      break;
    }
  }
  Serial.println("PASSED");
  delay(20000);
}

float convertDividedVoltage(float dividedVoltage)
{
  return (dividedVoltage * (R1 + R2)) / R2;
}

bool checkCellVoltage(int relayPin, int adsMaster, int adsPin)
{
  Serial.print("Checking battery ");
  Serial.print(relayPin - 1);
  Serial.print("\n");

  digitalWrite(relayPin, HIGH);
  delay(2000);

  float result = ADS[adsMaster].readADC(adsPin)*(5/1023);

  Serial.print("Read cell voltage as: ");
  Serial.print(result);
  Serial.print("\n");

  digitalWrite(relayPin, LOW);
  delay(1000);

  if (result >= (1-CELL_TOLERANCE)*CELL_VOLTAGE && result <= (1+CELL_TOLERANCE)*CELL_VOLTAGE)
  {
    Serial.println("Passed");
    return true;
  } else 
  {
    Serial.println("Failed");
    return false;
  }
}
