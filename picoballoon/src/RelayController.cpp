//
// RelayController.cpp
// Library C++ code
// ----------------------------------
// Developed with embedXcode
// http://embedXcode.weebly.com
//
// Project 		pbf-2
//
// Created by 	Aykut Celik, 13/01/2017 14:22
// 				Aykut Celik
//
// Copyright 	(c) Aykut Celik, 2017
// Licence		<#licence#>
//
// See 			RelayController.h and ReadMe.txt for references
//


// Library header
#include "RelayController.h"
void RelayController::setup()
{
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    digitalWrite(6, LOW);
    digitalWrite(7,HIGH);
}
void RelayController::turnOn()
{
    digitalWrite(7, HIGH);
}
void RelayController::turnOff()
{
    //digitalWrite(7, LOW);
}
// Code
