#pragma once
#include <Arduino.h>

#include <knx.h>
#include "ModbusMaster.h"
#include "Modbus.h"
//#include "hardware.h"
#include "ModbusGateway.h"
#include "Device.h"
#include "KnxHelper.h"
#include "wiring_private.h" // pinPeripheral() function

// instantiate ModbusMaster object
Modbus Slave[MaxCountSlaves];
Modbus TestSlave;

uint32_t ModbusDelay = 0;
uint32_t ModbusCycle = 0;
uint32_t TestDelay = 0;

// Serial Settings ModBus
Uart Serial3(&sercom2, 3, 4, SERCOM_RX_PAD_1, UART_TX_PAD_0); //+pinPeripheral

void SERCOM2_Handler()
{
    Serial3.IrqHandler();
}

void preTransmission()
{
#ifdef LED_YELLOW_PIN
    digitalWrite(MAX485_RE_NEG, 1);
#endif
    digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
#ifdef LED_YELLOW_PIN
    digitalWrite(MAX485_RE_NEG, 0);
#endif
    digitalWrite(MAX485_DE, 0);
}

void idle()
{
    Schedule::loop();
    knx.loop();
}

bool modbusParitySerial(uint32_t baud)
{

    switch (knx.paramByte(MOD_BusParitySelection))
    {
    case 0: // Even (1 stop bit)
        Serial3.begin(baud, SERIAL_8E1);
        SERIAL_DEBUG.println("Parity: Even (1 stop bit)");
        return true;
        break;
    case 1: // Odd (1 stop bit)
        Serial3.begin(baud, SERIAL_8O1);
        SERIAL_DEBUG.println("Parity: Odd (1 stop bit)");
        return true;
        break;
    case 2: // None (2 stop bits)
        Serial3.begin(baud, SERIAL_8N2);
        SERIAL_DEBUG.println("Parity: None (2 stop bits)");
        return true;
        break;
    case 3: // None (1 stop bit)
        Serial3.begin(baud, SERIAL_8N1);
        SERIAL_DEBUG.println("Parity: None (1 stop bit)");
        return true;
        break;

    default:
        SERIAL_DEBUG.print("Parity: Error: ");
        SERIAL_DEBUG.println(knx.paramByte(MOD_BusParitySelection));
        return false;
        break;
    }
}

bool modbusInitSerial()
{
    // Set Modbus communication baudrate
    switch (knx.paramByte(MOD_BusBaudrateSelection))
    {
    case 0:
        SERIAL_DEBUG.println("Baudrate: 1200kBit/s");
        return modbusParitySerial(1200);

        break;
    case 1:
        SERIAL_DEBUG.println("Baudrate: 2400kBit/s");
        return modbusParitySerial(2400);
        break;
    case 2:
        SERIAL_DEBUG.println("Baudrate: 4800kBit/s");
        return modbusParitySerial(4800);
        break;
    case 3:
        SERIAL_DEBUG.println("Baudrate: 9600kBit/s");
        return modbusParitySerial(9600);
        break;
    case 4:
        SERIAL_DEBUG.println("Baudrate: 19200kBit/s");
        return modbusParitySerial(19200);
        break;
    case 5:
        SERIAL_DEBUG.println("Baudrate: 38400kBit/s");
        return modbusParitySerial(38400);
        break;
    case 6:
        SERIAL_DEBUG.println("Baudrate: 56000kBit/s");
        return modbusParitySerial(56000);
        break;
    case 7:
        SERIAL_DEBUG.println("Baudrate: 115200kBit/s");
        return modbusParitySerial(115200);
        break;
    default:
        SERIAL_DEBUG.print("Baudrate: Error: ");
        SERIAL_DEBUG.println(knx.paramByte(MOD_BusBaudrateSelection));
        return false;
        break;
    }
}

void modbusInitSlaves()

