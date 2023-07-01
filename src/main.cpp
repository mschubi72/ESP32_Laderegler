#include "main.h"
#include "lcdstatus.h"
#include "ds18b20stat.h"
#include "ads1115.h"
#include "DPM8600.h"

#include <esp_task_wdt.h>
#include <Preferences.h>


#define DEBUGLEVEL DEBUGLEVEL_VERBOSE
// #define MQTTDEBUG

#include "debug.h"

using namespace std;

#define WDT_TIMEOUT 10 // 5 seconds WDT

String phoneNumber = PHONENUMBER;
String apiKey = APIKEY;

State currentState;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
Ticker temperatureTimer;
Ticker voltageTimer;
Ticker lcdTimer;
Ticker dpmTimer;
Ticker debugTimer;

WiFiUDP ntpUDP;
LcdStatus lcdstatus = NULL;
DS18B20Stat ds18b20status = NULL;
Ads1115 ads1115 = NULL;
// Dpm8624 dpm8624 = NULL;
DPM8600 dpm8624 = DPM8600(1, &currentState);


bool ipReady = false;
u_int currentTime = 0;

const char* CHARGE_ENABLED = "CE";
const char* INVERTER_ENABLED = "IE";
bool chargeEnabled = true; //is battery charging enabled
bool inverterEnabled = true; //is inverter for feeding enabled

Preferences preferences;

AsyncWebServer webserver(11111);

void sendMessage(String message)
{

  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  debugV("Message URL:");
  debuglnV(url.c_str());
  esp_task_wdt_reset();

  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  esp_task_wdt_reset();
  delay(5);
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200)
  {
    debuglnV("Message sent successfully");
  }
  else
  {
    debuglnE("Error sending the message");
    debugE("HTTP response code: ");
    debuglnE(httpResponseCode);
  }
  esp_task_wdt_reset();
  // Free resources
  http.end();
}

void switchRelay(bool switchRelayInOn, bool switchRelayOutOn, State *state)
{
  // http://192.168.1.87/switch/lader-power_relay_out/turn_on
  String urlIn = "http://192.168.1.87/switch/lader-power_relay_in/turn_on";
  String urlOut = "http://192.168.1.87/switch/lader-power_relay_out/turn_on";
  if (!switchRelayInOn)
  {
    urlIn = "http://192.168.1.87/switch/lader-power_relay_in/turn_off";
  }
  if (!switchRelayOutOn)
  {
    urlOut = "http://192.168.1.87/switch/lader-power_relay_out/turn_off";
  }

  // Serial.printf("Switch RelayIn: %s... ", urlIn);
  HTTPClient http;
  http.begin(urlIn);
  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  esp_task_wdt_reset();
  delay(5);
  // Send HTTP POST request
  int httpResponseCode = http.POST(urlIn);
  if (httpResponseCode == 200)
  {
    // Serial.println(" Message sent successfully");
    state->relay_in = switchRelayInOn;
  }
  else
  {
    debuglnE("Error sending the message");
    debugE("HTTP response code: ");
    debuglnE(httpResponseCode);
  }
  esp_task_wdt_reset();
  delay(5);
  // Free resources
  http.end();

  // Serial.printf("Switch RelayOut: %s... ", urlOut);
  // HTTPClient http;
  http.begin(urlOut);
  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  esp_task_wdt_reset();
  delay(5);

  // Send HTTP POST request
  httpResponseCode = http.POST(urlIn);
  if (httpResponseCode == 200)
  {
    // Serial.println(" Message sent successfully");
    state->relay_out = switchRelayOutOn;
  }
  else
  {
    debuglnE("Error sending the message");
    debugE("HTTP response code: ");
    debuglnE(httpResponseCode);
  }

  esp_task_wdt_reset();
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
    ipReady = true;
    connectToMQTT();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("WIFI: Disconnected.");
    ipReady = false;
    mqttReconnectTimer.detach();
    wifiReconnectTimer.once(2, connectToWiFi);
    break;
  default:
    break;
  }
}

