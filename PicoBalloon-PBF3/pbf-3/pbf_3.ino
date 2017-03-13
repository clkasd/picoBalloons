//
// pbf-3
//
// Description of the project
// Developed with [embedXcode](http://embedXcode.weebly.com)
//
// Author 		Aykut Celik
// 				Aykut Celik
//
// Date			21/02/2017 14:47
// Version		<#version#>
//
// Copyright	© Aykut Celik, 2017
// Licence		<#licence#>
//
// See         ReadMe.txt for references
//


// Core library for code-sense - IDE-based
#if defined(ENERGIA) // LaunchPad specific
#include "Energia.h"
#elif defined(TEENSYDUINO) // Teensy specific
#include "Arduino.h"
#elif defined(ESP8266) // ESP8266 specific
#include "Arduino.h"
#elif defined(ARDUINO) // Arduino 1.8 specific
#include "Arduino.h"
#else // error
#error Platform not defined
#endif // end IDE
#include <Wire.h>
#include <SPI.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_BME280.h"// Set parameters
#include "APRSWrapper.h"
#include "SoftwareSerial.h"
#include "GPSManager.h"

#define SEALEVELPRESSURE_HPA (1013.25)
#define DEBUG
Adafruit_BME280 bme; // I2C
unsigned long delayTime;
float temp = 0;
float pressure = 0;
float altitude = 0;
float humidity = 0;
// Add setup code
APRSWrapper* wrapper;
GPSManager *gpsMan;

void printValues() {
    temp =bme.readTemperature();
    if(isnan(temp))
        temp=0;
    
    pressure = bme.readPressure() / 100.0F;
    if(isnan(pressure))
        pressure = 0;
    
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    if(isnan(altitude))
        altitude = 0;
    
    humidity = bme.readHumidity();
    if(isnan(humidity))
        humidity = 0;
    
    
    /*Serial.print("Temp : ");
    Serial.print((int)temp);
    Serial.print(" Hum : ");
    Serial.print((int)humidity);
    Serial.print(" Pres : ");
    Serial.print((int)pressure);
    Serial.print(" Alt : ");
    Serial.println((int)altitude);*/
}

void setup()
{
    /*
     pin allocations for bme280
     */
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    digitalWrite(10, HIGH);
    digitalWrite(11, LOW);
    
    
    Serial.begin(9600);
    Serial.println(F("PBF-3 Test"));
    bool status;
    status = bme.begin();
    if (!status) {
        delay(1000);
        setup();
    }
    
    wrapper = new APRSWrapper();
    
    gpsMan = new GPSManager();
    
    wrapper->ax25_init();
    
    wrapper->initialise_interrupt();
}
long startTime = -60000 ;

void loop()
{
    
    delay(100);
    printValues(); // get values from bme280
    
    gpsMan->sync();
    
    GPSFrame gpsFr = gpsMan->getGPSFrame();
    
    if((millis() - startTime) > 60000 )
    {
        startTime = millis();
        if(gpsFr.lock != 0)
        {
            //Creating APRSFrame struct
            APRSFrame frameToSend;
            frameToSend.APRSCallSign = CALLSIGN;
            frameToSend.APRSSSID = SSID;
            frameToSend.APRSComment = "http://picoballoon.ist PBF-3";
            frameToSend.temperature = (int)temp;
            frameToSend.humidity = (int)humidity;
            frameToSend.pressure = (int)pressure;
            frameToSend.WIDE1 = 1;
            frameToSend.WIDE2 = 1;
            frameToSend.lat = gpsFr.lat;
            frameToSend.lon = gpsFr.lon;
            frameToSend.sats = gpsFr.sats;
            frameToSend.alt = gpsFr.alt;
            
            wrapper->setFrame(frameToSend);
            wrapper->ax25_init();
            wrapper->tx_aprs();
            
#ifdef DEBUG
            Serial.println("Sent !");
#endif
        }
        else
        {
#ifdef DEBUG
            Serial.println("Cannot lock !");
#endif
        }
    }
}

















