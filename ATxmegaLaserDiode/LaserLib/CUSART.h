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
	BAUD_2400          = 0,	
	BAUD_4800          = 1,
	BAUD_9600          = 2,
	BAUD_14400_ER0P8   = 3,
	BAUD_14400_ERM0P1  = 4,
	BAUD_19200         = 5,
	BAUD_28800_ER0P8   = 6,
	BAUD_28800_ERM0P1  = 7,
	BAUD_38400         = 8,
	BAUD_57600_ER0P8   = 9,
	BAUD_57600_ERM0P1  = 10,
	BAUD_76800         = 11,
	BAUD_115200_ER0P8  = 12,
	BAUD_115200_ERM0P1 = 13,
	BAUD_230400_ER0P8  = 14,
	BAUD_230400_ERM0P1 = 15,
	BAUD_460800_ER0P8  = 16,
	BAUD_460800_ERM0P1 = 17,
	BAUD_921600_ER0P8  = 18,
	BAUD_921600_ERM0P1 = 19,
	BAUD_1M382_ER0P8   = 20,
	BAUD_1M382_ERM0P1  = 21,
	BAUD_1M843_ER0P8   = 22,
	BAUD_1M843_ERM0P1  = 23,
	BAUD_2M			   = 24,
	BAUD_2M304_ER0P8   = 25,
	BAUD_2M304_ERM0P1  = 26,
	BAUD_2M5_ER0P8     = 27,
	BAUD_2M5_ERM0P1    = 28,
	BAUD_3M_ER0P8      = 29,
	BAUD_3M_ERM0P1	   = 30,
	BAUD_4M			   = 31
} BAUDRATE;

typedef enum PARITY_ENUM {
	PARITY_DISABLE,
	PARITY_EVEN,
	PARITY_ODD
} PARITY;

typedef enum STOBITS_ENUM {
	STOPBITS_1BIT,
	STOPBITS_2BIT
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
	virtual void Initialize(BAUDRATE baud, PARITY parity, STOPBITS stopbits, bool Async) {};
	
	// Data operations
	virtual uint8_t GetReceivedByte() { return 0; };
	virtual void SetTransmittingByte(uint8_t data) {};
	
	// Interrupt control
	virtual void SetRxInterruptionCallback(void* sender, ISRCallback callback) {};
	virtual void SetTxInterruptionCallback(void* sender, ISRCallback callback) {};
protected:
private:
}; //CUSART

#endif //__CUSART_H__
