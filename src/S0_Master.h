#pragma

#include "Arduino.h"

class S0_Master
{
  public:
    S0_Master();

    bool initS0(uint8_t pinInt);

    void set_impulseProKwh(uint16_t value);
    uint16_t get_impulseProKwh();

    void set_ImpulseCounted(uint16_t value);
    uint16_t get_ImpulseCounted();

    void set_TimeStopp(uint32_t value);

  private:
    static S0_Master* instance;
    static void interrupt_S0();
    void isr();
    

    uint16_t _impulseProKwh;
    uint16_t _impulseCounted = 0;

    uint32_t _timeStart = 0;
    uint32_t _timeStopp = 0;
};