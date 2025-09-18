#pragma once

#include "hardware.h"
#include "PCF8575.h"
#include "TCA9555.h"

#define I2C_Expander_CH 16

void initI2cStatusLeds();
void setLED_ON();
void setLED_OFF();
void setLED(uint8_t pin, bool state);
void setLED_ERROR(bool state);
void setLED_Modbus(bool state);
void blink_S0_LED1(uint8_t ch);
void blink_S0_LED2(uint8_t ch);
