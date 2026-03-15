#include "ModbusModule.h"
#include "HardwareConfig.h"
#include "ModBusMaster.h"
#include "LED_Statusanzeige.h"
#include "Device.h"

// #define DEVICE_SMARTMF_MODBUS_RTU_3BE
#define SMARTMF_MODBUS_SERIAL Serial2

// uint32_t timer_time_between_Reg_Reads;
// uint32_t timer_time_between_Cycle_Reads;

bool run_cycle = true;
bool readyToSend = false; 

bool ModbusModule::idle_processing = false;
unsigned long ModbusModule::_timerCycleChannel = 0;

ModbusModule::ModbusModule()
{
    idle(ModbusModule::idleCallback);
}

const std::string ModbusModule::name()
{
    return "modbus";
}

const std::string ModbusModule::version()
{
    // hides the module in the version output on the console, because the firmware version is sufficient.
    return "";
}

void ModbusModule::setup(bool configured)
{
    // delay(1000);
    logDebugP("Setup0");
    logIndentUp();

    // setup Pins
#if defined(DEVICE_SMARTMF_1TE_MODBUS) || defined(DEVICE_SMARTMF_MODBUS_AUSSEN)
    pinMode(SMARTMF_LED, OUTPUT);
    digitalWrite(SMARTMF_LED, LOW);
#endif
#ifdef DEVICE_SMARTMF_MODBUS_RTU_3BE
    Wire.setSDA(12);
    Wire.setSCL(13);
    Wire.begin();
    Wire.setClock(400000);
    initHW(get_HW_ID());
#endif
    pinMode(SMARTMF_MODBUS_DIR_PIN, OUTPUT);
    SMARTMF_MODBUS_SERIAL.setRX(SMARTMF_MODBUS_RX_PIN);
    SMARTMF_MODBUS_SERIAL.setTX(SMARTMF_MODBUS_TX_PIN);

    modbusInitSerial(SMARTMF_MODBUS_SERIAL);

    if (configured)
    {
        // setupCustomFlash();                               // ********************************* anpassen wenn notwendig *********************
        setupChannels();
    }
    else
    {
#ifdef DEVICE_SMARTMF_MODBUS_RTU_3BE
        setLED_Modbus(false);
#endif
    }
    logIndentDown();
}

void ModbusModule::setupChannels()
{

    for (uint8_t i = 0; i < ParamMOD_VisibleChannels; i++)
    {
        _channels[i] = new ModbusChannel(i, 3, 3, SMARTMF_MODBUS_SERIAL);
        _channels[i]->setup();
        _channels[i]->idle(idleCallback);
        _channels[i]->preTransmission(preTransmission);
        _channels[i]->postTransmission(postTransmission);
    }
#ifdef DEVICE_SMARTMF_MODBUS_RTU_3BE
    if (ParamMOD_VisibleChannels != 0)
        setLED_Modbus(true);
    else
        setLED_Modbus(false);
#endif
}

void ModbusModule::setupCustomFlash()
{
    logDebugP("initialize modbus flash");
    OpenKNX::Flash::Driver _modbusStorage;
#ifdef ARDUINO_ARCH_ESP32
    _modbusStorage.init("modbus");
#else
    _modbusStorage.init("modbus", modbus_FLASH_OFFSET, modbus_FLASH_SIZE);
#endif

    logTraceP("write modbus data");
    // _modbusStorage.writeByte(0, 0x11);
    // _modbusStorage.writeWord(1, 0xFFFF);
    // _modbusStorage.writeInt(3, 6666666);
    // for (size_t i = 0; i < 4095; i++)
    // {
    //     _modbusStorage.writeByte(i, i);
    // }
    // _modbusStorage.commit();

    logDebugP("read modbus data");
    logIndentUp();
    // logHexDebugP(_modbusStorage.flashAddress(), 4095);
    // logDebugP("byte: %02X", _modbusStorage.readByte(0)); // UINT8
    // logDebugP("word: %i", _modbusStorage.readWord(1));   // UINT16
    // logDebugP("int: %i", _modbusStorage.readInt(3));     // UINT32

    logIndentDown();
}

void ModbusModule::idleCallback()
{
    idle_processing = true;
    openknx.loop();
    idle_processing = false;
    _timerCycleChannel = millis();
}

void ModbusModule::preTransmission()
{
    digitalWrite(SMARTMF_MODBUS_DIR_PIN, 1);
}

void ModbusModule::postTransmission()
{
    digitalWrite(SMARTMF_MODBUS_DIR_PIN, 0);
}

uint8_t ModbusModule::findNextReadyToSend(int size)
{
    for (int i = 1; i <= size; i++) // i=1, damit wir die 0 als Rückgabewert haben, für kein CH is ready
    {
        if (readyToSendModbus[i])
        {
            readyToSendModbus[i] = false;
            return i;
        }
    }
    return 0;
}

int ModbusModule::findNextActive(int size, int currentIndex)
{
    for (int i = 1; i <= size; i++) // i=1, damit wir nicht wieder currentIndex selbst nehmen
    {
        int idx = (currentIndex + i) % size;
        if (_channels[idx]->isActiveCH())
        {
            return idx; // hier haben wir den nächsten activen CH gefunden
        }
    }
    return 0;
}

