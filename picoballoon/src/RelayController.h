///
/// @file		RelayController.h
/// @brief		Library header
/// @details	<#details#>
/// @n
/// @n @b		Project pbf-2
/// @n @a		Developed with [embedXcode](http://embedXcode.weebly.com)
///
/// @author		Aykut Celik
/// @author		Aykut Celik
///
/// @date		13/01/2017 14:22
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

#ifndef RelayController_cpp
#define RelayController_cpp
#define CHARGEDURATION 30000
class RelayController{
public:
    void setup();
    void turnOn();
    void turnOff();
};

#endif // end RelayController_cpp
