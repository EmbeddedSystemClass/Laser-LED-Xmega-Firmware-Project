/*
* CTimer.h
*
* Created: 05.03.2016 16:47:36
* Author: Vladislav
*/


#ifndef __CTIMER1_H__
#define __CTIMER1_H__

#include <stdint.h>
#include <stdlib.h>

#include "sys_interrupts.h"
#include "TimerDef.h"

//typedef void (*TimerCallbackFunc)(void* owner);

class CTimer1
{
public:
	CTimer1();
	~CTimer1();

	// Basic timer functions
	virtual void Initialize(TIMER_WAVEFORMGEN_MODE wgm, TIMER_CLOCKSELECT clock) = 0;
	virtual void Start(uint16_t period) = 0;
	virtual void Stop() = 0;
	virtual void Reset() = 0;
	virtual void EnableChannel(TIMER_CHANNELS channels) = 0;
	virtual void DisableChannel(TIMER_CHANNELS channels) = 0;
	virtual void SetPeriod(uint16_t value) = 0;
	virtual void SetCOMPA(uint16_t value) = 0;
	virtual void SetCOMPB(uint16_t value) = 0;
	
	// Interrupts
	virtual void SetOVFCallback(ISRCallback func, void* owner, TC_OVFINTLVL_t intlvl) = 0;
	virtual void SetERRCallback(ISRCallback func, void* owner, TC_ERRINTLVL_t intlvl) = 0;
	virtual void SetCOMPACallback(ISRCallback func, void* owner, TC_CCAINTLVL_t intlvl) = 0;
	virtual void SetCOMPBCallback(ISRCallback func, void* owner, TC_CCBINTLVL_t intlvl) = 0;
protected:
}; //CTimer

#endif //__CTIMER_H__
