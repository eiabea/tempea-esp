#include "tempea-config.h"
#include "../../src/settings.h"

TempeaConfig::TempeaConfig(int eeprom_addr){
  initialized = false;
  valid = false;
  loaded = false;
  address = eeprom_addr;

  if(!initialized){
    EEPROM.begin(sizeof(eeprom_config));
    initialized = true;
  }
}

TempeaConfig::~TempeaConfig(){
  //TODO change 
  EEPROM.end();
}

bool TempeaConfig::load(){
  //init eeprom if not done before
  if(!initialized){
    EEPROM.begin(sizeof(eeprom_config));
    initialized = true;
  }
  EEPROM.get(address, config);
  loaded = true;

  if(validate()){
    valid = true;
    Serial.printf("Info: TempeaConfig is valid\n");
    return valid;
  } 
  
  valid = false;
  Serial.printf("Error: TempeaConfig is invalid\n");

  return valid;
}

bool TempeaConfig::save(){
  if(!initialized){
    Serial.printf("Error: EEPROM has to be initialized before\n");
    return false;
  }

  if(validate()){
    valid = true;
    Serial.printf("Info: TempeaConfig is valid ... saving\n");
    EEPROM.put(address, config);
    return EEPROM.commit();
  } 

  valid = false;
  Serial.printf("Error: TempeaConfig is invalid");

  return valid;
}

bool TempeaConfig::reset(){
  if(!initialized){
    Serial.printf("Error: EEPROM has to be initialized before\n");
    return false;
  }

  memset(&config, 0x00, sizeof(eeprom_config));

  valid = false;
  Serial.printf("Info: TempeaConfig reset ... saving\n");
  EEPROM.put(address, config);
  return EEPROM.commit();
}

// ########################################## VALIDATION

bool TempeaConfig::validate_ssid(){
  // zero
  if(memall(&config.wifi_ssid, 0x00, sizeof(config))){
#if DEBUG == 1
    Serial.println("config.wifi_ssid zero failed");
#endif
    return false;
  }

  // strlen
  size_t s = strlen(config.wifi_ssid);
  if(s < 1){
#if DEBUG == 1
    Serial.println("config.wifi_ssid strlen failed");
#endif
    return false;
  }

  // startchar
  if(strpbrk(config.wifi_ssid, INVALID_SSID_START_CHARS) == config.wifi_ssid){
#if DEBUG == 1
    Serial.printf("config.wifi_ssid startchar failed (%s)\n", config.wifi_ssid);
#endif
    return false;
  }

  // inavlidchar
  if(strpbrk(config.wifi_ssid, INVALID_SSID_CHARS) != NULL){
#if DEBUG == 1
    Serial.printf("config.wifi_ssid invlidchar failed (%s)\n", config.wifi_ssid);
#endif
    return false;
  }  

  return true;  
}

bool TempeaConfig::validate_password(){
  // zero
  if(memall(&config.wifi_password, 0x00, sizeof(config))){
#if DEBUG == 1
    Serial.printf("config.wifi_password zero failed (%s)\n", config.wifi_password);
#endif
    return false;
  }

  // strlen
  size_t s = strlen(config.wifi_password);
  if(s < 1){
#if DEBUG == 1
    Serial.println("config.wifi_password strlen failed (%s)");
#endif
    return false;
  }

  return true;  
}

bool TempeaConfig::validate_host(){
  // invalid IP
  for(uint8_t i=0; i<INVALID_MQTT_HOSTS_SIZE; i++){
    if(memcmp(config.mqtt_host, INVALID_MQTT_HOSTS[i], MQTT_HOST_LEN) == 0){
#if DEBUG == 1
      Serial.printf("config.mqtt_host invalid IP failed %d\n", i);
#endif
      return false;
    }
  }

  return true;  
}

bool TempeaConfig::validate_port(){
  // zero
  if(config.mqtt_port == 0x0000){
#if DEBUG == 1
    Serial.println("config.mqtt_port zero failed");
#endif
    return false;
  }

  return true;
}

bool TempeaConfig::validate_clientid(){
  // zero
  if(memall(&config.mqtt_client_id, 0x00, sizeof(config))){
#if DEBUG == 1
    Serial.printf("config.mqtt_client_id zero failed (%s)\n", config.mqtt_client_id);
#endif
    return false;
  }

  // strlen
  size_t s = strlen(config.mqtt_client_id);
  if(s < 1){
#if DEBUG == 1
    Serial.printf("config.mqtt_client_id strlen failed (%s)\n", config.mqtt_client_id);
#endif
    return false;
  }

  // alphabet
  if(!stralpha(&config.mqtt_client_id, &VALID_CLIENT_ALPHABET)){
#if DEBUG == 1
    Serial.printf("config.mqtt_client_id alphabet failed (%s)\n", config.mqtt_client_id);
#endif
    return false;
  }

  return true;  
}

bool TempeaConfig::validate_topic(){
  // zero
  if(memall(&config.mqtt_topic, 0x00, sizeof(config))){
#if DEBUG == 1
    Serial.println("config.mqtt_topic zero failed");
#endif
    return false;
  }

    // strlen
  size_t s = strlen(config.mqtt_topic);
  if(s < 1){
#if DEBUG == 1
    Serial.println("config.mqtt_topic strlen failed");
#endif
    return false;
  }

  // alphabet
  if(!stralpha(&config.mqtt_topic, &VALID_TOPIC_ALPHABET)){
#if DEBUG == 1
    Serial.printf("config.mqtt_topic alphabet failed (%s)\n", config.mqtt_topic);
#endif
    return false;
  }

  return true;  
}

bool TempeaConfig::validate(){
#if DEBUG == 1
  Serial.println("Validate Config");
  print();
#endif

  // allzero
  if(memall(&config, 0x00, sizeof(config))){
    Serial.println("TempeaConfig is all zero");
    return false;
  }

  return validate_ssid() & 
    validate_password() &
    validate_host() &
    validate_port() &
    validate_clientid() &
    validate_topic();
}

void TempeaConfig::print(){
  Serial.printf("WIFI SSID: %s\n",                config.wifi_ssid);
  Serial.printf("WIFI PASSWORD: %s\n",            config.wifi_password);
  Serial.printf("MQTT HOST: %u.%u.%u.%u\n",       config.mqtt_host[0], config.mqtt_host[1], config.mqtt_host[2], config.mqtt_host[3]);
  Serial.printf("MQTT PORT: %u\n",                config.mqtt_port);
  Serial.printf("MQTT CLIENT ID: %s\n",           config.mqtt_client_id);
  Serial.printf("MQTT TOPIC: %s\n",               config.mqtt_topic);
}

eeprom_config* TempeaConfig::get(){
  return &config;
}