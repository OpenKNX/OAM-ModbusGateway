#include <Arduino.h>
#include <knx.h>

#include "ModbusGateway.h"
#include "KnxHelper.h"
#include "S0Function.h"
#include "hardware.h"
#include "Device.h"

//-------- Test only --------------------
//#define Serial_Debug_S0_Int
unsigned long time_S0_LED_Blink[2] = {0};

typedef enum
{
    cycle = 1,
    calc_Zaehler = 2,
    set_Mom_0 = 3,
    set_KO_value_Zaehler = 4,
    set_KO_value_Verbrauch = 5,
    send_KO = 6,
} stateMaschine;

stateMaschine State[2] = {cycle, cycle};

//-------- S0 variable -------------------
bool lSendZaehlerWert_S0[2] = {false};
bool lSendMomLeistung_S0[2] = {false};
bool impuls_S0[2] = {0};
bool startinit = false;
uint32_t sendDelay_S0[2] = {0};
uint32_t minSendDelay_S0[2] = {0};
uint32_t sendDelayCon_S0[2] = {0};
uint32_t minSendDelayCon_S0[2] = {0};
float mom_S0[2] = {1};
float mom_S0_old[2] = {10};

uint16_t S0_Zaehler[2] = {1, 1};
uint16_t S0_Zaehler_old[2] = {0, 0};
uint16_t S0_impuls[2];
unsigned long time_S0_start[2] = {0};
unsigned long time_S0_stopp[2] = {0};
uint16_t zaehler_Impulse[2] = {5, 5};
uint32_t abs_S0[2] = {0};

//bool S01_impuls = false;
//bool S02_impuls = false;

