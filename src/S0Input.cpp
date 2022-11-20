#include "S0Input.h"
#include "knxprod.h"
#include "Helper.h"
#include "HelperFunc.h"
#include "KnxHelper.h"
#include "LED_Statusanzeige.h"
#include "knx.h"

#ifndef ARDUINO_ARCH_RP2040
uint8_t S0Input::sCallbackCounter = 1;
void *S0Input::sInstance[10] = {nullptr};
#endif

S0Input::S0Input(/* args */)
{
}

S0Input::~S0Input()
{
}

// das ist der Ersatz für attachInterrupt
void S0Input::attachInterruptHandlerMethod(pin_size_t iPin, PinStatus iMode)
{
#ifdef ARDUINO_ARCH_RP2040
    attachInterruptParam(iPin, callbackDispatcher, iMode, this);
#else
    sInstance[sCallbackCounter] = this;
    switch (sCallbackCounter)
    {
        case 0:
            attachInterrupt(iPin, callback0, iMode);
            break;
        case 1:
            attachInterrupt(iPin, callback1, iMode);
            break;
        case 2:
            attachInterrupt(iPin, callback2, iMode);
            break;
        case 3:
            attachInterrupt(iPin, callback3, iMode);
            break;
        case 4:
            attachInterrupt(iPin, callback4, iMode);
            break;
        case 5:
            attachInterrupt(iPin, callback5, iMode);
            break;
        case 6:
            attachInterrupt(iPin, callback6, iMode);
            break;
        case 7:
            attachInterrupt(iPin, callback7, iMode);
            break;
        case 8:
            attachInterrupt(iPin, callback8, iMode);
            break;
        case 9:
            attachInterrupt(iPin, callback9, iMode);
            break;

        default:
            break;
    }
    sCallbackCounter++;
#endif
}

// diese Methode (statisch) empfängt alle callbacks und leitet die an die jeweilige Instanz weiter
void S0Input::callbackDispatcher(void *iInstance)
{
    S0Input *self = static_cast<S0Input *>(iInstance);
    self->pinInterruptHandler();
}

#ifndef ARDUINO_ARCH_RP2040
void S0Input::callback0()
{
    S0Input::callbackDispatcher(sInstance[0]);
}

void S0Input::callback1()
{
    S0Input::callbackDispatcher(sInstance[1]);
}

void S0Input::callback2()
{
    S0Input::callbackDispatcher(sInstance[2]);
}

void S0Input::callback3()
{
    S0Input::callbackDispatcher(sInstance[3]);
}

void S0Input::callback4()
{
    S0Input::callbackDispatcher(sInstance[4]);
}

void S0Input::callback5()
{
    S0Input::callbackDispatcher(sInstance[5]);
}

void S0Input::callback6()
{
    S0Input::callbackDispatcher(sInstance[6]);
}

void S0Input::callback7()
{
    S0Input::callbackDispatcher(sInstance[7]);
}

void S0Input::callback8()
{
    S0Input::callbackDispatcher(sInstance[8]);
}

void S0Input::callback9()
{
    S0Input::callbackDispatcher(sInstance[9]);
}
#endif

// das ist die Methode, die den interrupt behandelt
void S0Input::pinInterruptHandler()
{
    // hier kommt das Coding für den Interrupthandler hin
    _time_S0_start = millis();
    _impulseCounted++;
    _newImpulse = true;
}

// im setup oder im constructor, auf jeden Fall IN der Klasse und nicht außerhalb,
// muss der neue attach stehen
void S0Input::setup(uint8_t Pin, uint8_t LedPin, uint8_t channel)
{
    _pinInt = Pin;
    _ledPin = LedPin;
    _channel = channel;

    // Set Timer
    _timer_ProMin = millis();
    _time_S0_start = millis();

    // hier kannst Du pro Instanz ein attach machen
    pinMode(_pinInt, INPUT);
    attachInterruptHandlerMethod(digitalPinToInterrupt(_pinInt), FALLING);
}

void S0Input::set_impulseProKwh(uint16_t impulseProKwh)
{
    _impulseProKwh = impulseProKwh;
}

//----------------- Mom Verbrauch: Mindestleistung/Durchfluss - Berechnung = 0(W/l/m3) -------------------
void S0Input::set_maxPulsLength(uint16_t minValueCalc)
{
    // Berechnung max Pulsdauer für Mindestleistung/durchfluss
    // Dauer = 3600sek * Impulse / Mindestleistung
    _maxPulsLength = 3600 * _impulseProKwh / minValueCalc;
#ifdef Serial_Debug_S0
    SERIAL_DEBUG.print("maxPulsLength_CH");
    SERIAL_DEBUG.print(_channel + 1);
    SERIAL_DEBUG.print(": ");
    SERIAL_DEBUG.println(_maxPulsLength);
#endif
}

