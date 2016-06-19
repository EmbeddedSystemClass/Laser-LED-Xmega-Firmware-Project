/* 
* CRelayTimer.cpp
*
* Created: 28.03.2016 12:10:07
* Author: TPU_2
*/

#include <avr/io.h>

#include "CRelayTimer.h"
#include "LaserLib/int8a.h"

// default constructor
CRelayTimer::CRelayTimer()
{
} //CRelayTimer

// default destructor
CRelayTimer::~CRelayTimer()
{
} //~CRelayTimer

void CRelayTimer::Initialize(TIMER_WAVEFORMGEN_MODE WGM, TIMER_COMPAREMATCHOUT_MODE COM, TIMER_CLOCKSELECT CS, bool ForceOutput)
{
	uint8_t temp = 0;
	
	// Set waveform generation mode
	temp |= ((((WGM & 0x2) >> 1) << WGM21) | ((WGM & 0x1) << WGM20));
	
	// Clock selection
	temp |= (CS << CS20);
	
	// Output compare mode
	temp |= (COM << COM20);
	
	// Set control register
	TCCR2 = temp | ((ForceOutput?1:0) << FOC2);
	
	// Enable interrupts
	//TIMSK |=  (1 << OCIE2) | (1 << TOIE2);
	
	start = false;
}

void CRelayTimer::Start(uint8_t period)
{
	if (!start)
	{
		TCNT2 = period;
		TIMSK |=  (1 << OCIE2);
		start = true;
	}
}

void CRelayTimer::Start(uint16_t period)
{
	// not support
}

void CRelayTimer::Stop()
{
	TIMSK &=  ~(1 << OCIE2);
	start = false;
}

void CRelayTimer::SetCOMPA(uint16_t value)
{
	// not support
}

void CRelayTimer::SetCOMPB(uint16_t value)
{
	// not support
}

void CRelayTimer::SetCOMPC(uint16_t value)
{
	// not support
}

void CRelayTimer::SetCOMPA(uint8_t value)
{
	OCR2 = value;
}

void CRelayTimer::SetCOMPB(uint8_t value)
{
	// not support
}

void CRelayTimer::SetCOMPC(uint8_t value)
{
	// not support
}

void CRelayTimer::AsyncMode(bool mode)
{
	if (mode)
		ASSR |= (1 << AS2);
	else
		ASSR &= ~(1 << AS2);
}

bool CRelayTimer::WaitTCNTUpdateBusy(uint8_t timeout)
{
	while (timeout-- && (ASSR & (1 << TCN2UB))) ;
	
	if (ASSR & (1 << TCN2UB))
		return true;
	else
		return false;
}

bool CRelayTimer::WaitOutputCompareUpdateBusy(uint8_t timeout)
{
	while (timeout-- && (ASSR & (1 << OCR2UB))) ;
	
	if (ASSR & (1 << OCR2UB))
		return true;
	else
		return false;
}

bool CRelayTimer::WaitControlUpdateBusy(uint8_t timeout)
{
	while (timeout-- && (ASSR & (1 << TCR2UB))) ;
	
	if (ASSR & (1 << TCR2UB))
		return true;
	else
		return false;
}

void CRelayTimer::ResetPrescaler()
{
	SFIOR |= (1 << PSR2);
}

void CRelayTimer::SetOVFCallback(ISRCallback func, void* owner)
{
	InterruptFuncTable[TIMER2_OVF_vect_num] = func;
	InterruptSenderTable[TIMER2_OVF_vect_num] = owner;
}

void CRelayTimer::SetCAPTCallback(ISRCallback func, void* owner)
{
	// not support
}

void CRelayTimer::SetCOMPACallback(ISRCallback func, void* owner)
{
	InterruptFuncTable[TIMER2_COMP_vect_num] = func;
	InterruptSenderTable[TIMER2_COMP_vect_num] = owner;
}

void CRelayTimer::SetCOMPBCallback(ISRCallback func, void* owner)
{
	// not support
}

void CRelayTimer::SetCOMPCCallback(ISRCallback func, void* owner)
{
	// not support
}
