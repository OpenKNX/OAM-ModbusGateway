#pragma once

#include <knx.h>

#define zaehlerElek     1
#define zaehlerWasser   2
#define zaehlerGas      3
#define zaehlerSonstig  4

#define unit_l  0
#define unit_m3 1

void Process_S0(uint8_t channel);

uint16_t setZaehlerImpulse(uint8_t i, uint16_t impulse);

void functionS01();
void functionS02();

void sendZaehlerStand(int i, uint16_t S0_Zaehler[], uint16_t S0_Zaehler_old[]);
void sendZaehlerStand_2(int i, uint16_t S0_Zaehler[], uint16_t S0_Zaehler_old[]);
