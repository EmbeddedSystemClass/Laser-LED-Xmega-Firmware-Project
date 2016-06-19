/*
* CTimerD1.cpp
*
* Created: 21.04.2016 15:21:16
* Author: TPU_2
*/


#include "CTimerD1.h"
#include "../LaserLib/intx128.h"

// default constructor
CTimerD1::CTimerD1()
{
} //CTimerD1

// default destructor
CTimerD1::~CTimerD1()
{
} //~CTimerD1

// Basic timer functions
void CTimerD1::Initialize(TIMER_WAVEFORMGEN_MODE wgm, TIMER_CLOCKSELECT clock)
{
	clockSrc = clock;
	
	// Stop timer
	TCD1.CTRLA = (TCD1.CTRLA & ~TC1_CLKSEL_gm) | TC_CLKSEL_OFF_gc;
	
	// No events
	TCD1.CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;
	
	// Reset
	TCD1.CNT = 0;
	TCD1.CCA = 0;
	TCD1.CCB = 0;
	TCD1.PER = 0;
	
	// Waveform generation mode
	TCD1.CTRLB = (TCD1.CTRLB & ~TC1_WGMODE_gm) | wgm;
	
	// Start timer
	//TCD1.CTRLA = (TCD1.CTRLA & TC1_CLKSEL_gm) | clock;
	
	// Disable interrupts
	TCD1.INTCTRLA = TC_OVFINTLVL_OFF_gc;
	TCD1.INTCTRLB = TC_CCDINTLVL_OFF_gc | TC_CCCINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc | TC_CCAINTLVL_OFF_gc;
}

void CTimerD1::Start(uint16_t period)
{
	// Set period of timer
	TCD1.PER = period;
	
	// Start timer
	TCD1.CTRLA = (TCD1.CTRLA & ~TC1_CLKSEL_gm) | clockSrc;
}

void CTimerD1::Stop()
{
	// Stop timer
	TCD1.CTRLA = (TCD1.CTRLA & ~TC1_CLKSEL_gm) | TC_CLKSEL_OFF_gc;
	TCD1.CNT = 0;
}

void CTimerD1::Reset()
{
	// Stop timer
	uint16_t tmp = TCD1.CTRLA;
	TCD1.CTRLA = (TCD1.CTRLA & ~TC1_CLKSEL_gm) | TC_CLKSEL_OFF_gc;
	TCD1.CNT = 0;
	TCD1.CTRLA = tmp;
}

void CTimerD1::EnableChannel(TIMER_CHANNELS channels)
{
	TCD1.CTRLB = (TCD1.CTRLB & ~0xF0) | channels;
}

void CTimerD1::DisableChannel(TIMER_CHANNELS channels)
{
	TCD1.CTRLB &= ~channels;
}

void CTimerD1::SetPeriod(uint16_t value)
{
	TCD1.PER = value;
}

void CTimerD1::SetCOMPA(uint16_t value)
{
	TCD1.CCA = value;
}

void CTimerD1::SetCOMPB(uint16_t value)
{
	TCD1.CCB = value;
}

// Interrupts
void CTimerD1::SetOVFCallback(ISRCallback func, void* owner, TC_OVFINTLVL_t intlvl)
{
	if (func != NULL)
	TCD1.INTCTRLA = (TCD1.INTCTRLA & TC1_OVFINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCD1_OVF_vect_num] = func;
	InterruptSenderTable[TCD1_OVF_vect_num] = owner;
}

void CTimerD1::SetERRCallback(ISRCallback func, void* owner, TC_ERRINTLVL_t intlvl)
{
	if (func != NULL)
	TCD1.INTCTRLA = (TCD1.INTCTRLA & TC1_ERRINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCD1_ERR_vect_num] = func;
	InterruptSenderTable[TCD1_ERR_vect_num] = owner;
}

void CTimerD1::SetCOMPACallback(ISRCallback func, void* owner, TC_CCAINTLVL_t intlvl)
{
	if (func != NULL)
	TCD1.INTCTRLB = (TCD1.INTCTRLB & TC1_CCAINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCD1_CCA_vect_num] = func;
	InterruptSenderTable[TCD1_CCA_vect_num] = owner;
}

void CTimerD1::SetCOMPBCallback(ISRCallback func, void* owner, TC_CCBINTLVL_t intlvl)
{
	if (func != NULL)
	TCD1.INTCTRLB = (TCD1.INTCTRLB & TC1_CCBINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCD1_CCB_vect_num] = func;
	InterruptSenderTable[TCD1_CCB_vect_num] = owner;
}