#ifndef _TEMPEACONFIG_H_
#define _TEMPEACONFIG_H_

#include <stdint.h>
#include <string.h>

#include <HardwareSerial.h>
#include <ESP_EEPROM.h>

#include "../utils/utils.h"

#define EEPROM_CONFIG_ADDR    0x00

#ifndef _PARAMS_VALIDATION
#define _PARAMS_VALIDATION
  static const char INVALID_SSID_START_CHARS[] = "!#;";
  static const char INVALID_SSID_CHARS[] = "+]\"\t";

  #define INVALID_MQTT_HOSTS_SIZE 3
  static const uint8_t INVALID_MQTT_HOSTS[3][4] = {{0, 0, 0, 0}, {127, 0, 0, 1}, {255, 255, 255, 255}};
  static const char VALID_CLIENT_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  // TODO recheck topic with a spec
  static const char VALID_TOPIC_ALPHABET[] = "#/+ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
#endif


#define WIFI_SSID_LEN 32
#define WIFI_PASSWORD_LEN 63
#define MQTT_HOST_LEN 4
#define MQTT_CLIENT_ID_LEN 23
#define MQTT_TOPIC_LEN 50

#pragma pack(1)
typedef struct {            // EEPROM CONFIG (aka NonVolatileMemory)
      char        wifi_ssid[WIFI_SSID_LEN];
      char        wifi_password[WIFI_PASSWORD_LEN];
      uint8_t     mqtt_host[MQTT_HOST_LEN];
      char        mqtt_client_id[MQTT_CLIENT_ID_LEN];
      char        mqtt_topic[MQTT_TOPIC_LEN];     //TODO check 50 chars is enough
      uint16_t    mqtt_port;
} eeprom_config;

#define EEPROM_CONFIG_SIZE 174

// Configuration of the Application
class TempeaConfig {
    bool initialized;
    bool valid;
    bool loaded;
    int address;
    eeprom_config config;
  public:
    TempeaConfig(int eeprom_addr);
    ~TempeaConfig();
    bool load();
    bool save();
    bool reset();
    void print();
    bool validate();
    bool validate_ssid();
    bool validate_password();
    bool validate_host();
    bool validate_port();
    bool validate_clientid();
    bool validate_topic();
    eeprom_config* get();
};

#endif