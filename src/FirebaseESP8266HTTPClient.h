/**
 * HTTP Client wrapper v1.1.5
 * 
 * This library provides ESP8266 to perform REST API by GET PUT, POST, PATCH, DELETE data from/to with Google's Firebase database using get, set, update
 * and delete calls. 
 * 
 * The library was test and work well with ESP32s based module and add support for multiple stream event path.
 * 
 * The MIT License (MIT)
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
 * 
 * 
 * Permission is hereby granted, free of charge, to any person returning a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef FirebaseESP8266HTTPClient_H
#define FirebaseESP8266HTTPClient_H

#ifdef ESP8266

//ARDUINO_ESP8266_GIT_VER
//2.6.2 0xbc204a9b
//2.6.1 0x482516e3
//2.6.0 0x643ec203
//2.5.2 0x8b899c12
//2.5.1 0xac02aff5
//2.5.0 0x951aeffa
//2.5.0-beta3 0x21db8fc9
//2.5.0-beta2 0x0fd86a07
//2.5.0-beta1 0x9c1e03a1
//2.4.2 0xbb28d4a3
//2.4.1 0x614f7c32
//2.4.0 0x4ceabea9
//2.4.0-rc2 0x0c897c37
//2.4.0-rc1 0xf6d232f1

#include <Arduino.h>
#include <core_version.h>
#include <time.h>

#ifndef ARDUINO_ESP8266_GIT_VER
#error Your ESP8266 Arduino Core SDK is outdated, please update. From Arduino IDE go to Boards Manager and search 'esp8266' then select the latest version.
#endif

//2.6.1 BearSSL bug
#if ARDUINO_ESP8266_GIT_VER == 0x482516e3
#error Due to bugs in BearSSL in ESP8266 Arduino Core SDK version 2.6.1, please update ESP8266 Arduino Core SDK to newer version. The issue was found here https:\/\/github.com/esp8266/Arduino/issues/6811.
#endif

#if ARDUINO_ESP8266_GIT_VER != 0xf6d232f1 && ARDUINO_ESP8266_GIT_VER != 0x0c897c37 && ARDUINO_ESP8266_GIT_VER != 0x4ceabea9 && ARDUINO_ESP8266_GIT_VER != 0x614f7c32 && ARDUINO_ESP8266_GIT_VER != 0xbb28d4a3
#include <WiFiClientSecure.h>
#include <CertStoreBearSSL.h>
#define FB_ESP8266_SSL_CLIENT BearSSL::WiFiClientSecure

#elif ARDUINO_ESP8266_GIT_VER == 0xbb28d4a3
#define USING_AXTLS
#include <WiFiClientSecureAxTLS.h>
using namespace axTLS;
#define FB_ESP8266_SSL_CLIENT axTLS::WiFiClientSecure
#else
#define USING_AXTLS
#include <WiFiClientSecure.h>
#define FB_ESP8266_SSL_CLIENT WiFiClientSecure
#endif

#define FS_NO_GLOBALS
#include <FS.h>
#include <SD.h>
#include "FirebaseFS.h"

#ifdef USE_LITTLEFS
#include <LittleFS.h>
#define FLASH_FS LittleFS
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#define FLASH_FS SPIFFS
#endif

#if __has_include(<WiFiEspAT.h>) || __has_include(<espduino.h>)
#error WiFi UART bridge was not supported.
#endif

#define FIREBASE_DEFAULT_TCP_TIMEOUT 5000 //5 seconds

/// HTTP client errors
#define FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_REFUSED (-1)
#define FIREBASE_ERROR_HTTPC_ERROR_SEND_HEADER_FAILED (-2)
#define FIREBASE_ERROR_HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
#define FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED (-4)
#define FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_LOST (-5)
#define FIREBASE_ERROR_HTTPC_ERROR_NO_STREAM (-6)
#define FIREBASE_ERROR_HTTPC_ERROR_NO_HTTP_SERVER (-7)
#define FIREBASE_ERROR_HTTPC_ERROR_TOO_LESS_RAM (-8)
#define FIREBASE_ERROR_HTTPC_ERROR_ENCODING (-9)
#define FIREBASE_ERROR_HTTPC_ERROR_STREAM_WRITE (-10)
#define FIREBASE_ERROR_HTTPC_ERROR_READ_TIMEOUT (-11)
#define FIREBASE_ERROR_BUFFER_OVERFLOW (-12)
#define FIREBASE_ERROR_DATA_TYPE_MISMATCH -13
#define FIREBASE_ERROR_PATH_NOT_EXIST -14
#define FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_INUSED -15
#define FIREBASE_ERROR_HTTPC_NO_FCM_TOPIC_PROVIDED -16
#define FIREBASE_ERROR_HTTPC_NO_FCM_DEVICE_TOKEN_PROVIDED -17
#define FIREBASE_ERROR_HTTPC_NO_FCM_SERVER_KEY_PROVIDED -18
#define FIREBASE_ERROR_HTTPC_NO_FCM_INDEX_NOT_FOUND_IN_DEVICE_TOKEN_PROVIDED -19
#define FIREBASE_ERROR_HTTPC_MAX_REDIRECT_REACHED -20
#define FIREBASE_ERROR_EXPECTED_JSON_DATA -21
#define FIREBASE_ERROR_CANNOT_CONFIG_TIME -22
#define FIREBASE_ERROR_SSL_RX_BUFFER_SIZE_TOO_SMALL -23
#define FIREBASE_ERROR_FILE_IO_ERROR -24
#define FIREBASE_ERROR_UNINITIALIZED -25

#define FIREBASE_ERROR_TOKEN_SET_TIME -26
#define FIREBASE_ERROR_TOKEN_PARSE_PK -27
#define FIREBASE_ERROR_TOKEN_SIGN -28
#define FIREBASE_ERROR_TOKEN_EXCHANGE -29
#define FIREBASE_ERROR_TOKEN_NOT_READY -30

/// HTTP codes see RFC7231

#define FIREBASE_ERROR_HTTP_CODE_OK 200
#define FIREBASE_ERROR_HTTP_CODE_NON_AUTHORITATIVE_INFORMATION 203
#define FIREBASE_ERROR_HTTP_CODE_NO_CONTENT 204
#define FIREBASE_ERROR_HTTP_CODE_MOVED_PERMANENTLY 301
#define FIREBASE_ERROR_HTTP_CODE_FOUND 302
#define FIREBASE_ERROR_HTTP_CODE_USE_PROXY 305
#define FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT 307
#define FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT 308
#define FIREBASE_ERROR_HTTP_CODE_BAD_REQUEST 400
#define FIREBASE_ERROR_HTTP_CODE_UNAUTHORIZED 401
#define FIREBASE_ERROR_HTTP_CODE_FORBIDDEN 403
#define FIREBASE_ERROR_HTTP_CODE_NOT_FOUND 404
#define FIREBASE_ERROR_HTTP_CODE_METHOD_NOT_ALLOWED 405
#define FIREBASE_ERROR_HTTP_CODE_NOT_ACCEPTABLE 406
#define FIREBASE_ERROR_HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED 407
#define FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT 408
#define FIREBASE_ERROR_HTTP_CODE_LENGTH_REQUIRED 411
#define FIREBASE_ERROR_HTTP_CODE_PRECONDITION_FAILED 412
#define FIREBASE_ERROR_HTTP_CODE_PAYLOAD_TOO_LARGE 413
#define FIREBASE_ERROR_HTTP_CODE_URI_TOO_LONG 414
#define FIREBASE_ERROR_HTTP_CODE_MISDIRECTED_REQUEST 421
#define FIREBASE_ERROR_HTTP_CODE_UNPROCESSABLE_ENTITY 422
#define FIREBASE_ERROR_HTTP_CODE_TOO_MANY_REQUESTS 429
#define FIREBASE_ERROR_HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE 431
#define FIREBASE_ERROR_HTTP_CODE_INTERNAL_SERVER_ERROR 500
#define FIREBASE_ERROR_HTTP_CODE_NOT_IMPLEMENTED 501
#define FIREBASE_ERROR_HTTP_CODE_BAD_GATEWAY 502
#define FIREBASE_ERROR_HTTP_CODE_SERVICE_UNAVAILABLE 503
#define FIREBASE_ERROR_HTTP_CODE_GATEWAY_TIMEOUT 504
#define FIREBASE_ERROR_HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED 505
#define FIREBASE_ERROR_HTTP_CODE_LOOP_DETECTED 508
#define FIREBASE_ERROR_HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED 511


class FirebaseESP8266HTTPClient
{

  friend class FirebaseESP8266;
  friend class FirebaseData;
  friend class FCMObject;

public:
  FirebaseESP8266HTTPClient();
  ~FirebaseESP8266HTTPClient();

  bool begin(const char *host, uint16_t port);

  bool connected(void);

  int send(const char *header, const char *payload);

  bool send(const char *header);

  WiFiClient *stream(void);

  void setCACert(const char *caCert);
  void setCACertFile(std::string &caCertFile, uint8_t storageType, uint8_t sdPin);
  bool connect(void);


protected:
  std::unique_ptr<FB_ESP8266_SSL_CLIENT> _wcs = std::unique_ptr<FB_ESP8266_SSL_CLIENT>(new FB_ESP8266_SSL_CLIENT());
  std::unique_ptr<char> _cacert;
  std::string _host = "";
  uint16_t _port = 0;

  std::string _CAFile = "";
  uint8_t _CAFileStoreageType = 0;

  int _certType = -1;
  uint16_t timeout = FIREBASE_DEFAULT_TCP_TIMEOUT;

  uint8_t _sdPin = 15;
  bool _clockReady = false;
  uint16_t _bsslRxSize = 512;
  uint16_t _bsslTxSize = 512;
  bool fragmentable = false;
  int chunkSize = 1024;
  bool mflnChecked = false;
};

#endif /* ESP8266 */

#endif /* FirebaseESP8266HTTPClient_H */