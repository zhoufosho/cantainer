/*********************************************************************
*
*
*
*
*********************************************************************/

#include <string.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <Phant.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_VCNL4010.h"
#include "HX711.h"

#define WIFISSID "SPDL-Air"
#define PASSWORD "Irf9Z.34N"

#define WEIGHT_CHANGE_THRESHOLD 100

#define NUM_FIELDS 4

#define ARDUINO_SIGNAL "arduino"
#define WEB_SIGNAL "web"

/* wifi stuff */

ESP8266WiFiMulti WiFiMulti;
HTTPClient http;

char server[] = "data.sparkfun.com";    // name address for data.sparkFun (using DNS)

/* phant stuffs */
const String publicKey = "1nwWg1x2GzIlzKwLQ07z";
const String privateKey = "0mYp7qx4n0i4kXq1lavk";

String fieldNames[NUM_FIELDS] = {"name", "weight", "cal_per_g", "id"};
String fieldData[NUM_FIELDS];

String foodId = "";

Phant phant("data.sparkfun.com", publicKey, privateKey);


/* oled stuff */

#define OLED_RESET 2
Adafruit_SSD1306 display(OLED_RESET);


#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

/* vcnl4010 */
Adafruit_VCNL4010 vcnl;

#define PROX_THRESHOLD 3000


/* hx711 */

#define DOUT  12
#define CLK  14

HX711 scale(DOUT, CLK);

float calibration_factor = -470; //-210000 / 453.59 grams in a pound;

float currentWeight = 0;


/* bitmap stuff */


#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


void printScaleCalibration() {

  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 1);
  Serial.print(" lbs"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  Serial.print(" calibration_factor: ");
  Serial.println(calibration_factor);
  
  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 5;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 5;
  }

}

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

  // hacky way to get signals sent by web
  path += "?grep[sender]=";
  path += WEB_SIGNAL;
  
  return path;
}



String createPostPath(String fieldName) {
  
  String path = "/input/";
  path += publicKey;
  path += "?private_key=";
  path += privateKey;

  /* creates blanks for all other fields */
  for (int i=0; i<NUM_FIELDS; i++)
  {
    path += "&";
    path += fieldNames[i];
    if(fieldNames[i] == fieldName){
      path += "=";
      path += fieldData[i];
    }
  }

  path += "&sender=";
  path += ARDUINO_SIGNAL;

  return path;

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


void postFoodData(String fieldName) {
    String postPath = createPostPath(fieldName);
    Serial.println("POSTING........");
    Serial.println(postPath);
      
    http.begin(server, 80, postPath);
    int httpCode = http.GET();
  
    if(httpCode == 200) {
      String payload = http.getString();
      Serial.print("POST SUCCESS! ");
      Serial.println(payload);
    }
    http.end();
  
}

void handSensedHandler() {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Hand sensed");
    display.display();
    delay(400);
    display.clearDisplay();
    display.display();
}


void printStandardDisplay() {
  /* TO DO: PRETTIFY THIS DISPLAY */
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(getFieldData("name"));
  Serial.println(getFieldData("name"));


  float calPerG = getFieldData("cal_per_g").toFloat();
  float calories = currentWeight * calPerG;

  if(currentWeight < 1) {
    display.print("0");
  } else {
    display.print(currentWeight);    
    calories = 0;
  }
  display.println(" grams");

  display.print(calories);
  display.println(" calories");
  
  display.display();
}

void setup()   {                
  Serial.begin(9600);
  
  /* OLED display */
  Serial.println("starting oled...");
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)  
  display.clearDisplay();

  /* set up vcnl4010 */
  Serial.println("VCNL4010 setup");
  if (! vcnl.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }
  Serial.println("Found VCNL4010");

  /* set up wifi */
  WiFiMulti.addAP(WIFISSID, PASSWORD);
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting...");
  }

  Serial.println("Connected");  

  /* set up load cell */
  scale.set_scale();
  scale.tare();  //Reset the scale to 0

  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);

  /* set up initial weight */
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  currentWeight = scale.get_units();
}

