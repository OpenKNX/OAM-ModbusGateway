#pragma once

#include "hardware.h"
#include "PCF8575.h"

#define I2C_Expander_CH 16

void initI2cStatusLeds();
void setLED_ON();
void setLED_OFF();
void setLED(uint8_t pin, bool state);
void blink_S0_LED(uint8_t ch);
