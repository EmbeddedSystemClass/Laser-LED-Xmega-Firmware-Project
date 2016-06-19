/* 
* CTimerD.cpp
*
* Created: 21.04.2016 15:21:16
* Author: TPU_2
*/


#include "CTimerD.h"
#include "../LaserLib/intx128.h"

// default constructor
CTimerD::CTimerD()
{
} //CTimerD

// default destructor
CTimerD::~CTimerD()
{
} //~CTimerD

// Basic timer functions
void CTimerD::Initialize(TIMER_WAVEFORMGEN_MODE wgm, TIMER_CLOCKSELECT clock)
{	
	clockSrc = clock;
	
	// Stop timer
	TCD0.CTRLA = (TCD0.CTRLA & ~TC0_CLKSEL_gm) | TC_CLKSEL_OFF_gc;
	
	// No events
	TCD0.CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;
	
	// Reset
	TCD0.CNT = 0;
	TCD0.CCA = 0;
	TCD0.CCB = 0;
	TCD0.CCC = 0;
	TCD0.CCD = 0;
	TCD0.PER = 0;
	
	// Waveform generation mode
	TCD0.CTRLB = (TCD0.CTRLB & ~TC0_WGMODE_gm) | wgm;
	
	// Start timer
	//TCD0.CTRLA = (TCD0.CTRLA & TC0_CLKSEL_gm) | clock;
	
	// Disable interrupts
	TCD0.INTCTRLA = TC_OVFINTLVL_OFF_gc;
	TCD0.INTCTRLB = TC_CCDINTLVL_OFF_gc | TC_CCCINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc | TC_CCAINTLVL_OFF_gc;
}

void CTimerD::Start(uint16_t period)
{
	// Set period of timer
	TCD0.PER = period;
	
	// Start timer
	TCD0.CTRLA = (TCD0.CTRLA & ~TC0_CLKSEL_gm) | clockSrc;
}

void CTimerD::Stop()
{
	// Stop timer
	TCD0.CTRLA = (TCD0.CTRLA & ~TC0_CLKSEL_gm) | TC_CLKSEL_OFF_gc;
	TCD0.CNT = 0;
}

void CTimerD::EnableChannel(TIMER_CHANNELS channels)
{
	TCD0.CTRLB = (TCD0.CTRLB & ~0xF0) | channels;
}

void CTimerD::DisableChannel(TIMER_CHANNELS channels)
{
	TCD0.CTRLB &= ~channels;
}

void CTimerD::SetPeriod(uint16_t value)
{
	TCD0.PER = value;
}

void CTimerD::SetCOMPA(uint16_t value)
{
	TCD0.CCA = value;
}

void CTimerD::SetCOMPB(uint16_t value)
{
	TCD0.CCB = value;
}

void CTimerD::SetCOMPC(uint16_t value)
{
	TCD0.CCC = value;
}

void CTimerD::SetCOMPD(uint16_t value)
{
	TCD0.CCD = value;
}

// Interrupts
void CTimerD::SetOVFCallback(ISRCallback func, void* owner, TC_OVFINTLVL_t intlvl)
{
	if (func != NULL)
		TCD0.INTCTRLA = (TCD0.INTCTRLA & TC0_OVFINTLVL_gm) | intlvl;
		
	InterruptFuncTable[TCD0_OVF_vect_num] = func;
	InterruptSenderTable[TCD0_OVF_vect_num] = owner;
}

void CTimerD::SetERRCallback(ISRCallback func, void* owner, TC_ERRINTLVL_t intlvl)
{
	if (func != NULL)
		TCD0.INTCTRLA = (TCD0.INTCTRLA & TC0_ERRINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCD0_ERR_vect_num] = func;
	InterruptSenderTable[TCD0_ERR_vect_num] = owner;
}

void CTimerD::SetCOMPACallback(ISRCallback func, void* owner, TC_CCAINTLVL_t intlvl)
{
	if (func != NULL)
		TCD0.INTCTRLB = (TCD0.INTCTRLB & TC0_CCAINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCD0_CCA_vect_num] = func;
	InterruptSenderTable[TCD0_CCA_vect_num] = owner;
}

void CTimerD::SetCOMPBCallback(ISRCallback func, void* owner, TC_CCBINTLVL_t intlvl)
{
	if (func != NULL)
		TCD0.INTCTRLB = (TCD0.INTCTRLB & TC0_CCBINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCD0_CCB_vect_num] = func;
	InterruptSenderTable[TCD0_CCB_vect_num] = owner;
}

void CTimerD::SetCOMPCCallback(ISRCallback func, void* owner, TC_CCCINTLVL_t intlvl)
{
	if (func != NULL)
		TCD0.INTCTRLB = (TCD0.INTCTRLB & TC0_CCCINTLVL_gm) | intlvl;
		
	InterruptFuncTable[TCD0_CCC_vect_num] = func;
	InterruptSenderTable[TCD0_CCC_vect_num] = owner;
}

void CTimerD::SetCOMPDCallback(ISRCallback func, void* owner, TC_CCDINTLVL_t intlvl)
{
	if (func != NULL)
		TCD0.INTCTRLB = (TCD0.INTCTRLB & TC0_CCDINTLVL_gm) | intlvl;
		
	InterruptFuncTable[TCD0_CCD_vect_num] = func;
	InterruptSenderTable[TCD0_CCD_vect_num] = owner;
}