/* 
* CRelayTimer.h
*
* Created: 28.03.2016 12:10:09
* Author: TPU_2
*/


#ifndef __CRELAYTIMER_H__
#define __CRELAYTIMER_H__

#include "LaserLib/sys_interrupts.h"
#include "LaserLib/CTimer.h"
#include <stdlib.h>
#include <stdint.h>

class CRelayTimer : public CTimer
{
//functions
public:
	CRelayTimer();
	~CRelayTimer();

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
	
	// Extended timer functions
	void AsyncMode(bool mode);
	bool WaitTCNTUpdateBusy(uint8_t timeout);
	bool WaitOutputCompareUpdateBusy(uint8_t timeout);
	bool WaitControlUpdateBusy(uint8_t timeout);
	void ResetPrescaler();
	
	// interrupts
	virtual void SetOVFCallback(ISRCallback func, void* owner);
	virtual void SetCAPTCallback(ISRCallback func, void* owner);
	virtual void SetCOMPACallback(ISRCallback func, void* owner);
	virtual void SetCOMPBCallback(ISRCallback func, void* owner);
	virtual void SetCOMPCCallback(ISRCallback func, void* owner);
protected:
private:
	bool start;
}; //CRelayTimer

#endif //__CRELAYTIMER_H__
