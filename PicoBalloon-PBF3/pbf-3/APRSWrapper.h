///
/// @file		APRSWrapper.h
/// @brief		Library header
/// @details	<#details#>
/// @n
/// @n @b		Project pbf-2
/// @n @a		Developed with [embedXcode](http://embedXcode.weebly.com)
///
/// @author		Aykut Celik
/// @author		Aykut Celik
///
/// @date		03/01/2017 13:16
/// @version	<#version#>
///
/// @copyright	(c) Aykut Celik, 2017
/// @copyright	<#licence#>
///
/// @see		ReadMe.txt for references
///


// Core library for code-sense - IDE-based
#if defined(WIRING) // Wiring specific
#include "Wiring.h"
#elif defined(MAPLE_IDE) // Maple specific
#include "WProgram.h"
#elif defined(ROBOTIS) // Robotis specific
#include "libpandora_types.h"
#include "pandora.h"
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
#elif defined(TEENSYDUINO) // Teensy specific
#include "Arduino.h"
#elif defined(REDBEARLAB) // RedBearLab specific
#include "Arduino.h"
#elif defined(RFDUINO) // RFduino specific
#include "Arduino.h"
#elif defined(SPARK) // Spark specific
#include "application.h"
#elif defined(ARDUINO) // Arduino 1.0 and 1.5 specific
#include "Arduino.h"
#else // error
#error Platform not defined
#endif // end IDE

#ifndef APRSWrapper_cpp
#define APRSWrapper_cpp
#define CALLSIGN "CHANGEME"
#define SSID "11"
struct APRSFrame
{
    int WIDE1 = 0;
    int WIDE2 = 0;
    double lat = 0;
    double lon = 0;
    int alt = 0;
    int battery = 0;
    int sats = 0;
    int temperature = 0;
    int humidity = 0;
    int pressure = 0;
    
    char* APRSCallSign = new char[7];
    char APRSSSID = 0;
    char* APRSComment = "";
};


class APRSWrapper
{
public:
    void ax25_frame(char *scallsign, char sssid, char *dcallsign, char dssid, char ttl1, char ttl2, char *data, ...);
    void ax25_init(void);
    void tx_aprs();
    void initialise_interrupt();
    void setFrame(APRSFrame fr);
    APRSFrame getFrame();
    void sendParameters();
    int aprstxstatus = 0;
private:
    APRSFrame frame;
    uint16_t seq = 0;
};





















#endif // end APRSWrapper_cpp
