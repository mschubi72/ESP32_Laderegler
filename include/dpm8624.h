#ifndef DPM8624_H
#define DPM8624_H

#include "main.h"

// class uses Serial2 for dpm8624 communication
// Serial2 must be initialized before using this class
// implementation is quick&dirty and my by not error proven
// all settings has to be check via getMethods...
// setMethods set the Values directly via RS485
// commands wait 50ms after execution
class Dpm8624
{
private:
    static const char* CMD_SET_VOLTAGE;
    static const char* CMD_SET_CURRENT;
    static const char* CMD_SET_VOLTAGE_AND_CURRENT;
    static const char* CMD_ENABLE_OUTPUT;

    static const char* CMD_GET_CURRENT_VOLTAGE;
    static const char* CMD_GET_VOLTAGE_PROG;
    static const char* CMD_GET_CURRENT_PROG;
    static const char* CMD_GET_VOLTAGE_OUT;
    static const char* CMD_GET_CURRENT_OUT;
    static const char* CMD_GET_CC_CV_MODE;
    static const char* CMD_GET_OUTPUT_ENABLED;

    State *status;
    String s_cmd;
    u_int i_current = 0;
    u_int i_voltage = 0;

public:
    Dpm8624(State *state);

    // max. 60V, 24A, Settings in mV an mA)
    // are values out of specs, nothing happens
    void setupDPM(u_int initialVoltage_mV, u_int initialCurrent_mA);
    void setCurrent(u_int newCurrent_mA);
    void setVoltage(u_int newVoltage_mV);
    void setVoltageCurrent(u_int newCurrent_mA, u_int newVoltage_mV);

    //enabled Output, but also use new settings
    void enableOutput();
    void disableOutput();
    // return the Current settings - hopeful same as set :)
    u_int getCurrentLimit();
    // return the Voltage settings - hopeful same as set :)
    u_int getVoltageLimit();
    // return the actual Current
    u_int getCurrentOutput();
    // return the actual Voltage
    u_int getVoltageOutput();
    // return the actual Output Power in mW
    u_int getPower();

    bool isOutputEnabled();
    // returns if Output operates in Current or Voltage Mode
    bool isCVMode();
};

#endif
