/*
 * intx128.h
 *
 * Created: 04.03.2016 15:02:11
 *  Author: Vladislav
 */ 


#ifndef INTX128_H_
#define INTX128_H_

#include "avr/interrupt.h"
#include "sys_interrupts.h"

#define NUM_INERRUPTS	34

extern void* InterruptSenderTable[NUM_INERRUPTS+1];
extern ISRCallback InterruptFuncTable[NUM_INERRUPTS+1];

#endif /* INTX128_H_ */