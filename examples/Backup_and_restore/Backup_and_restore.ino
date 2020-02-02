/*
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2019 mobizt
 *
 * 
 * This example is for FirebaseESP8266 Arduino library v 2.7.7 or later
*/

//This example shows how to backup and restore database data



//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>

#define FIREBASE_HOST "YOUR_FIREBASE_PROJECT.firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH "YOUR_FIREBASE_DATABASE_SECRET"
#define WIFI_SSID "YOUR_WIFI_AP"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

//Define Firebase Data object
FirebaseData firebaseData;

void setup()
{

  Serial.begin(115200);

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

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData.setResponseSize(1024);

  Serial.println("------------------------------------");
  Serial.println("Backup test...");

  //Download and save data to SD card.
  //{TARGET_NODE_PATH} is the full path of database to backup and restore.
  //{FILE_NAME} is file name in 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension)

  if (!Firebase.backup(firebaseData, StorageType::SD, "/{TARGET_NODE_PATH}", "/{FILE_NAME}"))
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.fileTransferError());
	Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("PASSED");
    Serial.println("BACKUP FILE: " + firebaseData.getBackupFilename());
    Serial.println("FILE SIZE: " + String(firebaseData.getBackupFileSize()));
	Serial.println("------------------------------------");
    Serial.println();
  }

  Serial.println("------------------------------------");
  Serial.println("Restore test...");

  //Restore data to defined database path using backup file on SD card.
  //{TARGET_NODE_PATH} is the full path of database to restore
  //{FILE_NAME} is file name in 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension)

  if (!Firebase.restore(firebaseData, StorageType::SD, "/{TARGET_NODE_PATH}", "/{FILE_NAME}"))
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.fileTransferError());
	Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("PASSED");
    Serial.println("BACKUP FILE: " + firebaseData.getBackupFilename());
	Serial.println("------------------------------------");
    Serial.println();
  }
}

void loop()
{
}
