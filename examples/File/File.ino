/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2020 mobizt
 *
*/

//This example shows how to store and read binary data from file on SD card to database.

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"

/** The database secret is obsoleted, please use other authentication methods, 
 * see examples in the Authentications folder. 
*/
#define FIREBASE_AUTH "DATABASE_SECRET"

//Define Firebase Data object
FirebaseData fbdo;

String path = "/Test";

File file;

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

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo.setResponseSize(1024);

  //Mount SD card
  if (!SD.begin(15))
  {
    Serial.println("SD Card mounted failed");
    return;
  }

  //Delete demo files
  if (SD.exists("/file1.txt"))
    SD.remove("/file1.txt");

  if (SD.exists("/file2.txt"))
    SD.remove("/file2.txt");

  if (SD.exists("/file3.txt"))
    SD.remove("/file3.txt");

  Serial.println("------------------------------------");
  Serial.println("Set file data test...");

  //Write demo data to file
  file = SD.open("/file1.txt", FILE_WRITE);
  uint8_t v = 0;
  for (int i = 0; i < 512; i++)
  {
    file.write(v);
    v++;
  }

  file.close();

  //In case of Root CA was set, set this option to false to disable low memory for secured mode BearSSL to support large file data
  //Firebase.lowMemBSSL(false);

  //Set file (read file from SD card and set to database)
  //File name must be in 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension)
  if (Firebase.setFile(fbdo, StorageType::SD, path + "/Binary/File/data", "/file1.txt"))
  {
    Serial.println("PASSED");
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.fileTransferError());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Serial.println("------------------------------------");
  Serial.println("Get file data test...");

  //Get file (download file to SD card)
  //File name must be in 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension)
  if (Firebase.getFile(fbdo, StorageType::SD, path + "/Binary/File/data", "/file2.txt"))
  {

    Serial.println("PASSED");
    Serial.println("DATA");

    //Readout the downloaded file
    file = SD.open("/file2.txt", FILE_READ);
    int i = 0;

    while (file.available())
    {
      if (i > 0 && i % 16 == 0)
        Serial.println();

      v = file.read();

      if (v < 16)
        Serial.print("0");

      Serial.print(v, HEX);
      Serial.print(" ");
      i++;
    }
    Serial.println();
    Serial.println("------------------------------------");
    Serial.println();
    file.close();
  }
  else
  {

    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.fileTransferError());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Serial.println("------------------------------------");
  Serial.println("Append file data test...");

  if (SD.exists("/file1.txt"))
    SD.remove("/file1.txt");

  //Write demo data to file
  file = SD.open("/file1.txt", FILE_WRITE);
  for (int i = 255; i >= 0; i--)
    file.write((uint8_t)i);

  file.close();

  //Append file data to database
  //File name must be in 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension)
  if (Firebase.pushFile(fbdo, StorageType::SD, path + "/Binary/File/Logs", "/file1.txt"))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("PUSH NAME: " + fbdo.pushName());
    Serial.println("------------------------------------");

    Serial.println();

    Serial.println("------------------------------------");
    Serial.println("Get appended file data test...");

    //Get the recently appended file (download file to SD card)
    //File name must be in 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension)
    if (Firebase.getFile(fbdo, StorageType::SD, path + "/Binary/File/Logs/" + fbdo.pushName(), "/file3.txt"))
    {

      Serial.println("PASSED");
      Serial.println("DATA");

      //Readout the downloaded file
      file = SD.open("/file3.txt", FILE_READ);
      int i = 0;
      int idx = 0;

      while (file.available())
      {
        i = file.read();
        if (i < 16)
          Serial.print("0");

        Serial.print(i, HEX);
        Serial.print(" ");
        
        if (idx > 0 && (idx + 1) % 16 == 0)
          Serial.println();
        idx++;
      }
      Serial.println();
      Serial.println("------------------------------------");
      Serial.println();
      file.close();
    }
    else
    {

      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.fileTransferError());
      Serial.println("------------------------------------");
      Serial.println();
    }
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.fileTransferError());
    Serial.println("------------------------------------");
    Serial.println();
  }
}

void loop()
{
}
