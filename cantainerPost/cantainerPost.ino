#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <Phant.h>


//#define WIFISSID "PacificPenguin"
//#define PASSWORD "patientpanda$$9"

#define WIFISSID "SPDL-Air"
#define PASSWORD "Irf9Z.34N"

#define NUM_FIELDS 4

char server[] = "data.sparkfun.com";    // name address for data.sparkFun (using DNS)

// phant stuffs
const String publicKey = "1nwWg1x2GzIlzKwLQ07z";
const String privateKey = "0mYp7qx4n0i4kXq1lavk";

String fieldNames[NUM_FIELDS] = {"name", "weight", "cal_per_g", "id"};
String fieldData[NUM_FIELDS];


String foodId = "-1";


ESP8266WiFiMulti WiFiMulti;
HTTPClient http;

Phant phant("data.sparkfun.com", publicKey, privateKey);


int getFieldIndex(String field) {
  int f = 0;
  for(f=0; f<NUM_FIELDS; f++) {
    if (field == fieldNames[f]) 
      return f;
  }

  // no match, return -1
  if (f + 1 == NUM_FIELDS && field != fieldNames[f]) {
     return -1;
  } 
}


String getFieldData(String field) {

  String data = "";
  int index = getFieldIndex(field);
  if (index > -1) {
    data = fieldData[index];
  }
  
  return data;

}

void setFieldData(String field, String data) {
  int index = getFieldIndex(field); 
  if(index >= 0) {
    fieldData[index] = data;
  }
  
}

String getStreamPath() {
  String path = "/output/";
  path += publicKey;
  path += ".json";
  
  return path;
}

String createPostPath() {
  
  String path = "/input/";
  path += publicKey;
  path += "?private_key=";
  path += privateKey;


  for (int i=0; i<NUM_FIELDS; i++)
  {
    path += "&";
    path += fieldNames[i];
    path += "=";
    path += fieldData[i];
  }

  return path;

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  WiFiMulti.addAP(WIFISSID, PASSWORD);

  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting...");
  }

  Serial.println("Connected");  

}

String getLatestJsonEntry(String payload) {
    int startJson = payload.indexOf("{");
    int endJson = payload.indexOf("}");

    String json = payload.substring(startJson, endJson);
    return json;
}

String getValueFromLatestJson(String field, String json) {

  String latest = json.substring(json.indexOf(field) + field.length() + 2); // adding chars : and "
  latest = latest.substring(0, latest.indexOf(","));
  
  latest.replace("\"", "");
  latest.trim();
  return latest;
}


void postFoodData() {
    String postPath = createPostPath();
    Serial.println("POSTING........");
    Serial.println(postPath);
      
    http.begin(server, 80, postPath);
    int httpCode = http.GET();
  
    if(httpCode == 200) {
      String payload = http.getString();
      Serial.println(payload);
    }
    http.end();
  
}


void loop() {
  // put your main code here, to run repeatedly:
  String lastFoodId = getFieldData("id");

  String path = getStreamPath();
  http.begin(server, 80, path);

  Serial.println(path);
  
  int httpCode = http.GET();
  if(httpCode == 200) {
      String payload = http.getString();
    
      // hacky way to get json values
      // TO DO: USE A REAL JSON LIBRARY
 
      String json = getLatestJsonEntry(payload);
 
      String foodId = getValueFromLatestJson(String("id"), json);
      Serial.print("Latest food id: ");
      Serial.println(foodId);
     
      if(foodId != lastFoodId) {

          // TO DO: FILL IN WITH UPDATING FOOD VALUES ON SCREEN
          Serial.println("changed");
          String foodName = getValueFromLatestJson(String("name"), json);
          String foodId = getValueFromLatestJson(String("id"), json);
          String foodCals = getValueFromLatestJson(String("cal_per_g"), json);

          Serial.print("Now the jar holds food: ");
          Serial.println(foodName);

      }

      
   }
   http.end();




  setFieldData("id", "808");
  setFieldData("name", "nachos");
  setFieldData("weight", "200");
  setFieldData("cal_per_g", "200");
  //postFoodData();
  

  delay(10000);

}
