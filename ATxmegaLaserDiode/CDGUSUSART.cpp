/* 
* CDGUSUSART.cpp
*
* Created: 23.03.2016 13:12:15
* Author: TPU_2
*/

#include "CDGUSUSART.h"
//#include "LaserLib/sys_interrupts.h"
#include "LaserLib/int8a.h"
#include <avr/io.h>
//#include <avr/interrupt.h>

// USART status flags
#define LOCAL_USARTSTATUS_DATAEMPTY_ERR	0x20
#define LOCAL_USARTSTATUS_FRAME_ERR		0x10
#define LOCAL_USARTSTATUS_DATAOVRR_ERR	0x08
#define LOCAL_USARTSTATUS_PARITY_ERR	0x04
#define LOCAL_USARTSTATUS_MASK			0x3C

// Parity
#define PARITY_DISABLED	(0 << UPM0)
#define PARITY_EVEN		(2 << UPM0)
#define PARITY_ODD		(3 << UPM0)

// Stop bits
#define STOPBITS_1BIT	(0 << USBS)
#define STOPBITS_2BIT	(1 << USBS)

#ifdef U2X
uint16_t ubrr_table[14] = {832u, 416u, 207u, 138u, 103u, 68u, 51u, 34u, 25u, 16u, 8u, 7u, 3u, 1u};
#else
uint16_t ubrr_table[14] = {416u, 207u, 103u, 68u, 51u, 34u, 25u, 16u, 12u, 8u, 3u, 3u, 1u, 0u};
#endif

// default constructor
CDGUSUSART::CDGUSUSART()
{
} //CDGUSUSART

// default destructor
CDGUSUSART::~CDGUSUSART()
{
} //~CDGUSUSART


void CDGUSUSART::Initialize(BAUDRATE baud, PARITY parity, STOPBITS stopbits, bool Async)
{
	DDRD |= 0x02; // PD1 - output
	DDRD &= 0xFE; // PD0 - input
	PORTD |= 0x0;
	
	// Control and status register
#ifdef U2X
	UCSRA = (1 << U2X);
#else
	UCSRA = (0 << U2X);
#endif

	// Parity param set
	uint8_t temp = 0;
	switch (parity)
	{
		case DISABLE:
			temp |= PARITY_DISABLED;
		break;
		case EVEN:
			temp |= PARITY_EVEN;
		break;
		case ODD:
			temp |= PARITY_ODD;
		break;
	}
	
	// Stop bits param set
	switch (stopbits)
	{
		case STOP_1BIT:
			temp |= STOPBITS_1BIT;
		break;
		case STOP_2BIT:
			temp |= STOPBITS_2BIT;
		break;
	}

	// Configure USART
	UCSRB = (1 << RXCIE) | (1 << TXCIE) | (1 << RXEN) | (1 << TXEN);
	UCSRC = ((Async?0:1) << UMSEL) | temp | (3 << UCSZ0); // 8 bit
	
	// Set baud rate
	uint16_t UBRR = ubrr_table[baud];
	UBRRL = (uint8_t)(UBRR & 0xff);
	UBRRH = (uint8_t)(UBRR >> 8);
} //CUSART

uint8_t CDGUSUSART::GetReceivedByte()
{
	return UDR;
}

void CDGUSUSART::SetTransmittingByte(uint8_t data)
{
	while ( !( UCSRA & (1<<UDRE)) );
	UDR = data;
}

void CDGUSUSART::SetRxInterruptionCallback(void* sender, ISRCallback callback)
{
	InterruptSenderTable[USART_RXC_vect_num] = sender;
	InterruptFuncTable[USART_RXC_vect_num] = callback;
}

void CDGUSUSART::SetTxInterruptionCallback(void* sender, ISRCallback callback)
{
	InterruptSenderTable[USART_TXC_vect_num] = sender;
	InterruptFuncTable[USART_TXC_vect_num] = callback;
}
