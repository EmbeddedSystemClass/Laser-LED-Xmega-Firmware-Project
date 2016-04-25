/* 
* CTimer0.h
*
* Created: 05.03.2016 17:07:02
* Author: Vladislav
*/


#ifndef __CTIMER0_H__
#define __CTIMER0_H__

#include "CTimer.h"

class CTimer0 : public CTimer
{
//functions
public:
	CTimer0(void* owner);
	~CTimer0();
	
	virtual void Start(uint8_t period);
	virtual void Start(uint16_t period);
	virtual void Stop();
protected:
private:
}; //CTimer0

#endif //__CTIMER0_H__
