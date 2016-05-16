/* 
* CTimer.h
*
* Created: 05.03.2016 16:47:36
* Author: Vladislav
*/


#ifndef __CTIMER_H__
#define __CTIMER_H__

#include <stdint.h>
#include <stdlib.h>

#include "sys_interrupts.h"

//typedef void (*TimerCallbackFunc)(void* owner);

typedef enum TIMER_WAVEFORMGEN_MODE_ENUM
{
	WGM_Normal = 0,
	WGM_PWM = 1,
	WGM_CTC = 2,
	WGM_FastPWM = 3
} TIMER_WAVEFORMGEN_MODE, *PTIMER_WAVEFORMGEN_MODE;

typedef enum TIMER_COMPAREMATCHOUT_MODE_ENUM
{
	COM_Normal = 0,
	COM_Toggle = 1,
	COM_Clear = 2,
	COM_Set = 3
} TIMER_COMPAREMATCHOUT_MODE, *PTIMER_COMPAREMATCHOUT_MODE;

typedef enum TIMER_CLOCKSELECT_ENUM
{
	CS_NoClock = 0,
	CS_NoPrescale = 1,
	CS_DIV8 = 2,
	CS_DIV32 = 3,
	CS_DIV64 = 4,
	CS_DIV128 =5,
	CS_DIV256 = 6,
	CS_DIV1024 = 7
} TIMER_CLOCKSELECT, *PTIMER_CLOCKSELECT;

class CTimer
{
public:
	CTimer();
	~CTimer();

	// Basic timer functions
	virtual void Initialize(TIMER_WAVEFORMGEN_MODE WGM, TIMER_COMPAREMATCHOUT_MODE COM, TIMER_CLOCKSELECT CS, bool ForceOutput);
	virtual void Start(uint8_t period);
	virtual void Start(uint16_t period);
	virtual void Stop();
	virtual void SetCOMPA(uint16_t value);
	virtual void SetCOMPB(uint16_t value);
	virtual void SetCOMPC(uint16_t value);
	virtual void SetCOMPA(uint8_t value);
	virtual void SetCOMPB(uint8_t value);
	virtual void SetCOMPC(uint8_t value);
	
	// Interrupts
	virtual void SetOVFCallback(ISRCallback func, void* owner);
	virtual void SetCAPTCallback(ISRCallback func, void* owner);
	virtual void SetCOMPACallback(ISRCallback func, void* owner);
	virtual void SetCOMPBCallback(ISRCallback func, void* owner);
	virtual void SetCOMPCCallback(ISRCallback func, void* owner);
protected:
}; //CTimer

#endif //__CTIMER_H__