void connectToWiFi()
{
  WiFi.setHostname(WIFI_HOSTNAME); //has to be before calling WiFi.mode....
  WiFi.mode(WIFI_STA);

  // FIX FOR USING 2.3.0 CORE (only .begin if not connected)
  if (WiFi.status() == WL_CONNECTED)
    return;

  // Serial.println("WIFI: Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onMQTTConnect(bool sessionPresent)
{
  Serial.println("MQTT: Connected.");
  mqttClient.subscribe(MQTT_TOPIC_SENSOR, 0);
  mqttClient.subscribe(MQTT_TOPIC_SOLAR, 0);

  mqttClient.subscribe(MQTT_TOPIC_POWER_IN, 0);
  mqttClient.subscribe(MQTT_TOPIC_POWER_OUT, 0);
  mqttClient.subscribe(MQTT_TOPIC_RELAY_IN, 0);
  mqttClient.subscribe(MQTT_TOPIC_RELAY_OUT, 0);

  mqttClient.publish(MQTT_TOPIC_AVAILABLE, 0, true, CMD_ALIVE);
  announceToHomeAssistant();
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
  // https://github.com/marvinroger/async-mqtt-client/issues/35
  char new_payload[len + 1];
  new_payload[len] = '\0';
  strncpy(new_payload, payload, len);
  esp_task_wdt_reset();
  processStateJson(topic, new_payload);
  esp_task_wdt_reset();
}

void processStateJson(char *topic, char *payload)
{
  StaticJsonDocument<512> jsonBuffer;
  debugMqtt("Topic: ");
  debuglnMqtt(topic);
  debugMqtt("Payload: ");
  debuglnMqtt(payload);
  int16_t watt = 0;

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
    esp_task_wdt_reset();
    delay(5);
    if (root.containsKey("Time"))
    {
      debuglnMqtt(root["Time"].as<const char *>());
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
    watt = atoi(payload);
    // Serial.println("Solarleistung: " + String(watt));
    currentState.solarPower = watt;
    esp_task_wdt_reset();
    delay(5);
  }
  else if (strcmp(topic, MQTT_TOPIC_POWER_IN) == 0) // Ladeleistung
  {
    watt = atoi(payload);
    currentState.chargePowerRaw = watt;
  }
  else if (strcmp(topic, MQTT_TOPIC_POWER_OUT) == 0) // Einspeiseleistung Akku
  {
    watt = atoi(payload);
    // Serial.println("Einspeiseleistung: " + String(watt));
    currentState.feedPowerBat = watt;
  }
  else if (strcmp(topic, MQTT_TOPIC_RELAY_IN) == 0) // Relay Status Lader
  {
    if (strcmp(payload, "ON") == 0)
    {
      currentState.relay_in = true;
    }
    else
    {
      currentState.relay_in = false;
    }
    // Serial.println("Relay Lader: " + String(payload));
  }
  else if (strcmp(topic, MQTT_TOPIC_RELAY_OUT) == 0) // Relay Status Einspeisung
  {
    if (strcmp(payload, "ON") == 0)
    {
      currentState.relay_out = true;
    }
    else
    {
      currentState.relay_out = false;
    }
    // Serial.println("Relay Einspeisung: " + String(payload));
  }
  else
  {
    debuglnE("*** Unknown Topic ***");
    debuglnE(topic);
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
  // Serial.println("MQTT: Annoucing to Home Assistant");

  //    StaticJsonBuffer<2048> jsonObject;
  StaticJsonDocument<256> mqttstate;
  //   JsonObject& mqttstate = jsonObject.createObject();
  mqttstate["formattedTime"] = currentState.formattedTime;
  mqttstate["batStatus"] = currentState.batStatus;
  mqttstate["batVoltage"] = currentState.batVoltage;
  mqttstate["chargePowerRaw"] = currentState.chargePowerRaw;
  mqttstate["chargePower"] = currentState.chargePower;
  mqttstate["feedPowerBat"] = currentState.feedPowerBat;
  mqttstate["chargePowerRaw"] = currentState.chargePowerRaw;
  mqttstate["tempBat1"] = currentState.tempBat1;
  mqttstate["tempBat2"] = currentState.tempBat2;
  mqttstate["tempInverter"] = currentState.tempInverter;
  mqttstate["tempDpm"] = currentState.tempDpm;

  char payload[256];
  serializeJson(mqttstate, payload);
  // Serial.println(payload);
  //  Serial.println(sizeof payload);
  esp_task_wdt_reset();
  delay(5);
  uint16_t returnval = mqttClient.publish(MQTT_TOPIC_STATUS, 2, true, payload);
  debugfMqtt("mqttClient.publish return %i\n", returnval);
  esp_task_wdt_reset();
}

// Replaces placeholder for main html page
String processMainHtml(const String& var){
  /*
 
  debugfD("\tRelay_In: %s, Relay_Out: %s\n\n", state->relay_in ? "On" : "Off", state->relay_out ? "On" : "Off");
  esp_task_wdt_reset();
  debugfD("\tDPM->V: %f, A: %f, VA: %i, ON: %f, CC: %f, T: %f\n",
          dpm8624.read('v'), dpm8624.read('c'), currentState.chargePower, dpm8624.read('p'), dpm8624.read('s'), dpm8624.read('t'));
  debugfD("\tTimecode: %d\n", currentTime);
  debugfD("\tCharger enabled: %s; Inverter enabled: %s\n", chargeEnabled ? "true" : "false", inverterEnabled ? "true" : "false" );
  esp_task_wdt_reset();
}
  */

  if(var == "BAT_STATUS"){
    return String(currentState.batStatus);
  }else if(var == "BAT_VOLTAGE"){
    return String(currentState.batVoltage);
  }else if(var == "TEMP1"){
    return String(currentState.tempBat1);
  }else if(var == "TEMP2"){
    return String(currentState.tempBat2);
  }else if(var == "SOLAR"){
    return String(currentState.solarPower);
  }else if(var == "CHARGE"){
    return String(currentState.chargePower);
  }else if(var == "FEED"){
    return String(currentState.feedPowerBat);
  }else if(var == "FLAT"){
    return String(currentState.flatPower);
  }else if(var == "CHARGE_E"){
    return chargeEnabled ? String("true") : String("false");
  }else if(var == "INV_E"){
    return inverterEnabled ? String("true") : String("false");
  }

  return String();
}

void prepareWebServer(){
/*
  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello from server 1");
});
*/
webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processMainHtml);
  });
  webserver.begin();
  
}

