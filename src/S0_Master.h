#pragma

#include "Arduino.h"

#define S0_CH1 0
#define S0_CH2 1
#define S0_CH3 2
#define S0_Ch4 3

#define zaehlerElek     1
#define zaehlerWasser   2
#define zaehlerGas      3
#define zaehlerSonstig  4


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
    static S0_Master* instance;
    static void interrupt_S0();
    void isr();
    
    bool _newImpulse = false; 

    uint8_t _ledPin;
    uint8_t _channel;

    uint16_t _impulseProKwh;        
    uint16_t _impulseCounted = 0;
    uint16_t _meterValue = 0;

    uint32_t _timeStart = 0;
    uint32_t _timeStopp = 0;
    uint32_t _time_S0_LED_Blink = 0;

    float _currentConsumption = 0;
};