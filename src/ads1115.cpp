#include "ads1115.h"
#include <ADS1X15.h>

using namespace std;

ADS1115 ads;

Ads1115::Ads1115(State *state)
{
    status = state;
}

void Ads1115::setupADS()
{
    Wire.setPins(PIN_ADS_SDA, PIN_ADS_SCL);
    Wire.begin();

    ads = ADS1115(0x48, &Wire);
    ads.begin();
    ads.setGain(1);     // 6.144 volt
    ads.setDataRate(0); // medium
    // single shot mode
    ads.setMode(1);
    Serial.println("ADS1115 initialized...");
    Serial.printf("\tSDA Pin: %i, SCL Pin: %i \n", PIN_ADS_SDA, PIN_ADS_SCL);
}

uint8_t Ads1115::getCapacityPercent(float voltage)
{
  if (voltage < 19.0)
    return 0;
  if (voltage > 19.0 && voltage < 21.6)
    return 0;
  if (voltage > 21.6 && voltage < 25.6)
    return 1;
  if (voltage > 25.6 && voltage < 25.8)
    return 10;
  if (voltage > 25.8 && voltage < 26.0)
    return 20;
  if (voltage > 26.0 && voltage < 26.2)
    return 30;
  if (voltage > 26.2 && voltage < 26.4)
    return 40;
  if (voltage > 26.4 && voltage < 26.6)
    return 70;
  if (voltage > 26.6 && voltage < 26.8)
    return 90;
  if (voltage > 26.8 && voltage < 27.0)
    return 99;
  if (voltage > 20.0 && voltage < 30)
    return 100;
  if (voltage > 30.0)
    return 200;
  return 254;
}

void Ads1115::updateVoltage()
{
    float val_01 = ads.readADC(1);
    batVoltage = ads.toVoltage(val_01);
    /*
    Serial.print("\n\t ADC1115-1: ");
    Serial.print(batVoltage);
    Serial.print(" V; corrected: ");
    Serial.println(batVoltage*14.0);
*/
    batVoltageSum = batVoltage;
    val_01 = ads.readADC(0);
    batVoltage = ads.toVoltage(val_01);
    /*
      Serial.print("\t ADC1115-0: ");
      Serial.print(batVoltage);
      Serial.print(" V; corrected: ");
      Serial.println(batVoltage*14.0);
      */
    batVoltageSum += batVoltage;

    int tempBat = 100 * (batVoltageSum * 14.0 / 2.0);
    if (tempBat > 0)
    { // sometimes Error on reading voltage
        status->batVoltage = tempBat / 100.0;
        // status->batVoltage = roundf(100.0f * status->batVoltage)/100.0f;
        status->batPercent = getCapacityPercent(status->batVoltage);
  
        if (status->batVoltage > BAT_FULL)
        {
            status->batStatus = 5;
        }
        else if (status->batVoltage > BAT_70)
        {
            status->batStatus = 4;
        }
        else if (status->batVoltage > BAT_40)
        {
            status->batStatus = 3;
        }
        else if (status->batVoltage > BAT_20)
        {
            status->batStatus = 2;
        }
        else if (status->batVoltage > BAT_EMPTY)
        {
            status->batStatus = 1;
        }
        else
        {
            status->batStatus = 0;
            Serial.printf("->BatStatus in Case: %i mit %fV\n", status->batStatus, status->batVoltage);
        }
    }
}
