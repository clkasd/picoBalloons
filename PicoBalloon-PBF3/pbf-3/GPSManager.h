///
/// @file		GPSManager.h
/// @brief		Library header
/// @details	<#details#>
/// @n
/// @n @b		Project pbf-2
/// @n @a		Developed with [embedXcode](http://embedXcode.weebly.com)
///
/// @author		Aykut Celik
/// @author		Aykut Celik
///
/// @date		06/01/2017 14:47
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
#include "SoftwareSerial.h"
#ifndef GPSManager_cpp
#define GPSManager_cpp
struct GPSFrame
{
    int32_t lat = 0, lon = 0, alt = 0, maxalt = 0;
    uint8_t lock =0, sats = 0, hour = 0, minute = 0, second = 0;
};
class GPSManager
{
public:
    GPSManager();
    void resetGPS();
    void sendUBX(uint8_t *MSG, uint8_t len);
    void setupGPS();
    uint8_t gps_check_nav(void);
    boolean getUBX_ACK(uint8_t *MSG);
    void setGps_MaxPerformanceMode();
    void setGPS_PowerSaveMode();
    void checkDynamicModel();
    void setGPS_GNSS();
    void setGPS_DynamicModel6();
    void gps_ubx_checksum(uint8_t* data, uint8_t len, uint8_t* cka,uint8_t* ckb);
    bool _gps_verify_checksum(uint8_t* data, uint8_t len);
    void gps_get_data();
    void setGPS_DynamicModel3();
    void init();
    void sync();
    void gps_get_position();
    void gps_get_time();
    GPSFrame getGPSFrame();
    void gps_check_lock();
private:
    uint8_t buf[60];
    int GPSerror = 0,navmode = 0,psm_status = 0,lat_int=0,lon_int=0;
    int32_t lat = 0, lon = 0, alt = 0, maxalt = 0, lat_dec = 0, lon_dec =0 ,tslf=0;
    int errorstatus=0;
    uint8_t lock =0, sats = 0, hour = 0, minute = 0, second = 0;
    uint8_t oldhour = 0, oldminute = 0, oldsecond = 0;
    
    volatile boolean lockvariables = 0;
    void prepare_data();
    /* Error Status Bit Level Field :
     Bit 0 = GPS Error Condition Noted Switch to Max Performance Mode
     Bit 1 = GPS Error Condition Noted Cold Boot GPS
     Bit 2 = DS18B20 temp sensor status 0 = OK 1 = Fault
     Bit 3 = Current Dynamic Model 0 = Flight 1 = Pedestrian
     Bit 4 = PSM Status 0 = PSM On 1 = PSM Off
     Bit 5 = Lock 0 = GPS Locked 1= Not Locked
     
     So error 8 means the everything is fine just the GPS is in pedestrian mode.
     Below 1000 meters the code puts the GPS in the more accurate pedestrian mode.
     Above 1000 meters it switches to dynamic model 6 i.e flight mode and turns the LED's off for additional power saving.
     So as an example error code 40 = 101000 means GPS not locked and in pedestrian mode.
     */
    
    
};


#endif // end GPSManager_cpp
