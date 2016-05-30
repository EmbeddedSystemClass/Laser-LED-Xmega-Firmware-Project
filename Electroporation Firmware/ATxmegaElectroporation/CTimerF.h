/*
* CTimerC.h
*
* Created: 21.04.2016 15:21:17
* Author: TPU_2
*/


#ifndef __CTIMERF_H__
#define __CTIMERF_H__

#include "LaserLib/CTimer.h"

class CTimerF : public CTimer
{
	public:
	CTimerF();
	~CTimerF();
	
	// Basic timer functions
	virtual void Initialize(TIMER_WAVEFORMGEN_MODE wgm, TIMER_CLOCKSELECT clock);
	virtual void Start(uint16_t period);
	virtual void Stop();
	virtual void EnableChannel(TIMER_CHANNELS channels);
	virtual void DisableChannel(TIMER_CHANNELS channels);
	virtual void ChannelSet(TIMER_CHANNELS channels);
	virtual void ChannelReset(TIMER_CHANNELS channels);
	virtual void SetPeriod(uint16_t value);
	virtual void SetCOMPA(uint16_t value);
	virtual void SetCOMPB(uint16_t value);
	virtual void SetCOMPC(uint16_t value);
	virtual void SetCOMPD(uint16_t value);
	
	// Interrupts
	virtual void SetOVFCallback(ISRCallback func, void* owner, TC_OVFINTLVL_t intlvl);
	virtual void SetERRCallback(ISRCallback func, void* owner, TC_ERRINTLVL_t intlvl);
	virtual void SetCOMPACallback(ISRCallback func, void* owner, TC_CCAINTLVL_t intlvl);
	virtual void SetCOMPBCallback(ISRCallback func, void* owner, TC_CCBINTLVL_t intlvl);
	virtual void SetCOMPCCallback(ISRCallback func, void* owner, TC_CCCINTLVL_t intlvl);
	virtual void SetCOMPDCallback(ISRCallback func, void* owner, TC_CCDINTLVL_t intlvl);
	protected:
	private:
	TIMER_CLOCKSELECT clockSrc;
}; //CTimerC

#endif //__CTIMERC_H__
