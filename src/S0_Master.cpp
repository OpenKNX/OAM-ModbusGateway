#include "S0_Master.h"
#include "Arduino.h"
#include "knxprod.h"
#include "Helper.h"
#include "HelperFunc.h"
#include "KnxHelper.h"
#include "LED_Statusanzeige.h"
#include <knx.h>

S0_Master *S0_Master::_instance = NULL;

S0_Master::S0_Master(){
    // instance = this;
};

bool S0_Master::initS0(uint8_t pinInt, uint8_t ledPin, uint8_t channel)
{
    _instance = this;

    _ledPin = ledPin;
    _channel = channel;

    _timer_ProMin = millis();

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
    //instance->isr();
}

void S0_Master::set_ImpulseCounted(uint16_t value)
{
    _impulseCounted = value;
}

void S0_Master::set_impulseProKwh(uint16_t value)
{
    _impulseProKwh = value;
}

uint16_t S0_Master::get_ImpulseCounted()
{
    return _impulseCounted;
}

uint16_t S0_Master::get_impulseProKwh()
{
    return _impulseProKwh;
}

float S0_Master::getCurrentConsumption()
{
    return _currentConsumption;
}

void S0_Master::process(uint8_t channel)
{
    // ************************************************
    // LED Blinkfunktion
    if (delayCheck(_time_S0_LED_Blink, 100) && _statusLedOn)
    {
        setLED(_ledPin, LOW);
        _statusLedOn = false;
#ifdef Debug_S0_LED
        digitalWrite(Diag_LED, false);
#endif
        SERIAL_DEBUG.println("-");
    }

    // ************************************************
    // Timer 1min
    if (delayCheck(_timer_ProMin, 10000))
    {
        _timer_ProMin = millis();

        knx.getGroupObject(getComBIN(BIN_KoS0_AnzahlImpulse, channel)).value(_impulseCounted_ProMin, getDPT(VAL_DPT_7));

#ifdef Serial_Debug_S0
        SERIAL_DEBUG.print(channel);
        SERIAL_DEBUG.print("_Pulse Pro Min: ");
        SERIAL_DEBUG.println(_impulseCounted_ProMin);
#endif
        _impulseCounted_ProMin = 0;
    }

    /***************************************************
    * Neuer Impulse detektiert
    ****************************************************/
    if (_newImpulse == true)
    {
        _newImpulse = false;

        // Anzahl Impulse pro Minute
        _impulseCounted_ProMin++;
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.print("Pulse: ");
        SERIAL_DEBUG.println(_impulseCounted_ProMin);
#endif

        setLED(_ledPin, HIGH);
        _statusLedOn = true;
#ifdef Debug_S0_LED
        digitalWrite(Diag_LED, true);
#endif
        _time_S0_LED_Blink = millis();

#ifdef Serial_Debug_S0
        SERIAL_DEBUG.print("S0_");
        SERIAL_DEBUG.println(channel + 1);
#endif

        //-------------------------------------- Zähler ----------------------------------------------------------
        if (_impulseCounted >= _impulseProKwh)
        {
            // Zählerstand um eins erhöhen
            _meterValue++;
            // Gezählte Impulse wieder auf Null setzen
            _impulseCounted = 0;
        }
        //-------------------------------------- Zähler ENDE -----------------------------------------------------

        //-------------------------------------- Mom Verbrauch: --------------------------------------------------

        // Berechnung max Pulsdauer für Mindestleistung/durchfluss
        // Dauer = 3600sek * Impulse / Mindestleistung
        switch (knx.paramByte(getParBIN(BIN_S0DefineZaehler, channel)))
        {
            case zaehlerElek:
                _currentConsumption = 3600.0 / ((_timeStopp - _timeStart) / (_impulseProKwh * 1.0));
                break;
        }
        //-------------------------------------- Mom Verbrauch ENDE -----------------------------------------------
    }
}