int ModbusModule::findNextReady(int size, int currentIndex)
{
    for (int i = currentIndex + 1; i < size; i++)
    {
        if (_channels[i]->isReadyCH())
            return i;
    }
    return size;
}

void ModbusModule::errorHandling()
{
    ErrorHandlingLED();
}

void ModbusModule::ErrorHandlingLED()
{
    bool error = false;
    for (int i = 0; i < ParamMOD_VisibleChannels; i++)
    {
        if (_error[i])
        {
            error = true;
        }
    }
    if (error)
    {
        // setLED_ERROR(HIGH);
#if defined(DEVICE_SMARTMF_1TE_MODBUS) || defined(DEVICE_SMARTMF_MODBUS_AUSSEN)
        digitalWrite(SMARTMF_LED, HIGH);
#endif
#ifdef DEVICE_SMARTMF_MODBUS_RTU_3BE
        setLED_ERROR(true);
#endif
    }
    else
    {
        // setLED_ERROR(LOW);
#if defined(DEVICE_SMARTMF_1TE_MODBUS) || defined(DEVICE_SMARTMF_MODBUS_AUSSEN)
        digitalWrite(SMARTMF_LED, LOW);
#endif
#ifdef DEVICE_SMARTMF_MODBUS_RTU_3BE
        setLED_ERROR(false);
#endif
    }
}

void ModbusModule::loop(bool configured)
{

    // if (delayCheck(_timer1, 1000))
    //{
    //     logDebugP("LoopModule");
    //     _timer1 = millis();
    //     logDebugP("CH%i:", _channel);
    // }

    if (configured)
    {
        if (ParamMOD_VisibleChannels == 0)
            return;

        uint8_t processed = 0;
        uint8_t count = 0;
        uint8_t result;
        uint16_t diag_register = 0;
        do
        {
            errorHandling();

            _channels[_currentChannel]->loop(readyToSend); // loop -> only for KNX send send cyclically
            _currentChannel = findNextActive(ParamMOD_VisibleChannels, _currentChannel);

            // if (!idle_processing)
            //{
            //     uint8_t ch = findNextReadyToSend(ParamMOD_VisibleChannels, _currentChannel);
            //
            //    if (ch != 0)
            //    {
            //        if (delayCheck(_timerCycleSendChannel, 10))
            //        {
            //            _channels[ch - 1]->knxToModbus();
            //            _timerCycleSendChannel;
            //        }
            //    }
            //}

            if (!idle_processing)
            {

                if (delayCheck(_timerCycleChannel, (ParamMOD_BusDelayRequest * 10)+5)) // Zeit zwischen zwei Modbus Register Abfragen +5ms default Wartezeit
                {
                    // prüft ob ein CH eine Modbus Botschaft senden will und gibt diese CH-nummer zurück
                    uint8_t ch = findNextReadyToSend(ParamMOD_VisibleChannels);
                    if (ch != 0) // KNX to MODBUS Abfrage
                    {
                        _channels[ch - 1]->knxToModbus();
                    }
                    else if (run_cycle) // MODBUS to KNX Abfrage
                    {
                        result = _channels[_channel]->readModbus(true);                                 // read cyclically the Modbus-Channels
                        if (result != result_old[_channel] && _channels[_channel]->getDirection() == 1) // Prüft auf änderung und ob CH ModbusToKnx ist
                        {
                            if (result == 1)
                            {
                                logInfoP("CH%i: run again", _channel + 1);
                                _error[_channel] = false;
                            }
                            else
                            {
                                logInfoP("CH%i: ERROR: %i", _channel + 1, result, HEX);
                                _error[_channel] = true;
                            }
                            // Diagnose Objekt schicken
                            diag_register = ((uint16_t)result << 8) | _channel; // Setzt aktuellen CH auf LSB & Error Code auf MSB
                            KoMOD_DebugModbus.value(diag_register, DPT_Value_2_Ucount);
                            result_old[_channel] = result;
                        }

                        // Sucht nächsten aktiven und wartenden Channel
                        _channel = findNextReady(ParamMOD_VisibleChannels, _channel);

                        // setzt _channel counter wieder zurück
                        if (_channel >= ParamMOD_VisibleChannels)
                        {
                            _channel = 0;
                            run_cycle = false;
                            _timerCycle = millis();
                        }
                    }
                    _timerCycleChannel = millis();
                }
            }

            // Wartet xsek bis der nächste komplette Abfragezyklus gestartet wird
            if (delayCheck(_timerCycle, (ParamMOD_BusDelayCycle * 1000)) && !run_cycle)
            {
                run_cycle = true;
                readyToSend = true; // Nachdem alle CH durchgelaufen sind, darf auf den Bus gesendet werden
            }
        } while (openknx.freeLoopIterate(ParamMOD_VisibleChannels, count, processed));
    }
}

#ifdef OPENKNX_DUALCORE

void ModbusModule::setup1(bool configured)
{
    delay(1000);
    // logDebugP("Setup1");
}

