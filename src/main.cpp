#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "settings.h"
#include "tempea-config.h"
#include "tempea-mqtt.h"
#include "sensor-temp.h"
#include "utils.h"

static TempeaConfig config = TempeaConfig(EEPROM_CONFIG_ADDR);

// ############################################################# WIFI AND MQTT CLIENT

static WiFiClient wifiClient = WiFiClient();
static bool wifiConnected = false;

// ############################################################# SERVER

ESP8266WebServer server(WIFI_CONFIG_SERVER_PORT);

static const char* index_html= "<title>tempea-config</title>"
"<style>"
"body, input {"
"background-color: #2c3e50 !important;"
"color: #ecf0f1;"
"font: 27px 'Trebuchet MS';}"
"h1 {"
"color: #27ae60;"
"border-bottom: 2px solid #c0392b;}"
"form {"
"width: 600px;"
"height: 600px;"
"position: absolute;"
"left:0; right:0;"
"top:0; bottom:0;"
"margin:auto;"
"overflow:hidden;}"
"div {"
"margin:10px;"
"float:left;"
"width: 100%;}"
"input {"
"margin-right: 15px;"
"font-size: 24px;"
"float:right;"
"border: 4px solid #2980b9;}"
".submit {"
"font-weight: bold;}"
"</style>"
"<form action='/config' method='POST'>"
"<h1>tempea</h1>"
"<div class='label'>Wifi SSID"
"<input name='wifi_ssid' placeholder='config.get()->wifi_ssid'>"
"</div>"
"<div>Wifi Password"
"<input name='wifi_password' placeholder='config.get()->wifi_password'>"
"</div>"
"<div>MQTT Host"
"<input name='mqtt_host' placeholder='config.get()->mqtt_host'>"
"</div>"
"<div>MQTT Port"
"<input name='mqtt_port' placeholder='config.get()->mqtt_port'>"
"</div>"
"<div>MQTT Client ID"
"<input name='mqtt_client_id' placeholder='config.get()->mqtt_client_id'>"
"</div>"
"<div>MQTT Topic"
"<input name='mqtt_topic' placeholder='config.get()->mqtt_topic'>"
"</div>"
"<div>"
"<input class='submit' type='submit' value='Set'>"
"</div>"
"</form>";

