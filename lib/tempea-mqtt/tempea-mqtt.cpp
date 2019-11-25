#include "tempea-mqtt.h"

MqttClient::MqttClient(WiFiClient *wc, TempeaConfig* c){
  wifiClient = wc;
  config = c;
  mqttClient = new PubSubClient(config->get()->mqtt_host, config->get()->mqtt_port, *wifiClient);
}

MqttClient::~MqttClient(){}

bool MqttClient::connect(){
  if (!mqttClient->connected()) {
    return mqttClient->connect(config->get()->mqtt_client_id);
  }
  return true;
}

bool MqttClient::publish(char *msg){
  return mqttClient->publish(config->get()->mqtt_topic, msg);
}