/* 
* CLaserBoard.h
*
* Created: 09.04.2016 15:23:56
* Author: Vladislav
*/


#ifndef __CLASERBOARD_H__
#define __CLASERBOARD_H__

#define F_CPU 16000000

#include <avr/io.h>
#include "LaserLib/int8a.h"

class CLaserBoard
{
//functions
public:
	CLaserBoard();
	~CLaserBoard();
	
	void Init_Relay();
	
	void Relay1On();
	void Relay2On();
	void Relay1Off();
	void Relay2Off();
	
	void SPI_MasterInit();
	void SPI_MasterTransmit(char data);
	
	void SetINT1Callback(ISRCallback func, void* owner);
	void SetTIM1Callback(ISRCallback func, void* owner);
	
	void SetDACValue(uint16_t data);
protected:
private:

}; //CLaserBoard

#endif //__CLASERBOARD_H__
