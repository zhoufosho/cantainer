/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>

#define DHTPIN 2     // GPIO 2 pin of ESP8266
#define DHTTYPE DHT22   // DHT 22  (AM2302)

const char* ssid     = "PacificPenguin";
const char* password = "patientpanda$$9";


#define NUM_FIELDS 4

char server[] = "data.sparkfun.com";    // name address for data.sparkFun (using DNS)

const char* host = "data.sparkfun.com";
const char* publicKey = "1nwWg1x2GzIlzKwLQ07z";
const char* privateKey = "0mYp7qx4n0i4kXq1lavk";
const String fieldNames[NUM_FIELDS] = {"cal_per_g", "id", "name", "weight"};
String fieldData[NUM_FIELDS];


void setup() {
  Serial.begin(115200);
 
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("ESP8266 Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected"); 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  delay(5000);

  Serial.print("connecting to ");
  Serial.println(host);
 
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
 
  // We now create a URI for the request
  String url = "/input/";
  url += publicKey;
  url += "?private_key=";
  url += privateKey;


 fieldData[0] = "blah";
  fieldData[1] = "cookie";
  fieldData[2] = "blah";
  fieldData[3] = "200";

  for (int i=0; i<NUM_FIELDS; i++)
    {
      url += "&";
      url += fieldNames[i];
      url += "=";
      url += fieldData[i];
    }
 
  Serial.print("Requesting URL: ");
  Serial.println(url);
 
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);
 
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
 
  Serial.println();
  Serial.println("closing connection");
  delay(600000); // Send data every 10 minutes
} 
