#ifndef ADS1115_H
#define ADS1115_H

#include "main.h"

class Ads1115
{
private:
    State *status;
    float batVoltage = 0.0;
    float batVoltageSum = 0.0;

public:
    Ads1115(State *state);

    // init ADS
    void setupADS();

    uint8_t getCapacityPercent(float voltage);
    
    // get the battery voltage and store in status
    void updateVoltage();
};

#endif