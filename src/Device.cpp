#include <Arduino.h>

#include "Device.h"
#include "KnxHelper.h"
#include "LED_Statusanzeige.h"
#include "Logic.h"
#include "Modbus.h"
#include "ModbusGateway.h"
#include "ModbusMaster.h"
#include "ModbusRead.h"
//#include "S0_Master.h"
#include "HelperFunc.h"
#include "wiring_private.h" // pinPeripheral() function
#include <knx.h>

#include "S0Input.h"

S0Input S0[MAX_S0_CHANNELS];
#ifdef BOARD_MASIFI_MODBUS_BREAKOUT
    SerialPIO modbusSerial = SerialPIO(MODBUS_UART_TX_PIN, MODBUS_UART_RX_PIN, 64U);
#endif

uint32_t heartbeatDelay = 0;
uint32_t gStartupDelay = 0;
Logic gLogic;

uint8_t usedModbusChannels = 0;

bool modbusReady = false;
bool S0Ready = false;

uint16_t getPar(uint16_t PAR, uint8_t CH)
{
    return MOD_ParamBlockOffset + (CH * MOD_ParamBlockSize) + PAR;
}

uint16_t getCom(uint16_t COM, uint8_t CH)
{
    return MOD_KoOffset + (CH * MOD_KoBlockSize) + COM;
}

// Functions
void ProcessHeartbeat()
{
    // the first heartbeat is send directly after startup delay of the device
    if (heartbeatDelay == 0 || delayCheck(heartbeatDelay, getDelayPattern(LOG_HeartbeatDelayBase)))
    {
        // we waited enough, let's send a heartbeat signal
        knx.getGroupObject(LOG_KoHeartbeat).value(true, getDPT(VAL_DPT_1));
        heartbeatDelay = millis();
        // debug entry point
        gLogic.debug();
    }
}

void ProcessReadRequests()
{
    // this method is called after startup delay and executes read requests, which should just happen once after startup
    static bool sCalledProcessReadRequests = false;
    if (!sCalledProcessReadRequests)
    {
        // put here own readRequest coding
        sCalledProcessReadRequests = true;
    }
    gLogic.processReadRequests();
}

// true solange der Start des gesamten Moduls verzögert werden soll
bool startupDelay()
{
    return !delayCheck(gStartupDelay, getDelayPattern(LOG_StartupDelayBase, true));
}

bool processDiagnoseCommand()
{
    char *lBuffer = gLogic.getDiagnoseBuffer();
    bool lOutput = false;
    if (lBuffer[0] == 'v')
    {
        // Command v: return filmware version, do not forward this to logic,
        // because it means firmware version of the outermost module
        // sprintf(lBuffer, "VER [%d] %d.%d", cFirmwareMajor, cFirmwareMinor, cFirmwareRevision);
        lOutput = true;
    }
    else
    {
        // let's check other modules for this command
        // put here local diagnose coding
        if (!lOutput)
            lOutput = gLogic.processDiagnoseCommand();
    }
    return lOutput;
}

void ProcessDiagnoseCommand(GroupObject &iKo)
{
    // this method is called as soon as iKo is changed
    // an external change is expected
    // because this iKo also is changed within this method,
    // the method is called again. This might result in
    // an endless loop. This is prevented by the isCalled pattern.
    static bool sIsCalledProcessDiagnoseCommand = false;
    if (!sIsCalledProcessDiagnoseCommand)
    {
        sIsCalledProcessDiagnoseCommand = true;
        // diagnose is interactive and reacts on commands
        gLogic.initDiagnose(iKo);
        if (processDiagnoseCommand())
            gLogic.outputDiagnose(iKo);
        sIsCalledProcessDiagnoseCommand = false;
    }
};

