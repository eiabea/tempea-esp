#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdint.h>

#define DEBUG             1
#define WIFI_SSID         "AwesomeWifiName"
#define WIFI_PASSWORD     "password"
#define WIFI_RETRY_COUNT  30
#define LED_PIN           1
#define DS18B20_DATA_PIN  5
#ifndef _MQTT_HOST
  #define _MQTT_HOST
  const static IPAddress MQTT_HOST(172, 16, 0, 2);
#endif //_MQTT_HOST
#define MQTT_PORT         1883
#define MQTT_CLIENT_ID    "ESP8266"
#define MQTT_TOPIC        "esp_temp"
#define SLEEP_SEC         10
#define BLINK_ON          300
#define BLINK_OFF         100

// printf function wrapper
// #ifndef _PRINTF_
// #define _PRINTF_
//   #define _PRINTF_BUFFER_LENGTH_ 100
//   static char _printf_buffer_[_PRINTF_BUFFER_LENGTH_];
//   #define printf(format, ...)  {                                      
//       snprintf(_printf_buffer_, sizeof(_printf_buffer_), format, ##__VA_ARGS__);  
//       Serial.print(_printf_buffer_);                                              
//     }
// #endif

OneWire ow(DS18B20_DATA_PIN);
DallasTemperature ds(&ow);

WiFiClient wclient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, wclient);
static char str_temp[10];
static char str_topic[20];

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
      Serial.printf("Connected to %s\n", WIFI_SSID);
      return true;
    }
    Serial.printf("Trying to connect to wifi %u\n", t);
    if(t == 0){
       WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    delay(1000);
  }
  Serial.printf("Error: Unable to connect to %s\n", WIFI_SSID);
  return false;
}

bool do_connect_mqtt(void){
  if (!mqtt.connected()) {
    mqtt.connect(MQTT_CLIENT_ID);
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
  Serial.printf(str_topic,"%s\n",MQTT_TOPIC);
  mqtt.publish(str_topic, str_temp);
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

void setup(){
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
}

void loop(){
  
  if(do_connect()){
    blink(1);

    float temp = measure_temp();
    if(temp == -127){
      Serial.printf("Temperature sensor not connected on pin %u\n", DS18B20_DATA_PIN);
    }else{
      Serial.printf("Temp: %f\n", temp);

      bool mqtt_err = false;
      mqtt_err = do_connect_mqtt() | mqtt_err;
      mqtt_err = publish_mqtt(temp) | mqtt_err;
      if(mqtt_err){
        Serial.printf("Unable to publish data to mqtt\n");
      }
    }

    blink(2);
  } else {
    Serial.printf("Unable to connect to wifi\n");
    blink(5);
  }

  esp_sleep(SLEEP_SEC);
}
