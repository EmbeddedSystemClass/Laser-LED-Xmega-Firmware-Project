/* 
* CTimerC.cpp
*
* Created: 21.04.2016 15:21:16
* Author: TPU_2
*/


#include "CTimerC.h"
#include "../LaserLib/intx128.h"

// default constructor
CTimerC::CTimerC()
{
} //CTimerC

// default destructor
CTimerC::~CTimerC()
{
} //~CTimerC

// Basic timer functions
void CTimerC::Initialize(TIMER_WAVEFORMGEN_MODE wgm, TIMER_CLOCKSELECT clock)
{	
	clockSrc = clock;
	
	// Stop timer
	TCC0.CTRLA = (TCC0.CTRLA & ~TC0_CLKSEL_gm) | TC_CLKSEL_OFF_gc;
	
	// No events
	TCC0.CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;
	
	// Reset
	TCC0.CNT = 0;
	TCC0.CCA = 0;
	TCC0.CCB = 0;
	TCC0.CCC = 0;
	TCC0.CCD = 0;
	TCC0.PER = 0;
	
	// Waveform generation mode
	TCC0.CTRLB = (TCC0.CTRLB & ~TC0_WGMODE_gm) | wgm;
	
	// Start timer
	//TCC0.CTRLA = (TCC0.CTRLA & TC0_CLKSEL_gm) | clock;
	
	// Disable interrupts
	TCC0.INTCTRLA = TC_OVFINTLVL_OFF_gc;
	TCC0.INTCTRLB = TC_CCDINTLVL_OFF_gc | TC_CCCINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc | TC_CCAINTLVL_OFF_gc;
}

void CTimerC::Start(uint16_t period)
{
	// Set period of timer
	TCC0.PER = period;
	
	// Start timer
	TCC0.CTRLA = (TCC0.CTRLA & ~TC0_CLKSEL_gm) | clockSrc;
}

void CTimerC::Stop()
{
	// Stop timer
	TCC0.CTRLA = (TCC0.CTRLA & ~TC0_CLKSEL_gm) | TC_CLKSEL_OFF_gc;
	TCC0.CNT = 0;
}

void CTimerC::EnableChannel(TIMER_CHANNELS channels)
{
	TCC0.CTRLB = (TCC0.CTRLB & ~0xF0) | channels;
}

void CTimerC::DisableChannel(TIMER_CHANNELS channels)
{
	TCC0.CTRLB &= ~channels;
}

void CTimerC::SetPeriod(uint16_t value)
{
	TCC0.PER = value;
}

void CTimerC::SetCOMPA(uint16_t value)
{
	TCC0.CCA = value;
}

void CTimerC::SetCOMPB(uint16_t value)
{
	TCC0.CCB = value;
}

void CTimerC::SetCOMPC(uint16_t value)
{
	TCC0.CCC = value;
}

void CTimerC::SetCOMPD(uint16_t value)
{
	TCC0.CCD = value;
}

// Interrupts
void CTimerC::SetOVFCallback(ISRCallback func, void* owner, TC_OVFINTLVL_t intlvl)
{
	if (func != NULL)
		TCC0.INTCTRLA = (TCC0.INTCTRLA & TC0_OVFINTLVL_gm) | intlvl;
		
	InterruptFuncTable[TCC0_OVF_vect_num] = func;
	InterruptSenderTable[TCC0_OVF_vect_num] = owner;
}

void CTimerC::SetERRCallback(ISRCallback func, void* owner, TC_ERRINTLVL_t intlvl)
{
	if (func != NULL)
		TCC0.INTCTRLA = (TCC0.INTCTRLA & TC0_ERRINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCC0_ERR_vect_num] = func;
	InterruptSenderTable[TCC0_ERR_vect_num] = owner;
}

void CTimerC::SetCOMPACallback(ISRCallback func, void* owner, TC_CCAINTLVL_t intlvl)
{
	if (func != NULL)
		TCC0.INTCTRLB = (TCC0.INTCTRLB & TC0_CCAINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCC0_CCA_vect_num] = func;
	InterruptSenderTable[TCC0_CCA_vect_num] = owner;
}

void CTimerC::SetCOMPBCallback(ISRCallback func, void* owner, TC_CCBINTLVL_t intlvl)
{
	if (func != NULL)
		TCC0.INTCTRLB = (TCC0.INTCTRLB & TC0_CCBINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCC0_CCB_vect_num] = func;
	InterruptSenderTable[TCC0_CCB_vect_num] = owner;
}

void CTimerC::SetCOMPCCallback(ISRCallback func, void* owner, TC_CCCINTLVL_t intlvl)
{
	if (func != NULL)
		TCC0.INTCTRLB = (TCC0.INTCTRLB & TC0_CCCINTLVL_gm) | intlvl;
		
	InterruptFuncTable[TCC0_CCC_vect_num] = func;
	InterruptSenderTable[TCC0_CCC_vect_num] = owner;
}

void CTimerC::SetCOMPDCallback(ISRCallback func, void* owner, TC_CCDINTLVL_t intlvl)
{
	if (func != NULL)
		TCC0.INTCTRLB = (TCC0.INTCTRLB & TC0_CCDINTLVL_gm) | intlvl;
		
	InterruptFuncTable[TCC0_CCD_vect_num] = func;
	InterruptSenderTable[TCC0_CCD_vect_num] = owner;
}