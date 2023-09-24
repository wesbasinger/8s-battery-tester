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
uint16_t val[4];

uint32_t last = 0, now = 0;

float CELL_VOLTAGE = 3.0;
float CELL_TOLERANCE = 0.05;
int R1 = 10000;
int R2 = 1500;

int ratio = R2 / (R1 + R2);


bool results[16];
int resultIdx = 0;

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
        results[resultIdx] = true;
      } else 
      {
        results[resultIdx] = false;
      }
      resultsIdx ++;
    }
  }
  // check voltage divider circuits, 2nd two ADC modules
  for (int i=2; i<4; i++)
  {
    for (int
  }
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

  if (result >= 0.95*CELL_VOLTAGE && result <= 1.05*CELL_VOLTAGE)
  {
    Serial.println("Passed");
    return true
  } else 
  {
    Serial.println("Failed");
    return false;
  }
}

void ADS_request_all()
{
  //  Serial.println(__FUNCTION__);
  for (int i = 0; i < 4; i++)
  {
    if (ADS[i].isConnected()) ADS[i].requestADC(0);
    delayMicroseconds(200);  // get them evenly spaced in time ...
  }
}


bool ADS_read_all()
{
  //  Serial.println(__FUNCTION__);
  for (int i = 0; i < 4; i++)
  {
    if (ADS[i].isConnected() && ADS[i].isBusy()) return true;
  }
  //  Serial.print("IDX:\t");
  //  Serial.println(idx);
  for (int i = 0; i < 4; i++)
  {
    if (ADS[i].isConnected())
    {
      val[i] = ADS[i].getValue();
    }
  }
  ADS_request_all();
  return false;
}


void ADS_print_all()
{
  //  Serial.println(__FUNCTION__);
  //  print duration since last print.
  now = millis();
  Serial.print(now - last);
  last = now;
  Serial.println();

  //  PRINT ALL VALUES
  for (int i = 0; i < 4; i++)
  {
    Serial.print(val[i]);
    Serial.print("\t");
  }
  Serial.println();
}


// -- END OF FILE --
