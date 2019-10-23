#include "sensor-temp.h"

TempSensor::TempSensor(uint8_t ds18b20_pin){
  oneWire = new OneWire(ds18b20_pin);
  dallasTemperature = new DallasTemperature(oneWire);
}

TempSensor::~TempSensor(){}

float TempSensor::getTempC(){
  dallasTemperature->requestTemperatures();
  // TODO maybe a delay will be needed
  // delay(750);
  return dallasTemperature->getTempCByIndex(DALLAS_TEMP_BY_C_INDEX);
}