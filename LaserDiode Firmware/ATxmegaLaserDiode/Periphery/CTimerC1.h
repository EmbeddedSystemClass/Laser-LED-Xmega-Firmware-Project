/* 
* CTimerC1.h
*
* Created: 08.06.2016 17:15:15
* Author: TPU_2
*/


#ifndef __CTIMERC1_H__
#define __CTIMERC1_H__

#include "../LaserLib/CTimer1.h"

class CTimerC1 : public CTimer1
{
public:
	CTimerC1();
	~CTimerC1();
	
	// Basic timer functions
	virtual void Initialize(TIMER_WAVEFORMGEN_MODE wgm, TIMER_CLOCKSELECT clock);
	virtual void Start(uint16_t period);
	virtual void Stop();
	virtual void Reset();
	virtual void EnableChannel(TIMER_CHANNELS channels);
	virtual void DisableChannel(TIMER_CHANNELS channels);
	virtual void SetPeriod(uint16_t value);
	virtual void SetCOMPA(uint16_t value);
	virtual void SetCOMPB(uint16_t value);
	
	// Interrupts
	virtual void SetOVFCallback(ISRCallback func, void* owner, TC_OVFINTLVL_t intlvl);
	virtual void SetERRCallback(ISRCallback func, void* owner, TC_ERRINTLVL_t intlvl);
	virtual void SetCOMPACallback(ISRCallback func, void* owner, TC_CCAINTLVL_t intlvl);
	virtual void SetCOMPBCallback(ISRCallback func, void* owner, TC_CCBINTLVL_t intlvl);
protected:
private:
	TIMER_CLOCKSELECT clockSrc;
}; //CTimerC

#endif //__CTIMERC_H__

