/*************************************************** 
  This is a sketch to test the soil sensor based
  on the SHT10 temperature & humidity sensor 
  
  Written by Marco Schwartz for Open Home Automation
 ****************************************************/

// Include Sensirion library
#include <Sensirion.h>

// Sensor pins
const uint8_t dataPin  =  6;
const uint8_t clockPin =  7;

// Variables for the temperature & humidity sensor
float temperature;
float humidity;
float dewpoint;

// Create sensor instance
Sensirion soilSensor = Sensirion(dataPin, clockPin);

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  // Make a measurement
  soilSensor.measure(&temperature, &humidity, &dewpoint);

  // Print results
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %");
  Serial.println("");
  
  // Wait 100 ms before next measurement
  delay(100);  
}
