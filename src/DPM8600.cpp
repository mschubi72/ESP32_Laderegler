
#include <Arduino.h>
#include <DPM8600.h>

// DECLARATION
// ------------------------------
DPM8600::DPM8600(int8_t address, State *state)
{
    status = state;
    if ((address < 10) && (address >= 1))
    {
        _address = "0" + String(address);
    }
    else if ((address >= 10 && address < 100))
    {
        _address = String(address);
    }
    else
    {
        _address = "01";
    }
}

// PUBLIC
// ------------------------------
int DPM8600::begin(HardwareSerial &serial, int8_t maxRetry)
{
    _serial = &serial;
    _maxRetry = maxRetry;
    float maxCurrent = read('m');
    return (maxCurrent > 0) ? 1 : -1;
}

int DPM8600::power(bool on)
{
    int response = write('p', (on) ? 1 : 0);
    return response;
}

int DPM8600::writeVC(float v, float c)
{
    int8_t retry = 0;
    String response = "";
    bool completed = false;
    int _v = floor(v * 100);
    int _c = floor(c * 1000);

    if ((_v < 0) || (_v > 65535) || (_c < 0) || (_c > 65535))
    {
        return -16; // Value outside boundaries
    }

    do
    {
        // Clear response
        // response = "";

        // Send command
        _serial->println(":" + _address + "w20=" + String(_v) + "," + String(_c) + ",");

        // Listen for reply
        completed = listen(response);

        // Add a retry
        retry += 1;

    } while ((!completed) && (retry < _maxRetry));

    return (completed) ? 1 : -24;
}

int DPM8600::write(char cmd, float value)
{

    int x = 0;
    String command = "";
    // x100 multiplication for voltage and x1000 for current because of the resolution
    switch (cmd)
    {
    case 'c':
    case 'C':
        x = floor(value * 1000);
        command = "11";
        break;
    case 'v':
    case 'V':
        x = floor(value * 100);
        command = "10";
        break;
    default:
        if (value != 0 && value != 1)
        {
            return -20;
        }
        x = floor(value);
        command = "12";
        // Default is power. In case of a mistake, error will be triggered.
    }
    if (x < 0 || x > 65535)
    {
        return -16; // Value outside boundaries
    }

    int8_t retry = 0;
    String response = "";
    bool completed = false;

    do
    {
        // Clear response
        // response = "";

        // Send command
        _serial->println(":" + _address + "w" + command + "=" + String(x) + ",");

        // Listen for reply
        completed = listen(response);

        // Add a retry
        retry += 1;

    } while ((!completed) && (retry < _maxRetry));

    if (!completed)
    {
        switch (cmd)
        {
        case 'c':
        case 'C':
            return -21; // Setting current error
        case 'p':
        case 'P':
            return -22; // Setting power error
        default:
            return -23; // Setting voltage error
        }
    }
    else
    {
        return 1; // 1 is success for functions that should not return other datatypes
    }
}

float DPM8600::read(char cmd)
{

    int8_t retry = 0;
    bool completed = false;
    String response = "";
    String command = "";

    // Getting the correct command number
    switch (cmd)
    {
    case 'v':
    case 'V':
        command = "30";
        break; // Voltage
    case 'c':
    case 'C':
        command = "31";
        break; // Current
    case 'p':
    case 'P':
        command = "12";
        break; // Power
    case 's':
    case 'S':
        command = "32";
        break; // CC/CV
    case 'm':
    case 'M':
        command = "01";
        break; // Max Current
    default:
        command = "33";
        break; // Internal temperature
    }

    do
    {
        // Clear response
        // response = "";

        // Send a command
        _serial->println(":" + _address + "r" + command + "=0,");

        // Listen for a reply
        completed = listen(response);

        // Add a retry
        retry += 1;

    } while ((!completed) && (retry < _maxRetry)); // Stop running either when data received or max attempts happened.

    if (!completed)
    {
        switch (cmd)
        {
        case 'v':
        case 'V':
            return -10; // Voltage reading error
        case 'c':
        case 'C':
            return -11; // Current reading error
        case 'p':
        case 'P':
            return -12; // Power reading error
        case 's':
        case 'S':
            return -13; // CC/CV reading error
        case 'm':
        case 'M':
            return -14; // Max current reading error
        default:
            return -15; // Internal temperature reading error
        }
    }

    float result = processString(response);

    switch (cmd)
    {
    case 'v':
    case 'V':
        return result / 100; // Voltage has 2 decimals
    case 'c':
    case 'C':
    case 'm':
    case 'M':
        return result / 1000; // Current has 3 decimals
    default:
        return result;
    }
}

void DPM8600::setupDPM(float initialVoltage_V, float initialCurrent_A)
{
    writeVC(initialVoltage_V, initialCurrent_A);
    Serial.println("DPM8624 initialized...");
    Serial.printf("\tdefault Voltage: %fV, default Current: %fA \n", initialVoltage_V, initialCurrent_A);
}

void DPM8600::updateStatus()
{
    float VA = 0.0;

    VA = read('t');
    if (VA == -15)
    { // failure
        status->tempDpm = 0.0;
    }
    else
    {
        status->tempDpm = VA;
    }

    int powerstat = read('p');
    if (powerstat < 1)
    {
        status->chargePower = 0;
        return;
    }

    float U = read('v');
    float A = read('c');
    if (U < 0. || A < 0.)
    { // failure
        status->chargePower = 0;
    }
    else
    {

        VA = U * A;
        status->chargePower = (uint16_t)VA;
    }
}

// PRIVATE
// ------------------------------
bool DPM8600::listen(String &response)
{
    unsigned long errorTimer = millis();
    while ((millis() - errorTimer) < 250)
    {
        if (_serial->available())
        {
            char inChar = (char)_serial->read();
            if (inChar == '\n')
            {
                // No need to implement response checking as no way to check whether the command was correct, only received in a correct format. But format is always correct.
                return true;
            }
            else
            {
                response += inChar;
            }
        }
    }
    return false;
}

float DPM8600::processString(String str)
{
    str.remove(0, 7);
    str.replace(".", "");
    return atof(str.c_str());
}
