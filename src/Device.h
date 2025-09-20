#pragma once
#include "Helper.h"
#include "Schedule.h"

// Mat's first version of Hardware autoconfig
#define HW_BREAKOUT 8
#define HW_20       7
#define HW_21       6
#define HW_1TE      5

void ProcessHeartbeat();

uint8_t get_HW_ID();
void initHW(uint8_t hwID);
uint8_t get_PROG_LED_PIN(uint8_t hwID);
uint8_t get_PROG_LED_PIN_ACTIVE_ON(uint8_t hwID);
uint8_t get_PROG_BUTTON_PIN(uint8_t hwID);
uint8_t get_SAVE_INTERRUPT_PIN(uint8_t hwID);
uint16_t getPar(uint16_t PAR, uint8_t CH);
uint16_t getCom(uint16_t COM, uint8_t CH);
