#include <Arduino.h>
#include <unity.h>

#include "tempea-config.h"

static TempeaConfig config(EEPROM_CONFIG_ADDR);

// expected Variables

// valids

static eeprom_config expected_validConfig = {
   "0123456789012345678901234567890",
   "01234567890123456789012345678901234567890123456789012345678901",
   { 0xff, 0xff, 0xff, 0xff },
   12345,
   "0123456789012345678901",
   "0123456789012345678901234567890123456789012345678"
};

// invalids

static eeprom_config expected_invalidConfig = {
   "",
   "",
   { 0x00, 0x00, 0x00, 0x00 },
   0,
   "",
   ""
};

static char *invalid_wifi_ssid_zero = "";
static char *invalid_wifi_ssid_startchar_0= "!123456789012345678901234567890";
static char *invalid_wifi_ssid_startchar_1= "#123456789012345678901234567890";
static char *invalid_wifi_ssid_startchar_2= ";123456789012345678901234567890";
static char *invalid_wifi_ssid_inavlidchar_0= "jsdlfjllejklejfw  JLSJKLenfkj/";
static char *invalid_wifi_ssid_inavlidchar_1= "jsdlfjllejklej]  JLSJKLenfkj";
static char *invalid_wifi_ssid_inavlidchar_2= "jsdlfjl+jklejfw + JLSJKLenfkj";
static char *invalid_wifi_ssid_inavlidchar_3= "jsdlfjllej\"ejfw  JLSJKLenfkj";
static char *invalid_wifi_ssid_inavlidchar_4= "jsdlfjllejklej\tfw  JLSJKLenfkj";

static char *invalid_wifi_password_zero = "";

static uint8_t invalid_mqtt_host_zero[] = {0, 0, 0, 0};
static uint8_t invalid_mqtt_host_0[] = {127, 0, 0, 1};
static uint8_t invalid_mqtt_host_1[] = {255, 255, 255, 255};

static uint16_t invalid_mqtt_port_zero = 0x0000;

static char *valid_mqtt_clientid_alphabet_0= "0123456789012345678901";
static char *valid_mqtt_clientid_alphabet_1= "ABCDEFGHIJKLMNOPQRSTUV";
static char *valid_mqtt_clientid_alphabet_2= "WXYZabcdefghijklmnopqr";
static char *valid_mqtt_clientid_alphabet_3= "stuvwxyz0328";
static char *invalid_mqtt_clientid_zero = "";
static char *invalid_mqtt_clientid_alphabet_1= "kflduiennkl388/373#";

static char *valid_mqtt_topic_alphabet_0= "#/+ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrst";
static char *valid_mqtt_topic_alphabet_1= "uvwxyz0123456789";
static char *invalid_mqtt_topic_zero = "";
static char *invalid_mqtt_topic_alphabet_1= "JKLDSFOIEKLF/3dfklef/djklfjel7**djfkje/dkfle}";



// void setUp(void) {
// // set stuff up here
// }

void tearDown(void) {
  // clean stuff up here
  // config.reset();
}

void set_valid_config(){
  eeprom_config* conf = config.get();
  
  //copy data
  memcpy(conf, &expected_validConfig, sizeof(eeprom_config));

  //save to EEPROM
  config.save();
}

void test_invalid_zero_config(){
  config.reset();

  config.load();

  bool valid = config.validate();

  TEST_ASSERT_EQUAL(valid, false);
}

void test_all_zero (){
  set_valid_config();
  config.load();
  eeprom_config* conf = config.get();
  memset(conf->wifi_ssid, 0x00, sizeof(char)* WIFI_SSID_LEN);
  memcpy(conf->wifi_password, 0x00, sizeof(char) * WIFI_PASSWORD_LEN);
  memcpy(conf->mqtt_host, 0x00, sizeof(char)* MQTT_HOST_LEN);
  conf->mqtt_port = 0x0000;
  memcpy(conf->mqtt_client_id, 0x00, sizeof(char)* MQTT_CLIENT_ID_LEN);
  memcpy(conf->mqtt_topic, 0x00, sizeof(char)* MQTT_TOPIC_LEN);
  bool valid = config.validate();
  TEST_ASSERT_EQUAL(valid, false);
}

void test_invalid_wifi_ssid_zero (){
  config.load();
  eeprom_config* conf = config.get();
  memset(conf->wifi_ssid, 0x00, sizeof(char)* WIFI_SSID_LEN);
  memcpy(conf->wifi_ssid, invalid_wifi_ssid_zero, sizeof(invalid_wifi_ssid_zero));
  bool valid = config.validate();
  TEST_ASSERT_EQUAL(valid, false);
}

void test_invalid_wifi_ssid_startchar_0 (){
  config.load();
  eeprom_config* conf = config.get();
  memset(conf->wifi_ssid, 0x00, sizeof(char)* WIFI_SSID_LEN);
  memcpy(conf->wifi_ssid, invalid_wifi_ssid_startchar_0, sizeof(invalid_wifi_ssid_startchar_0));
  bool valid = config.validate();
  TEST_ASSERT_EQUAL(valid, false);
}

