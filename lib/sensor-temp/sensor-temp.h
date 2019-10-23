#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <OneWire.h>
#include <DallasTemperature.h>

#define DALLAS_TEMP_BY_C_INDEX  0

class TempSensor {
    OneWire* oneWire;
    DallasTemperature* dallasTemperature;
  public:
    TempSensor(uint8_t ds18b20_pin);
    ~TempSensor();
    float getTempC();
};

#endif