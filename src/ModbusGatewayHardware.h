#pragma once

//Anzahl an S0 INPUTS
#define MAX_S0_CHANNELS 2

//DIAG LED
#define Diag_LED 9

//Gehäuse Anzeige
#define i2cAddr_LED   0x24
#define MODBUS_STATUS 5
#define MODBUS_ERROR  4
#define MBUS_STATUS   3
#define MBUS_ERROR    2
#define S01_LED       1
#define S02_LED       0
#define LED_ON_TIME_S0 150

//Gehäuse Anzeige
#define  i2cAddr_LED   0x24

//HW-ID
#define ID1 23
#define ID2 24
#define ID3 22

// Modbus 
#define MaxCountSlaves 10
/*!
  We're using a MAX485-compatible RS485 Transceiver.
  The Data Enable and Receiver Enable pins are hooked up as follows:
*/
#define MAX485_DE 12


// S0
#define S0_CH1 2
#define S0_CH2 5

// Virtuelle Zähler
#define maxMeters 4





