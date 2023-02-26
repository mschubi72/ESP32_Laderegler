#include <dpm8624.h>

using namespace std;

DPM8624::DPM8624(u_int initialVoltage_mV, u_int initialCurrent_mA)
{
    setVoltageCurrent(initialVoltage_mV, initialCurrent_mA);
}

void DPM8624::setCurrent(u_int newCurrent_mA)
{
}

void DPM8624::setVoltage(u_int newVoltage_mV)
{
    ////Serial2.print(":01w10=1234,\r\n");
    if (newVoltage_mV > 60000)
        return; // out of spec

    s_cmd = CMD_SET_VOLTAGE + String(newVoltage_mV) + ",\r\n";
    
    Serial2.print(s_cmd);
    delay(50);
}

void DPM8624::setVoltageCurrent(u_int newCurrent_mA, u_int newVoltage_mV)
{
    if (newCurrent_mA > 24000 || newVoltage_mV > 60000)
        return;

    s_cmd = CMD_SET_VOLTAGE_AND_CURRENT + String(newVoltage_mV) + "," + String(newCurrent_mA) + ",\r\n";

    Serial2.print(s_cmd);
    delay(50);
}

void DPM8624::enableOutput()
{
}

void DPM8624::disableOutput()
{
}

// return the Current settings - hopeful same as set :)
u_int DPM8624::getCurrentLimit()
{
    u_int currentlimit = 0;

    return currentlimit;
}

// return the Voltage settings - hopeful same as set :)
u_int DPM8624::getVoltageLimit()
{
    u_int voltagelimit = 0;

    return voltagelimit;
}

// return the actual Current
u_int DPM8624::getCurrentOutput()
{
    u_int currentout = 0;

    return currentout;
}

// return the actual Voltage
u_int DPM8624::getVoltageOutput()
{
    u_int voltageout = 0;

    return voltageout;
}

// return the actual Output Power in mW
u_int DPM8624::getPower()
{
    u_int powerout = 0;

    return powerout;
}

bool DPM8624::isOutputEnabled()
{
    bool isenabled = true;

    return isenabled;
}

// returns if Output operates in Current or Voltage Mode
bool DPM8624::isCVMode()
{
    bool iscv = true;

    return iscv;
}