void handleConfig() {

  if( server.hasArg("wifi_ssid") && server.arg("wifi_ssid") != NULL){
    Serial.print("SET config.get()->wifi_ssid:");
    Serial.println(server.arg("wifi_ssid"));
    server.arg("wifi_ssid").toCharArray(config.get()->wifi_ssid, WIFI_SSID_LEN);
  }
  if( server.hasArg("wifi_password") && server.arg("wifi_password") != NULL){
    Serial.print("SET config.get()->wifi_password:");
    Serial.println(server.arg("wifi_password"));
    server.arg("wifi_password").toCharArray(config.get()->wifi_password, WIFI_PASSWORD_LEN);
  }
  if( server.hasArg("mqtt_host") && server.arg("mqtt_host") != NULL){
    Serial.print("SET config.get()->mqtt_host:");
    Serial.println(server.arg("mqtt_host"));

    // TODO malloc should not happen inside of functions --> move them to global space ...
    char tmp_mqtt_host_str[16] = { 0 };
    unsigned int tmp_mqtt_host[4];

    server.arg("mqtt_host").toCharArray(tmp_mqtt_host_str, 16);

    if(sscanf(tmp_mqtt_host_str, "%u.%u.%u.%u", &tmp_mqtt_host[0], &tmp_mqtt_host[1], &tmp_mqtt_host[2], &tmp_mqtt_host[3]) == 4){
      config.get()->mqtt_host[0] = tmp_mqtt_host[0];
      config.get()->mqtt_host[1] = tmp_mqtt_host[1];
      config.get()->mqtt_host[2] = tmp_mqtt_host[2];
      config.get()->mqtt_host[3] = tmp_mqtt_host[3];

    }
    //TODO maybe send 400 on else
  }
  if( server.hasArg("mqtt_port") && server.arg("mqtt_port") != NULL){
    Serial.print("SET config.get()->mqtt_port:");
    Serial.println(server.arg("mqtt_port"));
    int tmp_mqtt_port = server.arg("mqtt_port").toInt();
    if(tmp_mqtt_port > 0){
      config.get()->mqtt_port = tmp_mqtt_port;
    }
  }
  if( server.hasArg("mqtt_client_id") && server.arg("mqtt_client_id") != NULL){
    Serial.print("SET config.get()->mqtt_client_id:");
    Serial.println(server.arg("mqtt_client_id"));
    server.arg("mqtt_client_id").toCharArray(config.get()->mqtt_client_id, sizeof(config.get()->mqtt_client_id));
  }
  if( server.hasArg("mqtt_topic") && server.arg("mqtt_topic") != NULL){
    Serial.print("SET config.get()->mqtt_topic:");
    Serial.println(server.arg("mqtt_topic"));
    server.arg("mqtt_topic").toCharArray(config.get()->mqtt_topic, sizeof(config.get()->mqtt_topic));
  }

  if(config.save()){
    server.send(200, "text/html", "<p>Set Config</p>");
    ESP.restart();
  } else {
    server.send(400, "text/text", "400: Invalid Request");
  }
  //TODO check if we have to send a 400 on any request
  // server.send(400, "text/plain", "400: Invalid Request");
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void handleRoot() {
  server.send(200, "text/html", index_html);
}

const char *ssid = WIFI_CONFIG_SSID;

bool do_connect_wifi(void){
  // Stage one: check for default connection
  for(byte t = 0; t<WIFI_RETRY_COUNT; t++){
    if(WiFi.status() == WL_CONNECTED){
      Serial.printf("Connected to %s\n", config.get()->wifi_ssid);
      return true;
    }
    Serial.printf("Trying to connect to wifi %u\n", t);
    Serial.printf("SSID: %s, PW: %s\n", config.get()->wifi_ssid, config.get()->wifi_password);
    if(t == 0){
      WiFi.begin(config.get()->wifi_ssid, config.get()->wifi_password);
    }
    delay(1000);
  }
  Serial.printf("Error: Unable to connect to %s\n", config.get()->wifi_ssid);
  return false;
}

void esp_sleep(uint8_t seconds){
  Serial.printf("Going to sleep for %u\n", seconds);
#if DEBUG == 1
  delay(seconds * 1000);
#else
  ESP.deepSleep(seconds*1000000); 
#endif
}

void setup(){
  delay(1000);
#if DEBUG == 1
  Serial.begin(115200);
  String reason = ESP.getResetReason();
  int n = reason.length(); 

  // declaring character array 
  char char_array[n + 1]; 

  // copying the contents of the 
  // string to char array 
  strcpy(char_array, reason.c_str()); 

  Serial.println();
  Serial.println("ESP got reset");
  Serial.print("Reason: ");
  for (int i = 0; i < n; i++) 
      Serial.printf("%c", char_array[i]); 

  Serial.println();
#else
  // Serial is only available if pin is not defined as output
  pinMode(LED_PIN, OUTPUT);
#endif

  //reset the config if needed
#if RESET_CONFIG_ON_BOOT == 1
  config.reset();
#endif

  //inital config setup
  config.load();
}

void run_setup(){
  Serial.printf("run_setup");

  WiFi.setOutputPower(20.5);
  WiFi.softAP(ssid);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/config", HTTP_POST, handleConfig); 
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  for(;;){
    server.handleClient();
  }
}

static char str_temp[10];

void loop(){
  
  if(config.validate()){
    Serial.printf("Config exists and was loaded");
  } else {
    run_setup();
  }

  if(do_connect_wifi()){
    Serial.printf("run_main");

    blink(1);

    MqttClient mqttClient = MqttClient(&wifiClient, &config);
    TempSensor tempSensor = TempSensor(DS18B20_DATA_PIN);

    float temp = tempSensor.getTempC();
    if(temp == -127){
      Serial.printf("Temperature sensor not connected on pin %u\n", DS18B20_DATA_PIN);
    }else{
      Serial.printf("Temp: %f\n", temp);

      Serial.printf(str_temp,"%f\n",temp);

      bool mqtt_err = false;
      mqtt_err = mqttClient.connect() | mqtt_err;
      mqtt_err = mqttClient.publish(str_temp) | mqtt_err;
      if(mqtt_err){
        Serial.printf("Unable to publish data to mqtt\n");
        run_setup();
      }
    }

    blink(2);
    esp_sleep(SLEEP_SEC);
    return;
  } else {
    Serial.printf("Unable to connect to wifi\n");
    blink(5);
    run_setup();
  }
}
