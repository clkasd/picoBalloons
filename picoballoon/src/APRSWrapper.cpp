//
// APRSWrapper.cpp
// Library C++ code
// ----------------------------------
// Developed with embedXcode
// http://embedXcode.weebly.com
//
// Project 		pbf-2
//
// Created by 	Aykut Celik, 03/01/2017 13:16
// 				Aykut Celik
//
// Copyright 	(c) Aykut Celik, 2017
// Licence		<#licence#>
//
// See 			APRSWrapper.h and ReadMe.txt for references
//


// Library header
#include "APRSWrapper.h"

#include <util/crc16.h>
// Prototypes


// Define variables and constants

#define HX1ENABLE 5
#define HX1TX     3

#define APRS_TELEM_INTERVAL 24

#define APRS_DEVID "APRS"
#define APRS_COMMENT "https://picoballoon.ist PBF-2"

#define APRS_PARM1    ":%-9s:PARM.Satellites"
#define APRS_UNIT1    ":%-9s:UNIT.Sats"
#define APRS_EQNS1    ":%-9s:EQNS.0,1,0"

#define APRS_PARM2   ",Temperature"
#define APRS_UNIT2   ",deg.C"
#define APRS_EQNS2   ",0,1,-100"

#define APRS_PARM3   ",TXCounter"
#define APRS_UNIT3   ",Packets"
#define APRS_EQNS3   ",0,1,0"


#define ONE_SECOND F_CPU / 1024 / 16

#define BAUD_RATE      (1200)
#define TABLE_SIZE     (512)
#define PREAMBLE_BYTES (50)
#define REST_BYTES     (5)
#define RTTY_BAUD 50     // RTTY Baud rate (Recommended = 50)
#define PLAYBACK_RATE    (F_CPU / 256)
#define SAMPLES_PER_BAUD (PLAYBACK_RATE / BAUD_RATE)
#define PHASE_DELTA_1200 (((TABLE_SIZE * 1200L) << 7) / PLAYBACK_RATE)
#define PHASE_DELTA_2200 (((TABLE_SIZE * 2200L) << 7) / PLAYBACK_RATE)
#define PHASE_DELTA_XOR  (PHASE_DELTA_1200 ^ PHASE_DELTA_2200)




static const uint8_t PROGMEM _sine_table[] = {
#include "sine_table.h"
};
int aprstxstatus = 0;
volatile static uint8_t *_txbuf = 0;
volatile static uint8_t  _txlen = 0;

char *ax25_base91enc(char *s, uint8_t n, uint32_t v)
{
    /* Creates a Base-91 representation of the value in v in the string */
    /* pointed to by s, n-characters long. String length should be n+1. */
    
    for(s += n, *s = '\0'; n; n--)
    {
        *(--s) = v % 91 + 33;
        v /= 91;
    }
    
    return(s);
}
static uint8_t *_ax25_callsign(uint8_t *s, char *callsign, char ssid)
{
    char i;
    for(i = 0; i < 6; i++)
    {
        if(*callsign) *(s++) = *(callsign++) << 1;
        else *(s++) = ' ' << 1;
    }
    *(s++) = ('0' + ssid) << 1;
    return(s);
}
ISR(TIMER1_COMPA_vect)
{
    //Serial.println("Timer1 interrupt occured");
}
ISR(TIMER2_OVF_vect)
{
    
    static uint16_t phase  = 0;
    static uint16_t step   = PHASE_DELTA_1200;
    static uint16_t sample = 0;
    static uint8_t rest    = PREAMBLE_BYTES + REST_BYTES;
    static uint8_t byte;
    static uint8_t bit     = 7;
    static int8_t bc       = 0;
    /* Update the PWM output */
    OCR2B = pgm_read_byte(&_sine_table[(phase >> 7) & 0x1FF]);
    phase += step;
    
    if(++sample < SAMPLES_PER_BAUD) return;
    sample = 0;
    
    /* Zero-bit insertion */
    if(bc == 5)
    {
        step ^= PHASE_DELTA_XOR;
        bc = 0;
        return;
    }
    
    /* Load the next byte */
    if(++bit == 8)
    {
        bit = 0;
        
        if(rest > REST_BYTES || !_txlen)
        {
            if(!--rest)
            {
                /* Disable radio and interrupt */
                
                PORTD &= ~_BV(HX1ENABLE); // Turn the HX1 Off
                //aprstxstatus=0;
                TIMSK2 &= ~_BV(TOIE2);
                
                /* Prepare state for next run */
                phase = sample = 0;
                step  = PHASE_DELTA_1200;
                rest  = PREAMBLE_BYTES + REST_BYTES;
                bit   = 7;
                bc    = 0;
                return;
            }
            
            /* Rest period, transmit ax.25 header */
            byte = 0x7E;
            bc = -1;
        }
        else
        {
            /* Read the next byte from memory */
            byte = *(_txbuf++);
            if(!--_txlen) rest = REST_BYTES + 2;
            if(bc < 0) bc = 0;
        }
    }
    
    /* Find the next bit */
    if(byte & 1)
    {
        /* 1: Output frequency stays the same */
        if(bc >= 0) bc++;
    }
    else
    {
        /* 0: Toggle the output frequency */
        step ^= PHASE_DELTA_XOR;
        if(bc >= 0) bc = 0;
    }
    
    byte >>= 1;
}