void S0Input::process()
{
    uint8_t lsendMode = knx.paramByte(getParBIN(BIN_S0SendModeCounter, _channel));
    uint8_t lsendMode_con = knx.paramByte(getParBIN(BIN_S0SendModeCon, _channel));

    // Definiert LED-Blickdauer
    if (delayCheck(_time_S0_LED_Blink, 100))
    {
        setLED(_ledPin, LOW);
    }

    // Impulse pro Minute Zähler
    if (delayCheck(_timer_ProMin, 60000))
    {
        _timer_ProMin = millis();
        knx.getGroupObject(getComBIN(BIN_KoS0_AnzahlImpulse, _channel)).value(_impulseCounted_ProMin, getDPT(VAL_DPT_7));
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.print("Impl/min_CH");
        SERIAL_DEBUG.print(_channel + 1);
        SERIAL_DEBUG.print(": ");
        SERIAL_DEBUG.println(_impulseCounted_ProMin);
#endif
        _impulseCounted_ProMin = 0;
    }

    // prüft ob Pulslänge zu lange (Mindestleistung)
    if (delayCheck(_time_S0_start, _maxPulsLength))
    {
        _det_maxPuls = true;
        _currentConsumption = 0;
        //_lsend_currentConsumption = true;
    }
    else
    {
        _det_maxPuls = false;
    }

    // we waited enough, let's send the value
    if ((lsendMode == 2 || lsendMode == 3) && delayCheck(_timer_SendDelay, knx.paramWord(getParBIN(BIN_S0SendDelay, _channel)) * 1000))
    {
        // if (startinit)
        // {
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.print("zykl. send Zähler CH");
        SERIAL_DEBUG.println(_channel + 1);
#endif
        _lsend_MeterValue = true;
        // }
    }
    else if ((lsendMode_con == 2 || lsendMode_con == 3) && delayCheck(_timer_SendDelay_con, knx.paramWord(getParBIN(BIN_S0SendDelay, _channel)) * 1000))
    {
        // if (startinit)
        // {
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.print("zykl. send Verbrauch CH");
        SERIAL_DEBUG.println(_channel + 1);
#endif
        _lsend_currentConsumption = true;
        // }
    }

    // Process wenn neuer Impuls detektiert wurde
    if (_newImpulse)
    {
        _newImpulse = false;

#ifdef Serial_Debug_S0
        SERIAL_DEBUG.print("Puls_CH");
        SERIAL_DEBUG.print(_channel + 1);
        SERIAL_DEBUG.print(": ");
        SERIAL_DEBUG.println(_time_S0_start - _time_S0_stopp);
#endif

        // Prüft ob zwei Impulse empfangen wurden. Erst danach kann der Mom Verbauch ermittelt werden
        if (_impulseCounted >= 2)
        {
            _det_TwoPulses = true;
        }

        // Zählt Impulse pro Minute
        _impulseCounted_ProMin++;

        // LED Anzeige
        setLED(_ledPin, HIGH);
        _time_S0_LED_Blink = millis();

        //-------------------------------------- Zähler ----------------------------------------------------------
        if (_impulseCounted >= _impulseProKwh)
        {
            // increase Meter Value +1
            _meterValue++;

            // senden bei Wertänderung
            if ((lsendMode == 1 || lsendMode == 3) && _meterValue - _meterValue_old >= knx.paramWord(getParBIN(BIN_S0SendminValuechange, _channel)))
            {
                if (delayCheck(_timer_minSendDelay, knx.paramWord(getParBIN(BIN_S0SendminValueDelay, _channel)) * 1000))
                {
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.print("Änd. send Zähler CH");
                    SERIAL_DEBUG.println(_channel + 1);
#endif
                    _lsend_MeterValue = true;
                }
            }

            // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
            knx.getGroupObject(getComBIN(BIN_KoS0_Ges_Verbrauch, _channel)).valueNoSend(_meterValue, getDPT(VAL_DPT_13)); // MOD_KoS01_ZaehlerWert+channel da KO nur 1 Byte auseinander liegen

#ifdef Serial_Debug_S0
            SERIAL_DEBUG.print("neuer Zählerstand_CH");
            SERIAL_DEBUG.print(_channel + 1);
            SERIAL_DEBUG.print(": ");
            SERIAL_DEBUG.println(_meterValue);
#endif
            _impulseCounted = 0;
        }
        //-------------------------------------- Zähler ENDE -----------------------------------------------------

        //-------------------------------------- Mom Verbrauch ---------------------------------------------------
        if (!_det_maxPuls && _det_TwoPulses) // nur berechnen wenn Pulslänge kleiner max Pulslänge Mindestleistung/durchfluss && Zwei Impulse empfangen wurden
        {

            switch (knx.paramByte(getParBIN(BIN_S0DefineZaehler, _channel)))
            {
                // Zähler Elektrisch
                case zaehlerElek:
                    // calculation mom Verbrauch (W)
                    _currentConsumption = 3600.0 / ((float)(_time_S0_start - _time_S0_stopp) /(float) _impulseProKwh);
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.print("Mom(W) CH");
                    SERIAL_DEBUG.print(_channel + 1);
                    SERIAL_DEBUG.print(": ");
#endif
                    break;
                // Zähler Wasser
                case zaehlerWasser:
                    switch (knx.paramByte(getParBIN(BIN_S0DefineUnit, _channel)))
                    {
                        case unit_l:
#ifdef Serial_Debug_S0
                            SERIAL_DEBUG.print(" Mom(l/h) CH");
                            SERIAL_DEBUG.print(_channel + 1);
                            SERIAL_DEBUG.print(": ");
#endif
                            // calculation mom Verbrauch (l)
                            _currentConsumption = 1000 / ((_time_S0_start - _time_S0_stopp) / 1000.0 / (3600.0 / _impulseProKwh));
                            break;
                        case unit_m3:
#ifdef Serial_Debug_S0
                            SERIAL_DEBUG.print(" Mom(m3/s) CH");
                            SERIAL_DEBUG.print(_channel + 1);
                            SERIAL_DEBUG.print(": ");
#endif
                            // calculation mom Verbrauch (m3)
                            _currentConsumption = 1 / ((_time_S0_start - _time_S0_stopp) / (_impulseProKwh * 1000.0));
                            break;
                        default:
#ifdef Serial_Debug_S0
                            SERIAL_DEBUG.println("Wrong Zähler-Unit");
#endif
                            return;
                            break;
                    }
                    break;
                // Zähler Gas
                case zaehlerGas:
                    switch (knx.paramByte(getParBIN(BIN_S0DefineUnit, _channel)))
                    {
                        case unit_l:
#ifdef Serial_Debug_S0
                            SERIAL_DEBUG.print(" Mom(l/h) CH");
                            SERIAL_DEBUG.print(_channel + 1);
                            SERIAL_DEBUG.print(": ");
#endif
                            // calculation mom Verbrauch (l)
                            _currentConsumption = 1000 / ((_time_S0_start - _time_S0_stopp) / 1000.0 / (3600.0 / _impulseProKwh));
                            break;
                        case unit_m3:
#ifdef Serial_Debug_S0
                            SERIAL_DEBUG.print(" Mom(m3/s) CH");
                            SERIAL_DEBUG.print(_channel + 1);
                            SERIAL_DEBUG.print(": ");
#endif
                            // calculation mom Verbrauch (m3)
                            _currentConsumption = 1 / ((_time_S0_start - _time_S0_stopp) / (_impulseProKwh * 1000.0));
                            break;
                        default:
#ifdef Serial_Debug_S0
                            SERIAL_DEBUG.println("Wrong Zähler-Unit");
#endif
                            return;
                            break;
                    }
                    break;
                case zaehlerSonstig:
                    break;
                default:
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.println("Wrong Zähler-Typ");
#endif
                    return;
                    break;
            }

#ifdef Serial_Debug_S0
            SERIAL_DEBUG.println(_currentConsumption);
#endif

            // senden bei Wertänderung
            if ((lsendMode_con == 1 || lsendMode_con == 3) && abs(_currentConsumption - _currentConsumption_old) >= knx.paramWord(getParBIN(BIN_S0SendminValuechangeCon, _channel)))
            {
                if (delayCheck(_timer_minSendDelay_con, knx.paramWord(getParBIN(BIN_S0SendminValueDelayCon, _channel)) * 1000))
                {
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.print("Änd. send Zähler CH");
                    SERIAL_DEBUG.println(_channel + 1);
                    SERIAL_DEBUG.println(_currentConsumption);
                    SERIAL_DEBUG.println(_currentConsumption_old);
#endif
                    _lsend_currentConsumption = true;
                }
            }

            float _currentConsumption_unit = 0;
            switch (knx.paramByte(getParBIN(BIN_S0DefineZaehler, _channel)))
            {
                case zaehlerElek:
                    // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                    knx.getGroupObject(getComBIN(BIN_KoS0_Akt1_Verbrauch, _channel)).valueNoSend(_currentConsumption, getDPT(VAL_DPT_14)); // MOD_KoS01_ZaehlerWert+channel da KO nur 1 Byte auseinander liegen                                                                                                                 // Umrechnung in KW
                    _currentConsumption_unit= _currentConsumption / 1000.0;
                    knx.getGroupObject(getComBIN(BIN_KoS0_Akt2_Verbrauch, _channel)).valueNoSend(_currentConsumption_unit, getDPT(VAL_DPT_9)); // MOD_KoS01_ZaehlerWert+channel da KO nur 1 Byte auseinander liegen
                    break;
                case zaehlerWasser:
                    switch (knx.paramByte(getParBIN(BIN_S0DefineUnit, _channel)))
                    {
                        case unit_l:
                            // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                            knx.getGroupObject(getComBIN(BIN_KoS0_Akt1_Verbrauch, _channel)).valueNoSend(_currentConsumption, getDPT(VAL_DPT_9)); // l/h
                            // umrechnung in m3/s
                            //_currentConsumption = _currentConsumption / 60000.0;
                            knx.getGroupObject(getComBIN(BIN_KoS0_Akt2_Verbrauch, _channel)).valueNoSend(_currentConsumption, getDPT(VAL_DPT_14)); // m3/s
                            break;
                        case unit_m3:
                            // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                            knx.getGroupObject(getComBIN(BIN_KoS0_Akt1_Verbrauch, _channel)).valueNoSend(_currentConsumption, getDPT(VAL_DPT_14)); // m3/s
                            // umrechnung in l/h
                            _currentConsumption = _currentConsumption * 60000.0;
                            knx.getGroupObject(getComBIN(BIN_KoS0_Akt2_Verbrauch, _channel)).valueNoSend(_currentConsumption, getDPT(VAL_DPT_9)); // l/h
                            break;
                            break;
                        default:
                            break;
                    }
                    break;

                case zaehlerGas:
                    switch (knx.paramByte(getParBIN(BIN_S0DefineUnit, _channel)))
                    {
                        case unit_l:
                            // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                            knx.getGroupObject(getComBIN(BIN_KoS0_Akt1_Verbrauch, _channel)).valueNoSend(_currentConsumption, getDPT(VAL_DPT_9)); // l/h
                            // umrechnung in m3/s
                            _currentConsumption = _currentConsumption / 60000.0;
                            knx.getGroupObject(getComBIN(BIN_KoS0_Akt2_Verbrauch, _channel)).valueNoSend(_currentConsumption, getDPT(VAL_DPT_14)); // m3/s
                            break;
                        case unit_m3:
                            // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                            knx.getGroupObject(getComBIN(BIN_KoS0_Akt1_Verbrauch, _channel)).valueNoSend(_currentConsumption, getDPT(VAL_DPT_14)); // m3/s
                            // umrechnung in l/h
                            _currentConsumption = _currentConsumption * 60000.0;
                            knx.getGroupObject(getComBIN(BIN_KoS0_Akt2_Verbrauch, _channel)).valueNoSend(_currentConsumption, getDPT(VAL_DPT_9)); // l/h
                            break;
                            break;
                        default:
                            break;
                    }
                    break;

                default:
                    break;
            }

            // Speichert letzten Timer-Wert
            _time_S0_stopp = _time_S0_start;

        } // if(!_det_maxPuls && _det_TwoPulses)
    }

    // Send Zählerwerte
    if (_lsend_MeterValue)
    {
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.print("Send Zähler_CH");
        SERIAL_DEBUG.println(_channel + 1);
#endif
        // send KNX
        knx.getGroupObject(getComBIN(BIN_KoS0_Ges_Verbrauch, _channel)).objectWritten();
        // speichert letzten Zählerwert
        _meterValue_old = _meterValue;

        _timer_SendDelay = millis();    // Timer für Zyklisches Senden
        _timer_minSendDelay = millis(); // Timer für Mindestintervall (bei Wertänderung)

        _lsend_MeterValue = false;
    }
    else if (_lsend_currentConsumption)
    {
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.print("Send Verbrauch_CH");
        SERIAL_DEBUG.println(_channel + 1);
#endif
        // send KNX
        knx.getGroupObject(getComBIN(BIN_KoS0_Akt1_Verbrauch, _channel)).objectWritten(); // KW oder m3/s
        knx.getGroupObject(getComBIN(BIN_KoS0_Akt2_Verbrauch, _channel)).objectWritten(); // W oder l/h
        // speichert letzten Verbrauchswert
        _currentConsumption_old = _currentConsumption;

        _timer_SendDelay_con = millis();    // Timer für Zyklisches Senden
        _timer_minSendDelay_con = millis(); // Timer für Mindestintervall (bei Wertänderung)

        _lsend_currentConsumption = false;
    }
}