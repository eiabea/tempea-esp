#include "sensor-temp.h"

TempSensor::TempSensor(uint8_t ds18b20_pin){
  oneWire = new OneWire(ds18b20_pin);
  dallasTemperature = new DallasTemperature(oneWire);
}

TempSensor::~TempSensor(){}

float TempSensor::getTempC(){
  float temp;
  int timeout = 30; // 3 s timeout in 100ms delays

  do{
    dallasTemperature->requestTemperatures(); 
    temp = dallasTemperature->getTempCByIndex(DALLAS_TEMP_BY_C_INDEX);
    delay(100);
    timeout--;
    if(timeout < 0){
      return -273.15; // zero degree Kelvin if an error occured
    } 
  } while (temp == -127.0);

  return temp;
}