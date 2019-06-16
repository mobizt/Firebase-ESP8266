/*
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2019 mobizt
 *
*/


//This example shows how to use different Firebase Data objects to handle to streaming and other for store, read, update data.
//Pros - No delay streaming
//     - Always connected stream, no stream data missing or interrupt
//Cons - Use more memory
//     - Unexpected error

//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

#define WIFI_SSID "YOUR_WIFI_AP"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define FIREBASE_HOST "YOUR_FIREBASE_PROJECT.firebaseio.com" //Do not include https:// in FIREBASE_HOST
#define FIREBASE_AUTH "YOUR_FIREBASE_DATABASE_SECRET"

//Define Firebase Data objects
FirebaseData firebaseData1;
FirebaseData firebaseData2;

unsigned long sendDataPrevMillis1;

uint16_t count1;

String json;

String path = "";

void setup()
{

  Serial.begin(115200);
  Serial.println();
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  Serial.println("------------------------------------");
  Serial.println("Begin stream 1...");
  if (!Firebase.beginStream(firebaseData2, path + "/Stream/data1"))
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData2.errorReason());
    Serial.println();
  }
  else
  {
    Serial.println("PASSED");
    Serial.println("------------------------------------");
    Serial.println();
  }
}

void loop()
{

  if (!Firebase.readStream(firebaseData2))
  {
    Serial.println("Can't read stream data");
    Serial.println("REASON: " + firebaseData2.errorReason());
    Serial.println();
  }

  if (firebaseData2.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

  if (firebaseData2.streamAvailable())
  {
    Serial.println("------------------------------------");
    Serial.println("Stream Data Available...");
    Serial.println("STREAM PATH: " + firebaseData2.streamPath());
    Serial.println("EVENT PATH: " + firebaseData2.dataPath());
    Serial.println("DATA TYPE: " + firebaseData2.dataType());
    Serial.println("EVENT TYPE: " + firebaseData2.eventType());
    Serial.print("VALUE: ");
    if (firebaseData2.dataType() == "int")
      Serial.println(firebaseData2.intData());
    else if (firebaseData2.dataType() == "float")
      Serial.println(firebaseData2.floatData(), 5);
    else if (firebaseData2.dataType() == "double")
      Serial.println(firebaseData2.doubleData(), 9);
    else if (firebaseData2.dataType() == "boolean")
      Serial.println(firebaseData2.boolData() == 1 ? "true" : "false");
    else if (firebaseData2.dataType() == "string")
      Serial.println(firebaseData2.stringData());
    else if (firebaseData2.dataType() == "json")
      Serial.println(firebaseData2.jsonData());
    else if (firebaseData2.dataType() == "blob")
    {
      std::vector<uint8_t> blob = firebaseData2.blobData();

      Serial.println();

      for (int i = 0; i < blob.size(); i++)
      {
        if (i > 0 && i % 16 == 0)
          Serial.println();

        if (i < 16)
          Serial.print("0");

        Serial.print(blob[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      
    }
    Serial.println("------------------------------------");
    Serial.println();
  }

  if (millis() - sendDataPrevMillis1 > 15000)
  {
    sendDataPrevMillis1 = millis();
    count1++;

    //Create demo data
    uint8_t data[256];
    for (int i = 0; i < 256; i++)
      data[i] = i;
    data[255] = rand();

    Serial.println("------------------------------------");
    Serial.println("Set Blob Data 1...");
    if (Firebase.setBlob(firebaseData1, path + "/Stream/data1", data, sizeof(data)))
    {
      Serial.println("PASSED");
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData1.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }

    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    Serial.println();
  }
}