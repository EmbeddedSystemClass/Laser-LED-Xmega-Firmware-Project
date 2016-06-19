/*
 * int128a.h
 *
 * Created: 04.03.2016 15:02:11
 *  Author: Vladislav
 */ 


#ifndef INT128A_H_
#define INT128A_H_

#include "avr/interrupt.h"
#include "sys_interrupts.h"

#define NUM_INERRUPTS	34

extern void* InterruptSenderTable[NUM_INERRUPTS+1];
extern ISRCallback InterruptFuncTable[NUM_INERRUPTS+1];

#endif /* INT128A_H_ */