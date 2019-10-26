/*
 * Google's Firebase Realtime Database Arduino Library for ESP8266, version 2.6.3
 * 
 * October 26, 2019
 * 
 * Feature Added:
 * - FirebaseJson optimized for flash string usage.
 * 
 * Feature Fixed: 
 * - WiFi client unhandle access.
 * 
 * 
 * This library provides ESP8266 to perform REST API by GET PUT, POST, PATCH, DELETE data from/to with Google's Firebase database using get, set, update
 * and delete calls. 
 * 
 * The library was test and work well with ESP8266 based module and add support for multiple stream event path.
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

#ifndef FirebaseESP8266_CPP
#define FirebaseESP8266_CPP

#include "FirebaseESP8266.h"

struct FirebaseESP8266::FirebaseDataType
{
    static const uint8_t NULL_ = 1;
    static const uint8_t INTEGER = 2;
    static const uint8_t FLOAT = 3;
    static const uint8_t DOUBLE = 4;
    static const uint8_t BOOLEAN = 5;
    static const uint8_t STRING = 6;
    static const uint8_t JSON = 7;
    static const uint8_t ARRAY = 8;
    static const uint8_t BLOB = 9;
    static const uint8_t FILE = 10;
    static const uint8_t TIMESTAMP = 11;
    static const uint8_t SHALLOW = 12;
    static const uint8_t ANY = 13;
};

struct FirebaseESP8266::FirebaseMethod
{
    static const uint8_t PUT = 0;
    static const uint8_t PUT_SILENT = 1;
    static const uint8_t POST = 2;
    static const uint8_t GET = 3;
    static const uint8_t GET_SILENT = 4;
    static const uint8_t STREAM = 5;
    static const uint8_t PATCH = 6;
    static const uint8_t PATCH_SILENT = 7;
    static const uint8_t DELETE = 8;
    static const uint8_t BACKUP = 9;
    static const uint8_t RESTORE = 10;
    static const uint8_t GET_RULES = 11;
    static const uint8_t SET_RULES = 12;
    static const uint8_t GET_SHALLOW = 13;
    static const uint8_t GET_PRIORITY = 14;
    static const uint8_t SET_PRIORITY = 15;
};

struct FirebaseESP8266::FCMMessageType
{
    static const uint8_t SINGLE = 0;
    static const uint8_t MULTICAST = 1;
    static const uint8_t TOPIC = 2;
};

FirebaseESP8266::FirebaseESP8266()
{
}

FirebaseESP8266::~FirebaseESP8266()
{
    std::string().swap(_host);
    std::string().swap(_auth);
    _rootCA = nullptr;
}

void FirebaseESP8266::begin(const String &host, const String &auth)
{
    int p1 = -1;
    int p2 = 0;
    char *tmp = new char[20];
    char *h = new char[host.length() + 1];
    memset(h, 0, host.length() + 1);
    strcpy(h, host.c_str());
    char *_h = new char[host.length() + 1];
    memset(_h, 0, host.length() + 1);

    _host.clear();
    _auth.clear();
    _rootCA = nullptr;
    _rootCAFile.clear();
    _host.reserve(100);
    _auth.reserve(100);

    memset(tmp, 0, 20);
    strcpy_P(tmp, ESP8266_FIREBASE_STR_111);
    p1 = strpos(h, tmp, 0);
    if (p1 != -1)
    {
        if (h[strlen(h) - 1] == '/')
            p2 = 1;

        strncpy(_h, h + p1 + strlen_P(ESP8266_FIREBASE_STR_111), strlen(h) - p1 - p2 - strlen_P(ESP8266_FIREBASE_STR_111));
        _host = _h;
    }

    if (p1 == -1)
    {
        memset(tmp, 0, 20);
        strcpy_P(tmp, ESP8266_FIREBASE_STR_112);
        p1 = strpos(h, tmp, 0);
        if (p1 != -1)
        {
            if (h[strlen(h) - 1] == '/')
                p2 = 1;

            strncpy(_h, h + p1 + strlen_P(ESP8266_FIREBASE_STR_112), strlen(h) - p1 - p2 - strlen_P(ESP8266_FIREBASE_STR_112));
            _host = _h;
        }
    }

    if (_host.length() == 0)
        _host = h;

    _auth = auth.c_str();
    _port = FIEBASE_PORT;
    delete[] h;
    delete[] _h;
    delete[] tmp;
}

void FirebaseESP8266::begin(const String &host, const String &auth, const char *rootCA)
{
    begin(host, auth);
    if (rootCA)
    {
        setClock();
        _rootCA = std::shared_ptr<const char>(rootCA);
    }
}

void FirebaseESP8266::begin(const String &host, const String &auth, const String &rootCAFile, uint8_t storageType)
{
    begin(host, auth);
    if (rootCAFile.length() > 0)
    {
        setClock();
        _rootCAFile = rootCAFile.c_str();
        _rootCAFileStoreageType = storageType;
        if (storageType == StorageType::SD && !_sdOk)
            _sdOk = sdTest();
    }
}

void FirebaseESP8266::reconnectWiFi(bool reconnect)
{
    _reconnectWiFi = reconnect;
    WiFi.setAutoReconnect(reconnect);
}

void FirebaseESP8266::setReadTimeout(FirebaseData &dataObj, int millisec)
{
    if (millisec <= 900000)
        dataObj._readTimeout = millisec;
}

void FirebaseESP8266::setwriteSizeLimit(FirebaseData &dataObj, const String &size)
{
    dataObj._writeLimit = size.c_str();
}

bool FirebaseESP8266::getRules(FirebaseData &dataObj)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, 0, ESP8266_FIREBASE_STR_103, FirebaseMethod::GET_RULES, FirebaseDataType::JSON, "", "", "");
}

bool FirebaseESP8266::setRules(FirebaseData &dataObj, const String &rules)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, 0, ESP8266_FIREBASE_STR_103, FirebaseMethod::SET_RULES, FirebaseDataType::JSON, rules.c_str(), "", "");
}

bool FirebaseESP8266::buildRequest(FirebaseData &dataObj, uint8_t firebaseMethod, uint8_t firebaseDataType, const std::string &path, const char *buff, bool queue, const std::string &priority, const std::string &etag)
{
    if (!reconnect(dataObj))
        return false;

    bool flag;
    dataObj.queryFilter.clear();

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    unsigned long lastTime = millis();

    if (dataObj._streamCall || dataObj._fcmCall)
        while ((dataObj._streamCall || dataObj._fcmCall) && millis() - lastTime < 1000)
            delay(1);

    if (dataObj._streamCall || dataObj._fcmCall)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_INUSED;
        return false;
    }

    for (int i = 0; i < maxRetry; i++)
    {
        flag = sendRequest(dataObj, 0, path.c_str(), firebaseMethod, firebaseDataType, buff, priority, etag);
        if (flag)
            break;

        if (dataObj._httpCode == HTTPC_ERROR_CONNECTION_INUSED && dataObj._firebaseCall)
        {
            if (dataObj._firebaseCall && WiFi.status() != WL_CONNECTED)
                return false;
        }
        else
        {
            if (dataObj._maxRetry > 0)
                if (!flag && commError(dataObj))
                    errCount++;
        }
    }

    dataObj._qID = 0;

    if (!queue && !flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        if (firebaseMethod == FirebaseMethod::PUT || firebaseMethod == FirebaseMethod::PUT_SILENT || firebaseMethod == FirebaseMethod::POST || firebaseMethod == FirebaseMethod::PATCH || firebaseMethod == FirebaseMethod::PATCH_SILENT)
            dataObj.addQueue(firebaseMethod, 0, firebaseDataType, path.c_str(), "", buff, false, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    if (flag)
        dataObj._firebaseCall = false;
    return flag;
}

bool FirebaseESP8266::buildRequestFile(FirebaseData &dataObj, uint8_t storageType, uint8_t firebaseMethod, const std::string &path, const std::string &fileName, bool queue, const std::string &priority, const std::string &etag)
{

    if (!reconnect(dataObj))
        return false;

    dataObj.queryFilter.clear();
    dataObj._fileName = fileName.c_str();

    bool flag;

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    unsigned long lastTime = millis();

    if (dataObj._streamCall || dataObj._fcmCall)
        while ((dataObj._streamCall || dataObj._fcmCall) && millis() - lastTime < 1000)
            delay(1);

    if (dataObj._streamCall || dataObj._fcmCall)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_INUSED;
        return false;
    }

    for (int i = 0; i < maxRetry; i++)
    {

        flag = sendRequest(dataObj, storageType, path.c_str(), firebaseMethod, FirebaseDataType::FILE, "", priority, etag);
        if (flag)
            break;

        if (dataObj._httpCode == HTTPC_ERROR_CONNECTION_INUSED && dataObj._firebaseCall)
        {
            if (dataObj._firebaseCall && WiFi.status() != WL_CONNECTED)
                return false;
        }
        else
        {

            if (dataObj._maxRetry > 0)
                if (!flag && commError(dataObj))
                    errCount++;
        }
    }

    dataObj._qID = 0;

    if (!queue && !flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        dataObj.addQueue(firebaseMethod, storageType, FirebaseDataType::FILE, path.c_str(), fileName.c_str(), "", false, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    if (flag)
        dataObj._firebaseCall = false;

    return flag;
}

bool FirebaseESP8266::pathExist(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    if (sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET_SILENT, FirebaseDataType::STRING, "", "", ""))
        return !dataObj._pathNotExist;
    else
        return false;
}

String FirebaseESP8266::getETag(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    if (sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET_SILENT, FirebaseDataType::STRING, "", "", ""))
        return dataObj._etag.c_str();
    else
        return String();
}

bool FirebaseESP8266::getShallowData(FirebaseData &dataObj, const String &path)
{
    bool flag = false;
    flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET_SHALLOW, FirebaseDataType::STRING, "", "", "");
    return flag;
}

void FirebaseESP8266::enableClassicRequest(FirebaseData &dataObj, bool flag)
{
    dataObj._classicRequest = flag;
}

bool FirebaseESP8266::setPriority(FirebaseData &dataObj, const String &path, float priority)
{
    char *num = new char[20];
    memset(num, 0, 20);
    dtostrf(priority, 7, 6, num);
    char *p = new char[path.length() + strlen_P(ESP8266_FIREBASE_STR_156) + 1];
    strcpy(p, path.c_str());
    strcat_P(p, ESP8266_FIREBASE_STR_156);
    bool flag = buildRequest(dataObj, FirebaseMethod::SET_PRIORITY, FirebaseDataType::FLOAT, p, num, false, "", "");
    delete[] num;
    delete[] p;
    return flag;
}

bool FirebaseESP8266::getPriority(FirebaseData &dataObj, const String &path)
{
    char *p = new char[path.length() + strlen_P(ESP8266_FIREBASE_STR_156) + 1];
    strcpy(p, path.c_str());
    strcat_P(p, ESP8266_FIREBASE_STR_156);
    bool flag = buildRequest(dataObj, FirebaseMethod::GET_PRIORITY, FirebaseDataType::FLOAT, p, "", false, "", "");
    delete[] p;
    return flag;
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, int intValue)
{
    return pushInt(dataObj, path, intValue);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, int intValue, float priority)
{
    return pushInt(dataObj, path, intValue, priority);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, float floatValue)
{
    return pushFloat(dataObj, path, floatValue);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, float floatValue, float priority)
{
    return pushFloat(dataObj, path, floatValue, priority);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, double doubleValue)
{
    return pushDouble(dataObj, path, doubleValue);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, double doubleValue, float priority)
{
    return pushDouble(dataObj, path, doubleValue, priority);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, bool boolValue)
{
    return pushBool(dataObj, path, boolValue);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, bool boolValue, float priority)
{
    return pushBool(dataObj, path, boolValue, priority);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, const char *stringValue)
{
    return pushString(dataObj, path, stringValue);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, const String &stringValue)
{
    return pushString(dataObj, path, stringValue);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, const char *stringValue, float priority)
{
    return pushString(dataObj, path, stringValue, priority);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, const String &stringValue, float priority)
{
    return pushString(dataObj, path, stringValue, priority);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, FirebaseJson &json)
{
    return pushJSON(dataObj, path, json);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, FirebaseJson &json, float priority)
{
    return pushJSON(dataObj, path, json, priority);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, FirebaseJsonArray &arr)
{
    return pushArray(dataObj, path, arr);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, FirebaseJsonArray &arr, float priority)
{
    return pushArray(dataObj, path, arr, priority);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size)
{
    return pushBlob(dataObj, path, blob, size);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size, float priority)
{
    return pushBlob(dataObj, path, blob, size, priority);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, uint8_t storageType, const String &path, const String &fileName)
{
    return pushFile(dataObj, storageType, path, fileName);
}

bool FirebaseESP8266::push(FirebaseData &dataObj, uint8_t storageType, const String &path, const String &fileName, float priority)
{
    return pushFile(dataObj, storageType, path, fileName, priority);
}

template <typename T>
bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, T value)
{
    if (std::is_same<T, int>::value)
        return pushInt(dataObj, path, value);
    else if (std::is_same<T, double>::value)
        return pushDouble(dataObj, path, value);
    else if (std::is_same<T, bool>::value)
        return pushBool(dataObj, path, value);
    else if (std::is_same<T, const char *>::value)
        return pushString(dataObj, path, value);
    else if (std::is_same<T, const String &>::value)
        return pushString(dataObj, path, value);
    else if (std::is_same<T, FirebaseJson &>::value)
        return pushJson(dataObj, path, value);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return pushArray(dataObj, path, value);
}

template <typename T>
bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, T value, size_t size)
{
    if (std::is_same<T, uint8_t *>::value)
        return pushBlob(dataObj, path, value, size);
}

template <typename T>
bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, T value, float priority)
{
    if (std::is_same<T, int>::value)
        return pushInt(dataObj, path, value, priority);
    else if (std::is_same<T, double>::value)
        return pushDouble(dataObj, path, value, priority);
    else if (std::is_same<T, bool>::value)
        return pushBool(dataObj, path, value, priority);
    else if (std::is_same<T, const char *>::value)
        return pushString(dataObj, path, value, priority);
    else if (std::is_same<T, const String &>::value)
        return pushString(dataObj, path, value, priority);
    else if (std::is_same<T, FirebaseJson &>::value)
        return pushJson(dataObj, path, value, priority);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return pushArray(dataObj, path, value, priority);
}

template <typename T>
bool FirebaseESP8266::push(FirebaseData &dataObj, const String &path, T value, size_t size, float priority)
{
    if (std::is_same<T, uint8_t *>::value)
        return pushBlob(dataObj, path, value, size, priority);
}

bool FirebaseESP8266::pushInt(FirebaseData &dataObj, const String &path, int intValue)
{
    return pushInt(dataObj, path.c_str(), intValue, false, "");
}

bool FirebaseESP8266::pushInt(FirebaseData &dataObj, const String &path, int intValue, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = pushInt(dataObj, path.c_str(), intValue, false, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushInt(FirebaseData &dataObj, const std::string &path, int intValue, bool queue, const std::string &priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    itoa(intValue, buff, 10);
    bool flag = buildRequest(dataObj, FirebaseMethod::POST, FirebaseDataType::INTEGER, path, buff, queue, priority, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushFloat(FirebaseData &dataObj, const String &path, float floatValue)
{
    return pushInt(dataObj, path.c_str(), floatValue, false, "");
}

bool FirebaseESP8266::pushFloat(FirebaseData &dataObj, const String &path, float floatValue, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = pushFloat(dataObj, path.c_str(), floatValue, false, buff);
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushFloat(FirebaseData &dataObj, const std::string &path, float floatValue, bool queue, const std::string &priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(floatValue, 7, 6, buff);
    bool flag = buildRequest(dataObj, FirebaseMethod::POST, FirebaseDataType::FLOAT, path, buff, queue, priority, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushDouble(FirebaseData &dataObj, const String &path, double doubleValue)
{
    return pushDouble(dataObj, path.c_str(), doubleValue, false, "");
}

bool FirebaseESP8266::pushDouble(FirebaseData &dataObj, const String &path, double doubleValue, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = pushDouble(dataObj, path.c_str(), doubleValue, false, buff);
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushDouble(FirebaseData &dataObj, const std::string &path, double doubleValue, bool queue, const std::string &priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(doubleValue, 12, 9, buff);
    bool flag = buildRequest(dataObj, FirebaseMethod::POST, FirebaseDataType::DOUBLE, path, buff, queue, priority, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushBool(FirebaseData &dataObj, const String &path, bool boolValue)
{
    return pushBool(dataObj, path.c_str(), boolValue, false, "");
}

bool FirebaseESP8266::pushBool(FirebaseData &dataObj, const String &path, bool boolValue, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = pushBool(dataObj, path.c_str(), boolValue, false, buff);
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushBool(FirebaseData &dataObj, const std::string &path, bool boolValue, bool queue, const std::string &priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);

    if (boolValue)
        strcpy_P(buff, ESP8266_FIREBASE_STR_107);
    else
        strcpy_P(buff, ESP8266_FIREBASE_STR_106);

    bool flag = buildRequest(dataObj, FirebaseMethod::POST, FirebaseDataType::BOOLEAN, path, buff, queue, priority, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushString(FirebaseData &dataObj, const String &path, const String &stringValue)
{
    bool flag = buildRequest(dataObj, FirebaseMethod::POST, FirebaseDataType::STRING, path.c_str(), stringValue.c_str(), false, "", "");
    return flag;
}

bool FirebaseESP8266::pushString(FirebaseData &dataObj, const String &path, const String &stringValue, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = buildRequest(dataObj, FirebaseMethod::POST, FirebaseDataType::STRING, path.c_str(), stringValue.c_str(), false, buff, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushJSON(FirebaseData &dataObj, const String &path, FirebaseJson &json)
{
    std::string s;
    json._toStdString(s);
    bool flag = buildRequest(dataObj, FirebaseMethod::POST, FirebaseDataType::JSON, path.c_str(), s.c_str(), false, "", "");
    std::string().swap(s);
    return flag;
}

bool FirebaseESP8266::pushJSON(FirebaseData &dataObj, const String &path, FirebaseJson &json, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    std::string s;
    json._toStdString(s);
    bool flag = buildRequest(dataObj, FirebaseMethod::POST, FirebaseDataType::JSON, path.c_str(), s.c_str(), false, buff, "");
    std::string().swap(s);
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushArray(FirebaseData &dataObj, const String &path, FirebaseJsonArray &arr)
{
    std::string s;
    arr._toStdString(s);
    bool flag = buildRequest(dataObj, FirebaseMethod::POST, FirebaseDataType::ARRAY, path.c_str(), s.c_str(), false, "", "");
    std::string().swap(s);
    return flag;
}

bool FirebaseESP8266::pushArray(FirebaseData &dataObj, const String &path, FirebaseJsonArray &arr, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    std::string s;
    arr._toStdString(s);
    bool flag = buildRequest(dataObj, FirebaseMethod::POST, FirebaseDataType::ARRAY, path.c_str(), s.c_str(), false, buff, "");
    std::string().swap(s);
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushBlob(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size)
{
    return pushBlob(dataObj, path.c_str(), blob, size, false, "");
}

bool FirebaseESP8266::pushBlob(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = pushBlob(dataObj, path.c_str(), blob, size, false, buff);
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushBlob(FirebaseData &dataObj, const std::string &path, uint8_t *blob, size_t size, bool queue, const std::string &priority)
{
    if (dataObj._maxBlobSize < size)
    {
        dataObj._httpCode = _HTTP_CODE_PAYLOAD_TOO_LARGE;
        return false;
    }

    size_t tmpSize = strlen_P(ESP8266_FIREBASE_STR_92) + 1;
    char *tmp = new char[tmpSize];
    memset(tmp, 0, tmpSize);

    std::string blobStr = "";
    strcpy_P(tmp, ESP8266_FIREBASE_STR_92);
    blobStr = tmp;
    blobStr += base64_encode_string((const unsigned char *)blob, size);
    memset(tmp, 0, tmpSize);
    strcpy_P(tmp, ESP8266_FIREBASE_STR_3);
    blobStr += tmp;
    bool flag = buildRequest(dataObj, FirebaseMethod::POST, FirebaseDataType::BLOB, path, blobStr.c_str(), queue, priority, "");
    std::string().swap(blobStr);
    delete[] tmp;
    return flag;
}

bool FirebaseESP8266::pushFile(FirebaseData &dataObj, uint8_t storageType, const String &path, const String &fileName)
{
    return buildRequestFile(dataObj, storageType, FirebaseMethod::POST, path.c_str(), fileName.c_str(), false, "", "");
}

bool FirebaseESP8266::pushFile(FirebaseData &dataObj, uint8_t storageType, const String &path, const String &fileName, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = buildRequestFile(dataObj, storageType, FirebaseMethod::POST, path.c_str(), fileName.c_str(), false, buff, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::pushTimestamp(FirebaseData &dataObj, const String &path)
{
    size_t tmpSize = strlen_P(ESP8266_FIREBASE_STR_154) + 1;
    char *tmp = new char[tmpSize];
    memset(tmp, 0, tmpSize);

    std::string blobStr = "";
    strcpy_P(tmp, ESP8266_FIREBASE_STR_154);
    bool flag = buildRequest(dataObj, FirebaseMethod::POST, FirebaseDataType::TIMESTAMP, path.c_str(), tmp, false, "", "");
    delete[] tmp;
    return flag;
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, int intValue)
{
    return setInt(dataObj, path, intValue);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, int intValue, float priority)
{
    return setInt(dataObj, path, intValue, priority);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, int intValue, const String &ETag)
{
    return setInt(dataObj, path, intValue, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, int intValue, float priority, const String &ETag)
{
    return setInt(dataObj, path, intValue, priority, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, float floatValue)
{
    return setFloat(dataObj, path, floatValue);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, float floatValue, float priority)
{
    return setFloat(dataObj, path, floatValue, priority);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, float floatValue, const String &ETag)
{
    return setFloat(dataObj, path, floatValue, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, float floatValue, float priority, const String &ETag)
{
    return setFloat(dataObj, path, floatValue, priority, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, double doubleValue)
{
    return setDouble(dataObj, path, doubleValue);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, double doubleValue, float priority)
{
    return setDouble(dataObj, path, doubleValue, priority);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, double doubleValue, const String &ETag)
{
    return setDouble(dataObj, path, doubleValue, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, double doubleValue, float priority, const String &ETag)
{
    return setDouble(dataObj, path, doubleValue, priority, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, bool boolValue)
{
    return setBool(dataObj, path, boolValue);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, bool boolValue, float priority)
{
    return setBool(dataObj, path, boolValue, priority);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, bool boolValue, const String &ETag)
{
    return setBool(dataObj, path, boolValue, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, bool boolValue, float priority, const String &ETag)
{
    return setBool(dataObj, path, boolValue, priority, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, const char *stringValue)
{
    return setString(dataObj, path, stringValue);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, const String &stringValue)
{
    return setString(dataObj, path, stringValue);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, const char *stringValue, float priority)
{
    return setString(dataObj, path, stringValue, priority);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, const String &stringValue, float priority)
{
    return setString(dataObj, path, stringValue, priority);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, const char *stringValue, const String &ETag)
{
    return setString(dataObj, path, stringValue, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, const String &stringValue, const String &ETag)
{
    return setString(dataObj, path, stringValue, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, const char *stringValue, float priority, const String &ETag)
{
    return setString(dataObj, path, stringValue, priority, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, const String &stringValue, float priority, const String &ETag)
{
    return setString(dataObj, path, stringValue, priority, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, FirebaseJson &json)
{
    return setJSON(dataObj, path, json);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, FirebaseJsonArray &arr)
{
    return setArray(dataObj, path, arr);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, FirebaseJson &json, float priority)
{
    return setJSON(dataObj, path, json, priority);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, FirebaseJsonArray &arr, float priority)
{
    return setArray(dataObj, path, arr, priority);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, FirebaseJson &json, const String &ETag)
{
    return setJSON(dataObj, path, json, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, FirebaseJsonArray &arr, const String &ETag)
{
    return setArray(dataObj, path, arr, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, FirebaseJson &json, float priority, const String &ETag)
{
    return setJSON(dataObj, path, json, priority, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, FirebaseJsonArray &arr, float priority, const String &ETag)
{
    return setArray(dataObj, path, arr, priority, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size)
{
    return setBlob(dataObj, path, blob, size);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size, float priority)
{
    return setBlob(dataObj, path, blob, size, priority);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size, const String &ETag)
{
    return setBlob(dataObj, path, blob, size, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size, float priority, const String &ETag)
{
    return setBlob(dataObj, path, blob, size, priority, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, uint8_t storageType, const String &path, const String &fileName)
{
    return setFile(dataObj, storageType, path, fileName);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, uint8_t storageType, const String &path, const String &fileName, float priority)
{
    return setFile(dataObj, storageType, path, fileName, priority);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, uint8_t storageType, const String &path, const String &fileName, const String &ETag)
{
    return setFile(dataObj, storageType, path, fileName, ETag);
}

bool FirebaseESP8266::set(FirebaseData &dataObj, uint8_t storageType, const String &path, const String &fileName, float priority, const String &ETag)
{
    return setFile(dataObj, storageType, path, fileName, priority, ETag);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, T value)
{
    if (std::is_same<T, int>::value)
        return setInt(dataObj, path, value);
    else if (std::is_same<T, double>::value)
        return setDouble(dataObj, path, value);
    else if (std::is_same<T, bool>::value)
        return setBool(dataObj, path, value);
    else if (std::is_same<T, const char *>::value)
        return setString(dataObj, path, value);
    else if (std::is_same<T, const String &>::value)
        return setString(dataObj, path, value);
    else if (std::is_same<T, FirebaseJson &>::value)
        return setJson(dataObj, path, value);
    else if (std::is_same<T, FirebaseJson *>::value)
        return setJson(dataObj, path, &value);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return setArray(dataObj, path, value);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, T value, size_t size)
{
    if (std::is_same<T, uint8_t *>::value)
        return setBlob(dataObj, path, value, size);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, T value, float priority)
{
    if (std::is_same<T, int>::value)
        return setInt(dataObj, path, value, priority);
    else if (std::is_same<T, double>::value)
        return setDouble(dataObj, path, value, priority);
    else if (std::is_same<T, bool>::value)
        return setBool(dataObj, path, value, priority);
    else if (std::is_same<T, const char *>::value)
        return setString(dataObj, path, value, priority);
    else if (std::is_same<T, const String &>::value)
        return setString(dataObj, path, value, priority);
    else if (std::is_same<T, FirebaseJson &>::value)
        return setJson(dataObj, path, value, priority);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return setArray(dataObj, path, value, priority);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, T value, size_t size, float priority)
{
    if (std::is_same<T, uint8_t *>::value)
        return setBlob(dataObj, path, value, size, priority);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, T value, const String &ETag)
{
    if (std::is_same<T, int>::value)
        return setInt(dataObj, path, value, ETag);
    else if (std::is_same<T, double>::value)
        return setDouble(dataObj, path, value, ETag);
    else if (std::is_same<T, bool>::value)
        return setBool(dataObj, path, value, ETag);
    else if (std::is_same<T, const char *>::value)
        return setString(dataObj, path, value, ETag);
    else if (std::is_same<T, const String &>::value)
        return setString(dataObj, path, value, ETag);
    else if (std::is_same<T, FirebaseJson &>::value)
        return setJson(dataObj, path, value, ETag);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return setArray(dataObj, path, value, ETag);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, T value, size_t size, const String &ETag)
{
    if (std::is_same<T, uint8_t *>::value)
        return setBlob(dataObj, path, value, size, ETag);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, T value, float priority, const String &ETag)
{
    if (std::is_same<T, int>::value)
        return setInt(dataObj, path, value, priority, ETag);
    else if (std::is_same<T, double>::value)
        return setDouble(dataObj, path, value, priority, ETag);
    else if (std::is_same<T, bool>::value)
        return setBool(dataObj, path, value, priority, ETag);
    else if (std::is_same<T, const char *>::value)
        return setString(dataObj, path, value, priority, ETag);
    else if (std::is_same<T, const String &>::value)
        return setString(dataObj, path, value, priority, ETag);
    else if (std::is_same<T, FirebaseJson &>::value)
        return setJson(dataObj, path, value, priority, ETag);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return setArray(dataObj, path, value, priority, ETag);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &dataObj, const String &path, T value, size_t size, float priority, const String &ETag)
{
    if (std::is_same<T, uint8_t *>::value)
        return setBlob(dataObj, path, value, size, priority, ETag);
}

bool FirebaseESP8266::setInt(FirebaseData &dataObj, const String &path, int intValue)
{
    return setInt(dataObj, path.c_str(), intValue, false, "", "");
}

bool FirebaseESP8266::setInt(FirebaseData &dataObj, const String &path, int intValue, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = setInt(dataObj, path.c_str(), intValue, false, buff, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setInt(FirebaseData &dataObj, const String &path, int intValue, const String &ETag)
{
    return setInt(dataObj, path.c_str(), intValue, false, "", ETag.c_str());
}

bool FirebaseESP8266::setInt(FirebaseData &dataObj, const String &path, int intValue, float priority, const String &ETag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = setInt(dataObj, path.c_str(), intValue, false, buff, ETag.c_str());
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setInt(FirebaseData &dataObj, const std::string &path, int intValue, bool queue, const std::string &priority, const std::string &etag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    itoa(intValue, buff, 10);
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::INTEGER, path, buff, queue, priority, etag);
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setFloat(FirebaseData &dataObj, const String &path, float floatValue)
{
    return setFloat(dataObj, path.c_str(), floatValue, false, "", "");
}

bool FirebaseESP8266::setFloat(FirebaseData &dataObj, const String &path, float floatValue, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = setFloat(dataObj, path.c_str(), floatValue, false, buff, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setFloat(FirebaseData &dataObj, const String &path, float floatValue, const String &ETag)
{
    return setFloat(dataObj, path.c_str(), floatValue, false, "", ETag.c_str());
}

bool FirebaseESP8266::setFloat(FirebaseData &dataObj, const String &path, float floatValue, float priority, const String &ETag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = setFloat(dataObj, path.c_str(), floatValue, false, buff, ETag.c_str());
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setFloat(FirebaseData &dataObj, const std::string &path, float floatValue, bool queue, const std::string &priority, const std::string &etag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(floatValue, 7, 6, buff);
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::FLOAT, path, buff, queue, priority, etag);
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setDouble(FirebaseData &dataObj, const String &path, double doubleValue)
{
    return setDouble(dataObj, path.c_str(), doubleValue, false, "", "");
}

bool FirebaseESP8266::setDouble(FirebaseData &dataObj, const String &path, double doubleValue, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = setDouble(dataObj, path.c_str(), doubleValue, false, buff, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setDouble(FirebaseData &dataObj, const String &path, double doubleValue, const String &ETag)
{
    return setDouble(dataObj, path.c_str(), doubleValue, false, "", ETag.c_str());
}

bool FirebaseESP8266::setDouble(FirebaseData &dataObj, const String &path, double doubleValue, float priority, const String &ETag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = setDouble(dataObj, path.c_str(), doubleValue, false, buff, ETag.c_str());
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setDouble(FirebaseData &dataObj, const std::string &path, double doubleValue, bool queue, const std::string &priority, const std::string &etag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(doubleValue, 12, 9, buff);
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::DOUBLE, path, buff, false, priority, etag);
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setBool(FirebaseData &dataObj, const String &path, bool boolValue)
{
    return setBool(dataObj, path.c_str(), boolValue, false, "", "");
}

bool FirebaseESP8266::setBool(FirebaseData &dataObj, const String &path, bool boolValue, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = setBool(dataObj, path.c_str(), boolValue, false, buff, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setBool(FirebaseData &dataObj, const String &path, bool boolValue, const String &ETag)
{
    return setBool(dataObj, path.c_str(), boolValue, false, "", ETag.c_str());
}

bool FirebaseESP8266::setBool(FirebaseData &dataObj, const String &path, bool boolValue, float priority, const String &ETag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = setBool(dataObj, path.c_str(), boolValue, false, buff, ETag.c_str());
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setBool(FirebaseData &dataObj, const std::string &path, bool boolValue, bool queue, const std::string &priority, const std::string &etag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    if (boolValue)
        strcpy_P(buff, ESP8266_FIREBASE_STR_107);
    else
        strcpy_P(buff, ESP8266_FIREBASE_STR_106);

    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::BOOLEAN, path, buff, false, priority, etag);
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setString(FirebaseData &dataObj, const String &path, const String &stringValue)
{
    return buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::STRING, path.c_str(), stringValue.c_str(), false, "", "");
}

bool FirebaseESP8266::setString(FirebaseData &dataObj, const String &path, const String &stringValue, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::STRING, path.c_str(), stringValue.c_str(), false, buff, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setString(FirebaseData &dataObj, const String &path, const String &stringValue, const String &ETag)
{
    return buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::STRING, path.c_str(), stringValue.c_str(), false, "", ETag.c_str());
}

bool FirebaseESP8266::setString(FirebaseData &dataObj, const String &path, const String &stringValue, float priority, const String &ETag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::STRING, path.c_str(), stringValue.c_str(), false, buff, ETag.c_str());
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setJSON(FirebaseData &dataObj, const String &path, FirebaseJson &json)
{
    std::string s;
    json._toStdString(s);
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::JSON, path.c_str(), s.c_str(), false, "", "");
    std::string().swap(s);
    return flag;
}

bool FirebaseESP8266::setJSON(FirebaseData &dataObj, const String &path, FirebaseJson &json, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    std::string s;
    json._toStdString(s);
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::JSON, path.c_str(), s.c_str(), false, buff, "");
    delete[] buff;
    std::string().swap(s);
    return flag;
}

bool FirebaseESP8266::setJSON(FirebaseData &dataObj, const String &path, FirebaseJson &json, const String &ETag)
{
    std::string s;
    json._toStdString(s);
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::JSON, path.c_str(), s.c_str(), false, "", ETag.c_str());
    std::string().swap(s);
    return flag;
}

bool FirebaseESP8266::setJSON(FirebaseData &dataObj, const String &path, FirebaseJson &json, float priority, const String &ETag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    std::string s;
    json._toStdString(s);
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::JSON, path.c_str(), s.c_str(), false, buff, ETag.c_str());
    delete[] buff;
    std::string().swap(s);
    return flag;
}

bool FirebaseESP8266::setArray(FirebaseData &dataObj, const String &path, FirebaseJsonArray &arr)
{
    String arrStr;
    arr.toString(arrStr);
    return buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::ARRAY, path.c_str(), arrStr.c_str(), false, "", "");
}

bool FirebaseESP8266::setArray(FirebaseData &dataObj, const String &path, FirebaseJsonArray &arr, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    String arrStr;
    arr.toString(arrStr);
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::ARRAY, path.c_str(), arrStr.c_str(), false, buff, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setArray(FirebaseData &dataObj, const String &path, FirebaseJsonArray &arr, const String &ETag)
{
    String arrStr;
    arr.toString(arrStr);
    return buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::ARRAY, path.c_str(), arrStr.c_str(), false, "", ETag.c_str());
}

bool FirebaseESP8266::setArray(FirebaseData &dataObj, const String &path, FirebaseJsonArray &arr, float priority, const String &ETag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    String arrStr;
    arr.toString(arrStr);
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::ARRAY, path.c_str(), arrStr.c_str(), false, buff, ETag.c_str());
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setBlob(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size)
{
    return setBlob(dataObj, path.c_str(), blob, size, false, "", "");
}

bool FirebaseESP8266::setBlob(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = setBlob(dataObj, path.c_str(), blob, size, false, buff, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setBlob(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size, const String &ETag)
{
    return setBlob(dataObj, path.c_str(), blob, size, false, "", ETag.c_str());
}

bool FirebaseESP8266::setBlob(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size, float priority, const String &ETag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = setBlob(dataObj, path.c_str(), blob, size, false, buff, ETag.c_str());
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setBlob(FirebaseData &dataObj, const std::string &path, uint8_t *blob, size_t size, bool queue, const std::string &priority, const std::string &etag)
{
    if (dataObj._maxBlobSize < size)
    {
        dataObj._httpCode = _HTTP_CODE_PAYLOAD_TOO_LARGE;
        return false;
    }

    size_t tmpSize = strlen_P(ESP8266_FIREBASE_STR_92) + 1;
    char *tmp = new char[tmpSize];
    memset(tmp, 0, tmpSize);

    std::string blobStr = "";
    strcpy_P(tmp, ESP8266_FIREBASE_STR_92);
    blobStr = tmp;
    blobStr += base64_encode_string((const unsigned char *)blob, size);
    memset(tmp, 0, tmpSize);
    strcpy_P(tmp, ESP8266_FIREBASE_STR_3);
    blobStr += tmp;
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT_SILENT, FirebaseDataType::BLOB, path, blobStr.c_str(), false, priority, etag);
    delete[] tmp;
    std::string().swap(blobStr);
    return flag;
}

bool FirebaseESP8266::setFile(FirebaseData &dataObj, uint8_t storageType, const String &path, const String &fileName)
{
    return buildRequestFile(dataObj, storageType, FirebaseMethod::PUT_SILENT, path.c_str(), fileName.c_str(), false, "", "");
}

bool FirebaseESP8266::setFile(FirebaseData &dataObj, uint8_t storageType, const String &path, const String &fileName, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = buildRequestFile(dataObj, storageType, FirebaseMethod::PUT_SILENT, path.c_str(), fileName.c_str(), false, buff, "");
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setFile(FirebaseData &dataObj, uint8_t storageType, const String &path, const String &fileName, const String &ETag)
{
    return buildRequestFile(dataObj, storageType, FirebaseMethod::PUT_SILENT, path.c_str(), fileName.c_str(), false, "", ETag.c_str());
}

bool FirebaseESP8266::setFile(FirebaseData &dataObj, uint8_t storageType, const String &path, const String &fileName, float priority, const String &ETag)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    bool flag = buildRequestFile(dataObj, storageType, FirebaseMethod::PUT_SILENT, path.c_str(), fileName.c_str(), false, buff, ETag.c_str());
    delete[] buff;
    return flag;
}

bool FirebaseESP8266::setTimestamp(FirebaseData &dataObj, const String &path)
{
    size_t tmpSize = strlen_P(ESP8266_FIREBASE_STR_154) + 1;
    char *tmp = new char[tmpSize];
    memset(tmp, 0, tmpSize);

    strcpy_P(tmp, ESP8266_FIREBASE_STR_154);
    bool flag = buildRequest(dataObj, FirebaseMethod::PUT, FirebaseDataType::TIMESTAMP, path.c_str(), tmp, false, "", "");
    delete[] tmp;
    return flag;
}

bool FirebaseESP8266::updateNode(FirebaseData &dataObj, const String path, FirebaseJson &json)
{
    std::string s;
    json._toStdString(s);
    bool flag = buildRequest(dataObj, FirebaseMethod::PATCH, FirebaseDataType::JSON, path.c_str(), s.c_str(), false, "", "");
    std::string().swap(s);
    return flag;
}

bool FirebaseESP8266::updateNode(FirebaseData &dataObj, const String &path, FirebaseJson &json, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    std::string s;
    json._toStdString(s);
    bool flag = buildRequest(dataObj, FirebaseMethod::PATCH, FirebaseDataType::JSON, path.c_str(), s.c_str(), false, buff, "");
    delete[] buff;
    std::string().swap(s);
    return flag;
}

bool FirebaseESP8266::updateNodeSilent(FirebaseData &dataObj, const String &path, FirebaseJson &json)
{
    std::string s;
    json._toStdString(s);
    bool flag = buildRequest(dataObj, FirebaseMethod::PATCH_SILENT, FirebaseDataType::JSON, path.c_str(), s.c_str(), false, "", "");
    std::string().swap(s);
    return flag;
}

bool FirebaseESP8266::updateNodeSilent(FirebaseData &dataObj, const String &path, FirebaseJson &json, float priority)
{
    size_t buffSize = 50;
    char *buff = new char[buffSize];
    memset(buff, 0, buffSize);
    dtostrf(priority, 7, 6, buff);
    std::string s;
    json._toStdString(s);
    bool flag = buildRequest(dataObj, FirebaseMethod::PATCH_SILENT, FirebaseDataType::JSON, path.c_str(), s.c_str(), false, buff, "");
    delete[] buff;
    std::string().swap(s);
    return flag;
}

bool FirebaseESP8266::get(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    bool flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::ANY, "", "", "");
    return flag;
}

bool FirebaseESP8266::getInt(FirebaseData &dataObj, const String &path)
{
    return getFloat(dataObj, path);
}

bool FirebaseESP8266::getInt(FirebaseData &dataObj, const String &path, int &target)
{

    bool flag;
    dataObj.queryFilter.clear();

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::INTEGER, "", "", "");
        target = dataObj.intData();
        if (flag)
            break;

        if (dataObj._maxRetry > 0)
            if (!flag && commError(dataObj))
                errCount++;
    }

    if (!flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        dataObj.addQueue(FirebaseMethod::GET, 0, FirebaseDataType::INTEGER, path.c_str(), "", "", false, &target, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    if (flag && dataObj._dataType != FirebaseDataType::INTEGER && dataObj._dataType != FirebaseDataType::FLOAT && dataObj._dataType != FirebaseDataType::DOUBLE)
        flag = false;

    return flag;
}

bool FirebaseESP8266::getFloat(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    bool flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::FLOAT, "", "", "");
    if (dataObj._dataType != FirebaseDataType::INTEGER && dataObj._dataType != FirebaseDataType::FLOAT && dataObj._dataType != FirebaseDataType::DOUBLE)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getFloat(FirebaseData &dataObj, const String &path, float &target)
{

    bool flag;
    dataObj.queryFilter.clear();

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::FLOAT, "", "", "");
        target = dataObj.floatData();
        if (flag)
            break;

        if (dataObj._maxRetry > 0)
            if (!flag && commError(dataObj))
                errCount++;
    }

    if (!flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        dataObj.addQueue(FirebaseMethod::GET, 0, FirebaseDataType::FLOAT, path.c_str(), "", "", false, nullptr, &target, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    if (flag && dataObj._dataType != FirebaseDataType::INTEGER && dataObj._dataType != FirebaseDataType::FLOAT && dataObj._dataType != FirebaseDataType::DOUBLE)
        flag = false;

    return flag;
}

bool FirebaseESP8266::getDouble(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    bool flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::DOUBLE, "", "", "");
    if (dataObj._dataType != FirebaseDataType::INTEGER && dataObj._dataType != FirebaseDataType::FLOAT && dataObj._dataType != FirebaseDataType::DOUBLE)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getDouble(FirebaseData &dataObj, const String &path, double &target)
{

    bool flag;
    dataObj.queryFilter.clear();

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::DOUBLE, "", "", "");
        target = dataObj.floatData();
        if (flag)
            break;

        if (dataObj._maxRetry > 0)
            if (!flag && commError(dataObj))
                errCount++;
    }

    if (!flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        dataObj.addQueue(FirebaseMethod::GET, 0, FirebaseDataType::DOUBLE, path.c_str(), "", "", false, nullptr, nullptr, &target, nullptr, nullptr, nullptr, nullptr, nullptr);

    if (flag && dataObj._dataType != FirebaseDataType::INTEGER && dataObj._dataType != FirebaseDataType::FLOAT && dataObj._dataType != FirebaseDataType::DOUBLE)
        flag = false;

    return flag;
}

bool FirebaseESP8266::getBool(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    bool flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::BOOLEAN, "", "", "");
    if (dataObj._dataType != FirebaseDataType::BOOLEAN)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getBool(FirebaseData &dataObj, const String &path, bool &target)
{

    bool flag;
    dataObj.queryFilter.clear();

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::BOOLEAN, "", "", "");
        target = dataObj.boolData();
        if (flag)
            break;

        if (dataObj._maxRetry > 0)
            if (!flag && commError(dataObj))
                errCount++;
    }

    if (!flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        dataObj.addQueue(FirebaseMethod::GET, 0, FirebaseDataType::BOOLEAN, path.c_str(), "", "", false, nullptr, nullptr, nullptr, &target, nullptr, nullptr, nullptr, nullptr);

    if (flag && dataObj._dataType != FirebaseDataType::BOOLEAN)
        flag = false;

    return flag;
}

bool FirebaseESP8266::getString(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    bool flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::STRING, "", "", "");
    if (dataObj._dataType != FirebaseDataType::STRING)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getString(FirebaseData &dataObj, const String &path, String &target)
{

    bool flag;
    dataObj.queryFilter.clear();

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::STRING, "", "", "");
        target = dataObj.stringData();
        if (flag)
            break;

        if (dataObj._maxRetry > 0)
            if (!flag && commError(dataObj))
                errCount++;
    }

    if (!flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        dataObj.addQueue(FirebaseMethod::GET, 0, FirebaseDataType::STRING, path.c_str(), "", "", false, nullptr, nullptr, nullptr, nullptr, &target, nullptr, nullptr, nullptr);

    if (flag && dataObj._dataType != FirebaseDataType::STRING)
        flag = false;

    return flag;
}

bool FirebaseESP8266::getJSON(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    bool flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::JSON, "", "", "");
    if (dataObj._dataType != FirebaseDataType::JSON)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getJSON(FirebaseData &dataObj, const String &path, FirebaseJson *target)
{

    bool flag;
    dataObj.queryFilter.clear();

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::JSON, "", "", "");
        target = dataObj.jsonObjectPtr();
        if (flag)
            break;

        if (dataObj._maxRetry > 0)
            if (!flag && commError(dataObj))
                errCount++;
    }

    if (!flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        dataObj.addQueue(FirebaseMethod::GET, 0, FirebaseDataType::JSON, path.c_str(), "", "", false, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, target, nullptr);

    if (flag && dataObj._dataType != FirebaseDataType::JSON)
        flag = false;

    return flag;
}

bool FirebaseESP8266::getJSON(FirebaseData &dataObj, const String &path, QueryFilter &query)
{
    dataObj.queryFilter.clear();
    if (query._orderBy != "")
        dataObj.setQuery(query);

    bool flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::JSON, "", "", "");
    if (dataObj._dataType != FirebaseDataType::JSON)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getJSON(FirebaseData &dataObj, const String &path, QueryFilter &query, FirebaseJson *target)
{
    dataObj.queryFilter.clear();
    if (query._orderBy != "")
        dataObj.setQuery(query);

    bool flag;

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::JSON, "", "", "");
        target = dataObj.jsonObjectPtr();
        if (flag)
            break;

        if (dataObj._maxRetry > 0)
            if (!flag && commError(dataObj))
                errCount++;
    }

    if (!flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        dataObj.addQueue(FirebaseMethod::GET, 0, FirebaseDataType::JSON, path.c_str(), "", "", true, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, target, nullptr);

    if (flag && dataObj._dataType != FirebaseDataType::JSON)
        flag = false;

    return flag;
}

bool FirebaseESP8266::getArray(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    bool flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::ARRAY, "", "", "");
    if (dataObj._dataType != FirebaseDataType::ARRAY)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getArray(FirebaseData &dataObj, const String &path, FirebaseJsonArray *target)
{

    bool flag;
    dataObj.queryFilter.clear();

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::JSON, "", "", "");
        target = dataObj.jsonArrayPtr();
        if (flag)
            break;

        if (dataObj._maxRetry > 0)
            if (!flag && commError(dataObj))
                errCount++;
    }

    if (!flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        dataObj.addQueue(FirebaseMethod::GET, 0, FirebaseDataType::JSON, path.c_str(), "", "", false, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, target);

    if (flag && dataObj._dataType != FirebaseDataType::JSON)
        flag = false;

    return flag;
}

bool FirebaseESP8266::getArray(FirebaseData &dataObj, const String &path, QueryFilter &query)
{
    dataObj.queryFilter.clear();
    if (query._orderBy != "")
        dataObj.setQuery(query);

    bool flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::JSON, "", "", "");
    if (dataObj._dataType != FirebaseDataType::JSON)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getArray(FirebaseData &dataObj, const String &path, QueryFilter &query, FirebaseJsonArray *target)
{
    dataObj.queryFilter.clear();
    if (query._orderBy != "")
        dataObj.setQuery(query);

    bool flag;

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::JSON, "", "", "");
        target = dataObj.jsonArrayPtr();
        if (flag)
            break;

        if (dataObj._maxRetry > 0)
            if (!flag && commError(dataObj))
                errCount++;
    }

    if (!flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        dataObj.addQueue(FirebaseMethod::GET, 0, FirebaseDataType::JSON, path.c_str(), "", "", true, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, target);

    if (flag && dataObj._dataType != FirebaseDataType::JSON)
        flag = false;

    return flag;
}

bool FirebaseESP8266::getBlob(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    bool flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::BLOB, "", "", "");
    if (dataObj._dataType != FirebaseDataType::BLOB)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getBlob(FirebaseData &dataObj, const String &path, std::vector<uint8_t> &target)
{

    dataObj.queryFilter.clear();

    bool flag;

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        flag = sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::GET, FirebaseDataType::BLOB, "", "", "");
        if (flag)
            break;

        if (dataObj._maxRetry > 0)
            if (!flag && commError(dataObj))
                errCount++;
    }

    if (!flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        dataObj.addQueue(FirebaseMethod::GET, 0, FirebaseDataType::BLOB, path.c_str(), "", "", false, nullptr, nullptr, nullptr, nullptr, nullptr, &target, nullptr, nullptr);

    if (flag && dataObj._dataType != FirebaseDataType::BLOB)
        flag = false;

    return flag;
}

bool FirebaseESP8266::getFile(FirebaseData &dataObj, uint8_t storageType, const String &nodePath, const String &fileName)
{
    dataObj.queryFilter.clear();
    dataObj._fileName.clear();
    dataObj._fileName = fileName.c_str();

    bool flag;

    uint8_t errCount = 0;
    uint8_t maxRetry = dataObj._maxRetry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        flag = sendRequest(dataObj, storageType, nodePath.c_str(), FirebaseMethod::GET, FirebaseDataType::FILE, "", "", "");
        if (flag)
            break;

        if (dataObj._maxRetry > 0)
            if (!flag && (commError(dataObj) || dataObj._file_transfer_error.length() > 0))
                errCount++;
    }

    if (!flag && errCount == maxRetry && dataObj._qMan._maxQueue > 0)
        dataObj.addQueue(FirebaseMethod::GET, storageType, FirebaseDataType::FILE, nodePath.c_str(), fileName.c_str(), "", false, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    return flag;
}

bool FirebaseESP8266::deleteNode(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::DELETE, FirebaseDataType::STRING, "", "", "");
}

bool FirebaseESP8266::deleteNode(FirebaseData &dataObj, const String &path, const String &ETag)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, 0, path.c_str(), FirebaseMethod::DELETE, FirebaseDataType::STRING, "", "", ETag.c_str());
}

bool FirebaseESP8266::beginStream(FirebaseData &dataObj, const String path)
{
    return firebaseConnectStream(dataObj, path.c_str());
}

bool FirebaseESP8266::readStream(FirebaseData &dataObj)
{
    if (!reconnect(dataObj))
        return false;

    return getServerStreamResponse(dataObj);
}

bool FirebaseESP8266::endStream(FirebaseData &dataObj)
{
    bool flag = false;
    dataObj._streamPath.clear();
    forceEndHTTP(dataObj);

    if (!apConnected(dataObj))
    {
        dataObj._isStream = false;
        dataObj._streamStop = true;
        return true;
    }

    flag = dataObj._net.connected();
    if (!flag)
    {
        dataObj._isStream = false;
        dataObj._streamStop = true;
    }
    return !flag;
}

int FirebaseESP8266::firebaseConnect(FirebaseData &dataObj, const std::string &path, const uint8_t method, uint8_t dataType, const std::string &payload, const std::string &priority)
{

    dataObj._firebaseError.clear();

    if (dataObj._pause)
        return 0;

    if (!apConnected(dataObj))
        return HTTPC_ERROR_CONNECTION_LOST;

    if (path.length() == 0 || _host.length() == 0 || _auth.length() == 0)
    {
        dataObj._httpCode = _HTTP_CODE_BAD_REQUEST;
        return _HTTP_CODE_BAD_REQUEST;
    }

    size_t buffSize = 32;
    char *buff = new char[buffSize];

    size_t tempSize = 200;
    char *temp = new char[tempSize];

    int len = 0;
    size_t toRead = 0;
    bool httpConnected = false;

    size_t payloadStrSize = payload.length() + 100;

    char *payloadStr = new char[payloadStrSize];
    memset(payloadStr, 0, payloadStrSize);

    int httpCode = HTTPC_ERROR_CONNECTION_REFUSED;

    //init the firebase data
    resetFirebasedataFlag(dataObj);
    dataObj._path.clear();
    dataObj._etag.clear();

    char *slash = new char[10];
    memset(slash, 0, 10);
    strcpy_P(slash, ESP8266_FIREBASE_STR_1);

    if (method == FirebaseMethod::STREAM)
    {
        //stream path change? reset the current (keep alive) connection
        if (path != dataObj._streamPath)
            dataObj._streamPathChanged = true;
        if (!dataObj._isStream || dataObj._streamPathChanged)
        {
            if (dataObj._net.connected())
                forceEndHTTP(dataObj);
        }

        dataObj._streamPath.clear();

        if (path.length() > 0)
            if (path[0] != '/')
                dataObj._streamPath = slash;

        dataObj._streamPath += path;
    }
    else
    {
        //last requested method was stream?, reset the connection
        if (dataObj._isStream)
            forceEndHTTP(dataObj);
        if (method != FirebaseMethod::BACKUP && method != FirebaseMethod::RESTORE)
        {
            if (path.length() > 0)
                if (path[0] != '/')
                    dataObj._path = slash;

            dataObj._path += path;
        }

        dataObj._isStreamTimeout = false;
    }
    delete[] slash;

    setSecure(dataObj);

    httpConnected = dataObj._net.begin(_host, _port);

    if (!httpConnected)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_REFUSED;
        goto EXIT_1;
    }

    //Prepare for string and JSON payloads
    if (method != FirebaseMethod::GET && method != FirebaseMethod::GET_RULES && method != FirebaseMethod::GET_SHALLOW && method != FirebaseMethod::GET_PRIORITY && method != FirebaseMethod::GET_SILENT && method != FirebaseMethod::STREAM &&
        method != FirebaseMethod::DELETE && method != FirebaseMethod::RESTORE)
    {

        if (priority.length() > 0)
        {
            if (dataType == FirebaseDataType::JSON)
            {
                if (payload.length() > 0)
                {

                    size_t _len = payload.length();
                    strcpy(payloadStr, payload.c_str());
                    char *t = new char[strlen_P(ESP8266_FIREBASE_STR_127) + 1];
                    memset(t, 0, strlen_P(ESP8266_FIREBASE_STR_127) + 1);
                    strcpy_P(t, ESP8266_FIREBASE_STR_127);
                    uint32_t x = payload.find_last_of(t);
                    delete[] t;
                    if (x != std::string::npos && x != 0)
                        for (size_t i = x; i < _len; i++)
                            payloadStr[i] = '\0';

                    strcat_P(payloadStr, ESP8266_FIREBASE_STR_157);
                    strcat(payloadStr, priority.c_str());
                    strcat_P(payloadStr, ESP8266_FIREBASE_STR_127);
                }
            }
            else
            {

                memset(payloadStr, 0, payloadStrSize);
                strcpy_P(payloadStr, ESP8266_FIREBASE_STR_169);
                strcat_P(payloadStr, ESP8266_FIREBASE_STR_3);
                strcat_P(payloadStr, ESP8266_FIREBASE_STR_4);
                strcat_P(payloadStr, ESP8266_FIREBASE_STR_161);

                if (dataType == FirebaseDataType::STRING)
                    strcat_P(payloadStr, ESP8266_FIREBASE_STR_3);
                strcat(payloadStr, payload.c_str());
                if (dataType == FirebaseDataType::STRING)
                    strcat_P(payloadStr, ESP8266_FIREBASE_STR_3);

                strcat_P(payloadStr, ESP8266_FIREBASE_STR_157);
                strcat(payloadStr, priority.c_str());
                strcat_P(payloadStr, ESP8266_FIREBASE_STR_127);
            }
        }
        else
        {
            memset(payloadStr, 0, payloadStrSize);
            if (dataType == FirebaseDataType::STRING)
                strcpy_P(payloadStr, ESP8266_FIREBASE_STR_3);
            strcat(payloadStr, payload.c_str());
            if (dataType == FirebaseDataType::STRING)
                strcat_P(payloadStr, ESP8266_FIREBASE_STR_3);
        }
    }

    //Prepare request header
    if (method != FirebaseMethod::BACKUP && method != FirebaseMethod::RESTORE && dataType != FirebaseDataType::FILE)
        sendFirebaseRequest(dataObj, _host.c_str(), method, dataType, path.c_str(), _auth.c_str(), strlen(payloadStr));
    else
    {
        if (dataObj._storageType == StorageType::SPIFFS)
        {
            if (!SPIFFS.begin())
            {
                dataObj._file_transfer_error.clear();
                memset(temp, 0, tempSize);
                strcat_P(temp, ESP8266_FIREBASE_STR_163);
                dataObj._file_transfer_error = temp;
                goto EXIT_1;
            }
        }
        else if (dataObj._storageType == StorageType::SD)
        {
            if (_sdInUse)
            {
                dataObj._file_transfer_error.clear();
                memset(temp, 0, tempSize);
                strcat_P(temp, ESP8266_FIREBASE_STR_84);
                dataObj._file_transfer_error = temp;
                goto EXIT_1;
            }

            if (!_sdOk)
                _sdOk = sdTest();

            if (!_sdOk)
            {
                memset(temp, 0, tempSize);
                strcat_P(temp, ESP8266_FIREBASE_STR_85);
                dataObj._file_transfer_error = temp;
                goto EXIT_1;
            }

            _sdInUse = true;
        }

        if (method == FirebaseMethod::BACKUP || method == FirebaseMethod::RESTORE)
        {
            if (method == FirebaseMethod::BACKUP)
            {
                if (dataObj._storageType == StorageType::SPIFFS)
                {
                    SPIFFS.remove(dataObj._backupFilename.c_str());
                    _file = SPIFFS.open(dataObj._backupFilename.c_str(), "w");
                }
                else if (dataObj._storageType == StorageType::SD)
                {
                    SD.remove(dataObj._backupFilename.c_str());
                    file = SD.open(dataObj._backupFilename.c_str(), FILE_WRITE);
                }
            }
            else if (method == FirebaseMethod::RESTORE)
            {
                if (dataObj._storageType == StorageType::SPIFFS && SPIFFS.exists(dataObj._backupFilename.c_str()))
                    _file = SPIFFS.open(dataObj._backupFilename.c_str(), "r");
                else if (dataObj._storageType == StorageType::SD && SD.exists(dataObj._backupFilename.c_str()))
                    file = SD.open(dataObj._backupFilename.c_str(), FILE_READ);
                else
                {
                    memset(temp, 0, tempSize);
                    strcat_P(temp, ESP8266_FIREBASE_STR_83);
                    dataObj._file_transfer_error = temp;
                    goto EXIT_1;
                }

                if (dataObj._storageType == StorageType::SPIFFS)
                    len = _file.size();
                else if (dataObj._storageType == StorageType::SD)
                    len = file.size();
            }
        }

        if (dataType == FirebaseDataType::FILE)
        {
            if (method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::POST)
            {
                if (dataObj._storageType == StorageType::SPIFFS)
                {

                    if (SPIFFS.exists(dataObj._fileName.c_str()))
                        _file = SPIFFS.open(dataObj._fileName.c_str(), "r");
                    else
                    {
                        dataObj._file_transfer_error.clear();
                        memset(temp, 0, tempSize);
                        strcat_P(temp, ESP8266_FIREBASE_STR_83);
                        dataObj._file_transfer_error = temp;
                        goto EXIT_1;
                    }
                }
                else if (dataObj._storageType == StorageType::SD)
                {
                    if (SD.exists(dataObj._fileName.c_str()))
                        file = SD.open(dataObj._fileName.c_str(), FILE_READ);
                    else
                    {
                        memset(temp, 0, tempSize);
                        strcat_P(temp, ESP8266_FIREBASE_STR_83);
                        dataObj._file_transfer_error = temp;
                        goto EXIT_1;
                    }
                }

                if (dataObj._storageType == StorageType::SPIFFS)
                    len = (4 * ceil(_file.size() / 3.0)) + strlen_P(ESP8266_FIREBASE_STR_93) + 1;
                else if (dataObj._storageType == StorageType::SD)
                    len = (4 * ceil(file.size() / 3.0)) + strlen_P(ESP8266_FIREBASE_STR_93) + 1;
            }
            else if (method == FirebaseMethod::GET)
            {
                size_t folderSize = 200;
                char *folder = new char[folderSize];
                memset(folder, 0, folderSize);

                char *fstr = new char[60];
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_1);

                int p1 = rstrpos(dataObj._fileName.c_str(), fstr, dataObj._fileName.length() - 1);

                if (p1 > -1 && p1 != 0)
                    strncpy(folder, dataObj._fileName.c_str(), p1 - 1);

                if (dataObj._storageType == StorageType::SPIFFS)
                {
                    _file = SPIFFS.open(dataObj._fileName.c_str(), "w");
                }
                else if (dataObj._storageType == StorageType::SD)
                {

                    if (!SD.exists(folder))
                        createDirs(folder, dataObj._storageType);

                    SD.remove(dataObj._fileName.c_str());

                    file = SD.open(dataObj._fileName.c_str(), FILE_WRITE);
                }
                delete[] folder;
                delete[] fstr;
            }
        }

        if ((!_file && dataObj._storageType == StorageType::SPIFFS) || (!file && dataObj._storageType == StorageType::SD))
        {
            memset(temp, 0, tempSize);
            strcat_P(temp, ESP8266_FIREBASE_STR_86);
            dataObj._file_transfer_error = temp;
            goto EXIT_1;
        }

        if (dataType == FirebaseDataType::FILE)
            sendFirebaseRequest(dataObj, _host.c_str(), method, dataType, dataObj._path.c_str(), _auth.c_str(), len);

        else
            sendFirebaseRequest(dataObj, _host.c_str(), method, dataType, dataObj._backupNodePath.c_str(), _auth.c_str(), len);
    }

    if (method == FirebaseMethod::GET_SILENT || method == FirebaseMethod::PATCH_SILENT || (method == FirebaseMethod::PUT_SILENT && dataType == FirebaseDataType::BLOB))
        dataObj._isSilentResponse = true;

    if (dataType == FirebaseDataType::BLOB)
        std::vector<uint8_t>().swap(dataObj._blob);

    //Send request w/wo payload

    if (!apConnected(dataObj))
        return HTTPC_ERROR_CONNECTION_LOST;

    httpCode = dataObj._net.sendRequest("", payloadStr);

    if (method == FirebaseMethod::RESTORE || (dataType == FirebaseDataType::FILE && (method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::POST)))
    {
        if (dataType == FirebaseDataType::FILE && (method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::POST))
        {
            memset(buff, 0, buffSize);
            strcpy_P(buff, ESP8266_FIREBASE_STR_93);

            if (!apConnected(dataObj))
                return HTTPC_ERROR_CONNECTION_LOST;

            httpCode = dataObj._net.sendRequest("", buff);

            send_base64_encode_file(dataObj._net._client.get(), dataObj._fileName, dataObj._storageType);

            memset(buff, 0, buffSize);
            buff[0] = '"';
            buff[1] = '\0';

            if (!apConnected(dataObj))
                return HTTPC_ERROR_CONNECTION_LOST;

            httpCode = dataObj._net.sendRequest("", buff);
        }
        else
        {
            while (len)
            {
                toRead = len;
                if (toRead > buffSize)
                    toRead = buffSize - 1;

                memset(buff, 0, buffSize);
                if (dataObj._storageType == StorageType::SPIFFS)
                    _file.read((uint8_t *)buff, toRead);
                else if (dataObj._storageType == StorageType::SD)
                    file.read((uint8_t *)buff, toRead);

                buff[toRead] = '\0';

                if (!apConnected(dataObj))
                    return HTTPC_ERROR_CONNECTION_LOST;

                httpCode = dataObj._net.sendRequest("", buff);

                len -= toRead;

                if (len <= 0)
                    break;
            }
        }

        endFileTransfer(dataObj);
    }

    delete[] payloadStr;
    delete[] buff;
    delete[] temp;
    return httpCode;

EXIT_1:

    delete[] payloadStr;
    delete[] buff;
    delete[] temp;
    return HTTPC_ERROR_CONNECTION_REFUSED;
}

bool FirebaseESP8266::sendRequest(FirebaseData &dataObj, uint8_t storageType, const std::string &path, const uint8_t method, uint8_t dataType, const std::string &payload, const std::string &priority, const std::string &etag)
{

    bool flag = false;
    dataObj._firebaseError.clear();

    if (!reconnect(dataObj))
        return false;

    if (dataObj._pause || dataObj._file_transfering)
        return true;

    if (path.length() == 0 || _host.length() == 0 || _auth.length() == 0)
    {
        dataObj._httpCode = _HTTP_CODE_BAD_REQUEST;
        return false;
    }

    if ((method == FirebaseMethod::PUT || method == FirebaseMethod::POST || method == FirebaseMethod::PATCH || method == FirebaseMethod::PATCH_SILENT || method == FirebaseMethod::SET_RULES) && payload.length() == 0 && dataType != FirebaseDataType::STRING && dataType != FirebaseDataType::BLOB && dataType != FirebaseDataType::FILE)
    {
        dataObj._httpCode = _HTTP_CODE_BAD_REQUEST;
        return false;
    }

    if (!reconnect(dataObj))
        return false;

    if (dataObj._firebaseCall)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_INUSED;
        return false;
    }

    dataObj._firebaseCall = true;
    dataObj._qID = 0;
    dataObj._etag2 = etag;
    dataObj._priority = priority;
    dataObj._storageType = storageType;

    //Get the current WiFi client from current firebase data
    //Check for connection status
    if (dataObj._net.connected())
        dataObj._httpConnected = true;
    else
        dataObj._httpConnected = false;

    if (dataObj._httpConnected)
    {
        if (method == FirebaseMethod::STREAM)
        {
            dataObj._streamMillis = millis();
            dataObj._firebaseCall = false;
            return false;
        }
        else
        {
            if (!dataObj._keepAlive)
            {
                dataObj._streamMillis = millis() + 50;
                dataObj._interruptRequest = true;
                if (dataObj._net.connected())
                {
                    forceEndHTTP(dataObj);
                    if (dataObj._net.connected())
                    {
                        if (!dataObj._isStream)
                        {
                            dataObj._firebaseCall = false;
                            return false;
                        }
                    }
                }
                dataObj._httpConnected = false;
            }
        }
    }

    if (!dataObj._keepAlive && dataObj._httpConnected)
    {
        dataObj._firebaseCall = false;
        return false;
    }

    dataObj._httpConnected = true;
    dataObj._interruptRequest = false;
    dataObj._redirectURL.clear();
    dataObj._r_method = method;
    dataObj._r_dataType = dataType;

    if (!apConnected(dataObj))
        return false;

    int httpCode = firebaseConnect(dataObj, path, method, dataType, payload, priority);

    if (httpCode == 0)
    {

        if (method == FirebaseMethod::BACKUP)
        {
            dataObj._file_transfering = true;
        }
        else if (method == FirebaseMethod::RESTORE || (dataType == FirebaseDataType::FILE && (method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::POST)))
        {
            dataObj._file_transfering = true;
        }

        dataObj._dataType2 = dataType;

        if (method == FirebaseMethod::STREAM)
        {
            unsigned long ml = millis();
            if (dataObj._streamMillis == 0 || ml - dataObj._streamMillis > 50)
                dataObj._streamMillis = ml;

            dataObj._dataMillis = millis();
            dataObj._isStreamTimeout = false;

            if (!apConnected(dataObj))
                return false;

            if (dataObj._firebaseCall || dataObj._fcmCall)
                return false;

            dataObj._streamCall = true;
            flag = getServerResponse(dataObj);
            dataObj._streamCall = false;

            if (!flag)
                forceEndHTTP(dataObj);
        }
        else if (method == FirebaseMethod::BACKUP || (dataType == FirebaseDataType::FILE && method == FirebaseMethod::GET))
        {
            flag = getDownloadResponse(dataObj);
        }
        else if (method == FirebaseMethod::RESTORE || (dataType == FirebaseDataType::FILE && method == FirebaseMethod::PUT_SILENT))
        {
            flag = getUploadResponse(dataObj);
        }
        else
        {
            dataObj._path.clear();
            dataObj._path = path;
            flag = getServerResponse(dataObj);
            dataObj._dataAvailable = dataObj._data.length() > 0 || dataObj._blob.size() > 0;

            if (dataObj._file_transfering)
                endFileTransfer(dataObj);
        }

        if (!flag)
            forceEndHTTP(dataObj);
    }
    else
    {
        //can't establish connection
        dataObj._httpCode = httpCode;
        dataObj._httpConnected = false;
        dataObj._firebaseCall = false;
        return false;
    }

    dataObj._firebaseCall = false;
    return flag;
}

bool FirebaseESP8266::clientAvailable(FirebaseData &dataObj, bool available)
{
    if (!reconnect(dataObj))
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
        return false;
    }

    if (!dataObj._net._client)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
        return false;
    }

    if (available)
        return dataObj._net._client->connected() && dataObj._net._client->available();
    else
        return dataObj._net._client->connected() && !dataObj._net._client->available();
}

bool FirebaseESP8266::getServerResponse(FirebaseData &dataObj)
{

    if (dataObj._pause)
        return true;

    if (!apConnected(dataObj))
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
        return false;
    }

    if (!dataObj._net.connected() || dataObj._interruptRequest)
        return cancelCurrentResponse(dataObj);
    if (!handleNetClientNotConnected(dataObj))
        return false;
    if (!dataObj._httpConnected && !dataObj._isStream)
        return false;

    bool flag = false;

    dataObj._data.clear();

    std::string jsonRes = "";

    char *lineBuf = new char[FIREBASE_RESPONSE_SIZE];
    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);

    uint16_t tempBufSize = FIREBASE_RESPONSE_SIZE;
    char *tmp = new char[tempBufSize];
    memset(tmp, 0, tempBufSize);

    uint16_t eventTypeSize = 30;
    char *eventType = new char[eventTypeSize];
    memset(eventType, 0, eventTypeSize);

    char *fstr = new char[60];
    memset(fstr, 0, 60);

    char c;
    int p1, p2;
    bool isStream = false;
    dataObj._httpCode = -1000;
    dataObj._contentLength = -1;
    dataObj._bufferOverflow = false;
    dataObj._pushName.clear();

    bool hasEvent = false;
    bool hasEventData = false;
    bool hasBlob = false;
    bool rulesBegin = false;
    bool payLoadBegin = false;

    unsigned long dataTime = millis();

    size_t lfCount = 0;
    size_t charPos = 0;
    int res = 0;

    if (!dataObj._isStream)
        while (clientAvailable(dataObj, false) && millis() - dataTime < dataObj._net.timeout)
        {
            if (!apConnected(dataObj))
                return false;
            delay(1);
        }

    dataTime = millis();

    if (clientAvailable(dataObj, false) && !dataObj._isStream)
        dataObj._httpCode = HTTPC_ERROR_READ_TIMEOUT;

    if (clientAvailable(dataObj, true))
    {
        while (clientAvailable(dataObj, true))
        {
            if (dataObj._interruptRequest)
                return cancelCurrentResponse(dataObj);

            if (!reconnect(dataObj))
            {
                dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
                return false;
            }

            res = dataObj._net._client->read();

            if (res < 0 || res > 0xff)
                continue;

            c = (char)res;

            if (payLoadBegin && dataObj._contentLength > 0)
            {
                if (charPos % 128 == 0)
                {
                    dataTime = millis();
                    delayMicroseconds(10);
                }
            }

            if (!hasBlob)
            {
                if (rulesBegin && charPos <= FIREBASE_RESPONSE_SIZE)
                    strcat_c(lineBuf, c);
                else if (c < 0xff && c != '\r' && c != '\n' && charPos <= FIREBASE_RESPONSE_SIZE)
                    strcat_c(lineBuf, c);
            }
            else
            {
                if (c < 0xff && c != '"' && c != '\r' && c != '\n' && dataObj._httpCode != _HTTP_CODE_NO_CONTENT)
                    strcat_c(lineBuf, c);

                continue;
            }

            if (charPos >= strlen_P(ESP8266_FIREBASE_STR_92) - 1 && !hasBlob)
            {
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_92);
                if (strpos(lineBuf, fstr, 0) == 0)
                {
                    hasBlob = true;
                    if (dataObj._dataType2 != FirebaseDataType::BLOB)
                        dataObj._httpCode = FIREBASE_ERROR_DATA_TYPE_MISMATCH;
                    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
                }
            }

            if (c < 0xff)
                charPos++;

            if (strlen(lineBuf) > FIREBASE_RESPONSE_SIZE)
            {
                dataObj._bufferOverflow = true;
                memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
            }

            if (rulesBegin)
            {
                if (jsonRes.length() < FIREBASE_RESPONSE_SIZE)
                    jsonRes += c;
                else
                    dataObj._bufferOverflow = true;
            }

            if (c == '\n' && !rulesBegin)
            {
                dataTime = millis();

                if (strlen(lineBuf) == 0 && lfCount > 0)
                    payLoadBegin = true;

                if (strlen(lineBuf) > 0)
                {
                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_5);

                    p1 = strpos(lineBuf, fstr, 0);
                    if (p1 != -1)
                    {

                        memset(tmp, 0, tempBufSize);
                        strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_5), strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_5));
                        dataObj._httpCode = atoi(tmp);
                    }

                    if (dataObj._httpCode == _HTTP_CODE_TEMPORARY_REDIRECT)
                    {

                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_95);

                        p1 = strpos(lineBuf, fstr, 0);
                        if (p1 != -1)
                        {
                            memset(tmp, 0, tempBufSize);
                            dataObj._redirectURL.clear();
                            strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_95), strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_95));
                            dataObj._redirectURL = tmp;
                            int res = firebaseConnect(dataObj, dataObj._redirectURL.c_str(), dataObj._r_method, dataObj._r_dataType, "", dataObj._priority);

                            if (res == 0)
                                goto EXIT_4;

                            goto EXIT_3;
                        }
                    }

                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_150);

                    p1 = strpos(lineBuf, fstr, 0);
                    if (p1 != -1)
                    {
                        memset(tmp, 0, tempBufSize);
                        strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_150), strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_150));
                        dataObj._etag = tmp;
                    }

                    if (dataObj._httpCode == _HTTP_CODE_NO_CONTENT)
                    {
                        memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
                        continue;
                    }

                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_7);

                    if (strpos(lineBuf, fstr, 0) != -1)
                    {
                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_102);

                        p1 = strpos(lineBuf, fstr, 0);
                        if (p1 != -1)
                        {
                            dataObj._firebaseError.clear();
                            memset(tmp, 0, tempBufSize);
                            strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_102) + 1, strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_102));
                            dataObj._firebaseError.append(tmp, strlen(tmp) - 1);
                        }

                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_8);

                        p1 = strpos(lineBuf, fstr, 0);
                        if (p1 != -1)
                        {
                            memset(tmp, 0, tempBufSize);
                            strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_8), strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_8));

                            memset(fstr, 0, 60);
                            strcpy_P(fstr, ESP8266_FIREBASE_STR_9);

                            if (strcmp(tmp, fstr) == 0)
                                isStream = true;
                        }

                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_10);

                        p1 = strpos(lineBuf, fstr, 0);
                        if (p1 != -1)
                        {
                            memset(tmp, 0, tempBufSize);
                            strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_10), strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_10));

                            memset(fstr, 0, 60);
                            strcpy_P(fstr, ESP8266_FIREBASE_STR_11);

                            if (strcmp(tmp, fstr) == 0)
                                dataObj._keepAlive = true;
                            else
                                dataObj._keepAlive = false;
                        }

                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_12);

                        p1 = strpos(lineBuf, fstr, 0);
                        if (p1 != -1)
                        {

                            memset(tmp, 0, tempBufSize);
                            strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_12), strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_12));

                            dataObj._contentLength = atoi(tmp);
                        }

                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_13);

                        p1 = strpos(lineBuf, fstr, 0);
                        if (p1 != -1)
                        {
                            memset(eventType, 0, eventTypeSize);
                            strncpy(eventType, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_13), strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_13));
                            hasEvent = true;
                            isStream = true;
                            dataObj._httpCode = _HTTP_CODE_OK;
                            memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
                        }

                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_14);

                        p1 = strpos(lineBuf, fstr, 0);
                        if (p1 != -1)
                        {
                            hasEventData = true;
                            isStream = true;
                            dataObj._httpCode = _HTTP_CODE_OK;

                            memset(tmp, 0, tempBufSize);
                            strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_14), strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_14));
                            memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
                            strcpy(lineBuf, tmp);
                            break;
                        }
                    }
                }

                if (dataObj._httpCode == _HTTP_CODE_OK && lfCount > 0 && strlen(lineBuf) == 0 && dataObj._r_method == FirebaseMethod::GET_RULES)
                    rulesBegin = true;

                if (!hasEventData || !hasEvent)
                    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);

                lfCount++;
                charPos = 0;
            }

            if (millis() - dataTime > dataObj._net.timeout)
            {
                dataObj._httpCode = HTTPC_ERROR_READ_TIMEOUT;
                break;
            }
        }

        if (dataObj._httpCode == _HTTP_CODE_OK || dataObj._httpCode == _HTTP_CODE_PRECONDITION_FAILED)
        {
            if (rulesBegin)
            {
                memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
                if (!dataObj._bufferOverflow)
                    strcpy(lineBuf, jsonRes.c_str());
                std::string().swap(jsonRes);
            }

            if (dataObj._r_method == FirebaseMethod::SET_RULES)
            {
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_104);

                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
            }

            //JSON stream data?
            if (isStream)
            {
                if (hasEventData && hasEvent)
                {
                    bool m = false;
                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_15);
                    m |= strpos(eventType, fstr, 0) != -1;

                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_16);
                    m |= strpos(eventType, fstr, 0) != -1;

                    if (m)
                    {
                        dataObj._eventType = eventType;

                        //Parses json response for path
                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_17);
                        p1 = strpos(lineBuf, fstr, 0);
                        if (p1 != -1 && p1 < FIREBASE_RESPONSE_SIZE)
                        {
                            memset(fstr, 0, 60);
                            strcpy_P(fstr, ESP8266_FIREBASE_STR_3);
                            p2 = strpos(lineBuf, fstr, p1 + strlen_P(ESP8266_FIREBASE_STR_17));
                            if (p2 != -1)
                            {
                                dataObj._path.clear();
                                memset(tmp, 0, tempBufSize);
                                strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_17), p2 - p1 - strlen_P(ESP8266_FIREBASE_STR_17));
                                dataObj._path = tmp;
                            }
                        }

                        //Parses json response for data
                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_18);
                        p1 = strpos(lineBuf, fstr, 0);
                        if (p1 != -1 && p1 < FIREBASE_RESPONSE_SIZE)
                        {

                            dataObj._data.clear();
                            memset(tmp, 0, tempBufSize);
                            strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_18), strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_18) - 1);
                            dataObj._data = tmp;

                            setDataType(dataObj, dataObj._data.c_str());
                            bool samePath = dataObj._path == dataObj._path2;
                            memset(fstr, 0, 60);
                            strcpy_P(fstr, ESP8266_FIREBASE_STR_1);
                            bool rootPath = strcmp(dataObj._path.c_str(), fstr) == 0;
                            bool emptyPath = dataObj._path2.length() == 0;
                            bool sameData = dataObj._data == dataObj._data2;

                            if (dataObj._data.length() >= strlen_P(ESP8266_FIREBASE_STR_92) && !hasBlob)
                            {
                                memset(fstr, 0, 60);
                                strcpy_P(fstr, ESP8266_FIREBASE_STR_92);
                                if (strcmp(dataObj._data.substr(0, strlen_P(ESP8266_FIREBASE_STR_92)).c_str(), fstr) == 0)
                                {
                                    char *tmp = new char[dataObj._data.length() + 1];
                                    memset(tmp, 0, dataObj._data.length() + 1);
                                    strcpy(tmp, dataObj._data.c_str());
                                    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
                                    strncpy(lineBuf, tmp + strlen_P(ESP8266_FIREBASE_STR_92), dataObj._data.length() - strlen_P(ESP8266_FIREBASE_STR_92) - 1);
                                    hasBlob = true;
                                    std::vector<uint8_t>().swap(dataObj._blob);
                                    dataObj._dataType = FirebaseDataType::BLOB;
                                    base64_decode_string(lineBuf, dataObj._blob);
                                    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
                                    dataObj._data.clear();
                                    dataObj._data2.clear();
                                    delete[] tmp;
                                }
                            }

                            //Any stream update?
                            if ((!samePath && (!rootPath || emptyPath)) || (samePath && !sameData && !dataObj._streamPathChanged))
                            {
                                dataObj._streamDataChanged = true;
                                dataObj._data2.clear();
                                dataObj._data2 = dataObj._data;
                                dataObj._path2.clear();
                                dataObj._path2 = dataObj._path;
                            }
                            else
                                dataObj._streamDataChanged = false;

                            dataObj._dataMillis = millis();
                            flag = true;
                            dataObj._dataAvailable = flag;
                            dataObj._isStreamTimeout = false;
                        }

                        dataObj._streamPathChanged = false;
                    }
                    else
                    {
                        //Firebase keep alive event
                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_11);
                        if (strcmp(eventType, fstr) == 0)
                        {
                            dataObj._isStreamTimeout = false;
                            dataObj._dataMillis = millis();
                        }

                        //Firebase cancel and auth_revoked events
                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_109);
                        bool m2 = strcmp(eventType, fstr) == 0;
                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_110);
                        m2 |= strcmp(eventType, fstr) == 0;

                        if (m2)
                        {
                            dataObj._isStreamTimeout = false;
                            dataObj._dataMillis = millis();
                            dataObj._eventType = eventType;
                            //make stream available status
                            dataObj._streamDataChanged = true;
                            dataObj._dataAvailable = true;
                        }
                    }
                }
                dataObj._streamMillis = millis();
            }
            else
            {
                //Just text payload

                dataObj._data.clear();

                if (hasBlob)
                {
                    dataObj._dataType = FirebaseDataType::BLOB;
                    base64_decode_string(lineBuf, dataObj._blob);
                }
                else
                {
                    memset(tmp, 0, tempBufSize);
                    strcpy(tmp, lineBuf);
                    dataObj._data = tmp;

                    setDataType(dataObj, lineBuf);

                    if (dataObj._priority_val_flag)
                    {
                        char *path = new char[dataObj._path.length()];
                        memset(path, 0, dataObj._path.length());
                        strncpy(path, dataObj._path.c_str(), dataObj._path.length() - strlen_P(ESP8266_FIREBASE_STR_156));
                        dataObj._path = path;
                        delete[] path;
                    }

                    //Push (POST) data?
                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_20);
                    p1 = strpos(lineBuf, fstr, 0);
                    if (p1 != -1)
                    {
                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_3);
                        p2 = strpos(lineBuf, fstr, p1 + strlen_P(ESP8266_FIREBASE_STR_20));
                        if (p2 != -1)
                        {
                            dataObj._pushName.clear();
                            memset(tmp, 0, tempBufSize);
                            strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_20), p2 - p1 - strlen_P(ESP8266_FIREBASE_STR_20));
                            dataObj._pushName = tmp;
                            dataObj._dataType = -1;
                            dataObj._dataType2 = -1;
                            dataObj._data.clear();
                        }
                    }
                }
            }
        }
        else
        {
            if (dataObj._isSilentResponse)
            {
                if (dataObj._httpCode == _HTTP_CODE_NO_CONTENT)
                {
                    dataObj._httpCode = _HTTP_CODE_OK;
                    dataObj._path.clear();
                    dataObj._data.clear();
                    dataObj._pushName.clear();
                    dataObj._dataType = -1;
                    dataObj._dataType2 = -1;
                    dataObj._dataAvailable = false;
                }
            }
            else
                dataObj._contentLength = -1;
        }

        memset(fstr, 0, 60);
        strcpy_P(fstr, ESP8266_FIREBASE_STR_151);
        if (strcmp(dataObj._etag.c_str(), fstr) == 0)
            dataObj._pathNotExist = true;
        else
            dataObj._pathNotExist = false;

        if (dataObj._dataType != FirebaseDataType::NULL_)
        {
            bool _n1 = dataObj._dataType == FirebaseDataType::DOUBLE || dataObj._dataType == FirebaseDataType::FLOAT || dataObj._dataType == FirebaseDataType::INTEGER;
            bool _n2 = dataObj._dataType2 == FirebaseDataType::DOUBLE || dataObj._dataType2 == FirebaseDataType::FLOAT || dataObj._dataType2 == FirebaseDataType::INTEGER;

            if (dataObj._dataType2 == dataObj._dataType || (_n1 && _n2))
                dataObj._mismatchDataType = false;
            else
                dataObj._mismatchDataType = true;
        }

        if (!dataObj._httpCode)
        {
            dataObj._contentLength = -1;
            dataObj._httpCode = HTTPC_ERROR_NO_HTTP_SERVER;
        }
        dataObj._httpConnected = false;
        dataObj._streamMillis = millis();

        goto EXIT_2;
    }

    if (dataObj._httpCode == -1000 && dataObj._isStream)
        flag = true;

    dataObj._httpConnected = false;
    dataObj._streamMillis = millis();
    delete[] lineBuf;
    delete[] tmp;
    delete[] eventType;
    std::string().swap(jsonRes);
    delete[] fstr;

    return flag;

EXIT_2:

    delete[] lineBuf;
    delete[] tmp;
    delete[] eventType;
    std::string().swap(jsonRes);
    delete[] fstr;

    if (dataObj._httpCode == HTTPC_ERROR_READ_TIMEOUT)
        return false;

    return dataObj._httpCode == _HTTP_CODE_OK || (dataObj._isStream && dataObj._httpCode == -1000);

EXIT_3:

    delete[] lineBuf;
    delete[] tmp;
    delete[] eventType;
    std::string().swap(jsonRes);
    delete[] fstr;

    return true;

EXIT_4:
    delete[] lineBuf;
    delete[] tmp;
    delete[] eventType;
    std::string().swap(jsonRes);
    delete[] fstr;
    return getServerResponse(dataObj);
}

bool FirebaseESP8266::getDownloadResponse(FirebaseData &dataObj)
{

    if (dataObj._pause)
    {
        endFileTransfer(dataObj);
        return true;
    }

    if (!apConnected(dataObj))
        return false;

    if (!dataObj._net._client)
    {
        endFileTransfer(dataObj);
        return false;
    }

    dataObj._httpCode = -1000;
    char c = 0;
    size_t buffSize = 128;
    char *buff = new char[buffSize + 1];

    char *lineBuf = new char[FIREBASE_RESPONSE_SIZE];
    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);

    uint16_t tempBufSize = FIREBASE_RESPONSE_SIZE;
    char *tmp = new char[tempBufSize];
    memset(tmp, 0, tempBufSize);

    uint16_t contentTypeSize = 30;
    char *contentType = new char[contentTypeSize];
    memset(contentType, 0, contentTypeSize);

    uint16_t contentDispositionSize = 50;
    char *contentDisposition = new char[contentDispositionSize];
    memset(contentDisposition, 0, contentDispositionSize);

    char *fstr = new char[60];

    size_t contentLength = 0;
    int p1 = 0;
    int p2 = 0;
    bool beginPayload = false;
    size_t count = 0;
    size_t toRead = count;
    size_t cnt = 0;
    size_t tmo = 30000 + dataObj._net.timeout;

    int res = 0;

    unsigned long dataTime = millis();

    while (clientAvailable(dataObj, false) && millis() - dataTime < tmo)
    {
        if (!apConnected(dataObj))
            return false;
        delay(1);
    }

    dataTime = millis();
    if (clientAvailable(dataObj, true))
    {

        while (clientAvailable(dataObj, true) || count > 0)
        {
            if (dataObj._interruptRequest)
                return cancelCurrentResponse(dataObj);

            if (!reconnect(dataObj))
            {
                dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
                return false;
            }

            if (!beginPayload)
            {
                c = dataObj._net._client->read();
                if (c != '\r' && c != '\n')
                    strcat_c(lineBuf, c);
            }
            else
            {

                if (contentLength <= 4)
                {
                    dataObj._httpCode = _HTTP_CODE_NOT_FOUND;
                    continue;
                }
                else
                {

                    toRead = count;
                    cnt = 0;
                    yield();
                    if (toRead > buffSize)
                        toRead = buffSize;

                    memset(buff, 0, buffSize + 1);
                    cnt = 0;

                    while (cnt < toRead)
                    {

                        res = dataObj._net._client->read();

                        if (res < 0 || res > 0xff)
                        {
                            //seem to be bug? due to content length header is not match the actual data
                            count = 0;
                            break;
                        }

                        c = (char)res;

                        if (res >= 0 && res <= 0xff && c >= 0x20)
                        {
                            if (dataObj._fileName == "" && c < 0xff)
                            {
                                buff[cnt] = c;
                                cnt++;
                            }
                            else if (dataObj._fileName != "" && c != '"')
                            {
                                buff[cnt] = c;
                                cnt++;
                            }
                        }
                    }

                    if (cnt == 0)
                        break;

                    count -= cnt;
                    dataTime = millis();
                    toRead = cnt;
                    buff[cnt] = '\0';
                    if (dataObj._storageType == StorageType::SPIFFS)
                    {
                        if (dataObj._fileName == "")
                            _file.write((uint8_t *)buff, toRead);
                        else
                            base64_decode_SPIFFS(_file, buff, toRead);
                    }
                    else if (dataObj._storageType == StorageType::SD)
                    {
                        if (dataObj._fileName == "")
                            file.write((uint8_t *)buff, toRead);
                        else
                            base64_decode_file(file, buff, toRead);
                    }

                    continue;
                }
            }

            if (c == '\n' && !beginPayload)
            {

                dataTime = millis();
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_5);
                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {
                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_6);
                    p2 = strpos(lineBuf, fstr, p1 + strlen_P(ESP8266_FIREBASE_STR_5));
                    if (p2 != -1)
                    {
                        memset(tmp, 0, tempBufSize);
                        strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_5), p2 - p1 - strlen_P(ESP8266_FIREBASE_STR_5));
                        dataObj._httpCode = atoi(tmp);
                    }
                }
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_102);
                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {
                    memset(tmp, 0, tempBufSize);
                    dataObj._firebaseError.clear();
                    strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_102) + 1, strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_102) - 2);
                    dataObj._firebaseError = tmp;
                }

                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_8);
                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {
                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_79);
                    p2 = strpos(lineBuf, fstr, p1 + strlen_P(ESP8266_FIREBASE_STR_8));
                    if (p2 != -1)
                        strncpy(contentType, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_8), p2 - p1 - strlen_P(ESP8266_FIREBASE_STR_8));
                }

                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_12);
                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {
                    memset(tmp, 0, tempBufSize);
                    strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_12), strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_12));
                    contentLength = atoi(tmp);
                    dataObj._backupzFileSize = contentLength;
                    count = contentLength;

                    if (dataObj._fileName != "")
                        count -= strlen_P(ESP8266_FIREBASE_STR_93) - 1;
                }
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_80);
                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {
                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_79);
                    p2 = strpos(lineBuf, fstr, p1 + strlen_P(ESP8266_FIREBASE_STR_80));
                    if (p2 != -1)
                        strncpy(contentDisposition, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_80), p2 - p1 - strlen_P(ESP8266_FIREBASE_STR_80));
                }

                bool k1, k2;
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_81);
                k1 = strcmp(contentType, fstr) == 0;
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_82);
                k2 = strcmp(contentDisposition, fstr) == 0;

                if (dataObj._httpCode == _HTTP_CODE_OK && strlen(lineBuf) == 0 && contentLength > 0 && k1 && k2)
                {
                    beginPayload = true;
                    if (dataObj._fileName != "")
                    {
                        for (size_t i = 0; i < strlen_P(ESP8266_FIREBASE_STR_93); i++)
                            dataObj._net._client->read();
                    }
                }

                memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
            }

            if (millis() - dataTime > dataObj._net.timeout)
            {
                dataObj._httpCode = HTTPC_ERROR_READ_TIMEOUT;
                break;
            }
        }
    }

    delete[] buff;
    delete[] lineBuf;
    delete[] contentType;
    delete[] contentDisposition;
    delete[] fstr;

    endFileTransfer(dataObj);

    if (dataObj._httpCode != -1000 && dataObj._httpCode != _HTTP_CODE_OK)
    {
        dataObj._file_transfer_error.clear();

        memset(tmp, 0, tempBufSize);
        if (dataObj._httpCode == _HTTP_CODE_NOT_FOUND)
        {
            strcpy_P(tmp, ESP8266_FIREBASE_STR_88);
        }
        else
        {
            strcpy_P(tmp, ESP8266_FIREBASE_STR_87);
            strcat(tmp, dataObj.errorReason().c_str());
        }

        dataObj._file_transfer_error = tmp;

        delete[] tmp;
        return false;
    }

    delete[] tmp;
    return true;
}

bool FirebaseESP8266::getUploadResponse(FirebaseData &dataObj)
{

    if (dataObj._pause)
        return true;

    if (!apConnected(dataObj))
        return false;

    if (!dataObj._net._client)
        return false;

    int _httpCode = -1000;
    char c = 0;

    char *lineBuf = new char[FIREBASE_RESPONSE_SIZE];
    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);

    uint16_t tempBufSize = FIREBASE_RESPONSE_SIZE;
    char *tmp = new char[tempBufSize];
    memset(tmp, 0, tempBufSize);

    char *fstr = new char[60];

    int p1, p2;
    size_t tmo = dataObj._net.timeout + 5000;
    bool beginPayload = false;
    int res = 0;

    unsigned long dataTime = millis();

    if (!dataObj._isStream)
        while (clientAvailable(dataObj, false) && millis() - dataTime < tmo)
        {
            if (!apConnected(dataObj))
                return false;
            delay(1);
        }

    dataTime = millis();

    if (clientAvailable(dataObj, true))
    {

        while (clientAvailable(dataObj, true))
        {
            if (dataObj._interruptRequest)
                return cancelCurrentResponse(dataObj);

            if (!reconnect(dataObj))
            {
                dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
                return false;
            }

            res = dataObj._net._client->read();

            if (res < 0 || res > 0xff)
                continue;

            c = (char)res;

            strcat_c(lineBuf, c);

            if (c == '\n' && !beginPayload)
            {
                dataTime = millis();
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_102);
                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {

                    memset(tmp, 0, tempBufSize);
                    dataObj._firebaseError.clear();
                    strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_102) + 1, strlen(lineBuf) - p1 - strlen_P(ESP8266_FIREBASE_STR_102) - 2);
                    dataObj._firebaseError = tmp;
                }
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_5);
                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {
                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_6);
                    p2 = strpos(lineBuf, fstr, p1 + strlen_P(ESP8266_FIREBASE_STR_5));
                    if (p2 != -1)
                    {
                        memset(tmp, 0, tempBufSize);
                        strncpy(tmp, lineBuf + p1 + strlen_P(ESP8266_FIREBASE_STR_5), p2 - p1 - strlen_P(ESP8266_FIREBASE_STR_5));
                        _httpCode = atoi(tmp);
                    }
                }

                memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
            }

            if (millis() - dataTime > dataObj._net.timeout)
            {
                _httpCode = HTTPC_ERROR_READ_TIMEOUT;
                dataObj._httpCode = HTTPC_ERROR_READ_TIMEOUT;
                break;
            }
        }
    }

    dataObj._file_transfering = false;

    if (_httpCode == _HTTP_CODE_NO_CONTENT)
        dataObj._httpCode = _HTTP_CODE_OK;
    else
        dataObj._httpCode = _httpCode;

    if (dataObj._httpCode != -1000 && _httpCode != _HTTP_CODE_NO_CONTENT)
    {
        dataObj._firebaseError.clear();

        memset(tmp, 0, tempBufSize);
        strcpy_P(tmp, ESP8266_FIREBASE_STR_87);
        strcat(tmp, dataObj.errorReason().c_str());
        dataObj._file_transfer_error = tmp;
    }

    delete[] lineBuf;
    delete[] tmp;
    delete[] fstr;

    return _httpCode == _HTTP_CODE_NO_CONTENT;
}

void FirebaseESP8266::endFileTransfer(FirebaseData &dataObj)
{
    _sdInUse = false;
    if (_file)
        _file.close();
    if (file)
        file.close();
    _sdOk = false;
    dataObj._file_transfering = false;
}

bool FirebaseESP8266::firebaseConnectStream(FirebaseData &dataObj, const std::string &path)
{

    if (dataObj._pause || dataObj._file_transfering)
        return true;

    dataObj._streamStop = false;

    if (!dataObj._isStreamTimeout && dataObj._isStream && path == dataObj._streamPath)
        return true;

    if (path.length() == 0 || _host.length() == 0 || _auth.length() == 0)
    {
        dataObj._httpCode = _HTTP_CODE_BAD_REQUEST;
        return false;
    }

    if (!apConnected(dataObj))
        return false;

    if (millis() - dataObj._streamResetMillis > 50)
        delay(50);

    bool flag;
    flag = dataObj._streamPath.length() == 0;
    flag |= firebaseConnect(dataObj, path, FirebaseMethod::STREAM, FirebaseDataType::STRING, "", "") == 0;
    dataObj._dataMillis = millis();
    return flag;
}

bool FirebaseESP8266::getServerStreamResponse(FirebaseData &dataObj)
{
    bool res = false;

    if (dataObj._pause)
        return true;

    if (dataObj._streamStop)
        return true;

    unsigned long ml = millis();
    if (dataObj._streamMillis == 0)
        dataObj._streamMillis = ml;
    if (dataObj._streamResetMillis == 0)
        dataObj._streamResetMillis = ml;

    //Reset firebase data every 50 ms and extend delay for 50 ms before stream response checking
    //to allow other http connection that may happen
    if (ml - dataObj._streamResetMillis > 100)
    {
        dataObj._streamResetMillis = ml;
        dataObj._streamDataChanged = false;
        dataObj._data2.clear();
        dataObj._path2.clear();
        dataObj._dataAvailable = false;
        dataObj._isStreamTimeout = false;

        delay(50);
        return true;
    }

    if (ml - dataObj._streamMillis > 50)
    {
        dataObj._streamMillis = ml;
        std::string path = "";

        if (!reconnect(dataObj))
            return false;

        //Stream timeout
        if (dataObj._dataMillis > 0 && millis() - dataObj._dataMillis > KEEP_ALIVE_TIMEOUT)
        {
            dataObj._dataMillis = millis();
            dataObj._isStreamTimeout = true;
            path = dataObj._streamPath;

            if (!reconnect(dataObj))
                return false;

            if (dataObj._firebaseCall || dataObj._fcmCall)
                return false;

            dataObj._streamCall = true;

            firebaseConnectStream(dataObj, path.c_str());

            res = false;

            res = getServerResponse(dataObj);

            if (!dataObj._httpConnected)
                dataObj._httpCode = HTTPC_ERROR_NOT_CONNECTED;

            dataObj._streamCall = false;

            return res;
        }

        //last connection was not close
        if (dataObj._httpConnected)
            return true;

        dataObj._httpConnected = true;
        resetFirebasedataFlag(dataObj);

        if (!apConnected(dataObj))
            return false;

        if (dataObj._firebaseCall || dataObj._fcmCall)
            return false;

        dataObj._streamCall = true;

        if (dataObj._net._client->connected() && !dataObj._isStream)
            forceEndHTTP(dataObj);

        if (!dataObj._net._client->connected())
        {
            path = dataObj._streamPath;
            firebaseConnectStream(dataObj, path.c_str());
        }

        std::string().swap(path);

        res = false;

        res = getServerResponse(dataObj);

        dataObj._streamCall = false;

        if (!dataObj._httpConnected)
            dataObj._httpCode = HTTPC_ERROR_NOT_CONNECTED;

        return res;
    }

    return true;
}

bool FirebaseESP8266::apConnected(FirebaseData &dataObj)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
        dataObj._firebaseCall = false;
        dataObj._streamCall = false;
        dataObj._fcmCall = false;
        return false;
    }
    return true;
}

void FirebaseESP8266::forceEndHTTP(FirebaseData &dataObj)
{
    if (!apConnected(dataObj))
        return;

    if (dataObj._net._client->available() > 0)
        dataObj._net._client->read();

    dataObj._net._client->stop();
    delay(50);
}

void FirebaseESP8266::processErrorQueue(FirebaseData &dataObj, QueueInfoCallback callback)
{
    if (dataObj._firebaseCall)
        return;

    if (!apConnected(dataObj))
        return;

    if (dataObj._qMan._queueCollection.size() > 0)
    {

        for (uint8_t i = 0; i < dataObj._qMan._queueCollection.size(); i++)
        {
            QueueItem item = dataObj._qMan._queueCollection[i];

            if (item.firebaseMethod == FirebaseMethod::GET)
            {
                switch (item.firebaseDataType)
                {

                case FirebaseDataType::INTEGER:
                    if (Firebase.getInt(dataObj, item.path.c_str()))
                    {

                        if (item.intPtr)
                            *item.intPtr = dataObj.intData();

                        dataObj.clearQueueItem(item);
                        dataObj._qMan.remove(i);
                    }

                    break;

                case FirebaseDataType::FLOAT:

                    if (Firebase.getFloat(dataObj, item.path.c_str()))
                    {

                        if (item.floatPtr)
                            *item.floatPtr = dataObj.floatData();

                        dataObj.clearQueueItem(item);
                        dataObj._qMan.remove(i);
                    }

                    break;

                case FirebaseDataType::DOUBLE:

                    if (Firebase.getDouble(dataObj, item.path.c_str()))
                    {

                        if (item.doublePtr)
                            *item.doublePtr = dataObj.doubleData();

                        dataObj.clearQueueItem(item);
                        dataObj._qMan.remove(i);
                    }

                    break;

                case FirebaseDataType::BOOLEAN:

                    if (Firebase.getBool(dataObj, item.path.c_str()))
                    {

                        if (item.boolPtr)
                            *item.boolPtr = dataObj.boolData();

                        dataObj.clearQueueItem(item);
                        dataObj._qMan.remove(i);
                    }

                    break;

                case FirebaseDataType::STRING:

                    if (Firebase.getString(dataObj, item.path.c_str()))
                    {

                        if (item.stringPtr)
                            *item.stringPtr = dataObj.stringData();

                        dataObj.clearQueueItem(item);
                        dataObj._qMan.remove(i);
                    }

                    break;

                case FirebaseDataType::JSON:

                    if (item.queryFilter._orderBy.length() > 0)
                    {
                        if (Firebase.getJSON(dataObj, item.path.c_str(), item.queryFilter))
                        {

                            if (item.jsonPtr)
                                item.jsonPtr = dataObj.jsonObjectPtr();

                            dataObj.clearQueueItem(item);
                            dataObj._qMan.remove(i);
                        }
                    }
                    else
                    {
                        if (Firebase.getJSON(dataObj, item.path.c_str()))
                        {

                            if (item.jsonPtr)
                                item.jsonPtr = dataObj.jsonObjectPtr();

                            dataObj.clearQueueItem(item);
                            dataObj._qMan.remove(i);
                        }
                    }

                    break;
                case FirebaseDataType::ARRAY:

                    if (item.queryFilter._orderBy.length() > 0)
                    {
                        if (Firebase.getArray(dataObj, item.path.c_str(), item.queryFilter))
                        {

                            if (item.arrPtr)
                                item.arrPtr = dataObj.jsonArrayPtr();

                            dataObj.clearQueueItem(item);
                            dataObj._qMan.remove(i);
                        }
                    }
                    else
                    {
                        if (Firebase.getArray(dataObj, item.path.c_str()))
                        {

                            if (item.arrPtr)
                                item.arrPtr = dataObj.jsonArrayPtr();

                            dataObj.clearQueueItem(item);
                            dataObj._qMan.remove(i);
                        }
                    }

                    break;

                case FirebaseDataType::BLOB:

                    if (Firebase.getBlob(dataObj, item.path.c_str()))
                    {
                        if (item.blobPtr)
                            *item.blobPtr = dataObj.blobData();

                        dataObj.clearQueueItem(item);
                        dataObj._qMan.remove(i);
                    }

                    break;

                case FirebaseDataType::FILE:

                    if (Firebase.getFile(dataObj, item.storageType, item.path.c_str(), item.filename.c_str()))
                    {
                        dataObj.clearQueueItem(item);
                        dataObj._qMan.remove(i);
                    }

                    break;

                case FirebaseDataType::ANY:

                    if (Firebase.get(dataObj, item.path.c_str()))
                    {
                        dataObj.clearQueueItem(item);
                        dataObj._qMan.remove(i);
                    }

                    break;

                default:
                    break;
                }
            }
            else if (item.firebaseMethod == FirebaseMethod::POST || item.firebaseMethod == FirebaseMethod::PUT || item.firebaseMethod == FirebaseMethod::PUT_SILENT || item.firebaseMethod == FirebaseMethod::PATCH || item.firebaseMethod == FirebaseMethod::PATCH_SILENT)
            {
                if (item.firebaseDataType == FirebaseDataType::FILE)
                {
                    if (buildRequestFile(dataObj, item.storageType, item.firebaseMethod, item.path.c_str(), item.filename.c_str(), true, "", ""))
                    {
                        dataObj.clearQueueItem(item);
                        dataObj._qMan.remove(i);
                    }
                }
                else
                {
                    if (buildRequest(dataObj, item.firebaseMethod, item.firebaseDataType, item.path.c_str(), item.payload.c_str(), true, "", ""))
                    {
                        dataObj.clearQueueItem(item);
                        dataObj._qMan.remove(i);
                    }
                }
            }
        }
    }
}

uint32_t FirebaseESP8266::getErrorQueueID(FirebaseData &dataObj)
{
    return dataObj._qID;
}

bool FirebaseESP8266::isErrorQueueExisted(FirebaseData &dataObj, uint32_t errorQueueID)
{

    for (uint8_t i = 0; i < dataObj._qMan._queueCollection.size(); i++)
    {
        QueueItem q = dataObj._qMan._queueCollection[i];
        if (q.qID == errorQueueID)
            return true;
    }
    return false;
}

void FirebaseESP8266::sendFirebaseRequest(FirebaseData &dataObj, const char *host, uint8_t method, uint8_t dataType, const char *path, const char *auth, size_t payloadLength)
{
    uint8_t retryCount = 0;
    uint8_t maxRetry = 5;

    uint8_t http_method = 0;

    size_t headerSize = 400;
    char *request = new char[headerSize];
    memset(request, 0, headerSize);

    size_t numBufSize = 50;

    char *contentLength = new char[numBufSize];

    memset(contentLength, 0, numBufSize);

    char *temp = new char[20];
    memset(temp, 0, 20);
    dataObj._shallow_flag = false;
    dataObj._priority_val_flag = false;

    char *num = new char[numBufSize];

    if (method == FirebaseMethod::STREAM)
    {
        strcpy_P(request, ESP8266_FIREBASE_STR_22);
        dataObj._isStream = true;
    }
    else
    {
        if (method == FirebaseMethod::PUT || method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::SET_PRIORITY || method == FirebaseMethod::SET_RULES)
        {
            http_method = FirebaseMethod::PUT;
            if (dataObj._classicRequest)
                strcpy_P(request, ESP8266_FIREBASE_STR_24);
            else
                strcpy_P(request, ESP8266_FIREBASE_STR_23);
        }
        else if (method == FirebaseMethod::POST)
        {
            http_method = FirebaseMethod::POST;
            strcpy_P(request, ESP8266_FIREBASE_STR_24);
        }
        else if (method == FirebaseMethod::GET || method == FirebaseMethod::GET_SILENT || method == FirebaseMethod::GET_SHALLOW || method == FirebaseMethod::GET_PRIORITY || method == FirebaseMethod::BACKUP || method == FirebaseMethod::GET_RULES)
        {
            http_method = FirebaseMethod::GET;
            strcpy_P(request, ESP8266_FIREBASE_STR_25);
        }
        else if (method == FirebaseMethod::PATCH || method == FirebaseMethod::PATCH_SILENT || method == FirebaseMethod::RESTORE)
        {
            http_method = FirebaseMethod::PATCH;
            strcpy_P(request, ESP8266_FIREBASE_STR_26);
        }
        else if (method == FirebaseMethod::DELETE)
        {
            http_method = FirebaseMethod::DELETE;
            if (dataObj._classicRequest)
                strcpy_P(request, ESP8266_FIREBASE_STR_24);
            else
                strcpy_P(request, ESP8266_FIREBASE_STR_27);
        }

        strcat_P(request, ESP8266_FIREBASE_STR_6);

        dataObj._isStream = false;
    }

    if (!apConnected(dataObj))
        return;

    retryCount = 0;
    while (dataObj._net.sendRequest(request, "") != 0)
    {
        retryCount++;
        if (retryCount > maxRetry)
            break;
    }

    if (strlen(path) > 0)
    {
        if (path[0] != '/')
        {
            memset(request, 0, headerSize);
            strcpy_P(request, ESP8266_FIREBASE_STR_1);
            dataObj._net.sendRequest(request, "");
        }
    }

    dataObj._net.sendRequest(path, "");

    memset(request, 0, headerSize);

    if (method == FirebaseMethod::PATCH || method == FirebaseMethod::PATCH_SILENT)
        strcat_P(request, ESP8266_FIREBASE_STR_1);

    strcat_P(request, ESP8266_FIREBASE_STR_2);

    strcat(request, auth);

    if (dataObj._readTimeout > 0)
    {
        strcat_P(request, ESP8266_FIREBASE_STR_158);
        itoa(dataObj._readTimeout, temp, 10);
        strcat(request, temp);
        strcat_P(request, ESP8266_FIREBASE_STR_159);
    }

    if (dataObj._writeLimit.length() > 0)
    {
        strcat_P(request, ESP8266_FIREBASE_STR_160);
        strcat(request, dataObj._writeLimit.c_str());
    }

    if (method == FirebaseMethod::GET_SHALLOW)
    {
        strcat_P(request, ESP8266_FIREBASE_STR_155);
        dataObj._shallow_flag = true;
    }

    if (method == FirebaseMethod::GET && dataObj.queryFilter._orderBy != "")
    {

        strcat_P(request, ESP8266_FIREBASE_STR_96);
        strcat(request, dataObj.queryFilter._orderBy.c_str());

        if (method == FirebaseMethod::GET && dataObj.queryFilter._limitToFirst != "")
        {
            strcat_P(request, ESP8266_FIREBASE_STR_97);
            strcat(request, dataObj.queryFilter._limitToFirst.c_str());
        }

        if (method == FirebaseMethod::GET && dataObj.queryFilter._limitToLast != "")
        {
            strcat_P(request, ESP8266_FIREBASE_STR_98);
            strcat(request, dataObj.queryFilter._limitToLast.c_str());
        }

        if (method == FirebaseMethod::GET && dataObj.queryFilter._startAt != "")
        {
            strcat_P(request, ESP8266_FIREBASE_STR_99);
            strcat(request, dataObj.queryFilter._startAt.c_str());
        }

        if (method == FirebaseMethod::GET && dataObj.queryFilter._endAt != "")
        {
            strcat_P(request, ESP8266_FIREBASE_STR_100);
            strcat(request, dataObj.queryFilter._endAt.c_str());
        }

        if (method == FirebaseMethod::GET && dataObj.queryFilter._equalTo != "")
        {
            strcat_P(request, ESP8266_FIREBASE_STR_101);
            strcat(request, dataObj.queryFilter._equalTo.c_str());
        }
    }

    if (method == FirebaseMethod::BACKUP)
    {
        strcat_P(request, ESP8266_FIREBASE_STR_162);
        strcat_P(request, ESP8266_FIREBASE_STR_28);

        dataObj._net.sendRequest(request, "");

        memset(request, 0, headerSize);

        char *filename = new char[headerSize];
        memset(filename, 0, headerSize);

        for (size_t i = 0; i < dataObj._backupNodePath.length(); i++)
        {

            if (dataObj._backupNodePath[i] == '/')
                strcat_P(filename, ESP8266_FIREBASE_STR_4);
            else
                strcat_c(filename, dataObj._backupNodePath[i]);
        }

        dataObj._net.sendRequest(filename, "");

        delete[] filename;
    }

    if (method == FirebaseMethod::GET && dataObj._fileName.length() > 0)
    {

        strcat_P(request, ESP8266_FIREBASE_STR_28);
        strcat(request, dataObj._fileName.c_str());
    }

    if (method == FirebaseMethod::GET_SILENT || method == FirebaseMethod::RESTORE || method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::PATCH_SILENT)
        strcat_P(request, ESP8266_FIREBASE_STR_29);

    strcat_P(request, ESP8266_FIREBASE_STR_30);
    strcat_P(request, ESP8266_FIREBASE_STR_31);
    strcat(request, host);
    strcat_P(request, ESP8266_FIREBASE_STR_21);
    strcat_P(request, ESP8266_FIREBASE_STR_32);
    strcat_P(request, ESP8266_FIREBASE_STR_33);

    //Timestamp cannot use with ETag header, otherwise cases internal server error
    if (dataObj.queryFilter._orderBy.length() == 0 && dataType != FirebaseDataType::TIMESTAMP && (method == FirebaseMethod::DELETE || method == FirebaseMethod::GET || method == FirebaseMethod::GET_SILENT || method == FirebaseMethod::PUT || method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::POST))
        strcat_P(request, ESP8266_FIREBASE_STR_148);

    if (dataObj._etag2.length() > 0 && (method == FirebaseMethod::PUT || method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::DELETE))
    {
        strcat_P(request, ESP8266_FIREBASE_STR_149);
        strcat(request, dataObj._etag2.c_str());
        strcat_P(request, ESP8266_FIREBASE_STR_21);
    }

    if (dataObj._classicRequest && http_method != FirebaseMethod::GET && http_method != FirebaseMethod::POST && http_method != FirebaseMethod::PATCH)
    {
        strcat_P(request, ESP8266_FIREBASE_STR_153);

        if (http_method == FirebaseMethod::PUT)
            strcat_P(request, ESP8266_FIREBASE_STR_23);
        else if (http_method == FirebaseMethod::DELETE)
            strcat_P(request, ESP8266_FIREBASE_STR_27);

        strcat_P(request, ESP8266_FIREBASE_STR_21);
    }

    if (method == FirebaseMethod::STREAM)
    {
        strcat_P(request, ESP8266_FIREBASE_STR_34);
        strcat_P(request, ESP8266_FIREBASE_STR_35);
    }
    else if (method == FirebaseMethod::BACKUP || method == FirebaseMethod::RESTORE)
    {
        strcat_P(request, ESP8266_FIREBASE_STR_34);
    }
    else
    {
        strcat_P(request, ESP8266_FIREBASE_STR_36);
        strcat_P(request, ESP8266_FIREBASE_STR_37);
    }

    if (method != FirebaseMethod::BACKUP && method != FirebaseMethod::RESTORE)
        strcat_P(request, ESP8266_FIREBASE_STR_38);

    if (method == FirebaseMethod::GET_PRIORITY || method == FirebaseMethod::SET_PRIORITY)
        dataObj._priority_val_flag = true;

    if (method == FirebaseMethod::PUT || method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::POST || method == FirebaseMethod::PATCH || method == FirebaseMethod::PATCH_SILENT || method == FirebaseMethod::RESTORE || method == FirebaseMethod::SET_RULES || method == FirebaseMethod::SET_PRIORITY)
    {
        strcat_P(request, ESP8266_FIREBASE_STR_12);
        itoa(payloadLength, contentLength, 10);
        strcat(request, contentLength);
    }

    strcat_P(request, ESP8266_FIREBASE_STR_21);
    strcat_P(request, ESP8266_FIREBASE_STR_21);
    dataObj._net.sendRequest(request, "");

    delete[] contentLength;
    delete[] num;
    delete[] request;
    delete[] temp;
}

bool FirebaseESP8266::cancelCurrentResponse(FirebaseData &dataObj)
{
    forceEndHTTP(dataObj);
    dataObj._isStream = false;
    dataObj._streamDataChanged = false;
    dataObj._dataMillis = millis();
    dataObj._data2.clear();
    dataObj._path2.clear();
    dataObj._dataAvailable = false;
    dataObj._isStreamTimeout = false;
    dataObj._httpCode = HTTPC_ERROR_CONNECTION_REFUSED;
    return false;
}

void FirebaseESP8266::setDataType(FirebaseData &dataObj, const char *data)
{

    size_t len = 32;
    char *temp = new char[len];
    bool typeSet = false;

    if (strlen(data) > 0)
    {

        int p1 = 0;
        int p2 = strlen(data) - 1;

        while (isspace(data[p1]))
        {
            p1++;
            if (p1 == p2)
                break;
        }

        while (isspace(data[p2]))
        {
            p2--;
            if (p2 == 0)
                break;
        }

        if (!typeSet && data[p1] == '{' && data[p2] == '}')
        {
            typeSet = true;
            dataObj._dataType = FirebaseDataType::JSON;
        }

        if (!typeSet && data[p1] == '[' && data[p2] == ']')
        {
            typeSet = true;
            dataObj._dataType = FirebaseDataType::ARRAY;
        }

        if (!typeSet)
        {
            if (strcmp(data, DEF_ESP8266_FIREBASE_STR_106) == 0 || strcmp(data, DEF_ESP8266_FIREBASE_STR_107) == 0)
            {
                typeSet = true;
                dataObj._dataType = FirebaseDataType::BOOLEAN;
            }

            if (!typeSet)
            {
                memset(temp, 0, len);
                strncpy(temp, data, strlen(DEF_ESP8266_FIREBASE_STR_92));
                if (strcmp(temp, DEF_ESP8266_FIREBASE_STR_92) == 0)
                {
                    typeSet = true;
                    dataObj._dataType = FirebaseDataType::BLOB;
                }
            }

            if (!typeSet)
            {
                memset(temp, 0, len);
                strncpy(temp, data, strlen(DEF_ESP8266_FIREBASE_STR_93));
                if (strcmp(temp, DEF_ESP8266_FIREBASE_STR_93) == 0)
                {
                    typeSet = true;
                    dataObj._dataType = FirebaseDataType::FILE;
                }
            }
        }

        if (!typeSet && data[p1] == '"' && data[p2] == '"')
        {
            typeSet = true;
            dataObj._dataType = FirebaseDataType::STRING;
        }

        if (!typeSet && strpos(data, DEF_ESP8266_FIREBASE_STR_4, 0) != -1)
        {
            typeSet = true;
            if (strlen(data) <= 7)
                dataObj._dataType = FirebaseDataType::FLOAT;
            else
                dataObj._dataType = FirebaseDataType::DOUBLE;
        }

        else if (!typeSet && strcmp(data, DEF_ESP8266_FIREBASE_STR_19) == 0)
        {
            typeSet = true;
            dataObj._dataType = FirebaseDataType::NULL_;
        }

        if (!typeSet)
        {
            typeSet = true;
            double d = atof(data);
            if (d > 0x7fffffff)
                dataObj._dataType = FirebaseDataType::DOUBLE;
            else
                dataObj._dataType = FirebaseDataType::INTEGER;
        }

        if (strcmp(data, DEF_ESP8266_FIREBASE_STR_19) == 0 && dataObj.queryFilter._orderBy == "")
            dataObj._data.clear();
        else if (strcmp(data, DEF_ESP8266_FIREBASE_STR_19) == 0 && dataObj.queryFilter._orderBy != "")
            dataObj._dataType = FirebaseDataType::JSON;
    }
    else
    {
        dataObj._dataType = FirebaseDataType::NULL_;
    }

    dataObj._dataTypeNum = dataObj._dataType;

    delete[] temp;
}

void FirebaseESP8266::setSecure(FirebaseData &dataObj)
{
    if (dataObj._net._certType == -1)
    {
        if (!_clockReady)
            setClock();

        dataObj._net._clockReady = _clockReady;

        if (_rootCAFile.length() == 0)
        {
            if (_rootCA)
                dataObj._net.setRootCA(_rootCA.get());
            else
                dataObj._net.setRootCA(nullptr);
        }
        else
        {
            dataObj._net.setRootCAFile(_rootCAFile, _rootCAFileStoreageType, _sdPin);
        }
    }
}

bool FirebaseESP8266::commError(FirebaseData &dataObj)
{
    return dataObj._httpCode == HTTPC_ERROR_CONNECTION_REFUSED || dataObj._httpCode == HTTPC_ERROR_CONNECTION_LOST ||
           dataObj._httpCode == HTTPC_ERROR_SEND_PAYLOAD_FAILED || dataObj._httpCode == HTTPC_ERROR_SEND_HEADER_FAILED ||
           dataObj._httpCode == HTTPC_ERROR_NOT_CONNECTED || dataObj._httpCode == HTTPC_ERROR_READ_TIMEOUT;
}

void FirebaseESP8266::resetFirebasedataFlag(FirebaseData &dataObj)
{
    dataObj._bufferOverflow = false;
    dataObj._streamDataChanged = false;
    dataObj._streamPathChanged = false;
    dataObj._dataAvailable = false;
    dataObj._pushName.clear();
}
bool FirebaseESP8266::handleNetClientNotConnected(FirebaseData &dataObj)
{
    if (!dataObj._net.connected())
    {
        dataObj._contentLength = -1;
        dataObj._dataType = FirebaseDataType::NULL_;
        dataObj._httpCode = HTTPC_ERROR_NOT_CONNECTED;
        dataObj._data.clear();
        dataObj._path.clear();
        resetFirebasedataFlag(dataObj);
        return false;
    }
    return true;
}

bool FirebaseESP8266::reconnect(FirebaseData &dataObj)
{
    bool flag = reconnect();
    if (!flag)
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
    return flag;
}

bool FirebaseESP8266::reconnect()
{
    if (_reconnectWiFi && WiFi.status() != WL_CONNECTED)
    {
        if (_lastReconnectMillis == 0)
        {
            WiFi.reconnect();
            _lastReconnectMillis = millis();
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            if (millis() - _lastReconnectMillis > _reconnectTimeout)
                _lastReconnectMillis = 0;
            return false;
        }
        else
        {
            _lastReconnectMillis = 0;
        }
    }
    return WiFi.status() == WL_CONNECTED;
}

void FirebaseESP8266::errorToString(int httpCode, std::string &buff)
{

    buff.clear();
    switch (httpCode)
    {
    case HTTPC_ERROR_CONNECTION_REFUSED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_39);
        return;
    case HTTPC_ERROR_SEND_HEADER_FAILED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_40);
        return;
    case HTTPC_ERROR_SEND_PAYLOAD_FAILED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_41);
        return;
    case HTTPC_ERROR_NOT_CONNECTED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_42);
        return;
    case HTTPC_ERROR_CONNECTION_LOST:
        p_memCopy(buff, ESP8266_FIREBASE_STR_43);
        return;
    case HTTPC_ERROR_NO_HTTP_SERVER:
        p_memCopy(buff, ESP8266_FIREBASE_STR_44);
        return;
    case _HTTP_CODE_BAD_REQUEST:
        p_memCopy(buff, ESP8266_FIREBASE_STR_45);
        return;
    case _HTTP_CODE_NON_AUTHORITATIVE_INFORMATION:
        p_memCopy(buff, ESP8266_FIREBASE_STR_46);
        return;
    case _HTTP_CODE_NO_CONTENT:
        p_memCopy(buff, ESP8266_FIREBASE_STR_47);
        return;
    case _HTTP_CODE_MOVED_PERMANENTLY:
        p_memCopy(buff, ESP8266_FIREBASE_STR_48);
        return;
    case _HTTP_CODE_USE_PROXY:
        p_memCopy(buff, ESP8266_FIREBASE_STR_49);
        return;
    case _HTTP_CODE_TEMPORARY_REDIRECT:
        p_memCopy(buff, ESP8266_FIREBASE_STR_50);
        return;
    case _HTTP_CODE_PERMANENT_REDIRECT:
        p_memCopy(buff, ESP8266_FIREBASE_STR_51);
        return;
    case _HTTP_CODE_UNAUTHORIZED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_52);
        return;
    case _HTTP_CODE_FORBIDDEN:
        p_memCopy(buff, ESP8266_FIREBASE_STR_53);
        return;
    case _HTTP_CODE_NOT_FOUND:
        p_memCopy(buff, ESP8266_FIREBASE_STR_54);
        return;
    case _HTTP_CODE_METHOD_NOT_ALLOWED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_55);
        return;
    case _HTTP_CODE_NOT_ACCEPTABLE:
        p_memCopy(buff, ESP8266_FIREBASE_STR_56);
        return;
    case _HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_57);
        return;
    case _HTTP_CODE_REQUEST_TIMEOUT:
        p_memCopy(buff, ESP8266_FIREBASE_STR_58);
        return;
    case _HTTP_CODE_LENGTH_REQUIRED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_59);
        return;
    case _HTTP_CODE_TOO_MANY_REQUESTS:
        p_memCopy(buff, ESP8266_FIREBASE_STR_60);
        return;
    case _HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE:
        p_memCopy(buff, ESP8266_FIREBASE_STR_61);
        return;
    case _HTTP_CODE_INTERNAL_SERVER_ERROR:
        p_memCopy(buff, ESP8266_FIREBASE_STR_62);
        return;
    case _HTTP_CODE_BAD_GATEWAY:
        p_memCopy(buff, ESP8266_FIREBASE_STR_63);
        return;
    case _HTTP_CODE_SERVICE_UNAVAILABLE:
        p_memCopy(buff, ESP8266_FIREBASE_STR_64);
        return;
    case _HTTP_CODE_GATEWAY_TIMEOUT:
        p_memCopy(buff, ESP8266_FIREBASE_STR_65);
        return;
    case _HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_66);
        return;
    case _HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_67);
        return;
    case _HTTP_CODE_PRECONDITION_FAILED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_152);
        return;
    case HTTPC_ERROR_READ_TIMEOUT:
        p_memCopy(buff, ESP8266_FIREBASE_STR_69);
        return;
    case FIREBASE_ERROR_DATA_TYPE_MISMATCH:
        p_memCopy(buff, ESP8266_FIREBASE_STR_70);
        return;
    case FIREBASE_ERROR_PATH_NOT_EXIST:
        p_memCopy(buff, ESP8266_FIREBASE_STR_71);
        return;
    case HTTPC_ERROR_CONNECTION_INUSED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_94);
        return;
    case FIREBASE_ERROR_BUFFER_OVERFLOW:
        p_memCopy(buff, ESP8266_FIREBASE_STR_68);
        return;
    case HTTPC_NO_FCM_TOPIC_PROVIDED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_144);
        return;
    case HTTPC_NO_FCM_DEVICE_TOKEN_PROVIDED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_145);
        return;
    case HTTPC_NO_FCM_SERVER_KEY_PROVIDED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_146);
        return;
    case HTTPC_NO_FCM_INDEX_NOT_FOUND_IN_DEVICE_TOKEN_PROVIDED:
        p_memCopy(buff, ESP8266_FIREBASE_STR_147);
        return;

    default:
        return;
    }
}

bool FirebaseESP8266::sendFCMMessage(FirebaseData &dataObj, uint8_t messageType)
{

    if (dataObj.fcm._server_key.length() == 0)
    {
        dataObj._httpCode = HTTPC_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    if (dataObj.fcm._deviceToken.size() == 0 && messageType == FirebaseESP8266::FCMMessageType::SINGLE)
    {
        dataObj._httpCode = HTTPC_NO_FCM_DEVICE_TOKEN_PROVIDED;
        return false;
    }

    if (messageType == FirebaseESP8266::FCMMessageType::SINGLE && dataObj.fcm._deviceToken.size() > 0 && dataObj.fcm._index > dataObj.fcm._deviceToken.size() - 1)
    {
        dataObj._httpCode = HTTPC_NO_FCM_INDEX_NOT_FOUND_IN_DEVICE_TOKEN_PROVIDED;
        return false;
    }

    if (messageType == FirebaseESP8266::FCMMessageType::TOPIC && dataObj.fcm._topic.length() == 0)
    {
        dataObj._httpCode = HTTPC_NO_FCM_TOPIC_PROVIDED;
        return false;
    }

    if (!reconnect(dataObj))
        return false;

    bool res = false;
    unsigned long lastTime = millis();

    if (dataObj._streamCall || dataObj._firebaseCall || dataObj._fcmCall)
        while ((dataObj._streamCall || dataObj._firebaseCall || dataObj._fcmCall) && millis() - lastTime < 1000)
            delay(1);

    if (dataObj._streamCall || dataObj._firebaseCall || dataObj._fcmCall)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_INUSED;
        return false;
    }

    dataObj._fcmCall = true;

    if (dataObj._net.connected())
        forceEndHTTP(dataObj);

    res = dataObj.fcm.fcm_connect(dataObj._net);

    if (!res)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_REFUSED;
        dataObj._fcmCall = false;
        return false;
    }

    res = dataObj.fcm.fcm_send(dataObj._net, dataObj._httpCode, messageType);
    dataObj._fcmCall = false;
    return res;
}

bool FirebaseESP8266::sendMessage(FirebaseData &dataObj, uint16_t index)
{
    setSecure(dataObj);
    dataObj.fcm._index = index;
    return sendFCMMessage(dataObj, FirebaseESP8266::FCMMessageType::SINGLE);
}

bool FirebaseESP8266::broadcastMessage(FirebaseData &dataObj)
{
    setSecure(dataObj);
    return sendFCMMessage(dataObj, FirebaseESP8266::FCMMessageType::MULTICAST);
}

bool FirebaseESP8266::sendTopic(FirebaseData &dataObj)
{
    setSecure(dataObj);
    return sendFCMMessage(dataObj, FirebaseESP8266::FCMMessageType::TOPIC);
}

void FirebaseESP8266::setStreamCallback(FirebaseData &dataObj, StreamEventCallback dataAvailablecallback, StreamTimeoutCallback timeoutCallback)
{
    int index = dataObj._index;

    bool hasHandle = false;

    if (dataObj._index != -1 || dataObj._Qindex != -1)
        hasHandle = true;
    else
    {
        index = dataObjectIndex;
        dataObjectIndex++;
    }

    dataObj._index = index;
    dataObj._dataAvailableCallback = dataAvailablecallback;
    dataObj._timeoutCallback = timeoutCallback;

    //object created
    if (hasHandle)
        firebaseDataObject[index] = dataObj;
    else
        firebaseDataObject.push_back(dataObj);

    if (!_callback_function)
        set_scheduled_callback(std::bind(&FirebaseESP8266::processFirebaseStream, this));
}

void FirebaseESP8266::removeStreamCallback(FirebaseData &dataObj)
{
    int index = dataObj._index;

    if (index != -1)
    {

        dataObj._index = -1;
        dataObj._dataAvailableCallback = NULL;
        dataObj._timeoutCallback = NULL;

        firebaseDataObject.erase(firebaseDataObject.begin() + index);
    }
}

void FirebaseESP8266::beginAutoRunErrorQueue(FirebaseData &dataObj, QueueInfoCallback callback)
{

    int index = dataObj._Qindex;

    bool hasHandle = false;

    if (dataObj._index != -1 || dataObj._Qindex != -1)
        hasHandle = true;
    else
    {
        index = dataObjectIndex;
        dataObjectIndex++;
    }

    dataObj._Qindex = index;

    if (callback)
        dataObj._queueInfoCallback = callback;
    else
        dataObj._queueInfoCallback = NULL;

    //object created
    if (hasHandle)
        firebaseDataObject[index] = dataObj;
    else
        firebaseDataObject.push_back(dataObj);

    set_scheduled_callback(std::bind(&FirebaseESP8266::processAllErrorQueues, this));
}

void FirebaseESP8266::clearErrorQueue(FirebaseData &dataObj)
{
    for (uint8_t i = 0; i < dataObj._qMan._queueCollection.size(); i++)
    {
        QueueItem item = dataObj._qMan._queueCollection[i];
        dataObj.clearQueueItem(item);
    }
}

bool FirebaseESP8266::backup(FirebaseData &dataObj, uint8_t storageType, const String &nodePath, const String &fileName)
{
    dataObj._backupDir.clear();
    dataObj._backupNodePath = nodePath.c_str();
    dataObj._backupFilename = fileName.c_str();
    dataObj._fileName.clear();
    bool flag = sendRequest(dataObj, storageType, nodePath.c_str(), FirebaseMethod::BACKUP, FirebaseDataType::JSON, "", "", "");
    return flag;
}

bool FirebaseESP8266::restore(FirebaseData &dataObj, uint8_t storageType, const String &nodePath, const String &fileName)
{
    dataObj._backupDir.clear();
    dataObj._backupNodePath = nodePath.c_str();
    dataObj._backupFilename = fileName.c_str();
    dataObj._fileName.clear();
    bool flag = sendRequest(dataObj, storageType, nodePath.c_str(), FirebaseMethod::RESTORE, FirebaseDataType::JSON, "", "", "");
    return flag;
}

void FirebaseESP8266::setMaxRetry(FirebaseData &dataObj, uint8_t num)
{
    dataObj._maxRetry = num;
}

void FirebaseESP8266::setMaxErrorQueue(FirebaseData &dataObj, uint8_t num)
{
    dataObj._qMan._maxQueue = num;

    if (dataObj._qMan._queueCollection.size() > num)
    {
        for (uint8_t i = dataObj._qMan._queueCollection.size() - 1; i >= num; i--)
        {
            QueueItem item = dataObj._qMan._queueCollection[i];
            dataObj.clearQueueItem(item);
        }
    }
}

bool FirebaseESP8266::saveErrorQueue(FirebaseData &dataObj, const String &filename, uint8_t storageType)
{

    if (storageType == StorageType::SD)
    {
        if (!sdTest())
            return false;
        file = SD.open(filename.c_str(), FILE_WRITE);
    }
    else if (storageType == StorageType::SPIFFS)
    {
        SPIFFS.begin();
        _file = SPIFFS.open(filename.c_str(), "w");
    }

    if ((storageType == StorageType::SPIFFS && !_file) || (storageType == StorageType::SD && !file))
        return false;

    uint8_t idx = 0;
    std::string buff = "";

    size_t nbufSize = 10;
    char *nbuf = new char[nbufSize];

    for (uint8_t i = 0; i < dataObj._qMan._queueCollection.size(); i++)
    {
        QueueItem item = dataObj._qMan._queueCollection[i];

        if (item.firebaseMethod != FirebaseESP8266::FirebaseMethod::GET)
        {
            if (idx > 0)
                buff.append("\r");

            memset(nbuf, 0, nbufSize);
            itoa(item.firebaseDataType, nbuf, 10);
            buff.append(nbuf);
            buff.append("~");

            memset(nbuf, 0, nbufSize);
            itoa(item.firebaseMethod, nbuf, 10);
            buff.append(nbuf);
            buff.append("~");

            buff += item.path.c_str();
            buff.append("~");

            buff += item.payload.c_str();
            buff.append("~");

            for (size_t j = 0; j < item.blob.size(); j++)
            {
                memset(nbuf, 0, nbufSize);
                itoa(item.blob[j], nbuf, 10);
            }

            buff.append("~");

            buff += item.filename.c_str();

            memset(nbuf, 0, nbufSize);
            itoa(item.storageType, nbuf, 10);
            buff.append(nbuf);
            buff.append("~");

            idx++;
        }
    }
    if (storageType == StorageType::SD)
    {

        file.print(buff.c_str());
        file.close();
    }
    else if (storageType == StorageType::SPIFFS)
    {

        _file.print(buff.c_str());
        _file.close();
    }

    delete[] nbuf;
    std::string().swap(buff);

    return true;
}

bool FirebaseESP8266::restoreErrorQueue(FirebaseData &dataObj, const String &filename, uint8_t storageType)
{
    return openErrorQueue(dataObj, filename, storageType, 1) != 0;
}

uint8_t FirebaseESP8266::errorQueueCount(FirebaseData &dataObj, const String &filename, uint8_t storageType)
{
    return openErrorQueue(dataObj, filename, storageType, 0);
}

bool FirebaseESP8266::deleteStorageFile(const String &filename, uint8_t storageType)
{

    if (storageType == StorageType::SD)
    {
        if (!sdTest())
            return false;
        return SD.remove(filename.c_str());
    }
    else
    {
        SPIFFS.begin();
        return SPIFFS.remove(filename.c_str());
    }
}

uint8_t FirebaseESP8266::openErrorQueue(FirebaseData &dataObj, const String &filename, uint8_t storageType, uint8_t mode)
{

    uint8_t count = 0;

    if (storageType == StorageType::SD)
    {
        if (!sdTest())
            return 0;
        file = SD.open(filename.c_str(), FILE_READ);
    }
    else if (storageType == StorageType::SPIFFS)
    {
        SPIFFS.begin();
        _file = SPIFFS.open(filename.c_str(), "r");
    }

    if ((storageType == StorageType::SPIFFS && !_file) || (storageType == StorageType::SD && !file))
        return 0;

    std::string t = "";

    uint8_t c = 0;

    while (file.available() || _file.available())
    {
        if (storageType == StorageType::SPIFFS)
            c = _file.read();
        if (storageType == StorageType::SD)
            c = file.read();

        t += (char)c;
    }

    if (storageType == StorageType::SPIFFS)
        _file.close();
    else if (storageType == StorageType::SD)
        file.close();

    std::vector<std::string> p = splitString(dataObj._maxBlobSize, t.c_str(), '\r');

    for (size_t i = 0; i < p.size(); i++)
    {
        std::vector<std::string> q = splitString(dataObj._maxBlobSize, p[i].c_str(), '~');

        if (q.size() >= 6)
        {
            count++;

            if (mode == 1)
            {
                //Restore Firebase Error Queues
                QueueItem item;

                item.firebaseDataType = atoi(q[0].c_str());
                item.firebaseMethod = atoi(q[1].c_str());
                item.path.append(q[2].c_str());
                item.payload.append(q[3].c_str());

                for (size_t j = 0; j < q[4].length(); j++)
                    item.blob.push_back(atoi(q[4].c_str()));

                item.filename.append(q[5].c_str());

                //backwards compatibility to old APIs
                if (q.size() == 7)
                    item.storageType = atoi(q[6].c_str());

                dataObj._qMan._queueCollection.push_back(item);
            }
        }
    }
    std::string().swap(t);

    return count;
}

bool FirebaseESP8266::isErrorQueueFull(FirebaseData &dataObj)
{
    if (dataObj._qMan._maxQueue > 0)
        return dataObj._qMan._queueCollection.size() >= dataObj._qMan._maxQueue;
    return false;
}

uint8_t FirebaseESP8266::errorQueueCount(FirebaseData &dataObj)
{
    return dataObj._qMan._queueCollection.size();
}

std::vector<std::string> FirebaseESP8266::splitString(int size, const char *str, const char delim)
{
    uint16_t index = 0;
    uint16_t len = strlen(str);
    int buffSize = (int)(size * 1.4f);
    char *buff = new char[buffSize];
    std::vector<std::string> out;

    for (uint16_t i = 0; i < len; i++)
    {
        if (str[i] == delim)
        {
            memset(buff, 0, buffSize);
            strncpy(buff, (char *)str + index, i - index);
            buff[i - index] = '\0';
            index = i + 1;
            out.push_back(buff);
        }
    }
    if (index < len + 1)
    {
        memset(buff, 0, buffSize);
        strncpy(buff, (char *)str + index, len - index);
        buff[len - index] = '\0';
        out.push_back(buff);
    }

    delete[] buff;

    return out;
}

void FirebaseESP8266::processFirebaseStream()
{

    for (size_t id = 0; id < firebaseDataObject.size(); id++)
    {
        if ((firebaseDataObject[id].get()._dataAvailableCallback || firebaseDataObject[id].get()._timeoutCallback))
        {

            Firebase.readStream(firebaseDataObject[id].get());

            if (firebaseDataObject[id].get().streamTimeout() && firebaseDataObject[id].get()._timeoutCallback)
                firebaseDataObject[id].get()._timeoutCallback(true);

            if (firebaseDataObject[id].get().streamAvailable() && firebaseDataObject[id].get()._dataAvailableCallback)
            {

                StreamData s;
                s._json = &firebaseDataObject[id].get()._json;
                s._streamPath = firebaseDataObject[id].get()._streamPath;
                s._data = firebaseDataObject[id].get()._data;
                s._path = firebaseDataObject[id].get()._path;

                s._dataType = firebaseDataObject[id].get()._dataType;
                s._dataTypeStr = firebaseDataObject[id].get().getDataType(s._dataType);
                s._eventTypeStr = firebaseDataObject[id].get()._eventType;

                if (s._dataType == FirebaseESP8266::FirebaseDataType::BLOB)
                {
                    s._blob = firebaseDataObject[id].get()._blob;
                    //Free ram in case of callback data was used
                    firebaseDataObject[id].get()._blob.clear();
                }

                firebaseDataObject[id].get()._dataAvailableCallback(s);
                s.empty();
            }
        }
    }

    set_scheduled_callback(std::bind(&FirebaseESP8266::processFirebaseStream, this));
}

void FirebaseESP8266::processAllErrorQueues()
{

    for (size_t id = 0; id < firebaseDataObject.size(); id++)
    {
        if (firebaseDataObject[id].get()._queueInfoCallback)
            Firebase.processErrorQueue(firebaseDataObject[id].get(), firebaseDataObject[id].get()._queueInfoCallback);
        else
            Firebase.processErrorQueue(firebaseDataObject[id].get(), NULL);
    }

    set_scheduled_callback(std::bind(&FirebaseESP8266::processAllErrorQueues, this));
}

void FirebaseESP8266::p_memCopy(std::string &buff, const char *p, bool empty)
{
    if (empty)
        buff.clear();
    size_t len = strlen_P(p) + 10;
    char *b = new char[len];
    memset(b, 0, len);
    strcpy(b, p);
    buff += b;
    delete[] b;
}

bool FirebaseESP8266::sdTest()
{
    File file;
    std::string filepath = "";
    p_memCopy(filepath, ESP8266_FIREBASE_STR_73, true);

    SD.begin(SD_CS_PIN);
    _sdPin = SD_CS_PIN;

    file = SD.open(filepath.c_str(), FILE_WRITE);
    if (!file)
        goto EXIT_5;

    if (!file.write(32))
        goto EXIT_5;
    file.close();

    file = SD.open(filepath.c_str());
    if (!file)
        goto EXIT_5;

    while (file.available())
    {
        if (file.read() != 32)
            goto EXIT_5;
    }
    file.close();

    SD.remove(filepath.c_str());

    std::string().swap(filepath);

    return true;

EXIT_5:
    std::string().swap(filepath);
    return false;
}

void FirebaseESP8266::createDirs(std::string dirs, uint8_t storageType)
{
    std::string dir = "";
    size_t count = 0;
    for (size_t i = 0; i < dirs.length(); i++)
    {
        dir.append(1, dirs[i]);
        count++;
        if (dirs[i] == '/')
        {
            if (dir.length() > 0)
            {
                if (storageType == StorageType::SD)
                    SD.mkdir(dir.substr(0, dir.length() - 1).c_str());
            }

            count = 0;
        }
    }
    if (count > 0)
    {
        if (storageType == StorageType::SD)
            SD.mkdir(dir.c_str());
    }
    std::string().swap(dir);
}

bool FirebaseESP8266::replace(std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

std::string FirebaseESP8266::base64_encode_string(const unsigned char *src, size_t len)
{
    unsigned char *out, *pos;
    const unsigned char *end, *in;

    size_t olen;

    olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

    if (olen < len)
        return std::string();

    std::string outStr;
    outStr.resize(olen);
    out = (unsigned char *)&outStr[0];

    end = src + len;
    in = src;
    pos = out;

    while (end - in >= 3)
    {
        *pos++ = ESP8266_FIREBASE_base64_table[in[0] >> 2];
        *pos++ = ESP8266_FIREBASE_base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = ESP8266_FIREBASE_base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = ESP8266_FIREBASE_base64_table[in[2] & 0x3f];
        in += 3;
        yield();
    }

    if (end - in)
    {
        *pos++ = ESP8266_FIREBASE_base64_table[in[0] >> 2];
        if (end - in == 1)
        {
            *pos++ = ESP8266_FIREBASE_base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else
        {
            *pos++ = ESP8266_FIREBASE_base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = ESP8266_FIREBASE_base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    return outStr;
}

void FirebaseESP8266::send_base64_encode_file(SSL_CLIENT *_client, const std::string &filePath, uint8_t storageType)
{
    if (storageType == StorageType::SPIFFS)
        _file = SPIFFS.open(filePath.c_str(), "r");
    else if (storageType == StorageType::SD)
        file = SD.open(filePath.c_str(), FILE_READ);

    if ((storageType == StorageType::SPIFFS && !_file) || (storageType == StorageType::SD && !file))
        return;

    size_t chunkSize = 512;
    size_t fbufSize = 3;
    size_t byteAdd = 0;
    size_t byteSent = 0;

    unsigned char *buff = new unsigned char[chunkSize];
    memset(buff, 0, chunkSize);

    size_t len = 0;

    if (storageType == StorageType::SPIFFS)
        len = _file.size();
    else if (storageType == StorageType::SD)
        len = file.size();

    size_t fbufIndex = 0;
    unsigned char *fbuf = new unsigned char[3];

    while (file.available() || _file.available())
    {
        memset(fbuf, 0, fbufSize);
        if (len - fbufIndex >= 3)
        {
            if (storageType == StorageType::SPIFFS)
                _file.read(fbuf, 3);
            else if (storageType == StorageType::SD)
                file.read(fbuf, 3);

            buff[byteAdd++] = ESP8266_FIREBASE_base64_table[fbuf[0] >> 2];
            buff[byteAdd++] = ESP8266_FIREBASE_base64_table[((fbuf[0] & 0x03) << 4) | (fbuf[1] >> 4)];
            buff[byteAdd++] = ESP8266_FIREBASE_base64_table[((fbuf[1] & 0x0f) << 2) | (fbuf[2] >> 6)];
            buff[byteAdd++] = ESP8266_FIREBASE_base64_table[fbuf[2] & 0x3f];

            if (len > chunkSize)
            {
                if (byteAdd >= chunkSize)
                {
                    byteSent += byteAdd;
                    _client->write(buff, byteAdd);
                    memset(buff, 0, chunkSize);
                    byteAdd = 0;
                }
            }
            fbufIndex += 3;

            yield();
        }
        else
        {
            if (len - fbufIndex == 1)
            {
                if (storageType == StorageType::SPIFFS)
                    fbuf[0] = _file.read();
                else if (storageType == StorageType::SD)
                    fbuf[0] = file.read();
            }
            else if (len - fbufIndex == 2)
            {
                if (storageType == StorageType::SPIFFS)
                {
                    fbuf[0] = _file.read();
                    fbuf[1] = _file.read();
                }
                else if (storageType == StorageType::SD)
                {
                    fbuf[0] = file.read();
                    fbuf[1] = file.read();
                }
            }

            break;
        }
    }

    if (storageType == StorageType::SPIFFS)
        _file.close();
    else if (storageType == StorageType::SD)
        file.close();

    if (byteAdd > 0)
        _client->write(buff, byteAdd);

    if (len - fbufIndex > 0)
    {
        memset(buff, 0, chunkSize);
        byteAdd = 0;

        buff[byteAdd++] = ESP8266_FIREBASE_base64_table[fbuf[0] >> 2];
        if (len - fbufIndex == 1)
        {
            buff[byteAdd++] = ESP8266_FIREBASE_base64_table[(fbuf[0] & 0x03) << 4];
            buff[byteAdd++] = '=';
        }
        else
        {
            buff[byteAdd++] = ESP8266_FIREBASE_base64_table[((fbuf[0] & 0x03) << 4) | (fbuf[1] >> 4)];
            buff[byteAdd++] = ESP8266_FIREBASE_base64_table[(fbuf[1] & 0x0f) << 2];
        }
        buff[byteAdd++] = '=';

        _client->write(buff, byteAdd);
    }

    delete[] buff;
    delete[] fbuf;
}

bool FirebaseESP8266::base64_decode_string(const std::string src, std::vector<uint8_t> &out)
{
    unsigned char *dtable = new unsigned char[256];
    memset(dtable, 0x80, 256);
    for (size_t i = 0; i < sizeof(ESP8266_FIREBASE_base64_table) - 1; i++)
        dtable[ESP8266_FIREBASE_base64_table[i]] = (unsigned char)i;
    dtable['='] = 0;

    unsigned char *block = new unsigned char[4];
    unsigned char tmp;
    size_t i, count;
    int pad = 0;
    size_t extra_pad;
    size_t len = src.length();

    count = 0;

    for (i = 0; i < len; i++)
    {
        if (dtable[(uint8_t)src[i]] != 0x80)
            count++;
    }

    if (count == 0)
        goto exit;

    extra_pad = (4 - count % 4) % 4;

    count = 0;
    for (i = 0; i < len + extra_pad; i++)
    {
        unsigned char val;

        if (i >= len)
            val = '=';
        else
            val = src[i];

        tmp = dtable[val];

        if (tmp == 0x80)
            continue;

        if (val == '=')
            pad++;

        block[count] = tmp;
        count++;
        if (count == 4)
        {
            out.push_back((block[0] << 2) | (block[1] >> 4));
            count = 0;
            if (pad)
            {
                if (pad == 1)
                    out.push_back((block[1] << 4) | (block[2] >> 2));
                else if (pad > 2)
                    goto exit;

                break;
            }
            else
            {
                out.push_back((block[1] << 4) | (block[2] >> 2));
                out.push_back((block[2] << 6) | block[3]);
            }
        }
    }

    delete[] block;
    delete[] dtable;

    return true;

exit:
    delete[] block;
    delete[] dtable;

    return false;
}

bool FirebaseESP8266::base64_decode_file(File &file, const char *src, size_t len)
{
    unsigned char *dtable = new unsigned char[256];
    memset(dtable, 0x80, 256);
    for (size_t i = 0; i < sizeof(ESP8266_FIREBASE_base64_table) - 1; i++)
        dtable[ESP8266_FIREBASE_base64_table[i]] = (unsigned char)i;
    dtable['='] = 0;

    unsigned char *block = new unsigned char[4];
    unsigned char tmp;
    size_t i, count;
    int pad = 0;
    size_t extra_pad;

    count = 0;

    for (i = 0; i < len; i++)
    {
        if (dtable[(uint8_t)src[i]] != 0x80)
            count++;
    }

    if (count == 0)
        goto exit;

    extra_pad = (4 - count % 4) % 4;

    count = 0;
    for (i = 0; i < len + extra_pad; i++)
    {
        unsigned char val;

        if (i >= len)
            val = '=';
        else
            val = src[i];
        tmp = dtable[val];
        if (tmp == 0x80)
            continue;

        if (val == '=')
            pad++;

        block[count] = tmp;
        count++;
        if (count == 4)
        {
            file.write((block[0] << 2) | (block[1] >> 4));
            count = 0;
            if (pad)
            {
                if (pad == 1)
                    file.write((block[1] << 4) | (block[2] >> 2));
                else if (pad > 2)
                    goto exit;

                break;
            }
            else
            {
                file.write((block[1] << 4) | (block[2] >> 2));
                file.write((block[2] << 6) | block[3]);
            }
        }
    }

    delete[] block;
    delete[] dtable;

    return true;

exit:

    delete[] block;
    delete[] dtable;

    return false;
}

bool FirebaseESP8266::base64_decode_SPIFFS(fs::File &file, const char *src, size_t len)
{
    unsigned char *dtable = new unsigned char[256];
    memset(dtable, 0x80, 256);
    for (size_t i = 0; i < sizeof(ESP8266_FIREBASE_base64_table) - 1; i++)
        dtable[ESP8266_FIREBASE_base64_table[i]] = (unsigned char)i;
    dtable['='] = 0;

    unsigned char *block = new unsigned char[4];
    unsigned char tmp;
    size_t i, count;
    int pad = 0;
    size_t extra_pad;

    count = 0;

    for (i = 0; i < len; i++)
    {
        if (dtable[(uint8_t)src[i]] != 0x80)
            count++;
    }

    if (count == 0)
        goto exit;

    extra_pad = (4 - count % 4) % 4;

    count = 0;
    for (i = 0; i < len + extra_pad; i++)
    {
        unsigned char val;

        if (i >= len)
            val = '=';
        else
            val = src[i];
        tmp = dtable[val];
        if (tmp == 0x80)
            continue;

        if (val == '=')
            pad++;

        block[count] = tmp;
        count++;
        if (count == 4)
        {
            file.write((block[0] << 2) | (block[1] >> 4));
            count = 0;
            if (pad)
            {
                if (pad == 1)
                    file.write((block[1] << 4) | (block[2] >> 2));
                else if (pad > 2)
                    goto exit;
                break;
            }
            else
            {
                file.write((block[1] << 4) | (block[2] >> 2));
                file.write((block[2] << 6) | block[3]);
            }
        }
    }

    delete[] block;
    delete[] dtable;

    return true;

exit:
    delete[] block;
    delete[] dtable;

    return false;
}

void FirebaseESP8266::strcat_c(char *str, char c)
{
    for (; *str; str++)
        ;
    *str++ = c;
    *str++ = 0;
}
int FirebaseESP8266::strpos(const char *haystack, const char *needle, int offset)
{
    size_t len = strlen(haystack);
    char *_haystack = new char[len];
    memset(_haystack, 0, len);
    strncpy(_haystack, haystack + offset, strlen(haystack) - offset);
    char *p = strstr(_haystack, needle);
    int r = -1;
    if (p)
        r = p - _haystack + offset;
    delete[] _haystack;
    return r;
}

int FirebaseESP8266::rstrpos(const char *haystack, const char *needle, int offset)
{
    size_t len = strlen(haystack);
    char *_haystack = new char[len];
    memset(_haystack, 0, len);
    strncpy(_haystack, haystack + offset, len - offset);
    char *p = rstrstr(_haystack, needle);
    int r = -1;
    if (p)
        r = p - _haystack + offset;
    delete[] _haystack;
    return r;
}
char *FirebaseESP8266::rstrstr(const char *haystack, const char *needle)
{
    size_t needle_length = strlen(needle);
    const char *haystack_end = haystack + strlen(haystack) - needle_length;
    const char *p;
    size_t i;
    for (p = haystack_end; p >= haystack; --p)
    {
        for (i = 0; i < needle_length; ++i)
        {
            if (p[i] != needle[i])
                goto next;
        }
        return (char *)p;
    next:;
    }
    return 0;
}

void FirebaseESP8266::setClock()
{
    reconnect();
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    time_t now = time(nullptr);
    uint8_t tryCount = 0;
    while (now < 8 * 3600 * 2)
    {
        delay(100);
        now = time(nullptr);
        tryCount++;
        if (tryCount > 50)
            break;
    }

    if (tryCount <= 50)
        _clockReady = true;

    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
}

FirebaseData::FirebaseData() {}

FirebaseData ::~FirebaseData()
{
    clear();
}

void FirebaseData::clear()
{
    std::string().swap(_path);
    std::string().swap(_path2);
    std::string().swap(_data);
    std::string().swap(_data2);
    std::string().swap(_streamPath);
    std::string().swap(_pushName);
    std::string().swap(_backupNodePath);
    std::string().swap(_backupDir);
    std::string().swap(_backupFilename);
    std::string().swap(_file_transfer_error);
    std::string().swap(_fileName);
    std::string().swap(_redirectURL);
    std::string().swap(_firebaseError);
    std::string().swap(_backupNodePath);
    std::string().swap(_backupDir);
    std::string().swap(_backupFilename);
    std::string().swap(_eventType);
    std::string().swap(_etag);
    std::string().swap(_etag2);
    std::string().swap(_priority);

    for (uint8_t i = 0; i < _qMan._queueCollection.size(); i++)
    {
        QueueItem item = _qMan._queueCollection[i];
        clearQueueItem(item);
    }
}

void FirebaseData::addQueue(uint8_t FirebaseMethod,
                            uint8_t storageType,
                            uint8_t FirebaseDataType,
                            const std::string path,
                            const std::string filename,
                            const std::string payload,
                            bool isQuery,
                            int *intTarget,
                            float *floatTarget,
                            double *doubleTarget,
                            bool *boolTarget,
                            String *stringTarget,
                            std::vector<uint8_t> *blobTarget,
                            FirebaseJson *jsonTarget,
                            FirebaseJsonArray *arrTarget)
{
    if (_qMan._queueCollection.size() < _qMan._maxQueue && payload.length() <= _maxBlobSize)
    {
        QueueItem item;
        item.firebaseMethod = FirebaseMethod;
        item.firebaseDataType = FirebaseDataType;
        item.path = path;
        item.filename = filename;
        item.payload = payload;

        if (isQuery)
            item.queryFilter = queryFilter;
        else
            item.queryFilter.clear();

        item.stringPtr = stringTarget;
        item.intPtr = intTarget;
        item.floatPtr = floatTarget;
        item.doublePtr = doubleTarget;
        item.boolPtr = boolTarget;
        item.jsonPtr = jsonTarget;
        item.arrPtr = arrTarget;
        item.blobPtr = blobTarget;
        item.storageType = storageType;
        item.qID = random(100000, 200000);

        if (_qMan.add(item))
            _qID = item.qID;
        else
            _qID = 0;
    }
}

void FirebaseData::clearQueueItem(QueueItem &item)
{
    std::string().swap(item.path);
    std::string().swap(item.filename);
    std::string().swap(item.payload);

    item.stringPtr = nullptr;
    item.intPtr = nullptr;
    item.floatPtr = nullptr;
    item.doublePtr = nullptr;
    item.boolPtr = nullptr;
    item.blobPtr = nullptr;
    item.queryFilter.clear();
}

void FirebaseData::setQuery(QueryFilter &query)
{
    queryFilter._orderBy = query._orderBy;
    queryFilter._limitToFirst = query._limitToFirst;
    queryFilter._limitToLast = query._limitToLast;
    queryFilter._startAt = query._startAt;
    queryFilter._endAt = query._endAt;
    queryFilter._equalTo = query._equalTo;
}

SSL_CLIENT FirebaseData::getWiFiClient()
{
    return *_net._client;
}

bool FirebaseData::pauseFirebase(bool pause)
{

    if (WiFi.status() != WL_CONNECTED)
        return false;

    if (_net.connected() && pause != _pause)
    {
        if (_net._client->available() > 0)
            _net._client->read();

        _net._client->stop();
        delay(50);
        if (!_net.connected())
        {
            _pause = pause;
            return true;
        }
        return false;
    }
    else
    {
        _pause = pause;
        return true;
    }
}

String FirebaseData::dataType()
{
    switch (_dataType)
    {
    case FirebaseESP8266::FirebaseDataType::JSON:
        return FPSTR(ESP8266_FIREBASE_STR_74);
    case FirebaseESP8266::FirebaseDataType::ARRAY:
        return FPSTR(ESP8266_FIREBASE_STR_174);
    case FirebaseESP8266::FirebaseDataType::STRING:
        return FPSTR(ESP8266_FIREBASE_STR_75);
    case FirebaseESP8266::FirebaseDataType::FLOAT:
        return FPSTR(ESP8266_FIREBASE_STR_76);
    case FirebaseESP8266::FirebaseDataType::DOUBLE:
        return FPSTR(ESP8266_FIREBASE_STR_108);
    case FirebaseESP8266::FirebaseDataType::BOOLEAN:
        return FPSTR(ESP8266_FIREBASE_STR_105);
    case FirebaseESP8266::FirebaseDataType::INTEGER:
        return FPSTR(ESP8266_FIREBASE_STR_77);
    case FirebaseESP8266::FirebaseDataType::BLOB:
        return FPSTR(ESP8266_FIREBASE_STR_91);
    case FirebaseESP8266::FirebaseDataType::NULL_:
        return FPSTR(ESP8266_FIREBASE_STR_78);
    default:
        return std::string().c_str();
    }
}

String FirebaseData::eventType()
{
    std::string res = _eventType;
    return res.c_str();
}

String FirebaseData::ETag()
{
    std::string res = _etag;
    return res.c_str();
}

std::string FirebaseData::getDataType(uint8_t type)
{
    std::string res = "";
    char *t = new char[100];
    memset(t, 0, 100);

    if (type == FirebaseESP8266::FirebaseDataType::JSON)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_74);
        goto ex_get_type;
    }

    if (type == FirebaseESP8266::FirebaseDataType::ARRAY)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_174);
        goto ex_get_type;
    }

    if (type == FirebaseESP8266::FirebaseDataType::STRING)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_75);
        goto ex_get_type;
    }

    if (type == FirebaseESP8266::FirebaseDataType::FLOAT)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_76);
        goto ex_get_type;
    }

    if (type == FirebaseESP8266::FirebaseDataType::DOUBLE)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_108);
        goto ex_get_type;
    }

    if (type == FirebaseESP8266::FirebaseDataType::BOOLEAN)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_105);
        goto ex_get_type;
    }

    if (type == FirebaseESP8266::FirebaseDataType::INTEGER)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_77);
        goto ex_get_type;
    }

    if (type == FirebaseESP8266::FirebaseDataType::BLOB)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_91);
        goto ex_get_type;
    }

    if (type == FirebaseESP8266::FirebaseDataType::NULL_)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_78);
        goto ex_get_type;
    }

ex_get_type:

    res = t;
    delete[] t;

    return res;
}

std::string FirebaseData::getMethod(uint8_t method)
{
    std::string res = "";
    char *t = new char[100];
    memset(t, 0, 100);

    if (method == FirebaseESP8266::FirebaseMethod::GET)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_164);
        goto ex_get_method;
    }

    if (method == FirebaseESP8266::FirebaseMethod::PUT || method == FirebaseESP8266::FirebaseMethod::PATCH_SILENT)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_165);
        goto ex_get_method;
    }

    if (method == FirebaseESP8266::FirebaseMethod::POST)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_166);
        goto ex_get_method;
    }

    if (method == FirebaseESP8266::FirebaseMethod::PATCH || method == FirebaseESP8266::FirebaseMethod::PATCH_SILENT)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_167);
        goto ex_get_method;
    }

    if (method == FirebaseESP8266::FirebaseMethod::DELETE)
    {
        strcpy_P(t, ESP8266_FIREBASE_STR_168);
        goto ex_get_method;
    }

ex_get_method:

    res = t;
    delete[] t;

    return res;
}

String FirebaseData::streamPath()
{
    return _streamPath.c_str();
}

String FirebaseData::dataPath()
{
    return _path.c_str();
}

int FirebaseData::intData()
{
    if (_data.length() > 0 && (_dataType == FirebaseESP8266::FirebaseDataType::INTEGER || _dataType == FirebaseESP8266::FirebaseDataType::FLOAT || _dataType == FirebaseESP8266::FirebaseDataType::DOUBLE))
    {
        if (_r_dataType == FirebaseESP8266::FirebaseDataType::TIMESTAMP)
        {
            double d = atof(_data.c_str());
            int ts = d / 1000;
            return ts;
        }
        else
            return atoi(_data.c_str());
    }
    else
        return 0;
}

float FirebaseData::floatData()
{
    if (_data.length() > 0 && (_dataType == FirebaseESP8266::FirebaseDataType::INTEGER || _dataType == FirebaseESP8266::FirebaseDataType::FLOAT || _dataType == FirebaseESP8266::FirebaseDataType::DOUBLE))
        return atof(_data.c_str());
    else
        return 0;
}

double FirebaseData::doubleData()
{
    if (_data.length() > 0 && (_dataType == FirebaseESP8266::FirebaseDataType::INTEGER || _dataType == FirebaseESP8266::FirebaseDataType::FLOAT || _dataType == FirebaseESP8266::FirebaseDataType::DOUBLE))
    {
        return atof(_data.c_str());
    }
    else
        return 0;
}

bool FirebaseData::boolData()
{
    bool res = false;
    char *str = new char[10];
    memset(str, 0, 10);
    strcat_P(str, ESP8266_FIREBASE_STR_107);

    if (_data.length() > 0 && _dataType == FirebaseESP8266::FirebaseDataType::BOOLEAN)
        res = _data == str;

    delete[] str;

    return res;
}

String FirebaseData::stringData()
{
    if (_data.length() > 0 && _dataType == FirebaseESP8266::FirebaseDataType::STRING)
        return _data.substr(1, _data.length() - 2).c_str();
    else
        return std::string().c_str();
}

String FirebaseData::jsonString()
{
    if (_data.length() > 0 && _dataType == FirebaseESP8266::FirebaseDataType::JSON)
        return String(_data.c_str());
    else
        return String();
}

FirebaseJson *FirebaseData::jsonObjectPtr()
{
    if (_data.length() > 0 && _dataType == FirebaseESP8266::FirebaseDataType::JSON)
    {
        _json._setJsonData(_data);
        return &_json;
    }
    return &_json;
}

FirebaseJson &FirebaseData::jsonObject()
{
    if (_data.length() > 0 && _dataType == FirebaseESP8266::FirebaseDataType::JSON)
        _json._setJsonData(_data);
    return _json;
}

FirebaseJsonArray *FirebaseData::jsonArrayPtr()
{
    jsonArray();
    return &_jsonArr;
}

FirebaseJsonArray &FirebaseData::jsonArray()
{
    if (_data.length() > 0 && _dataType == FirebaseESP8266::FirebaseDataType::ARRAY)
    {
        char *tmp = new char[20];
        memset(tmp, 0, 20);

        std::string().swap(_jsonArr._json._jsonData._dbuf);
        std::string().swap(_jsonArr._json._tbuf);

        strcpy_P(tmp, FirebaseJson_STR_21);
        _jsonArr._json._toStdString(_jsonArr._jbuf, false);
        _jsonArr._json._rawbuf = tmp;
        _jsonArr._json._rawbuf += _data;

        memset(tmp, 0, 20);
        strcpy_P(tmp, FirebaseJson_STR_26);

        _jsonArr._json._parse(tmp, PRINT_MODE_PLAIN);

        std::string().swap(_jsonArr._json._tbuf);
        std::string().swap(_jsonArr._jbuf);
        _jsonArr._json.clearPathTk();
        _jsonArr._json._tokens.reset();
        _jsonArr._json._tokens = nullptr;
        delete[] tmp;
        _jsonArr._json._rawbuf = _jsonArr._json._jsonData._dbuf.substr(1, _jsonArr._json._jsonData._dbuf.length() - 2);
        _jsonArr._arrLen = _jsonArr._json._jsonData._len;
    }
    return _jsonArr;
}

FirebaseJsonData &FirebaseData::jsonData()
{
    return _jsonData;
}

std::vector<uint8_t> FirebaseData::blobData()
{
    if (_blob.size() > 0 && _dataType == FirebaseESP8266::FirebaseDataType::BLOB)
    {
        return _blob;
    }
    else
        return std::vector<uint8_t>();
}

String FirebaseData::pushName()
{
    if (_pushName.length() > 0)
        return _pushName.c_str();
    else
        return std::string().c_str();
}

bool FirebaseData::isStream()
{
    return _isStream;
}

bool FirebaseData::httpConnected()
{
    return _httpConnected;
}

bool FirebaseData::streamTimeout()
{
    return _isStreamTimeout;
}

bool FirebaseData::dataAvailable()
{
    return _dataAvailable;
}

bool FirebaseData::streamAvailable()
{
    bool flag = !_httpConnected && _dataAvailable && _streamDataChanged;
    _dataAvailable = false;
    _streamDataChanged = false;
    return flag;
}

bool FirebaseData::mismatchDataType()
{
    return _mismatchDataType;
}
bool FirebaseData::bufferOverflow()
{
    return _bufferOverflow;
}

size_t FirebaseData::getBackupFileSize()
{
    return _backupzFileSize;
}

String FirebaseData::getBackupFilename()
{
    return _backupFilename.c_str();
}

String FirebaseData::fileTransferError()
{
    return _file_transfer_error.c_str();
}

String FirebaseData::payload()
{
    return _data.c_str();
}

String FirebaseData::errorReason()
{
    std::string buff = "";

    if (_httpCode == _HTTP_CODE_OK)
    {
        if (_bufferOverflow)
            _httpCode = FIREBASE_ERROR_BUFFER_OVERFLOW;
        else if (_mismatchDataType)
            _httpCode = FIREBASE_ERROR_DATA_TYPE_MISMATCH;
        else if (_pathNotExist)
            _httpCode = FIREBASE_ERROR_PATH_NOT_EXIST;
    }

    Firebase.errorToString(_httpCode, buff);

    if (_firebaseError != "")
    {
        char *str = new char[100];
        memset(str, 0, 100);
        strcpy_P(str, ESP8266_FIREBASE_STR_132);
        buff += str;
        memset(str, 0, 100);
        strcpy_P(str, ESP8266_FIREBASE_STR_6);
        buff += str;
        delete[] str;
        buff += _firebaseError;
    }

    return buff.c_str();
}

int FirebaseData::httpCode()
{
    return _httpCode;
}

StreamData::StreamData()
{
}

StreamData::~StreamData()
{
    empty();
}

String StreamData::dataPath()
{
    return _path.c_str();
}

String StreamData::streamPath()
{
    return _streamPath.c_str();
}

int StreamData::intData()
{
    if (_data.length() > 0 && (_dataType == FirebaseESP8266::FirebaseDataType::INTEGER || _dataType == FirebaseESP8266::FirebaseDataType::FLOAT || _dataType == FirebaseESP8266::FirebaseDataType::DOUBLE))
        return atoi(_data.c_str());
    else
        return 0;
}

float StreamData::floatData()
{
    if (_data.length() > 0 && (_dataType == FirebaseESP8266::FirebaseDataType::INTEGER || _dataType == FirebaseESP8266::FirebaseDataType::FLOAT || _dataType == FirebaseESP8266::FirebaseDataType::DOUBLE))
        return atof(_data.c_str());
    else
        return 0;
}

double StreamData::doubleData()
{

    if (_data.length() > 0 && (_dataType == FirebaseESP8266::FirebaseDataType::INTEGER || _dataType == FirebaseESP8266::FirebaseDataType::FLOAT || _dataType == FirebaseESP8266::FirebaseDataType::DOUBLE))
        return atof(_data.c_str());
    else
        return 0.0;
}

bool StreamData::boolData()
{
    bool res = false;
    char *str = new char[10];
    memset(str, 0, 10);
    strcat_P(str, ESP8266_FIREBASE_STR_107);

    if (_data.length() > 0 && _dataType == FirebaseESP8266::FirebaseDataType::BOOLEAN)
        res = _data == str;

    delete[] str;

    return res;
}

String StreamData::stringData()
{
    if (_dataType == FirebaseESP8266::FirebaseDataType::STRING)
        return _data.c_str();
    else
        return std::string().c_str();
}

String StreamData::jsonString()
{

    if (_dataType == FirebaseESP8266::FirebaseDataType::JSON)
        return _data.c_str();
    else
        return std::string().c_str();
}

FirebaseJson *StreamData::jsonObject()
{
    if (_dataType == FirebaseESP8266::FirebaseDataType::JSON)
    {
        _json->_setJsonData(_data);
        return _json;
    }
    else
    {
        return _json;
    }
}

FirebaseJsonArray *StreamData::jsonArray()
{
    if (_data.length() > 0 && _dataType == FirebaseESP8266::FirebaseDataType::ARRAY)
    {
        char *tmp = new char[20];
        memset(tmp, 0, 20);

        std::string().swap(_jsonArr->_json._jsonData._dbuf);
        std::string().swap(_jsonArr->_json._tbuf);

        strcpy_P(tmp, FirebaseJson_STR_21);
        _jsonArr->_json._toStdString(_jsonArr->_jbuf, false);
        _jsonArr->_json._rawbuf = tmp;
        _jsonArr->_json._rawbuf += _data;

        memset(tmp, 0, 20);
        strcpy_P(tmp, FirebaseJson_STR_26);

        _jsonArr->_json._parse(tmp, PRINT_MODE_PLAIN);

        std::string().swap(_jsonArr->_json._tbuf);
        std::string().swap(_jsonArr->_jbuf);
        _jsonArr->_json.clearPathTk();
        _jsonArr->_json._tokens.reset();
        _jsonArr->_json._tokens = nullptr;
        delete[] tmp;
        _jsonArr->_json._rawbuf = _jsonArr->_json._jsonData._dbuf.substr(1, _jsonArr->_json._jsonData._dbuf.length() - 2);
        _jsonArr->_arrLen = _jsonArr->_json._jsonData._len;
    }
    return _jsonArr;
}

FirebaseJsonData *StreamData::jsonData()
{
    return _jsonData;
}

String StreamData::dataType()
{
    return _dataTypeStr.c_str();
}

String StreamData::eventType()
{
    return _eventTypeStr.c_str();
}

void StreamData::empty()
{
    std::string().swap(_streamPath);
    std::string().swap(_path);
    std::string().swap(_data);
    std::string().swap(_dataTypeStr);
    std::string().swap(_eventTypeStr);
    std::vector<uint8_t>().swap(_blob);
}

QueryFilter::QueryFilter()
{
}

QueryFilter::~QueryFilter()
{
    clear();
}

QueryFilter &QueryFilter::clear()
{
    std::string().swap(_orderBy);
    std::string().swap(_limitToFirst);
    std::string().swap(_limitToLast);
    std::string().swap(_startAt);
    std::string().swap(_endAt);
    std::string().swap(_equalTo);
    return *this;
}

QueryFilter &QueryFilter::orderBy(const String &val)
{
    char *tmp = new char[100];
    memset(tmp, 0, 100);
    strcpy_P(tmp, ESP8266_FIREBASE_STR_3);
    strcat(tmp, val.c_str());
    strcat_P(tmp, ESP8266_FIREBASE_STR_3);
    _orderBy = tmp;
    delete[] tmp;
    return *this;
}
QueryFilter &QueryFilter::limitToFirst(int val)
{
    char *num = new char[20];
    _limitToFirst = itoa(val, num, 10);
    delete[] num;
    return *this;
}

QueryFilter &QueryFilter::limitToLast(int val)
{
    char *num = new char[20];
    _limitToLast = itoa(val, num, 10);
    delete[] num;
    return *this;
}

QueryFilter &QueryFilter::startAt(float val)
{
    char *num = new char[20];
    memset(num, 0, 20);
    dtostrf(val, 7, 6, num);
    _startAt = num;
    delete[] num;
    return *this;
}

QueryFilter &QueryFilter::endAt(float val)
{
    char *num = new char[20];
    memset(num, 0, 20);
    dtostrf(val, 7, 6, num);
    _endAt = num;
    delete[] num;
    return *this;
}

QueryFilter &QueryFilter::startAt(const String &val)
{
    char *tmp = new char[100];
    memset(tmp, 0, 100);
    strcpy_P(tmp, ESP8266_FIREBASE_STR_3);
    strcat(tmp, val.c_str());
    strcat_P(tmp, ESP8266_FIREBASE_STR_3);
    _startAt = tmp;
    delete[] tmp;
    return *this;
}

QueryFilter &QueryFilter::endAt(const String &val)
{
    char *tmp = new char[100];
    memset(tmp, 0, 100);
    strcpy_P(tmp, ESP8266_FIREBASE_STR_3);
    strcat(tmp, val.c_str());
    strcat_P(tmp, ESP8266_FIREBASE_STR_3);
    _endAt = tmp;
    delete[] tmp;
    return *this;
}

QueryFilter &QueryFilter::equalTo(int val)
{
    char *num = new char[20];
    _equalTo = itoa(val, num, 10);
    delete[] num;
    return *this;
}

QueryFilter &QueryFilter::equalTo(const String &val)
{
    char *tmp = new char[100];
    memset(tmp, 0, 100);
    strcpy_P(tmp, ESP8266_FIREBASE_STR_3);
    strcat(tmp, val.c_str());
    strcat_P(tmp, ESP8266_FIREBASE_STR_3);
    _equalTo = tmp;
    delete[] tmp;
    return *this;
}

QueueManager::QueueManager()
{
}
QueueManager::~QueueManager()
{
    clear();
}

void QueueManager::clear()
{
    for (uint8_t i = 0; i < _queueCollection.size(); i++)
    {
        QueueItem item = _queueCollection[i];

        std::string().swap(item.path);
        std::string().swap(item.filename);
        std::string().swap(item.payload);

        item.stringPtr = nullptr;
        item.intPtr = nullptr;
        item.floatPtr = nullptr;
        item.doublePtr = nullptr;
        item.boolPtr = nullptr;
        item.jsonPtr = nullptr;
        item.arrPtr = nullptr;
        item.blobPtr = nullptr;
        item.queryFilter.clear();
    }
}

bool QueueManager::add(QueueItem q)
{

    if (_queueCollection.size() < _maxQueue)
    {
        _queueCollection.push_back(q);
        return true;
    }

    return false;
}

void QueueManager::remove(uint8_t index)
{
    _queueCollection.erase(_queueCollection.begin() + index);
}

QueueInfo::QueueInfo()
{
}

QueueInfo::~QueueInfo()
{
    clear();
}

uint8_t QueueInfo::totalQueues()
{
    return _totalQueue;
}

uint32_t QueueInfo::currentQueueID()
{
    return _currentQueueID;
}

bool QueueInfo::isQueueFull()
{
    return _isQueueFull;
}

String QueueInfo::dataType()
{
    return _dataType.c_str();
}

String QueueInfo::firebaseMethod()
{
    return _method.c_str();
}

String QueueInfo::dataPath()
{
    return _path.c_str();
}

void QueueInfo::clear()
{
    std::string().swap(_dataType);
    std::string().swap(_method);
    std::string().swap(_path);
}

FCMObject::FCMObject() {}
FCMObject::~FCMObject()
{
    clear();
}

void FCMObject::begin(const String &serverKey)
{
    _server_key = serverKey.c_str();
}

void FCMObject::addDeviceToken(const String &deviceToken)
{
    _deviceToken.push_back(deviceToken.c_str());
}
void FCMObject::removeDeviceToken(uint16_t index)
{
    if (_deviceToken.size() > 0)
    {
        std::string().swap(_deviceToken[index]);
        _deviceToken.erase(_deviceToken.begin() + index);
    }
}
void FCMObject::clearDeviceToken()
{
    for (size_t i = 0; i < _deviceToken.size(); i++)
    {
        std::string().swap(_deviceToken[i]);
        _deviceToken.erase(_deviceToken.begin() + i);
    }
}

void FCMObject::setNotifyMessage(const String &title, const String &body)
{
    _notify_title = title.c_str();
    _notify_body = body.c_str();
    _notify_icon = "";
    _notify_click_action = "";
}

void FCMObject::setNotifyMessage(const String &title, const String &body, const String &icon)
{
    _notify_title = title.c_str();
    _notify_body = body.c_str();
    _notify_icon = icon.c_str();
    _notify_click_action = "";
}

void FCMObject::setNotifyMessage(const String &title, const String &body, const String &icon, const String &click_action)
{
    _notify_title = title.c_str();
    _notify_body = body.c_str();
    _notify_icon = icon.c_str();
    _notify_click_action = click_action.c_str();
}

void FCMObject::clearNotifyMessage()
{
    _notify_title = "";
    _notify_body = "";
    _notify_icon = "";
    _notify_click_action = "";
}

void FCMObject::setDataMessage(const String &jsonString)
{
    _data_msg = jsonString.c_str();
}

void FCMObject::setDataMessage(FirebaseJson &json)
{
    json._toStdString(_data_msg);
}

void FCMObject::clearDataMessage()
{
    _data_msg = "";
}

void FCMObject::setPriority(const String &priority)
{
    _priority = priority.c_str();
}

void FCMObject::setCollapseKey(const String &key)
{
    _collapse_key = key.c_str();
}

void FCMObject::setTimeToLive(uint32_t seconds)
{
    if (seconds <= 2419200)
        _ttl = seconds;
    else
        _ttl = -1;
}

void FCMObject::setTopic(const String &topic)
{
    char *t = new char[100];
    memset(t, 0, 100);
    strcpy_P(t, ESP8266_FIREBASE_STR_134);
    _topic = t;
    _topic += topic.c_str();
    delete[] t;
}

String FCMObject::getSendResult()
{
    return _sendResult.c_str();
}

bool FCMObject::fcm_connect(FirebaseHTTPClient &net)
{
    char *host = new char[100];
    memset(host, 0, 100);
    strcpy_P(host, ESP8266_FIREBASE_STR_120);

    int httpConnected = net.begin(host, _port);

    delete[] host;

    if (!httpConnected)
        return false;

    return true;
}
void FCMObject::fcm_buildHeader(char *header, size_t payloadSize)
{
    char *len = new char[20];
    memset(len, 0, 20);

    strcpy_P(header, ESP8266_FIREBASE_STR_24);
    strcat_P(header, ESP8266_FIREBASE_STR_6);
    strcat_P(header, ESP8266_FIREBASE_STR_121);
    strcat_P(header, ESP8266_FIREBASE_STR_30);

    strcat_P(header, ESP8266_FIREBASE_STR_31);
    strcat_P(header, ESP8266_FIREBASE_STR_120);
    strcat_P(header, ESP8266_FIREBASE_STR_21);

    strcat_P(header, ESP8266_FIREBASE_STR_131);
    strcat(header, _server_key.c_str());
    strcat_P(header, ESP8266_FIREBASE_STR_21);

    strcat_P(header, ESP8266_FIREBASE_STR_32);

    strcat_P(header, ESP8266_FIREBASE_STR_8);
    strcat_P(header, ESP8266_FIREBASE_STR_129);
    strcat_P(header, ESP8266_FIREBASE_STR_21);

    strcat_P(header, ESP8266_FIREBASE_STR_12);
    itoa(payloadSize, len, 10);
    strcat(header, len);
    strcat_P(header, ESP8266_FIREBASE_STR_21);
    strcat_P(header, ESP8266_FIREBASE_STR_34);
    strcat_P(header, ESP8266_FIREBASE_STR_21);

    delete[] len;
}

void FCMObject::fcm_buildPayload(char *msg, uint8_t messageType)
{
    bool noti = _notify_title.length() > 0 || _notify_body.length() > 0 || _notify_icon.length() > 0 || _notify_click_action.length() > 0;
    size_t c = 0;

    strcat_P(msg, ESP8266_FIREBASE_STR_169);

    if (noti)
        strcat_P(msg, ESP8266_FIREBASE_STR_122);

    if (noti && _notify_title.length() > 0)
    {
        strcat_P(msg, ESP8266_FIREBASE_STR_123);
        strcat(msg, _notify_title.c_str());
        strcat_P(msg, ESP8266_FIREBASE_STR_3);
        c++;
    }

    if (noti && _notify_body.length() > 0)
    {
        if (c > 0)
            strcat_P(msg, ESP8266_FIREBASE_STR_132);
        strcat_P(msg, ESP8266_FIREBASE_STR_124);
        strcat(msg, _notify_body.c_str());
        strcat_P(msg, ESP8266_FIREBASE_STR_3);
        c++;
    }

    if (noti && _notify_icon.length() > 0)
    {
        if (c > 0)
            strcat_P(msg, ESP8266_FIREBASE_STR_132);
        strcat_P(msg, ESP8266_FIREBASE_STR_125);
        strcat(msg, _notify_icon.c_str());
        strcat_P(msg, ESP8266_FIREBASE_STR_3);
        c++;
    }

    if (noti && _notify_click_action.length() > 0)
    {
        if (c > 0)
            strcat_P(msg, ESP8266_FIREBASE_STR_132);
        strcat_P(msg, ESP8266_FIREBASE_STR_126);
        strcat(msg, _notify_click_action.c_str());
        strcat_P(msg, ESP8266_FIREBASE_STR_3);
        c++;
    }

    if (noti)
        strcat_P(msg, ESP8266_FIREBASE_STR_127);

    c = 0;

    if (messageType == FirebaseESP8266::FCMMessageType::SINGLE)
    {
        if (noti)
            strcat_P(msg, ESP8266_FIREBASE_STR_132);

        strcat_P(msg, ESP8266_FIREBASE_STR_128);
        strcat(msg, _deviceToken[_index].c_str());
        strcat_P(msg, ESP8266_FIREBASE_STR_3);

        c++;
    }
    else if (messageType == FirebaseESP8266::FCMMessageType::MULTICAST)
    {
        if (noti)
            strcat_P(msg, ESP8266_FIREBASE_STR_132);

        strcat_P(msg, ESP8266_FIREBASE_STR_130);

        for (uint16_t i = 0; i < _deviceToken.size(); i++)
        {
            if (i > 0)
                strcat_P(msg, ESP8266_FIREBASE_STR_132);

            strcat_P(msg, ESP8266_FIREBASE_STR_3);
            strcat(msg, _deviceToken[i].c_str());
            strcat_P(msg, ESP8266_FIREBASE_STR_3);
        }

        strcat_P(msg, ESP8266_FIREBASE_STR_133);

        c++;
    }
    else if (messageType == FirebaseESP8266::FCMMessageType::TOPIC)
    {
        if (noti)
            strcat_P(msg, ESP8266_FIREBASE_STR_132);

        strcat_P(msg, ESP8266_FIREBASE_STR_128);
        strcat(msg, _topic.c_str());
        strcat_P(msg, ESP8266_FIREBASE_STR_3);

        c++;
    }

    if (_data_msg.length() > 0)
    {
        if (c > 0)
            strcat_P(msg, ESP8266_FIREBASE_STR_132);

        strcat_P(msg, ESP8266_FIREBASE_STR_135);
        strcat(msg, _data_msg.c_str());
        c++;
    }

    if (_priority.length() > 0)
    {
        if (c > 0)
            strcat_P(msg, ESP8266_FIREBASE_STR_132);

        strcat_P(msg, ESP8266_FIREBASE_STR_136);
        strcat(msg, _priority.c_str());
        strcat_P(msg, ESP8266_FIREBASE_STR_3);
        c++;
    }

    if (_ttl > -1)
    {
        if (c > 0)
            strcat_P(msg, ESP8266_FIREBASE_STR_132);
        char *ttl = new char[20];
        memset(ttl, 0, 20);
        strcat_P(msg, ESP8266_FIREBASE_STR_137);
        itoa(_ttl, ttl, 10);
        strcat(msg, ttl);
        delete[] ttl;
        c++;
    }

    if (_collapse_key.length() > 0)
    {
        if (c > 0)
            strcat_P(msg, ESP8266_FIREBASE_STR_132);
        strcat_P(msg, ESP8266_FIREBASE_STR_138);
        strcat(msg, _collapse_key.c_str());
        strcat_P(msg, ESP8266_FIREBASE_STR_3);
        c++;
    }

    strcat_P(msg, ESP8266_FIREBASE_STR_127);
}

bool FCMObject::getFCMServerResponse(FirebaseHTTPClient &net, int &httpcode)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        httpcode = HTTPC_ERROR_CONNECTION_LOST;
        return false;
    }

    if (!net._client)
    {
        httpcode = HTTPC_ERROR_CONNECTION_LOST;
        return false;
    }

    bool flag = false;
    char *lineBuf = new char[FIREBASE_RESPONSE_SIZE];
    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);

    uint16_t tempBufSize = FIREBASE_RESPONSE_SIZE;
    char *tmp = new char[tempBufSize];
    memset(tmp, 0, tempBufSize);

    char *fstr = new char[60];
    memset(fstr, 0, 60);

    _sendResult = "";

    char c;
    int p1;
    httpcode = -1000;

    size_t lfCount = 0;
    size_t charPos = 0;

    bool payloadBegin = false;

    unsigned long dataTime = millis();

    while (net._client->connected() && !net._client->available() && millis() - dataTime < 5000)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            httpcode = HTTPC_ERROR_CONNECTION_LOST;
            return false;
        }
        delay(1);
    }

    dataTime = millis();
    if (net._client->connected() && net._client->available())
    {

        while (net._client->available())
        {

            if (WiFi.status() != WL_CONNECTED)
            {
                httpcode = HTTPC_ERROR_CONNECTION_LOST;
                return false;
            }

            c = net._client->read();

            if (c != '\r' && c != '\n')
                strcat_c(lineBuf, c);

            charPos++;

            if (c == '\n')
            {
                dataTime = millis();

                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_5);

                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {

                    memset(tmp, 0, tempBufSize);
                    strncpy(tmp, lineBuf + p1 + 9, strlen(lineBuf) - p1 - 9);
                    httpcode = atoi(tmp);
                }

                if (httpcode == _HTTP_CODE_OK && lfCount > 0 && strlen(lineBuf) == 0)
                    payloadBegin = true;

                if (!payloadBegin)
                    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);

                lfCount++;
                charPos = 0;
            }

            if (millis() - dataTime > 5000)
            {
                httpcode = HTTPC_ERROR_READ_TIMEOUT;
                break;
            }
        }

        _sendResult = lineBuf;

        if (!httpcode)
            httpcode = HTTPC_ERROR_NO_HTTP_SERVER;

        goto EXIT_5;
    }

    delete[] lineBuf;
    delete[] tmp;
    delete[] fstr;

    if (httpcode == -1000)
    {
        httpcode = 0;
        flag = true;
    }

    return flag;

EXIT_5:

    delete[] lineBuf;
    delete[] tmp;
    delete[] fstr;

    if (httpcode == HTTPC_ERROR_READ_TIMEOUT)
        return false;

    return httpcode == _HTTP_CODE_OK;
}

bool FCMObject::fcm_send(FirebaseHTTPClient &net, int &httpcode, uint8_t messageType)
{

    char *msg = new char[400];
    char *header = new char[400];

    memset(msg, 0, 400);
    memset(header, 0, 400);

    fcm_buildPayload(msg, messageType);

    fcm_buildHeader(header, strlen(msg));

    if (WiFi.status() != WL_CONNECTED)
    {
        httpcode = HTTPC_ERROR_CONNECTION_LOST;
        return false;
    }

    uint8_t retryCount = 0;
    httpcode = net.sendRequest(header, "");
    while (httpcode != 0)
    {
        retryCount++;
        if (retryCount > 5)
            break;

        httpcode = net.sendRequest(header, "");
    }

    if (httpcode != 0)
        return false;

    httpcode = net.sendRequest("", msg);

    delete[] msg;
    delete[] header;

    if (httpcode != 0)
        return false;

    bool res = getFCMServerResponse(net, httpcode);

    return res;
}

void FCMObject::clear()
{
    std::string().swap(_notify_title);
    std::string().swap(_notify_body);
    std::string().swap(_notify_icon);
    std::string().swap(_notify_click_action);
    std::string().swap(_data_msg);
    std::string().swap(_priority);
    std::string().swap(_collapse_key);
    std::string().swap(_topic);
    std::string().swap(_server_key);
    std::string().swap(_sendResult);
    _ttl = -1;
    _index = 0;
    clearDeviceToken();
    std::vector<std::string>().swap(_deviceToken);
}

void FCMObject::strcat_c(char *str, char c)
{
    for (; *str; str++)
        ;
    *str++ = c;
    *str++ = 0;
}

int FCMObject::strpos(const char *haystack, const char *needle, int offset)
{
    size_t len = strlen(haystack);
    char *_haystack = new char[len];
    memset(_haystack, 0, len);
    strncpy(_haystack, haystack + offset, strlen(haystack) - offset);
    char *p = strstr(_haystack, needle);
    int r = -1;
    if (p)
        r = p - _haystack + offset;
    delete[] _haystack;
    return r;
}

FirebaseESP8266 Firebase = FirebaseESP8266();

#endif