void processInputKo(GroupObject &iKo)
{
    // Compute modbus channel number
    int channel = (iKo.asap() - MOD_KoOffset - MOD_KoGO_BASE_) / MOD_KoBlockSize;
    if (channel >= 0 && channel < MOD_ChannelCount)
    {
#ifdef Serial_Debug_Modbus
        SERIAL_DEBUG.print("KO: ");
        SERIAL_DEBUG.println(channel + 1);
#endif
        // Get slave number
        int slave = knx.paramByte(getPar(MOD_CHModbusSlaveSelection, channel));
        if (slave >= 1 && slave < MaxCountSlaves)
        {
            Slave[slave - 1].ReadyToSendModbus(channel);
        }
    }
}

void ProcessKoCallback(GroupObject &iKo)
{
    // check if we evaluate own KO
    if (iKo.asap() == LOG_KoDiagnose)
    {
        ProcessDiagnoseCommand(iKo);
    }
    else
    {
        processInputKo(iKo);
        // always dispatch to Logicmodule, too
        gLogic.processInputKo(iKo);
    }
}

/**
 * @return upper border of used channels
 */
uint8_t getUsedModbusChannels()
{
    uint8_t countChannels = 0;
    for (int channel = 0; channel < MOD_ChannelCount; channel++)
    {
        if (knx.paramByte(getPar(MOD_CHModbusSlaveSelection, channel)) != 0) // wenn ein Slave gewählt wurde
        {
            if (knx.paramByte(getPar(MOD_CHModBusDptSelection, channel)) != 0) // und zusätzlich auch ein DPT
            {
                countChannels = channel + 1;
            }
        }
    }
    return countChannels;
}

bool setupModbus()
{
    // HardwareSerial &serial = Serial; //does not work
#ifdef LED_YELLOW_PIN
    pinMode(MAX485_RE_NEG, OUTPUT);
#endif
    pinMode(MAX485_DIR, OUTPUT);
// Init in receive mode
#ifdef LED_YELLOW_PIN
    digitalWrite(MAX485_RE_NEG, 0);
#endif
    digitalWrite(MAX485_DIR, 0);

#ifdef ARDUINO_ARCH_RP2040
    // serial = Serial2; // does not work!!! 
    #ifdef BOARD_MASIFI_MODBUS_V21
    // modbusInitSerial(Serial2);
    // modbusInitSlaves(Serial2);
    #endif
    #ifdef BOARD_MASIFI_MODBUS_BREAKOUT
    // modbusInitSerial(Serial1);
    // modbusInitSlaves(Serial1);
    modbusInitSerial(modbusSerial);
    modbusInitSlaves(modbusSerial);
    #endif
#else
    // serial = Serial3; // does not work!!! 
    modbusInitSerial(Serial3);
    modbusInitSlaves(Serial3);
#endif

#ifdef Serial_Debug_Modbus
    SERIAL_DEBUG.println("Modbus Setup Done");
#endif

    return true;
}

