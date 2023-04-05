#include "main.h"
#include "lcdstatus.h"
#include "ds18b20stat.h"
#include "ads1115.h"
#include "dpm8624.h"

#include <esp_task_wdt.h>

using namespace std;

#define WDT_TIMEOUT 5 // 5 seconds WDT

String phoneNumber = PHONENUMBER;
String apiKey = APIKEY;

State currentState;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
Ticker temperatureTimer;
Ticker voltageTimer;


WiFiUDP ntpUDP;
LcdStatus lcdstatus = NULL;
DS18B20Stat ds18b20status = NULL;
Ads1115 ads1115 = NULL;
Dpm8624 dpm8624 = NULL;

void sendMessage(String message)
{

  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  Serial.println("Message URL:");
  Serial.println(url);

  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200)
  {
    Serial.println("Message sent successfully");
  }
  else
  {
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

void toggleRelayPowerIn()
{

  // Data to send with HTTP POST
  String url = "http://192.168.1.87/switch/lader-power_relay_in/toggle";
  Serial.println("Message URL:");
  Serial.println(url);

  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200)
  {
    Serial.println("Message sent successfully");
  }
  else
  {
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

void onWiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_START:
    Serial.println("WIFI: Connecting...");
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    // Serial.println("WIFI: Connected! Waiting for IP...");
    break;
  case SYSTEM_EVENT_STA_LOST_IP:
    Serial.println("WIFI: Lost IP address...");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    // Serial.println("WIFI: Got IP!");
    Serial.print("WIFI: IP Address: ");
    Serial.println(WiFi.localIP());
    connectToMQTT();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("WIFI: Disconnected.");
    mqttReconnectTimer.detach();
    wifiReconnectTimer.once(2, connectToWiFi);
    break;
  default:
    break;
  }
}

