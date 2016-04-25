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
	WGM_Frequency = 1,
	WGM_SingleSlopePWM = 3,
	WGM_DualSlopeTopEventPWM = 5,
	WGM_DualSlopeBothEventPWM = 6,
	WGM_DualSlopeBottomEventPWM = 7
} TIMER_WAVEFORMGEN_MODE, *PTIMER_WAVEFORMGEN_MODE;

typedef enum TIMER_CLOCKSELECT_ENUM
{
	CS_NoClock = 0,
	CS_NoPrescale = 1,
	CS_DIV2 = 2,
	CS_DIV4 = 3,
	CS_DIV8 = 4,
	CS_DIV64 = 5,
	CS_DIV256 = 6,
	CS_DIV1024 = 7,
	CS_EventChannel0 = 8,
	CS_EventChannel1 = 9,
	CS_EventChannel2 = 10,
	CS_EventChannel3 = 11,
	CS_EventChannel4 = 12,
	CS_EventChannel5 = 13,
	CS_EventChannel6 = 14,
	CS_EventChannel7 = 15,
} TIMER_CLOCKSELECT, *PTIMER_CLOCKSELECT;

typedef enum TIMER_CHANNELS_ENUM
{
	TIMER_CHANNEL_A	= 0x10,
	TIMER_CHANNEL_B	= 0x20,
	TIMER_CHANNEL_C	= 0x40,
	TIMER_CHANNEL_D	= 0x80
} TIMER_CHANNELS, *PTIMER_CHANNELS;

class CTimer
{
public:
	CTimer();
	~CTimer();

	// Basic timer functions
	virtual void Initialize(TIMER_WAVEFORMGEN_MODE wgm, TIMER_CLOCKSELECT clock) = 0;
	virtual void Start(uint16_t period) = 0;
	virtual void Stop() = 0;
	virtual void EnableChannel(TIMER_CHANNELS channels) = 0;
	virtual void DisableChannel(TIMER_CHANNELS channels) = 0;
	virtual void SetPeriod(uint16_t value) = 0;
	virtual void SetCOMPA(uint16_t value) = 0;
	virtual void SetCOMPB(uint16_t value) = 0;
	virtual void SetCOMPC(uint16_t value) = 0;
	virtual void SetCOMPD(uint16_t value) = 0;
	
	// Interrupts
	virtual void SetOVFCallback(ISRCallback func, void* owner, TC_OVFINTLVL_t intlvl) = 0;
	virtual void SetERRCallback(ISRCallback func, void* owner, TC_ERRINTLVL_t intlvl) = 0;
	virtual void SetCOMPACallback(ISRCallback func, void* owner, TC_CCAINTLVL_t intlvl) = 0;
	virtual void SetCOMPBCallback(ISRCallback func, void* owner, TC_CCBINTLVL_t intlvl) = 0;
	virtual void SetCOMPCCallback(ISRCallback func, void* owner, TC_CCCINTLVL_t intlvl) = 0;
	virtual void SetCOMPDCallback(ISRCallback func, void* owner, TC_CCDINTLVL_t intlvl) = 0;
protected:
}; //CTimer

#endif //__CTIMER_H__
