#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdint.h>
#include <stdio.h>
#include <ESP_EEPROM.h>

#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define DEBUG             1

#define WIFI_CONFIG_SSID     "tempbeaCONFIG" //TODO check or add cpuid for SSID
// #define WIFI_SSID         "MoBase"
// #define WIFI_PASSWORD     "1224481632"

#define WIFI_RETRY_COUNT  30
#define LED_PIN           1
#define DS18B20_DATA_PIN  5
// #ifndef _MQTT_HOST
  // #define _MQTT_HOST
  // const static IPAddress MQTT_HOST(192, 168, 1, 184);
  // const static IPAddress MQTT_HOST(172, 16, 0, 2);
// #endif //_MQTT_HOST
// #define MQTT_PORT         1883

#ifndef _DEFAULT_MQTT_HOST
  #define _DEFAULT_MQTT_HOST
  // const static IPAddress MQTT_HOST(192, 168, 1, 184);
  const static IPAddress DEFAULT_MQTT_HOST(192, 168, 0, 1);
#endif //_DEFAULT_MQTT_HOST
#define DEFAULT_MQTT_CLIENT_ID    "ESP8266"
#define DEFAULT_MQTT_TOPIC        "esp_temp"

#define SLEEP_SEC         10
#define BLINK_ON          300
#define BLINK_OFF         100

// ############################################################# PROTOTYPES

bool save_config(void);
bool load_config(void);
int8_t check_config(void);

// ############################################################# TEMP SENSOR

OneWire ow(DS18B20_DATA_PIN);
DallasTemperature ds(&ow);

// ############################################################# WIFI AND MQTT CLIENT

WiFiClient wclient;
// PubSubClient mqtt(MQTT_HOST, MQTT_PORT, wclient);
PubSubClient* mqtt = NULL;
static char str_temp[10];
static char str_topic[20];
static bool wifiConnected = false;

// ############################################################# EEPROM AND CONFIG

// EEPROM (aka NonVolatileMemory)
static bool eepromInitDone = false;
#define EEPROM_CONFIG_ADDR    0x00
struct EEPROM_CONFIG {
  char        wifi_ssid[32];
  char        wifi_password[63];
  uint8_t     mqtt_host[4];
  uint16_t    mqtt_port;
  char        mqtt_client_id[23];
  char        mqtt_topic[50];     //TODO check 50 chars is enough
} config;
static bool configLoaded = false;
static bool configValid = false;

bool memall(const void* arr, uint8_t cmp, size_t size){
  const char* p = (char*)arr;

	if(size < 1)
		return false;

	for(size_t i=0;i<size;i++){
		if(*(uint8_t*)p != cmp)
			return false;
		p++;
	}
	return true;
}

// ############################################################# SERVER

const char* INDEX_HTML= "<title>tempbea-config</title>"
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
"<h1>tempbea</h1>"
"<div class='label'>Wifi SSID"
"<input name='wifi_ssid' placeholder='config.wifi_ssid'>"
"</div>"
"<div>Wifi Password"
"<input name='wifi_password' placeholder='config.wifi_password'>"
"</div>"
"<div>MQTT Host"
"<input name='mqtt_host' placeholder='config.mqtt_host'>"
"</div>"
"<div>MQTT Port"
"<input name='mqtt_port' placeholder='config.mqtt_port'>"
"</div>"
"<div>MQTT Client ID"
"<input name='mqtt_client_id' placeholder='config.mqtt_client_id'>"
"</div>"
"<div>MQTT Topic"
"<input name='mqtt_topic' placeholder='config.mqtt_topic'>"
"</div>"
"<div>"
"<input class='submit' type='submit' value='Set'>"
"</div>"
"</form>";

