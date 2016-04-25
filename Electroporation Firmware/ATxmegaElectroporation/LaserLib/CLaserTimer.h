/* 
* CLaserTimer.h
*
* Created: 09.03.2016 14:55:51
* Author: TPU_2
*/


#ifndef __CLASERTIMER_H__
#define __CLASERTIMER_H__

#include <stdint.h>


class CLaserTimer
{
public:
	CLaserTimer();
	~CLaserTimer();
	
	void Initialize(uint16_t Freq);
	void StartLaserTimer(int16_t Freq);
	void StopLaserTimer();
protected:
	void StartTimerPulse();
	void StopTimerPulse();
	
	static void OnLaser_TIM1COMPACallback(void* sender);
	static void OnLaser_TIM0COMPCallback(void* sender);
private:
}; //CLaserTimer

#endif //__CLASERTIMER_H__
