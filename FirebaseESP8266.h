/*
 * Google's Firebase real time database - ESP8266 library for Arduino, version 1.0
 * 
 * This library provides ESP8266 to perform REST API by GET PUT, POST, PATCH, DELETE data from/to with Google's Firebase database using get, set, update
 * and delete calls. 
 * 
 * The library was test and work well with ESP8266s based module and add support for multiple stream event path.
 * 
 * The MIT License (MIT)
 * Copyright (c) 2019 K. Suwatchai (Mobizt)
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
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
#ifndef FirebaseESP8266_H
#define FirebaseESP8266_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "FirebaseESP8266HTTPClient.h"


#define  FIEBASE_PORT 443

#define FIREBASE_REQ_BUFFER_SIZE 400
#define FIREBASE_RESPONSE_SIZE 400
#define FIREBASE_DATA_SIZE 400
#define FIREBASE_PATH_SIZE 400

#define HOST_BUF_SIZE 200
#define AUTH_BUF_SIZE 200
#define PUSH_NAME_BUF_SIZE 30

#define KEEP_ALIVE_TIMEOUT 30000



class FirebaseData;

class FirebaseESP8266 {
  public:

    typedef struct FirebaseDataType;
    typedef struct FirebaseMethod;

    FirebaseESP8266();
    ~FirebaseESP8266();

   /**
   * The Firebase's credentials initialization.
   * \param host - Your Firebase database project host i.e. Your_ProjectID.firebaseio.com.
   * \param auth - Your database secret.
   */
    void begin(const String host, const String auth);
	void begin(const char* host, const char* auth);
  
   /**
   * Auto reconnect WiFi when connection lost during request.
   * \param reconnect - True for auto reconnect.
   */
    void reconnectWiFi(bool reconnect);
  
   /**
   * Check for the existence of node path.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Node path to be checked.
   */
    bool pathExist(FirebaseData &dataObj, const String path);
    bool pathExist(FirebaseData &dataObj, const char* path);
  
   /**
   * Post or append/create new child node integer data.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Parent node path to be append.
   * \param intValue - Integer value to be append.
   * \return The operating status. True for success operation and False for failed operation.
   * To get new push node name, call FirebaseData.pushName().
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   */
    bool pushInt(FirebaseData &dataObj, const String path, int intValue);
    bool pushInt(FirebaseData &dataObj, const char* path, int intValue);
  
  /**
   * Post or append/create new child node float data.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Parent node path to be append.
   * \param floatValue - Float value to be append.
   * \return The operating status. TTrue for success operation and False for failed operation.
   * To get new push node name, call FirebaseData.pushName().
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   */
    bool pushFloat(FirebaseData &dataObj, const String path, float floatValue);
    bool pushFloat(FirebaseData &dataObj, const char* path, float floatValue);
  
  /**
   * Post or append/create new child node String data.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Parent node path to be append.
   * \param StringValue - String value to be append.
   * \return The operating status. True for success operation and False for failed operation.
   * To get new push node name, call FirebaseData.pushName().
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   */
    bool pushString(FirebaseData &dataObj, const String path, const String stringValue);
    bool pushString(FirebaseData &dataObj, const char* path, const char* stringValue);
  
  /**
   * Post or append/create new node JSON data.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Parent node path to be append.
   * \param jsonString - Raw JSON String to be append (should be correct JSONformat).
   * \return The operating status. True for success operation and False for failed operation.
   * To get new push node name, call FirebaseData.pushName().
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   */
    bool pushJSON(FirebaseData &dataObj, const String path, const String jsonString);
    bool pushJSON(FirebaseData &dataObj, const char* path, const char* jsonString);
  
  /**
   * Put or set integer data to child node at the specified path.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Node path to be set value.
   * \param intValue - Integer value to be set.
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   * To get actual payload data from operation, call FirebaseData.intData().
   */
    bool setInt(FirebaseData &dataObj, const String path, int intValue);
    bool setInt(FirebaseData &dataObj, const char* path, int intValue);
  
  /**
   * Put or set float data to node at the specified path.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Node path to be set value.
   * \param floatValue - Float value to be set.
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   * To get actual payload data from operation, call FirebaseData.floatData().
   */
    bool setFloat(FirebaseData &dataObj, const String path, float floatValue);
    bool setFloat(FirebaseData &dataObj, const char* path, float floatValue);
  
  /**
   * Put or set String data to child node at the specified path.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Node path to be set value.
   * \param stringValue - String value to be set.
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   * To get actual payload data from operation, call FirebaseData.stringData().
   */
    bool setString(FirebaseData &dataObj, const String path, const String stringValue);
    bool setString(FirebaseData &dataObj, const char* path, const char* stringValue);
  
  /**
   * Put or set JSON data to child node at the specified path.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Node path to be set value.
   * \param jsonString - Raw JSON string to be set (should be correct JSONformat).
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   * To get actual payload data from operation, call FirebaseData.jsonData().
   */
    bool setJSON(FirebaseData &dataObj, const String path, const String jsonString);
    bool setJSON(FirebaseData &dataObj, const char* path, const char* jsonString);
  
  /**
   * Patch or update JSON data to child node at the specified path.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Node path to be update the value.
   * \param jsonString - Raw JSON string to be update (should be correct JSONformat contains key/value pairs).
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   * To get actual payload data from operation, call FirebaseData.jsonData().
   */
    bool updateNode(FirebaseData &dataObj, const String path, const String jsonString);
    bool updateNode(FirebaseData &dataObj, const char* path, const char* jsonString);
  
  /**
   * Get the integer data from child node at the specified path.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Node path to be read the value.
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   * To get actual return integer data, call FirebaseData.intData(). If the data at node path is float type, 
   * the return data will be rounded  and if the data at node path is other types than integer or float, 
   * the return integer data will be zero.
   */
    bool getInt(FirebaseData &dataObj, const String path);
    bool getInt(FirebaseData &dataObj, const char* path);
  
  /**
   * Get the float data from child node at the specified path.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Node path to be read the value.
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   * To get actual return float data, call FirebaseData.floatData(). If the data at node path is other types than float or integer, 
   * the return float data will be zero.
   */
    bool getFloat(FirebaseData &dataObj, const String path);
    bool getFloat(FirebaseData &dataObj, const char* path);
  
  /**
   * Get the String data from child node at the specified path.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Node path to be read the value.
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   * To get actual return String data, call FirebaseData.stringData(). If the data at node path is other types than String, 
   * the return String data will be empty string.
   */
    bool getString(FirebaseData &dataObj, const String path);
    bool getString(FirebaseData &dataObj, const char* path);
  
   /**
   * Get the raw JSON String data from child node at the specified path.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Node path to be read the value.
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   * To get actual data type, call FirebaseData.dataType().
   * To get actual return raw JSON String data, call FirebaseData.jsonData(). If the data at node path is other types than JSON, 
   * the return String data will be empty string.
   */
    bool getJSON(FirebaseData &dataObj, const String path);
    bool getJSON(FirebaseData &dataObj, const char* path);
  
   /**
   * Delete the node at specified path. The entire children nodes also deleted.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Node path to be deleted.
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   */
    bool deleteNode(FirebaseData &dataObj, const String path);
    bool deleteNode(FirebaseData &dataObj, const char* path);
  
   /**
   * Begin monitoring the change of data at node and its children at specified path.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \param path - Parent node path to be monitor.
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   */
    bool beginStream(FirebaseData &dataObj, const String path);
    bool beginStream(FirebaseData &dataObj, const char* path);
  
   /**
   * Read the stream event data at specified path. Should be called in loop or loop tasks after begin stream called in setup().
   * \param dataObj - FirebaseData object that requred for internal works.
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   */
    bool readStream(FirebaseData &dataObj);
  
  /**
   * End the stream operation at specified path. Can be start stream again by call beginStream once.
   * \param dataObj - FirebaseData object that requred for internal works.
   * \return The operating status. True for success operation and False for failed operation.
   * To get error reason, call FirebaseData.errorReason().
   */
    bool endStream(FirebaseData &dataObj);
  
  /**
   * Use in FirebaseData class
   */
    void replace_char(char* str, char in, char out);
    void errorToString(int httpCode, char* buf);


  protected:
    void firebaseBegin(const char* host, const char* auth, uint16_t port);
    bool sendRequest(FirebaseData &dataObj, const char* path, const uint8_t _method, uint8_t dataType, const char* payload);
    bool firebaseConnectStream(FirebaseData &dataObj, const char* path);
    bool getServerStreamResponse(FirebaseData &dataObj);
    bool getServerResponse(FirebaseData &dataObj);

    void buildFirebaseRequest(FirebaseData &dataObj, const char* host, uint8_t _method, const char* path, const char* auth, const char* payload, char* request);
    void resetFirebasedataFlag(FirebaseData &dataObj);
    bool handleTCPNotConnected(FirebaseData &dataObj);
    void forceEndHTTP(FirebaseData &dataObj);
    int firebaseConnect(FirebaseData &dataObj, const char* path, const uint8_t _method, uint8_t dataType, const char* payload);
    bool cancelCurrentResponse(FirebaseData &dataObj);
    void setDataType(FirebaseData &dataObj, const char* response);

    void strcat_c (char *str, char c);
    int strpos(const char *haystack, const char *needle, int offset);
    int rstrpos(const char *haystack, const char *needle, int offset);
    char* rstrstr(const char* haystack, const char* needle);

    char _host[HOST_BUF_SIZE];
    char _auth[AUTH_BUF_SIZE];
    uint16_t _port;
    bool _reconnectWiFi;

};