const char *ssid = WIFI_CONFIG_SSID;
ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void handleConfig() {

  if( server.hasArg("wifi_ssid") && server.arg("wifi_ssid") != NULL){
    Serial.print("SET config.wifi_ssid:");
    Serial.println(server.arg("wifi_ssid"));
    server.arg("wifi_ssid").toCharArray(config.wifi_ssid, sizeof(config.wifi_ssid));
  }
  if( server.hasArg("wifi_password") && server.arg("wifi_password") != NULL){
    Serial.print("SET config.wifi_password:");
    Serial.println(server.arg("wifi_password"));
    server.arg("wifi_password").toCharArray(config.wifi_password, sizeof(config.wifi_password));
  }
  if( server.hasArg("mqtt_host") && server.arg("mqtt_host") != NULL){
    Serial.print("SET config.mqtt_host:");
    Serial.println(server.arg("mqtt_host"));
    // TODO malloc should not happen inside of functions --> move them to global space ...
    char tmp_mqtt_host_str[16] = { 0 };
    uint8_t tmp_mqtt_host[4];

    server.arg("mqtt_host").toCharArray(tmp_mqtt_host_str, 16);
    if(sscanf(tmp_mqtt_host_str, "%d.%d.%d.%d", tmp_mqtt_host[0], tmp_mqtt_host[1], tmp_mqtt_host[2], tmp_mqtt_host[3]) == 4){
      memcpy(config.mqtt_host, tmp_mqtt_host, sizeof(config.mqtt_host));
    }
    //TODO maybe send 400 on else
  }
  if( server.hasArg("mqtt_port") && server.arg("mqtt_port") != NULL){
    Serial.print("SET config.mqtt_port:");
    Serial.println(server.arg("mqtt_port"));
    int tmp_mqtt_port = server.arg("mqtt_port").toInt();
    if(tmp_mqtt_port > 0){
      config.mqtt_port = tmp_mqtt_port;
    }
  }
  if( server.hasArg("mqtt_client_id") && server.arg("mqtt_client_id") != NULL){
    Serial.print("SET config.mqtt_client_id:");
    Serial.println(server.arg("mqtt_client_id"));
    server.arg("mqtt_client_id").toCharArray(config.mqtt_client_id, sizeof(config.mqtt_client_id));
  }
  if( server.hasArg("mqtt_topic") && server.arg("mqtt_topic") != NULL){
    Serial.print("SET config.mqtt_topic:");
    Serial.println(server.arg("mqtt_topic"));
    server.arg("mqtt_topic").toCharArray(config.mqtt_topic, sizeof(config.mqtt_topic));
  }

  if(save_config()){
    server.send(200, "text/html", "<p>Set Config</p>");
    ESP.restart();
  } else {
    server.send(400, "text/text", "400: Invalid Request");
  }
  //TODO check if we have to send a 400 on any request
  // server.send(400, "text/plain", "400: Invalid Request");
}

void blink(byte times){
  #if DEBUG == 1
  Serial.print("LED: ");
  #endif

  for(byte x=0; x<times; x++){
    #if DEBUG == 1
    Serial.printf("%u", 1);
    #else
    digitalWrite(LED_PIN, LOW);
    #endif
    delay(BLINK_ON);
    #if DEBUG == 1
    Serial.printf("%u", 0);
    #else
    digitalWrite(LED_PIN, HIGH);
    #endif
    delay(BLINK_OFF);
  }

  #if DEBUG == 1
  Serial.println();
  #endif
}

bool do_connect(void){
  // Stage one: check for default connection
  for(byte t = 0; t<WIFI_RETRY_COUNT; t++){
    if(WiFi.status() == WL_CONNECTED){
      Serial.printf("Connected to %s\n", config.wifi_ssid);
      return true;
    }
    Serial.printf("Trying to connect to wifi %u\n", t);
    if(t == 0){
       WiFi.begin(config.wifi_ssid, config.wifi_password);
    }
    delay(1000);
  }
  Serial.printf("Error: Unable to connect to %s\n", config.wifi_ssid);
  return false;
}

bool do_connect_mqtt(void){
  if (mqtt == NULL){
    mqtt = new PubSubClient(config.mqtt_host, config.mqtt_port, wclient);
  }
  if (!mqtt->connected()) {
    mqtt->connect(config.mqtt_client_id);
    return true;
  }
  return false;
}

float measure_temp(void){
  ds.requestTemperatures();
  // TODO maybe a delay will be needed
  //delay(750);
  return ds.getTempCByIndex(0);
}

bool publish_mqtt(float temp){
  Serial.printf(str_temp,"%f\n",temp);
  Serial.printf(str_topic,"%s\n",config.mqtt_topic);
  mqtt->publish(str_topic, str_temp);
  return true;
}