void setup()
{
  pinMode(22, OUTPUT); // LED

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

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS initialized...");
  
  setupRS485();
  WiFi.onEvent(onWiFiEvent);
  mqttClient.onConnect(onMQTTConnect);
  mqttClient.onDisconnect(onMQTTDisconnect);
  mqttClient.onMessage(onMQTTMessage);

  // Get connected
  connectToWiFi();
  Serial.print("Wait for IP ");
  while (!ipReady)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println(" done.");

  setupOTA(WIFI_HOSTNAME, OTA_PORT);

  dpm8624.begin(Serial2);
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
  setenv("TZ", MY_TZ, 1); // Set environment variable with your time zone
  tzset();

  preferences.begin("laderegler", false);
  chargeEnabled = preferences.getBool(CHARGE_ENABLED, true);
  inverterEnabled = preferences.getBool(INVERTER_ENABLED, true);
  preferences.end();

  esp_task_wdt_init(WDT_TIMEOUT, true); // enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);               // add current thread to WDT watch
  esp_task_wdt_reset();
  delay(2000);
  esp_task_wdt_reset();
  delay(2000);
  esp_task_wdt_reset();

  sendMessage(String("Laderegler started. IP: " + WiFi.localIP().toString()).c_str());
  ds18b20status.printHWaddresses();
  Serial.println("Setup  done.\n==>Now in running state...\n");
  temperatureTimer.attach(INTERVAL_ASK_TEMPERATURE, []()
                          { ds18b20status.updateTemperature(); });
  voltageTimer.attach(INTERVAL_ASK_BAT_VOLTAGE, []()
                      { ads1115.updateVoltage(); });
  lcdTimer.attach(INTERVAL_REFRESH_DISPLAY, []()
                  { lcdstatus.updateFullScreen(); });
  dpmTimer.attach(INTERVAL_ASK_BAT_VOLTAGE, []()
                  { dpm8624.updateStatus(); });
  //  debugTimer.attach(5, []()
  //                    { debugState(&currentState); });
  prepareWebServer();
}

