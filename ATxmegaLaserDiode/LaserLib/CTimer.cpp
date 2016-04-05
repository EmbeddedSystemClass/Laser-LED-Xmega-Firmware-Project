/* 
* CTimer.cpp
*
* Created: 05.03.2016 16:47:36
* Author: Vladislav
*/


#include "CTimer.h"
#include "sys_interrupts.h"

// default constructor
CTimer::CTimer()
{

} //CTimer

CTimer::CTimer(void* owner)
{
	Owner = owner;
} //CTimer

// default destructor
CTimer::~CTimer()
{
} //~CTimer

void CTimer::SetOVFCallback(TimerCallbackFunc func)
{
	OVF_callback = func;	
}
