#include "main.h"
#include "lcdstatus.h"

#include <esp_task_wdt.h>

using namespace std;

//#define DEBUG //"Schalter" zum aktivieren

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

#define WDT_TIMEOUT 5 //5 seconds WDT

String phoneNumber = "4915118026736";
String apiKey = "7278274";

State currentState;


AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
LcdStatus lcdstatus=NULL;

void sendMessage(String message){

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
  if (httpResponseCode == 200){
    Serial.println("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

void toggleRelayPowerIn(){

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
  if (httpResponseCode == 200){
    Serial.println("Message sent successfully");
  }
  else{
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
    Serial.println("WIFI: Connected! Waiting for IP...");
    break;
  case SYSTEM_EVENT_STA_LOST_IP:
    Serial.println("WIFI: Lost IP address...");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.println("WIFI: Got IP!");
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

  Serial.println("WIFI: Connecting to WiFi...");
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
  if (strcmp(topic, MQTT_TOPIC_SENSOR) == 0) //Powermeter
  {
   //Serial.println("*** Powermeter ***");
  
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
      //Serial.println("Momentanverbrauch: " + String(watt));
      currentState.flatPower = watt;
      currentState.processed = false;
    }
  }
  else if (strcmp(topic, MQTT_TOPIC_SOLAR) == 0) //Solarpower
  {
    //Serial.println("*** Solarpower ***");
    int16_t watt = atoi(payload);
      //Serial.println("Solarleistung: " + String(watt));
      currentState.solarPower = watt;
  }
  else{
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
 Serial.print("Reset/Boot Reason was: "); 
    esp_reset_reason_t reason = esp_reset_reason();
    switch (reason) {
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

  Serial.println("Booting setup...");

  setupRS485();
  WiFi.onEvent(onWiFiEvent);
  mqttClient.onConnect(onMQTTConnect);
  mqttClient.onDisconnect(onMQTTDisconnect);
  mqttClient.onMessage(onMQTTMessage);

  // Get connected
  connectToWiFi();

  // setupOTA(WIFI_HOSTNAME, OTA_PORT);

  Serial.print("MOSI: ");
  Serial.println(MOSI);
  Serial.print("MISO: ");
  Serial.println(MISO);
  Serial.print("SCK: ");
  Serial.println(SCK);
  Serial.print("SS: ");
  Serial.println(SS);

  Serial.print("SDA: ");
  Serial.println(SDA);
  Serial.print("SCL: ");
  Serial.println(SCL);

  timeClient.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());

  Serial.println("---");
  Serial.println("init Serial 2...");

  Serial.println("Serial Txd is on pin: " + String(TX));
  Serial.println("Serial Rxd is on pin: " + String(RX));

  Serial.println("Write Spannung");
  // Serial2.print(":01w10=1234,\r\n");
  Serial2.print(":01w20=1450,5000,\r\n");
  delay(50);
  Serial2.print(":01w12=1,\r\n");
  Serial.println("written???");
  lcdstatus = LcdStatus(&currentState);
  lcdstatus.setupLCD();
  analogReadResolution(12);
  analogSetWidth(12);
  
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
  // Send Message to WhatsAPP
  sendMessage("Hello from ESP32!");
  // Send Message to WhatsAPP
  esp_task_wdt_reset();
  delay(2000);
  esp_task_wdt_reset();
  delay(2000);
  esp_task_wdt_reset();
  sendMessage(String("Laderegler hat nun IP: " + WiFi.localIP().toString()).c_str());
}

// the loop function runs over and over again forever
int tickerC = 0;
void loop()
{
 esp_task_wdt_reset();
  if(tickerC > 0) tickerC--;

  if(tickerC == 0 && currentState.flatPower > 1500){
    tickerC = 60*10; // 10 Minuten
    esp_task_wdt_reset();
    Serial.println("Stromover1");
    Serial.println(currentState.flatPower);
    Serial.println("Stromover2");
    string msg = "Verbrauch ??ber 1,5KW - " + std::to_string(currentState.flatPower);
    Serial.println(msg.c_str());
    sendMessage(String(msg.c_str()));
  }
  ArduinoOTA.handle();
  lcdstatus.updateHeaderStatus();
//toggleRelayPowerIn();
 esp_task_wdt_reset();
 
  /*
    u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB12_tr);
  timeClient.update();
  u8g2.drawStr(6, 16, timeClient.getFormattedTime().c_str());
  u8g2.sendBuffer();
String a;
Serial2.print(":01r10=0,\r\n");
delay(50);
while(Serial2.available()) {
  a= Serial2.readString();// read the incoming data as string
  Serial.println(a);
}
Serial2.print(":01r11=0,\r\n");
delay(50);
while(Serial2.available()) {
  a= Serial2.readString();// read the incoming data as string
  Serial.println(a);
}
Serial2.print(":01r30=0,\r\n");
delay(50);
while(Serial2.available()) {
  a= Serial2.readString();// read the incoming data as string
  Serial.println(a);
}
Serial2.print(":01r31=0,\r\n");
delay(50);
while(Serial2.available()) {
  a= Serial2.readString();// read the incoming data as string
  Serial.println(a);
}
  // u8g2.clearBuffer();
  // u8g2.setFont(u8g2_font_ncenB12_tr);
  // u8g2.drawStr(0, 12, "Hello World!");
  // u8g2.setFont(u8g2_font_ncenB10_tr);
  // u8g2.drawStr(0,22, String("IP: " + WiFi.localIP().toString()).c_str() );
  // u8g2.setCursor(0,40);
  // u8g2.print(WiFi.localIP());
  // timeClient.update();

  Serial.println(timeClient.getFormattedTime());
  // u8g2.setFont(u8g2_font_ncenB12_tr);
  // u8g2.drawStr(6, 16, timeClient.getFormattedTime().c_str());
  // u8g2.sendBuffer();
  // u8g2.drawXBM( 116, 52, u8g_logo_width, u8g_logo_height, image_bits3);
  /*
  u8g2.setFont(u8g2_font_unifont_t_symbols);
  for (int x = 0x2100; x < 0x2740; x = x + 16)
  {
    u8g2.clearBuffer();
    for (int y = 0; y < 8; y++)
    {
      u8g2.drawGlyph(16 * y, 30, x + y);
      u8g2.drawGlyph(16 * y, 50, x + 8 + y);
    }
    u8g2.sendBuffer();
    delay(150);
  }
  */
  // u8g2.drawGlyph(6, 50, 0x2603);	/* dec 9731/hex 2603 Snowman
  // u8g2.drawGlyph(120, 27, 0x21af);	/* Blitz */
  /*
    u8g2.setFont(u8g2_font_unifont_t_emoticons);
    for (int x = 0x0020; x < 0x0100; x = x + 16)
    {
      u8g2.clearBuffer();
      for (int y = 0; y < 8; y++)
      {
        u8g2.drawGlyph(16 * y, 30, x + y);
        u8g2.drawGlyph(16 * y, 50, x + 8 + y);
      }
      u8g2.sendBuffer();
      delay(150);
    }
    ////
    //  u8g2.drawGlyph(26, 50, 0x0031);	/* Smiley

    u8g2.setFont(u8g2_font_battery19_tn);
    u8g2.drawGlyph(46, 50, 0x0030);  /* Akku /
    u8g2.drawGlyph(57, 50, 0x0031);  /* Akku /
    u8g2.drawGlyph(68, 50, 0x0032);  /* Akku /
    u8g2.drawGlyph(79, 50, 0x0033);  /* Akku /
    u8g2.drawGlyph(90, 50, 0x0034);  /* Akku /
    u8g2.drawGlyph(101, 50, 0x0035); /* Akku /
    u8g2.drawGlyph(112, 50, 0x0035); /* Akku /
    u8g2.setFont(u8g2_font_unifont_t_weather);
    u8g2.drawGlyph(6, 64, 0x0033);  /* Sonne /
    u8g2.drawGlyph(25, 64, 0x0034); /* Sonne /
    u8g2.drawGlyph(42, 64, 0x0035); /* Sonne /

    u8g2.sendBuffer();

    digitalWrite(22, HIGH); // turn the LED on
    delay(500);             // wait for 500 milliseconds
    digitalWrite(22, LOW);  // turn the LED off
    delay(1500);            // wait for 500 milliseconds
    u8g2.clearBuffer();
    digitalWrite(22, HIGH);
    // u8g2.drawXBM( 116, 52, u8g_logo_width, u8g_logo_height, image_bits4);
    delay(500);
    digitalWrite(22, LOW);
    */
  delay(1000);
}