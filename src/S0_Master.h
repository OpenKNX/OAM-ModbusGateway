#pragma

#include "Arduino.h"

#define zaehlerElek 1
#define zaehlerWasser 2
#define zaehlerGas 3
#define zaehlerSonstig 4

class S0_Master
{
  public:
    S0_Master();

    bool initS0(uint8_t pinInt, uint8_t ledPin, uint8_t channel);
    void process(uint8_t channel);

    void set_impulseProKwh(uint16_t value);
    void set_ImpulseCounted(uint16_t value);

    uint16_t get_impulseProKwh();
    uint16_t get_ImpulseCounted();
    float getCurrentConsumption();

    void set_TimeStopp(uint32_t value);

  private:
    static S0_Master* _instance;

    
    static void interrupt_S0();
    void isr();

    bool _newImpulse = false;
    bool _statusLedOn = false;

    uint8_t _pinInt;
    uint8_t _ledPin;
    uint8_t _channel;

    uint16_t _impulseProKwh;
    uint16_t _impulseCounted = 0;
    uint16_t _impulseCounted_ProMin = 0;
    uint16_t _impulseCounted_ProMin_stopp = 0;
    uint16_t _meterValue = 0;

    uint32_t _timeStart = 0;
    uint32_t _timeStopp = 0;
    uint32_t _time_S0_LED_Blink = 0;
    uint32_t _timer_ProMin = 0;

    float _currentConsumption = 0;
};