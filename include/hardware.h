#pragma once
#include "HardwareConfig.h"

#ifdef ARDUINO_ARCH_RP2040
//     #include "OpenKNXHardware.h"

//     #ifdef BOARD_MASIFI_SENSOR_BREAKOUT
//         // #define OPENKNX_RECOVERY_ON LOW
//         #define PROG_LED_PIN 1
//         #define PROG_LED_PIN_ACTIVE_ON HIGH
//         #define INFO_LED_PIN 3
//         #define INFO_LED_PIN_ACTIVE_ON HIGH
//         #define PROG_BUTTON_PIN 0
//         #define PROG_BUTTON_PIN_INTERRUPT_ON FALLING
//         #define SAVE_INTERRUPT_PIN 22
//         #define KNX_UART_RX_PIN 17
//         #define KNX_UART_TX_PIN 16
//         #define HARDWARE_NAME "MASIFI-SENSOR-BREAKOUT"
//     // #define SmartMF_HardwareVariant_PIN 26
//     // #define SmartMF_HardwareRevision_PIN1 10
//     // #define SmartMF_HardwareRevision_PIN2 11
//     // #define SmartMF_HardwareRevision_PIN3 12
//     #endif

    #ifdef BOARD_PIPICOW
        #define PROG_LED_PIN 32
        #define PROG_LED_PIN_ACTIVE_ON HIGH
    #endif
#endif

#ifdef ARDUINO_ARCH_SAMD
//     #ifdef BOARD_KONNEKTING_MI
//         #define PROG_LED_PIN A3
//         #define PROG_LED_PIN_ACTIVE_ON HIGH
//         #define PROG_BUTTON_PIN 38
//         #define PROG_BUTTON_PIN_INTERRUPT_ON FALLING
//         #define INFO_LED_PIN A5
//         #define INFO_LED_PIN_ACTIVE_ON HIGH
//         #define HARDWARE_NAME "KONNEKTING-MI"
//         #define OPENKNX_NO_BOOT_PULSATING 1
//     #endif
    #ifdef DEVICE_WEMOS_MINI_D1
        #define PROG_LED_PIN 3
        #define PROG_LED_PIN_ACTIVE_ON HIGH
        #define PROG_BUTTON_PIN 5
        #define PROG_BUTTON_PIN_INTERRUPT_ON FALLING
        #define INFO_LED_PIN 4
        #define INFO_LED_PIN_ACTIVE_ON HIGH
        #define HARDWARE_NAME "WEMOS-MINI-D1"
    #endif

//     #ifdef BOARD_MASIFI_V3
//         #define PROG_LED_PIN 13
//         #define PROG_LED_PIN_ACTIVE_ON HIGH
//         #define PROG_BUTTON_PIN 11
//         #define PROG_BUTTON_PIN_INTERRUPT_ON FALLING
//         #define SAVE_INTERRUPT_PIN A2 // 8
//         #define INFO_LED_PIN 38
//         #define INFO_LED_PIN_ACTIVE_ON HIGH
//         #define COUNT_1WIRE_BUSMASTER 1
//         #define COUNT_1WIRE_CHANNEL 30
//         // #define COUNT_PM_CHANNEL 6
//         // #define COUNT_LOG_CHANNEL 80
//         // Buzzer
//         #define BUZZER_PIN 9
//         #define I2C_1WIRE_DEVICE_ADDRESSS 0x18  // Address of DS2484 1-Wire-Busmaster chip
//         #define I2C_EEPROM_DEVICE_ADDRESSS 0x50 // Address of 24LC256 eeprom chip
//         #define I2C_RGBLED_DEVICE_ADDRESS 0x60  // Address of PCA9632 RBGW-LED-Driver
//     #endif
#endif

#ifdef ARDUINO_ARCH_ESP32
    #ifdef BOARD_ESP32_DEV
        #define PROG_LED_PIN 2
        #define PROG_LED_PIN_ACTIVE_ON HIGH
        // #define PROG_BUTTON_PIN 35
        // #define PROG_BUTTON_PIN_INTERRUPT_ON FALLING
        #define DEVICE_ID "ESP32-DEV"
    #endif
//     #ifdef BOARD_ESP32_WT32_ETH01
//         #define PROG_LED_PIN 2
//         #define PROG_LED_PIN_ACTIVE_ON HIGH
//         #define PROG_BUTTON_PIN 4
//         #define PROG_BUTTON_PIN_INTERRUPT_ON FALLING

//         #define ETH_PHY_TYPE ETH_PHY_LAN8720
//         #define ETH_PHY_ADDR 1
//         #define ETH_PHY_MDC 23
//         #define ETH_PHY_MDIO 18
//         #define ETH_PHY_POWER 16
//         #define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN

//         #define HARDWARE_NAME "ESP32_WT32_ETH01"
//     #endif
#endif
