/**
 * Google's Firebase Realtime Database Arduino Library for ESP8266, v3.2.3
 * 
 * June 10, 2021
 *
 *   Updates:
 * 
 * - Add payload and dataTypeEnum functions for the StreamData object.
 *
 *
 * 
 * This library provides ESP8266 to perform REST API by GET PUT, POST, PATCH, DELETE data from/to with Google's Firebase database using get, set, update
 * and delete calls. 
 * 
 * The library was tested and work well with ESP8266 based modules.
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

#ifndef FirebaseESP8266_CPP
#define FirebaseESP8266_CPP

#ifdef ESP8266

#include "FirebaseESP8266.h"

FirebaseESP8266::FirebaseESP8266()
{
}

FirebaseESP8266::~FirebaseESP8266()
{
    if (ut)
        delete ut;
}

void FirebaseESP8266::begin(FirebaseConfig *config, FirebaseAuth *auth)
{
    init(config, auth);

    if (_cfg->service_account.json.path.length() > 0)
    {
        if (!Signer.parseSAFile())
            _cfg->signer.tokens.status = token_status_uninitialized;
    }

    if (_cfg->signer.tokens.legacy_token.length() > 0)
        Signer.setTokenType(token_type_legacy_token);
    else if (Signer.tokenSigninDataReady())
    {
        if (_auth->token.uid.length() == 0)
            Signer.setTokenType(token_type_oauth2_access_token);
        else
            Signer.setTokenType(token_type_custom_token);
    }
    else if (Signer.userSigninDataReady())
        Signer.setTokenType(token_type_id_token);

    struct fb_esp_url_info_t uinfo;
    _cfg->_int.fb_auth_uri = _cfg->signer.tokens.token_type == token_type_legacy_token || _cfg->signer.tokens.token_type == token_type_id_token;

    if (_cfg->host.length() > 0)
        _cfg->database_url = _cfg->host;

    if (_cfg->database_url.length() > 0)
    {
        ut->getUrlInfo(_cfg->database_url.c_str(), uinfo);
        _cfg->database_url = uinfo.host;
    }
    if (strlen_P(_cfg->cert.data))
        _cfg->_int.fb_caCert = _cfg->cert.data;

    if (_cfg->cert.file.length() > 0)
    {
        if (_cfg->cert.file_storage == mem_storage_type_sd && !_cfg->_int.fb_sd_rdy)
            _cfg->_int.fb_sd_rdy = ut->sdTest(_cfg->_int.fb_file);
        else if (_cfg->cert.file_storage == mem_storage_type_flash && !_cfg->_int.fb_flash_rdy)
            ut->flashTest();
    }

    Signer.handleToken();
}

void FirebaseESP8266::begin(const String &databaseURL, const String &databaseSecret)
{
    _cfg_.database_url = databaseURL.c_str();
    _cfg_.signer.tokens.legacy_token = databaseSecret.c_str();
    begin(&_cfg_, &_auth_);
}

void FirebaseESP8266::begin(const String &databaseURL, const String &databaseSecret, const char *caCert, float GMTOffset)
{
    _cfg_.database_url = databaseURL.c_str();
    _cfg_.signer.tokens.legacy_token = databaseSecret.c_str();
    if (strlen_P(caCert))
    {
        float _gmtOffset = GMTOffset;
        _cfg_.cert.data = caCert;
        if (GMTOffset >= -12.0 && GMTOffset <= 14.0)
            _gmtOffset = GMTOffset;
        ut->setClock(_gmtOffset);
    }
    begin(&_cfg_, &_auth_);
}

void FirebaseESP8266::begin(const String &databaseURL, const String &databaseSecret, const String &caCertFile, uint8_t storageType, float GMTOffset)
{
    _cfg_.database_url = databaseURL.c_str();
    _cfg_.signer.tokens.legacy_token = databaseSecret.c_str();
    if (caCertFile.length() > 0)
    {
        float _gmtOffset = GMTOffset;
        _cfg_.cert.file = caCertFile.c_str();
        _cfg_.cert.file_storage = storageType;
        if (GMTOffset >= -12.0 && GMTOffset <= 14.0)
            _gmtOffset = GMTOffset;
        ut->setClock(_gmtOffset);
    }
    begin(&_cfg_, &_auth_);
}

bool FirebaseESP8266::signUp(FirebaseConfig *config, FirebaseAuth *auth, const char *email, const char *password)
{
    init(config, auth);
    return Signer.getIdToken(true, email, password);
}

bool FirebaseESP8266::sendEmailVerification(FirebaseConfig *config, const char *idToken)
{
    init(config, nullptr);
    return Signer.handleEmailSending(idToken, fb_esp_user_email_sending_type_verify);
}

bool FirebaseESP8266::sendResetPassword(FirebaseConfig *config, const char *email)
{
    init(config, nullptr);
    return Signer.handleEmailSending(email, fb_esp_user_email_sending_type_reset_psw);
}

void FirebaseESP8266::end(FirebaseData &fbdo)
{
    endStream(fbdo);
    removeStreamCallback(fbdo);
    fbdo.clear();
}

struct token_info_t FirebaseESP8266::authTokenInfo()
{
    return Signer.tokenInfo;
}

bool FirebaseESP8266::ready()
{
    return Signer.tokenReady();
}

bool FirebaseESP8266::authenticated()
{
    return Signer.authenticated;
}

void FirebaseESP8266::init(FirebaseConfig *config, FirebaseAuth *auth)
{
    _auth = auth;
    _cfg = config;

    if (_cfg == nullptr)
        _cfg = &_cfg_;

    if (_auth == nullptr)
        _auth = &_auth_;

    if (ut)
        delete ut;

    ut = new UtilsClass(config);

    RTDB.begin(ut);

    _cfg->_int.fb_reconnect_wifi = WiFi.getAutoReconnect();

    _cfg->signer.signup = false;
    _cfg_.signer.signup = false;
    Signer.begin(ut, _cfg, _auth);
    std::string().swap(_cfg_.signer.tokens.error.message);
}

void FirebaseESP8266::reconnectWiFi(bool reconnect)
{
    WiFi.setAutoReconnect(reconnect);
}

void FirebaseESP8266::setFloatDigits(uint8_t digits)
{
    if (digits < 7)
        _cfg->_int.fb_float_digits = digits;
}

void FirebaseESP8266::setDoubleDigits(uint8_t digits)
{
    if (digits < 9)
        _cfg->_int.fb_double_digits = digits;
}

void FirebaseESP8266::setReadTimeout(FirebaseData &fbdo, int millisec)
{
    RTDB.setReadTimeout(&fbdo, millisec);
}

void FirebaseESP8266::setwriteSizeLimit(FirebaseData &fbdo, const String &size)
{
    fbdo._ss.rtdb.write_limit = size.c_str();
}

bool FirebaseESP8266::getRules(FirebaseData &fbdo)
{
    return RTDB.getRules(&fbdo);
}

bool FirebaseESP8266::setRules(FirebaseData &fbdo, const String &rules)
{
    return RTDB.setRules(&fbdo, rules.c_str());
}

bool FirebaseESP8266::setQueryIndex(FirebaseData &fbdo, const String &path, const String &node, const String &databaseSecret)
{
    return RTDB.setQueryIndex(&fbdo, path.c_str(), node.c_str(), databaseSecret.c_str());
}

bool FirebaseESP8266::removeQueryIndex(FirebaseData &fbdo, const String &path, const String &databaseSecret)
{
    return RTDB.removeQueryIndex(&fbdo, path.c_str(), databaseSecret.c_str());
}

bool FirebaseESP8266::setReadWriteRules(FirebaseData &fbdo, const String &path, const String &var, const String &readVal, const String &writeVal, const String &databaseSecret)
{
    return RTDB.setReadWriteRules(&fbdo, path.c_str(), var.c_str(), readVal.c_str(), writeVal.c_str(), databaseSecret.c_str());
}

bool FirebaseESP8266::pathExist(FirebaseData &fbdo, const String &path)
{
    return RTDB.pathExisted(&fbdo, path.c_str());
}

bool FirebaseESP8266::pathExisted(FirebaseData &fbdo, const String &path)
{
    return RTDB.pathExisted(&fbdo, path.c_str());
}

String FirebaseESP8266::getETag(FirebaseData &fbdo, const String &path)
{
    return RTDB.getETag(&fbdo, path.c_str());
}

bool FirebaseESP8266::getShallowData(FirebaseData &fbdo, const String &path)
{
    return RTDB.getShallowData(&fbdo, path.c_str());
}

void FirebaseESP8266::enableClassicRequest(FirebaseData &fbdo, bool flag)
{
    fbdo._ss.classic_request = flag;
}

bool FirebaseESP8266::setPriority(FirebaseData &fbdo, const String &path, float priority)
{
    return RTDB.setPriority(&fbdo, path.c_str(), priority);
}

bool FirebaseESP8266::setPriorityAsync(FirebaseData &fbdo, const String &path, float priority)
{
    return RTDB.setPriorityAsync(&fbdo, path.c_str(), priority);
}

bool FirebaseESP8266::getPriority(FirebaseData &fbdo, const String &path)
{
    return RTDB.getPriority(&fbdo, path.c_str());
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, int intValue)
{
    return RTDB.pushInt(&fbdo, path.c_str(), intValue);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, int intValue)
{
    return RTDB.pushIntAsync(&fbdo, path.c_str(), intValue);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, int intValue, float priority)
{
    return RTDB.pushInt(&fbdo, path.c_str(), intValue, priority);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, int intValue, float priority)
{
    return RTDB.pushIntAsync(&fbdo, path.c_str(), intValue, priority);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, float floatValue)
{
    return RTDB.pushFloat(&fbdo, path.c_str(), floatValue);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, float floatValue)
{
    return RTDB.pushFloatAsync(&fbdo, path.c_str(), floatValue);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, float floatValue, float priority)
{
    return RTDB.pushFloat(&fbdo, path.c_str(), floatValue, priority);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, float floatValue, float priority)
{
    return RTDB.pushFloatAsync(&fbdo, path.c_str(), floatValue, priority);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, double doubleValue)
{
    return RTDB.pushDouble(&fbdo, path.c_str(), doubleValue);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, double doubleValue)
{
    return RTDB.pushDoubleAsync(&fbdo, path.c_str(), doubleValue);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, double doubleValue, float priority)
{
    return RTDB.pushDouble(&fbdo, path.c_str(), doubleValue, priority);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, double doubleValue, float priority)
{
    return RTDB.pushDoubleAsync(&fbdo, path.c_str(), doubleValue, priority);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, bool boolValue)
{
    return RTDB.pushBool(&fbdo, path.c_str(), boolValue);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, bool boolValue)
{
    return RTDB.pushBoolAsync(&fbdo, path.c_str(), boolValue);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, bool boolValue, float priority)
{
    return RTDB.pushBool(&fbdo, path.c_str(), boolValue, priority);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, bool boolValue, float priority)
{
    return RTDB.pushBoolAsync(&fbdo, path.c_str(), boolValue, priority);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, const char *stringValue)
{
    return RTDB.pushString(&fbdo, path.c_str(), stringValue);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, const char *stringValue)
{
    return RTDB.pushStringAsync(&fbdo, path.c_str(), stringValue);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, const String &stringValue)
{
    return RTDB.pushString(&fbdo, path.c_str(), stringValue);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, const String &stringValue)
{
    return RTDB.pushStringAsync(&fbdo, path.c_str(), stringValue);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, const char *stringValue, float priority)
{
    return RTDB.pushString(&fbdo, path.c_str(), stringValue, priority);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, const char *stringValue, float priority)
{
    return RTDB.pushStringAsync(&fbdo, path.c_str(), stringValue, priority);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, const String &stringValue, float priority)
{
    return RTDB.pushString(&fbdo, path.c_str(), stringValue, priority);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, const String &stringValue, float priority)
{
    return RTDB.pushStringAsync(&fbdo, path.c_str(), stringValue, priority);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, FirebaseJson &json)
{
    return RTDB.pushJSON(&fbdo, path.c_str(), &json);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json)
{
    return RTDB.pushJSONAsync(&fbdo, path.c_str(), &json);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority)
{
    return RTDB.pushJSON(&fbdo, path.c_str(), &json, priority);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority)
{
    return RTDB.pushJSONAsync(&fbdo, path.c_str(), &json, priority);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr)
{
    return RTDB.pushArray(&fbdo, path.c_str(), &arr);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr)
{
    return RTDB.pushArrayAsync(&fbdo, path.c_str(), &arr);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, float priority)
{
    return RTDB.pushArray(&fbdo, path.c_str(), &arr, priority);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, float priority)
{
    return RTDB.pushArrayAsync(&fbdo, path.c_str(), &arr, priority);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size)
{
    return RTDB.pushBlob(&fbdo, path.c_str(), blob, size);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size)
{
    return RTDB.pushBlobAsync(&fbdo, path.c_str(), blob, size);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, float priority)
{
    return RTDB.pushBlob(&fbdo, path.c_str(), blob, size, priority);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, float priority)
{
    return RTDB.pushBlobAsync(&fbdo, path.c_str(), blob, size, priority);
}

bool FirebaseESP8266::push(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName)
{
    return RTDB.pushFile(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str());
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName)
{
    return RTDB.pushFileAsync(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str());
}

bool FirebaseESP8266::push(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, float priority)
{
    return RTDB.pushFile(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), priority);
}

bool FirebaseESP8266::pushAsync(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, float priority)
{
    return RTDB.pushFileAsync(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), priority);
}

template <typename T>
bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, T value)
{
    if (std::is_same<T, int>::value)
        return pushInt(fbdo, path, value);
    else if (std::is_same<T, double>::value)
        return pushDouble(fbdo, path, value);
    else if (std::is_same<T, bool>::value)
        return pushBool(fbdo, path, value);
    else if (std::is_same<T, const char *>::value)
        return pushString(fbdo, path, value);
    else if (std::is_same<T, const String &>::value)
        return pushString(fbdo, path, value);
    else if (std::is_same<T, FirebaseJson &>::value)
        return pushJson(fbdo, path, value);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return pushArray(fbdo, path, value);
}

template <typename T>
bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, T value, size_t size)
{
    if (std::is_same<T, uint8_t *>::value)
        return RTDB.pushBlob(&fbdo, path.c_str(), value, size);
}

template <typename T>
bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, T value, float priority)
{
    if (std::is_same<T, int>::value)
        return pushInt(fbdo, path, value, priority);
    else if (std::is_same<T, double>::value)
        return pushDouble(fbdo, path, value, priority);
    else if (std::is_same<T, bool>::value)
        return pushBool(fbdo, path, value, priority);
    else if (std::is_same<T, const char *>::value)
        return pushString(fbdo, path, value, priority);
    else if (std::is_same<T, const String &>::value)
        return pushString(fbdo, path, value, priority);
    else if (std::is_same<T, FirebaseJson &>::value)
        return pushJson(fbdo, path, value, priority);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return pushArray(fbdo, path, value, priority);
}

template <typename T>
bool FirebaseESP8266::push(FirebaseData &fbdo, const String &path, T value, size_t size, float priority)
{
    if (std::is_same<T, uint8_t *>::value)
        return RTDB.pushBlob(&fbdo, path.c_str(), value, size, priority);
}

bool FirebaseESP8266::pushInt(FirebaseData &fbdo, const String &path, int intValue)
{
    return RTDB.pushInt(&fbdo, path.c_str(), intValue);
}

bool FirebaseESP8266::pushIntAsync(FirebaseData &fbdo, const String &path, int intValue)
{
    return RTDB.pushIntAsync(&fbdo, path.c_str(), intValue);
}

bool FirebaseESP8266::pushInt(FirebaseData &fbdo, const String &path, int intValue, float priority)
{
    return RTDB.pushInt(&fbdo, path.c_str(), intValue, priority);
}

bool FirebaseESP8266::pushIntAsync(FirebaseData &fbdo, const String &path, int intValue, float priority)
{
    return RTDB.pushIntAsync(&fbdo, path.c_str(), intValue, priority);
}

bool FirebaseESP8266::pushFloat(FirebaseData &fbdo, const String &path, float floatValue)
{
    return RTDB.pushFloat(&fbdo, path.c_str(), floatValue);
}

bool FirebaseESP8266::pushFloatAsync(FirebaseData &fbdo, const String &path, float floatValue)
{
    return RTDB.pushFloatAsync(&fbdo, path.c_str(), floatValue);
}

bool FirebaseESP8266::pushFloat(FirebaseData &fbdo, const String &path, float floatValue, float priority)
{
    return RTDB.pushFloat(&fbdo, path.c_str(), floatValue, priority);
}

bool FirebaseESP8266::pushFloatAsync(FirebaseData &fbdo, const String &path, float floatValue, float priority)
{
    return RTDB.pushFloat(&fbdo, path.c_str(), floatValue, priority);
}

bool FirebaseESP8266::pushDouble(FirebaseData &fbdo, const String &path, double doubleValue)
{
    return RTDB.pushDouble(&fbdo, path.c_str(), doubleValue);
}

bool FirebaseESP8266::pushDoubleAsync(FirebaseData &fbdo, const String &path, double doubleValue)
{
    return RTDB.pushDoubleAsync(&fbdo, path.c_str(), doubleValue);
}

bool FirebaseESP8266::pushDouble(FirebaseData &fbdo, const String &path, double doubleValue, float priority)
{
    return RTDB.pushDouble(&fbdo, path.c_str(), doubleValue, priority);
}

bool FirebaseESP8266::pushDoubleAsync(FirebaseData &fbdo, const String &path, double doubleValue, float priority)
{
    return RTDB.pushDoubleAsync(&fbdo, path.c_str(), doubleValue, priority);
}

bool FirebaseESP8266::pushBool(FirebaseData &fbdo, const String &path, bool boolValue)
{
    return RTDB.pushBool(&fbdo, path.c_str(), boolValue);
}

bool FirebaseESP8266::pushBoolAsync(FirebaseData &fbdo, const String &path, bool boolValue)
{
    return RTDB.pushBoolAsync(&fbdo, path.c_str(), boolValue);
}

bool FirebaseESP8266::pushBool(FirebaseData &fbdo, const String &path, bool boolValue, float priority)
{
    return RTDB.pushBool(&fbdo, path.c_str(), boolValue, priority);
}

bool FirebaseESP8266::pushBoolAsync(FirebaseData &fbdo, const String &path, bool boolValue, float priority)
{
    return RTDB.pushBoolAsync(&fbdo, path.c_str(), boolValue, priority);
}

bool FirebaseESP8266::pushString(FirebaseData &fbdo, const String &path, const String &stringValue)
{
    return RTDB.pushString(&fbdo, path.c_str(), stringValue);
}

bool FirebaseESP8266::pushStringAsync(FirebaseData &fbdo, const String &path, const String &stringValue)
{
    return RTDB.pushStringAsync(&fbdo, path.c_str(), stringValue);
}

bool FirebaseESP8266::pushString(FirebaseData &fbdo, const String &path, const String &stringValue, float priority)
{
    return RTDB.pushString(&fbdo, path.c_str(), stringValue, priority);
}

bool FirebaseESP8266::pushStringAsync(FirebaseData &fbdo, const String &path, const String &stringValue, float priority)
{
    return RTDB.pushStringAsync(&fbdo, path.c_str(), stringValue, priority);
}

bool FirebaseESP8266::pushJSON(FirebaseData &fbdo, const String &path, FirebaseJson &json)
{
    return RTDB.pushJSON(&fbdo, path.c_str(), &json);
}

bool FirebaseESP8266::pushJSONAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json)
{
    return RTDB.pushJSONAsync(&fbdo, path.c_str(), &json);
}

bool FirebaseESP8266::pushJSON(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority)
{
    return RTDB.pushJSON(&fbdo, path.c_str(), &json, priority);
}

bool FirebaseESP8266::pushJSONAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority)
{
    return RTDB.pushJSONAsync(&fbdo, path.c_str(), &json, priority);
}

bool FirebaseESP8266::pushArray(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr)
{
    return RTDB.pushArray(&fbdo, path.c_str(), &arr);
}

bool FirebaseESP8266::pushArrayAsync(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr)
{
    return RTDB.pushArrayAsync(&fbdo, path.c_str(), &arr);
}

bool FirebaseESP8266::pushArray(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, float priority)
{
    return RTDB.pushArray(&fbdo, path.c_str(), &arr, priority);
}

bool FirebaseESP8266::pushArrayAsync(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, float priority)
{
    return RTDB.pushArrayAsync(&fbdo, path.c_str(), &arr, priority);
}

bool FirebaseESP8266::pushBlob(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size)
{
    return RTDB.pushBlob(&fbdo, path.c_str(), blob, size);
}

bool FirebaseESP8266::pushBlobAsync(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size)
{
    return RTDB.pushBlobAsync(&fbdo, path.c_str(), blob, size);
}

bool FirebaseESP8266::pushBlob(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, float priority)
{
    return RTDB.pushBlob(&fbdo, path.c_str(), blob, size, priority);
}

bool FirebaseESP8266::pushBlobAsync(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, float priority)
{
    return RTDB.pushBlobAsync(&fbdo, path.c_str(), blob, size, priority);
}

bool FirebaseESP8266::pushFile(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName)
{
    return RTDB.pushFile(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str());
}

bool FirebaseESP8266::pushFileAsync(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName)
{
    return RTDB.pushFileAsync(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str());
}

bool FirebaseESP8266::pushFile(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, float priority)
{
    return RTDB.pushFile(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), priority);
}

bool FirebaseESP8266::pushFileAsync(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, float priority)
{
    return RTDB.pushFileAsync(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), priority);
}

bool FirebaseESP8266::pushTimestamp(FirebaseData &fbdo, const String &path)
{
    return RTDB.pushTimestamp(&fbdo, path.c_str());
}

bool FirebaseESP8266::pushTimestampAsync(FirebaseData &fbdo, const String &path)
{
    return RTDB.pushTimestampAsync(&fbdo, path.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, int intValue)
{
    return RTDB.setInt(&fbdo, path.c_str(), intValue);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, int intValue)
{
    return RTDB.setIntAsync(&fbdo, path.c_str(), intValue);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, int intValue, float priority)
{
    return RTDB.setInt(&fbdo, path.c_str(), intValue, priority);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, int intValue, float priority)
{
    return RTDB.setIntAsync(&fbdo, path.c_str(), intValue, priority);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, int intValue, const String &ETag)
{
    return RTDB.setInt(&fbdo, path.c_str(), intValue, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, int intValue, const String &ETag)
{
    return RTDB.setIntAsync(&fbdo, path.c_str(), intValue, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, int intValue, float priority, const String &ETag)
{
    return RTDB.setInt(&fbdo, path.c_str(), intValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, int intValue, float priority, const String &ETag)
{
    return RTDB.setIntAsync(&fbdo, path.c_str(), intValue, priority, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, float floatValue)
{
    return RTDB.setFloat(&fbdo, path.c_str(), floatValue);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, float floatValue)
{
    return RTDB.setFloatAsync(&fbdo, path.c_str(), floatValue);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, float floatValue, float priority)
{
    return RTDB.setFloat(&fbdo, path.c_str(), floatValue, priority);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, float floatValue, float priority)
{
    return RTDB.setFloatAsync(&fbdo, path.c_str(), floatValue, priority);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, float floatValue, const String &ETag)
{
    return RTDB.setFloat(&fbdo, path.c_str(), floatValue, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, float floatValue, const String &ETag)
{
    return RTDB.setFloatAsync(&fbdo, path.c_str(), floatValue, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, float floatValue, float priority, const String &ETag)
{
    return RTDB.setFloat(&fbdo, path.c_str(), floatValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, float floatValue, float priority, const String &ETag)
{
    return RTDB.setFloatAsync(&fbdo, path.c_str(), floatValue, priority, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, double doubleValue)
{
    return RTDB.setDouble(&fbdo, path.c_str(), doubleValue);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, double doubleValue)
{
    return RTDB.setDoubleAsync(&fbdo, path.c_str(), doubleValue);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, double doubleValue, float priority)
{
    return RTDB.setDouble(&fbdo, path.c_str(), doubleValue, priority);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, double doubleValue, float priority)
{
    return RTDB.setDoubleAsync(&fbdo, path.c_str(), doubleValue, priority);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, double doubleValue, const String &ETag)
{
    return RTDB.setDouble(&fbdo, path.c_str(), doubleValue, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, double doubleValue, const String &ETag)
{
    return RTDB.setDoubleAsync(&fbdo, path.c_str(), doubleValue, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, double doubleValue, float priority, const String &ETag)
{
    return RTDB.setDouble(&fbdo, path.c_str(), doubleValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, double doubleValue, float priority, const String &ETag)
{
    return RTDB.setDoubleAsync(&fbdo, path.c_str(), doubleValue, priority, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, bool boolValue)
{
    return RTDB.setBool(&fbdo, path.c_str(), boolValue);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, bool boolValue)
{
    return RTDB.setBoolAsync(&fbdo, path.c_str(), boolValue);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, bool boolValue, float priority)
{
    return RTDB.setBool(&fbdo, path.c_str(), boolValue, priority);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, bool boolValue, float priority)
{
    return RTDB.setBoolAsync(&fbdo, path.c_str(), boolValue, priority);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, bool boolValue, const String &ETag)
{
    return RTDB.setBool(&fbdo, path.c_str(), boolValue, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, bool boolValue, const String &ETag)
{
    return RTDB.setBoolAsync(&fbdo, path.c_str(), boolValue, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, bool boolValue, float priority, const String &ETag)
{
    return RTDB.setBool(&fbdo, path.c_str(), boolValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, bool boolValue, float priority, const String &ETag)
{
    return RTDB.setBoolAsync(&fbdo, path.c_str(), boolValue, priority, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, const char *stringValue)
{
    return RTDB.setString(&fbdo, path.c_str(), stringValue);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, const char *stringValue)
{
    return RTDB.setStringAsync(&fbdo, path.c_str(), stringValue);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, const String &stringValue)
{
    return RTDB.setString(&fbdo, path.c_str(), stringValue);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, const String &stringValue)
{
    return RTDB.setStringAsync(&fbdo, path.c_str(), stringValue);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, const char *stringValue, float priority)
{
    return RTDB.setString(&fbdo, path.c_str(), stringValue, priority);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, const char *stringValue, float priority)
{
    return RTDB.setStringAsync(&fbdo, path.c_str(), stringValue, priority);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, const String &stringValue, float priority)
{
    return RTDB.setString(&fbdo, path.c_str(), stringValue, priority);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, const String &stringValue, float priority)
{
    return RTDB.setStringAsync(&fbdo, path.c_str(), stringValue, priority);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, const char *stringValue, const String &ETag)
{
    return RTDB.setString(&fbdo, path.c_str(), stringValue, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, const char *stringValue, const String &ETag)
{
    return RTDB.setStringAsync(&fbdo, path.c_str(), stringValue, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, const String &stringValue, const String &ETag)
{
    return RTDB.setString(&fbdo, path.c_str(), stringValue, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, const String &stringValue, const String &ETag)
{
    return RTDB.setStringAsync(&fbdo, path.c_str(), stringValue, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, const char *stringValue, float priority, const String &ETag)
{
    return RTDB.setString(&fbdo, path.c_str(), stringValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, const char *stringValue, float priority, const String &ETag)
{
    return RTDB.setStringAsync(&fbdo, path.c_str(), stringValue, priority, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, const String &stringValue, float priority, const String &ETag)
{
    return RTDB.setString(&fbdo, path.c_str(), stringValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, const String &stringValue, float priority, const String &ETag)
{
    return RTDB.setStringAsync(&fbdo, path.c_str(), stringValue, priority, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, FirebaseJson &json)
{
    return RTDB.setJSON(&fbdo, path.c_str(), &json);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json)
{
    return RTDB.setJSONAsync(&fbdo, path.c_str(), &json);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr)
{
    return RTDB.setArray(&fbdo, path.c_str(), &arr);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr)
{
    return RTDB.setArrayAsync(&fbdo, path.c_str(), &arr);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority)
{
    return RTDB.setJSON(&fbdo, path.c_str(), &json, priority);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority)
{
    return RTDB.setJSONAsync(&fbdo, path.c_str(), &json, priority);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, float priority)
{
    return RTDB.setArray(&fbdo, path.c_str(), &arr, priority);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, float priority)
{
    return RTDB.setArrayAsync(&fbdo, path.c_str(), &arr, priority);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, FirebaseJson &json, const String &ETag)
{
    return RTDB.setJSON(&fbdo, path.c_str(), &json, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json, const String &ETag)
{
    return RTDB.setJSONAsync(&fbdo, path.c_str(), &json, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, const String &ETag)
{
    return RTDB.setArray(&fbdo, path.c_str(), &arr, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, const String &ETag)
{
    return RTDB.setArrayAsync(&fbdo, path.c_str(), &arr, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority, const String &ETag)
{
    return RTDB.setJSON(&fbdo, path.c_str(), &json, priority, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority, const String &ETag)
{
    return RTDB.setJSONAsync(&fbdo, path.c_str(), &json, priority, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, float priority, const String &ETag)
{
    return RTDB.setArray(&fbdo, path.c_str(), &arr, priority, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, float priority, const String &ETag)
{
    return RTDB.setArrayAsync(&fbdo, path.c_str(), &arr, priority, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size)
{
    return RTDB.setBlob(&fbdo, path.c_str(), blob, size);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size)
{
    return RTDB.setBlobAsync(&fbdo, path.c_str(), blob, size);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, float priority)
{
    return RTDB.setBlob(&fbdo, path.c_str(), blob, size, priority);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, float priority)
{
    return RTDB.setBlobAsync(&fbdo, path.c_str(), blob, size, priority);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, const String &ETag)
{
    return RTDB.setBlob(&fbdo, path.c_str(), blob, size, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, const String &ETag)
{
    return RTDB.setBlobAsync(&fbdo, path.c_str(), blob, size, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, float priority, const String &ETag)
{
    return RTDB.setBlob(&fbdo, path.c_str(), blob, size, priority, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, float priority, const String &ETag)
{
    return RTDB.setBlobAsync(&fbdo, path.c_str(), blob, size, priority, ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName)
{
    return RTDB.setFile(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName)
{
    return RTDB.setFileAsync(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, float priority)
{
    return RTDB.setFile(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), priority);
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, float priority)
{
    return RTDB.setFileAsync(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), priority);
}

bool FirebaseESP8266::set(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, const String &ETag)
{
    return RTDB.setFile(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, const String &ETag)
{
    return RTDB.setFileAsync(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), ETag.c_str());
}

bool FirebaseESP8266::set(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, float priority, const String &ETag)
{
    return RTDB.setFile(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), priority, ETag.c_str());
}

bool FirebaseESP8266::setAsync(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, float priority, const String &ETag)
{
    return RTDB.setFileAsync(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), priority, ETag.c_str());
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, T value)
{
    if (std::is_same<T, int>::value)
        return setInt(fbdo, path, value);
    else if (std::is_same<T, double>::value)
        return setDouble(fbdo, path, value);
    else if (std::is_same<T, bool>::value)
        return setBool(fbdo, path, value);
    else if (std::is_same<T, const char *>::value)
        return setString(fbdo, path, value);
    else if (std::is_same<T, const String &>::value)
        return setString(fbdo, path, value);
    else if (std::is_same<T, FirebaseJson &>::value)
        return setJson(fbdo, path, value);
    else if (std::is_same<T, FirebaseJson *>::value)
        return setJson(fbdo, path, &value);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return setArray(fbdo, path, value);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, T value, size_t size)
{
    if (std::is_same<T, uint8_t *>::value)
        return RTDB.setBlob(&fbdo, path.c_str(), value, size);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, T value, float priority)
{
    if (std::is_same<T, int>::value)
        return setInt(fbdo, path, value, priority);
    else if (std::is_same<T, double>::value)
        return setDouble(fbdo, path, value, priority);
    else if (std::is_same<T, bool>::value)
        return setBool(fbdo, path, value, priority);
    else if (std::is_same<T, const char *>::value)
        return setString(fbdo, path, value, priority);
    else if (std::is_same<T, const String &>::value)
        return setString(fbdo, path, value, priority);
    else if (std::is_same<T, FirebaseJson &>::value)
        return setJson(fbdo, path, value, priority);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return setArray(fbdo, path, value, priority);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, T value, size_t size, float priority)
{
    if (std::is_same<T, uint8_t *>::value)
        return RTDB.setBlob(&fbdo, path.c_str(), value, size, priority);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, T value, const String &ETag)
{
    if (std::is_same<T, int>::value)
        return setInt(fbdo, path, value, ETag);
    else if (std::is_same<T, double>::value)
        return setDouble(fbdo, path, value, ETag);
    else if (std::is_same<T, bool>::value)
        return setBool(fbdo, path, value, ETag);
    else if (std::is_same<T, const char *>::value)
        return setString(fbdo, path, value, ETag);
    else if (std::is_same<T, const String &>::value)
        return setString(fbdo, path, value, ETag);
    else if (std::is_same<T, FirebaseJson &>::value)
        return setJson(fbdo, path, value, ETag);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return setArray(fbdo, path, value, ETag);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, T value, size_t size, const String &ETag)
{
    if (std::is_same<T, uint8_t *>::value)
        return RTDB.setBlob(&fbdo, path.c_str(), value, size, ETag.c_str());
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, T value, float priority, const String &ETag)
{
    if (std::is_same<T, int>::value)
        return setInt(fbdo, path, value, priority, ETag);
    else if (std::is_same<T, double>::value)
        return setDouble(fbdo, path, value, priority, ETag);
    else if (std::is_same<T, bool>::value)
        return setBool(fbdo, path, value, priority, ETag);
    else if (std::is_same<T, const char *>::value)
        return setString(fbdo, path, value, priority, ETag);
    else if (std::is_same<T, const String &>::value)
        return setString(fbdo, path, value, priority, ETag);
    else if (std::is_same<T, FirebaseJson &>::value)
        return setJson(fbdo, path, value, priority, ETag);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return setArray(fbdo, path, value, priority, ETag);
}

template <typename T>
bool FirebaseESP8266::set(FirebaseData &fbdo, const String &path, T value, size_t size, float priority, const String &ETag)
{
    if (std::is_same<T, uint8_t *>::value)
        return RTDB.setBlob(&fbdo, path.c_str(), value, size, priority, ETag.c_str());
}

bool FirebaseESP8266::setInt(FirebaseData &fbdo, const String &path, int intValue)
{
    return RTDB.setInt(&fbdo, path.c_str(), intValue);
}

bool FirebaseESP8266::setIntAsync(FirebaseData &fbdo, const String &path, int intValue)
{
    return RTDB.setIntAsync(&fbdo, path.c_str(), intValue);
}

bool FirebaseESP8266::setInt(FirebaseData &fbdo, const String &path, int intValue, float priority)
{
    return RTDB.setInt(&fbdo, path.c_str(), intValue, priority);
}

bool FirebaseESP8266::setIntAsync(FirebaseData &fbdo, const String &path, int intValue, float priority)
{
    return RTDB.setIntAsync(&fbdo, path.c_str(), intValue, priority);
}

bool FirebaseESP8266::setInt(FirebaseData &fbdo, const String &path, int intValue, const String &ETag)
{
    return RTDB.setInt(&fbdo, path.c_str(), intValue, ETag.c_str());
}

bool FirebaseESP8266::setIntAsync(FirebaseData &fbdo, const String &path, int intValue, const String &ETag)
{
    return RTDB.setIntAsync(&fbdo, path.c_str(), intValue, ETag.c_str());
}

bool FirebaseESP8266::setInt(FirebaseData &fbdo, const String &path, int intValue, float priority, const String &ETag)
{
    return RTDB.setInt(&fbdo, path.c_str(), intValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setIntAsync(FirebaseData &fbdo, const String &path, int intValue, float priority, const String &ETag)
{
    return RTDB.setIntAsync(&fbdo, path.c_str(), intValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setFloat(FirebaseData &fbdo, const String &path, float floatValue)
{
    return RTDB.setFloat(&fbdo, path.c_str(), floatValue);
}

bool FirebaseESP8266::setFloatAsync(FirebaseData &fbdo, const String &path, float floatValue)
{
    return RTDB.setFloatAsync(&fbdo, path.c_str(), floatValue);
}

bool FirebaseESP8266::setFloat(FirebaseData &fbdo, const String &path, float floatValue, float priority)
{
    return RTDB.setFloat(&fbdo, path.c_str(), floatValue, priority);
}

bool FirebaseESP8266::setFloatAsync(FirebaseData &fbdo, const String &path, float floatValue, float priority)
{
    return RTDB.setFloatAsync(&fbdo, path.c_str(), floatValue, priority);
}

bool FirebaseESP8266::setFloat(FirebaseData &fbdo, const String &path, float floatValue, const String &ETag)
{
    return RTDB.setFloat(&fbdo, path.c_str(), floatValue, ETag.c_str());
}

bool FirebaseESP8266::setFloatAsync(FirebaseData &fbdo, const String &path, float floatValue, const String &ETag)
{
    return RTDB.setFloatAsync(&fbdo, path.c_str(), floatValue, ETag.c_str());
}

bool FirebaseESP8266::setFloat(FirebaseData &fbdo, const String &path, float floatValue, float priority, const String &ETag)
{
    return RTDB.setFloat(&fbdo, path.c_str(), floatValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setFloatAsync(FirebaseData &fbdo, const String &path, float floatValue, float priority, const String &ETag)
{
    return RTDB.setFloatAsync(&fbdo, path.c_str(), floatValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setDouble(FirebaseData &fbdo, const String &path, double doubleValue)
{
    return RTDB.setDouble(&fbdo, path.c_str(), doubleValue);
}

bool FirebaseESP8266::setDoubleAsync(FirebaseData &fbdo, const String &path, double doubleValue)
{
    return RTDB.setDoubleAsync(&fbdo, path.c_str(), doubleValue);
}

bool FirebaseESP8266::setDouble(FirebaseData &fbdo, const String &path, double doubleValue, float priority)
{
    return RTDB.setDouble(&fbdo, path.c_str(), doubleValue, priority);
}

bool FirebaseESP8266::setDoubleAsync(FirebaseData &fbdo, const String &path, double doubleValue, float priority)
{
    return RTDB.setDoubleAsync(&fbdo, path.c_str(), doubleValue, priority);
}

bool FirebaseESP8266::setDouble(FirebaseData &fbdo, const String &path, double doubleValue, const String &ETag)
{
    return RTDB.setDouble(&fbdo, path.c_str(), doubleValue, ETag.c_str());
}

bool FirebaseESP8266::setDoubleAsync(FirebaseData &fbdo, const String &path, double doubleValue, const String &ETag)
{
    return RTDB.setDoubleAsync(&fbdo, path.c_str(), doubleValue, ETag.c_str());
}

bool FirebaseESP8266::setDouble(FirebaseData &fbdo, const String &path, double doubleValue, float priority, const String &ETag)
{
    return RTDB.setDouble(&fbdo, path.c_str(), doubleValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setDoubleAsync(FirebaseData &fbdo, const String &path, double doubleValue, float priority, const String &ETag)
{
    return RTDB.setDoubleAsync(&fbdo, path.c_str(), doubleValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setBool(FirebaseData &fbdo, const String &path, bool boolValue)
{
    return RTDB.setBool(&fbdo, path.c_str(), boolValue);
}

bool FirebaseESP8266::setBoolAsync(FirebaseData &fbdo, const String &path, bool boolValue)
{
    return RTDB.setBoolAsync(&fbdo, path.c_str(), boolValue);
}

bool FirebaseESP8266::setBool(FirebaseData &fbdo, const String &path, bool boolValue, float priority)
{
    return RTDB.setBool(&fbdo, path.c_str(), boolValue, priority);
}

bool FirebaseESP8266::setBoolAsync(FirebaseData &fbdo, const String &path, bool boolValue, float priority)
{
    return RTDB.setBoolAsync(&fbdo, path.c_str(), boolValue, priority);
}

bool FirebaseESP8266::setBool(FirebaseData &fbdo, const String &path, bool boolValue, const String &ETag)
{
    return RTDB.setBool(&fbdo, path.c_str(), boolValue, ETag.c_str());
}

bool FirebaseESP8266::setBoolAsync(FirebaseData &fbdo, const String &path, bool boolValue, const String &ETag)
{
    return RTDB.setBoolAsync(&fbdo, path.c_str(), boolValue, ETag.c_str());
}

bool FirebaseESP8266::setBool(FirebaseData &fbdo, const String &path, bool boolValue, float priority, const String &ETag)
{
    return RTDB.setBool(&fbdo, path.c_str(), boolValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setBoolAsync(FirebaseData &fbdo, const String &path, bool boolValue, float priority, const String &ETag)
{
    return RTDB.setBoolAsync(&fbdo, path.c_str(), boolValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setString(FirebaseData &fbdo, const String &path, const String &stringValue)
{
    return RTDB.setString(&fbdo, path.c_str(), stringValue);
}

bool FirebaseESP8266::setStringAsync(FirebaseData &fbdo, const String &path, const String &stringValue)
{
    return RTDB.setStringAsync(&fbdo, path.c_str(), stringValue);
}

bool FirebaseESP8266::setString(FirebaseData &fbdo, const String &path, const String &stringValue, float priority)
{
    return RTDB.setString(&fbdo, path.c_str(), stringValue, priority);
}

bool FirebaseESP8266::setStringAsync(FirebaseData &fbdo, const String &path, const String &stringValue, float priority)
{
    return RTDB.setStringAsync(&fbdo, path.c_str(), stringValue, priority);
}

bool FirebaseESP8266::setString(FirebaseData &fbdo, const String &path, const String &stringValue, const String &ETag)
{
    return RTDB.setString(&fbdo, path.c_str(), stringValue, ETag.c_str());
}

bool FirebaseESP8266::setStringAsync(FirebaseData &fbdo, const String &path, const String &stringValue, const String &ETag)
{
    return RTDB.setStringAsync(&fbdo, path.c_str(), stringValue, ETag.c_str());
}

bool FirebaseESP8266::setString(FirebaseData &fbdo, const String &path, const String &stringValue, float priority, const String &ETag)
{
    return RTDB.setString(&fbdo, path.c_str(), stringValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setStringAsync(FirebaseData &fbdo, const String &path, const String &stringValue, float priority, const String &ETag)
{
    return RTDB.setStringAsync(&fbdo, path.c_str(), stringValue, priority, ETag.c_str());
}

bool FirebaseESP8266::setJSON(FirebaseData &fbdo, const String &path, FirebaseJson &json)
{
    return RTDB.setJSON(&fbdo, path.c_str(), &json);
}

bool FirebaseESP8266::setJSONAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json)
{
    return RTDB.setJSONAsync(&fbdo, path.c_str(), &json);
}

bool FirebaseESP8266::setJSON(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority)
{
    return RTDB.setJSON(&fbdo, path.c_str(), &json, priority);
}

bool FirebaseESP8266::setJSONAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority)
{
    return RTDB.setJSONAsync(&fbdo, path.c_str(), &json, priority);
}

bool FirebaseESP8266::setJSON(FirebaseData &fbdo, const String &path, FirebaseJson &json, const String &ETag)
{
    return RTDB.setJSON(&fbdo, path.c_str(), &json, ETag.c_str());
}

bool FirebaseESP8266::setJSONAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json, const String &ETag)
{
    return RTDB.setJSONAsync(&fbdo, path.c_str(), &json, ETag.c_str());
}

bool FirebaseESP8266::setJSON(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority, const String &ETag)
{
    return RTDB.setJSON(&fbdo, path.c_str(), &json, priority, ETag.c_str());
}

bool FirebaseESP8266::setJSONAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority, const String &ETag)
{
    return RTDB.setJSONAsync(&fbdo, path.c_str(), &json, priority, ETag.c_str());
}

bool FirebaseESP8266::setArray(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr)
{
    return RTDB.setArray(&fbdo, path.c_str(), &arr);
}

bool FirebaseESP8266::setArrayAsync(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr)
{
    return RTDB.setArrayAsync(&fbdo, path.c_str(), &arr);
}

bool FirebaseESP8266::setArray(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, float priority)
{
    return RTDB.setArray(&fbdo, path.c_str(), &arr, priority);
}

bool FirebaseESP8266::setArrayAsync(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, float priority)
{
    return RTDB.setArrayAsync(&fbdo, path.c_str(), &arr, priority);
}

bool FirebaseESP8266::setArray(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, const String &ETag)
{
    return RTDB.setArray(&fbdo, path.c_str(), &arr, ETag.c_str());
}

bool FirebaseESP8266::setArrayAsync(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, const String &ETag)
{
    return RTDB.setArrayAsync(&fbdo, path.c_str(), &arr, ETag.c_str());
}

bool FirebaseESP8266::setArray(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, float priority, const String &ETag)
{
    return RTDB.setArray(&fbdo, path.c_str(), &arr, priority, ETag.c_str());
}

bool FirebaseESP8266::setArrayAsync(FirebaseData &fbdo, const String &path, FirebaseJsonArray &arr, float priority, const String &ETag)
{
    return RTDB.setArrayAsync(&fbdo, path.c_str(), &arr, priority, ETag.c_str());
}

bool FirebaseESP8266::setBlob(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size)
{
    return RTDB.setBlob(&fbdo, path.c_str(), blob, size);
}

bool FirebaseESP8266::setBlobAsync(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size)
{
    return RTDB.setBlobAsync(&fbdo, path.c_str(), blob, size);
}

bool FirebaseESP8266::setBlob(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, float priority)
{
    return RTDB.setBlob(&fbdo, path.c_str(), blob, size, priority);
}

bool FirebaseESP8266::setBlobAsync(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, float priority)
{
    return RTDB.setBlobAsync(&fbdo, path.c_str(), blob, size, priority);
}

bool FirebaseESP8266::setBlob(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, const String &ETag)
{
    return RTDB.setBlob(&fbdo, path.c_str(), blob, size, ETag.c_str());
}

bool FirebaseESP8266::setBlobAsync(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, const String &ETag)
{
    return RTDB.setBlobAsync(&fbdo, path.c_str(), blob, size, ETag.c_str());
}

bool FirebaseESP8266::setBlob(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, float priority, const String &ETag)
{
    return RTDB.setBlob(&fbdo, path.c_str(), blob, size, priority, ETag.c_str());
}

bool FirebaseESP8266::setBlobAsync(FirebaseData &fbdo, const String &path, uint8_t *blob, size_t size, float priority, const String &ETag)
{
    return RTDB.setBlobAsync(&fbdo, path.c_str(), blob, size, priority, ETag.c_str());
}

bool FirebaseESP8266::setFile(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName)
{
    return RTDB.setFile(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str());
}

bool FirebaseESP8266::setFileAsync(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName)
{
    return RTDB.setFileAsync(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str());
}

bool FirebaseESP8266::setFile(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, float priority)
{
    return RTDB.setFile(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), priority);
}

bool FirebaseESP8266::setFileAsync(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, float priority)
{
    return RTDB.setFileAsync(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), priority);
}

bool FirebaseESP8266::setFile(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, const String &ETag)
{
    return RTDB.setFile(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), ETag.c_str());
}

bool FirebaseESP8266::setFileAsync(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, const String &ETag)
{
    return RTDB.setFileAsync(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), ETag.c_str());
}

bool FirebaseESP8266::setFile(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, float priority, const String &ETag)
{
    return RTDB.setFile(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), priority, ETag.c_str());
}

bool FirebaseESP8266::setFileAsync(FirebaseData &fbdo, uint8_t storageType, const String &path, const String &fileName, float priority, const String &ETag)
{
    return RTDB.setFileAsync(&fbdo, getMemStorageType(storageType), path.c_str(), fileName.c_str(), priority, ETag.c_str());
}

bool FirebaseESP8266::setTimestamp(FirebaseData &fbdo, const String &path)
{
    return RTDB.setTimestamp(&fbdo, path.c_str());
}

bool FirebaseESP8266::setTimestampAsync(FirebaseData &fbdo, const String &path)
{
    return RTDB.setTimestampAsync(&fbdo, path.c_str());
}

bool FirebaseESP8266::updateNode(FirebaseData &fbdo, const String path, FirebaseJson &json)
{
    return RTDB.updateNode(&fbdo, path.c_str(), &json);
}

bool FirebaseESP8266::updateNodeAsync(FirebaseData &fbdo, const String path, FirebaseJson &json)
{
    return RTDB.updateNodeAsync(&fbdo, path.c_str(), &json);
}

bool FirebaseESP8266::updateNode(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority)
{
    return RTDB.updateNode(&fbdo, path.c_str(), &json, priority);
}

bool FirebaseESP8266::updateNodeAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority)
{
    return RTDB.updateNodeAsync(&fbdo, path.c_str(), &json, priority);
}

bool FirebaseESP8266::updateNodeSilent(FirebaseData &fbdo, const String &path, FirebaseJson &json)
{
    return RTDB.updateNodeSilent(&fbdo, path.c_str(), &json);
}

bool FirebaseESP8266::updateNodeSilentAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json)
{
    return RTDB.updateNodeSilentAsync(&fbdo, path.c_str(), &json);
}

bool FirebaseESP8266::updateNodeSilent(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority)
{
    return RTDB.updateNodeSilent(&fbdo, path.c_str(), &json, priority);
}

bool FirebaseESP8266::updateNodeSilentAsync(FirebaseData &fbdo, const String &path, FirebaseJson &json, float priority)
{
    return RTDB.updateNodeSilentAsync(&fbdo, path.c_str(), &json, priority);
}

bool FirebaseESP8266::get(FirebaseData &fbdo, const String &path)
{
    return RTDB.get(&fbdo, path.c_str());
}

bool FirebaseESP8266::getInt(FirebaseData &fbdo, const String &path)
{
    return RTDB.getInt(&fbdo, path.c_str());
}

bool FirebaseESP8266::getInt(FirebaseData &fbdo, const String &path, int &target)
{
    return RTDB.getInt(&fbdo, path.c_str(), &target);
}

bool FirebaseESP8266::getFloat(FirebaseData &fbdo, const String &path)
{
    return RTDB.getFloat(&fbdo, path.c_str());
}

bool FirebaseESP8266::getFloat(FirebaseData &fbdo, const String &path, float &target)
{
    return RTDB.getFloat(&fbdo, path.c_str(), &target);
}

bool FirebaseESP8266::getDouble(FirebaseData &fbdo, const String &path)
{
    return RTDB.getDouble(&fbdo, path.c_str());
}

bool FirebaseESP8266::getDouble(FirebaseData &fbdo, const String &path, double &target)
{
    return RTDB.getDouble(&fbdo, path.c_str(), &target);
}

bool FirebaseESP8266::getBool(FirebaseData &fbdo, const String &path)
{
    return RTDB.getBool(&fbdo, path.c_str());
}

bool FirebaseESP8266::getBool(FirebaseData &fbdo, const String &path, bool &target)
{
    return RTDB.getBool(&fbdo, path.c_str(), &target);
}

bool FirebaseESP8266::getString(FirebaseData &fbdo, const String &path)
{
    return RTDB.getString(&fbdo, path.c_str());
}

bool FirebaseESP8266::getString(FirebaseData &fbdo, const String &path, String &target)
{
    return RTDB.getString(&fbdo, path.c_str(), &target);
}

bool FirebaseESP8266::getJSON(FirebaseData &fbdo, const String &path)
{
    return RTDB.getJSON(&fbdo, path.c_str());
}

bool FirebaseESP8266::getJSON(FirebaseData &fbdo, const String &path, FirebaseJson *target)
{
    return RTDB.getJSON(&fbdo, path.c_str(), target);
}

bool FirebaseESP8266::getJSON(FirebaseData &fbdo, const String &path, QueryFilter &query)
{
    return RTDB.getJSON(&fbdo, path.c_str(), &query);
}

bool FirebaseESP8266::getJSON(FirebaseData &fbdo, const String &path, QueryFilter &query, FirebaseJson *target)
{
    return RTDB.getJSON(&fbdo, path.c_str(), &query, target);
}

bool FirebaseESP8266::getArray(FirebaseData &fbdo, const String &path)
{
    return RTDB.getArray(&fbdo, path.c_str());
}

bool FirebaseESP8266::getArray(FirebaseData &fbdo, const String &path, FirebaseJsonArray *target)
{
    return RTDB.getArray(&fbdo, path.c_str(), target);
}

bool FirebaseESP8266::getArray(FirebaseData &fbdo, const String &path, QueryFilter &query)
{
    return RTDB.getArray(&fbdo, path.c_str(), &query);
}

bool FirebaseESP8266::getArray(FirebaseData &fbdo, const String &path, QueryFilter &query, FirebaseJsonArray *target)
{
    return RTDB.getArray(&fbdo, path.c_str(), &query, target);
}

bool FirebaseESP8266::getBlob(FirebaseData &fbdo, const String &path)
{
    return RTDB.getBlob(&fbdo, path.c_str());
}

bool FirebaseESP8266::getBlob(FirebaseData &fbdo, const String &path, std::vector<uint8_t> &target)
{
    return RTDB.getBlob(&fbdo, path.c_str(), &target);
}

bool FirebaseESP8266::getFile(FirebaseData &fbdo, uint8_t storageType, const String &nodePath, const String &fileName)
{
    return RTDB.getFile(&fbdo, getMemStorageType(storageType), nodePath.c_str(), fileName.c_str());
}

bool FirebaseESP8266::deleteNode(FirebaseData &fbdo, const String &path)
{
    return RTDB.deleteNode(&fbdo, path.c_str());
}

bool FirebaseESP8266::deleteNode(FirebaseData &fbdo, const String &path, const String &ETag)
{
    return RTDB.deleteNode(&fbdo, path.c_str(), ETag.c_str());
}

bool FirebaseESP8266::deleteNodesByTimestamp(FirebaseData &fbdo, const String &path, const String &timestampNode, size_t limit, unsigned long dataRetentionPeriod)
{
  return RTDB.deleteNodesByTimestamp(&fbdo, path.c_str(), timestampNode.c_str(), limit, dataRetentionPeriod);
}

bool FirebaseESP8266::beginStream(FirebaseData &fbdo, const String &path)
{
    return RTDB.beginStream(&fbdo, path.c_str());
}

bool FirebaseESP8266::beginMultiPathStream(FirebaseData &fbdo, const String &parentPath, const String *childPath, size_t size)
{
    return RTDB.beginMultiPathStream(&fbdo, parentPath.c_str(), childPath, size);
}

bool FirebaseESP8266::readStream(FirebaseData &fbdo)
{
    return RTDB.readStream(&fbdo);
}

bool FirebaseESP8266::endStream(FirebaseData &fbdo)
{
    return RTDB.endStream(&fbdo);
}

void FirebaseESP8266::processErrorQueue(FirebaseData &fbdo, FirebaseData::QueueInfoCallback callback)
{
    return RTDB.processErrorQueue(&fbdo, callback);
}

uint32_t FirebaseESP8266::getErrorQueueID(FirebaseData &fbdo)
{
    return RTDB.getErrorQueueID(&fbdo);
}

bool FirebaseESP8266::isErrorQueueExisted(FirebaseData &fbdo, uint32_t errorQueueID)
{
    return RTDB.isErrorQueueExisted(&fbdo, errorQueueID);
}

void FirebaseESP8266::errorToString(int httpCode, std::string &buff)
{
    Signer.errorToString(httpCode, buff);
}

void FirebaseESP8266::setStreamCallback(FirebaseData &fbdo, FirebaseData::StreamEventCallback dataAvailableCallback, FirebaseData::StreamTimeoutCallback timeoutCallback)
{
    RTDB.setStreamCallback(&fbdo, dataAvailableCallback, timeoutCallback);
}

void FirebaseESP8266::setMultiPathStreamCallback(FirebaseData &fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback, FirebaseData::StreamTimeoutCallback timeoutCallback)
{
    RTDB.setMultiPathStreamCallback(&fbdo, multiPathDataCallback, timeoutCallback);
}

void FirebaseESP8266::removeStreamCallback(FirebaseData &fbdo)
{
    RTDB.removeStreamCallback(&fbdo);
}

void FirebaseESP8266::removeMultiPathStreamCallback(FirebaseData &fbdo)
{
    RTDB.removeMultiPathStreamCallback(&fbdo);
}

void FirebaseESP8266::beginAutoRunErrorQueue(FirebaseData &fbdo, FirebaseData::QueueInfoCallback callback)
{
    RTDB.beginAutoRunErrorQueue(&fbdo, callback);
}

void FirebaseESP8266::endAutoRunErrorQueue(FirebaseData &fbdo)
{
    RTDB.endAutoRunErrorQueue(&fbdo);
}

void FirebaseESP8266::clearErrorQueue(FirebaseData &fbdo)
{
    RTDB.clearErrorQueue(&fbdo);
}

bool FirebaseESP8266::backup(FirebaseData &fbdo, uint8_t storageType, const String &nodePath, const String &dirPath)
{
    return RTDB.backup(&fbdo, getMemStorageType(storageType), nodePath.c_str(), dirPath.c_str());
}

bool FirebaseESP8266::restore(FirebaseData &fbdo, uint8_t storageType, const String &nodePath, const String &dirPath)
{
    return RTDB.restore(&fbdo, getMemStorageType(storageType), nodePath.c_str(), dirPath.c_str());
}

void FirebaseESP8266::setMaxRetry(FirebaseData &fbdo, uint8_t num)
{
    RTDB.setMaxRetry(&fbdo, num);
}

void FirebaseESP8266::setMaxErrorQueue(FirebaseData &fbdo, uint8_t num)
{
    RTDB.setMaxErrorQueue(&fbdo, num);
}

bool FirebaseESP8266::saveErrorQueue(FirebaseData &fbdo, const String &filename, uint8_t storageType)
{
    return RTDB.saveErrorQueue(&fbdo, filename.c_str(), getMemStorageType(storageType));
}

bool FirebaseESP8266::restoreErrorQueue(FirebaseData &fbdo, const String &filename, uint8_t storageType)
{
    return RTDB.restoreErrorQueue(&fbdo, filename.c_str(), getMemStorageType(storageType));
}

uint8_t FirebaseESP8266::errorQueueCount(FirebaseData &fbdo, const String &filename, uint8_t storageType)
{
    return RTDB.errorQueueCount(&fbdo, filename.c_str(), getMemStorageType(storageType));
}

bool FirebaseESP8266::deleteStorageFile(const String &filename, uint8_t storageType)
{
    return RTDB.deleteStorageFile(filename.c_str(), getMemStorageType(storageType));
}

bool FirebaseESP8266::isErrorQueueFull(FirebaseData &fbdo)
{
    return RTDB.isErrorQueueFull(&fbdo);
}

uint8_t FirebaseESP8266::errorQueueCount(FirebaseData &fbdo)
{
    return RTDB.errorQueueCount(&fbdo);
}

bool FirebaseESP8266::handleFCMRequest(FirebaseData &fbdo, fb_esp_fcm_msg_type messageType)
{
    if (!fbdo.reconnect())
        return false;

    if (!ut->waitIdle(fbdo._ss.http_code))
        return false;

    if (fbdo.fcm._server_key.length() == 0)
    {
        fbdo._ss.http_code = FIREBASE_ERROR_HTTPC_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    if (fbdo.fcm._deviceToken.size() == 0 && messageType == fb_esp_fcm_msg_type::msg_single)
    {
        fbdo._ss.http_code = FIREBASE_ERROR_HTTPC_NO_FCM_DEVICE_TOKEN_PROVIDED;
        return false;
    }

    if (messageType == fb_esp_fcm_msg_type::msg_single && fbdo.fcm._deviceToken.size() > 0 && fbdo.fcm._index > fbdo.fcm._deviceToken.size() - 1)
    {
        fbdo._ss.http_code = FIREBASE_ERROR_HTTPC_NO_FCM_INDEX_NOT_FOUND_IN_DEVICE_TOKEN_PROVIDED;
        return false;
    }

    if (messageType == fb_esp_fcm_msg_type::msg_topic && fbdo.fcm._topic.length() == 0)
    {
        fbdo._ss.http_code = FIREBASE_ERROR_HTTPC_NO_FCM_TOPIC_PROVIDED;
        return false;
    }

    if (Signer.getCfg()->_int.fb_processing)
    {
        fbdo._ss.http_code = FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_INUSED;
        return false;
    }

    Signer.getCfg()->_int.fb_processing = true;

    fbdo.fcm.fcm_begin(fbdo);

    return fbdo.fcm.fcm_send(fbdo, messageType);
}

bool FirebaseESP8266::sendMessage(FirebaseData &fbdo, uint16_t index)
{
    fbdo.fcm._index = index;
    return handleFCMRequest(fbdo, fb_esp_fcm_msg_type::msg_single);
}

bool FirebaseESP8266::broadcastMessage(FirebaseData &fbdo)
{
    return handleFCMRequest(fbdo, fb_esp_fcm_msg_type::msg_multicast);
}

bool FirebaseESP8266::sendTopic(FirebaseData &fbdo)
{
    return handleFCMRequest(fbdo, fb_esp_fcm_msg_type::msg_topic);
}

bool FirebaseESP8266::sdBegin(int8_t ss)
{
    if (Signer.getCfg())
    {
        Signer.getCfg()->_int.sd_config.sck = -1;
        Signer.getCfg()->_int.sd_config.miso = -1;
        Signer.getCfg()->_int.sd_config.mosi = -1;
        Signer.getCfg()->_int.sd_config.ss = ss;
    }

    if (ss > -1)
        return SD_FS.begin(ss);
    else
        return SD_FS.begin(SD_CS_PIN);
}

fb_esp_mem_storage_type FirebaseESP8266::getMemStorageType(uint8_t old_type)
{
    return (fb_esp_mem_storage_type)(old_type);
}

bool FirebaseESP8266::setSystemTime(time_t ts)
{
    return ut->setTimestamp(ts) == 0;
}

FirebaseESP8266 Firebase = FirebaseESP8266();

#endif /* ESP8266 */

#endif /* FirebaseESP8266_CPP */