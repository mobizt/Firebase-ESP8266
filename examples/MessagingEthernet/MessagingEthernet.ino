/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP8266
 *
 * Copyright (c) 2022 mobizt
 *
 */

// This example is for ESP8266 and ENC28J60 Ethernet module.

/**
 *
 * The ENC28J60 Ethernet module and ESP8266 board, SPI port wiring connection.
 *
 * ESP8266 (Wemos D1 Mini or NodeMCU)        ENC28J60
 *
 * GPIO12 (D6) - MISO                        SO
 * GPIO13 (D7) - MOSI                        SI
 * GPIO14 (D5) - SCK                         SCK
 * GPIO16 (D0) - CS                          CS
 * GND                                       GND
 * 3V3                                       VCC
 *
 */

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#include <ENC28J60lwIP.h>
//#include <W5100lwIP.h>
//#include <W5500lwIP.h>

#define FIREBASE_FCM_SERVER_KEY "FIREBASE_PROJECT_CLOUD_MESSAGING_SERVER_KEY"
#define FIREBASE_FCM_DEVICE_TOKEN_1 "RECIPIENT_DEVICE_TOKEN"
#define FIREBASE_FCM_DEVICE_TOKEN_2 "ANOTHER_RECIPIENT_DEVICE_TOKEN"

#define ETH_CS_PIN 16 // GPIO 16 connected to Ethernet module (ENC28J60) CS pin

ENC28J60lwIP eth(ETH_CS_PIN);
// Wiznet5100lwIP eth(ETH_CS_PIN);
// Wiznet5500lwIP eth(ETH_CS_PIN);

SPI_ETH_Module spi_ethernet_module;

// Define Firebase Data object
FirebaseData fbdo;

unsigned long lastTime = 0;

int count = 0;

void sendMessage();

void setup()
{

    Serial.begin(115200);

    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV4); // 4 MHz?
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    eth.setDefault(); // use ethernet for default route
    if (!eth.begin())
    {
        Serial.println("ethernet hardware not found ... sleeping");
        while (1)
        {
            delay(1000);
        }
    }
    else
    {
        Serial.print("connecting ethernet");
        while (!eth.connected())
        {
            Serial.print(".");
            delay(1000);
        }
    }
    Serial.println();
    Serial.print("ethernet IP address: ");
    Serial.println(eth.localIP());

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    Firebase.reconnectWiFi(true);

    spi_ethernet_module.enc28j60 = &eth;
    // spi_ethernet_module.w5100 = &eth;
    // spi_ethernet_module.w5500 = &eth;

    fbdo.fcm.begin(FIREBASE_FCM_SERVER_KEY, &spi_ethernet_module);

    fbdo.fcm.addDeviceToken(FIREBASE_FCM_DEVICE_TOKEN_1);

    fbdo.fcm.addDeviceToken(FIREBASE_FCM_DEVICE_TOKEN_2);

    fbdo.fcm.setPriority("high");

    fbdo.fcm.setTimeToLive(1000);

    sendMessage();
}

void loop()
{

    if (millis() - lastTime > 60 * 1000 || lastTime == 0)
    {
        lastTime = millis();
        sendMessage();
    }
}

void sendMessage()
{

    fbdo.fcm.setNotifyMessage("Notification", "Hello World! " + String(count));

    FirebaseJson msg;
    msg.add("myData", count);

    fbdo.fcm.setDataMessage(msg.raw());

    // Firebase.broadcastMessage(fbdo)
    // Firebase.sendTopic(fbdo)
    Serial.printf("Send message... %s\n", Firebase.sendMessage(fbdo, 0) ? "ok" : fbdo.errorReason().c_str());

    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
        Serial.println(fbdo.fcm.getSendResult());

    Serial.println();

    count++;
}
