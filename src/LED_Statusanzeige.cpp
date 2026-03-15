#include "LED_Statusanzeige.h"
#include "Device.h"
#include "hardware.h"

#define PCF_8575 1
#define TCA_9555 2

#define PCF_Mod_LED 3
#define PCF_Err_LED 4
#define TCA_Mod_LED 0
#define TCA_Err_LED 1

PCF8575 pcf8575_LED_CH1_16(0x24, &Wire);
TCA9555 tca9555_LED_CH1_16(0x22, &Wire);

uint16_t state_LED_Out = 0xFFFF;

uint8_t ioExp = 0;

void initI2cStatusLeds()
{
    // CHeck with Io-Expander is used
    if (pcf8575_LED_CH1_16.isConnected())
    {
        SERIAL_DEBUG.println("Status LED IO-EXP: PCF8575");
        // LED I2C Inputs
        for (int i = 0; i < I2C_Expander_CH; i++)
        {
            pcf8575_LED_CH1_16.pinMode(i, OUTPUT);
        }
        pcf8575_LED_CH1_16.pcf8575_Clear();
        ioExp = PCF_8575;
    }
    if (tca9555_LED_CH1_16.isConnected())
    {
        SERIAL_DEBUG.println("Status LED IO-EXP: TCA95555");
        // LED I2C Inputs
        for (int i = 0; i < I2C_Expander_CH; i++)
        {
            tca9555_LED_CH1_16.pinMode1(i, OUTPUT);
        }
        tca9555_LED_CH1_16.write16(0xFFFF);
        ioExp = TCA_9555;
    }
}

void setLED_ON()
{
    switch (ioExp)
    {
    case PCF_8575:
        pcf8575_LED_CH1_16.pcf8575_WriteALL(0x0000);
        break;
    case TCA_9555:
        tca9555_LED_CH1_16.write16(0x0000);
        break;

    default:
        SERIAL_DEBUG.println("Status Anzeige. IO-Exp not defined");
        break;
    }
}

void setLED_OFF()
{
    switch (ioExp)
    {
    case PCF_8575:
        pcf8575_LED_CH1_16.pcf8575_WriteALL(0xFFFF);
        break;
    case TCA_9555:
        tca9555_LED_CH1_16.write16(0xFFFF);
        break;

    default:
        SERIAL_DEBUG.println("Status Anzeige. IO-Exp not defined");
        break;
    }
}

void setLED(uint8_t pin, bool state)
{
    switch (ioExp)
    {
    case PCF_8575:
        if (state == true) // LED ON --> PIN = LOW
            state_LED_Out &= ~(1 << pin);
        if (state == false) // LED OFF --> PIN = HIGH
            state_LED_Out |= 1 << pin;
        pcf8575_LED_CH1_16.pcf8575_WriteALL(state_LED_Out);
        break;
    case TCA_9555:
        tca9555_LED_CH1_16.write1(!pin, state);
        break;

    default:
        SERIAL_DEBUG.println("Status Anzeige. IO-Exp not defined");
        break;
    }
}

void setLED_ERROR(bool state)
{
#ifdef ARDUINO_ARCH_RP2040
    switch (ioExp)
    {
    case PCF_8575:
        if (state == true) // LED ON --> PIN = LOW
            state_LED_Out &= ~(1 << PCF_Err_LED);
        if (state == false) // LED OFF --> PIN = HIGH
            state_LED_Out |= 1 << PCF_Err_LED;
        pcf8575_LED_CH1_16.pcf8575_WriteALL(state_LED_Out);
        break;
    case TCA_9555:
        tca9555_LED_CH1_16.write1(TCA_Err_LED, !state);
        break;

    default:
        SERIAL_DEBUG.println("Status Anzeige. IO-Exp not defined");
        break;
    }
#endif
}

void setLED_Modbus(bool state)
{
    switch (ioExp)
    {
    case PCF_8575:
        if (state == true) // LED ON --> PIN = LOW
            state_LED_Out &= ~(1 << PCF_Mod_LED);
        if (state == false) // LED OFF --> PIN = HIGH
            state_LED_Out |= 1 << PCF_Mod_LED;
        pcf8575_LED_CH1_16.pcf8575_WriteALL(state_LED_Out);
        break;
    case TCA_9555:
        tca9555_LED_CH1_16.write1(TCA_Mod_LED, !state);
        break;

    default:
        SERIAL_DEBUG.println("Status Anzeige. IO-Exp not defined");
        break;
    }
}

void blink_S0_LED1(uint8_t ch)
{
}

void blink_S0_LED12(uint8_t ch)
{
}