void APRSWrapper::ax25_frame(char *scallsign, char sssid, char *dcallsign, char dssid, char ttl1, char ttl2, char *data, ...)
{
    static uint8_t frame[100];
    uint8_t *s;
    uint16_t x;
    va_list va;
    
    va_start(va, data);
    
    /* Write in the callsigns and paths */
    s = _ax25_callsign(frame, dcallsign, dssid);
    s = _ax25_callsign(s, scallsign, sssid);
    if (ttl1) s = _ax25_callsign(s, "WIDE1", ttl1);
    if (ttl2) s = _ax25_callsign(s, "WIDE2", ttl2);
    
    /* Mark the end of the callsigns */
    s[-1] |= 1;
    
    *(s++) = 0x03; /* Control, 0x03 = APRS-UI frame */
    *(s++) = 0xF0; /* Protocol ID: 0xF0 = no layer 3 data */
    
    vsnprintf((char *) s, 100 - (s - frame) - 2, data, va);
    va_end(va);
    
    /* Calculate and append the checksum */
    for(x = 0xFFFF, s = frame; *s; s++)
        x = _crc_ccitt_update(x, *s);
    
    *(s++) = ~(x & 0xFF);
    *(s++) = ~((x >> 8) & 0xFF);
    
    /* Point the interrupt at the data to be transmit */
    _txbuf = frame;
    _txlen = s - frame;
    
    /* Enable the timer and key the radio */
    TIMSK2 |= _BV(TOIE2);
}
void APRSWrapper::initialise_interrupt()
{
    // initialize Timer1
    cli();          // disable global interrupts
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;     // same for TCCR1B
    OCR1A = F_CPU / 1024 / RTTY_BAUD - 1;  // set compare match register to desired timer count:
    TCCR1B |= (1 << WGM12);   // turn on CTC mode:
    // Set CS10 and CS12 bits for:
    TCCR1B |= (1 << CS10);
    TCCR1B |= (1 << CS12);
    // enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);
    sei();          // enable global interrupts
}

