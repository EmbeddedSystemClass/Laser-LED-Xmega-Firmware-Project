/*
 * intx128.cpp
 *
 * Created: 04.03.2016 15:02:29
 *  Author: Vladislav
 */ 

#ifndef INTX128_CPP_
#define INTX128_CPP_

#include "intx128.h"
#include <stdlib.h>
#include <avr/io.h>

void* InterruptSenderTable[NUM_INERRUPTS];
ISRCallback InterruptFuncTable[NUM_INERRUPTS];

#endif