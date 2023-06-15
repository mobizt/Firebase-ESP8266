#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40313)
#error "Mixed versions compilation."
#endif

#ifndef FirebaseFS_H
#define FirebaseFS_H

#include <Arduino.h>
#include "mbfs/MB_MCU.h"

#pragma once

#define FIREBASE_ESP8266_CLIENT 1

/**
 * To use other flash file systems
 *
 * LittleFS File system
 *
 * #include <LittleFS.h>
 * #define DEFAULT_FLASH_FS LittleFS //For ESP8266 LitteFS
 *
 *
 * FAT File system
 *
 * #include <FFat.h>
 * #define DEFAULT_FLASH_FS FFat  //For ESP32 FAT
 *
 */
#if defined(ESP8266)
#define DEFAULT_FLASH_FS SPIFFS
#elif defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_NANO_RP2040_CONNECT)
#include <LittleFS.h>
#define DEFAULT_FLASH_FS LittleFS
#endif

/**
 * To use SD card file systems with different hardware interface
 * e.g. SDMMC hardware bus on the ESP32
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD#faq
 *
 * #include <SD_MMC.h>
 * #define DEFAULT_SD_FS SD_MMC //For ESP32 SDMMC
 *
 */

#if defined(ESP8266)
#include <SD.h>
#define DEFAULT_SD_FS SD
#define CARD_TYPE_SD 1
#elif defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_NANO_RP2040_CONNECT)
// Use SDFS (ESP8266SdFat) instead of SD
#include <SDFS.h>
#define DEFAULT_SD_FS SDFS
#define CARD_TYPE_SD 1
#endif

// For RTDB legacy token usage only
// #define USE_LEGACY_TOKEN_ONLY

// Enable the error string from fbdo.errorReason */
// You can get the error code from fbdo.errorCode() when disable this option
#define ENABLE_ERROR_STRING

// Comment to exclude the Firebase Realtime Database
#define ENABLE_RTDB

#define ENABLE_ERROR_QUEUE

// Comment to exclude Firebase Cloud Messaging
#define ENABLE_FCM

/** Use PSRAM for supported ESP8266 module */
#define FIREBASE_USE_PSRAM

// To enable OTA updates
#define ENABLE_OTA_FIRMWARE_UPDATE

// Use Keep Alive connection mode
#define USE_CONNECTION_KEEP_ALIVE_MODE

// For ESP8266 ENC28J60 Ethernet module
// #define ENABLE_ESP8266_ENC28J60_ETH

// For ESP8266 W5100 Ethernet module
// #define ENABLE_ESP8266_W5100_ETH

// For ESP8266 W5500 Ethernet module
// #define ENABLE_ESP8266_W5500_ETH

// To enable external Client for ESP8266.
// This will enable automatically for other devices.
// #define FB_ENABLE_EXTERNAL_CLIENT

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
// You can create your own header file "CustomFirebaseFS.h" in the same diectory of
// "FirebaseFS.h" and put your own custom config to overwrite or
// change the default config in "FirebaseFS.h".
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://

/** This is an example of "CustomFirebaseFS.h"

#pragma once

#ifndef CustomFirebaseFS_H
#define CustomFirebaseFS_H

// To enable external Client for ESP8266 and Raspberry Pi Pico.
// This will enable automatically for other devices.
//  #define FB_ENABLE_EXTERNAL_CLIENT

// For ESP8266 W5100 Ethernet module
// #define ENABLE_ESP8266_W5100_ETH

// For ESP8266 W5500 Ethernet module
// #define ENABLE_ESP8266_W5500_ETH

// Use LittleFS instead of SPIFFS
#include "LittleFS.h"
#undef DEFAULT_FLASH_FS // remove Flash FS defined macro
#define DEFAULT_FLASH_FS LittleFS

// Disable Error Queue and FCM.
#undef ENABLE_ERROR_QUEUE
#undef ENABLE_FCM

#endif

*/
#if __has_include("CustomFirebaseFS.h")
#include "CustomFirebaseFS.h"
#endif

#endif

/////////////////////////////////// WARNING ///////////////////////////////////
// Using RP2040 Pico Arduino SDK, FreeRTOS with LittleFS will cause device hangs
// when write the data to flash filesystem.
// Do not include FreeRTOS.h or even it excluded from compilation by using macro, it  issue.