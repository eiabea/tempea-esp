#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdint.h>

#define DEBUG             0
#define WIFI_SSID         "AwesomeWifiName"
#define WIFI_PASSWORD     "password"
#define LED_PIN           2
#define DS18B20_DATA_PIN  5
#ifndef _MQTT_HOST
  #define _MQTT_HOST
  const static IPAddress MQTT_HOST(172, 16, 0, 2);
#endif //_MQTT_HOST
#define MQTT_PORT         1883
#define MQTT_CLIENT_ID    "ESP8266"
#define MQTT_TOPIC        "esp_temp"
#define SLEEP_SEC         60
#define BLINK_ON          300
#define BLINK_OFF         100

// printf function wrapper
#ifndef _PRINTF_
#define _PRINTF_
  #define _PRINTF_BUFFER_LENGTH_ 100
  static char _printf_buffer_[_PRINTF_BUFFER_LENGTH_];
  #define printf(format, ...)  {                                      \
      snprintf(_printf_buffer_, sizeof(_printf_buffer_), format, ##__VA_ARGS__);  \
      Serial.print(_printf_buffer_);                                              \
    }
#endif

OneWire ow(DS18B20_DATA_PIN);
DallasTemperature ds(&ow);

WiFiClient wclient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, wclient);
static char str_temp[10];
static char str_topic[20];

void blink(byte times){
  for(byte x=0; x<times; x++){
    digitalWrite(LED_PIN, LOW);
    delay(BLINK_OFF);
    digitalWrite(LED_PIN, HIGH);
    delay(BLINK_ON);
  }
}

bool do_connect(void){
  // Stage one: check for default connection
  for(byte t = 0; t<30; t++){
    if(WiFi.status() == WL_CONNECTED){
      printf("Connected to %s", WIFI_SSID);
      return true;
    }
    printf("Trying to connect to wifi %u", t);
    if(t == 0){
       WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    delay(1000);
  }
  printf("Error: Unable to connect to %s", WIFI_SSID);
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
  sprintf(str_temp,"%f",temp);
  sprintf(str_topic,"%s",MQTT_TOPIC);
  mqtt.publish(str_topic, str_temp);
  return true;
}

void esp_sleep(uint8_t seconds){
  printf("Going to sleep for %u", seconds);
  ESP.deepSleep(seconds*1000000); 
}

void setup(){
  Serial.begin(9600);
#if DEBUG == 1
  printf("%s", ESP.getResetReason());
#endif
}

void loop(){
  
  if(do_connect()){
    blink(1);

    float temp = measure_temp();
    printf("Temp: %f", temp);

    bool mqtt_err = false;
    mqtt_err = do_connect_mqtt() | mqtt_err;
    mqtt_err = publish_mqtt(temp) | mqtt_err;
    if(mqtt_err){
      printf("Unable to publish data to mqtt");
    }
    blink(2);
  } else {
    printf("Unable to connect to wifi");
  }

  esp_sleep(SLEEP_SEC);
}
