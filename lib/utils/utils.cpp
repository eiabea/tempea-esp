#include "utils.h"

#include <Arduino.h>
#include "../../src/settings.h"

bool memall(const void* arr, uint8_t cmp, size_t size){
  if(size < 1)
		return false;
  const char* p = (char*)arr;
	for(size_t i=0;i<size;i++){
		if(*(uint8_t*)p != cmp)
			return false;
		p++;
	}
	return true;
}

void blink(byte times){
#if DEBUG == 1
  Serial.print("LED: ");
#endif

  for(byte x=0; x<times; x++){
#if DEBUG == 1
    Serial.printf("%u", 1);
    digitalWrite(LED_PIN, LOW);
#endif
    delay(BLINK_ON);
#if DEBUG == 1
    Serial.printf("%u", 0);
    digitalWrite(LED_PIN, HIGH);
#endif
    delay(BLINK_OFF);
  }

#if DEBUG == 1
  Serial.println();
#endif
}

bool stralpha(const void* arr, const void* alpha){
  if(arr == NULL || alpha == NULL)
    return false;
  const char* p_arr = (char*)arr;
  const char* p_alpha = (char*)alpha;
  size_t s_arr = strlen(p_arr);
  size_t s_alpha = strlen(p_alpha);
  if(s_arr < 1 || s_alpha < 1)
		return false;
  for(size_t i=0;i<s_arr;i++){
    bool match = false;
    p_alpha = (char*)alpha;
    for(size_t j=0;j<s_alpha;j++){
      if(*p_arr == *p_alpha){
        match = true;
        break;
      }
      p_alpha++;
    }
    if(!match)
      return false;
		p_arr++;
	}
  return true;
}