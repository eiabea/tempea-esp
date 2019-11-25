#ifndef _TEMPEAMQTT_H_
#define _TEMPEAMQTT_H_

#include <PubSubClient.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#include "tempea-config.h"

class MqttClient{
    TempeaConfig* config;
    WiFiClient* wifiClient;
    PubSubClient* mqttClient;
  public:
    MqttClient(WiFiClient *wc, TempeaConfig* c);
    ~MqttClient();
    bool connect();
    bool publish(char *msg);
};

#endif