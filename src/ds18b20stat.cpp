#include <ds18b20stat.h>
#include <OneWire.h>
#include <DallasTemperature.h>

using namespace std;

OneWire oneWire(SENSOR_DATA_PIN);
DallasTemperature oneWireSensors(&oneWire);

DS18B20Stat::DS18B20Stat(State *state)
{
    status = state;
}

void DS18B20Stat::setupSensors()
{
    oneWireSensors.begin();
}

void DS18B20Stat::updateTemperature()
{
    oneWireSensors.requestTemperatures();
    status->tempBat1 = oneWireSensors.getTempCByIndex(BatteryLeftIdx);
    status->tempBat2 = oneWireSensors.getTempCByIndex(BatteryRightIdx);
    status->tempInverter = oneWireSensors.getTempCByIndex(InverterIdx);
    Serial.println(" ");
    Serial.print("S0: ");
    Serial.print(status->tempBat1);
    Serial.println(" ºC");
    Serial.print("S1: ");
    Serial.print(status->tempBat2);
    Serial.println(" ºC");
    Serial.print("S2: ");
    Serial.print(status->tempInverter);
    Serial.println(" ºC");
}

void DS18B20Stat::printHWaddresses()
{
    // DeviceAddress sensor1 = { 0x28, 0xFF, 0x48, 0xC8, 0x89, 0x16, 0x3, 0xF2 };
    byte i;
    byte addr[8];

    while (true)
    {
        if (!oneWire.search(addr))
        {
            Serial.println(" No more addresses.");
            Serial.println();
            oneWire.reset_search();
            delay(250);
            break;
        }
        Serial.print(" ROM =");
        for (i = 0; i < 8; i++)
        {
            Serial.write(' ');
            Serial.print(addr[i], HEX);
        }
    }
}