void connectToWiFi()
{
  WiFi.mode(WIFI_STA);

  // FIX FOR USING 2.3.0 CORE (only .begin if not connected)
  if (WiFi.status() == WL_CONNECTED)
    return;

  // Serial.println("WIFI: Connecting to WiFi...");
  WiFi.setHostname(WIFI_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onMQTTConnect(bool sessionPresent)
{
  Serial.println("MQTT: Connected.");
  mqttClient.subscribe(MQTT_TOPIC_SENSOR, 0);
  mqttClient.subscribe(MQTT_TOPIC_SOLAR, 0);
  mqttClient.publish(MQTT_TOPIC_AVAILABLE, 0, true, CMD_ALIVE);
  announceToHomeAssistant();
  updateState();
}

void connectToMQTT()
{
  Serial.println("MQTT: Connecting to broker...");
  mqttClient.setClientId(WIFI_HOSTNAME);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
  mqttClient.setWill(MQTT_TOPIC_AVAILABLE, 0, true, CMD_DEAD);
  mqttClient.connect();
}

void onMQTTDisconnect(AsyncMqttClientDisconnectReason reason)
{
  Serial.println("MQTT: Disconnected.");
  if (WiFi.isConnected())
  {
    mqttReconnectTimer.once(10, connectToMQTT);
  }
}

void onMQTTMessage(
    char *topic, char *payload,
    AsyncMqttClientMessageProperties properties,
    size_t len, size_t index, size_t total)
{
  // Serial.println("MQTT: message received");
  // Serial.println(payload);
  processStateJson(topic, payload);
}

void processStateJson(char *topic, char *payload)
{
  StaticJsonDocument<512> jsonBuffer;
  DEBUG_PRINT("Topic: ");
  DEBUG_PRINTLN(topic);
  DEBUG_PRINT("Payload: ");
  DEBUG_PRINTLN(payload);

  // JsonObject& root = jsonBuffer.parseObject(payload);
  if (strcmp(topic, MQTT_TOPIC_SENSOR) == 0) // Powermeter
  {
    // Serial.println("*** Powermeter ***");

    auto error = deserializeJson(jsonBuffer, payload);
    if (error)
    {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(error.c_str());
      return;
    }

    JsonObject root = jsonBuffer.as<JsonObject>(); // get the root object

    if (root.containsKey("Time"))
    {
      DEBUG_PRINTLN(root["Time"].as<const char *>());
    }

    if (root.containsKey("SGMDD"))
    {
      int16_t watt = root["SGMDD"]["Momentan"].as<int16_t>();
      // Serial.println("Momentanverbrauch: " + String(watt));
      currentState.flatPower = watt;
      currentState.processed = false;
    }
  }
  else if (strcmp(topic, MQTT_TOPIC_SOLAR) == 0) // Solarpower
  {
    // Serial.println("*** Solarpower ***");
    int16_t watt = atoi(payload);
    // Serial.println("Solarleistung: " + String(watt));
    currentState.solarPower = watt;
  }
  else
  {
    Serial.print("*** Unknown Topic ***");
    Serial.println(topic);
  }
  /*
        if (root.containsKey("brightness")) {
            currentState.brightness = root["brightness"].as<uint8_t>();
        }

        if (root.containsKey("effect")) {
            string effect = root["effect"].as<const char*>();
            if (!effect.empty()) {
                currentState.effect = effect;
            }
        }

        if (root.containsKey("color")) {
            currentState.color.r = root["color"]["r"].as<uint8_t>();
            currentState.color.g = root["color"]["g"].as<uint8_t>();
            currentState.color.b = root["color"]["b"].as<uint8_t>();
        }

        updateState();
        */
}

void announceToHomeAssistant()
{
  Serial.println("MQTT: Annoucing to Home Assistant");
  /*
      StaticJsonBuffer<2048> jsonObject;
      JsonObject& discovery = jsonObject.createObject();
      discovery.set("name", NAME);
      discovery.set("unique_id", WiFi.macAddress());
      discovery.set("platform", "mqtt_json");
      discovery.set("state_topic", MQTT_TOPIC_STATE);
      discovery.set("availability_topic", MQTT_TOPIC_AVAILABLE);
      discovery.set("command_topic", MQTT_TOPIC_COMMAND);
      discovery.set("brightness", true);
      discovery.set("rgb", true);
      discovery.set("effect", true);

      /// Adds effect list
      JsonArray& effectList = discovery.createNestedArray("effect_list");
      for(std::map<string,int>::iterator e = effects.begin(); e != effects.end(); ++e) {
           effectList.add(e->first.c_str());
      }

      char payload[2048];
      discovery.printTo(payload);
      Serial.println(payload);
      mqttClient.publish(
          "homeassistant/light/ledstrip/config",
          2,
          true,
          payload
      );
      */
}

void updateState()
{
  Serial.println("STATE: Updating...");
  /*
      (currentState.on) ? ledstrip.start() : ledstrip.stop();
      ledstrip.setBrightness(currentState.brightness);
      ledstrip.setMode(effects[currentState.effect.c_str()]);
      ledstrip.setSpeed(currentState.speed);
      ledstrip.setColor(
          currentState.color.r,
          currentState.color.g,
          currentState.color.b
      );

      StaticJsonBuffer<512> currentJsonStateBuffer;
      JsonObject& currentJsonState = currentJsonStateBuffer.createObject();

      currentJsonState.set("speed", currentState.speed);
      currentJsonState.set("brightness", currentState.brightness);
      currentJsonState.set("state", currentState.on ? "ON" : "OFF");
      currentJsonState.set("effect", currentState.effect.c_str());

      JsonObject& currentJsonStateColor = currentJsonState.createNestedObject("color");
      currentJsonStateColor.set("r", currentState.color.r);
      currentJsonStateColor.set("g", currentState.color.g);
      currentJsonStateColor.set("b", currentState.color.b);

      char payload[512];
      currentJsonState.printTo(payload);

      // Store current state in EEPROM.
      Serial.println("EEPROM: Writing current state...");
      EEPROM.writeString(0, payload);
      EEPROM.commit();

      if (mqttClient.connected()) {
          Serial.println("STATE: Updating state on MQTT state topic");
          Serial.println(payload);
          mqttClient.publish(
              "ledstrip/state",
              2,
              true,
              payload
          );
      }
      */
}

void setup()
{
  pinMode(22, OUTPUT);

  Serial.begin(MONITOR_BAUDRATE);
  Serial.println("\n\n--------------------------------------------------------------------------");
  Serial.printf("Serial initialized...\n\tRX Pin: %i, TX Pin: %i \n", RX, TX);

  Serial.print("Reset/Boot Reason was: ");
  esp_reset_reason_t reason = esp_reset_reason();
  switch (reason)
  {
  case ESP_RST_UNKNOWN:
    Serial.println("Reset reason can not be determined");
    break;

  case ESP_RST_POWERON:
    Serial.println("Reset due to power-on event");
    break;

  case ESP_RST_EXT:
    Serial.println("Reset by external pin (not applicable for ESP32)");
    break;

  case ESP_RST_SW:
    Serial.println("Software reset via esp_restart");
    break;

  case ESP_RST_PANIC:
    Serial.println("Software reset due to exception/panic");
    break;

  case ESP_RST_INT_WDT:
    Serial.println("Reset (software or hardware) due to interrupt watchdog");
    break;

  case ESP_RST_TASK_WDT:
    Serial.println("Reset due to task watchdog");
    break;

  case ESP_RST_WDT:
    Serial.println("Reset due to other watchdogs");
    break;

  case ESP_RST_DEEPSLEEP:
    Serial.println("Reset after exiting deep sleep mode");
    break;

  case ESP_RST_BROWNOUT:
    Serial.println("Brownout reset (software or hardware)");
    break;

  case ESP_RST_SDIO:
    Serial.println("Reset over SDIO");
    break;

  default:
    break;
  }

  Serial.printf("\nBooting setup Version: %s / %s\n", __DATE__, __TIME__);

  setupRS485();
  WiFi.onEvent(onWiFiEvent);
  mqttClient.onConnect(onMQTTConnect);
  mqttClient.onDisconnect(onMQTTDisconnect);
  mqttClient.onMessage(onMQTTMessage);

  // Get connected
  connectToWiFi();

  // setupOTA(WIFI_HOSTNAME, OTA_PORT);
  

  // Serial.println(WiFi.localIP());

  // Serial.println("Write Spannung");
  //  Serial2.print(":01w10=1234,\r\n");
  // Serial2.print(":01w20=1450,5000,\r\n");
  // delay(50);
  // Serial2.print(":01w12=1,\r\n");
  // Serial.println("written???");
  dpm8624 = Dpm8624(&currentState);
  dpm8624.setupDPM(DEFAULT_BAT_VOLTAGE, DEFAULT_BAT_CURRENT);
  lcdstatus = LcdStatus(&currentState);
  lcdstatus.setupLCD();
  ds18b20status = DS18B20Stat(&currentState);
  ds18b20status.setupSensors();
  ads1115 = Ads1115(&currentState);
  ads1115.setupADS();
  analogReadResolution(12);
  analogSetWidth(12);

  configTime(0, 0, NTP_POOL);
  setenv("TZ", MY_TZ, 1);            // Set environment variable with your time zone
  tzset();

  esp_task_wdt_init(WDT_TIMEOUT, true); // enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);               // add current thread to WDT watch
  // Send Message to WhatsAPP
  sendMessage("Hello from ESP32!");
  // Send Message to WhatsAPP
  esp_task_wdt_reset();
  delay(2000);
  esp_task_wdt_reset();
  delay(2000);
  esp_task_wdt_reset();
  sendMessage(String("Laderegler hat nun IP: " + WiFi.localIP().toString()).c_str());
  ds18b20status.printHWaddresses();
  Serial.println("Setup  done. Start running...");
  temperatureTimer.attach(INTERVAL_ASK_TEMPERATURE, [](){ds18b20status.updateTemperature();});
  voltageTimer.attach(INTERVAL_ASK_BAT_VOLTAGE, [](){ads1115.updateVoltage();});
}

// the loop function runs over and over again forever
int tickerC = 0;

float voltBat = 0.0;

void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.print(&timeinfo, "%A, %d.%m.%Y %H:%M:%S ");
   if (timeinfo.tm_isdst == 1)               // Daylight Saving Time flag
    Serial.println("\tDST");  
  else
    Serial.println("\tSTD");
}

void loop()
{
 
  esp_task_wdt_reset();
 printLocalTime();
  /*
  if (minutes % 2 == 0 && seconds < 2)
  {
    ds18b20status.updateTemperature();
    ads1115.updateVoltage();
    Serial.print(timeClient.getFormattedTime());
    Serial.println(" UTC");
  }
  */
//Serial.println(timeClient.getFormattedTime());

  if (tickerC > 0)
    tickerC--;
  if (tickerC == 0 && currentState.flatPower > 1500)
  {
    tickerC = 60 * 10; // 10 Minuten
    esp_task_wdt_reset();
    Serial.println("Stromover1");
    Serial.println(currentState.flatPower);
    Serial.println("Stromover2");
    string msg = "Verbrauch über 1,5KW - " + std::to_string(currentState.flatPower);
    Serial.println(msg.c_str());
    sendMessage(String(msg.c_str()));
  }
  ArduinoOTA.handle();
  lcdstatus.updateHeaderStatus();
  // toggleRelayPowerIn();
  esp_task_wdt_reset();

  delay(1000);
}