void esp_sleep(uint8_t seconds){
  Serial.printf("Going to sleep for %u\n", seconds);
#if DEBUG == 1
  delay(seconds * 1000);
#else
  ESP.deepSleep(seconds*1000000); 
#endif
}

void print_config(void){
  Serial.printf("WIFI SSID: %s\n",config.wifi_ssid);
  Serial.printf("WIFI PASSWORD: %s\n",config.wifi_password);
  Serial.printf("MQTT HOST: %u.%u.%u.%u\n", config.mqtt_host[0], config.mqtt_host[1], config.mqtt_host[2], config.mqtt_host[3]);
  Serial.printf("MQTT PORT: %u\n",config.mqtt_port);
  Serial.printf("MQTT CLIENT ID: %s\n",config.mqtt_client_id);
  Serial.printf("MQTT TOPIC: %s\n",config.mqtt_topic);
}

int8_t check_config(void){

  #if DEBUG == 1
  print_config();
  #endif

  //check if loaded config from EEPROM is valid

  if(memall(&config, 0x00, sizeof(config))){
    Serial.println("Config is all zero");
    return -1;
  }

  //check if config.wifi_ssid has a length
  if(strlen(config.wifi_ssid) < 1){
    return -1;
  }

  // check if config.wifi_ssid is all zero
  bool all_zero = true;
  for(uint8_t i = 0; i<sizeof(config.wifi_ssid); i++){
    Serial.printf("0x%02X\n", config.wifi_ssid[i]);
    if(config.wifi_ssid[i] != 0x00){
      all_zero = false;
      break;
    }
  }
  if(all_zero){
    Serial.println("All Zero");
    return -1;
  }

  if (strlen(config.wifi_password) < 1) {
    return -2;
  } 
  if (strlen(config.mqtt_client_id) < 1) {
    return -4;
  } 
  if(config.mqtt_host == (uint32_t)0x00000000) {
    return -4;
  } 
  if(config.mqtt_port == 0x0000){
    return -5; // TODO check 127.0.0.1
  }
  if(strlen(config.mqtt_topic) < 1){
    return -6;
  }

  return 1;
}

bool load_config(void){
  //init eeprom if not done before
  if(!eepromInitDone){
    EEPROM.begin(sizeof(EEPROM_CONFIG));
    eepromInitDone = true;
  }
  EEPROM.get(EEPROM_CONFIG_ADDR, config);
  configLoaded = true;

  int8_t configCheck = check_config();

  if(configCheck > 0){
    configValid = true;
    Serial.printf("Info: Config is valid\n");
  } else {
    configValid = false;
    Serial.printf("Error: Config is invalid: %d\n", configValid);
  }

  return configValid;
}

bool save_config(void){
  if(!eepromInitDone){
    Serial.printf("Error: EEPROM has to be initialized before\n");
    return false;
  }

  int8_t configCheck = check_config();
  if(configCheck > 0){
    configValid = true;
    Serial.printf("Info: Config is valid ... saving\n");
    EEPROM.put(EEPROM_CONFIG_ADDR, config);
    return EEPROM.commit();
  } else {
    configValid = false;
    Serial.printf("Error: Config is invalid: %d\n", configValid);
  }

  return false;
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

  //inital config setup
  load_config();
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

void loop(){
  
  if(configValid){
    Serial.printf("Config exists and was loaded");
  } else {
    run_setup();
  }

  // if(do_connect()){
  //   Serial.printf("run_main");

  //   blink(1);

  //   float temp = measure_temp();
  //   if(temp == -127){
  //     Serial.printf("Temperature sensor not connected on pin %u\n", DS18B20_DATA_PIN);
  //   }else{
  //     Serial.printf("Temp: %f\n", temp);

  //     bool mqtt_err = false;
  //     mqtt_err = do_connect_mqtt() | mqtt_err;
  //     mqtt_err = publish_mqtt(temp) | mqtt_err;
  //     if(mqtt_err){
  //       Serial.printf("Unable to publish data to mqtt\n");
  //       run_setup();
  //     }
  //   }

  //   blink(2);
  //   esp_sleep(SLEEP_SEC);
  //   return;
  // } else {
  //   Serial.printf("Unable to connect to wifi\n");
  //   blink(5);
  //   run_setup();
  // }
}
