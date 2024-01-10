#ifndef CONFIGURATION_h
#define CONFIGURATION_h

#include <stdlib.h>
#include <stdint.h>
#include "secconfig.h"

// There are not many reasons to change this
#define MONITOR_BAUDRATE 115200

// #define DEBUG //"Schalter" zum aktivieren
/*
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif
*/

// Examle WiFi Connection information
// #define WIFI_SSID "My-SID"
// #define WIFI_PASSWORD "My_PW"
// #define WIFI_HOSTNAME "my-hostname"

// OTA setup
const uint16_t OTA_PORT = 3232;
// const char *OTA_PASSWORD = "password";

// MQTT Example Server information
// #define MQTT_HOST "192.168.10.100"
// #define MQTT_USERNAME "mqtt"
// #define MQTT_PASSWORD "mqttpw"
// #define MQTT_PORT 1883

// MQTT topics used by this program
#define MQTT_TOPIC_SENSOR "tele/SGMDD-MS/SENSOR"
#define MQTT_TOPIC_SOLAR "solarpower/sensor/solarpower_leistung/state"
#define MQTT_TOPIC_POWER_IN "lader-power/sensor/lader-power_power_in/state"
#define MQTT_TOPIC_POWER_OUT "lader-power/sensor/lader-power_power_out/state"
#define MQTT_TOPIC_RELAY_IN "lader-power/switch/lader-power_relay_in/state"
#define MQTT_TOPIC_RELAY_OUT "lader-power/switch/lader-power_relay_out/state"

/// sensor/solarpower_leistung"
// solarpower/sensor/solarpower_leistung/state

// #define MQTT_TOPIC_COMMAND reporting status
#define MQTT_TOPIC_AVAILABLE "esp32lader/available"
#define MQTT_TOPIC_STATUS "esp32lader/state"

#define DEFAULT_BAT_VOLTAGE 29.2
#define DEFAULT_BAT_CURRENT 0.1
#define DEFAULT_POWER_THRESHOLD 20
// #define BAT_EMPTY 25.6
#define BAT_EMPTY 24.2
#define BAT_20 24.8 //25.8
#define BAT_30 26.0
#define BAT_40 26.2
#define BAT_70 26.4
#define BAT_FULL 27.0

// Port for RS485 Communication to DPM8624
#define RS485_RXD2 16
#define RS485_TXD2 17

// Port for OneWire
#define SENSOR_DATA_PIN 14

// I2C Pins for OLED Display
#define PIN_OLED_SCL 22
#define PIN_OLED_SDA 21 // 0

// I2C Pins for ADS1115 Display
#define PIN_ADS_SCL 4
#define PIN_ADS_SDA 2

// intervals for actions in seconds
#define INTERVAL_ASK_TEMPERATURE 60
#define INTERVAL_ASK_BAT_VOLTAGE 10
#define INTERVAL_REFRESH_DISPLAY 2
#define INTERVAL_PROCESS_ACTION 10

#define NTP_POOL "de.pool.ntp.org"
// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
// Europe/Berlin
#define MY_TZ "CET-1CEST,M3.5.0,M10.5.0/3"
#define MORNING 530
#define EVENING 2100

#define u8g_logo_width 32
#define u8g_logo_height 16

static unsigned char image_bits1[] = {
    0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0xfe,
    0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0xfe, 0xc0, 0x03, 0x0e, 0xfe,
    0xe0, 0x00, 0x1c, 0xfe, 0x20, 0x20, 0x18, 0xfe, 0x00, 0x20, 0x10, 0xfe,
    0x00, 0x20, 0x00, 0xfe, 0x00, 0x70, 0x00, 0xfe, 0x00, 0x70, 0x00, 0xfe,
    0x04, 0xf8, 0x80, 0xfe, 0x04, 0x00, 0x80, 0xfe, 0x0c, 0x00, 0xc0, 0xfe,
    0x78, 0x00, 0x78, 0xfe, 0xf0, 0x01, 0x3e, 0xfe, 0xc0, 0xff, 0x0f, 0xfe,
    0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0xfe,
    0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0xfe,
    0x00, 0x00, 0x00, 0xfe};

static unsigned char image_bits2[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xfc, 0xf1, 0xff,
    0x1f, 0xff, 0xe3, 0xff, 0xdf, 0xdf, 0xe7, 0xff, 0xff, 0xdf, 0xef, 0xff,
    0xff, 0xdf, 0xff, 0xff, 0xff, 0x8f, 0xff, 0xff, 0xff, 0x8f, 0xff, 0xff,
    0xfb, 0x07, 0x7f, 0xff, 0xfb, 0xff, 0x7f, 0xff, 0xf3, 0xff, 0x3f, 0xff,
    0x87, 0xff, 0x87, 0xff, 0x0f, 0xfe, 0xc1, 0xff, 0x3f, 0x00, 0xf0, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff};

static unsigned char image_bits3[] = {
    0x00, 0xf0, 0x00, 0xf0, 0x0c, 0xf6, 0x0c, 0xf6, 0x40, 0xf0, 0x40, 0xf0,
    0x40, 0xf0, 0xc0, 0xf0, 0x06, 0xf4, 0x1c, 0xf7, 0x10, 0xf1, 0xe0, 0xf0};

static unsigned char image_bits4[] = {
    0xff, 0xff, 0xff, 0xff, 0xf3, 0xf9, 0xf3, 0xf9, 0xbf, 0xff, 0xbf, 0xff,
    0xbf, 0xff, 0x3f, 0xff, 0xf9, 0xfb, 0xe3, 0xf8, 0xef, 0xfe, 0x1f, 0xff};

#endif