void Process_S0(uint8_t channel)
{
    float mom_S0[channel];
    uint32_t maxPulsLength;

    uint32_t lCycle;
    uint8_t lsendMode;
    uint8_t lsendModeCon;
    bool det_maxPuls = false;
    ;

    /*
    //only test
    if (delayCheck(time_S0_LED_Blink[1], 100))
    {
        digitalWrite(Diag_LED, false);
    }
    //only test ENDE

    switch (State[channel])
    {
    case cycle:

        //----------------- Mom Verbrauch: Mindestleistung/durchfluss - Berechnung = 0(W/l/m3) -------------------
        // Berechnung max Pulsdauer für Mindestleistung/durchfluss
        // Dauer = 3600sek * Impulse / Mindestleistung
        maxPulsLength = 3600 * knx.paramWord(MOD_S01Impulse + channel) / knx.paramWord(MOD_DefineMinValueS01 + (2*channel));
        #ifdef Serial_Debug_S0
            SERIAL_DEBUG.println(maxPulsLength);
            SERIAL_DEBUG.println(time_S0_start[channel]);
            SERIAL_DEBUG.println(knx.paramWord(MOD_S01Impulse + channel));
            SERIAL_DEBUG.println(knx.paramWord(MOD_DefineMinValueS01 + (2*channel)));
#endif
        // prüfen ob aktueller puls länger ist als Pulslänge min Leistung oder Durchfluss
        if (startinit && delayCheck(time_S0_start[channel], maxPulsLength))
        {
#ifdef Serial_Debug_S0
           // SERIAL_DEBUG.print("Max Puls ");
#endif
            //State[channel] = set_Mom_0;
            //return;
        }

        lsendMode = knx.paramByte(MOD_S01SendModeCounter + channel);
        // we waited enough, let's send the value
        if ((lsendMode == 2 || lsendMode == 3) && delayCheck(sendDelay_S0[channel], knx.paramInt(MOD_SendDelayS01 + (4 * channel)) * 1000)) //S01 und S02 liegen 4 Bytes auseinander
        {
            if (startinit)
            {
#ifdef Serial_Debug_S0
                SERIAL_DEBUG.println("zykl. send Zähler");
#endif
                lSendZaehlerWert_S0[channel] = true;
                State[channel] = send_KO;
                return;
            }
        }

        lsendModeCon = knx.paramByte(MOD_S01SendModeCon + channel);
        // we waited enough, let's send the value
        if (startinit && (lsendModeCon == 2 || lsendModeCon == 3) && delayCheck(sendDelayCon_S0[channel], knx.paramInt(MOD_SendDelayConS01 + (4 * channel)) * 1000)) //S01 und S02 liegen 4 Bytes auseinander
        {
#ifdef Serial_Debug_S0
            SERIAL_DEBUG.println("zykl. send Verbrauch");
#endif
            lSendMomLeistung_S0[channel] = true;
            State[channel] = send_KO;
            return;
        }

        if (impuls_S0[channel] == true)
        {
            impuls_S0[channel] = false;

            //Init bis erster Wert eingelesen wurde
            if (time_S0_start[channel] != 0 && !startinit)
            {
                startinit = true;
                State[channel] = send_KO;
#ifdef Serial_Debug_S0
                SERIAL_DEBUG.println("init Done");
#endif
                return;
            }

            //only test
            digitalWrite(Diag_LED, true);
            time_S0_LED_Blink[1] = millis();
            //only test ENDE

#ifdef Serial_Debug_S0
            SERIAL_DEBUG.print("S0_");
            SERIAL_DEBUG.print(channel + 1);
            SERIAL_DEBUG.print(" ");
            SERIAL_DEBUG.print((time_S0_stopp[channel] - time_S0_start[channel]));
#endif

            //-------------------------------------- Zähler ----------------------------------------------------------
            S0_impuls[channel]++;
            if (S0_impuls[channel] >= zaehler_Impulse[channel])
            {
                S0_impuls[channel] = 0;
                S0_Zaehler[channel]++;

#ifdef Serial_Debug_S0
                SERIAL_DEBUG.print(" Zähler: ");
                SERIAL_DEBUG.print(S0_Zaehler[channel]);
                SERIAL_DEBUG.print(" ");
                SERIAL_DEBUG.print(S0_Zaehler_old[channel]);
                SERIAL_DEBUG.print(" ");
#endif

                // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                knx.getGroupObject(MOD_KoS01_Ges_Verbrauch + (4 * channel)).valueNoSend(S0_Zaehler[channel], getDPT(VAL_DPT_13)); // MOD_KoS01_ZaehlerWert+channel da KO nur 1 Byte auseinander liegen

                // senden bei Wertänderung
                if ((lsendMode == 1 || lsendMode == 3) && S0_Zaehler[channel] - S0_Zaehler_old[channel] >= knx.paramWord(MOD_SendminValuechangeS01 + (2 * channel)))
                {
                    if (delayCheck(minSendDelay_S0[channel], knx.paramWord(MOD_SendminValueDelayS01 + (2 * channel)) * 1000))
                    {
                        minSendDelay_S0[channel] = millis();
                        lSendZaehlerWert_S0[channel] = true;
                    }
                }

            } // if (S0_impuls[channel] >= zaehler_Impulse[channel])
            //-------------------------------------- ENDE Zähler ----------------------------------------------------------
            switch (knx.paramByte(MOD_DefineS0zaehler1 + channel))
            {
            case zaehlerElek:
                //calculation mom Verbrauch (W)
                mom_S0[channel] = 3600.0 / ((time_S0_stopp[channel] - time_S0_start[channel]) / (zaehler_Impulse[channel] * 1.0));

#ifdef Serial_Debug_S0
                SERIAL_DEBUG.print(" Mom (W): ");
#endif
                break;

            case zaehlerWasser:
                switch (knx.paramByte(MOD_DefineUnitS01 + channel))
                {
                case unit_l:
                    //calculation mom Verbrauch (l/h)
                    mom_S0[channel] = 3600.0 / ((time_S0_stopp[channel] - time_S0_start[channel]) / (zaehler_Impulse[channel] * 1.0));
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.print(" Mom Was (l/h): ");
#endif
                    break;
                case unit_m3:
                    //calculation mom Verbrauch (m3/s)
                    mom_S0[channel] = 1 / ((time_S0_stopp[channel] - time_S0_start[channel]) / (zaehler_Impulse[channel] * 1000.0));
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.print(" Mom Was (m3/s): ");
#endif
                    break;
                default:
                    break;
                }
                break;

            case zaehlerGas:
                switch (knx.paramByte(MOD_DefineUnitS01 + channel))
                {
                case unit_l:
                    //calculation mom Verbrauch (l/h)
                    mom_S0[channel] = 3600.0 / ((time_S0_stopp[channel] - time_S0_start[channel]) / (zaehler_Impulse[channel] * 1.0));
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.print(" Mom Gas (l/h): ");
#endif
                    break;
                case unit_m3:
                    //calculation mom Verbrauch (m3/s)
                    mom_S0[channel] = 1 / ((time_S0_stopp[channel] - time_S0_start[channel]) / (zaehler_Impulse[channel] * 1000.0));
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.print(" Mom Gas (m3/s): ");
#endif
                    break;
                default:
                    break;
                }
                break;

            default:
                break;
            }

            // senden bei Wertänderung
            lsendModeCon = knx.paramByte(MOD_S01SendModeCon + channel);
#ifdef Serial_Debug_S0
           // SERIAL_DEBUG.println(mom_S0[channel]);
           // SERIAL_DEBUG.println(mom_S0_old[channel]);
#endif
            if ((lsendModeCon == 1 || lsendModeCon == 3) && abs(mom_S0[channel] - mom_S0_old[channel]) >= knx.paramWord(MOD_SendminValuechangeConS01 + (2 * channel)))
            {
                if (delayCheck(minSendDelayCon_S0[channel], knx.paramWord(MOD_SendminValueDelayConS01 + (2 * channel)) * 1000))
                {
                    minSendDelayCon_S0[channel] = millis();
                    lSendMomLeistung_S0[channel] = true;
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.println("send Verbrauch");
#endif
                }
            }

            mom_S0_old[channel] = mom_S0[channel];

            switch (knx.paramByte(MOD_DefineS0zaehler1 + channel))
            {
            case zaehlerElek:
                // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                knx.getGroupObject(MOD_KoS01_Akt1_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_14)); // MOD_KoS01_ZaehlerWert+channel da KO nur 1 Byte auseinander liegen
                mom_S0[channel] = mom_S0[channel] / 1000.0;                                                                    // Umrechnung in KW
                knx.getGroupObject(MOD_KoS01_Akt2_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_9));  // MOD_KoS01_ZaehlerWert+channel da KO nur 1 Byte auseinander liegen
                break;
            case zaehlerWasser:
                switch (knx.paramByte(MOD_DefineUnitS01 + channel))
                {
                case unit_l:
                    // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                    knx.getGroupObject(MOD_KoS01_Akt2_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_9));  // l/h
                    mom_S0[channel] = mom_S0[channel] / 60000.0;                                                                   // umrechnung in m3/s
                    knx.getGroupObject(MOD_KoS01_Akt1_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_14)); // m3/s
                    break;
                case unit_m3:
                    // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                    knx.getGroupObject(MOD_KoS01_Akt1_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_14)); // m3/s
                    mom_S0[channel] = mom_S0[channel] * 60000.0;                                                                   // umrechnung in l/h
                    knx.getGroupObject(MOD_KoS01_Akt2_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_9));  // l/h
                    break;
                    break;
                default:
                    break;
                }
                break;

            case zaehlerGas:
                switch (knx.paramByte(MOD_DefineUnitS01 + channel))
                {
                case unit_l:
                    // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                    knx.getGroupObject(MOD_KoS01_Akt2_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_9));  // l/h
                    mom_S0[channel] = mom_S0[channel] / 60000.0;                                                                   // umrechnung in m3/s
                    knx.getGroupObject(MOD_KoS01_Akt1_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_14)); // m3/s
                    break;
                case unit_m3:
                    // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                    knx.getGroupObject(MOD_KoS01_Akt1_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_14)); // m3/s
                    mom_S0[channel] = mom_S0[channel] * 60000.0;                                                                   // umrechnung in l/h
                    knx.getGroupObject(MOD_KoS01_Akt2_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_9));  // l/h
                    break;
                    break;
                default:
                    break;
                }
                break;

            default:
                break;
            }

#ifdef Serial_Debug_S0
            SERIAL_DEBUG.println(mom_S0[channel]);
#endif
            //-------------------------------------- ENDE Mom Verbrauch ----------------------------------------------------------
            time_S0_start[channel] = time_S0_stopp[channel];
        } //if (impuls_S0[channel] == true)

        if (lSendZaehlerWert_S0[channel] == true && lSendMomLeistung_S0[channel] == true)
        {
            State[channel] = send_KO;
        }
        break;

    case set_Mom_0:
        mom_S0[channel] = 0;
#ifdef Serial_Debug_S0
        switch (knx.paramByte(MOD_DefineS0zaehler1 + channel))
        {
        case zaehlerElek:
            SERIAL_DEBUG.print(" Mom (W): ");
            break;
        case zaehlerWasser:
            SERIAL_DEBUG.print(" Mom Was: ");
            break;
        case zaehlerGas:
            SERIAL_DEBUG.print(" Mom Gas: ");
            break;
        }
#endif
        State[channel] = cycle;
        break;

    case send_KO:
        if (lSendZaehlerWert_S0[channel] == true)
        {
            lSendZaehlerWert_S0[channel] = false;
#ifdef Serial_Debug_S0
            SERIAL_DEBUG.println("Send Zähler");
#endif
            knx.getGroupObject(MOD_KoS01_Ges_Verbrauch + (4 * channel)).objectWritten();
            S0_Zaehler_old[channel] = S0_Zaehler[channel];

            sendDelay_S0[channel] = millis();
            minSendDelay_S0[channel] = millis();
            //lSendZaehlerWert_S0[channel] = false;
        }
        if (lSendMomLeistung_S0[channel] == true)
        {
            lSendMomLeistung_S0[channel] = false;
#ifdef Serial_Debug_S0
            SERIAL_DEBUG.println("Send Leistung");
#endif
            knx.getGroupObject(MOD_KoS01_Akt1_Verbrauch + (4 * channel)).objectWritten(); // KW oder m3/s
            knx.getGroupObject(MOD_KoS01_Akt2_Verbrauch + (4 * channel)).objectWritten(); // W oder l/h

            sendDelayCon_S0[channel] = millis();
            minSendDelayCon_S0[channel] = millis();
            //lSendMomLeistung_S0[channel] = false;
        }
        State[channel] = cycle;
        break;

    default:
        State[channel] = cycle;
        break;
    }


    */

#ifdef Debug_S0_LED
    if (delayCheck(time_S0_LED_Blink[1], 100))
    {
        digitalWrite(Diag_LED, false);
    }
#endif

    //----------------- Mom Verbrauch: Mindestleistung/durchfluss - Berechnung = 0(W/l/m3) -------------------
    // Berechnung max Pulsdauer für Mindestleistung/durchfluss
    // Dauer = 3600sek * Impulse / Mindestleistung
    maxPulsLength = 3600 * zaehler_Impulse[channel] / knx.paramWord(MOD_DefineMinValueS01 + (2*channel));
    // prüfen ob aktueller puls länger ist als Pulslänge min Leistung oder Durchfluss

    if (delayCheck(time_S0_start[channel], maxPulsLength))
    {
        det_maxPuls = true;
        mom_S0[channel] = 0;
#ifdef Serial_Debug_S0
        //SERIAL_DEBUG.println("max Puls");
#endif
    }
    else
    {
        det_maxPuls = false;
    }

    lsendMode = knx.paramByte(MOD_S01SendModeCounter + channel);
    lsendModeCon = knx.paramByte(MOD_S01SendModeCon + channel);

    // we waited enough, let's send the value
    if ((lsendMode == 2 || lsendMode == 3) && delayCheck(sendDelay_S0[channel], knx.paramInt(MOD_SendDelayS01 + (4 * channel)) * 1000)) //S01 und S02 liegen 4 Bytes auseinander
    {
        lSendZaehlerWert_S0[channel] = true;
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.println("Zyklisch Zähler");
#endif
    }
    // we waited enough, let's send the value
    else if ((lsendModeCon == 2 || lsendModeCon == 3) && delayCheck(sendDelayCon_S0[channel], knx.paramInt(MOD_SendDelayConS01 + (4 * channel)) * 1000)) //S01 und S02 liegen 4 Bytes auseinander
    {
        lSendMomLeistung_S0[channel] = true;
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.println("Zyklisch Verb");
#endif
    }

    if (impuls_S0[channel] == true)
    {
#ifdef Debug_S0_LED
        digitalWrite(Diag_LED, true);
        time_S0_LED_Blink[1] = millis();
#endif

#ifdef Serial_Debug_S0
        SERIAL_DEBUG.print("S0_");
        SERIAL_DEBUG.print(channel + 1);
#endif

        //-------------------------------------- Zähler ----------------------------------------------------------
        S0_impuls[channel]++;
        if (S0_impuls[channel] >= zaehler_Impulse[channel])
        {
            S0_Zaehler[channel]++;
            // senden bei Wertänderung
            if ((lsendMode == 1 || lsendMode == 3) && S0_Zaehler[channel] - S0_Zaehler_old[channel] >= knx.paramWord(MOD_SendminValuechangeS01 + (2 * channel)))
            {
                if (delayCheck(minSendDelay_S0[channel], knx.paramWord(MOD_SendminValueDelayS01 + (2 * channel)) * 1000))
                {
                    minSendDelay_S0[channel] = millis();
                    lSendZaehlerWert_S0[channel] = true;
                }
            }
            // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
            knx.getGroupObject(MOD_KoS01_Ges_Verbrauch + (4 * channel)).valueNoSend(S0_Zaehler[channel], getDPT(VAL_DPT_13)); // MOD_KoS01_ZaehlerWert+channel da KO nur 1 Byte auseinander liegen

#ifdef Serial_Debug_S0
            SERIAL_DEBUG.print(" Zähler: ");
            SERIAL_DEBUG.print(S0_Zaehler[channel]);
#endif
            S0_impuls[channel] = 0;
        }
        //-------------------------------------- Zähler ENDE -----------------------------------------------------

        //-------------------------------------- Mom Verbrauch ---------------------------------------------------
        if (!det_maxPuls) // nur berechnen wenn Pulslänge kleiner max Pulslänge Mindestleistung/durchfluss
        {
            switch (knx.paramByte(MOD_DefineS0zaehler1 + channel))
            {
            case zaehlerElek:
                //calculation mom Verbrauch (W)
                mom_S0[channel] = 3600.0 / ((time_S0_stopp[channel] - time_S0_start[channel]) / (zaehler_Impulse[channel] * 1.0));

#ifdef Serial_Debug_S0
                SERIAL_DEBUG.print(" Mom (W): ");
#endif
                break;

            case zaehlerWasser:
                switch (knx.paramByte(MOD_DefineUnitS01 + channel))
                {
                case unit_l:
                    //calculation mom Verbrauch (l/h)
                    mom_S0[channel] = 3600.0 / ((time_S0_stopp[channel] - time_S0_start[channel]) / (zaehler_Impulse[channel] * 1.0));
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.print(" Mom W (l/h): ");
#endif
                    break;
                case unit_m3:
                    //calculation mom Verbrauch (m3/s)
                    mom_S0[channel] = 1 / ((time_S0_stopp[channel] - time_S0_start[channel]) / (zaehler_Impulse[channel] * 1000.0));
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.print(" Mom W (m3/s): ");
#endif
                    break;
                default:
                    break;
                }
                break;

            case zaehlerGas:
                switch (knx.paramByte(MOD_DefineUnitS01 + channel))
                {
                case unit_l:
                    //calculation mom Verbrauch (l/h)
                    mom_S0[channel] = 3600.0 / ((time_S0_stopp[channel] - time_S0_start[channel]) / (zaehler_Impulse[channel] * 1.0));
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.print(" Mom Gas (l/h): ");
#endif
                    break;
                case unit_m3:
                    //calculation mom Verbrauch (m3/s)
                    mom_S0[channel] = 1 / ((time_S0_stopp[channel] - time_S0_start[channel]) / (zaehler_Impulse[channel] * 1000.0));
#ifdef Serial_Debug_S0
                    SERIAL_DEBUG.print(" Mom Gas (m3/s): ");
#endif
                    break;
                default:
                    break;
                }
                break;

            default:
                break;
            }
        } // ENDE IF (processCalc)
        else
        {
#ifdef Serial_Debug_S0
            SERIAL_DEBUG.print(" Mom: ");
#endif
            mom_S0[channel] = 0;
        }
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.print(mom_S0[channel]);
        SERIAL_DEBUG.print(" ");
        SERIAL_DEBUG.print(mom_S0_old[channel]);
        SERIAL_DEBUG.print(" ");
        SERIAL_DEBUG.print(abs(mom_S0[channel] - mom_S0_old[channel]));
        SERIAL_DEBUG.print(" ");
        SERIAL_DEBUG.println((time_S0_stopp[channel] - time_S0_start[channel]));
#endif
        time_S0_start[channel] = time_S0_stopp[channel];

        // senden bei Wertänderung
        if ((lsendModeCon == 1 || lsendModeCon == 3) && abs(mom_S0[channel] - mom_S0_old[channel]) >= knx.paramWord(MOD_SendminValuechangeConS01 + (2 * channel)))
        {
            if (delayCheck(minSendDelayCon_S0[channel], knx.paramWord(MOD_SendminValueDelayConS01 + (2 * channel)) * 1000))
            {
                minSendDelayCon_S0[channel] = millis();
                lSendMomLeistung_S0[channel] = true;
#ifdef Serial_Debug_S0
                SERIAL_DEBUG.println("Wert-Än Verb");
#endif
            }
        }
        mom_S0_old[channel] = mom_S0[channel];

        switch (knx.paramByte(MOD_DefineS0zaehler1 + channel))
        {
        case zaehlerElek:
            // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
            knx.getGroupObject(MOD_KoS01_Akt1_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_14)); // MOD_KoS01_ZaehlerWert+channel da KO nur 1 Byte auseinander liegen
            mom_S0[channel] = mom_S0[channel] / 1000.0;                                                                    // Umrechnung in KW
            knx.getGroupObject(MOD_KoS01_Akt2_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_9));  // MOD_KoS01_ZaehlerWert+channel da KO nur 1 Byte auseinander liegen
            break;
        case zaehlerWasser:
            switch (knx.paramByte(MOD_DefineUnitS01 + channel))
            {
            case unit_l:
                // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                knx.getGroupObject(MOD_KoS01_Akt2_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_9));  // l/h
                mom_S0[channel] = mom_S0[channel] / 60000.0;                                                                   // umrechnung in m3/s
                knx.getGroupObject(MOD_KoS01_Akt1_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_14)); // m3/s
                break;
            case unit_m3:
                // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                knx.getGroupObject(MOD_KoS01_Akt1_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_14)); // m3/s
                mom_S0[channel] = mom_S0[channel] * 60000.0;                                                                   // umrechnung in l/h
                knx.getGroupObject(MOD_KoS01_Akt2_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_9));  // l/h
                break;
                break;
            default:
                break;
            }
            break;

        case zaehlerGas:
            switch (knx.paramByte(MOD_DefineUnitS01 + channel))
            {
            case unit_l:
                // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                knx.getGroupObject(MOD_KoS01_Akt2_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_9));  // l/h
                mom_S0[channel] = mom_S0[channel] / 60000.0;                                                                   // umrechnung in m3/s
                knx.getGroupObject(MOD_KoS01_Akt1_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_14)); // m3/s
                break;
            case unit_m3:
                // we always store the new value in KO, even it it is not sent (to satisfy potential read request)
                knx.getGroupObject(MOD_KoS01_Akt1_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_14)); // m3/s
                mom_S0[channel] = mom_S0[channel] * 60000.0;                                                                   // umrechnung in l/h
                knx.getGroupObject(MOD_KoS01_Akt2_Verbrauch + (4 * channel)).valueNoSend(mom_S0[channel], getDPT(VAL_DPT_9));  // l/h
                break;
                break;
            default:
                break;
            }
            break;

        default:
            break;
        }

        //-------------------------------------- Mom Verbrauch ENDE -----------------------------------------------
        impuls_S0[channel] = false;
    }

    // Send Zählerwerte
    if (lSendZaehlerWert_S0[channel])
    {
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.println("Send Zähler");
#endif

        knx.getGroupObject(MOD_KoS01_Ges_Verbrauch + (4 * channel)).objectWritten();

        S0_Zaehler_old[channel] = S0_Zaehler[channel];
        sendDelay_S0[channel] = millis();
        minSendDelay_S0[channel] = millis();
        lSendZaehlerWert_S0[channel] = false;
    }
    else if (lSendMomLeistung_S0[channel])
    {
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.println("Send Leistung");
#endif
        knx.getGroupObject(MOD_KoS01_Akt1_Verbrauch + (4 * channel)).objectWritten(); // KW oder m3/s
        knx.getGroupObject(MOD_KoS01_Akt2_Verbrauch + (4 * channel)).objectWritten(); // W oder l/h
        sendDelayCon_S0[channel] = millis();
        minSendDelayCon_S0[channel] = millis();
        lSendMomLeistung_S0[channel] = false;
    }
}

