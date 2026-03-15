#pragma once

// Mat's first version of Hardware autoconfig
#define HW_BREAKOUT 8
#define HW_20 7
#define HW_21 6

uint8_t get_HW_ID();
void initHW(uint8_t hwID);
uint8_t get_PROG_LED_PIN(uint8_t hwID);
uint8_t get_PROG_LED_PIN_ACTIVE_ON(uint8_t hwID);
uint8_t get_PROG_BUTTON_PIN(uint8_t hwID);
uint8_t get_SAVE_INTERRUPT_PIN(uint8_t hwID);
