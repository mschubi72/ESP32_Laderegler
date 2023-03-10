#ifndef CONFIGURATION_h
#define CONFIGURATION_h

#include <stdlib.h>
#include <stdint.h>

// There are not many reasons to change this
#define MONITOR_BAUDRATE 115200

// WiFi Connection information
#define WIFI_SSID "MS01"
#define WIFI_PASSWORD "yxcvbnmasdfgh"
#define WIFI_HOSTNAME "esp-lader"

// OTA setup
const uint16_t OTA_PORT = 3232;
//const char *OTA_PASSWORD = "password";

// MQTT Server information
#define MQTT_HOST "192.168.1.6"
#define MQTT_USERNAME "mqtt"
#define MQTT_PASSWORD "mqtt"
#define MQTT_PORT 1883

// MQTT topics used by this program
#define MQTT_TOPIC_SENSOR "tele/SGMDD/SENSOR"
#define MQTT_TOPIC_SOLAR "solarpower/sensor/solarpower_leistung/state"
///sensor/solarpower_leistung"
//solarpower/sensor/solarpower_leistung/state

//#define MQTT_TOPIC_COMMAND reporting status
#define MQTT_TOPIC_AVAILABLE "esp32lader/status"




// Port for RS485 Communication to DPM8624
#define RS485_RXD2 16
#define RS485_TXD2 17

// I2C Pins for OLED Display
#define PIN_OLED_SCL 4
#define PIN_OLED_SDA 0

#define u8g_logo_width 12
#define u8g_logo_height 12

static unsigned char image_bits1[] = {
 0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0xfe,
 0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0xfe,0xc0,0x03,0x0e,0xfe,
 0xe0,0x00,0x1c,0xfe,0x20,0x20,0x18,0xfe,0x00,0x20,0x10,0xfe,
 0x00,0x20,0x00,0xfe,0x00,0x70,0x00,0xfe,0x00,0x70,0x00,0xfe,
 0x04,0xf8,0x80,0xfe,0x04,0x00,0x80,0xfe,0x0c,0x00,0xc0,0xfe,
 0x78,0x00,0x78,0xfe,0xf0,0x01,0x3e,0xfe,0xc0,0xff,0x0f,0xfe,
 0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0xfe,
 0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0xfe,
 0x00,0x00,0x00,0xfe};

static unsigned char image_bits2[] = {
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xfc,0xf1,0xff,
 0x1f,0xff,0xe3,0xff,0xdf,0xdf,0xe7,0xff,0xff,0xdf,0xef,0xff,
 0xff,0xdf,0xff,0xff,0xff,0x8f,0xff,0xff,0xff,0x8f,0xff,0xff,
 0xfb,0x07,0x7f,0xff,0xfb,0xff,0x7f,0xff,0xf3,0xff,0x3f,0xff,
 0x87,0xff,0x87,0xff,0x0f,0xfe,0xc1,0xff,0x3f,0x00,0xf0,0xff,
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 0xff,0xff,0xff,0xff};

static unsigned char image_bits3[] = {
 0x00,0xf0,0x00,0xf0,0x0c,0xf6,0x0c,0xf6,0x40,0xf0,0x40,0xf0,
 0x40,0xf0,0xc0,0xf0,0x06,0xf4,0x1c,0xf7,0x10,0xf1,0xe0,0xf0};

static unsigned char image_bits4[] = {
 0xff,0xff,0xff,0xff,0xf3,0xf9,0xf3,0xf9,0xbf,0xff,0xbf,0xff,
 0xbf,0xff,0x3f,0xff,0xf9,0xfb,0xe3,0xf8,0xef,0xfe,0x1f,0xff};


#endif