void setupS0()
{
#ifdef Debug_S0_LED
    pinMode(Diag_LED, OUTPUT);
    digitalWrite(Diag_LED, false);
#endif

    if (knx.paramByte(getParBIN(BIN_InputTypes, S0_CH1)) == typeS0)
    {
        pinMode(S0_CH1_Pin, INPUT);
        // init
        // S0[S0_CH1].initS0(S0_CH1_Pin, S0_CH1_LED, 0);
        S0[S0_CH1].setup(S0_CH1_Pin, S0_CH1_LED, S0_CH1);
        // set Impulse / KWH
        S0[S0_CH1].set_impulseProKwh(knx.paramWord(getParBIN(BIN_S0Impulse, S0_CH1)));
        S0[S0_CH1].set_maxPulsLength(knx.paramWord(getParBIN(BIN_S0DefineMinValue, S0_CH1)));

// print value
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.println("---");
        SERIAL_DEBUG.println("S01 = Aktiv");
        SERIAL_DEBUG.print("Impulse(impl/KWh): ");
        SERIAL_DEBUG.println(knx.paramWord(getParBIN(BIN_S0Impulse, S0_CH1)));
        SERIAL_DEBUG.print("DefineUnit: ");
        SERIAL_DEBUG.println(knx.paramByte(getParBIN(BIN_S0DefineUnit, S0_CH1)));
        SERIAL_DEBUG.print("min Value: ");
        SERIAL_DEBUG.println(knx.paramWord(getParBIN(BIN_S0DefineMinValue, S0_CH1)));
        SERIAL_DEBUG.print("Sende Modus: ");
        SERIAL_DEBUG.println(knx.paramByte(getParBIN(BIN_S0SendModeCounter, S0_CH1)));
        SERIAL_DEBUG.print("Zyklisch Senden (s): ");
        SERIAL_DEBUG.println(knx.paramWord(getParBIN(BIN_S0SendDelay, S0_CH1)));
        SERIAL_DEBUG.println("---");
#endif
    }

    if (knx.paramByte(getParBIN(BIN_InputTypes, S0_CH2)) == typeS0)
    {

        pinMode(S0_CH2_Pin, INPUT);
        // init
        S0[S0_CH2].setup(S0_CH2_Pin, S0_CH2_LED, S0_CH2);
        // set Impulse / KWH
        S0[S0_CH2].set_impulseProKwh(knx.paramWord(getParBIN(BIN_S0Impulse, S0_CH2)));
        S0[S0_CH2].set_maxPulsLength(knx.paramWord(getParBIN(BIN_S0DefineMinValue, S0_CH2)));
// print value
#ifdef Serial_Debug_S0
        SERIAL_DEBUG.println("---");
        SERIAL_DEBUG.println("S02 = Aktiv");
        SERIAL_DEBUG.print("Impulse(impl/KWh): ");
        SERIAL_DEBUG.println(knx.paramWord(getParBIN(BIN_S0Impulse, S0_CH2)));
        SERIAL_DEBUG.print("DefineUnit: ");
        SERIAL_DEBUG.println(knx.paramByte(getParBIN(BIN_S0DefineUnit, S0_CH2)));
        SERIAL_DEBUG.print("min Value: ");
        SERIAL_DEBUG.println(knx.paramWord(getParBIN(BIN_S0DefineMinValue, S0_CH2)));
        SERIAL_DEBUG.print("Senden Modus Verbrauch: ");
        SERIAL_DEBUG.println(knx.paramByte(getParBIN(BIN_S0SendModeCon, S0_CH2)));
        SERIAL_DEBUG.print("Zyklisch Senden (s): ");
        SERIAL_DEBUG.println(knx.paramWord(getParBIN(BIN_S0SendDelay, S0_CH2)));
        SERIAL_DEBUG.println("---");
        

      
#endif
    }
#ifdef Serial_Debug_Modbus
    SERIAL_DEBUG.println("S0 Setup Done");
#endif
}

// Mat's first version of Hardware autoconfig
#define HW_BREAKOUT 8
#define HW_20       7
#define HW_21       6

uint8_t get_PROG_LED_PIN(uint8_t hwID)
{
#ifdef ARDUINO_ARCH_RP2040
    switch (hwID)
    {
        case HW_21: // HW 2.1
            return 11;
        case HW_20: // HW 2.0
            return 1;
        case HW_BREAKOUT: // Breakout-Board
            return 22;
        default:
            return 255;
    }
#else
    switch (hwID)
    {
        case 7: // V1.x
            return 9;
        case 6: // V2.x
            return 6;
        default:
            return 255;
    }
#endif
}

uint8_t get_PROG_BUTTON_PIN(uint8_t hwID)
{
#ifdef ARDUINO_ARCH_RP2040
    switch (hwID)
    {
        case HW_21: // HW 2.1
            return 14;
        case HW_20: // HW 2.0
            return 3;
        case HW_BREAKOUT: // Breakout-Board
            return 28;
        default:
            return 255;
    }
#else
    switch (hwID)
    {
        case 7: // V1.x
            return A1;
        case 6: // V2.x
            return 7;
        default:
            return 250;
    }
#endif
}

uint8_t get_PROG_LED_PIN_ACTIVE_ON(uint8_t hwID)
{
    return HIGH;
}

