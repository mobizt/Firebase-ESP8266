/*
 * HTTP Client wrapper v1.0.9
 * 
 * This library provides ESP8266 to perform REST API by GET PUT, POST, PATCH, DELETE data from/to with Google's Firebase database using get, set, update
 * and delete calls. 
 * 
 * The library was test and work well with ESP32s based module and add support for multiple stream event path.
 * 
 * The MIT License (MIT)
 * Copyright (c) 2019 K. Suwatchai (Mobizt)
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
#define SSL_CLIENT BearSSL::WiFiClientSecure

#elif ARDUINO_ESP8266_GIT_VER == 0xbb28d4a3
#define USING_AXTLS
#include <WiFiClientSecureAxTLS.h>
    using namespace axTLS;
#define SSL_CLIENT axTLS::WiFiClientSecure
#else
#define USING_AXTLS
#include <WiFiClientSecure.h>
#define SSL_CLIENT WiFiClientSecure
#endif

#define FS_NO_GLOBALS
#include <FS.h>
#include <SD.h>

/// HTTP client errors
#define HTTPC_ERROR_CONNECTION_REFUSED (-1)
#define HTTPC_ERROR_SEND_HEADER_FAILED (-2)
#define HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
#define HTTPC_ERROR_NOT_CONNECTED (-4)
#define HTTPC_ERROR_CONNECTION_LOST (-5)
#define HTTPC_ERROR_NO_STREAM (-6)
#define HTTPC_ERROR_NO_HTTP_SERVER (-7)
#define HTTPC_ERROR_TOO_LESS_RAM (-8)
#define HTTPC_ERROR_ENCODING (-9)
#define HTTPC_ERROR_STREAM_WRITE (-10)
#define HTTPC_ERROR_READ_TIMEOUT (-11)
#define FIREBASE_ERROR_BUFFER_OVERFLOW (-13)
#define FIREBASE_ERROR_DATA_TYPE_MISMATCH (-14)
#define FIREBASE_ERROR_PATH_NOT_EXIST (-15)

/// HTTP codes see RFC7231

#define _HTTP_CODE_OK 200
#define _HTTP_CODE_NON_AUTHORITATIVE_INFORMATION 203
#define _HTTP_CODE_NO_CONTENT 204
#define _HTTP_CODE_MOVED_PERMANENTLY 301
#define _HTTP_CODE_FOUND 302
#define _HTTP_CODE_USE_PROXY 305
#define _HTTP_CODE_TEMPORARY_REDIRECT 307
#define _HTTP_CODE_PERMANENT_REDIRECT 308
#define _HTTP_CODE_BAD_REQUEST 400
#define _HTTP_CODE_UNAUTHORIZED 401
#define _HTTP_CODE_FORBIDDEN 403
#define _HTTP_CODE_NOT_FOUND 404
#define _HTTP_CODE_METHOD_NOT_ALLOWED 405
#define _HTTP_CODE_NOT_ACCEPTABLE 406
#define _HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED 407
#define _HTTP_CODE_REQUEST_TIMEOUT 408
#define _HTTP_CODE_LENGTH_REQUIRED 411
#define _HTTP_CODE_PRECONDITION_FAILED 412
#define _HTTP_CODE_PAYLOAD_TOO_LARGE 413
#define _HTTP_CODE_URI_TOO_LONG 414
#define _HTTP_CODE_MISDIRECTED_REQUEST 421
#define _HTTP_CODE_UNPROCESSABLE_ENTITY 422
#define _HTTP_CODE_TOO_MANY_REQUESTS 429
#define _HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE 431
#define _HTTP_CODE_INTERNAL_SERVER_ERROR 500
#define _HTTP_CODE_NOT_IMPLEMENTED 501
#define _HTTP_CODE_BAD_GATEWAY 502
#define _HTTP_CODE_SERVICE_UNAVAILABLE 503
#define _HTTP_CODE_GATEWAY_TIMEOUT 504
#define _HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED 505
#define _HTTP_CODE_LOOP_DETECTED 508
#define _HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED 511

    class FirebaseHTTPClient
{

  friend class FirebaseESP8266;
  friend class FirebaseData;
  friend class FCMObject;

public:
  FirebaseHTTPClient();
  ~FirebaseHTTPClient();

  bool begin(const std::string host, uint16_t port);
  bool connected(void);
  int sendRequest(const char *header, const char *payload);
  void setRootCA(const char *rootCA);
  void setRootCAFile(std::string &rootCAFile, uint8_t storageType, uint8_t sdPin);
  


protected:
  bool
  connect(void);
  bool sendHeader(const char *header);

  std::string _host = "";
  uint16_t _port = 0;
  std::unique_ptr<SSL_CLIENT> _client = std::unique_ptr<SSL_CLIENT>(new SSL_CLIENT());
  
  int _certType = -1;
  uint16_t timeout = 5000;
  uint8_t _sdPin = 15;
  bool _clockReady = false;
  uint16_t _bsslRxSize = 512;
  uint16_t _bsslTxSize = 512;
};

#endif /* ESP8266 */

#endif /* FirebaseESP8266HTTPClient_H */