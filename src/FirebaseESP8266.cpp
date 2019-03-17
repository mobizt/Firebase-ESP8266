/*
 * Google's Firebase Realtime Database Arduino Library for ESP8266, version 1.0.1
 * 
 * March 17, 2019
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

#ifndef FirebaseESP8266_CPP
#define FirebaseESP8266_CPP

#include "FirebaseESP8266.h"

struct FirebaseESP8266::FirebaseDataType
{
    static const uint8_t NULL_ = 1;
    static const uint8_t INTEGER = 2;
    static const uint8_t FLOAT = 3;
    static const uint8_t STRING = 4;
    static const uint8_t JSON = 5;
    static const uint8_t BLOB = 6;
    static const uint8_t FILE = 7;
};

struct FirebaseESP8266::FirebaseMethod
{
    static const uint8_t PUT = 0;
    static const uint8_t PUT_SILENT = 1;
    static const uint8_t POST = 2;
    static const uint8_t GET = 3;
    static const uint8_t STREAM = 4;
    static const uint8_t PATCH = 5;
    static const uint8_t PATCH_SILENT = 6;
    static const uint8_t DELETE = 7;
    static const uint8_t BACKUP = 8;
    static const uint8_t RESTORE = 9;
    static const uint8_t GET_RULES = 10;
    static const uint8_t SET_RULES = 11;
};

FirebaseESP8266::FirebaseESP8266() {}
FirebaseESP8266::~FirebaseESP8266() {}

void FirebaseESP8266::begin(const String &host, const String &auth)
{
    _host.clear();
    _auth.clear();
    _host.reserve(100);
    _auth.reserve(100);
    _host = host.c_str();
    _auth = auth.c_str();
    _port = FIEBASE_PORT;
}

void FirebaseESP8266::reconnectWiFi(bool reconnect)
{
    _reconnectWiFi = reconnect;
    WiFi.setAutoReconnect(reconnect);
}

bool FirebaseESP8266::getRules(FirebaseData &dataObj)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, ESP8266_FIREBASE_STR_103, FirebaseMethod::GET_RULES, FirebaseDataType::JSON, "");
}

bool FirebaseESP8266::setRules(FirebaseData &dataObj, const String &rules)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, ESP8266_FIREBASE_STR_103, FirebaseMethod::SET_RULES, FirebaseDataType::JSON, rules.c_str());
}

bool FirebaseESP8266::pushInt(FirebaseData &dataObj, const String &path, int intValue)
{
    size_t bufSize = 50;
    char *buf = new char[bufSize];
    memset(buf, 0, bufSize);
    itoa(intValue, buf, 10);
    dataObj.queryFilter.clear();
    bool res = sendRequest(dataObj, path.c_str(), FirebaseMethod::POST, FirebaseDataType::INTEGER, buf);
    delete[] buf;
    return res;
}

bool FirebaseESP8266::pushFloat(FirebaseData &dataObj, const String &path, float floatValue)
{
    size_t bufSize = 50;
    char *buf = new char[bufSize];
    memset(buf, 0, bufSize);
    dtostrf(floatValue, 7, 6, buf);
    dataObj.queryFilter.clear();
    bool res = sendRequest(dataObj, path.c_str(), FirebaseMethod::POST, FirebaseDataType::FLOAT, buf);
    delete[] buf;
    return res;
}

bool FirebaseESP8266::pushString(FirebaseData &dataObj, const String &path, const String &stringValue)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, path.c_str(), FirebaseMethod::POST, FirebaseDataType::STRING, stringValue.c_str());
}

bool FirebaseESP8266::pushJSON(FirebaseData &dataObj, const String &path, const String &jsonString)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, path.c_str(), FirebaseMethod::POST, FirebaseDataType::JSON, jsonString.c_str());
}

bool FirebaseESP8266::pushBlob(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size)
{
    dataObj.queryFilter.clear();
    size_t tmpSize = strlen(ESP8266_FIREBASE_STR_92) + 1;
    char *tmp = new char[tmpSize];
    memset(tmp, 0, tmpSize);

    std::string blobStr = "";
    strcpy_P(tmp, ESP8266_FIREBASE_STR_92);
    blobStr = tmp;
    blobStr += base64_encode_string((const unsigned char *)blob, size);
    memset(tmp, 0, tmpSize);
    strcpy_P(tmp, ESP8266_FIREBASE_STR_3);
    blobStr += tmp;
    bool res = sendRequest(dataObj, path.c_str(), FirebaseMethod::POST, FirebaseDataType::BLOB, blobStr.c_str());
    std::string().swap(blobStr);
    delete[] tmp;
    return res;
}

bool FirebaseESP8266::pushFile(FirebaseData &dataObj, const String &path, const String &fileName)
{
    dataObj.queryFilter.clear();
    dataObj._fileName = fileName.c_str();
    return sendRequest(dataObj, path.c_str(), FirebaseMethod::POST, FirebaseDataType::FILE, "");
}

bool FirebaseESP8266::setInt(FirebaseData &dataObj, const String &path, int intValue)
{
    size_t bufSize = 50;
    char *buf = new char[bufSize];
    memset(buf, 0, bufSize);
    itoa(intValue, buf, 10);
    dataObj.queryFilter.clear();
    bool res = sendRequest(dataObj, path.c_str(), FirebaseMethod::PUT, FirebaseDataType::INTEGER, buf);
    delete[] buf;
    return res;
}

bool FirebaseESP8266::setFloat(FirebaseData &dataObj, const String &path, float floatValue)
{
    size_t bufSize = 50;
    char *buf = new char[bufSize];
    memset(buf, 0, bufSize);
    dtostrf(floatValue, 7, 6, buf);
    dataObj.queryFilter.clear();
    bool res = sendRequest(dataObj, path.c_str(), FirebaseMethod::PUT, FirebaseDataType::FLOAT, buf);
    delete[] buf;
    return res;
}

bool FirebaseESP8266::setString(FirebaseData &dataObj, const String &path, const String &stringValue)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, path.c_str(), FirebaseMethod::PUT, FirebaseDataType::STRING, stringValue.c_str());
}

bool FirebaseESP8266::setJSON(FirebaseData &dataObj, const String &path, const String &jsonString)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, path.c_str(), FirebaseMethod::PUT, FirebaseDataType::JSON, jsonString.c_str());
}
bool FirebaseESP8266::setBlob(FirebaseData &dataObj, const String &path, uint8_t *blob, size_t size)
{
    dataObj.queryFilter.clear();
    size_t tmpSize = strlen(ESP8266_FIREBASE_STR_92) + 1;
    char *tmp = new char[tmpSize];
    memset(tmp, 0, tmpSize);

    std::string blobStr = "";
    strcpy(tmp, ESP8266_FIREBASE_STR_92);
    blobStr = tmp;
    blobStr += base64_encode_string((const unsigned char *)blob, size);
    memset(tmp, 0, tmpSize);
    strcpy(tmp, ESP8266_FIREBASE_STR_3);
    blobStr += tmp;
    bool res = sendRequest(dataObj, path.c_str(), FirebaseMethod::PUT_SILENT, FirebaseDataType::BLOB, blobStr.c_str());
    std::string().swap(blobStr);
    delete[] tmp;
    return res;
}

bool FirebaseESP8266::setFile(FirebaseData &dataObj, const String &path, const String &fileName)
{
    dataObj.queryFilter.clear();
    dataObj._fileName.clear();
    dataObj._fileName = fileName.c_str();
    return sendRequest(dataObj, path.c_str(), FirebaseMethod::PUT_SILENT, FirebaseDataType::FILE, "");
}

bool FirebaseESP8266::updateNode(FirebaseData &dataObj, const String path, const String jsonString)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, path.c_str(), FirebaseMethod::PATCH, FirebaseDataType::JSON, jsonString.c_str());
}

bool FirebaseESP8266::updateNodeSilent(FirebaseData &dataObj, const String &path, const String &jsonString)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, path.c_str(), FirebaseMethod::PATCH_SILENT, FirebaseDataType::JSON, jsonString.c_str());
}

bool FirebaseESP8266::getInt(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    return getFloat(dataObj, path);
}

bool FirebaseESP8266::getFloat(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    bool flag = sendRequest(dataObj, path.c_str(), FirebaseMethod::GET, FirebaseDataType::FLOAT, "");
    if (dataObj._dataType != FirebaseDataType::INTEGER && dataObj._dataType != FirebaseDataType::FLOAT)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getString(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    bool flag = sendRequest(dataObj, path.c_str(), FirebaseMethod::GET, FirebaseDataType::STRING, "");
    if (dataObj._dataType != FirebaseDataType::STRING)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getJSON(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    bool flag = sendRequest(dataObj, path.c_str(), FirebaseMethod::GET, FirebaseDataType::JSON, "");
    if (dataObj._dataType != FirebaseDataType::JSON)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getJSON(FirebaseData &dataObj, const String &path, QueryFilter &query)
{
    dataObj.queryFilter.clear();
    if (query._orderBy != "")
    {

        dataObj.queryFilter._orderBy = query._orderBy;
        dataObj.queryFilter._limitToFirst = query._limitToFirst;
        dataObj.queryFilter._limitToLast = query._limitToLast;
        dataObj.queryFilter._startAt = query._startAt;
        dataObj.queryFilter._endAt = query._endAt;
        dataObj.queryFilter._equalTo = query._equalTo;
    }

    bool flag = sendRequest(dataObj, path.c_str(), FirebaseMethod::GET, FirebaseDataType::JSON, "");
    if (dataObj._dataType != FirebaseDataType::JSON)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getBlob(FirebaseData &dataObj, const String &path)
{
    dataObj.queryFilter.clear();
    bool flag = sendRequest(dataObj, path.c_str(), FirebaseMethod::GET, FirebaseDataType::BLOB, "");
    if (dataObj._dataType != FirebaseDataType::BLOB)
        flag = false;
    return flag;
}

bool FirebaseESP8266::getFile(FirebaseData &dataObj, const String &nodePath, const String &fileName)
{
    dataObj.queryFilter.clear();
    dataObj._fileName.clear();
    dataObj._fileName = fileName.c_str();
    return sendRequest(dataObj, nodePath.c_str(), FirebaseMethod::GET, FirebaseDataType::FILE, "");
}

bool FirebaseESP8266::deleteNode(FirebaseData &dataObj, const String path)
{
    dataObj.queryFilter.clear();
    return sendRequest(dataObj, path.c_str(), FirebaseMethod::DELETE, FirebaseDataType::STRING, "");
}

bool FirebaseESP8266::beginStream(FirebaseData &dataObj, const String path)
{
    return firebaseConnectStream(dataObj, path.c_str());
}

bool FirebaseESP8266::readStream(FirebaseData &dataObj)
{
    if (dataObj._streamStop)
        return true;
    return getServerStreamResponse(dataObj);
}

bool FirebaseESP8266::endStream(FirebaseData &dataObj)
{
    bool flag = false;
    dataObj._streamPath.clear();
    forceEndHTTP(dataObj);
    flag = dataObj._http.http_connected();
    if (!flag)
    {
        dataObj._isStream = false;
        dataObj._streamStop = true;
    }
    return !flag;
}

int FirebaseESP8266::firebaseConnect(FirebaseData &dataObj, const std::string &path, const uint8_t method, uint8_t dataType, const std::string &payload)
{

    dataObj._firebaseError.clear();

    if (dataObj._pause)
        return 0;

    if (WiFi.status() != WL_CONNECTED)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
        return -1;
    }

    if (path.length() == 0 || _host.length() == 0 || _auth.length() == 0)
    {
        dataObj._httpCode = _HTTP_CODE_BAD_REQUEST;
        return _HTTP_CODE_BAD_REQUEST;
    }

    size_t bufSize = 32;
    char *buf = new char[bufSize];

    size_t tempSize = 200;
    char *temp = new char[tempSize];

    int len = 0;
    size_t toRead = 0;
    bool httpConnected = false;

    size_t payloadStrSize = payload.length() + 10;
    size_t headerSize = 320;

    char *payloadStr = new char[payloadStrSize];
    memset(payloadStr, 0, payloadStrSize);

    char *header = new char[headerSize];
    memset(header, 0, headerSize);

    int httpCode = -1;

    //init the firebase data
    resetFirebasedataFlag(dataObj);
    dataObj._path.clear();

    if (method == FirebaseMethod::STREAM)
    {
        //stream path change? reset the current (keep alive) connection
        if (path != dataObj._streamPath)
            dataObj._streamPathChanged = true;
        if (!dataObj._isStream || dataObj._streamPathChanged)
        {
            if (dataObj._http.http_connected())
                forceEndHTTP(dataObj);
        }

        dataObj._streamPath.clear();
        dataObj._streamPath = path;
    }
    else
    {
        //last requested method was stream?, reset the connection
        if (dataObj._isStream)
            forceEndHTTP(dataObj);
        if (method != FirebaseMethod::BACKUP && method != FirebaseMethod::RESTORE)
            dataObj._path = path;
        dataObj._isStreamTimeout = false;
    }

    httpConnected = dataObj._http.http_begin(_host, _port);

    if (!httpConnected)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_REFUSED;
        goto EXIT_1;
    }

    //Prepare for string and JSON payloads
    if (method != FirebaseMethod::GET && method != FirebaseMethod::STREAM &&
        method != FirebaseMethod::DELETE && method != FirebaseMethod::RESTORE)
    {
        memset(payloadStr, 0, payloadStrSize);
        if (dataType == FirebaseDataType::STRING)
            strcpy_P(payloadStr, ESP8266_FIREBASE_STR_3);
        strcat(payloadStr, payload.c_str());
        if (dataType == FirebaseDataType::STRING)
            strcat_P(payloadStr, ESP8266_FIREBASE_STR_3);
    }

    //Prepare request header
    if (method != FirebaseMethod::BACKUP && method != FirebaseMethod::RESTORE && dataType != FirebaseDataType::FILE)
    {
        buildFirebaseRequest(dataObj, _host.c_str(), method, path.c_str(), _auth.c_str(), strlen(payloadStr), header);
    }
    else
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

        if (method == FirebaseMethod::BACKUP || method == FirebaseMethod::RESTORE)
        {

            if (method == FirebaseMethod::BACKUP)
            {
                SD.remove(dataObj._backupFilename.c_str());
                file = SD.open(dataObj._backupFilename.c_str(), FILE_WRITE);
            }
            else
            {
                if (SD.exists(dataObj._backupFilename.c_str()))
                    file = SD.open(dataObj._backupFilename.c_str(), FILE_READ);
                else
                {
                    memset(temp, 0, tempSize);
                    strcat_P(temp, ESP8266_FIREBASE_STR_83);
                    dataObj._file_transfer_error = temp;
                }
            }

            if (!file)
            {
                memset(temp, 0, tempSize);
                strcat_P(temp, ESP8266_FIREBASE_STR_86);
                dataObj._file_transfer_error = temp;
                goto EXIT_1;
            }

            if (method == FirebaseMethod::RESTORE)
                len = file.size();
        }

        if (dataType == FirebaseDataType::FILE)
        {

            if (method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::POST)
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

                len = (4 * ceil(file.size() / 3.0)) + strlen(ESP8266_FIREBASE_STR_93) + 1;
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

                if (!SD.exists(folder))
                    createDirs(folder);

                SD.remove(dataObj._fileName.c_str());

                file = SD.open(dataObj._fileName.c_str(), FILE_WRITE);
                delete[] folder;
                delete[] fstr;
            }

            if (!file)
            {
                memset(temp, 0, tempSize);
                strcat_P(temp, ESP8266_FIREBASE_STR_86);
                dataObj._file_transfer_error = temp;
                goto EXIT_1;
            }
        }

        if (dataType == FirebaseDataType::FILE)
            buildFirebaseRequest(dataObj, _host.c_str(), method, dataObj._path.c_str(), _auth.c_str(), len, header);
        else
            buildFirebaseRequest(dataObj, _host.c_str(), method, dataObj._backupNodePath.c_str(), _auth.c_str(), len, header);
    }

    if (method == FirebaseMethod::PATCH_SILENT || (method == FirebaseMethod::PUT_SILENT && dataType == FirebaseDataType::BLOB))
        dataObj._isSilentResponse = true;

    if (dataType == FirebaseDataType::BLOB)
        std::vector<uint8_t>().swap(dataObj._blob);

    //Send request w/wo payload

    httpCode = dataObj._http.http_sendRequest(header, payloadStr);

    if (method == FirebaseMethod::RESTORE || (dataType == FirebaseDataType::FILE && (method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::POST)))
    {

        if (dataType == FirebaseDataType::FILE && (method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::POST))
        {
            memset(buf, 0, bufSize);
            strcpy_P(buf, ESP8266_FIREBASE_STR_93);

            httpCode = dataObj._http.http_sendRequest("", buf);

            send_base64_encode_file(dataObj._http.client, dataObj._fileName);

            memset(buf, 0, bufSize);
            buf[0] = '"';
            buf[1] = '\0';
            httpCode = dataObj._http.http_sendRequest("", buf);
        }
        else
        {

            while (len)
            {
                toRead = len;
                if (toRead > bufSize)
                    toRead = bufSize - 1;

                memset(buf, 0, bufSize);
                file.read((uint8_t *)buf, toRead);
                buf[toRead] = '\0';
                httpCode = dataObj._http.http_sendRequest("", buf);

                len -= toRead;

                if (len <= 0)
                    break;
            }
        }

        endFileTransfer(dataObj);
    }

    delete[] payloadStr;
    delete[] header;
    delete[] buf;
    delete[] temp;
    return httpCode;

EXIT_1:

    delete[] payloadStr;
    delete[] header;
    delete[] buf;
    delete[] temp;
    return -1;
}

bool FirebaseESP8266::sendRequest(FirebaseData &dataObj, const std::string &path, const uint8_t method, uint8_t dataType, const std::string &payload)
{

    bool flag = false;
    dataObj._firebaseError.clear();

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

    //Try to reconnect WiFi if lost connection
    if (_reconnectWiFi && WiFi.status() != WL_CONNECTED)
    {
        uint8_t tryCount = 0;
        WiFi.reconnect();
        while (WiFi.status() != WL_CONNECTED)
        {
            tryCount++;
            delay(50);
            if (tryCount > 60)
                break;
        }
    }

    //If WiFi is not connected, return false
    if (WiFi.status() != WL_CONNECTED)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
        return false;
    }

    //Get the current WiFi client from current firebase data
    //Check for connection status
    if (dataObj._http.http_connected())
        dataObj._httpConnected = true;
    else
        dataObj._httpConnected = false;

    if (dataObj._httpConnected)
    {
        if (method == FirebaseMethod::STREAM)
        {
            dataObj._streamMillis = millis();
            return false;
        }
        else
        {
            if (!dataObj._keepAlive)
            {
                dataObj._streamMillis = millis() + 50;
                dataObj._interruptRequest = true;
                delay(20);
                if (dataObj._http.http_connected())
                {

                    delay(20);
                    forceEndHTTP(dataObj);
                    if (dataObj._http.http_connected())
                        return false;
                }
                dataObj._httpConnected = false;
            }
        }
    }

    if (!dataObj._keepAlive && dataObj._httpConnected)
        return false;

    dataObj._httpConnected = true;
    dataObj._interruptRequest = false;
    dataObj._redirectURL.clear();
    dataObj._r_method = method;
    dataObj._r_dataType = dataType;

    int httpCode = firebaseConnect(dataObj, path, method, dataType, payload);

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

            dataObj._dataMillis = millis();
            flag = getServerStreamResponse(dataObj);
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
        delay(100);
        return false;
    }

    return flag;
}

bool FirebaseESP8266::getServerResponse(FirebaseData &dataObj)
{

    if (dataObj._pause)
        return true;

    if (WiFi.status() != WL_CONNECTED)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
        return false;
    }

    WiFiClientSecure client = dataObj._http.client;
    if (!dataObj._http.http_connected() || dataObj._interruptRequest)
        return cancelCurrentResponse(dataObj);
    if (!handleTCPNotConnected(dataObj) || !dataObj._httpConnected)
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

    if (!dataObj._isStream)
        while (client.connected() && !client.available() && millis() - dataTime < dataObj._http.tcpTimeout)
            delay(1);

    dataTime = millis();
    if (client.connected() && client.available())
    {

        while (client.available())
        {
            yield();

            if (dataObj._interruptRequest)
                return cancelCurrentResponse(dataObj);

            c = client.read();

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

                if (c < 0xff && c != '\r' && c != '\n' && charPos <= FIREBASE_RESPONSE_SIZE)
                    strcat_c(lineBuf, c);
            }
            else
            {
                if (c < 0xff && c != '"' && c != '\r' && c != '\n' && dataObj._httpCode != _HTTP_CODE_NO_CONTENT)
                    strcat_c(lineBuf, c);

                continue;
            }

            if (charPos >= strlen(ESP8266_FIREBASE_STR_92) - 1 && !hasBlob)
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

            if (c == '\n')
            {
                dataTime = millis();

                if (strlen(lineBuf) == 0 && lfCount > 0)
                    payLoadBegin = true;

                if (rulesBegin)
                {
                    if (jsonRes.length() + strlen(lineBuf) <= FIREBASE_RESPONSE_SIZE)
                        jsonRes += lineBuf;
                    else
                        dataObj._bufferOverflow = true;
                }

                if (strlen(lineBuf) > 0)
                {
                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_5);

                    p1 = strpos(lineBuf, fstr, 0);
                    if (p1 != -1)
                    {

                        memset(tmp, 0, tempBufSize);
                        strncpy(tmp, lineBuf + p1 + 9, strlen(lineBuf) - p1 - 9);
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
                            strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_95), strlen(lineBuf) - p1 - strlen(ESP8266_FIREBASE_STR_95));
                            dataObj._redirectURL = tmp;
                            int res = firebaseConnect(dataObj, dataObj._redirectURL.c_str(), dataObj._r_method, dataObj._r_dataType, "");

                            if (res == 0)
                            {
                                goto EXIT_4;
                            }

                            goto EXIT_3;
                        }
                    }

                    if (dataObj._httpCode == _HTTP_CODE_NO_CONTENT)
                        continue;

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
                            strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_102) + 1, strlen(lineBuf) - p1 - strlen(ESP8266_FIREBASE_STR_102));
                            dataObj._firebaseError.append(tmp, strlen(tmp) - 1);
                        }

                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_8);

                        p1 = strpos(lineBuf, fstr, 0);
                        if (p1 != -1)
                        {
                            memset(tmp, 0, tempBufSize);
                            strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_8), strlen(lineBuf) - p1 - strlen(ESP8266_FIREBASE_STR_8));

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
                            strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_10), strlen(lineBuf) - p1 - strlen(ESP8266_FIREBASE_STR_10));

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
                            strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_12), strlen(lineBuf) - p1 - strlen(ESP8266_FIREBASE_STR_12));

                            dataObj._contentLength = atoi(tmp);
                        }

                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_13);

                        p1 = strpos(lineBuf, fstr, 0);
                        if (p1 != -1)
                        {
                            memset(eventType, 0, eventTypeSize);
                            strncpy(eventType, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_13), strlen(lineBuf) - p1 - strlen(ESP8266_FIREBASE_STR_13));

                            hasEvent = true;
                            isStream = true;
                            dataObj._httpCode = _HTTP_CODE_OK;
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
                            strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_14), strlen(lineBuf) - p1 - strlen(ESP8266_FIREBASE_STR_14));
                            memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
                            strcpy(lineBuf, tmp);
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

            if (millis() - dataTime > dataObj._http.tcpTimeout)
            {
                //cancelCurrentResponse(dataObj);
                dataObj._httpCode = HTTPC_ERROR_READ_TIMEOUT;
                break;
            }
        }

        if (dataObj._httpCode == _HTTP_CODE_OK)
        {
            if (rulesBegin)
            {
                if (jsonRes.length() + strlen(lineBuf) <= FIREBASE_RESPONSE_SIZE && !dataObj._bufferOverflow)
                {
                    jsonRes += lineBuf;
                    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
                    strcpy(lineBuf, jsonRes.c_str());
                }
                else
                {
                    dataObj._bufferOverflow = true;
                    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
                    std::string().swap(jsonRes);
                }
            }

            if (dataObj._r_method == FirebaseMethod::SET_RULES)
            {
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_104);

                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {
                    memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
                }
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

                        //Parses json response for path
                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_17);
                        p1 = strpos(lineBuf, fstr, 0);
                        if (p1 != -1 && p1 < FIREBASE_RESPONSE_SIZE)
                        {
                            memset(fstr, 0, 60);
                            strcpy_P(fstr, ESP8266_FIREBASE_STR_3);
                            p2 = strpos(lineBuf, fstr, p1 + strlen(ESP8266_FIREBASE_STR_17));
                            if (p2 != -1)
                            {
                                dataObj._path.clear();
                                memset(tmp, 0, tempBufSize);
                                strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_17), p2 - p1 - strlen(ESP8266_FIREBASE_STR_17));
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
                            strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_18), strlen(lineBuf) - p1 - strlen(ESP8266_FIREBASE_STR_18) - 1);
                            dataObj._data = tmp;

                            setDataType(dataObj, dataObj._data.c_str());
                            bool samePath = dataObj._path == dataObj._path2;
                            memset(fstr, 0, 60);
                            strcpy_P(fstr, ESP8266_FIREBASE_STR_1);
                            bool rootPath = strcmp(dataObj._path.c_str(), fstr) == 0;
                            bool emptyPath = dataObj._path2.length() == 0;
                            bool sameData = dataObj._data == dataObj._data2;

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
                        //Firebase keep alive data
                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_11);
                        if (strcmp(eventType, fstr) == 0)
                        {
                            dataObj._isStreamTimeout = false;
                            dataObj._dataMillis = millis();

                            //if (dataObj._timeoutCallback)
                            //  dataObj._timeoutCallback(false);
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

                    //Push (POST) data?
                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_20);
                    p1 = strpos(lineBuf, fstr, 0);
                    if (p1 != -1)
                    {
                        memset(fstr, 0, 60);
                        strcpy_P(fstr, ESP8266_FIREBASE_STR_3);
                        p2 = strpos(lineBuf, fstr, p1 + strlen(ESP8266_FIREBASE_STR_20));
                        if (p2 != -1)
                        {
                            dataObj._pushName.clear();
                            memset(tmp, 0, tempBufSize);
                            strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_20), p2 - p1 - strlen(ESP8266_FIREBASE_STR_20));
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

        if (dataObj._dataType == FirebaseDataType::NULL_)
        {
            dataObj._pathNotExist = true;
        }
        else
        {
            dataObj._pathNotExist = false;
            bool _n1 = dataObj._dataType == FirebaseDataType::FLOAT || dataObj._dataType == FirebaseDataType::INTEGER;
            bool _n2 = dataObj._dataType2 == FirebaseDataType::FLOAT || dataObj._dataType2 == FirebaseDataType::INTEGER;

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

    if (dataObj._httpCode == -1000)
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
    return dataObj._httpCode == _HTTP_CODE_OK;

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

    if (WiFi.status() != WL_CONNECTED)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
        endFileTransfer(dataObj);
        return false;
    }

    WiFiClientSecure tcp = dataObj._http.client;
    if (!tcp)
    {
        endFileTransfer(dataObj);
        return false;
    }

    dataObj._httpCode = -1000;
    char c = 0;
    size_t bufSize = 128;
    char *buf = new char[bufSize + 1];

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
    size_t tmo = 30000 + dataObj._http.tcpTimeout;

    unsigned long dataTime = millis();

    while (tcp.connected() && !tcp.available() && millis() - dataTime < tmo)
        delay(1);

    dataTime = millis();
    if (tcp.connected() && tcp.available())
    {

        while (tcp.available() || count > 0)
        {
            if (dataObj._interruptRequest)
                return cancelCurrentResponse(dataObj);

            if (!beginPayload)
            {
                c = tcp.read();
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
                    if (toRead > bufSize)
                        toRead = bufSize;

                    memset(buf, 0, bufSize + 1);
                    cnt = 0;

                    while (cnt < toRead)
                    {

                        c = tcp.read();
                        if (c >= 0x20)
                        {
                            if (dataObj._fileName == "" && c < 0xff)
                            {
                                buf[cnt] = c;
                                cnt++;
                            }
                            else if (dataObj._fileName != "" && c != '"')
                            {
                                buf[cnt] = c;
                                cnt++;
                            }
                        }
                    }

                    if (cnt > 0)
                    {
                        count -= cnt;
                        dataTime = millis();
                        toRead = cnt;
                        buf[cnt] = '\0';
                        if (dataObj._fileName == "")
                            file.write((uint8_t *)buf, toRead);
                        else
                            base64_decode_file(file, buf, toRead);
                    }

                    if (count == 0)
                        break;
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
                    p2 = strpos(lineBuf, fstr, p1 + strlen(ESP8266_FIREBASE_STR_5));
                    if (p2 != -1)
                    {
                        memset(tmp, 0, tempBufSize);
                        strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_5), p2 - p1 - strlen(ESP8266_FIREBASE_STR_5));
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
                    strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_102) + 1, strlen(lineBuf) - p1 - strlen(ESP8266_FIREBASE_STR_102) - 2);
                    dataObj._firebaseError = tmp;
                }

                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_8);
                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {
                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_79);
                    p2 = strpos(lineBuf, fstr, p1 + strlen(ESP8266_FIREBASE_STR_8));
                    if (p2 != -1)
                        strncpy(contentType, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_8), p2 - p1 - strlen(ESP8266_FIREBASE_STR_8));
                }

                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_12);
                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {
                    memset(tmp, 0, tempBufSize);
                    strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_12), strlen(lineBuf) - p1 - strlen(ESP8266_FIREBASE_STR_12));
                    contentLength = atoi(tmp);
                    dataObj._backupzFileSize = contentLength;
                    count = contentLength;

                    if (dataObj._fileName != "")
                        count -= strlen(ESP8266_FIREBASE_STR_93) - 1;
                }
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_80);
                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {
                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_79);
                    p2 = strpos(lineBuf, fstr, p1 + strlen(ESP8266_FIREBASE_STR_80));
                    if (p2 != -1)
                        strncpy(contentDisposition, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_80), p2 - p1 - strlen(ESP8266_FIREBASE_STR_80));
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
                        for (size_t i = 0; i < strlen(ESP8266_FIREBASE_STR_93); i++)
                            tcp.read();
                    }
                }

                memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
            }

            if (millis() - dataTime > dataObj._http.tcpTimeout)
            {
                dataObj._httpCode = HTTPC_ERROR_READ_TIMEOUT;
                break;
            }
        }
    }

    delete[] buf;
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

    if (WiFi.status() != WL_CONNECTED)
    {
        dataObj._httpCode = HTTPC_ERROR_CONNECTION_LOST;
        return false;
    }

    WiFiClientSecure tcp = dataObj._http.client;
    if (!tcp)
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
    size_t tmo = dataObj._http.tcpTimeout + 5000;
    bool beginPayload = false;

    unsigned long dataTime = millis();

    if (!dataObj._isStream)
        while (tcp.connected() && !tcp.available() && millis() - dataTime < tmo)
            delay(1);

    dataTime = millis();

    if (tcp.connected() && tcp.available())
    {

        while (tcp.available())
        {
            if (dataObj._interruptRequest)
                return cancelCurrentResponse(dataObj);

            c = tcp.read();

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
                    strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_102) + 1, strlen(lineBuf) - p1 - strlen(ESP8266_FIREBASE_STR_102) - 2);
                    dataObj._firebaseError = tmp;
                }
                memset(fstr, 0, 60);
                strcpy_P(fstr, ESP8266_FIREBASE_STR_5);
                p1 = strpos(lineBuf, fstr, 0);
                if (p1 != -1)
                {
                    memset(fstr, 0, 60);
                    strcpy_P(fstr, ESP8266_FIREBASE_STR_6);
                    p2 = strpos(lineBuf, fstr, p1 + strlen(ESP8266_FIREBASE_STR_5));
                    if (p2 != -1)
                    {
                        memset(tmp, 0, tempBufSize);
                        strncpy(tmp, lineBuf + p1 + strlen(ESP8266_FIREBASE_STR_5), p2 - p1 - strlen(ESP8266_FIREBASE_STR_5));
                        _httpCode = atoi(tmp);
                    }
                }

                memset(lineBuf, 0, FIREBASE_RESPONSE_SIZE);
            }

            if (millis() - dataTime > dataObj._http.tcpTimeout)
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
    file.close();
    _sdOk = false;
    dataObj._file_transfering = false;
}

bool FirebaseESP8266::firebaseConnectStream(FirebaseData &dataObj, const std::string &path)
{

    if (dataObj._pause || dataObj._file_transfering)
        return true;

    dataObj._streamStop = false;

    if (dataObj._isStream && path == dataObj._streamPath)
        return true;

    if (path.length() == 0 || _host.length() == 0 || _auth.length() == 0)
    {
        dataObj._httpCode = _HTTP_CODE_BAD_REQUEST;
        return false;
    }

    if (millis() - dataObj._streamResetMillis > 50)
        delay(50);

    bool flag;
    flag = dataObj._streamPath.length() == 0;
    flag |= firebaseConnect(dataObj, path, FirebaseMethod::STREAM, FirebaseDataType::STRING, "") == 0;
    dataObj._dataMillis = millis();
    return flag;
}

bool FirebaseESP8266::getServerStreamResponse(FirebaseData &dataObj)
{

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
    if (ml - dataObj._streamResetMillis > 50)
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

        //Stream timeout
        if (dataObj._dataMillis > 0 && millis() - dataObj._dataMillis > KEEP_ALIVE_TIMEOUT)
        {
            dataObj._dataMillis = millis();
            dataObj._isStreamTimeout = true;
            path = dataObj._streamPath;

            if (_reconnectWiFi && WiFi.status() != WL_CONNECTED)
            {
                uint8_t tryCount = 0;
                WiFi.reconnect();
                while (WiFi.status() != WL_CONNECTED)
                {
                    tryCount++;
                    delay(50);
                    if (tryCount > 60)
                        break;
                }
            }

            firebaseConnectStream(dataObj, path.c_str());
            return getServerResponse(dataObj);
        }

        //last connection was not close
        if (dataObj._httpConnected)
            return true;

        dataObj._httpConnected = true;
        resetFirebasedataFlag(dataObj);

        WiFiClientSecure tcp = dataObj._http.client;

        if (tcp.connected() && !dataObj._isStream)
            forceEndHTTP(dataObj);
        if (!tcp.connected())
        {
            path = dataObj._streamPath;
            firebaseConnectStream(dataObj, path.c_str());
        }

        std::string().swap(path);

        return getServerResponse(dataObj);
    }

    return true;
}

void FirebaseESP8266::forceEndHTTP(FirebaseData &dataObj)
{
    WiFiClientSecure client = dataObj._http.client;

    if (client.available() > 0)
    {
        client.flush();
        delay(50);
    }
    client.stop();
    delay(50);
}

void FirebaseESP8266::buildFirebaseRequest(FirebaseData &dataObj, const char *host, uint8_t method, const char *path, const char *auth, size_t payloadLength, char *request)
{
    size_t numBufSize = 50;
    char *contentLength = new char[numBufSize];
    memset(contentLength, 0, numBufSize);

    char *num = new char[numBufSize];

    if (method == FirebaseMethod::STREAM)
    {
        strcpy_P(request, ESP8266_FIREBASE_STR_22);
        dataObj._isStream = true;
    }
    else
    {
        if (method == FirebaseMethod::PUT || method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::SET_RULES)
            strcpy_P(request, ESP8266_FIREBASE_STR_23);
        else if (method == FirebaseMethod::POST)
            strcpy_P(request, ESP8266_FIREBASE_STR_24);
        else if (method == FirebaseMethod::GET || method == FirebaseMethod::BACKUP || method == FirebaseMethod::GET_RULES)
            strcpy_P(request, ESP8266_FIREBASE_STR_25);
        else if (method == FirebaseMethod::PATCH || method == FirebaseMethod::PATCH_SILENT || method == FirebaseMethod::RESTORE)
            strcpy_P(request, ESP8266_FIREBASE_STR_26);
        else if (method == FirebaseMethod::DELETE)
            strcpy_P(request, ESP8266_FIREBASE_STR_27);
        strcat_P(request, ESP8266_FIREBASE_STR_6);
        dataObj._isStream = false;
    }

    strcat(request, path);

    if (method == FirebaseMethod::PATCH || method == FirebaseMethod::PATCH_SILENT)
        strcat_P(request, ESP8266_FIREBASE_STR_1);

    strcat_P(request, ESP8266_FIREBASE_STR_2);
    strcat(request, auth);

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
        strcat_P(request, ESP8266_FIREBASE_STR_28);
        size_t filenameSize = FIREBASE_RESPONSE_SIZE;
        char *filename = new char[filenameSize];
        memset(filename, 0, filenameSize);

        for (size_t i = 0; i < dataObj._backupNodePath.length(); i++)
        {
            if (dataObj._backupNodePath[i] == '/')
                strcat_P(filename, ESP8266_FIREBASE_STR_4);
            else
                strcat_c(filename, dataObj._backupNodePath[i]);
        }

        strcat(request, filename);
        delete[] filename;
    }

    if (method == FirebaseMethod::GET && dataObj._fileName.length() > 0)
    {
        strcat_P(request, ESP8266_FIREBASE_STR_28);
        strcat(request, dataObj._fileName.c_str());
    }

    if (method == FirebaseMethod::RESTORE || method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::PATCH_SILENT)
        strcat_P(request, ESP8266_FIREBASE_STR_29);

    strcat_P(request, ESP8266_FIREBASE_STR_30);
    strcat_P(request, ESP8266_FIREBASE_STR_31);
    strcat(request, host);
    strcat_P(request, ESP8266_FIREBASE_STR_21);
    strcat_P(request, ESP8266_FIREBASE_STR_32);
    strcat_P(request, ESP8266_FIREBASE_STR_33);

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

    if (method == FirebaseMethod::PUT || method == FirebaseMethod::PUT_SILENT || method == FirebaseMethod::POST || method == FirebaseMethod::PATCH || method == FirebaseMethod::PATCH_SILENT || method == FirebaseMethod::RESTORE || method == FirebaseMethod::SET_RULES)
    {
        strcat_P(request, ESP8266_FIREBASE_STR_12);
        itoa(payloadLength, contentLength, 10);
        strcat(request, contentLength);
    }

    strcat_P(request, ESP8266_FIREBASE_STR_21);
    strcat_P(request, ESP8266_FIREBASE_STR_21);

    delete[] contentLength;
    delete[] num;
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
        if (!typeSet && data[0] == '{')
        {
            typeSet = true;
            dataObj._dataType = FirebaseDataType::JSON;
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
            else
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

        if (!typeSet && data[0] == '"')
        {
            typeSet = true;
            dataObj._dataType = FirebaseDataType::STRING;
        }

        if (!typeSet && strpos(data, DEF_ESP8266_FIREBASE_STR_4, 0) != -1)
        {
            typeSet = true;
            dataObj._dataType = FirebaseDataType::FLOAT;
        }

        else if (!typeSet && strcmp(data, DEF_ESP8266_FIREBASE_STR_19) == 0)
        {
            typeSet = true;
            dataObj._dataType = FirebaseDataType::NULL_;
        }

        if (!typeSet)
        {
            typeSet = true;
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

void FirebaseESP8266::resetFirebasedataFlag(FirebaseData &dataObj)
{
    dataObj._bufferOverflow = false;
    dataObj._streamDataChanged = false;
    dataObj._streamPathChanged = false;
    dataObj._dataAvailable = false;
    dataObj._pushName.clear();
}
bool FirebaseESP8266::handleTCPNotConnected(FirebaseData &dataObj)
{
    if (!dataObj._http.http_connected())
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

void FirebaseESP8266::errorToString(int httpCode, std::string &buf)
{

    buf.clear();
    switch (httpCode)
    {
    case HTTPC_ERROR_CONNECTION_REFUSED:
        p_memCopy(buf, ESP8266_FIREBASE_STR_39);
        return;
    case HTTPC_ERROR_SEND_HEADER_FAILED:
        p_memCopy(buf, ESP8266_FIREBASE_STR_40);
        return;
    case HTTPC_ERROR_SEND_PAYLOAD_FAILED:
        p_memCopy(buf, ESP8266_FIREBASE_STR_41);
        return;
    case HTTPC_ERROR_NOT_CONNECTED:
        p_memCopy(buf, ESP8266_FIREBASE_STR_42);
        return;
    case HTTPC_ERROR_CONNECTION_LOST:
        p_memCopy(buf, ESP8266_FIREBASE_STR_43);
        return;
    case HTTPC_ERROR_NO_HTTP_SERVER:
        p_memCopy(buf, ESP8266_FIREBASE_STR_44);
        return;
    case _HTTP_CODE_BAD_REQUEST:
        p_memCopy(buf, ESP8266_FIREBASE_STR_45);
        return;
    case _HTTP_CODE_NON_AUTHORITATIVE_INFORMATION:
        p_memCopy(buf, ESP8266_FIREBASE_STR_46);
        return;
    case _HTTP_CODE_NO_CONTENT:
        p_memCopy(buf, ESP8266_FIREBASE_STR_47);
        return;
    case _HTTP_CODE_MOVED_PERMANENTLY:
        p_memCopy(buf, ESP8266_FIREBASE_STR_48);
        return;
    case _HTTP_CODE_USE_PROXY:
        p_memCopy(buf, ESP8266_FIREBASE_STR_49);
        return;
    case _HTTP_CODE_TEMPORARY_REDIRECT:
        p_memCopy(buf, ESP8266_FIREBASE_STR_50);
        return;
    case _HTTP_CODE_PERMANENT_REDIRECT:
        p_memCopy(buf, ESP8266_FIREBASE_STR_51);
        return;
    case _HTTP_CODE_UNAUTHORIZED:
        p_memCopy(buf, ESP8266_FIREBASE_STR_52);
        return;
    case _HTTP_CODE_FORBIDDEN:
        p_memCopy(buf, ESP8266_FIREBASE_STR_53);
        return;
    case _HTTP_CODE_NOT_FOUND:
        p_memCopy(buf, ESP8266_FIREBASE_STR_54);
        return;
    case _HTTP_CODE_METHOD_NOT_ALLOWED:
        p_memCopy(buf, ESP8266_FIREBASE_STR_55);
        return;
    case _HTTP_CODE_NOT_ACCEPTABLE:
        p_memCopy(buf, ESP8266_FIREBASE_STR_56);
        return;
    case _HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED:
        p_memCopy(buf, ESP8266_FIREBASE_STR_57);
        return;
    case _HTTP_CODE_REQUEST_TIMEOUT:
        p_memCopy(buf, ESP8266_FIREBASE_STR_58);
        return;
    case _HTTP_CODE_LENGTH_REQUIRED:
        p_memCopy(buf, ESP8266_FIREBASE_STR_59);
        return;
    case _HTTP_CODE_TOO_MANY_REQUESTS:
        p_memCopy(buf, ESP8266_FIREBASE_STR_60);
        return;
    case _HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE:
        p_memCopy(buf, ESP8266_FIREBASE_STR_61);
        return;
    case _HTTP_CODE_INTERNAL_SERVER_ERROR:
        p_memCopy(buf, ESP8266_FIREBASE_STR_62);
        return;
    case _HTTP_CODE_BAD_GATEWAY:
        p_memCopy(buf, ESP8266_FIREBASE_STR_63);
        return;
    case _HTTP_CODE_SERVICE_UNAVAILABLE:
        p_memCopy(buf, ESP8266_FIREBASE_STR_64);
        return;
    case _HTTP_CODE_GATEWAY_TIMEOUT:
        p_memCopy(buf, ESP8266_FIREBASE_STR_65);
        return;
    case _HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED:
        p_memCopy(buf, ESP8266_FIREBASE_STR_66);
        return;
    case _HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED:
        p_memCopy(buf, ESP8266_FIREBASE_STR_67);
        return;
    case HTTPC_ERROR_READ_TIMEOUT:
        p_memCopy(buf, ESP8266_FIREBASE_STR_69);
        return;
    case FIREBASE_ERROR_DATA_TYPE_MISMATCH:
        p_memCopy(buf, ESP8266_FIREBASE_STR_70);
        return;
    case FIREBASE_ERROR_PATH_NOT_EXIST:
        p_memCopy(buf, ESP8266_FIREBASE_STR_71);
        return;
    case HTTPC_ERROR_CONNECTION_INUSED:
        p_memCopy(buf, ESP8266_FIREBASE_STR_94);
        return;
    case FIREBASE_ERROR_BUFFER_OVERFLOW:
        p_memCopy(buf, ESP8266_FIREBASE_STR_68);
        return;
    default:
        return;
    }
}

bool FirebaseESP8266::backup(FirebaseData &dataObj, const String &nodePath, const String &fileName)
{
    dataObj._backupDir.clear();
    dataObj._backupNodePath = nodePath.c_str();
    dataObj._backupFilename = fileName.c_str();
    dataObj._fileName.clear();
    bool flag = sendRequest(dataObj, nodePath.c_str(), FirebaseMethod::BACKUP, FirebaseDataType::JSON, "");
    return flag;
}

bool FirebaseESP8266::restore(FirebaseData &dataObj, const String &nodePath, const String &fileName)
{
    dataObj._backupDir.clear();
    dataObj._backupNodePath = nodePath.c_str();
    dataObj._backupFilename = fileName.c_str();
    dataObj._fileName.clear();
    bool flag = sendRequest(dataObj, nodePath.c_str(), FirebaseMethod::RESTORE, FirebaseDataType::JSON, "");
    return flag;
}

void FirebaseESP8266::p_memCopy(std::string &buf, const char *p, bool empty)
{
    if (empty)
        buf.clear();
    size_t len = strlen(p) + 1;
    char *b = new char[len];
    memset(b, 0, len);
    strcpy(b, p);
    buf += b;
    delete[] b;
}

inline std::string FirebaseESP8266::trim(std::string &str)
{
    str.erase(0, str.find_first_not_of(' ')); //prefixing spaces
    str.erase(str.find_last_not_of(' ') + 1); //surfixing spaces
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    return str;
}

bool FirebaseESP8266::sdTest()
{
    File file;
    std::string filepath = "";
    p_memCopy(filepath, ESP8266_FIREBASE_STR_73, true);

    SD.begin();

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

void FirebaseESP8266::createDirs(std::string dirs)
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
                SD.mkdir(dir.substr(0, dir.length() - 1).c_str());
            count = 0;
        }
    }
    if (count > 0)
        SD.mkdir(dir.c_str());
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
        return std::string(); /* integer overflow */

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