void test_invalid_wifi_ssid_startchar_1 (){
  config.load();
  eeprom_config* conf = config.get();
  memset(conf->wifi_ssid, 0x00, sizeof(char)* WIFI_SSID_LEN);
  memcpy(conf->wifi_ssid, invalid_wifi_ssid_startchar_1, sizeof(invalid_wifi_ssid_startchar_1));
  bool valid = config.validate();
  TEST_ASSERT_EQUAL(valid, false);
}

void test_invalid_wifi_ssid_startchar_2 (){
  config.load();
  eeprom_config* conf = config.get();
  memset(conf->wifi_ssid, 0x00, sizeof(char)* WIFI_SSID_LEN);
  memcpy(conf->wifi_ssid, invalid_wifi_ssid_startchar_2, sizeof(invalid_wifi_ssid_startchar_2));
  bool valid = config.validate();
  TEST_ASSERT_EQUAL(valid, false);
}

void test_invalid_wifi_ssid_inavlidchar_0 (){
  config.load();
  eeprom_config* conf = config.get();
  memset(conf->wifi_ssid, 0x00, sizeof(char)* WIFI_SSID_LEN);
  memcpy(conf->wifi_ssid, invalid_wifi_ssid_inavlidchar_0, sizeof(invalid_wifi_ssid_inavlidchar_0));
  bool valid = config.validate();
  TEST_ASSERT_EQUAL(valid, false);
}

void test_invalid_wifi_ssid_inavlidchar_1 (){
  config.load();
  eeprom_config* conf = config.get();
  memset(conf->wifi_ssid, 0x00, sizeof(char)* WIFI_SSID_LEN);
  memcpy(conf->wifi_ssid, invalid_wifi_ssid_inavlidchar_1, sizeof(invalid_wifi_ssid_inavlidchar_1));
  bool valid = config.validate();
  TEST_ASSERT_EQUAL(valid, false);
}

void test_invalid_wifi_ssid_inavlidchar_2 (){
  config.load();
  eeprom_config* conf = config.get();
  memset(conf->wifi_ssid, 0x00, sizeof(char)* WIFI_SSID_LEN);
  memcpy(conf->wifi_ssid, invalid_wifi_ssid_inavlidchar_2, sizeof(invalid_wifi_ssid_inavlidchar_2));
  bool valid = config.validate();
  TEST_ASSERT_EQUAL(valid, false);
}

void test_invalid_wifi_ssid_inavlidchar_3 (){
  config.load();
  eeprom_config* conf = config.get();
  memset(conf->wifi_ssid, 0x00, sizeof(char)* WIFI_SSID_LEN);
  memcpy(conf->wifi_ssid, invalid_wifi_ssid_inavlidchar_3, sizeof(invalid_wifi_ssid_inavlidchar_3));
  bool valid = config.validate();
  TEST_ASSERT_EQUAL(valid, false);
}

void test_invalid_wifi_ssid_inavlidchar_4 (){
  config.load();
  eeprom_config* conf = config.get();
  memset(conf->wifi_ssid, 0x00, sizeof(char)* WIFI_SSID_LEN);
  memcpy(conf->wifi_ssid, invalid_wifi_ssid_inavlidchar_4, sizeof(invalid_wifi_ssid_inavlidchar_4));
  bool valid = config.validate();
  TEST_ASSERT_EQUAL(valid, false);
}

void test_invalid_wifi_password_zero (){
  config.load();
  eeprom_config* conf = config.get();
  memset(conf->wifi_password, 0x00, sizeof(char)* WIFI_PASSWORD_LEN);
  memcpy(conf->wifi_password, invalid_wifi_password_zero, sizeof(invalid_wifi_password_zero));
  bool valid = config.validate();
  TEST_ASSERT_EQUAL(valid, false);
}

void test_invalid_mqtt_host_zero (){
  config.load();
  eeprom_config* conf = config.get();
  memset(conf->mqtt_host, 0x00, sizeof(char)* MQTT_HOST_LEN);
  memcpy(conf->mqtt_host, invalid_mqtt_host_zero, sizeof(conf->mqtt_host));
  bool valid = config.validate();
  TEST_ASSERT_EQUAL(valid, false);
}

void test_valid_config(){
  eeprom_config* conf = config.get();
  
  //copy data
  memcpy(conf, &expected_validConfig, sizeof(eeprom_config));

  config.print();

  //save to EEPROM
  config.save();
  
  config.load();
  eeprom_config* loaded_conf = config.get();

  int compared = memcmp(loaded_conf, &expected_validConfig, sizeof(eeprom_config));

  TEST_ASSERT_EQUAL(compared, 0);
}

void setup(){
  config.load();
}

void loop(){
  UNITY_BEGIN();

  RUN_TEST(test_invalid_zero_config);

  RUN_TEST(test_valid_config);

  UNITY_END();
}
