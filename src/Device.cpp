#include <Arduino.h>

#include "Device.h"
#include "LED_Statusanzeige.h"
#include "HardwareConfig.h"

uint8_t get_PROG_LED_PIN(uint8_t hwID)
{
#ifdef ARDUINO_ARCH_RP2040
    switch (hwID)
    {
    case HW_21: // HW 2.1
        return 11;
    case HW_20: // HW 2.0
        return 1;
    case HW_BREAKOUT: // Breakout-Board
        return 22;
    default:
        return 255;
    }
#endif
}

uint8_t get_PROG_BUTTON_PIN(uint8_t hwID)
{
#ifdef ARDUINO_ARCH_RP2040
    switch (hwID)
    {
    case HW_21: // HW 2.1
        return 14;
    case HW_20: // HW 2.0
        return 3;
    case HW_BREAKOUT: // Breakout-Board
        return 28;
    default:
        return 255;
    }
#endif
}

uint8_t get_PROG_LED_PIN_ACTIVE_ON(uint8_t hwID)
{
    return HIGH;
}

uint8_t get_SAVE_INTERRUPT_PIN(uint8_t hwID)
{
#ifdef ARDUINO_ARCH_RP2040
    switch (hwID)
    {
    case HW_21: // HW 2.1
        return 8;
    case HW_20: // HW 2.0
        return 20;
    case HW_BREAKOUT: // Breakout-Board
        return 6;
    default:
        return 255;
    }
#endif
}

uint8_t get_HW_ID()
{
    uint8_t hw_ID = 0;
#ifdef BOARD_MASIFI_MODBUS_BREAKOUT
    hw_ID = HW_BREAKOUT;
#else
    // Set Inputs
    pinMode(SMARTMF_HARDWARE_REVISION_ID1_PIN, INPUT_PULLUP);
    pinMode(SMARTMF_HARDWARE_REVISION_ID2_PIN, INPUT_PULLUP);
    pinMode(SMARTMF_HARDWARE_REVISION_ID3_PIN, INPUT_PULLUP);
    // read Inputs
    bitWrite(hw_ID, 0, digitalRead(SMARTMF_HARDWARE_REVISION_ID1_PIN));
    bitWrite(hw_ID, 1, digitalRead(SMARTMF_HARDWARE_REVISION_ID2_PIN));
    bitWrite(hw_ID, 2, digitalRead(SMARTMF_HARDWARE_REVISION_ID3_PIN));
#endif
    return hw_ID;
}

void initHW(uint8_t hwID)
{
#ifdef ARDUINO_ARCH_RP2040
    switch (hwID)
    {
    case HW_BREAKOUT:
        SERIAL_DEBUG.println("HW_ID: BREAKOUT-BOARD");
        break;
    case HW_20:
        SERIAL_DEBUG.println("HW_ID: V2.0");
        break;
    case HW_21:
        SERIAL_DEBUG.println("HW_ID: V2.1");
        break;
    default:
        SERIAL_DEBUG.print("HW_ID: ERROR ");
        SERIAL_DEBUG.println(hwID);
        break;
    }
#endif
    initI2cStatusLeds();
    setLED_OFF();
    // setLED_Modbus(LOW);
    // setLED_ERROR(LOW);
}
