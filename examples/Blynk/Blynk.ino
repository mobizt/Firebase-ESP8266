
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

//This example shows the basic usage of Blynk platform and Firebase RTDB.

//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

#define FIREBASE_HOST "YOUR_FIREBASE_PROJECT.firebaseio.com"
#define FIREBASE_AUTH "YOUR_FIREBASE_DATABASE_SECRET"
#define WIFI_SSID "YOUR_WIFI_AP"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

//Debug Blynk to serial port
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

//Auth token for your Blynk app project
#define BLYNK_AUTH "YOUR_BLYNK_APP_PROJECT_AUTH_TOKEN"

//Define FirebaseESP8266 data objects
FirebaseData firebaseData1;
FirebaseData firebaseData2;



String path = "/Blynk_Test/Int";

//D4 or GPIO2 on Wemos D1 mini
uint8_t BuiltIn_LED = 2;


/*

Blynk app Widget setup 
**********************

1. Button Widget (Switch type), Output -> Virtual pin V1
2. LED Widget, Input -> Virtual pin V2

*/
WidgetLED led(V2);



void setup()
{

  Serial.begin(115200);

  pinMode(BuiltIn_LED, OUTPUT);

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


  if (!Firebase.beginStream(firebaseData1, path))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData1.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Blynk.begin(BLYNK_AUTH, WIFI_SSID, WIFI_PASSWORD);
}

void loop()
{
  Blynk.run();

  if (!Firebase.readStream(firebaseData1))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't read stream data...");
    Serial.println("REASON: " + firebaseData1.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  if (firebaseData1.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

  if (firebaseData1.streamAvailable())
  {
    Serial.println("------------------------------------");
    Serial.println("Stream Data available...");
    Serial.println("STREAM PATH: " + firebaseData1.streamPath());
    Serial.println("EVENT PATH: " + firebaseData1.dataPath());
    Serial.println("DATA TYPE: " + firebaseData1.dataType());
    Serial.println("EVENT TYPE: " + firebaseData1.eventType());
    Serial.print("VALUE: ");
    if (firebaseData1.dataType() == "int")
    {

      Serial.println(firebaseData1.intData());
      if (firebaseData1.intData() == 0)
      {
        digitalWrite(BuiltIn_LED, LOW);
        led.off();
      }
      else if (firebaseData1.intData() == 1)
      {
        digitalWrite(BuiltIn_LED, HIGH);
        led.on();
      }
    }
    Serial.println("------------------------------------");
    Serial.println();
  }
}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable

  Serial.println("------------------------------------");
  Serial.println("Set integer...");
  //Also can use Firebase.set instead of Firebase.setInt
  if (Firebase.setInt(firebaseData2, path, pinValue))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData2.dataPath());
    Serial.println("TYPE: " + firebaseData2.dataType());
    Serial.print("VALUE: ");
    if (firebaseData2.dataType() == "int")
      Serial.println(firebaseData2.intData());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData2.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}

