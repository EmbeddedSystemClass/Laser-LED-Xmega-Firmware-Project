/* 
* CMBProtocol.h
*
* Created: 04.03.2016 14:36:31
* Author: Vladislav
*/


#ifndef __CMBPROTOCOL_H__
#define __CMBPROTOCOL_H__

#include <stdint.h>
#include <stdlib.h>

class CMBEventsHandler
{
public:
	~CMBEventsHandler();
	virtual void OnTransactionCallback(uint8_t* data, uint16_t length) = 0;
	virtual void OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length) = 0;
	virtual void OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length) = 0;
};

#endif //__CMBPROTOCOL_H__
