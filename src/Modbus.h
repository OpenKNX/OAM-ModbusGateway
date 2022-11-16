#pragma once

#include "knxprod.h"
#include "hardware.h"
#include "ModbusMaster.h"
#include <knx.h>				

class Modbus : public ModbusMaster
{
public:
    Modbus();

    void initSlave(uint8_t slave, Stream &serial, uint8_t RegisterPos, uint8_t RegisterStart);

    bool readModbus(uint8_t channel, bool readRequest);
	bool sendModbus(uint8_t channel);
    void ReadyToSendModbus(uint8_t channel);								 
    uint8_t getRegisterstart();
    uint8_t getSlaveID();

    bool getErrorState1(uint8_t channel);
    bool getErrorState2(uint8_t channel);
    void setSkipCounter(uint8_t channel, uint8_t value);
    uint8_t decreaseSkipCounter(uint8_t channel);
    uint8_t getSkipCounter(uint8_t channel);

    void handleMeters(uint8_t number);

private:
    uint8_t _slaveID;
    uint8_t _RegisterPos;          // 0 = MSB first            -    1 = LSB first
    uint8_t _RegisterStart;        // 0 = first address '0'    -    1 = first address '1'

    static bool _readyToSend[MOD_ChannelCount];										  
    
    typedef union Values
    {
        uint8_t lValueUint8_t;
        uint16_t lValueUint16_t;
        int16_t lValueInt16_t;
        uint32_t lValueUint32_t;
        int32_t lValueInt32_t;
        float lValue;
        bool lValueBool;
    } values_t;
    
    static uint32_t sendDelay[MOD_ChannelCount];
    static bool valueValid[MOD_ChannelCount];
    static bool errorState[MOD_ChannelCount][2];
    static uint8_t _skipCounter[MOD_ChannelCount];

    static values_t lastSentValue[MOD_ChannelCount];

    //virtuelle Zähler
    static float powerZ[maxMeters];
    static float counterZ[maxMeters];
    static float lastSentPowerZ[maxMeters];
    static float lastSentCounterZ[maxMeters];
    static float powerZ1;
    static float powerZ2;
    static float powerZ3;
    static float powerZ4;
    static float counterZ1;
    static float counterZ2;
    static float counterZ3;
    static float counterZ4;
    

    void printDebugResult(const char* dpt, uint16_t registerAddr, uint8_t result);
    bool modbusToKnx(uint8_t dpt, uint8_t channel, bool readRequest);
    bool knxToModbus(uint8_t dpt, uint8_t channel, bool readRequest);
    void debugMsgClear(uint8_t channel);
    void debugMsgSet(uint8_t channel);
    void processMeter(uint8_t channel, float newValue);
    void ErrorHandling(uint8_t channel);
    void ErrorHandlingLED();
    uint16_t adjustRegisterAddress(uint16_t u16ReadAddress);
    uint8_t sendProtocol(uint8_t channel, uint16_t registerAddr, uint16_t u16value);
    uint32_t convertFloatTo32Bit(float num);

};