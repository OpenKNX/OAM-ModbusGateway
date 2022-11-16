#include "HelperFunc.h"
#include <Arduino.h>
#include "ModbusGateway.h"

// bool delayCheck(uint32_t iOldTimer, uint32_t iDuration)
// {
//     return millis() - iOldTimer >= iDuration;
// }

uint16_t getParBIN(uint16_t PAR, uint8_t CH)
{
    return  BIN_ParamBlockOffset + (CH * BIN_ParamBlockSize) + PAR;
}

uint16_t getComBIN(uint16_t COM, uint8_t CH)
{
    return  BIN_KoOffset + (CH * BIN_KoBlockSize) + COM;
}
