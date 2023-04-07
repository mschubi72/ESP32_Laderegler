#ifndef MAIN_h
#define MAIN_h

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <stdint.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <time.h>

#include "configuration.h"
#include "version.h"
#include "ota.h"
#include "rs485.h"

#include <map>
#include <string>

using namespace std;

#define CMD_ON "ON"
#define CMD_DEAD "offline"
#define CMD_ALIVE "online"

// Connection starters
void connectToWiFi();
void connectToMQTT();

// Event handlers
void onWiFiEvent(WiFiEvent_t event);
void onMQTTConnect(bool sessionPresent);
void onMQTTDisconnect(AsyncMqttClientDisconnectReason reason);
void onMQTTMessage(
    char *topic,
    char *payload,
    AsyncMqttClientMessageProperties properties,
    size_t len,
    size_t index,
    size_t total);

typedef struct
{
    float batVoltage = 0.0;      // LiPo Voltage Volt
    uint8_t batStatus = 0;       // Status empty to full in 6 steps
    uint16_t solarPower = 0;     // Solar power feed Watt
    int16_t flatPower = 0;       // Power meter in/out Watt. if < 0 then solar power feed is larger than power usage
    uint16_t chargePowerRaw = 0; // Used power for charging Watt. Primary with all losses
    uint16_t chargePower = 1;    // Used power for charging Watt. Volt*Ampere on charger
    uint16_t feedPowerBat = 0;   // feed power of battery
    float tempBat1 = 0.0;        // Bat case 1 temperature
    float tempBat2 = 0.0;        // Bat case 2 temperature
    float tempInverter = 0.0;    // Inverter temperature
    bool processed = false;      // set false if value of flat power consumtion is set. So async processes is informed if there someting to do...
    bool relay_in = false;       
    bool relay_out = false;
    char *formattedTime;
} State;

// Helpers
void announceToHomeAssistant();
void processStateJson(char *topic, char *payload);
void debugState(State *state);

void doAction(); // do the Job of controlling

void connectToWiFi();
void onWiFiEvent(WiFiEvent_t event);

// Main program functions
void setup();
void loop();

#endif