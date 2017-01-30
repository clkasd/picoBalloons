//
// pbf-2
//
// Description of the project
// Developed with [embedXcode](http://embedXcode.weebly.com)
//
// Author 		Aykut Celik
// 				Aykut Celik
//
// Date			03/01/2017 11:26
// Version		<#version#>
//
// Copyright	© Aykut Celik, 2017
// Licence		<#licence#>
//
// See         ReadMe.txt for references
//


// Core library for code-sense - IDE-based
#if defined(WIRING) // Wiring specific
#include "Wiring.h"
#elif defined(MAPLE_IDE) // Maple specific
#include "WProgram.h"
#elif defined(MPIDE) // chipKIT specific
#include "WProgram.h"
#elif defined(DIGISPARK) // Digispark specific
#include "Arduino.h"
#elif defined(ENERGIA) // LaunchPad specific
#include "Energia.h"
#elif defined(LITTLEROBOTFRIENDS) // LittleRobotFriends specific
#include "LRF.h"
#elif defined(MICRODUINO) // Microduino specific
#include "Arduino.h"
#elif defined(SPARK) || defined(PARTICLE) // Particle / Spark specific
#include "Arduino.h"
#elif defined(TEENSYDUINO) // Teensy specific
#include "Arduino.h"
#elif defined(REDBEARLAB) // RedBearLab specific
#include "Arduino.h"
#elif defined(ESP8266) // ESP8266 specific
#include "Arduino.h"
#elif defined(ARDUINO) // Arduino 1.0 and 1.5 specific
#include "Arduino.h"
#else // error
#error Platform not defined
#endif // end IDE

// Set parameters
#define VFACTOR 0.0795
//#define DEBUG //Comment out to disable debug
// local libraries

#include "APRSWrapper.h"
#include "Adafruit_MCP9808.h"
#include "SoftwareSerial.h"
#include "GPSManager.h"
#include "RelayController.h"
// objects needed
APRSWrapper* wrapper;
Adafruit_MCP9808* tempSensor;
GPSManager *gpsMan;
RelayController* relayCon;

float VFinal; //This will store the converted data
int VRaw; //This will store our raw ADC data


void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Start");
#endif
    // Setting up necessary pins
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    digitalWrite(10, HIGH);
    digitalWrite(11, LOW);
    
    //objects init
    wrapper = new APRSWrapper();
    tempSensor = new Adafruit_MCP9808();
    gpsMan = new GPSManager();
    
    wrapper->ax25_init();
    wrapper->initialise_interrupt();
    if(!tempSensor->begin())
    {
        setup();
#ifdef DEBUG
        Serial.println("Couldn't find MCP9808!");
        while (1);
#endif
    }
}
long startTime = -60000 ;

// Add loop code
void loop()
{
    gpsMan->sync();
    
    tempSensor->shutdown_wake(0);
    float c = tempSensor->readTempC();
#ifdef DEBUG
    Serial.print("Temp: "); Serial.println(c);
#endif
    delay(250);
    tempSensor->shutdown_wake(1);
    
    
    GPSFrame gpsFr = gpsMan->getGPSFrame();
    
    
    if((millis() - startTime) > 60000 )
    {
        startTime = millis();
        if(gpsFr.lock != 0)
        {
            //Creating APRSFrame struct
            APRSFrame frameToSend;
            frameToSend.APRSCallSign = "TA2MUN";
            frameToSend.APRSSSID = 11;
            frameToSend.APRSComment = "https://picoballoon.ist PBF-2";
            frameToSend.temperature = (int) c;
            frameToSend.battery = 1245; // TODO değiştirilecek
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

/*
void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Start");
#endif
    // Setting up necessary pins
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    digitalWrite(10, HIGH);
    digitalWrite(11, LOW);
    relayCon = new RelayController();
    relayCon->setup();
    relayCon->turnOn();
    //relayCon->turnOff();
#ifdef DEBUG
    Serial.println("Relay Turned Off");
    Serial.println("Starting Voltage");
    VRaw = analogRead(A0);
    VFinal = VRaw * (VFACTOR);
    Serial.println(VFinal);
#endif
    while(true)
    {
        delay(CHARGEDURATION);
        VRaw = analogRead(A0);
        VFinal = VRaw * (VFACTOR);
        if(VFinal > 4)
            break;
    }
    relayCon->turnOn();
#ifdef DEBUG
    Serial.println("Relay Turned On");
    Serial.println("Voltage");
    VRaw = analogRead(A0);
    VFinal = VRaw * (VFACTOR);
    Serial.println(VFinal);
    delay(2000);
#endif
    //objects init
    wrapper = new APRSWrapper();
    tempSensor = new Adafruit_MCP9808();
    gpsMan = new GPSManager();
    
    delay(1000);
#ifdef DEBUG
    Serial.println("Objects Init Finished");
    Serial.println("Voltage");
    VRaw = analogRead(A0);
    VFinal = VRaw * (VFACTOR);
    Serial.println(VFinal);
#endif
    
    wrapper->ax25_init();
    wrapper->initialise_interrupt();
    if(!tempSensor->begin())
    {
        setup();
#ifdef DEBUG
        Serial.println("Couldn't find MCP9808!");
#endif
    }
}


// Add loop code
void loop()
{
#ifdef DEBUG
    Serial.println("Loop Start");
    Serial.println("Voltage");
    VRaw = analogRead(A0);
    VFinal = VRaw * (VFACTOR);
    Serial.println(VFinal);
#endif
    gpsMan->sync();
    
    tempSensor->shutdown_wake(0);
    float c = tempSensor->readTempC();
#ifdef DEBUG
    Serial.print("Temp: "); Serial.println(c);
#endif
    delay(250);
    tempSensor->shutdown_wake(1);
    
    GPSFrame gpsFr = gpsMan->getGPSFrame();
    
#ifdef DEBUG
    Serial.println("GPS Stats");
    Serial.println("Lat  -  Lon  -  Sats  -  Alt");
    Serial.print(gpsFr.lat);
    Serial.print(gpsFr.lon);
    Serial.print(gpsFr.sats);
    Serial.println(gpsFr.alt);
#endif
    //Creating APRSFrame struct
    APRSFrame frameToSend;
    frameToSend.APRSCallSign = "TA2MUN";
    frameToSend.APRSSSID = 9;
    frameToSend.APRSComment = "test";
    frameToSend.temperature = (int) c;
    frameToSend.battery = 1245; // TODO değiştirilecek
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
    Serial.println("APRS Package Sent !");
    Serial.println("Waiting before turnoff");
#endif
    delay(10000);
    relayCon->turnOff();
#ifdef DEBUG
    Serial.println("Relay Turned Off");
    Serial.println("Voltage");
    VRaw = analogRead(A0);
    VFinal = VRaw * (VFACTOR);
    Serial.println(VFinal);
    Serial.println("Charging Started");
#endif
    while(true)
    {
        delay(CHARGEDURATION);
        VRaw = analogRead(A0);
        VFinal = VRaw * (VFACTOR);
        if(VFinal > 4)
        {
            relayCon->turnOn();
            delay(500);
            //gpsMan->init();
            break;
        }
    }
}*/






























