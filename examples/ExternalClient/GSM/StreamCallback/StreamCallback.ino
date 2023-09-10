/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

/** This example shows the RTDB data changed notification with with SIM7600x, ESP8266 and TinyGSMClient. */

// To allow TinyGSM library integration, the following macro should be defined in src/ESP_Mail_FS.h or
// your custom config file src/Custom_ESP_Mail_FS.h.
//  #define TINY_GSM_MODEM_SIM7600

#include <SoftwareSerial.h>

#define ESP8266_RX_PIN 14 // ESP8266 GPIO 14 connected to SIM7600 TX Pin
#define ESP8266_TX_PIN 12 // ESP8266 GPIO 12 connected to SIM7600 RX Pin
#define ESP8266_PWR_PIN 5 // ESP8266 GPIO 5 connected to SIM7600 PWRKEY Pin
#define ESP8266_RESET 4   // ESP8266 GPIO 4 connected to SIM7600 RESET Pin
#define UART_BAUD 115200

SoftwareSerial softSerial;

#define TINY_GSM_MODEM_SIM7600

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
#define SerialAT softSerial

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[] = "YourAPN";
const char gprsUser[] = "";
const char gprsPass[] = "";

#include <Firebase_ESP_Client.h>
#include <TinyGsmClient.h>

TinyGsm modem(SerialAT);

// Multiple simultaneous connections should be supported
// TINY_GSM_MUX_COUNT on SIM7600 is 10.
TinyGsmClient gsm_client1(modem, 0);
TinyGsmClient gsm_client2(modem, 1);

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "API_KEY"

/* 3. Define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

// Define Firebase Data object
FirebaseData stream;
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

int count = 0;

volatile bool dataChanged = false;

void streamCallback(FirebaseStream data)
{
    Serial_Printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                  data.streamPath().c_str(),
                  data.dataPath().c_str(),
                  data.dataType().c_str(),
                  data.eventType().c_str());
    printResult(data); // see addons/RTDBHelper.h
    Serial.println();

    // This is the size of stream payload received (current and max value)
    // Max payload size is the payload size under the stream path since the stream connected
    // and read once and will not update until stream reconnection takes place.
    // This max value will be zero as no payload received in case of ESP8266 which
    // BearSSL reserved Rx buffer size is less than the actual stream payload.
    Serial_Printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());

    // Due to limited of stack memory, do not perform any task that used large memory here especially starting connect to server.
    // Just set this flag and check it status later.
    dataChanged = true;
}

void streamTimeoutCallback(bool timeout)
{
    if (timeout)
        Serial.println("stream timed out, resuming...\n");

    if (!stream.httpConnected())
        Serial_Printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void setup()
{

    Serial.begin(115200);

    delay(10);

    // A7600 Reset
    pinMode(ESP8266_RESET, OUTPUT);
    digitalWrite(ESP8266_RESET, LOW);
    delay(100);
    digitalWrite(ESP8266_RESET, HIGH);
    delay(3000);
    digitalWrite(ESP8266_RESET, LOW);

    pinMode(ESP8266_PWR_PIN, OUTPUT);
    digitalWrite(ESP8266_PWR_PIN, LOW);
    delay(100);
    digitalWrite(ESP8266_PWR_PIN, HIGH);
    delay(1000);
    digitalWrite(ESP8266_PWR_PIN, LOW);

    DBG("Wait...");

    delay(3000);

    SerialAT.begin(UART_BAUD, SERIAL_8N1, ESP8266_RX_PIN, ESP8266_TX_PIN);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    DBG("Initializing modem...");
    if (!modem.init())
    {
        DBG("Failed to restart modem, delaying 10s and retrying");
        return;
    }

    /*
    2 Automatic
    13 GSM Only
    14 WCDMA Only
    38 LTE Only
    */
    modem.setNetworkMode(38);
    if (modem.waitResponse(10000L) != 1)
    {
        DBG(" setNetworkMode faill");
    }

    String name = modem.getModemName();
    DBG("Modem Name:", name);

    String modemInfo = modem.getModemInfo();
    DBG("Modem Info:", modemInfo);

    Serial_Printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    // The WiFi credentials are required for WiFiNINA and WiFi101 libraries
    // due to it does not have reconnect feature.
    config.wifi.clearAP();
    config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // Or use legacy authenticate method
    // config.database_url = DATABASE_URL;
    // config.signer.tokens.legacy_token = "<database secret>";

    // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

    fbdo.setGSMClient(&gsm_client1, &modem, GSM_PIN, apn, gprsUser, gprsPass);
    stream.setGSMClient(&gsm_client2, &modem, GSM_PIN, apn, gprsUser, gprsPass);

    // Comment or pass false value when WiFi reconnection will control by your code or third party library
    Firebase.reconnectWiFi(true);

    // required for large file data, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
    stream.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

    Firebase.begin(&config, &auth);

    if (!Firebase.RTDB.beginStream(&stream, "/test/stream/data"))
        Serial_Printf("sream begin error, %s\n\n", stream.errorReason().c_str());

    Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
}

void loop()
{
    // For use with stream callback function
    // for non-ESP devices
    Firebase.RTDB.runStream();

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();
        count++;
        FirebaseJson json;
        json.add("data", "hello");
        json.add("num", count);
        Serial_Printf("Set json... %s\n\n", Firebase.RTDB.setJSON(&fbdo, "/test/stream/data/json", &json) ? "ok" : fbdo.errorReason().c_str());
    }

    if (dataChanged)
    {
        dataChanged = false;
        // When stream data is available, do anything here...
    }
}
