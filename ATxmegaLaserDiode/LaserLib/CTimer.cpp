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

// default destructor
CTimer::~CTimer()
{
} //~CTimer

void CTimer::Initialize(TIMER_WAVEFORMGEN_MODE WGM, TIMER_COMPAREMATCHOUT_MODE COM, TIMER_CLOCKSELECT CS, bool ForceOutput)
{
}

void CTimer::Start(uint8_t period)
{
	
}

void CTimer::Start(uint16_t period)
{
	
}

void CTimer::Stop()
{
	
}

void CTimer::SetCOMPA(uint16_t value)
{
	
}

void CTimer::SetCOMPB(uint16_t value)
{
	
}

void CTimer::SetCOMPC(uint16_t value)
{
	
}

void CTimer::SetCOMPA(uint8_t value)
{
	
}

void CTimer::SetCOMPB(uint8_t value)
{
	
}

void CTimer::SetCOMPC(uint8_t value)
{
	
}

void CTimer::SetOVFCallback(ISRCallback func, void* owner)
{

}

void CTimer::SetCAPTCallback(ISRCallback func, void* owner)
{

}
	
void CTimer::SetCOMPACallback(ISRCallback func, void* owner)
{

}

void CTimer::SetCOMPBCallback(ISRCallback func, void* owner)
{

}

void CTimer::SetCOMPCCallback(ISRCallback func, void* owner)
{

}
