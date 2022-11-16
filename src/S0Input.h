#include <Arduino.h>

#define S0_CH1 0
#define S0_CH2 1
#define S0_CH3 2
#define S0_Ch4 3

#define typeInative 0
#define typeBIN     1
#define typeS0      2
#define typeImpl    3

#define zaehlerElek     1
#define zaehlerWasser   2
#define zaehlerGas      3
#define zaehlerSonstig  4

#define unit_l   0
#define unit_m3  1

class S0Input
{
  private:
    bool _newImpulse = false;
    bool _statusLedOn = false;
    bool _lsend_MeterValue = false;
    bool _lsend_currentConsumption = false;
    bool _det_maxPuls = false;
    bool _det_TwoPulses = false;

    uint8_t _pinInt;
    uint8_t _ledPin;
    uint8_t _channel;
    //uint8_t _lsendMode;

    uint16_t _impulseProKwh;
    uint16_t _impulseCounted = 0;
    uint16_t _impulseCounted_ProMin = 0;
    uint16_t _impulseCounted_ProMin_stopp = 0;
    uint16_t _meterValue = 0;
    uint16_t _meterValue_old = 0;
    

    uint32_t _time_S0_start = 0;
    uint32_t _time_S0_stopp = 0;
    uint32_t _time_S0_LED_Blink = 0;
    uint32_t _timer_ProMin = 0;
    uint32_t _maxPulsLength = 0;
    uint32_t _timer_minSendDelay = 0;
    uint32_t _timer_SendDelay = 0;
    uint32_t _timer_minSendDelay_con = 0;
    uint32_t _timer_SendDelay_con = 0;

    float _currentConsumption = 0;
    float _currentConsumption_old = 0;

    static void callbackDispatcher(void *iInstance);
    void pinInterruptHandler();

#ifndef ARDUINO_ARCH_RP2040
    static uint8_t sCallbackCounter;
    static void *sInstance[10];
    static void callback0();
    static void callback1();
    static void callback2();
    static void callback3();
    static void callback4();
    static void callback5();
    static void callback6();
    static void callback7();
    static void callback8();
    static void callback9();
#endif

  public:
    S0Input(/* args */);
    ~S0Input();
    void setup(uint8_t Pin, uint8_t LedPin, uint8_t channel);
    void attachInterruptHandlerMethod(pin_size_t iPin, PinStatus iMode);
    void process();

    void set_impulseProKwh(uint16_t impulseProKwh);
    void set_maxPulsLength(uint16_t minValueCalc);
};