// the loop function runs over and over again forever
int tickerC = 0;

float voltBat = 0.0;

void printLocalTime(State *state)
{
  struct tm timeinfo;
  char buffer[35];
  if (!getLocalTime(&timeinfo))
  {
    debuglnE("Failed to obtain time");
    return;
  }
  strftime(buffer, 35, "%d.%m.%Y %H:%M:%S", &timeinfo);
  // Serial.println(buffer);

  state->formattedTime = strdup(buffer);
  currentTime = timeinfo.tm_hour * 100 + timeinfo.tm_min;
  // Serial.print("-->");
  // Serial.println(state->formattedTime);

  /*Serial.print(&timeinfo, "%A, %d.%m.%Y %H:%M:%S ");
   if (timeinfo.tm_isdst == 1)               // Daylight Saving Time flag
    Serial.println(" DST");
  else
    Serial.println(" STD");
    */
}

void debugState(State *state)
{
  debugfD("-------- State %s ----------\n", state->formattedTime);
  debugfD("\tBat Status: %i, Voltage: %fV, Ladeleistung: %iW(Raw)/%iW(DPM)\n", state->batStatus, state->batVoltage, state->chargePowerRaw, state->chargePower);
  debugfD("\tTemp Batterie1: %.1f°C, Batterie2: %.1f°C, Inverter: %.1f°C, DPM: %.1f°C\n", state->tempBat1, state->tempBat2, state->tempInverter, state->tempDpm);
  debugfD("\tEinspeiseleistung: %iW, Zähler: %iW, Solarleistung: %iW\n", state->feedPowerBat, state->flatPower, state->solarPower);
  debugfD("\tProcessed: %s\n", state->processed ? "true" : "false");
  debugfD("\tRelay_In: %s, Relay_Out: %s\n\n", state->relay_in ? "On" : "Off", state->relay_out ? "On" : "Off");
  esp_task_wdt_reset();
  debugfD("\tDPM->V: %f, A: %f, VA: %i, ON: %f, CC: %f, T: %f\n",
          dpm8624.read('v'), dpm8624.read('c'), currentState.chargePower, dpm8624.read('p'), dpm8624.read('s'), dpm8624.read('t'));
  debugfD("\tTimecode: %d\n", currentTime);
  debugfD("\tCharger enabled: %s; Inverter enabled: %s\n", chargeEnabled ? "true" : "false", inverterEnabled ? "true" : "false" );
  esp_task_wdt_reset();
}

