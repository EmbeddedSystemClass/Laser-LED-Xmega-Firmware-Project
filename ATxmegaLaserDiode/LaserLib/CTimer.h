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

typedef void (*TimerCallbackFunc)(void* owner);

class CTimer
{
public:
	CTimer();
	CTimer(void* owner);
	virtual ~CTimer();

	virtual void Start(uint8_t period);
	virtual void Start(uint16_t period);
	virtual void Stop();
	
	void SetOVFCallback(TimerCallbackFunc func);
protected:
	TimerCallbackFunc OVF_callback;
	void* Owner;
}; //CTimer

#endif //__CTIMER_H__
