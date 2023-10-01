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


ADS1114 ADS[2];

float CELL_VOLTAGE = 3.3;
float CELL_TOLERANCE = 0.05;
int R1 = 10000;
int R2 = 1500;

int ratio = R2 / (R1 + R2);


bool results[8];
int resultsIdx = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("ADS1X15_LIB_VERSION: ");
  Serial.println(ADS1X15_LIB_VERSION);

  for (uint8_t i = 0; i < 2; i++)
  {
    uint8_t address = 0x48 + i;
    ADS[i] = ADS1114(address);

    Serial.print(address, HEX);
    Serial.print("  ");
    Serial.println(ADS[i].begin() ? "connected" : "not connected");

    ADS[i].setDataRate(4);        // 0 = slow   4 = medium   7 = fast, but more noise
    
  }
}


void loop()
{

  Serial.println("press t to start the test");
  delay(2000);

  if(Serial.available()>0) {
    runTest();
  }

}

void runTest()
{

  Serial.println("Started test");
  // check voltage divider circuits, 2nd two ADC modules
  // actual test specifies to start with measuring 1 cell, and then ensuring that each successive cell adds a specified amount of voltage
  int numCells = 1;
  for (int i=0; i<2; i++)
  {
    Serial.print("Reading ADC ");
    Serial.print(i+1);
    Serial.print("\n");
    delay(500);
    for (int j=0; j<4; j++)
    {
      Serial.print("Reading channel ");
      Serial.print(j+1);
      Serial.print("\n");
      delay(500);
      int16_t result = ADS[i].readADC(j); // analog result
      Serial.print("Analog reads: ");
      Serial.print(result);
      Serial.print("\n");
      float factor = ADS[i].toVoltage(1);
      Serial.print("Divided Voltage: ");
      Serial.print(factor*result);
      Serial.print("\n");
      float sourceVoltage = convertDividedVoltage(factor*result);
      Serial.print("Source voltage: ");
      Serial.print(sourceVoltage);
      Serial.print("\n");
      delay(1000);
       if(sourceVoltage*numCells >  (1-CELL_TOLERANCE) * CELL_VOLTAGE * numCells && sourceVoltage*numCells < (1+CELL_TOLERANCE)*CELL_VOLTAGE * numCells)
       {
        results[resultsIdx] = true;
        Serial.println("Cell passed");
       } else 
       {
        results[resultsIdx] = false;
        Serial.println("Cell failed");
       }
       delay(1000);
       resultsIdx ++;
       numCells ++;
    }
  }

  Serial.println("Finished reading all cells");
  delay(1000);

  for (int i=0; i<8; i++)
  {
    if(!results[i])
    {
      Serial.print("Cell ");
      Serial.print(i+1);
      Serial.print(" failed\n");
      delay(1000);
    } else
    {
      Serial.print("Cell ");
      Serial.print(i+1);
      Serial.print('passed\n');
      delay(1000);
    }
  }

  Serial.println("Press the reset button on the Arduino to restart the test");
  delay(3600*1000);
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
