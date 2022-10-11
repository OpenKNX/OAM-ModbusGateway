#include "S0_Master.h"
#include "Arduino.h"
#include "LED_Statusanzeige.h"

S0_Master::S0_Master()
{
    instance = this;
};

bool S0_Master::initS0(uint8_t pinInt, uint8_t ledPin)
{
    _ledPin = ledPin;

    uint8_t irq = digitalPinToInterrupt(pinInt);
    if (irq != NOT_AN_INTERRUPT)
    {
        pinMode(pinInt, INPUT);
        attachInterrupt(irq, S0_Master::interrupt_S0, FALLING);
        return true;
    }
    else
    {
        return false;
    }
}

void S0_Master::isr()
{
    _timeStopp = millis();
    _impulseCounted++;
    _newImpulse = true;

#ifdef Serial_Int_output
    SERIAL_DEBUG.println("+");
#endif
}

void S0_Master::interrupt_S0()
{
    instance->isr();
}

void S0_Master::set_ImpulseCounted(uint16_t value)
{
    _impulseCounted = value;
}

uint16_t S0_Master::get_ImpulseCounted()
{
    return _impulseCounted;
}

void S0_Master::set_TimeStopp(uint32_t value)
{
    _timeStopp = value;
}

void S0_Master::process(uint8_t channel)
{
    // Neuer Impulse detektiert
    if (_newImpulse == true)
    {
        setLED(_ledPin, HIGH);                 
#ifdef Debug_S0_LED
        digitalWrite(Diag_LED, true);
#endif
        _time_S0_LED_Blink = millis();

#ifdef Serial_Debug_S0
        SERIAL_DEBUG.print("S0_");
        SERIAL_DEBUG.print(channel + 1);
#endif
    }
}
