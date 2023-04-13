#include <dpm8624.h>

using namespace std;

const char *Dpm8624::CMD_SET_VOLTAGE = ":01w10=";
const char *Dpm8624::CMD_SET_VOLTAGE_AND_CURRENT = ":01w20=";
const char *Dpm8624::CMD_ENABLE_OUTPUT = ":01w12=1";
const char *Dpm8624::CMD_DISABLE_OUTPUT = ":01w12=0";
const char *Dpm8624::CMD_GET_CURRENT_VOLTAGE = "";
const char *Dpm8624::CMD_GET_VOLTAGE_PROG = ":01r10=0";
const char *Dpm8624::CMD_GET_CURRENT_PROG = ":01r11=0";
const char *Dpm8624::CMD_GET_VOLTAGE_OUT = ":01r30=0";
const char *Dpm8624::CMD_GET_CURRENT_OUT = ":01r31=0";
const char *Dpm8624::CMD_GET_CC_CV_MODE = "01r32=0";
const char *Dpm8624::CMD_SET_CURRENT = ":01w11=";
const char *Dpm8624::CMD_GET_OUTPUT_ENABLED = ":01r12=0";

Dpm8624::Dpm8624(State *state)
{
    status = state;
}

void Dpm8624::setupDPM(u_int initialVoltage_mV, u_int initialCurrent_mA)
{
    setVoltageCurrent(initialVoltage_mV, initialCurrent_mA);
    Serial.println("DPM8624 initialized...");
    Serial.printf("\tdefault Voltage: %imV, default Current: %imA \n", initialVoltage_mV, initialCurrent_mA);
}


void Dpm8624::setCurrent(u_int newCurrent_mA)
{
        ////Serial2.print(":01w10=1234,\r\n");
    if (newCurrent_mA > 24000)
        return; // out of spec

    s_cmd = CMD_SET_CURRENT + String(newCurrent_mA) + ",\r\n";

    Serial2.print(s_cmd);
    delay(50);
}

void Dpm8624::setVoltage(u_int newVoltage_mV)
{
    ////Serial2.print(":01w10=1234,\r\n");
    if (newVoltage_mV > 60000)
        return; // out of spec

    s_cmd = CMD_SET_VOLTAGE + String(newVoltage_mV/10) + ",\r\n";

    Serial2.print(s_cmd);
    Serial.println(s_cmd);
    delay(50);
}

void Dpm8624::setVoltageCurrent(u_int newVoltage_mV, u_int newCurrent_mA)
{
    if (newCurrent_mA > 24000 || newVoltage_mV > 60000){
            Serial.printf("too much %d, %d",newCurrent_mA, newVoltage_mV);
        return;
    }

    s_cmd = CMD_SET_VOLTAGE_AND_CURRENT + String(newVoltage_mV/10) + "," + String(newCurrent_mA) + ",\r\n";

    Serial2.print(s_cmd);
    Serial.println(s_cmd);
    delay(50);
}

void Dpm8624::enableOutput()
{
    s_cmd = CMD_ENABLE_OUTPUT + String("") + ",\r\n";

    Serial2.print(s_cmd);
    delay(50);

}

void Dpm8624::disableOutput()
{
    s_cmd = CMD_DISABLE_OUTPUT + String("") + ",\r\n";

    Serial2.print(s_cmd);
    delay(50);

}

// return the Current settings - hopeful same as set :)
u_int Dpm8624::getCurrentLimit()
{
    s_cmd = CMD_GET_CURRENT_PROG + String("") + ",\r\n";

    Serial2.print(s_cmd);
    Serial.println(s_cmd);
    delay(50);
    String str = Serial2.readString();
    Serial.print("GET CURRENT: ");
    Serial.println(str);
    delay(50);
    u_int currentlimit = 0;

    return currentlimit;
}

// return the Voltage settings - hopeful same as set :)
u_int Dpm8624::getVoltageLimit()
{
    u_int voltagelimit = 0;

    return voltagelimit;
}

// return the actual Current
u_int Dpm8624::getCurrentOutput()
{
    u_int currentout = 0;

    return currentout;
}

// return the actual Voltage
u_int Dpm8624::getVoltageOutput()
{
    u_int voltageout = 0;

    return voltageout;
}

// return the actual Output Power in mW
u_int Dpm8624::getPower()
{
    u_int powerout = 0;

    return powerout;
}

bool Dpm8624::isOutputEnabled()
{
    bool isenabled = true;

    return isenabled;
}

// returns if Output operates in Current or Voltage Mode
bool Dpm8624::isCVMode()
{
    bool iscv = true;

    return iscv;
}
