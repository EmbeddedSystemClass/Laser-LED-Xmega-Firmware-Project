/* 
* CUSART.h
*
* Created: 05.03.2016 18:22:42
* Author: Vladislav
*/


#ifndef __CUSART_H__
#define __CUSART_H__

#include <stdint.h>
#include <stdbool.h>
#include "sys_interrupts.h"

typedef enum BAUDRATE_ENUM {
	BAUD_2400   = 0,
	BAUD_4800   = 1,
	BAUD_9600   = 2,
	BAUD_14400  = 3,
	BAUD_19200  = 4,
	BAUD_28800  = 5,
	BAUD_38400  = 6,
	BAUD_57600  = 7,
	BAUD_76800  = 8,
	BAUD_115200	= 9,
	BAUD_230400	= 10,
	BAUD_250K	= 11,
	BAUD_0_5M	= 12,
	BAUD_1M		= 13
} BAUDRATE;

typedef enum PARITY_ENUM {
	DISABLE,
	EVEN,
	ODD
} PARITY;

typedef enum STOBITS_ENUM {
	STOP_1BIT,
	STOP_2BIT
} STOPBITS;

// USART status flags
#define USARTSTATUS_DATAEMPTY_ERR	0x08
#define USARTSTATUS_FRAME_ERR		0x04
#define USARTSTATUS_DATAOVRR_ERR	0x02
#define USARTSTATUS_PARITY_ERR		0x01

class CUSART
{
//functions
public:
	CUSART();
	~CUSART();
	
	// Initialization
	virtual void Initialize(BAUDRATE baud, PARITY parity, STOPBITS stopbits, bool Async) = 0;
	
	// Data operations
	virtual uint8_t GetReceivedByte() = 0;
	virtual void SetTransmittingByte(uint8_t data) = 0;
	virtual bool IsTransmitting() = 0;
	virtual bool IsReceiving() = 0;
	virtual bool IsDataEmpty() = 0;
	
	// Interrupt control
	virtual void SetRxInterruptionCallback(void* sender, ISRCallback callback) {};
	virtual void SetTxInterruptionCallback(void* sender, ISRCallback callback) {};
protected:
private:
}; //CUSART

#endif //__CUSART_H__