class FirebaseData {

  public:
    FirebaseData();
  
   /**
   * Get refence to shared SSL WiFi client.
   */
   WiFiClientSecure getWiFiClient();
   
   /**
   * Pause/Unpause SSL WiFi client (WiFiClientSecure) from current Firebase call this allows you
   * to use shared SSL WiFi client.
   * \param pause True for pause and False for unpause
   * \return The operating status. True for success operation and False for failed operation.
   * Call FirebaseData.getWiFiClient to get shared WIFiClientSecure client.
   */
   bool pauseFirebase(bool pause);
   
   /**
   * Return the actual data type that return as payload from get/set/push calls.
   * \return The data type String (int, float, string and json).
   */
    String dataType();
	
    /**
   * Return the actual data type that return as payload from get/set/push calls.
   * \return The data type as FirebaseDataType struct (NULL_, INTEGER, FLOAT, STRING and JSON).
   */
    uint8_t dataTypeInt();
  
   /**
   * Return the current stream path.
   * \return The path String.
   */
    String streamPath();
	
   /**
   * Get the stream path string.
   */
    void streamPathBuf(char* buf);
  
   /**
   * Return the current data path.
   * \return The current path String from get/set/push calls or current changed data path from stream call.
   */
    String dataPath();
	
   /**
   * Get the current data path.
   */
    void dataPathBuf(char* buf);
  
