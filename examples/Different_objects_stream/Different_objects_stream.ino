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

void printJsonObjectContent(FirebaseData &data);

unsigned long sendDataPrevMillis1;

uint16_t count1;


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
      printJsonObjectContent(firebaseData2);
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

    FirebaseJson json;
    json.addInt("data1-1",count1).addInt("data1-2",count1 + 1).addInt("data1-3",count1 + 2);
    Serial.println("------------------------------------");
    Serial.println("Update Data 1...");
    if (Firebase.updateNode(firebaseData1, path + "/Stream/data1", json))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData1.dataPath());
      Serial.println("TYPE: " + firebaseData1.dataType());
      Serial.print("VALUE: ");
      if (firebaseData1.dataType() == "int")
        Serial.println(firebaseData1.intData());
      else if (firebaseData1.dataType() == "float")
        Serial.println(firebaseData1.floatData(), 5);
      else if (firebaseData1.dataType() == "double")
        printf("%.9lf\n", firebaseData1.doubleData());
      else if (firebaseData1.dataType() == "boolean")
        Serial.println(firebaseData1.boolData() == 1 ? "true" : "false");
      else if (firebaseData1.dataType() == "string")
        Serial.println(firebaseData1.stringData());
      else if (firebaseData1.dataType() == "json")
        Serial.println(firebaseData1.jsonData());
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

    count1+=3;
  }

  
}


void printJsonObjectContent(FirebaseData &data){
  size_t tokenCount = data.jsonObject().parse(false).getJsonObjectIteratorCount();
  String key;
  String value;
  FirebaseJsonObject jsonParseResult;
  Serial.println();
  for (size_t i = 0; i < tokenCount; i++)
  {
    data.jsonObject().jsonObjectiterator(i,key,value);
    jsonParseResult = data.jsonObject().parseResult();
    Serial.print("KEY: ");
    Serial.print(key);
    Serial.print(", ");
    Serial.print("VALUE: ");
    Serial.print(value); 
    Serial.print(", ");
    Serial.print("TYPE: ");
    Serial.println(jsonParseResult.type);        

  }
}