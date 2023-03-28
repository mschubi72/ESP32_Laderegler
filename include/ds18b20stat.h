#ifndef DS18B20STAT_H
#define DS18B20STAT_H

#include "main.h"

class DS18B20Stat
{
private:
    State *status; 
    static const int sensorcount = 3;  
    static const int BatteryLeftIdx = 0;
    static const int BatteryRightIdx = 1;
    static const int InverterIdx = 2;

public:
    // State Obejct
    DS18B20Stat(State* state);

//init DS18B20Stat - schould be late in setup()
void setupSensors();

//refresh temperature stats
void updateTemperature();

//print serial the HW address of OneWire Sensors
void printHWaddresses();
};

#endif