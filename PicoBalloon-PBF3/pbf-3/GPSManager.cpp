//
// GPSManager.cpp
// Library C++ code
// ----------------------------------
// Developed with embedXcode
// http://embedXcode.weebly.com
//
// Project 		pbf-2
//
// Created by 	Aykut Celik, 06/01/2017 14:47
// 				Aykut Celik
//
// Copyright 	(c) Aykut Celik, 2017
// Licence		<#licence#>
//
// See 			GPSManager.h and ReadMe.txt for references
//


// Library header
#include "GPSManager.h"
#include "SoftwareSerial.h"
SoftwareSerial gpsSer(13, 12); // RX, TX
GPSManager::GPSManager()
{
    gpsSer.begin(9600);
    init();
}
void GPSManager::init()
{
    resetGPS();
    setupGPS();
}
void wait(unsigned long delaytime) // Arduino Delay doesn't get CPU Speeds below 8Mhz
{
    unsigned long _delaytime=millis();
    while((_delaytime+delaytime)>=millis()){
    }
}
void GPSManager::resetGPS() {
    uint8_t set_reset[] = {
        0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0xFF, 0x87, 0x00, 0x00, 0x94, 0xF5           };
    sendUBX(set_reset, sizeof(set_reset)/sizeof(uint8_t));
}
void GPSManager::sendUBX(uint8_t *MSG, uint8_t len) {
    gpsSer.flush();
    gpsSer.write(0xFF);
    wait(100);
    for(int i=0; i<len; i++) {
        gpsSer.write(MSG[i]);
    }
}
void GPSManager::setupGPS() {
    // Turning off all GPS NMEA strings apart on the uBlox module
    // Taken from Project Swift (rather than the old way of sending ascii text)
    int gps_set_sucess=0;
    uint8_t setNMEAoff[] = {
        0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x80, 0x25, 0x00, 0x00, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xA9           };
    sendUBX(setNMEAoff, sizeof(setNMEAoff)/sizeof(uint8_t));
    while(!gps_set_sucess)
    {
        sendUBX(setNMEAoff, sizeof(setNMEAoff)/sizeof(uint8_t));
        gps_set_sucess=getUBX_ACK(setNMEAoff);
        if(!gps_set_sucess)
        {
            //blinkled(2);
        }
    }
    wait(500);
    setGPS_GNSS();
    wait(500);
    setGPS_DynamicModel6();
    wait(500);
    setGps_MaxPerformanceMode();
    wait(500);
}
uint8_t GPSManager::gps_check_nav(void)
{
    uint8_t request[8] = {
        0xB5, 0x62, 0x06, 0x24, 0x00, 0x00, 0x2A, 0x84           };
    sendUBX(request, 8);
    
    // Get the message back from the GPS
    gps_get_data();
    
    // Verify sync and header bytes
    if( buf[0] != 0xB5 || buf[1] != 0x62 ){
        GPSerror = 41;
    }
    if( buf[2] != 0x06 || buf[3] != 0x24 ){
        GPSerror = 42;
    }
    // Check 40 bytes of message checksum
    if( !_gps_verify_checksum(&buf[2], 40) ) {
        GPSerror = 43;
    }
    
    
    // Return the navigation mode and let the caller analyse it
    navmode = buf[8];
}
void GPSManager::checkDynamicModel() {
    if(alt<=1000&&sats>4) {
        if(navmode != 3)
        {
            setGPS_DynamicModel3();
            errorstatus |=(1 << 3);  // Set Bit 3 indicating we are in pedestrian mode
        }
    }
    else
    {
        if(navmode != 6){
            setGPS_DynamicModel6();
            errorstatus &= ~(1 << 3); // Unset bit 3 indicating we are in flight mode
            
        }
    }
}
void GPSManager::setGPS_PowerSaveMode() {
    // Power Save Mode
    uint8_t setPSM[] = {
        0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x01, 0x22, 0x92           }; // Setup for Power Save Mode (Default Cyclic 1s)
    sendUBX(setPSM, sizeof(setPSM)/sizeof(uint8_t));
}
void GPSManager::setGps_MaxPerformanceMode() {
    //Set GPS for Max Performance Mode
    uint8_t setMax[] = {
        0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x00, 0x21, 0x91           }; // Setup for Max Power Mode
    sendUBX(setMax, sizeof(setMax)/sizeof(uint8_t));
}
boolean GPSManager::getUBX_ACK(uint8_t *MSG) {
    uint8_t b;
    uint8_t ackByteID = 0;
    uint8_t ackPacket[10];
    unsigned long startTime = millis();
    
    // Construct the expected ACK packet
    ackPacket[0] = 0xB5;	// header
    ackPacket[1] = 0x62;	// header
    ackPacket[2] = 0x05;	// class
    ackPacket[3] = 0x01;	// id
    ackPacket[4] = 0x02;	// length
    ackPacket[5] = 0x00;
    ackPacket[6] = MSG[2];	// ACK class
    ackPacket[7] = MSG[3];	// ACK id
    ackPacket[8] = 0;		// CK_A
    ackPacket[9] = 0;		// CK_B
    
    // Calculate the checksums
    for (uint8_t ubxi=2; ubxi<8; ubxi++) {
        ackPacket[8] = ackPacket[8] + ackPacket[ubxi];
        ackPacket[9] = ackPacket[9] + ackPacket[8];
    }
    
    while (1) {
        
        // Test for success
        if (ackByteID > 9) {
            // All packets in order!
            return true;
        }
        
        // Timeout if no valid response in 3 seconds
        if (millis() - startTime > 3000) {
            return false;
        }
        
        // Make sure data is available to read
        if (gpsSer.available()) {
            b = gpsSer.read();
            
            // Check that bytes arrive in sequence as per expected ACK packet
            if (b == ackPacket[ackByteID]) {
                ackByteID++;
            }
            else {
                ackByteID = 0;	// Reset and look again, invalid order
            }
            
        }
    }
}
void GPSManager::setGPS_GNSS()
{
    // Sets CFG-GNSS to disable everything other than GPS GNSS
    // solution. Failure to do this means GPS power saving
    // doesn't work. Not needed for MAX7, needed for MAX8's
    int gps_set_sucess=0;
    uint8_t setgnss[] = {
        0xB5, 0x62, 0x06, 0x3E, 0x2C, 0x00, 0x00, 0x00,
        0x20, 0x05, 0x00, 0x08, 0x10, 0x00, 0x01, 0x00,
        0x01, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00,
        0x01, 0x01, 0x03, 0x08, 0x10, 0x00, 0x00, 0x00,
        0x01, 0x01, 0x05, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x01, 0x01, 0x06, 0x08, 0x0E, 0x00, 0x00, 0x00,
        0x01, 0x01, 0xFC, 0x11   };
    while(!gps_set_sucess)
    {
        sendUBX(setgnss, sizeof(setgnss)/sizeof(uint8_t));
        gps_set_sucess=getUBX_ACK(setgnss);
    }
}
void GPSManager::setGPS_DynamicModel6()
{
    int gps_set_sucess=0;
    uint8_t setdm6[] = {
        0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
        0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC           };
    while(!gps_set_sucess)
    {
        sendUBX(setdm6, sizeof(setdm6)/sizeof(uint8_t));
        gps_set_sucess=getUBX_ACK(setdm6);
    }
}
bool GPSManager::_gps_verify_checksum(uint8_t* data, uint8_t len)
{
    uint8_t a, b;
    gps_ubx_checksum(data, len, &a, &b);
    if( a != *(data + len) || b != *(data + len + 1))
        return false;
    else
        return true;
}
void GPSManager::gps_ubx_checksum(uint8_t* data, uint8_t len, uint8_t* cka,
                      uint8_t* ckb)
{
    *cka = 0;
    *ckb = 0;
    for( uint8_t i = 0; i < len; i++ )
    {
        *cka += *data;
        *ckb += *cka;
        data++;
    }
}
void GPSManager::gps_get_data()
{
    gpsSer.flush();
    // Clear buf[i]
    for(int i = 0;i<60;i++)
    {
        buf[i] = 0; // clearing buffer
    }
    int i = 0;
    unsigned long startTime = millis();
    
    while ((i<60) && ((millis() - startTime) < 1000) ) {
        if (gpsSer.available()) {
            buf[i] = gpsSer.read();
            i++;
        }
    }
}
void GPSManager::setGPS_DynamicModel3()
{
    int gps_set_sucess=0;
    uint8_t setdm3[] = {
        0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x03,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
        0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x76           };
    while(!gps_set_sucess)
    {
        sendUBX(setdm3, sizeof(setdm3)/sizeof(uint8_t));
        gps_set_sucess=getUBX_ACK(setdm3);
    }
}
void GPSManager::gps_get_position()
{
    GPSerror = 0;
    gpsSer.flush();
    // Request a NAV-POSLLH message from the GPS
    uint8_t request[8] = {
        0xB5, 0x62, 0x01, 0x02, 0x00, 0x00, 0x03, 0x0A           };
    sendUBX(request, 8);
    
    // Get the message back from the GPS
    gps_get_data();
    
    // Verify the sync and header bits
    if( buf[0] != 0xB5 || buf[1] != 0x62 )
        GPSerror = 21;
    if( buf[2] != 0x01 || buf[3] != 0x02 )
        GPSerror = 22;
    
    if( !_gps_verify_checksum(&buf[2], 32) ) {
        GPSerror = 23;
    }
    
    if(GPSerror == 0) {
        if(sats<4)
        {
            lat=0;
            lon=0;
            alt=0;
        }
        else
        {
            lon = (int32_t)buf[10] | (int32_t)buf[11] << 8 |
            (int32_t)buf[12] << 16 | (int32_t)buf[13] << 24;
            lat = (int32_t)buf[14] | (int32_t)buf[15] << 8 |
            (int32_t)buf[16] << 16 | (int32_t)buf[17] << 24;
            alt = (int32_t)buf[22] | (int32_t)buf[23] << 8 |
            (int32_t)buf[24] << 16 | (int32_t)buf[25] << 24;
        }
        // 4 bytes of latitude/longitude (1e-7)
        lon_int=abs(lon/10000000);
        lon_dec=(labs(lon) % 10000000)/10;
        lat_int=abs(lat/10000000);
        lat_dec=(labs(lat) % 10000000)/10;
        
        
        // 4 bytes of altitude above MSL (mm)
        
        alt /= 1000; // Correct to meters
    }
    
}
void GPSManager::gps_get_time()
{
    GPSerror = 0;
    gpsSer.flush();
    // Send a NAV-TIMEUTC message to the receiver
    uint8_t request[8] = {
        0xB5, 0x62, 0x01, 0x21, 0x00, 0x00, 0x22, 0x67           };
    sendUBX(request, 8);
    
    // Get the message back from the GPS
    gps_get_data();
    
    // Verify the sync and header bits
    if( buf[0] != 0xB5 || buf[1] != 0x62 )
        GPSerror = 31;
    if( buf[2] != 0x01 || buf[3] != 0x21 )
        GPSerror = 32;
    
    if( !_gps_verify_checksum(&buf[2], 24) ) {
        GPSerror = 33;
    }
    
    if(GPSerror == 0) {
        if(buf[22] > 23 || buf[23] > 59 || buf[24] > 59)
        {
            GPSerror = 34;
        }
        else {
            hour = buf[22];
            minute = buf[23];
            second = buf[24];
        }
    }
}
void GPSManager::gps_check_lock()
{
    GPSerror = 0;
    gpsSer.flush();
    // Construct the request to the GPS
    uint8_t request[8] = {
        0xB5, 0x62, 0x01, 0x06, 0x00, 0x00,
        0x07, 0x16                                                                                                                                                                  };
    sendUBX(request, 8);
    
    // Get the message back from the GPS
    gps_get_data();
    // Verify the sync and header bits
    if( buf[0] != 0xB5 || buf[1] != 0x62 ) {
        GPSerror = 11;
    }
    if( buf[2] != 0x01 || buf[3] != 0x06 ) {
        GPSerror = 12;
    }
    
    // Check 60 bytes minus SYNC and CHECKSUM (4 bytes)
    if( !_gps_verify_checksum(&buf[2], 56) ) {
        GPSerror = 13;
    }
    
    if(GPSerror == 0){
        // Return the value if GPSfixOK is set in 'flags'
        if( buf[17] & 0x01 )
            lock = buf[16];
        else
            lock = 0;
        
        sats = buf[53];
    }
    else {
        lock = 0;
    }
}


