/* 
* CTimer0.cpp
*
* Created: 05.03.2016 17:07:02
* Author: Vladislav
*/


#include "CTimer0.h"
#include <avr/io.h>

// default constructor
CTimer0::CTimer0(void* owner)
{
	//CTimer::CTimer(owner);
	Owner = owner;
	
	// Configure Timer/Counter 0
	ASSR   = (0 << AS0);
	TCCR0  = (1 << WGM01) | (0 << WGM00) | (7 << CS00); // CTC, No prescaler
	
	// Stop prescaler TIM0, TIM123
	SFIOR |= (1 << TSM) | (1 << PSR0);
	
	// Set period
	OCR0  = 10*16; // 10ms period
	
	TCNT0 = 0; // Reset TIM0
	
	// Enable interrupts TIM0
	TIMSK |= (1 << OCIE0);
} //CTimer0

// default destructor
CTimer0::~CTimer0()
{
} //~CTimer0

void CTimer0::Start(uint8_t period)
{
	// Start prescaler
	SFIOR &= ~(1 << PSR0);
}

void CTimer0::Start(uint16_t period)
{
	// no code
}

void CTimer0::Stop()
{
	// Stop prescaler
	SFIOR |=  (1 << PSR0);
	
	TCNT0 = 0; // Reset TIM0
}
