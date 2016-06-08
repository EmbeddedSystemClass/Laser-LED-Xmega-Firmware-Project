/*
* CTimerC1.cpp
*
* Created: 21.04.2016 15:21:16
* Author: TPU_2
*/


#include "CTimerC1.h"
#include "../LaserLib/intx128.h"

// default constructor
CTimerC1::CTimerC1()
{
} //CTimerC1

// default destructor
CTimerC1::~CTimerC1()
{
} //~CTimerC1

// Basic timer functions
void CTimerC1::Initialize(TIMER_WAVEFORMGEN_MODE wgm, TIMER_CLOCKSELECT clock)
{
	clockSrc = clock;
	
	// Stop timer
	TCC1.CTRLA = (TCC1.CTRLA & ~TC0_CLKSEL_gm) | TC_CLKSEL_OFF_gc;
	
	// No events
	TCC1.CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;
	
	// Reset
	TCC1.CNT = 0;
	TCC1.CCA = 0;
	TCC1.CCB = 0;
	TCC1.CCC = 0;
	TCC1.CCD = 0;
	TCC1.PER = 0;
	
	// Waveform generation mode
	TCC1.CTRLB = (TCC1.CTRLB & ~TC0_WGMODE_gm) | wgm;
	
	// Start timer
	//TCC1.CTRLA = (TCC1.CTRLA & TC0_CLKSEL_gm) | clock;
	
	// Disable interrupts
	TCC1.INTCTRLA = TC_OVFINTLVL_OFF_gc;
	TCC1.INTCTRLB = TC_CCDINTLVL_OFF_gc | TC_CCCINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc | TC_CCAINTLVL_OFF_gc;
}

void CTimerC1::Start(uint16_t period)
{
	// Set period of timer
	TCC1.PER = period;
	
	// Start timer
	TCC1.CTRLA = (TCC1.CTRLA & ~TC0_CLKSEL_gm) | clockSrc;
}

void CTimerC1::Stop()
{
	// Stop timer
	TCC1.CTRLA = (TCC1.CTRLA & ~TC0_CLKSEL_gm) | TC_CLKSEL_OFF_gc;
	TCC1.CNT = 0;
}

void CTimerC1::EnableChannel(TIMER_CHANNELS channels)
{
	TCC1.CTRLB = (TCC1.CTRLB & ~0xF0) | channels;
}

void CTimerC1::DisableChannel(TIMER_CHANNELS channels)
{
	TCC1.CTRLB &= ~channels;
}

void CTimerC1::SetPeriod(uint16_t value)
{
	TCC1.PER = value;
}

void CTimerC1::SetCOMPA(uint16_t value)
{
	TCC1.CCA = value;
}

void CTimerC1::SetCOMPB(uint16_t value)
{
	TCC1.CCB = value;
}

void CTimerC1::SetCOMPC(uint16_t value)
{
	TCC1.CCC = value;
}

void CTimerC1::SetCOMPD(uint16_t value)
{
	TCC1.CCD = value;
}

// Interrupts
void CTimerC1::SetOVFCallback(ISRCallback func, void* owner, TC_OVFINTLVL_t intlvl)
{
	if (func != NULL)
	TCC1.INTCTRLA = (TCC1.INTCTRLA & TC0_OVFINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCC1_OVF_vect_num] = func;
	InterruptSenderTable[TCC1_OVF_vect_num] = owner;
}

void CTimerC1::SetERRCallback(ISRCallback func, void* owner, TC_ERRINTLVL_t intlvl)
{
	if (func != NULL)
	TCC1.INTCTRLA = (TCC1.INTCTRLA & TC0_ERRINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCC1_ERR_vect_num] = func;
	InterruptSenderTable[TCC1_ERR_vect_num] = owner;
}

void CTimerC1::SetCOMPACallback(ISRCallback func, void* owner, TC_CCAINTLVL_t intlvl)
{
	if (func != NULL)
	TCC1.INTCTRLB = (TCC1.INTCTRLB & TC0_CCAINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCC1_CCA_vect_num] = func;
	InterruptSenderTable[TCC1_CCA_vect_num] = owner;
}

void CTimerC1::SetCOMPBCallback(ISRCallback func, void* owner, TC_CCBINTLVL_t intlvl)
{
	if (func != NULL)
	TCC1.INTCTRLB = (TCC1.INTCTRLB & TC0_CCBINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCC1_CCB_vect_num] = func;
	InterruptSenderTable[TCC1_CCB_vect_num] = owner;
}