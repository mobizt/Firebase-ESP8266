#include "./core/Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40405)
#error "Mixed versions compilation."
#endif

#ifndef FirebaseFS_H
#define FirebaseFS_H

#pragma once

#include <Arduino.h>
#include "mbfs/MB_MCU.h"

/**📍 Default Main Class */
#define FIREBASE_ESP8266_CLIENT 1

/** 📌 Predefined Build Options
 * ⛔ Use following build flag to disable all predefined options.
 * -D FIREBASE_DISABLE_ALL_OPTIONS
 */

/**📍 For enabling the device or library time setup from NTP server
 * ⛔ Use following build flag to disable.
 * -D DISABLE_NTP_TIME
 */
#define ENABLE_NTP_TIME

/**📍 For enabling the error string from error reason
 * ⛔ Use following build flag to disable.
 * -D DISABLE_ERROR_STRING
 */
#define ENABLE_ERROR_STRING

/**📍 For RTDB class compilation
 * ⛔ Use following build flag to disable.
 * -D DISABLE_RTDB
 */
#define FIREBASE_ENABLE_RTDB

/**📍 For RTDB Error Queue compilation
 * ⛔ Use following build flag to disable.
 * -D DISABLE_ERROR_QUEUE
 */
#define FIREBASE_ENABLE_ERROR_QUEUE

/**📍 For Firebase Cloud Messaging compilation
 * ⛔ Use following build flag to disable.
 * -D DISABLE_FCM
 */
#define FIREBASE_ENABLE_FCM

/**📍 For enabling PSRAM support
 * ⛔ Use following build flag to disable.
 * -D DISABLE_PSRAM
 */
#define FIREBASE_USE_PSRAM

/**📍 For enabling OTA updates support via RTDB, Firebase Storage and Google Cloud Storage buckets
 * ⛔ Use following build flag to disable.
 * -D DISABLE_OTA
 */
#define ENABLE_OTA_FIRMWARE_UPDATE

/**📍 For enabling Keep Alive connection mode
 * ⛔ Use following build flag to disable.
 * -D DISABLE_KEEP_ALIVE
 */
#define USE_CONNECTION_KEEP_ALIVE_MODE

/**📌 For enabling flash filesystem support
 *
 * 📍 For SPIFFS
 * #define DEFAULT_FLASH_FS SPIFFS
 *
 *
 * 📍 For LittleFS Filesystem
 * #include <LittleFS.h>
 * #define DEFAULT_FLASH_FS LittleFS
 *
 *
 * 📍 For SPIFFS Filesystem
 * #if defined(ESP32)
 * #include <SPIFFS.h>
 * #endif
 * #define DEFAULT_FLASH_FS SPIFFS
 *
 *
 * 📍 For FAT Filesystem
 * #include <FFat.h>
 * #define DEFAULT_FLASH_FS FFat  //For ESP32 FAT
 *
 * 🚫 Use following build flags to disable.
 * -D DISABLE_FLASH or -DDISABLE_FLASH in PlatformIO
 */

#if defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO)

#if defined(ESP8266) || defined(MB_ARDUINO_PICO)

#include <LittleFS.h>
#define DEFAULT_FLASH_FS LittleFS

#elif defined(ESP_ARDUINO_VERSION) /* ESP32 core >= v2.0.x */ /* ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(2, 0, 0) */

#include <LittleFS.h>
#define DEFAULT_FLASH_FS LittleFS

#else

#include <SPIFFS.h>
#define DEFAULT_FLASH_FS SPIFFS

#endif

#endif

// For ESP32, format SPIFFS or FFat if mounting failed
#define FORMAT_FLASH_IF_MOUNT_FAILED 1

/**📌 For enabling SD filesystem support
 *
 * 📍 For SD
 * #include <SD.h>
 * #define DEFAULT_SD_FS SD
 * #define CARD_TYPE_SD 1
 *
 * 📍 For SD MMC (ESP32)
 * #include <SD_MMC.h>
 * #define CARD_TYPE_SD_MMC SD_MMC //For ESP32 SDMMC
 * #define ESP_MAIL_CARD_TYPE_SD_MMC 1
 *
 * 📍 For SdFat on ESP32 and other devices except for ESP8266
 * #include <SdFat.h> //https://github.com/greiman/SdFat
 * static SdFat sd_fat_fs;   //should declare as static here
 * #define DEFAULT_SD_FS sd_fat_fs
 * #define CARD_TYPE_SD 1
 * #define SD_FS_FILE SdFile
 *
 *
 * ⛔ Use following build flags to disable.
 * -D DISABLE_SD or -DDISABLE_SD in PlatformIO
 */
#if defined(ESP32) || defined(ESP8266)
#include <SD.h>
#define DEFAULT_SD_FS SD
#define CARD_TYPE_SD 1
#elif defined(MB_ARDUINO_PICO)
// Use SDFS (ESP8266SdFat) instead of SD
#include <SDFS.h>
#define DEFAULT_SD_FS SDFS
#define CARD_TYPE_SD 1
#endif

/** 🔖 Optional Build Options
 *
 *
 * 🏷️ For ENC28J60 Ethernet module support in ESP8266
 * #define ENABLE_ESP8266_ENC28J60_ETH
 *
 * 🏷️ For W5500 Ethernet module support in ESP8266
 * #define ENABLE_ESP8266_W5500_ETH
 *
 * 🏷️ For W5100 Ethernet module support in ESP8266
 * #define ENABLE_ESP8266_W5100_ETH
 * 
 * 🏷️ For disabling on-board WiFI functionality in case external Client usage
 * #define FIREBASE_DISABLE_ONBOARD_WIFI
 *
 * 🏷️ For disabling native (sdk) Ethernet functionality in case external Client usage
 * #define FIREBASE_DISABLE_NATIVE_ETHERNET
 * 
 * 🏷️ For debug port assignment
 * #define FIREBASE_DEFAULT_DEBUG_PORT Serial
 *
 */

#if __has_include("CustomFirebaseFS.h")
#include "CustomFirebaseFS.h"
#endif

#include "./core/Firebase_Build_Options.h"

#endif