void APRSWrapper::ax25_init(void)
{
    /* Fast PWM mode, non-inverting output on OC2A */
    TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(CS20);
    pinMode(HX1TX, OUTPUT);
}
void APRSWrapper::tx_aprs()
{
    PORTD |= _BV(HX1ENABLE); // Same as digitalWrite(HX1_ENABLE, HIGH); but more efficient
    char slat[5];
    char slng[5];
    char stlm[9];
    char* ptr;
    double aprs_lat, aprs_lon;
    
    if(aprstxstatus == 0)
    {
        /* Convert the UBLOX-style coordinates to
         * the APRS compressed format */
        aprs_lat = 900000000 - frame.lat;
        aprs_lat = aprs_lat / 26 - aprs_lat / 2710 + aprs_lat / 15384615;
        aprs_lon = 900000000 + frame.lon / 2;
        aprs_lon = aprs_lon / 26 - aprs_lon / 2710 + aprs_lon / 15384615;
        int32_t aprs_alt = frame.alt * 32808 / 10000;
        ptr = stlm;
        /* Construct the compressed telemetry format */
        
        ax25_base91enc(ptr, 2, seq);
        ptr += 2;
        ax25_base91enc(ptr, 2, frame.sats);
        ptr += 2;
        ax25_base91enc(ptr, 2,frame.temperature + 100);
        ptr += 2;
        ax25_base91enc(ptr, 2,seq);
        if(frame.alt > 1000)
        {
            ax25_frame(
                       frame.APRSCallSign, frame.APRSSSID,
                       APRS_DEVID, 0,
                       0, frame.WIDE2,
                       "!/%s%sO   /A=%06ld|%s|%s",
                       ax25_base91enc(slat, 4, aprs_lat),
                       ax25_base91enc(slng, 4, aprs_lon),
                       aprs_alt, stlm, frame.APRSComment);
        }
        else
        {
            ax25_frame(
                       frame.APRSCallSign, frame.APRSSSID,
                       APRS_DEVID, 0,
                       frame.WIDE1, frame.WIDE2,
                       "!/%s%sO   /A=%06ld|%s|%s",
                       ax25_base91enc(slat, 4, aprs_lat),
                       ax25_base91enc(slng, 4, aprs_lon),
                       aprs_alt, stlm, frame.APRSComment);
        }
        
        seq++;
        if(seq % 30 == 0)
            aprstxstatus = 1;
    }
    else if(aprstxstatus == 1)
    {
        char s[10];
        strncpy_P(s, PSTR("TA2MUN"), 7);
        if(frame.APRSSSID) snprintf_P(s + strlen(s), 4, PSTR("-%i"), frame.APRSSSID);
        ax25_frame(
                   frame.APRSCallSign, frame.APRSSSID,
                   APRS_DEVID, 0,
                   0, 0,
                   APRS_PARM1 APRS_PARM2 APRS_PARM3,
                   s);
        aprstxstatus = 2;
    }
    else if(aprstxstatus == 2)
    {
        char s[10];
        strncpy_P(s, PSTR("TA2MUN"), 7);
        if(frame.APRSSSID) snprintf_P(s + strlen(s), 4, PSTR("-%i"), frame.APRSSSID);
        ax25_frame(
                   frame.APRSCallSign, frame.APRSSSID,
                   APRS_DEVID, 0,
                   0, 0,
                   APRS_UNIT1 APRS_UNIT2 APRS_UNIT3,
                   s);
        aprstxstatus = 3;
    }
    else if(aprstxstatus = 3)
    {
        char s[10];
        strncpy_P(s, PSTR("TA2MUN"), 7);
        if(frame.APRSSSID) snprintf_P(s + strlen(s), 4, PSTR("-%i"), frame.APRSSSID);
        ax25_frame(
                   frame.APRSCallSign, frame.APRSSSID,
                   APRS_DEVID, 0,
                   0, 0,
                   APRS_EQNS1 APRS_EQNS2 APRS_EQNS3,
                   s);
        aprstxstatus = 0;
    }
    
    
}
void APRSWrapper::setFrame(APRSFrame fr)
{
    frame = fr;
}
APRSFrame APRSWrapper::getFrame()
{
    return frame;
}
void APRSWrapper::sendParameters()
{
    PORTD |= _BV(HX1ENABLE); // Same as digitalWrite(HX1_ENABLE, HIGH); but more efficient
    if(aprstxstatus == 1)
    {
        char s[10];
        strncpy_P(s, PSTR("TA2MUN"), 7);
        if(frame.APRSSSID) snprintf_P(s + strlen(s), 4, PSTR("-%i"), frame.APRSSSID);
        ax25_frame(
                   frame.APRSCallSign, frame.APRSSSID,
                   APRS_DEVID, 0,
                   0, 0,
                   APRS_PARM1 APRS_PARM2 APRS_PARM3,
                   s);
        aprstxstatus = 2;
        Serial.println("First param sent");
    }
    else if(aprstxstatus == 2)
    {
        char s[10];
        strncpy_P(s, PSTR("TA2MUN"), 7);
        if(frame.APRSSSID) snprintf_P(s + strlen(s), 4, PSTR("-%i"), frame.APRSSSID);
        ax25_frame(
                   frame.APRSCallSign, frame.APRSSSID,
                   APRS_DEVID, 0,
                   0, 0,
                   APRS_UNIT1 APRS_UNIT2 APRS_UNIT3,
                   s);
        aprstxstatus = 3;
        Serial.println("second param");
    }
    else if(aprstxstatus = 3)
    {
        char s[10];
        strncpy_P(s, PSTR("TA2MUN"), 7);
        if(frame.APRSSSID) snprintf_P(s + strlen(s), 4, PSTR("-%i"), frame.APRSSSID);
        ax25_frame(
                   frame.APRSCallSign, frame.APRSSSID,
                   APRS_DEVID, 0,
                   0, 0,
                   APRS_EQNS1 APRS_EQNS2 APRS_EQNS3,
                   s);
        aprstxstatus = 1;
        Serial.println("third param");
    }

}

// Code