uint8_t get_SAVE_INTERRUPT_PIN(uint8_t hwID)
{
#ifdef ARDUINO_ARCH_RP2040
    switch (hwID)
    {
        case HW_21: // HW 2.1
            return 8;
        case HW_20: // HW 2.0
            return 20;
        case HW_BREAKOUT: // Breakout-Board
            return 6;
        default:
            return 255;
    }
#else
    switch (hwID)
    {
        case 6: // V2.x
            return 8; //D8
        case 7: // V1.x
        default:
            return 0;
    }
#endif
}

uint8_t get_HW_ID()
{
    uint8_t hw_ID = 0;
#ifdef BOARD_MASIFI_MODBUS_BREAKOUT
    hw_ID = HW_BREAKOUT;
#else
    // Set Inputs
    pinMode(ID1, INPUT_PULLUP);
    pinMode(ID2, INPUT_PULLUP);
    pinMode(ID3, INPUT_PULLUP);
    // read Inputs
    bitWrite(hw_ID, 0, digitalRead(ID1));
    bitWrite(hw_ID, 1, digitalRead(ID2));
    bitWrite(hw_ID, 2, digitalRead(ID3));
#endif
    return hw_ID;
}

void initHW(uint8_t hwID)
{
#ifdef ARDUINO_ARCH_RP2040    
    switch (hwID)
    {
        case HW_BREAKOUT:
            SERIAL_DEBUG.println("HW_ID: BREAKOUT-BOARD");
            break;
        case HW_20:
            SERIAL_DEBUG.println("HW_ID: V2.0");
            break;
        case HW_21:
            SERIAL_DEBUG.println("HW_ID: V2.1");
            break;
        default:
            SERIAL_DEBUG.print("HW_ID: ERROR ");
            SERIAL_DEBUG.println(hwID);
            break;
    }
#else
    switch (hwID)
    {
        case 7:
            SERIAL_DEBUG.println("HW_ID: V1.x");
            break;
        case 6:
            SERIAL_DEBUG.println("HW_ID: V2.x");
            break;

        default:
            SERIAL_DEBUG.print("HW_ID: ERROR ");
            SERIAL_DEBUG.println(hwID);
            break;
    }
#endif
    initI2cStatusLeds();
    setLED_OFF();
    setLED(MBUS_STATUS, LOW);
    setLED(MODBUS_ERROR, LOW);
}

void logicCallback(void *iInstance)
{
    gLogic.loop();
}

void appSetup()
{
    // Modbus
    modbusReady = setupModbus();
    // determine count of used channels
    usedModbusChannels = getUsedModbusChannels();
    SERIAL_DEBUG.print("Channel Count:");
    SERIAL_DEBUG.println(usedModbusChannels);

    // S0
    setupS0();

    if (knx.configured())
    {
        if (GroupObject::classCallback() == 0)
            GroupObject::classCallback(ProcessKoCallback);
        gStartupDelay = millis();
        heartbeatDelay = 0;
        Schedule::addCallback(logicCallback, nullptr);
        uint8_t lSaveInterruptPin = get_SAVE_INTERRUPT_PIN(get_HW_ID());
        gLogic.setup(lSaveInterruptPin);
        openknx.flashUserData()->readFlash();
        // We don't have a #define SAVE_INTERRUPT_PIN, so the following is not executed in common
        static bool sSaveInterruptAttached = false;
        if (!sSaveInterruptAttached && lSaveInterruptPin > 0)
        {
            printDebug("Save interrupt pin attached...\n");
            pinMode(lSaveInterruptPin, INPUT);
            attachInterrupt(digitalPinToInterrupt(lSaveInterruptPin), FlashUserData::onSafePinInterruptHandler, FALLING);
        }
        sSaveInterruptAttached = true;
    }
}

void appLoop()
{
    if (!knx.configured())
        return;

    // handle KNX stuff
    if (startupDelay())
        return;

    ProcessHeartbeat();
    ProcessReadRequests();

    if (knx.paramByte(getParBIN(BIN_InputTypes, S0_CH1)) == typeS0)
    {
        S0[0].process();
    }
    if (knx.paramByte(getParBIN(BIN_InputTypes, S0_CH2)) == typeS0)
    {
        S0[1].process();
    }

    ModbusRead(usedModbusChannels);
    Schedule::loop();
}