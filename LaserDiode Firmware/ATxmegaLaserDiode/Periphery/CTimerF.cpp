/*
* CTimerC.cpp
*
* Created: 21.04.2016 15:21:16
* Author: TPU_2
*/


#include "CTimerF.h"
#include "../LaserLib/intx128.h"

// default constructor
CTimerF::CTimerF()
{
} //CTimerC

// default destructor
CTimerF::~CTimerF()
{
} //~CTimerC

// Basic timer functions
void CTimerF::Initialize(TIMER_WAVEFORMGEN_MODE wgm, TIMER_CLOCKSELECT clock)
{
	clockSrc = clock;
	
	// Stop timer
	TCF0.CTRLA = (TCF0.CTRLA & ~TC0_CLKSEL_gm) | TC_CLKSEL_OFF_gc;
	
	// No events
	TCF0.CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;
	
	// Reset
	TCF0.CNT = 0;
	TCF0.CCA = 0;
	TCF0.CCB = 0;
	TCF0.CCC = 0;
	TCF0.CCD = 0;
	TCF0.PER = 0;
	
	// Waveform generation mode
	TCF0.CTRLB = (TCF0.CTRLB & ~TC0_WGMODE_gm) | wgm;
	
	// Start timer
	//TCF0.CTRLA = (TCF0.CTRLA & TC0_CLKSEL_gm) | clock;
	
	// Disable interrupts
	TCF0.INTCTRLA = TC_OVFINTLVL_OFF_gc;
	TCF0.INTCTRLB = TC_CCDINTLVL_OFF_gc | TC_CCCINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc | TC_CCAINTLVL_OFF_gc;
}

void CTimerF::Start(uint16_t period)
{
	// Set period of timer
	TCF0.PER = period;
	//TCF0.CNT = 0;
	
	// Start timer
	TCF0.CTRLA = (TCF0.CTRLA & ~TC0_CLKSEL_gm) | clockSrc;
}

void CTimerF::Stop()
{
	// Stop timer
	TCF0.CTRLA = (TCF0.CTRLA & ~TC0_CLKSEL_gm) | TC_CLKSEL_OFF_gc;
	TCF0.CNT = 0;
}

void CTimerF::EnableChannel(TIMER_CHANNELS channels)
{
	TCF0.CTRLB |= channels;
}

void CTimerF::DisableChannel(TIMER_CHANNELS channels)
{
	TCF0.CTRLB &= ~channels;
}

void CTimerF::ChannelSet(TIMER_CHANNELS channels)
{
	TCF0.CTRLC |= (channels >> 4);
}

void CTimerF::ChannelReset(TIMER_CHANNELS channels)
{
	TCF0.CTRLC &= ~(channels >> 4);
}

void CTimerF::SetPeriod(uint16_t value)
{
	TCF0.PER = value;
}

void CTimerF::SetCOMPA(uint16_t value)
{
	TCF0.CCA = value;
}

void CTimerF::SetCOMPB(uint16_t value)
{
	TCF0.CCB = value;
}

void CTimerF::SetCOMPC(uint16_t value)
{
	TCF0.CCC = value;
}

void CTimerF::SetCOMPD(uint16_t value)
{
	TCF0.CCD = value;
}

// Interrupts
void CTimerF::SetOVFCallback(ISRCallback func, void* owner, TC_OVFINTLVL_t intlvl)
{
	if (func != NULL)
		TCF0.INTCTRLA = (TCF0.INTCTRLA & TC0_OVFINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCF0_OVF_vect_num] = func;
	InterruptSenderTable[TCF0_OVF_vect_num] = owner;
}

void CTimerF::SetERRCallback(ISRCallback func, void* owner, TC_ERRINTLVL_t intlvl)
{
	if (func != NULL)
		TCF0.INTCTRLA = (TCF0.INTCTRLA & TC0_ERRINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCF0_ERR_vect_num] = func;
	InterruptSenderTable[TCF0_ERR_vect_num] = owner;
}

void CTimerF::SetCOMPACallback(ISRCallback func, void* owner, TC_CCAINTLVL_t intlvl)
{
	if (func != NULL)
		TCF0.INTCTRLB = (TCF0.INTCTRLB & TC0_CCAINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCF0_CCA_vect_num] = func;
	InterruptSenderTable[TCF0_CCA_vect_num] = owner;
}

void CTimerF::SetCOMPBCallback(ISRCallback func, void* owner, TC_CCBINTLVL_t intlvl)
{
	if (func != NULL)
		TCF0.INTCTRLB = (TCF0.INTCTRLB & TC0_CCBINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCF0_CCB_vect_num] = func;
	InterruptSenderTable[TCF0_CCB_vect_num] = owner;
}

void CTimerF::SetCOMPCCallback(ISRCallback func, void* owner, TC_CCCINTLVL_t intlvl)
{
	if (func != NULL)
		TCF0.INTCTRLB = (TCF0.INTCTRLB & TC0_CCCINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCF0_CCC_vect_num] = func;
	InterruptSenderTable[TCF0_CCC_vect_num] = owner;
}

void CTimerF::SetCOMPDCallback(ISRCallback func, void* owner, TC_CCDINTLVL_t intlvl)
{
	if (func != NULL)
		TCF0.INTCTRLB = (TCF0.INTCTRLB & TC0_CCDINTLVL_gm) | intlvl;
	
	InterruptFuncTable[TCF0_CCD_vect_num] = func;
	InterruptSenderTable[TCF0_CCD_vect_num] = owner;
}