void doAction()
{
  if (currentState.processed)
  { // value was processed
    //   Serial.println("Nothing to process...");
  }
  else
  { // has to be process

    debuglnD("Must be processed...");
    debugState(&currentState);
    currentState.processed = true;
    if (currentTime > EVENING || currentTime < MORNING)
    { // Nachteinspeisung
      debugfV("BatStatus: %i  (%i)\n", currentState.batStatus, currentTime);
      if (currentState.batStatus > 1)
      { // battery can provide power, so let's do it
        // >1 -> we have a switch hysteresis
        if (!currentState.relay_out)
        { // first time switch on
          debuglnV("vor sendMessage InverterOn");
          sendMessage(String("Switch Inverter on ") + String(currentState.formattedTime));
          debuglnV("nach sendMessage InverterOn");
        }
        esp_task_wdt_reset();
        debuglnV("vor SwitchRelay on");
        switchRelay(false, true, &currentState);
        debuglnV("nach SwitchRelay on");
      }
      else if (currentState.batStatus == 1)
      {
        // do nothing
        debuglnV("batStatus == 1 - do nothing.");
      }
      else
      { // Bat Empty -> switch off
        debugfV("RelayState: %i\n", currentState.relay_out);

        if (currentState.relay_out)
        { // first time switch off
          debuglnV("vor sendMessage InverterOff");
          sendMessage(String("Switch Inverter off ") + String(currentState.formattedTime));
        }
        debuglnV("nach sendMessage InverterOff");

        esp_task_wdt_reset();
        debuglnV("vor SwitchRelay off");
        switchRelay(false, false, &currentState);
        debuglnV("nach SwitchRelay off");
      }
    }
    else
    { // Tagsteuerung
      debuglnV("vor SwitchRelay on,off");
      switchRelay(true, false, &currentState);
      debuglnV("nach SwitchRelay on,off");

      // switchRelay(true, true, &currentState); //only for manual feeding

      if (currentState.flatPower < (-1 * DEFAULT_POWER_THRESHOLD))
      { // Einspeisung
        debuglnV("--> Einspeisung");

        float isOn = dpm8624.read('p');
        float current = dpm8624.read('c');
        if (isOn == 0.0)
        { // nicht eingeschaltet
          debuglnV("--> Einspeisung starten");

          dpm8624.write('c', 0.1);
          dpm8624.power(true);
          debuglnV("<-- Einspeisung starten");
        }
        else
        {
          debuglnV("--> Einspeisung erhöhen");

          float voltage = dpm8624.read('v');
          if (voltage < (DEFAULT_BAT_VOLTAGE - 0.2) || current < 0.2)
          { // prevent switch to constant voltage
            delay(10);
            //  dpm8624.write('v', DEFAULT_BAT_VOLTAGE+0.7);
            //  Serial.printf("setVoltage: %f\n",DEFAULT_BAT_VOLTAGE+0.7);
            delay(10);
          }

          int faktor = (-1 * currentState.flatPower) % DEFAULT_POWER_THRESHOLD;
          debugfV("PowerFaktor: %i\n", faktor);
          //          current += 0.1; // gehe hoch
          current += faktor * 0.1;
          dpm8624.write('c', current);
          debugfV("setCurrent: %f\n", current);
          delay(10);
          dpm8624.power(true);
          debuglnV("<-- Einspeisung erhöhen");
        }
        debuglnV("<-- Einspeisung");
      }
      else
      { // evtl. keine Einspeisung mehr
        if (currentState.flatPower >= 0)
        {
          debuglnV("--> evtl. keine Einspeisung");

          float isOn = dpm8624.read('p');
          float current = dpm8624.read('c');
          debugfV("isOn: %f, current: %f\n", isOn, current);
          if (isOn > 0.0)
          { // eingeschaltet
            if (current >= 0.2)
            { // geht noch runter
              debuglnV("--> Einspeisung erniedrigen");

              current -= 0.1;
              dpm8624.write('c', current);
              debuglnV("<-- Einspeisung erniedrigen");
            }
            else
            {
              debuglnV("--> Einspeisung abschalten");
              dpm8624.power(false);
              debuglnV("<-- Einspeisung abschalten");
            }
          }
          else
          {
            // fine, no lader
            debuglnV("<--> all fine, keine Ladung");
          }
          debuglnV("<-- keine Einspeisung");
        }
        else
        {
          // fine, lader in optimum
          debuglnV("<--> all fine, Ladung optimal");
        }
      }
    }
    if (currentState.batVoltage >= DEFAULT_BAT_VOLTAGE)
    {
      debuglnV("--> Ladung abschalten, Bat voll");
      dpm8624.power(false);
      debuglnV("<-- Ladung abschalten, Bat voll");
    }
    esp_task_wdt_reset();
    delay(500);
    esp_task_wdt_reset();
    announceToHomeAssistant();
    debugState(&currentState);
  }
}

int incomingByte = 0; // for incoming serial data
String erg;

void loop()
{
  esp_task_wdt_reset();
  printLocalTime(&currentState);
  // dpm8624.write('c',2.5);
  // dpm8624.power(false);
  /*
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
    */
  ArduinoOTA.handle();
  // toggleRelayPowerIn();
  esp_task_wdt_reset();

  delay(1000);
  esp_task_wdt_reset();
  delay(1000);
  esp_task_wdt_reset();
  delay(1000);
  esp_task_wdt_reset();

  doAction();
}