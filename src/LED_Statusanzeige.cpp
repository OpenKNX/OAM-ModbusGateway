#include "LED_Statusanzeige.h"

PCF8575 pcf8575_LED_CH1_16(i2cAddr_LED, &Wire);

uint16_t state_LED_Out = 0xFFFF;

void initI2cStatusLeds()
{
    // LED I2C Inputs
    for (int i = 0; i < I2C_Expander_CH; i++)
    {
        pcf8575_LED_CH1_16.pinMode(i, OUTPUT);
    }
    pcf8575_LED_CH1_16.pcf8575_Clear();
}

void setLED_ON()
{
    pcf8575_LED_CH1_16.pcf8575_WriteALL(0x0000);
}

void setLED_OFF()
{
    pcf8575_LED_CH1_16.pcf8575_WriteALL(0xFFFF);
}

void setLED(uint8_t pin, bool state)
{
    if(state == true) // LED ON --> PIN = LOW
        state_LED_Out &=  ~(1 << pin);  
    if(state == false) // LED OFF --> PIN = HIGH
        state_LED_Out |=  1 << pin;      

  pcf8575_LED_CH1_16.pcf8575_WriteALL(state_LED_Out);
}

void blink_S0_LED(uint8_t ch)
{

}