void ModbusModule::loop1(bool configured)
{
    if (delayCheck(_timer2, 7200))
    {
        // logDebugP("Loop1");
        _timer2 = millis();
    }
}
#endif

void ModbusModule::processInputKo(GroupObject &ko)
{
    logDebugP("processInputKo GA%04X", ko.asap());
    logHexDebugP(ko.valueRef(), ko.valueSize());

    // #define MOD_KoCalcNumber(index) (index + MOD_KoBlockOffset + _channelIndex * MOD_KoBlockSize)
    // #define MOD_KoCalcIndex(number) ((number >= MOD_KoCalcNumber(0) && number < MOD_KoCalcNumber(MOD_KoBlockSize)) ? (number - MOD_KoBlockOffset) % MOD_KoBlockSize : -1)
    // #define MOD_KoCalcChannel(number) ((number >= MOD_KoBlockOffset && number < MOD_KoBlockOffset + MOD_ChannelCount * MOD_KoBlockSize) ? (number - MOD_KoBlockOffset) / MOD_KoBlockSize : -1)

    // Compute modbus channel number
    // int channel = (iKo.asap() - MOD_KoOffset - MOD_KoGO_BASE_) / MOD_KoBlockSize;
    int channel = MOD_KoCalcChannel(ko.asap());
    if (channel >= 0 && channel < MOD_ChannelCount && _channels[channel]->getDirection() == 0)
    {
        logDebugP("->KO: %i", channel + 1);
        readyToSendModbus[channel + 1] = true;
        //_channels[_currentChannel]->knxToModbus();
    }
}

void ModbusModule::showHelp()
{
    openknx.console.printHelpLine("modbus", "Print a modbus text");
}

bool ModbusModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd.substr(0, 5) == "modbus")
    {
        logInfoP("modbus Info");
        logIndentUp();
        logInfoP("Info 1");
        logInfoP("Info 2");
        logIndentUp();
        logInfoP("Info 2a");
        logInfoP("Info 2b");
        logIndentDown();
        logInfoP("Info 3");
        logIndentDown();
        return true;
    }

    return false;
}

#ifdef ARDUINO_ARCH_RP2040
#ifndef OPENKNX_USB_EXCHANGE_IGNORE
void ModbusModule::registerUsbExchangeCallbacks()
{
    // Sample
    openknxUsbExchangeModule.onLoad("modbus.txt", [](UsbExchangeFile *file) -> void
                                    { file->write("Demo"); });
    openknxUsbExchangeModule.onEject("modbus.txt", [](UsbExchangeFile *file) -> bool
                                     {
        // File is required
        if (file == nullptr)
        {
            logError("ModbusModule", "File modbus.txt was deleted but is mandatory");
            return false;
        }
        return true; });
}
#endif
#endif

bool ModbusModule::modbusParitySerial(uint32_t baud, HardwareSerial &serial)
{
    switch (ParamMOD_BusParitySelection_Slave1)
    {
    case 0: // Even (1 stop bit)
        serial.begin(baud, SERIAL_8E1);
        logInfoP("Parity: Even (1 stop bit)");
        return true;
        break;
    case 1: // Odd (1 stop bit)
        serial.begin(baud, SERIAL_8O1);
        logInfoP("Parity: Odd (1 stop bit)");
        return true;
        break;
    case 2: // None (2 stop bits)
        serial.begin(baud, SERIAL_8N2);
        logInfoP("Parity: None (2 stop bits)");
        return true;
        break;
    case 3: // None (1 stop bit)
        serial.begin(baud, SERIAL_8N1);
        logInfoP("Parity: None (1 stop bit)");
        return true;
        break;

    default:
        logInfoP("Parity: Error: %i", ParamMOD_BusParitySelection_Slave1);
        return false;
        break;
    }
}

bool ModbusModule::modbusInitSerial(HardwareSerial &serial)
{
    // Set Modbus communication baudrate
    switch (ParamMOD_BusBaudrateSelection_Slave1)
    {
    case 0:
        logInfoP("Baudrate: 1200kBit/s");
        return modbusParitySerial(1200, serial);

        break;
    case 1:
        logInfoP("Baudrate: 2400kBit/s");
        return modbusParitySerial(2400, serial);
        break;
    case 2:
        logInfoP("Baudrate: 4800kBit/s");
        return modbusParitySerial(4800, serial);
        break;
    case 3:
        logInfoP("Baudrate: 9600kBit/s");
        return modbusParitySerial(9600, serial);
        break;
    case 4:
        logInfoP("Baudrate: 19200kBit/s");
        return modbusParitySerial(19200, serial);
        break;
    case 5:
        logInfoP("Baudrate: 38400kBit/s");
        return modbusParitySerial(38400, serial);
        break;
    case 6:
        logInfoP("Baudrate: 56000kBit/s");
        return modbusParitySerial(56000, serial);
        break;
    case 7:
        logInfoP("Baudrate: 115200kBit/s");
        return modbusParitySerial(115200, serial);
        break;
    default:
        logInfoP("Baudrate: Error: %i", ParamMOD_BusBaudrateSelection_Slave1);
        return false;
        break;
    }
}

ModbusModule openknxModbusModule;