void updateFoodValuesFromServer(String json) {
    String foodName, foodId, foodCals;
    
    Serial.println("changed");
    foodName = getValueFromLatestJson(String("name"), json);
    foodId = getValueFromLatestJson(String("id"), json);
    foodCals = getValueFromLatestJson(String("cal_per_g"), json);
  
    setFieldData("name", foodName);
    setFieldData("id", foodId);
    setFieldData("cal_per_g", foodCals);
  
    Serial.print("Now the jar holds food: ");
    Serial.println(foodName); 
}


void loop() {

  /* proximity stuff */
  
  int prox = vcnl.readProximity();
  Serial.print("Prox: "); Serial.println(prox);

  if(prox > PROX_THRESHOLD) {
      // text display tests
      handSensedHandler();
  }


  /* getting data from server */
  String path = getStreamPath();
  http.begin(server, 80, path);
  
  int httpCode = http.GET();
  if(httpCode == 200) {
      String payload = http.getString();
    
      // hacky way to get json values
      String json = getLatestJsonEntry(payload);
      String foodId = getValueFromLatestJson(String("id"), json);
      Serial.print("Latest food id: "); Serial.println(foodId);

      String lastFoodId = getFieldData("id");

     
      if(foodId != lastFoodId) {
         updateFoodValuesFromServer(json);
         
         // TO DO: FILL IN WITH UPDATING FOOD VALUES ON SCREEN
         
      }

      
   }
   http.end();


  /* scale */
  scale.set_scale(calibration_factor);
  float weight = scale.get_units();
  
  if(abs(currentWeight - weight) >= WEIGHT_CHANGE_THRESHOLD) {
    Serial.print("CHANGED!!!! NEW WEIGHT: ");
    Serial.println(weight);
    currentWeight = weight;
    setFieldData("weight", String(weight));
    postFoodData("weight");
  }
  
  
  /* display */
  printStandardDisplay();
  delay(400);
  display.clearDisplay();
  
}


void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  for (uint8_t i=0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }    
  display.display();
  delay(1);
}

void testdrawcircle(void) {
  for (int16_t i=0; i<display.height(); i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
    display.display();
    delay(1);
  }
}

void testfillrect(void) {
  uint8_t color = 1;
  for (int16_t i=0; i<display.height()/2; i+=3) {
    // alternate colors
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, color%2);
    display.display();
    delay(1);
    color++;
  }
}
//
//void testdrawtriangle(void) {
//  for (int16_t i=0; i<min(display.width(),display.height())/2; i+=5) {
//    display.drawTriangle(display.width()/2, display.height()/2-i,
//                     display.width()/2-i, display.height()/2+i,
//                     display.width()/2+i, display.height()/2+i, WHITE);
//    display.display();
//    delay(1);
//  }
//}
//
//void testfilltriangle(void) {
//  uint8_t color = WHITE;
//  for (int16_t i=min(display.width(),display.height())/2; i>0; i-=5) {
//    display.fillTriangle(display.width()/2, display.height()/2-i,
//                     display.width()/2-i, display.height()/2+i,
//                     display.width()/2+i, display.height()/2+i, WHITE);
//    if (color == WHITE) color = BLACK;
//    else color = WHITE;
//    display.display();
//    delay(1);
//  }
//}

void testdrawroundrect(void) {
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, WHITE);
    display.display();
    delay(1);
  }
}

void testfillroundrect(void) {
  uint8_t color = WHITE;
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
    delay(1);
  }
}
   
void testdrawrect(void) {
  for (int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display();
    delay(1);
  }
}

void testdrawline() {  
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);
  
  display.clearDisplay();
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);
  
  display.clearDisplay();
  for (int16_t i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, WHITE); 
    display.display();
    delay(1);
  }
  delay(250);
}

void testscrolltext(void) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.clearDisplay();
  display.println("scroll");
  display.display();
  delay(1);
 
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);    
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
}
