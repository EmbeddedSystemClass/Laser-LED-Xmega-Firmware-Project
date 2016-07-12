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

#ifndef FRAMEHEADER_LOWBYTE
#define FRAMEHEADER_LOWBYTE		0xAA
#endif

#ifndef FRAMEHEADER_HIGHBYTE
#define FRAMEHEADER_HIGHBYTE	0xCC
#endif

#define USE_CRC

class CMBEventsHandler
{
public:
	~CMBEventsHandler();
	virtual void OnTransactionCallback(uint8_t* data, uint16_t length) = 0;
	virtual void OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length) = 0;
	virtual void OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length) = 0;
	
protected:
	virtual void AllocateBuffers(uint16_t rx_bufSize, uint16_t tx_bufSize);
	virtual void FreeBuffers();
	
	// MODBUS protocol transaction methods
	virtual void FillRegDataTx(uint8_t addr, uint8_t* data, uint8_t length);
	virtual void FillRegDataRq(uint8_t addr, uint8_t length);
	virtual void FillVarDataTx(uint16_t addr, uint16_t* data, uint16_t length);
	virtual void FillVarDataRq(uint16_t addr, uint16_t length);
	
protected:
	// Buffers
	uint8_t* rx_buffer;
	uint8_t* tx_buffer;
	
	// Receiver variables
	uint8_t  rx_frame_length;
	uint16_t rx_bufferSize;
	
	// transmitter variables
	uint8_t  tx_frame_length;
	uint16_t tx_bufferSize;
};

#endif //__CMBPROTOCOL_H__