void functionS01()
{
    time_S0_stopp[0] = millis();
    impuls_S0[0] = true;

#ifdef Serial_Debug_S0_Int
    SERIAL_DEBUG.print("S0_1: ");
    SERIAL_DEBUG.println(S0_impuls[0]);
#endif
}

void functionS02()
{
    time_S0_stopp[1] = millis();
    impuls_S0[1] = true;
#ifdef Serial_Debug_S0_Int
    SERIAL_DEBUG.print("S0_2: ");
    SERIAL_DEBUG.println(S0_impuls[1]);
#endif
}

uint16_t setZaehlerImpulse(uint8_t i, uint16_t impulse)
{
    zaehler_Impulse[i] = impulse;
    return zaehler_Impulse[i];
}

void sendZaehlerStand(int i, uint16_t S0_Zaehler[], uint16_t S0_Zaehler_old[])
{
    if (S0_Zaehler[i] != S0_Zaehler_old[i])
    {
        S0_Zaehler_old[i] = S0_Zaehler[i];

        if (i == 0)
        {
            switch (knx.paramByte(MOD_DefineS0zaehler1))
            {
            case 1: // Elektrischer Zähler
                knx.getGroupObject(MOD_KoS01_Ges_Verbrauch).valueNoSend(S0_Zaehler[i], getDPT(VAL_DPT_14));
                break;
            case 2: // Wasser Zähler
                knx.getGroupObject(MOD_KoS01_Ges_Verbrauch).valueNoSend(S0_Zaehler[i], getDPT(VAL_DPT_13));
                break;
            case 3: // Gas Zähler
                knx.getGroupObject(MOD_KoS01_Ges_Verbrauch).valueNoSend(S0_Zaehler[i], getDPT(VAL_DPT_13));
                break;

            default:
                break;
            }
        }
        else if (i == 1)
        {
            switch (knx.paramByte(MOD_DefineS0zaehler2))
            {
            case 1: // Elektrischer Zähler
                knx.getGroupObject(MOD_KoS02_Ges_Verbrauch).valueNoSend(S0_Zaehler[i], getDPT(VAL_DPT_14));
                break;
            case 2: // Wasser Zähler
                knx.getGroupObject(MOD_KoS02_Ges_Verbrauch).valueNoSend(S0_Zaehler[i], getDPT(VAL_DPT_13));
                break;
            case 3: // Gas Zähler
                knx.getGroupObject(MOD_KoS02_Ges_Verbrauch).valueNoSend(S0_Zaehler[i], getDPT(VAL_DPT_13));
                break;

            default:
                break;
            }
        }
    }

#ifdef KDEBUG
    SERIAL_DEBUG.print("Zaehler");
    SERIAL_DEBUG.print(i);
    SERIAL_DEBUG.print(": ");
    SERIAL_DEBUG.println(S0_Zaehler[i]);
#endif
}

void sendZaehlerStand_2(int i, uint16_t S0_Zaehler[], uint16_t S0_Zaehler_old[])
{
    //Knx.write(32 + (i * 3), S0_Zaehler[i]);
    //Knx.task();
    S0_Zaehler_old[i] = S0_Zaehler[i];
#ifdef KDEBUG
    SERIAL_DEBUG.print("Zaehler");
    SERIAL_DEBUG.print(i);
    SERIAL_DEBUG.print(": ");
    SERIAL_DEBUG.println(S0_Zaehler[i]);
#endif
}