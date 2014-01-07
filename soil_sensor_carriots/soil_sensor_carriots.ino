/*************************************************** 
  This is a sketch to interface a soil sensor & Carriots
  using the Adafruit CC3000 breakout board (or WiFi shield)
  
  Written by Marco Schwartz for Open Home Automation
 ****************************************************/

// Libraries
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"
#include "DHT.h"
#include<stdlib.h>
#include <Sensirion.h>

// Define CC3000 chip pins
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

// Soil sensor pins
const uint8_t dataPin  =  6;
const uint8_t clockPin =  7;

// Soil sensor variables
float t;
float h;
float dewpoint;

// Create soil sensor object
Sensirion soilSensor = Sensirion(dataPin, clockPin);

// Create CC3000 instances
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2); // you can change this clock speed
                                
// WLAN parameters
#define WLAN_SSID       "yourSSID"
#define WLAN_PASS       "yourPassword"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

// Xively parameters
#define WEBSITE  "api.carriots.com"
#define API_KEY "yourApiKey"
#define DEVICE  "yourDeviceName@yourUserName"

uint32_t ip;

void setup(void)
{
  // Initialize
  Serial.begin(115200);
  
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
 
}

void loop(void)
{
  // Connect to WiFi network
  cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }  
 
  // Get the website IP & print it
  ip = 0;
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  }
  cc3000.printIPdotsRev(ip);
  
  // Get data & transform to integers
  soilSensor.measure(&t, &h, &dewpoint);
 
  // Convert data to String
  String temperature = doubleToString(t,2);
  String humidity = doubleToString(h,2);
 
  // Prepare JSON for Xively & get length
  int length = 0;

  String data = "{\"protocol\":\"v2\",\"device\":\""+String(DEVICE)+"\",\"at\":\"now\",\"data\":{\"Temperature\":"+String(temperature)+",\"Humidity\":"+String(humidity)+"}}";
  
  length = data.length();
  Serial.print("Data length");
  Serial.println(length);
  Serial.println();
  
  // Print request for debug purposes
  Serial.println("POST /streams HTTP/1.1");
  Serial.println("Host: api.carriots.com");
  Serial.println("Accept: application/json");
  Serial.println("User-Agent: Arduino-Carriots");
  Serial.println("Content-Type: application/json");
  Serial.println("carriots.apikey: " + String(API_KEY));
  Serial.println("Content-Length: " + String(length));
  Serial.print("Connection: close");
  Serial.println();
  Serial.println(data);
  
  // Send request
  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);
  if (client.connected()) {
    Serial.println("Connected!");
    client.println("POST /streams HTTP/1.1");
    client.println("Host: api.carriots.com");
    client.println("Accept: application/json");
    client.println("User-Agent: Arduino-Carriots");
    client.println("Content-Type: application/json");
    client.println("carriots.apikey: " + String(API_KEY));
    client.println("Content-Length: " + String(length));
    client.println("Connection: close");
    client.println();
    
    client.println(data);
    
  } else {
    Serial.println(F("Connection failed"));    
    return;
  }
  
  Serial.println(F("-------------------------------------"));
  while (client.connected()) {
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
  }
  client.close();
  Serial.println(F("-------------------------------------"));
  
  Serial.println(F("\n\nDisconnecting"));
  cc3000.disconnect();
  
  // Wait 10 seconds until next update
  delay(10000);
   
}

// Convert double to string
String doubleToString(float input,int decimalPlaces){
  if(decimalPlaces!=0){
    String string = String((int)(input*pow(10,decimalPlaces)));
      if(abs(input)<1){
        if(input>0)
          string = "0"+string;
        else if(input<0)
          string = string.substring(0,1)+"0"+string.substring(1);
      }
      return string.substring(0,string.length()-decimalPlaces)+"."+string.substring(string.length()-decimalPlaces);
    }
  else {
    return String((int)input);
  }
}