void GPSManager::prepare_data()
{
    gps_check_lock();
    gps_get_position();
    gps_get_time();
}
void GPSManager::sync()
{
    oldhour=hour;
    oldminute=minute;
    oldsecond=second;
    gps_check_nav();
    
    if(lock!=3) // Blink LED to indicate no lock
    {
        errorstatus |=(1 << 5);  // Set bit 5 (Lock 0 = GPS Locked 1= Not Locked)
    }
    else
    {
        errorstatus &= ~(1 << 5); // Unset bit 5 (Lock 0 = GPS Locked 1= Not Locked)
    }
    checkDynamicModel();

    if((lock==3) && (psm_status==0) && (sats>=5) &&((errorstatus & (1 << 0))==0)&&((errorstatus & (1 << 1))==0)) // Check we aren't in an error condition
    {
        setGPS_PowerSaveMode();
        wait(1000);
        psm_status=1;
        errorstatus &= ~(1 << 4); // Set Bit 4 Indicating PSM is on
    }
    if(!lockvariables) {
        
        prepare_data();
        if(alt>maxalt && sats >= 4)
        {
            maxalt=alt;
        }
    }
    if((oldhour==hour&&oldminute==minute&&oldsecond==second)||sats<=4) {
        tslf++;
    }
    else
    {
        tslf=0;
        errorstatus &= ~(1 << 0); // Unset bit 0 (Clear GPS Error Condition Noted Switch to Max Performance Mode)
        errorstatus &= ~(1 << 1); // Unset bit 1 (Clear GPS Error Condition Noted Cold Boot GPS)
    }
    if((tslf>10 && ((errorstatus & (1 << 0))==0)&&((errorstatus & (1 << 1))==0))) {
        setupGPS();
        wait(125);
        setGps_MaxPerformanceMode();
        wait(125);
        errorstatus |=(1 << 0); // Set Bit 1 (GPS Error Condition Noted Switch to Max Performance Mode)
        psm_status=0;
        errorstatus |=(1 << 4); // Set Bit 4 (Indicate PSM is disabled)
    }
    if(tslf>100 && ((errorstatus & (1 << 0))==1)&&((errorstatus & (1 << 1))==0)) {
        errorstatus |=(1 << 0); // Unset Bit 0 we've already tried that didn't work
        errorstatus |=(1 << 1); // Set bit 1 indicating we are cold booting the GPS
        gpsSer.flush();
        resetGPS();
        wait(125);
        setupGPS();
    }
}
GPSFrame GPSManager::getGPSFrame()
{
    GPSFrame frame;
    frame.alt = alt;
    frame.sats = sats;
    frame.lock = lock;
    frame.lat = lat;
    frame.lon = lon;
    frame.hour = hour;
    frame.minute = minute;
    frame.second = second;
    frame.maxalt = maxalt;
    return frame;
}



// Code
