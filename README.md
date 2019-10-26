# Firebase Realtime Database Arduino Library for ESP8266


Google's Firebase Realtime Database Arduino Library for ESP8266 v 2.6.3


This library supports ESP8266 MCU from Espressif. The following are platforms which library are also available.

* [ESP32 Firebase Arduino library]( https://github.com/mobizt/Firebase-ESP32)

* [Arduino MKR WiFi 1010, Arduino MKR VIDOR 4000 and Arduino UNO WiFi Rev.2](https://github.com/mobizt/Firebase-Arduino-WiFiNINA)

* [Arduino WiFi Shield 101 and Arduino MKR1000 WIFI](https://github.com/mobizt/Firebase-Arduino-WiFi101)




## Tested Devices

 * Wemos D1 Mini
 * NodeMCU


## Features



* **Supports Read (get), Store (set), Append (push), Patch (update) and Delete Data**

* **Supports Primitive data types: Integer, Float, Double, Boolean, String and JSON.**

* **Supports BLOB and File Stream Data.**

* **Support Read and Write Database Rules.**

* **Supports ETag, Priority, Data Limits, Timestamp, Filtering, etc.**

* **Supports Stream Event Callbacks**

* **Supports Data Backup and Restore.**

* **Supports Firebase Cloud Messaging.**

* **Supports SD and SPIFFS's CA certificate file (for Core SDK v2.5.x).**

* **Built-in easiest JSON parser and builder.**


## Changes from earlier version

For library v 2.6.3 (comes with FirebaseJson v 2.2.3) or later, FirebaseJson object will be used to handle JSON data instead of JSON string which, the following functions are affected:

getJson, setJson, pushJson, updateNode and updateNodeSilent.



## Dependencies


This library required **ESP8266 Core SDK version 2.4.0 and above**.

For Arduino IDE, ESP8266 Core SDK can be installed through **Boards Manager**. 

For PlatfoemIO IDE, ESP8266 Core SDK can be installed through **PIO Home** > **Platforms** > **Espressif 8266**.




## Installation


### Using Library Manager


At Arduino IDE, go to menu **Sketch** -> **Include Library** -> **Manage Libraries...**

In Library Manager Window, search **"firebase"** in the search form then select **"Firebase ESP8266 Client"**. 

Click **"Install"** button.



For PlatformIO IDE, using the following command.

**pio lib install "Firebase ESP8266 Client""**

Or at **PIO Home** -> **Library** -> **Registry** then search **Firebase ESP8266 Client**.

[More on PlatformIO...](https://platformio.org/lib/show/6247/Firebase%20ESP8266%20Client)


### Manual installation


For Arduino IDE, download zip file from repository (Github page) by select **Clone or download** dropdown at the top of repository, select **Download ZIP** 

From Arduino IDE, select menu **Sketch** -> **Include Library** -> **Add .ZIP Library...**.

Choose **Firebase-ESP8266-master.zip** that previously downloaded.

Go to menu **Files** -> **Examples** -> **Firebase-ESP8266-master** and choose one from examples.


For PlatformIO, in folder **"lib"**, create new folder named **"Firebase-ESP8266"** and add **[these files](https://github.com/mobizt/Firebase-ESP8266/tree/master/src)** in that folder.






## Usages


### Initialization


```C++
//1. Include Firebase ESP8266 library (this library)
#include "FirebaseESP8266.h"

//2. Include ESP8266WiFi.h and must be included after FirebaseESP8266.h
#include <ESP8266WiFi.h>

//3. Declare the Firebase Data object in global scope
FirebaseData firebaseData;

//4. Setup Firebase credential in setup()
Firebase.begin("yout_project_id.firebaseio.com", "your_Firebase_database_secret");

//5. Optional, set AP reconnection in setup()
Firebase.reconnectWiFi(true);

//6. Optional, set number of error retry
 Firebase.setMaxRetry(firebaseData, 3);

//7. Optional, set number of error resumable queues
Firebase.setMaxErrorQueue(firebaseData, 30);

//8. Optional, use classic HTTP GET and POST requests.  
Firebase.enableClassicRequest(firebaseData, true);
```


### Read Data


Data at specific node in Firebase RTDB can be read through these get functions.

The functions included `get`, `getInt`, `getFlot`, `getDouble`, `getBool`, `getString`, `getJSON`, `getArray`, `getBlob`, `getFile`.


These functions return boolean value indicates the success of operation which will be `true` if all of the following conditions were met.

* Server returns HTTP status 200

* The data types matched between request and response.



The database data's payload (response) can be read or access through the following Firebase Data object's functions.

* `firebaseData.intData`

* `firebaseData.floatData`

* `firebaseData.doubleData`

* `firebaseData.boolData`

* `firebaseData.stringData`

* `firebaseData.jsonString`

* `firebaseData.jsonObject`

* `firebaseData.jsonObjectPtr`

* `firebaseData.jsonArray` 

* `firebaseData.jsonArrayPtr`

* `firebaseData.jsonData` (for keeping parse/get result)

and

* `firebaseData.blobData`


Read the data which its type is not match the data type in database from above functions will return empty (string, object or array).


The data type of returning payload can be determined by `firebaseData.getDataType`.


BLOB and file stream data are store as special base64 encode string which only supported and implemented by this library.


Thhe encoded string will be prefixed with some header string ("file,base64," and "blob,base64,") for data type manipulation. 


The following example showed how to read integer value from "/test/int".


```C++
  if (Firebase.getInt(firebaseData, "/test/int")) {

    if (firebaseData.dataType() == "int")) {
      Serial.println(firebaseData.intData());
    }

  } else {
    Serial.println(firebaseData.errorReason());
  }
```



### Store Data

To store data at specific node in Firebase RTDB, use these set functions.

The function included `set`, `setInt`, `setFlot`, `setDouble`, `setBool`, `setString`, `setJSON`, `setArray`, `setBlob` and `setFile`. 


The above functions return boolean value indicates the success of operation which will be `true` if all of the following conditions matched.

* Server returns HTTP status 200

* The data types matched between request and response.


Only setBlob and setFile functions that make a silent request to Firebase server, thus no payload response returned. 

The **priority**, virtual node **".priority"** of each database node can be set through Firebase's set functions.

The priority value can be used in query or filtering the children data under defined database path.

**ETag** (unique identifier value) assigned to Firebase's set functions is used as conditional checking.

If defined Etag is not match the defined path's ETag, the set operation will fail with result **412 Precondition Failed**.

ETag at any database path can be read through `Firebase.getETag`.  ETag value changed upon the data was set or delete.

The server's **Timestamp** can be store in database through `Firebase.setTimestamp`. 

The returned **Timestamp** value can get from `firebaseData.getInt()`. 

The following example showed how to store file data to Flash memory at "/test/file_data".


```C++

if (Firebase.setFile(firebaseData, StorateType::SPIFFS, "/test/file_data", "/test.txt"))
{
  //SPIFFS.begin(); //not need to begin again due to it has been called in function.
  File file = SPIFFS.open("/test.txt", "r");

  while (file.available())
  {     
    Serial.print(file.read(), HEX);     
  }    
  file.close();
  Serial.println();

} else {
  Serial.println(firebaseData.fileTransferError());
}
```



### Append Data

To append new data to specific node in Firebase RTDB, use these push functions.

The function included `push`, `pushInt`, `pushFlot`, `pushDouble`, `pushBool`, `pushString`, `pushJSON`, `pushArray`, `pushBlob`, and `pushFile`.

These functions return boolean value indicates the success of operation.

The **unique key** of new appended node can be determined from `firebaseData.pushName`.

As get functions, the Firebase's push functions support **priority**.

**ETag** was not available after push unless read the **ETag** at that new appended unique key later with `Firebase.getETag`.

The server's **Timestamp** can be append in database through `Firebase.pushTimestamp`.

The unique key of Timestamp can be determined after Timestamp was appended.


The following example showed how to append new data (using FirebaseJson object) to "/test/append.


```C++

FirebaseJson json;
FirebaseJson json2;

json2.set("child_of_002", 123.456);
json.set("parent_001", "parent 001 text");
json.set("parent 002", json2);

if (Firebase.pushJSON(firebaseData, "/test/append", json)) {

  Serial.println(firebaseData.dataPath());

  Serial.println(firebaseData.pushName());

  Serial.println(firebaseData.dataPath() + "/"+ firebaseData.pushName());

} else {
  Serial.println(firebaseData.errorReason());
}
```



### Patch Data

Firebase's update functions used to pach or update new or existing data at defined database path.

These functions, `updateNode` and `updateNodeSilent` are available and work with JSON object (FirebaseJson object only)

If any key provided at defined database path in JSON object was not existed, new key will be created.

The server returns JSON data payload which was successfully patched.

Return of large JSON payload will cost the network data, alternative function `updateNodeSilent` should be used to save the network data.


The following example showed how to patch data at "/test".


```C++

FirebaseJson updateData;
FirebaseJson json;
json.set("_data2","_value2");
updateData.set("data1","value1");
updateData.set("data2", json);

if (Firebase.updateNode(firebaseData, "/test/update", updateData)) {

  Serial.println(firebaseData.dataPath());

  Serial.println(firebaseData.dataType());

  Serial.println(firebaseData.jsonString()); 

} else {
  Serial.println(firebaseData.errorReason());
}
```





### Delete Data


The following example showed how to delete data and its childs at "/test/append"

```C++
Firebase.deleteNode(firebaseData, "/test/append");
```





### Filtering Data

To filter or query the data, the following quey parameters are available through the QueryFilter class.

These parameters are `orderBy`, `limitToFirst`, `limitToLast`, `startAt`, `endAt`, and `equalTo`.

To filter data, parameter `orderBy` should be assigned.

Use **"$key"** as `orderBy` parameter if the key of child nodes was used for query.

Use **"$value"** as `orderBy` parameter if the value of child nodes was used for query.

Use **key (or full path) of child nodes** as `orderBy` parameter if all values of specific key were used for query.

Use **"$priority"** as `orderBy` parameter if child nodes's **"priority"** was used for query.



The above `orderBy` parameter can be combined with the following parameters for limited and ranged the queries.

`QueryFilter.limitToFirst` -  The total children (number) to filter from the first child.

`QueryFilter.limitToLast` -   The total last children (number) to filter. 

`QueryFilter.startAt` -       Starting value of range (number or string) of query upon orderBy param.

`QueryFilter.endAt` -         Ending value of range (number or string) of query upon orderBy param.

`QueryFilter.equalTo` -       Value (number or string) matches the orderBy param



The following example showed how to use queries parameter in QueryFilter class to filter the data at "/test/data"

```C++
//Assume that children that have key "sensor" are under "/test/data"

//Instantiate the QueryFilter class
QueryFilter query;

//Build query using specified child node key "sensor" under "/test/data"
query.orderBy("sensor");

//Query any child that its value is begin with 2 (number), assumed that its data type is float or integer
query.startAt(2);

//Query any child that its value is end with 8 (number), assumed that its data type is float or integer
query.endAt(8);

//Limit the maximum query result to return only the last 5 nodes
query.limitToLast(5);


if (Firebase.getJSON(firebaseData, "/test/data", query))
{
  //Success, then try to read the JSON payload value
  Serial.println(firebaseData.jsonString());
}
else
{
  //Failed to get JSON data at defined database path, print out the error reason
  Serial.println(firebaseData.errorReason());
}

//Clear all query parameters
query.clear();
```



### Stream Data



This library uses HTTP GET request with stream header to connect the stream.

The Firebase's functions that involved the stream operation are `beginStream`, `setStreamCallback` and/or `readStream`.

Function `beginStream` is to subscribe the stream changes at defined database path.

Function `setStreamCallback` is to assign the callback function that accept the **StreamData** class as parameter.

The **StreamData** contains stream event and data and interface function calls are similar to Firebase Data object.

To check the stream manually, use `readStream`.

Function `readStream` used in loop() task to continuous read the stream changes event and data.

After `readStream`, determine the availability of stream with Firebase Data object function `firebaseData.streamAvailable` 

Function `firebaseData.streamAvailable` returned true when new stream data was available. 

When new stream data was available, its data and event can be accessed from Firebase Data object functions.


* `firebaseData.intData`

* `firebaseData.floatData`

* `firebaseData.doubleData`

* `firebaseData.boolData`

* `firebaseData.stringData`

* `firebaseData.jsonString`

* `firebaseData.jsonObject`

* `firebaseData.jsonObjectPtr`

* `firebaseData.jsonArray` 

* `firebaseData.jsonArrayPtr`

* `firebaseData.jsonData` (for keeping parse/get result)

and

* `firebaseData.blobData`


Function `endStream` ends the stream operation.



The following example showed how to subscribe the stream changes at "/test/data" with callback function.

```C++

//In setup(), set the stream callback function to handle data
//streamCallback is the function that called when database data changes or updates occurred
//streamTimeoutCallback is the function that called when connection between server 
//and client was timeout during HTTP stream

Firebase.setStreamCallback(firebaseData, streamCallback, streamTimeoutCallback);

//In setup(), set the streaming path to "/test/data" and begin stream connection

if (!Firebase.beginStream(firebaseData, "/test/data"))
{
  //Could not begin stream connection, then print out the error detail
  Serial.println(firebaseData.errorReason());
}

  
  //Global function that handle stream data
void streamCallback(StreamData data)
{

  //Print out all information

  Serial.println("Stream Data...");
  Serial.println(data.streamPath());
  Serial.println(data.dataPath());
  Serial.println(data.dataType());

  //Print out value
  //Stream data can be many types which can be determined from function dataType

  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(), 5);
  else if (data.dataType() == "double")
    printf("%.9lf\n", data.doubleData());
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
    Serial.println(data.jsonString());

}

//Global function that notify when stream connection lost
//The library will resume the stream connection automatically
void streamTimeoutCallback(bool timeout)
{
  if(timeout){
    //Stream timeout occurred
    Serial.println("Stream timeout, resume streaming...");
  }  
}

```

The following example showed how to subscribe the stream changes at "/test/data" and read the stream manually.

```C++
//In setup(), set the streaming path to "/test/data" and begin stream connection
if (!Firebase.beginStream(firebaseData, "/test/data"))
{
  Serial.println(firebaseData.errorReason());
}

//In loop()
if (!Firebase.readStream(firebaseData))
{
  Serial.println(firebaseData.errorReason());
}

if (firebaseData.streamTimeout())
{
  Serial.println("Stream timeout, resume streaming...");
  Serial.println();
}

if (firebaseData.streamAvailable())
{

  if (firebaseData.dataType() == "int")
    Serial.println(firebaseData.intData());
  else if (firebaseData.dataType() == "float")
    Serial.println(firebaseData.floatData(), 5);
  else if (firebaseData.dataType() == "double")
    printf("%.9lf\n", firebaseData.doubleData());
  else if (firebaseData.dataType() == "boolean")
    Serial.println(firebaseData.boolData() == 1 ? "true" : "false");
  else if (firebaseData.dataType() == "string")
    Serial.println(firebaseData.stringData());
  else if (firebaseData.dataType() == "json")
    Serial.println(firebaseData.jsonString());
    
}
```



### Backup and Restore Data


This library allows you to backup and restore database at the definded path.

The backup file will store in SD card or Flash memory (SPIFFS).

Due to SD library used, only 8.3 DOS format file name was support.

The maximum 8 characters for file name and 3 characters for file extension.

The database restoration returned completed status only when Firebase server successfully update the data. 

Any failed operation will not affected the database (no updates or changes).

The following example showed how to backup all database at "/" and restore.

```C++
 String backupFileName = "";

 if (!Firebase.backup(firebaseData, StorateType::SD, "/", "/backup.txt"))
 {
   Serial.println(firebaseData.fileTransferError());
 }
 else
 {
   Serial.println(firebaseData.getBackupFilename());
   Serial.println(firebaseData.getBackupFileSize());
   backupFileName = firebaseData.getBackupFilename();
  }


  //Begin restore backed dup data back to database
  if (!Firebase.restore(firebaseData, StorateType::SD, "/", backupFileName))
  {
    Serial.println(firebaseData.fileTransferError());
  }
  else
  {
    Serial.println(firebaseData.getBackupFilename());
  }
```


### Database Error Handling

When read store, append and update operations were failed due to buffer overflow and network problems.

These operations can retry and queued after the retry amount was reach maximum retry set in function `setMaxRetry`.

```C++
//set maximum retry amount to 3
 Firebase.setMaxRetry(firebaseData, 3);
```

The function `setMaxErrorQueue` limits the maximum queues in Error Queue collection.

The full of queue collection can be checked through function `isErrorQueueFull`.


```C++
 //set maximum queues to 10
 Firebase.setMaxErrorQueue(firebaseData, 10);

 //determine whether Error Queue collection is full or not
 Firebase.isErrorQueueFull(firebaseData);
```

This library provides two approaches to run or process Error Queues with two functions. 

* `beginAutoRunErrorQueue`
* `processErrorQueue`

The function `beginAutoRunErrorQueue` will run or process queues automatically and can be call once. 

While function `processErrorQueue` will run or process queues and should call inside the loop().

With function `beginAutoRunErrorQueue`, you can assigned callback function that accept **QueueInfo** object as parameter.

Which contains all information about being processed queue, number of remaining queues and Error Queue collection status.

Otherwise, Error Queues can be trackacked manually with the following functions.

Function `getErrorQueueID` will return the unsigned integer presents the id of queue which will keep to use later.

Use `getErrorQueueID` and `isErrorQueueExisted` to check whether this queue id is still existed or not. 

If Error Queue ID is not existed in Error Queues collection, that queue is already done.

The following example showed how to run Error Queues automatically and track the status with callback function.

```C++

//In setup()

//Set the maximum Firebase Error Queues in collection (0 - 255).
//Firebase read/store operation causes by network problems and buffer overflow will be 
//added to Firebase Error Queues ollection.
Firebase.setMaxErrorQueue(firebaseData, 10);

//Begin to run Error Queues in Error Queue collection  
Firebase.beginAutoRunErrorQueue(firebaseData, callback);


//Use to stop the auto run queues
//Firebase.endAutoRunErrorQueue(firebaseData);

void errorQueueCallback (QueueInfo queueinfo){

  if (queueinfo.isQueueFull())
  {
    Serial.println("Queue is full");
  }

  Serial.print("Remaining queues: ");
  Serial.println(queueinfo.totalQueues());

  Serial.print("Being processed queue ID: ");
  Serial.println(queueinfo.currentQueueID());  

  Serial.print("Data type:");
  Serial.println(queueinfo.dataType()); 

  Serial.print("Method: ");
  Serial.println(queueinfo.method());

  Serial.print("Path: ");
  Serial.println(queueinfo.path());

  Serial.println();
}
```

The following example showed how to run Error Queues and track its status manually.

```C++
//In setup()

//Set the maximum Firebase Error Queues in collection (0 - 255).
//Firebase read/store operation causes by network problems and buffer overflow will be added to 
//Firebase Error Queues ollection.
Firebase.setMaxErrorQueue(firebaseData, 10);


//All of the following are in loop()

Firebase.processErrorQueue(firebaseData);

//Detrnine the queue status
if (Firebase.isErrorQueueFull(firebaseData))
{
  Serial.println("Queue is full");
}

//Remaining Error Queues in Error Queue collection
Serial.print("Remaining queues: ");
Serial.println(Firebase.errorQueueCount(firebaseData));

//Assumed that queueID is unsigned integer array of queue that added to Error Queue collection 
//when error and use Firebase.getErrorQueueID to get this Error Queue id.

for (uint8_t i = 0; i < LENGTH_OF_QUEUEID_ARRAY; i++)
{
  Serial.print("Error Queue ");
  Serial.print(queueID[i]);
  if (Firebase.isErrorQueueExisted(firebaseData, queueID[i]))
    Serial.println(" is queuing");
  else
    Serial.println(" is done");
}
Serial.println();
```


Error Queus can be saved as file in SD card or Flash memory with function `saveErrorQueue`.

Error Queues store as file can be restored to Error Queue collection with function `restoreErrorQueue`.

Two types of storage can be assigned with these functions, `StorageType::SPIFFS` and `StorageType::SD`.

Read data (get) operation is not support queues restore

The following example showed how to restore and save Error Queues in /test.txt file.

```C++
//To restore Error Queues

if (Firebase.errorQueueCount(firebaseData, "/test.txt", StorageType::SPIFFS) > 0)
{
    Firebase.restoreErrorQueue(firebaseData, "/test.txt", StorageType::SPIFFS);
    Firebase.deleteStorageFile("/test.txt", StorageType::SPIFFS);
}

//To save Error Queues to file
Firebase.saveErrorQueue(firebaseData, "/test.txt", StorageType::SPIFFS);

```




## Firebase Cloud Messaging (FCM)

There are two types of FCM message data that can be sent using this library e.g. **notification** and **custom data**.

These two types data can send all together or separately.

Function `Firebase.sendMessage` will send message to one recipient.

Function `Firebase.broadcastMessage` will broadcast or send message to multiple recipients.  

Function `Firebase.sendTopic` will send message to any recipient who subscribed to the topic.

The FCM message itself offers broad range of messaging options and capabilities for various recipient device platforms. 

For Android, iOS and web platforms, these basic options can be set and work for all platforms. 


Function `firebaseData.fcm.begin` used to assign server key of your Firebase project.

Function `firebaseData.fcm.addDeviceToken` used to add recipient registered device token which want to send message to. 

Functions `firebaseData.fcm.removeDeviceToken` and `firebaseData.fcm.clearDeviceToken` used to remove or clear recipient device.


For the notification message, title, body, icon (optional), and click_action (optional) can be set through `firebaseData.fcm.setNotifyMessage`. 

And clear these notify message data with `firebaseData.fcm.clearNotifyMessage`.

For the data message, provide your custom data as JSON object (FirebaseJson object or string) to `firebaseData.fcm.setDataMessage` which can be clear with `firebaseData.fcm.clearDataMessage`.

The other options are `priority`, `collapse key`, `Time to Live` of message and `topic` to send message to, can be set from the following functions.

Call `firebaseData.fcm.setPriority` for priority ("normal" or "high"), `firebaseData.fcm.setCollapseKey` for collapse key setup, `firebaseData.fcm.setTimeToLive` for life span of message setup between 0 sec. to 2,419,200 sec.  (or 4 weeks), and `firebaseData.fcm.setTopic` for assigning the topic that message to send to.


The following example showed how to send FCM message.

```C++
//Provide your Firebase project's server key here
firebaseData.fcm.begin(FIREBASE_FCM_SERVER_KEY);

//Prvide one or more the recipient registered token or instant ID token
firebaseData.fcm.addDeviceToken(FIREBASE_FCM_DEVICE_TOKEN);

//Provide the priority (optional)
firebaseData.fcm.setPriority("normal");

//Provide the time to live (optional)
firebaseData.fcm.setTimeToLive(5000);

//Set the notification message data
firebaseData.fcm.setNotifyMessage("Notification", "Hello World!", "firebase-logo.png", "http://www.google.com");

//Set the custom message data
firebaseData.fcm.setDataMessage("{\"myData\":\"myValue\"}");

//Send message to one recipient with inddex 1 (index starts from 0)
if (Firebase.sendMessage(firebaseData, 1))
{
  //Success, print the result returned from server
  Serial.println(firebaseData.fcm.getSendResult());
}
else
{
  //Failed, print the error reason
  Serial.println(firebaseData.errorReason());
}
```



## Parse, Create and Edit JSON Objects


This library includes FirebaseJson Arduino library, the easiest JSON parser, builder and editor.

FirebaseJson usages are so simple as you read, store and update(edit) the JSON node in Firebase RTDB.

Since you declare the FirebaseJson (object) or FirebaseJsonArray, use the functions `setJsonData`, `add`, `set` and `remove`
to build or edit JSON object and use `get` to parse the node's contents. 

Defined the relative path of the specific node to `add`, `set`, `remove` and `get` functions to add, set, remove and get its contents.


Function `FirebaseJson.setJsonData` is to set the JSON string to JSON object.


Function `FirebaseJson.add` is used for add the new node with the contents e.g. String, Number (int and double), Boolean, Array and Object to the defined relative path.


Function `FirebaseJson.set` is used for edit, overwrite, create new (if not exist) node with contents e.g. String, Number (int and double), Boolean, Array and Object at the defined relative path.


Function `FirebaseJson.remove` is used for remove node and all its children contents at the defined relative path. 


Function `FirebaseJson.toString` is used for (pretty or plain) print out the JSON object as Arduino string (this function takes String param).


Functions `FirebaseJson.iteratorBegin`, `FirebaseJson.iteratorGet` and `FirebaseJson.iteratorEnd` are used for parse all JSON object contents as list which can be iterated with index.


Function `FirebaseJson.clear` is used for clear JSON object contents.


Function `FirebaseJsonArray.add` is used for adding the new contents e.g. String, Number (int and double), Boolean, Array and Object to JSON array.


Function `FirebaseJsonArray.set` is for edit, overwrite, create new (if not exist) contents e.g. String, Number (int and double), Boolean, Array and Object at the defined relative path or defined index of JSON array.



Function `FirebaseJsonArray.remove` is used for remove array's contents at the defined relative path or defined index of JSON array.



Function `FirebaseJsonArray.toString` is used for (pretty or plain) print out the JSON array object as Arduino string (this function takes String param).


Function `FirebaseJsonArray.clear` is used for clear JSON object contents.


To acquired the JSON object or JSON Array from FirebaseData object which returned from get, set, push operations, these following functions are required.

`FirebaseData.jsonObject`

`FirebaseData.jsonObjectPtr`

`FirebaseData.jsonArray` and

`FirebaseData.jsonArrayPtr`

Function `FirebaseData.jsonObject` and `FirebaseData.jsonObjectPtr` will provide FirebaseJson (object) and FirebaseJson pointer respectively.

Function `FirebaseData.jsonArray` and `FirebaseData.jsonArrayPtr` will provide FirebaseJson Array and FirebaseJson Array pointer respectively.



The following example shows how to use FirebaseJson.

```C++
//Declare FirebaseJson object (global or local)
FirebaseJson json;

//Add key1 with value 100 to JSON object
json.add("name", "Living Room");

//Add temp1 with value 120 and temp1 with 40 to JSON object
//Note: temp2 is not the child of temp1 as in previous version.
json.add("temp1", 120).add("temp2", 40);

//Add nested child contents directly
json.set("unit/temp1", "Farenheit");
json.set("unit/temp2", "Celcius");

//To print out as prettify string
String jsonStr;
json.toString(jsonStr, true);
Serial.println(jsonStr);

/*
This is the result of above code

{
    "name": "Living Room",
    "temp1": 120,
    "temp2": 40,
    "unit": {
        "temp1": "Farenheit",
        "temp2": "Celcius"
    }
}
*/

//To set array to the above JSON using FirebaseJson directly
//Set (add) array indexes 0,1,2,5,7 under temp1, the original value will be replaced with new one.
json.set("temp1/[0]", 47);
json.set("temp1/[1]", 28);
json.set("temp1/[2]", 34);
json.set("temp1/[5]", 23); //null will be created at array index 3,4 due to it's not yet assigned
json.set("temp1/[7]", 25); //null will be created at array index 6

//Print out as prettify string
json.toString(jsonStr, true);
Serial.println(jsonStr);

/*
The result of above code

{
    "name": "Living Room",
    "temp1": [
        47,
        28,
        34,
        null,
        null,
         23,
        null,
        25
     ],
    "temp2": 40,
    "unit": {
        "temp1": "Farenheit",
        "temp2": "Celcius"
    }
 }
*/

//Try to remove temp1 array at index 1
json.remove("temp1/[1]");

//Try to remove temp2
json.remove("temp2");

//Print out as prettify string
json.toString(jsonStr, true);
Serial.println(jsonStr);

/*
The result of above code

{
    "name": "Living Room",
    "temp1": [
         47,
         34,
         null,
         null,
         23,
         null,
         25
    ],
    "unit": {
        "temp1": "Farenheit",
        "temp2": "Celcius"
    }
}
*/

//Now parse/read the contents from specific node unit/temp2
//FirebaseJsonData is required to keep the parse results which can be access later
FirebaseJsonData jsonData;

json.get(jsonData, "unit/temp2");

if (jsonData.success)
{
  //Print type of parsed data e.g string, int, double, bool, object, array, null and undefined
  Serial.println(jsonData.type);
  //Print its content e.g.string, int, double, bool whereas object, array and null also can access as string
  Serial.println(jsonData.stringValue);
  //Serial.println(jsonData.intValue);
  /Serial.println(jsonData.boolValue);
  //Serial.println(jsonData.doubleValue);
}

//The above code will show
/*
string
Celcius
*/

//To get the array temp from FirebaseJson

json.get(jsonData, "temp1");

//Prepare FirebaseJsonArray to take the array from FirebaseJson
FirebaseJsonArray myArr;

//Get array data
jsonData.getArray(myArr);

//Call get with FirebaseJsonData to parse the array at defined index i
for (size_t i = 0; i < myArr.size(); i++)
{
  //jsonData now used as temporary object to get the parse results
  myArr.get(jsonData, i);

  //Print its value
  Serial.print("Array index: ");
  Serial.print(i);
  Serial.print(", type: ");
  Serial.print(jsonData.type);
  Serial.print(", value: ");
  Serial.println(jsonData.stringValue);
}

/*
The result of above code
Array index: 0, type: int, value: 47
Array index: 1, type: int, value: 34
Array index: 2, type: null, value: null
Array index: 3, type: null, value: null
Array index: 4, type: int, value: 23
Array index: 5, type: null, value: null
Array index: 6, type: int, value: 25
*/
 



```


The following example shows how to use FirebaseJsonArray.

```C++
//Declare FirebaseJsonArray object (global or local)
FirebaseJsonArray arr;

//Add some data
arr.add("banana");
arr.add("mango");
arr.add("coconut");


//Change the array contents
arr.set("[1]/food", "salad");
arr.set("[1]/sweet", "cake");
arr.set("[1]/appetizer", "snack");
arr.set("[2]", "apple"); // or arr.set(2, "apple");
arr.set("[4]/[0]/[1]/amount", 20);

//Print out array as prettify string
String arrStr;
arr.toString(arrStr, true);
Serial.println(arrStr);

/*
This is the result of above code

[
    "banana",
    {
        "food": "salad",
        "sweet": "cake",
        "appetizer": "snack"
    },
    "apple",
    null,
    [
        [
            null,
            {
                "amount": 20
            }
        ]
    ]
]
*/

//Remove array content at /4/0/1/amount
arr.remove("[4]/[0]/[1]/amount");

//Print out as prettify string
arr.toString(arrStr, true);
Serial.println(arrStr);

/*
The result of above code

[
    "banana",
    {
        "food": "salad",
        "sweet": "cake",
        "appetizer": "snack"
    },
    "apple",
    null,
    [
        [
            null
        ]
    ]
]

*/

//Now parse/read the array contents at some index

FirebaseJsonData jsonData;

arr.get(jsonData, "[1]/food");

if(jsonData.success)
{
  //Type of parsed data
  Serial.println(jsonData.type);
  //Its value
  Serial.println(jsonData.stringValue);
  //Serial.println(jsonData.intValue);
  //Serial.println(jsonData.boolValue);
  //Serial.println(jsonData.doubleValue);

}

//The above code will show
/*
string
salad
*/


//To get the JSON object at array index 1 from FirebaseJsonArray
arr.get(jsonData, "[1]");// or arr.get(jsonData, 1);

//Prepare FirebaseJson to take the JSON object from FirebaseJsonArray
FirebaseJson myJson;

//Get FirebaseJson data
jsonData.getJSON(myJson);

//Parse the JSON object as list
//Get the number of items
size_t len = myJson.iteratorBegin();
String key, value = "";
int type = 0;
for (size_t i = 0; i < len; i++)
{
  //Get the item at index i, whereas key and value are the returned data
  myJson.iteratorGet(i, type, key, value);
  //Print the data
  Serial.print(i);
  Serial.print(", ");
  Serial.print("Type: ");
  Serial.print(type == JSON_OBJECT ? "object" : "array");
  if (type == JSON_OBJECT)
  {
    Serial.print(", Key: ");
    Serial.print(key);
  }
  Serial.print(", Value: ");
  Serial.println(value);
}
//Clear all list to free memory
myJson.iteratorEnd();


/*
The result of above code

0, Type: object, Key: food, Value: salad
1, Type: object, Key: sweet, Value: cake
2, Type: object, Key: appetizer, Value: snack

*/


```





See [Full Examples](/examples) for complete usages.

See [Function Description](/src/README.md) for all available functions.

[ESP8266 (ESP32) with Firebase realtime database: IoT Controlled RGB LEDs](https://www.javacodegeeks.com/2019/07/esp8266-esp32-firebase-realtime-database-iot.html)

[Serverless IoTs with Firebase Realtime Database and ESP8266 - Part 1](https://medium.com/@vibrologic/serverless-iots-with-firebase-realtime-database-and-esp8266-9937d98d1ae0)

[Serverless IoTs with Firebase Realtime Database and ESP8266 - Part 2](https://medium.com/@vibrologic/serverless-iots-with-firebase-realtime-database-and-esp8266-e624304c3197)

## License

The MIT License (MIT)

Copyright (c) 2019 K. Suwatchai (Mobizt)


Permission is hereby granted, free of charge, to any person returning a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