{
    // Test Slave
    TestSlave.initSlave(1, Serial3, 1, 1);
    TestSlave.preTransmission(preTransmission);
    TestSlave.postTransmission(postTransmission);
    TestSlave.idle(idle);

    for (uint8_t slaveIdx = 0; slaveIdx < MaxCountSlaves; slaveIdx++) {
        uint8_t slaveOffset = slaveIdx * (MOD_BusID_Slave2 - MOD_BusID_Slave1);
#ifdef Serial_Debug_Modbus
        SERIAL_DEBUG.print("Slave");
        SERIAL_DEBUG.print(slaveIdx+1);
        SERIAL_DEBUG.print(" ID: ");
        SERIAL_DEBUG.println(knx.paramInt(MOD_BusID_Slave1 + slaveOffset));
#endif
        // Modbus slave
        Slave[slaveIdx].initSlave(knx.paramInt(MOD_BusID_Slave1 + slaveOffset), Serial3,
                                  knx.paramByte(MOD_BusByteOrderSelectionSlave1 + slaveOffset),
                                  knx.paramByte(MOD_BusWordOrderSelectionSlave1 + slaveOffset));
        Slave[slaveIdx].preTransmission(preTransmission);
        Slave[slaveIdx].postTransmission(postTransmission);
        Slave[slaveIdx].idle(idle);
    }

    // last call to set the right Serial3 pins
    pinPeripheral(3, PIO_SERCOM_ALT);
    pinPeripheral(4, PIO_SERCOM_ALT);
}

bool ModbusRead(uint8_t usedModbusChannels)
{
    static uint8_t channel = 0;
    static uint8_t channel2 = 0;

    // Reading from Modbus
    if (channel2 < usedModbusChannels && delayCheck(ModbusCycle, (knx.paramByte(MOD_BusDelayCycle) * 1000)))
    {
        if (delayCheck(ModbusDelay, 50 + (knx.paramByte(MOD_BusDelayRequest) * 10)))
        {
            uint8_t slaveNumber = knx.paramByte(getPar(MOD_CHModbusSlaveSelection, channel2)) - 1;
            if (slaveNumber < MaxCountSlaves)
            {
#ifdef Serial_Debug_Modbus_Min 
                    SERIAL_DEBUG.print("CH");
                    SERIAL_DEBUG.print(channel2 + 1);
                    SERIAL_DEBUG.print(" S");
                    SERIAL_DEBUG.print(slaveNumber + 1);
                    SERIAL_DEBUG.print(" ID:");
                    SERIAL_DEBUG.print(Slave[slaveNumber].getSlaveID());
#endif
                //Prüft ob dieser CH bei der letzten Abfrage einen ERROR zurückgegeben hat
                if (Slave[slaveNumber].getErrorState1(channel2) == true && Slave[slaveNumber].getSkipCounter(channel2) > 0)
                {
                    Slave[slaveNumber].decreaseSkipCounter(channel2);
#ifdef Serial_Debug_Modbus_Min
                    SERIAL_DEBUG.print(" | Error skiped ");
                    SERIAL_DEBUG.println(Slave[slaveNumber].getSkipCounter(channel2));
#endif
                }
                // Die letzte Abfrage war Fehlerfrei -> eine neue Abfrage darf gestartet werden
                else
                {
#ifdef Serial_Debug_Modbus_Min
                    SERIAL_DEBUG.print(" Val: ");
#endif
                    Slave[slaveNumber].readModbus(channel2, true);

                    // Im Error Fall wir hier der SkipCounter neu gesetzt
                    if (Slave[slaveNumber].getErrorState1(channel2) == true && Slave[slaveNumber].getSkipCounter(channel2) == 0)
                    {
                        if (Slave[slaveNumber].getSkipCounter(channel2) == 0)
                        {
                            if (Slave[slaveNumber].getErrorState2(channel2) == true)
                                Slave[slaveNumber].setSkipCounter(channel2, 10);
                            else
                                Slave[slaveNumber].setSkipCounter(channel2, 2);
                        }
                    }
                }
                ModbusDelay = millis();
            }
            else
            {
                ModbusDelay = millis() + 1000;
            }

            if (++channel2 == usedModbusChannels)
            {
                channel2 = 0;
                ModbusCycle = millis();
                Slave[0].handleMeters(1);
                Slave[0].handleMeters(2);
                Slave[0].handleMeters(3);
                Slave[0].handleMeters(4);
            }
        }
    }

    // Regular KNX sending
    if (channel < usedModbusChannels)
    {
        uint8_t slaveNumber = knx.paramByte(getPar(MOD_CHModbusSlaveSelection, channel)) - 1;
        if (slaveNumber < MaxCountSlaves)
        {
            Slave[slaveNumber].readModbus(channel, false);
        }

        if (++channel == usedModbusChannels)
        {
            channel = 0;
        }
    }

    return true;
}