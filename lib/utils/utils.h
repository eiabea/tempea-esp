#ifndef _UTILS_H_
#define _UTILS_H_

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

bool memall(const void* arr, uint8_t cmp, size_t size);
void blink(byte times);
bool stralpha(const void* arr, const void* alpha);

#endif