void FirebaseESP8266::send_base64_encode_file(WiFiClientSecure &tcp, const std::string &filePath)
{

    File file = SD.open(filePath.c_str(), FILE_READ);

    if (!file)
        return;

    size_t chunkSize = 512;
    size_t fbufSize = 3;
    size_t byteAdd = 0;
    size_t byteSent = 0;

    unsigned char *buf = new unsigned char[chunkSize];
    memset(buf, 0, chunkSize);

    size_t len = file.size();
    size_t fbufIndex = 0;
    unsigned char *fbuf = new unsigned char[3];

    while (file.available())
    {
        memset(fbuf, 0, fbufSize);
        if (len - fbufIndex >= 3)
        {
            file.read(fbuf, 3);

            buf[byteAdd++] = ESP8266_FIREBASE_base64_table[fbuf[0] >> 2];
            buf[byteAdd++] = ESP8266_FIREBASE_base64_table[((fbuf[0] & 0x03) << 4) | (fbuf[1] >> 4)];
            buf[byteAdd++] = ESP8266_FIREBASE_base64_table[((fbuf[1] & 0x0f) << 2) | (fbuf[2] >> 6)];
            buf[byteAdd++] = ESP8266_FIREBASE_base64_table[fbuf[2] & 0x3f];

            if (len > chunkSize)
            {
                if (byteAdd >= chunkSize)
                {
                    byteSent += byteAdd;
                    tcp.write(buf, byteAdd);
                    memset(buf, 0, chunkSize);
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
                fbuf[0] = file.read();
            }
            else if (len - fbufIndex == 2)
            {
                fbuf[0] = file.read();
                fbuf[1] = file.read();
            }

            break;
        }
    }

    file.close();

    if (byteAdd > 0)
        tcp.write(buf, byteAdd);

    if (len - fbufIndex > 0)
    {

        memset(buf, 0, chunkSize);
        byteAdd = 0;

        buf[byteAdd++] = ESP8266_FIREBASE_base64_table[fbuf[0] >> 2];
        if (len - fbufIndex == 1)
        {
            buf[byteAdd++] = ESP8266_FIREBASE_base64_table[(fbuf[0] & 0x03) << 4];
            buf[byteAdd++] = '=';
        }
        else
        {
            buf[byteAdd++] = ESP8266_FIREBASE_base64_table[((fbuf[0] & 0x03) << 4) | (fbuf[1] >> 4)];
            buf[byteAdd++] = ESP8266_FIREBASE_base64_table[(fbuf[1] & 0x0f) << 2];
        }
        buf[byteAdd++] = '=';

        tcp.write(buf, byteAdd);
    }

    delete[] buf;
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
        return false;

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
                    return false;

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
        return false;

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
                    return false;
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
    char _haystack[strlen(haystack)];
    strncpy(_haystack, haystack + offset, strlen(haystack) - offset);
    char *p = strstr(_haystack, needle);
    if (p)
        return p - _haystack + offset;
    return -1;
}

int FirebaseESP8266::rstrpos(const char *haystack, const char *needle, int offset)
{
    char _haystack[strlen(haystack)];
    strncpy(_haystack, haystack + offset, strlen(haystack) - offset);
    char *p = rstrstr(_haystack, needle);
    if (p)
        return p - _haystack + offset;
    return -1;
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
}

WiFiClientSecure FirebaseData::getWiFiClient()
{
    return _http.client;
}

bool FirebaseData::pauseFirebase(bool pause)
{

    if (_http.http_connected() && pause != _pause)
    {
        if (_http.client.available() > 0)
        {
            _http.client.flush();
            delay(50);
        }
        _http.client.stop();
        delay(50);
        if (!_http.http_connected())
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

    if (_dataType == FirebaseESP8266::FirebaseDataType::JSON)
        return FPSTR(ESP8266_FIREBASE_STR_74);
    if (_dataType == FirebaseESP8266::FirebaseDataType::STRING)
        return FPSTR(ESP8266_FIREBASE_STR_75);
    if (_dataType == FirebaseESP8266::FirebaseDataType::FLOAT)
        return FPSTR(ESP8266_FIREBASE_STR_76);
    if (_dataType == FirebaseESP8266::FirebaseDataType::INTEGER)
        return FPSTR(ESP8266_FIREBASE_STR_77);
    if (_dataType == FirebaseESP8266::FirebaseDataType::BLOB)
        return FPSTR(ESP8266_FIREBASE_STR_91);
    if (_dataType == FirebaseESP8266::FirebaseDataType::NULL_)
        return FPSTR(ESP8266_FIREBASE_STR_78);
    return std::string().c_str();
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
    if (_data.length() > 0 && (_dataType == FirebaseESP8266::FirebaseDataType::INTEGER || _dataType == FirebaseESP8266::FirebaseDataType::FLOAT))
        return atoi(_data.c_str());
    else
        return 0;
}

float FirebaseData::floatData()
{
    if (_data.length() > 0 && (_dataType == FirebaseESP8266::FirebaseDataType::INTEGER || _dataType == FirebaseESP8266::FirebaseDataType::FLOAT))
        return atof(_data.c_str());
    else
        return 0.0;
}

String FirebaseData::stringData()
{
    if (_data.length() > 0 && _dataType == FirebaseESP8266::FirebaseDataType::STRING)
        return _data.substr(1, _data.length() - 2).c_str();
    else
        return std::string().c_str();
}

String FirebaseData::jsonData()
{
    if (_data.length() > 0 && _dataType == FirebaseESP8266::FirebaseDataType::JSON)
        return String(_data.c_str());
    else
        return String();
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
    return !_httpConnected && _dataAvailable && _streamDataChanged;
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

String FirebaseData::errorReason()
{
    std::string buf = "";

    if (_httpCode == _HTTP_CODE_OK)
    {
        if (_bufferOverflow)
            _httpCode = FIREBASE_ERROR_BUFFER_OVERFLOW;
        else if (_mismatchDataType)
            _httpCode = FIREBASE_ERROR_DATA_TYPE_MISMATCH;
        else if (_pathNotExist)
            _httpCode = FIREBASE_ERROR_PATH_NOT_EXIST;
    }

    Firebase.errorToString(_httpCode, buf);

    if (_firebaseError != "")
        buf += ", " + _firebaseError;

    return buf.c_str();
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
    if (_dataType == FirebaseESP8266::FirebaseDataType::FLOAT || _dataType == FirebaseESP8266::FirebaseDataType::INTEGER)
        return atoi(_data.c_str());
    else
        return 0;
}

float StreamData::floatData()
{
    if (_dataType == FirebaseESP8266::FirebaseDataType::FLOAT || _dataType == FirebaseESP8266::FirebaseDataType::INTEGER)
        return atof(_data.c_str());
    else
        return 0;
}

String StreamData::stringData()
{
    if (_dataType == FirebaseESP8266::FirebaseDataType::STRING)
        return _data.c_str();
    else
        return std::string().c_str();
}

String StreamData::jsonData()
{
    if (_dataType == FirebaseESP8266::FirebaseDataType::JSON)
        return _data.c_str();
    else
        return std::string().c_str();
}

String StreamData::dataType()
{
    return _dataTypeStr.c_str();
}

void StreamData::empty()
{
    std::string().swap(_streamPath);
    std::string().swap(_path);
    std::string().swap(_data);
    std::string().swap(_dataTypeStr);
}

QueryFilter::QueryFilter()
{
}

QueryFilter::~QueryFilter()
{
    clear();
}

void QueryFilter::clear()
{

    std::string().swap(_orderBy);
    std::string().swap(_limitToFirst);
    std::string().swap(_limitToLast);
    std::string().swap(_startAt);
    std::string().swap(_endAt);
    std::string().swap(_equalTo);
}

void QueryFilter::orderBy(const String &val)
{
    char *tmp = new char[100];
    memset(tmp, 0, 100);
    strcpy(tmp, ESP8266_FIREBASE_STR_3);
    strcat(tmp, val.c_str());
    strcat(tmp, ESP8266_FIREBASE_STR_3);
    _orderBy = tmp;
    delete[] tmp;
}
void QueryFilter::limitToFirst(int val)
{
    char *num = new char[20];
    _limitToFirst = itoa(val, num, 10);
    delete[] num;
}

void QueryFilter::limitToLast(int val)
{
    char *num = new char[20];
    _limitToLast = itoa(val, num, 10);
    delete[] num;
}

void QueryFilter::startAt(int val)
{
    char *num = new char[20];
    _startAt = itoa(val, num, 10);
    delete[] num;
}

void QueryFilter::endAt(int val)
{
    char *num = new char[20];
    _endAt = itoa(val, num, 10);
    delete[] num;
}

void QueryFilter::startAt(const String &val)
{
    char *tmp = new char[100];
    memset(tmp, 0, 100);
    strcpy(tmp, ESP8266_FIREBASE_STR_3);
    strcat(tmp, val.c_str());
    strcat(tmp, ESP8266_FIREBASE_STR_3);
    _startAt = tmp;
    delete[] tmp;
}

void QueryFilter::endAt(const String &val)
{
    char *tmp = new char[100];
    memset(tmp, 0, 100);
    strcpy(tmp, ESP8266_FIREBASE_STR_3);
    strcat(tmp, val.c_str());
    strcat(tmp, ESP8266_FIREBASE_STR_3);
    _endAt = tmp;
    delete[] tmp;
}

void QueryFilter::equalTo(int val)
{
    char *num = new char[20];
    _equalTo = itoa(val, num, 10);
    delete[] num;
}

void QueryFilter::equalTo(const String &val)
{
    char *tmp = new char[100];
    memset(tmp, 0, 100);
    strcpy(tmp, ESP8266_FIREBASE_STR_3);
    strcat(tmp, val.c_str());
    strcat(tmp, ESP8266_FIREBASE_STR_3);
    _equalTo = tmp;
    delete[] tmp;
}

FirebaseESP8266 Firebase = FirebaseESP8266();

#endif