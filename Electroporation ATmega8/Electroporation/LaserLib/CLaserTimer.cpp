/* 
* CLaserTimer.cpp
*
* Created: 09.03.2016 14:55:51
* Author: TPU_2
*/


#include "CLaserTimer.h"
#include "int128a.h"
#include <avr/io.h>

#define TIMER_PERIOD_1HZ	15625
#define TIMER_PERIOD_2HZ	15625/2
#define TIMER_PERIOD_3HZ	15625/3
#define TIMER_PERIOD_4HZ	15625/4
#define TIMER_PERIOD_5HZ	15625/5
#define TIMER_PERIOD_6HZ	15625/6

int16_t TimerPeriods[6] = {TIMER_PERIOD_1HZ, TIMER_PERIOD_2HZ, TIMER_PERIOD_3HZ, TIMER_PERIOD_4HZ, TIMER_PERIOD_5HZ, TIMER_PERIOD_6HZ};

// default constructor
CLaserTimer::CLaserTimer()
{
} //CLaserTimer

// default destructor
CLaserTimer::~CLaserTimer()
{
} //~CLaserTimer

void CLaserTimer::Initialize(uint16_t Freq)
{
	// Configure PORT B for Timer/Counter 1
	PORTB = (1 << PB5) | (1 << PB6);
	DDRB |= (1 << PB5) | (1 << PB6);
	
	// Configure Timer/Counter 1
	TCCR1A = 0; // Normal port operation, OC1A/OC1B/OC1C disconnected, CTC
	TCCR1B = (0 << WGM13) | (1 << WGM12) | (1 << CS10) | (1 << CS12); // CTC (Clear Timer on Compare), 1024 prescaler, Input capture is off
	
	// Configure Timer/Counter 0
	ASSR   = (0 << AS0);
	TCCR0  = (1 << WGM01) | (0 << WGM00) | (7 << CS00); // CTC, No prescaler
	
	// Stop prescaler TIM0, TIM123
	SFIOR |=  ((1 << TSM) | (1 << PSR0) | (1 << PSR321)); // Timer synchronization mode
	
	// Set period of Timer/Counter 1
	if ((Freq > 6) || (Freq < 1))
		OCR1A  = TimerPeriods[TIMER_PERIOD_6HZ]; // Default frequency if Freq out of range
	else
		OCR1A  = TimerPeriods[Freq-1];
	OCR0  = 255; // 10ms period
	
	TCNT1 = 0; // Reset TIM1
	TCNT0 = 0; // Reset TIM0
	
	// Enable interrupts TIM0
	TIMSK |= /*(1 << OCIE1A) |*/ (1 << OCIE0);

	// Start prescaler TIM0, TIM123
	SFIOR &=  ~(/*(1 << PSR0) |*/ (1 << PSR321));
	
	InterruptSenderTable[TIMER1_COMPA_vect_num] = this;
	InterruptFuncTable[TIMER1_COMPA_vect_num] = OnLaser_TIM1COMPACallback;
	
	InterruptSenderTable[TIMER0_COMP_vect_num] = this;
	InterruptFuncTable[TIMER0_COMP_vect_num] = OnLaser_TIM0COMPCallback;
}

void CLaserTimer::StartTimerPulse()
{
	// Start prescaler
	SFIOR &= ~(1 << PSR0);
}

void CLaserTimer::StopTimerPulse()
{
	// Stop prescaler
	SFIOR |=  (1 << PSR0);
	
	TCNT0 = 0; // Reset TIM0
}

void CLaserTimer::StartLaserTimer(int16_t Freq)
{
	uint16_t period = 0;
	
	// Reset prescaler
	SFIOR |= (1 << PSR321);
	
	// Set period of Timer/Counter 1
	if ((Freq > 6) || (Freq < 1))
		period  = TimerPeriods[TIMER_PERIOD_1HZ]; // Default frequency if Freq out of range
	else
		period  = TimerPeriods[Freq-1];
	OCR1A = period;
	
	// Reset timer
	TCNT1 = 0;

	// Start prescaler
	SFIOR &= ~(1 << PSR321);
	
	// Enable interrupt
	TIMSK |= (1 << OCIE1A);
}

void CLaserTimer::StopLaserTimer()
{
	// Stop prescaler
	//SFIOR |=  (1 << PSR321);
	
	// Disable interrupt
	TIMSK &= ~(1 << OCIE1A);
}

void CLaserTimer::OnLaser_TIM1COMPACallback(void* sender)
{
	//_delay_us(10);
	//PORTB &= ~((1 << PB5) | (1 << PB6));
	PORTB &= ~(/*(1 << PB5) |*/ (1 << PB6));
	
	CLaserTimer* owner = (CLaserTimer*)sender;
	
	owner->StartTimerPulse();
}

void CLaserTimer::OnLaser_TIM0COMPCallback(void* sender)
{
	PORTB |= /*(1 << PB5) |*/ (1 << PB6);
	
	CLaserTimer* owner = (CLaserTimer*)sender;
	
	owner->StopTimerPulse();
}