   /**
   * Return the error reason String from current call.
   * \return The error String.
   */
    String errorReason();

   /**
   * Get the error reason string.
   */	
    void errorReasonBuf(char* buf);
  
   /**
   * Return the ineger data from current get/set/push and stream calls.
   * \return The payload or returned integer data.
   */
    int intData();
  
   /**
   * Return the float data from current get/set/push and stream calls.
   * \return The payload or returned float data.
   */
    float floatData();
  
   /**
   * Return the String data from current get/set/push and stream calls.
   * \return The payload or returned String data.
   */
    String stringData();
	
   /**
   * Get the string data from current get/set/push and stream calls.
   */	
    void stringDataBuf(char* buf);
  
   /**
   * Return the raw JSON String data from current get/set/push and stream calls.
   * \return The payload or returned raw JSON String data.
   */
    String jsonData();
	
   /**
   * Get the raw JSON string data from current get/set/push and stream calls.
   */	
    void jsonDataBuf(char* buf);
  
   /**
   * Return the new created child node name (String) from push call.
   * \return The String of new append node name.
   */
    String pushName();
	
   /**
   * Get the new created child node name from push call.
   */	
    void pushNameBuf(char* buf);
  
   /**
   * Check the current FirebaseData object is currently work with stream.
   * \return The status. True for being stream.
   */
    bool isStream();
  
   /**
   * Check the current FirebaseData object is currently connected to server.
   * \return The connected status. True for connected.
   */
    bool httpConnected();
  
   /**
   * Check the current stream connection of FirebaseData object is timeout from server (30 sec is default).
   * Nothing to do when timeout, the stream is automatic resume.
   * \return The timeout status. True for timeout occurred.
   */
    bool streamTimeout();
  
   /**
   * Check the data is available from get/set/push calls.
   * \return The data available status. True for data is available.
   */
    bool dataAvailable();
  
   /**
   * Check the update/changes data is available from current stream  of FirebaseData object.
   * \return The stream data available status. True for stream data is available.
   */
    bool streamAvailable();
  
   /**
   * Check the matching between data type being call and the server return data type.
   * \return The mismatch status. True for mismatch.
   */
    bool mismatchDataType();
  
   /**
   * Check the http code return from all calls.
   * \return The integer data of http response.
   */
    int httpCode();
  
   /**
   * Check overflow of the return data buffer.
   * \return The overflow status. True for overflow. If overflow change the defined value in FirebaseESP8266.h
   * #define FIREBASE_RESPONSE_SIZE xxxx
   * #define FIREBASE_DATA_SIZE xxxx
   */
  bool bufferOverflow();
  


  protected:
    bool _isStreamTimeout;
    bool _isStream;
    bool _streamStop;
    bool _bufferOverflow;
    bool _streamDataChanged;
    bool _streamPathChanged;
    bool _dataAvailable;
    bool _keepAlive;
    bool _httpConnected;
    bool _interruptRequest;
    bool _mismatchDataType;
    bool _pathNotExist;
	bool _pause;
    uint8_t _dataType;
    uint8_t _dataType2;
    uint8_t _connectionStatus;

    char _path[FIREBASE_PATH_SIZE];
    char _path2[FIREBASE_PATH_SIZE];
    char _data[FIREBASE_DATA_SIZE];
    char _data2[FIREBASE_DATA_SIZE];
    char _streamPath[FIREBASE_PATH_SIZE];
    char _pushName[PUSH_NAME_BUF_SIZE];

    int _httpCode;
    int _contentLength;

    unsigned long _dataMillis;
    unsigned long _streamMillis;
    unsigned long _streamResetMillis;
	

    FirebaseHTTPClient _http;	
	
	void removeDQ(char* str);
	
	

    friend FirebaseESP8266;


};


extern FirebaseESP8266 Firebase;

#endif
