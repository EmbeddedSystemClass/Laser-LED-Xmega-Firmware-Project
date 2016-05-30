/* 
* CSoundPlayer.cpp
*
* Created: 14.04.2016 10:34:57
* Author: TPU_2
*/

#define F_CPU 32000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "CSoundPlayer.h"

//Definition of the notes' frequecies in Hertz.
#define c 261
#define d 294
#define e 329
#define f 349
#define g 391
#define gS 415
#define a 440
#define aS 455
#define b 466
#define cH 523
#define cSH 554
#define dH 587
#define dSH 622
#define eH 659
#define fH 698
#define fSH 740
#define gH 784
#define gSH 830
#define aH 880

//Tone constants
#define DELAYPERIOD_1s	62500//31250

#define DELAYPERIOD_A1s	62//31
#define DELAYPERIOD_S1s	1//2

#define PERIOD_1Hz		62500
#define MAXVOLUME_CC	31250

extern void __builtin_avr_delay_cycles(unsigned long);

//This function generates the square wave that makes the piezo speaker sound at a determinated frequency.
void CSoundPlayer::beep(uint16_t note, uint16_t duration)
{
	SoundStart(note, duration, 3);
	SoundStop();
}

// default constructor
CSoundPlayer::CSoundPlayer()
{
} //CSoundPlayer

// default destructor
CSoundPlayer::~CSoundPlayer()
{
} //~CSoundPlayer

void CSoundPlayer::InitializeToneTimer()
{
	// Start timer 4MHz tick rate
	TCE0.CTRLA = TC_CLKSEL_OFF_gc;
	
	// Enable output compare channel D, PWM with single slope waveform generation mode
	TCE0.CTRLB = TC0_CCDEN_bm | TC0_CCCEN_bm | TC_WGMODE_DS_B_gc;
	
	// No events
	TCE0.CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;
	
	// Set period for 1.000 Hz PWM
	TCE0.PER = PERIOD_1Hz;
	
	// Set 50% duty cycle
	TCE0.CCC = 0;
	TCE0.CCD = 0;//MAXVOLUME_CC;
	
	TCE0.CTRLA = TC_CLKSEL_DIV256_gc;	//Start tone timer;
	
	// Disable interrupt
	TCE0.INTCTRLA = TC_OVFINTLVL_OFF_gc;
	TCE0.INTCTRLB = TC_CCDINTLVL_OFF_gc | TC_CCCINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc | TC_CCAINTLVL_OFF_gc;
}

void CSoundPlayer::InitializeDurationTimer()
{
	// Start timer 4MHz tick rate
	//TCE1.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCE1.CTRLA = TC_CLKSEL_OFF_gc;
	
	// Enable output compare channel D, PWM with single slope waveform generation mode
	TCE1.CTRLB = TC_WGMODE_NORMAL_gc;
	
	// No events
	TCE1.CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;
	
	// Set period for 1s
	TCE1.PER = DELAYPERIOD_1s;
	
	// Disable interrupt
	TCE1.INTCTRLA = TC_OVFINTLVL_OFF_gc;
	TCE1.INTCTRLB = TC_CCBINTLVL_OFF_gc | TC_CCAINTLVL_OFF_gc;
}

void CSoundPlayer::SoundStart(uint16_t freq, uint16_t time, uint8_t volume)
{
	if (lock) return;
	
	uint16_t period_tc0 = PERIOD_1Hz/freq;
	uint16_t period_tc1 = DELAYPERIOD_A1s * time + (time >> DELAYPERIOD_S1s);
	
	// Config
	TCE0.CCD = period_tc0 >> (volume + 1);
	TCE0.PER = period_tc0;
	TCE1.PER = period_tc1;
	
	//Start timer
	//TCE0.CTRLA = TC_CLKSEL_DIV256_gc;	//Start tone timer
	TCE1.CTRLA = TC_CLKSEL_DIV1024_gc;	//Start duration timer
}

void CSoundPlayer::SoundStop()
{
	if (lock) return;
	
	while (!(TCE1.INTFLAGS & TC0_OVFIF_bm));
	TCE1.INTFLAGS &= TC0_OVFIF_bm;

	//Stop timer
	//TCE0.CTRLA = TC_CLKSEL_OFF_gc;		//Stop tone timer
	TCE0.CCD = 0;
	TCE1.CTRLA = TC_CLKSEL_OFF_gc;		//Stop duration timer

	// Reset duration timer
	TCE1.CNT = 0;
	TCE0.CNT = 0;

	_delay_ms(20);
} 

void CSoundPlayer::Lock()
{
	lock = true;
	TCE0.CTRLA = TC_CLKSEL_DIV256_gc;
}

void CSoundPlayer::SetPWM(uint16_t duty_cycle)
{
	TCE0.PER = 1024;	
	TCE0.CCC = duty_cycle * 10;
	TCE0.CTRLA = TC_CLKSEL_DIV4_gc;
}

void CSoundPlayer::UnLock()
{
	lock = false;
	TCE0.CTRLA = TC_CLKSEL_DIV256_gc;
	TCE0.CCC = 0;
}

void CSoundPlayer::Initialize()
{
	InitializeToneTimer();
	InitializeDurationTimer();
}

void CSoundPlayer::Play()
{
	if (lock) return;
	cli();

	beep(a, 500);
	beep(a, 500);
	beep(a, 500);
	beep(f, 350);
	beep(cH, 150);
	beep(a, 500);
	beep(f, 350);
	beep(cH, 150);
	beep(a, 650);
	
	_delay_ms(150);
	//end of first bit
	/*
	beep(eH, 500);
	beep(eH, 500);
	beep(eH, 500);
	beep(fH, 350);
	beep(cH, 150);
	beep(gS, 500);
	beep(f, 350);
	beep(cH, 150);
	beep(a, 650);
	
	_delay_ms(150);
	//end of second bit...
	
	beep(aH, 500);
	beep(a, 300);
	beep(a, 150);
	beep(aH, 400);
	beep(gSH, 200);
	beep(gH, 200);
	beep(fSH, 125);
	beep(fH, 125);
	beep(fSH, 250);
	
	_delay_ms(250);
	
	beep(aS, 250);
	beep(dSH, 400);
	beep(dH, 200);
	beep(cSH, 200);
	beep(cH, 125);
	beep(b, 125);
	beep(cH, 250);
	
	_delay_ms(250);
	
	beep(f, 125);
	beep(gS, 500);
	beep(f, 375);
	beep(a, 125);
	beep(cH, 500);
	beep(a, 375);
	beep(cH, 125);
	beep(eH, 650);
	
	//end of third bit... (Though it doesn't play well)
	//let's repeat it
	
	beep(aH, 500);
	beep(a, 300);
	beep(a, 150);
	beep(aH, 400);
	beep(gSH, 200);
	beep(gH, 200);
	beep(fSH, 125);
	beep(fH, 125);
	beep(fSH, 250);
	
	_delay_ms(250);
	
	beep(aS, 250);
	beep(dSH, 400);
	beep(dH, 200);
	beep(cSH, 200);
	beep(cH, 125);
	beep(b, 125);
	beep(cH, 250);
	
	_delay_ms(250);
	
	beep(f, 250);
	beep(gS, 500);
	beep(f, 375);
	beep(cH, 125);
	beep(a, 500);
	beep(f, 375);
	beep(cH, 125);
	beep(a, 650);
	//end of the song*/

	sei();
}