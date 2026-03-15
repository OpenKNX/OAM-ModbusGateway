#pragma once
#include "ModbusChannel.h"
#include "OpenKNX.h"
#ifdef ARDUINO_ARCH_RP2040
#ifndef OPENKNX_USB_EXCHANGE_IGNORE
#include "UsbExchangeModule.h"
#endif
#endif

// #define DEVICE_ID "SMARTMF-Modbus-RTU-3BE"
// #define DEVICE_NAME "Smart-MF Modbus-RTU Gateway"

// #define SMARTMF_HARDWARE_REVISION_ID1_PIN 2
// #define SMARTMF_HARDWARE_REVISION_ID2_PIN 3
// #define SMARTMF_HARDWARE_REVISION_ID3_PIN 15

// #define SMARTMF_MODBUS_RX_PIN 5
// #define SMARTMF_MODBUS_TX_PIN 4
// #define SMARTMF_MODBUS_DIR_PIN 24

class ModbusModule : public OpenKNX::Module, public ModbusMaster
{
private:
    bool _error[255];
    bool readyToSendModbus[255] = {0};
    uint8_t result_old[255] = {0x01};
    uint32_t _timer1 = 0;
    uint32_t _timer2 = 0;
    uint32_t _timerCycle = 0;

    uint32_t _timerCycleSendChannel = 0;
    uint8_t _currentChannel = 0;
    uint8_t _channel = 0;

    ModbusChannel *_channels[MOD_ChannelCount];
    OpenKNX::Flash::Driver *_modbusStorage = nullptr;
    static bool idle_processing;
    static uint32_t _timerCycleChannel;

    void setupCustomFlash();
    void setupChannels();
    int findNextActive(int size, int currentIndex);
    int findNextReady(int size, int currentIndex);
    uint8_t findNextReadyToSend(int size);
    void errorHandling();
    void ErrorHandlingLED();
#ifdef ARDUINO_ARCH_RP2040
#ifndef OPENKNX_USB_EXCHANGE_IGNORE
    void registerUsbExchangeCallbacks();
#endif
#endif
    static void idleCallback();
    static void preTransmission();
    static void postTransmission();

public:
    ModbusModule();
    void loop(bool configured) override;
    void setup(bool configured) override;
#ifdef OPENKNX_DUALCORE
    void loop1(bool configured) override;
    void setup1(bool configured) override;
#endif
    const std::string name() override;
    const std::string version() override;
    void processInputKo(GroupObject &ko) override;
    bool processCommand(const std::string cmd, bool diagnoseKo);
    void showHelp() override;

    bool modbusInitSerial(HardwareSerial &serial);
    bool modbusParitySerial(uint32_t baud, HardwareSerial &serial);
};

extern ModbusModule openknxModbusModule;