#include "S0_Master.h"
#include "Arduino.h"

S0_Master::S0_Master(){
  instance = this;
};


bool S0_Master::initS0(uint8_t pinInt)
{
    uint8_t irq = digitalPinToInterrupt(pinInt); 
    if (irq != NOT_AN_INTERRUPT)
    {
        pinMode(pinInt, INPUT);
        attachInterrupt(irq,S0_Master::interrupt_S0, FALLING);
        return true;
    }
    else
    {
        return false;
    }
}
 void S0_Master::isr() {
      _timeStopp = millis();
      SERIAL_DEBUG.println("+");
    }

 void S0_Master::interrupt_S0() {
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

