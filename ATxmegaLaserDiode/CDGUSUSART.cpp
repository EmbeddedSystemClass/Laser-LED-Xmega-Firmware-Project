/* 
* CDGUSUSART.cpp
*
* Created: 23.03.2016 13:12:15
* Author: TPU_2
*/

#include "CDGUSUSART.h"

// Laser Library
//#include "LaserLib/sys_interrupts.h"
#include "LaserLib/intx128.h"

// Standard
#include <avr/io.h>
#include <avr/interrupt.h>

// USART status flags
#define LOCAL_USARTSTATUS_DATAEMPTY_ERR	0x20
#define LOCAL_USARTSTATUS_FRAME_ERR		0x10
#define LOCAL_USARTSTATUS_DATAOVRR_ERR	0x08
#define LOCAL_USARTSTATUS_PARITY_ERR	0x04
#define LOCAL_USARTSTATUS_MASK			0x3C

#ifdef U2X
uint8_t bsel_table[32] =	{	12, 12, 12, 34, 138, 12, 34, 137, 12, 34, 135, 12, 33, 131, 31, 
								123, 27, 107, 19, 75, 7, 57, 3, 11, 0, 0, 0, 0, 0, 0, 0, 0};
int8_t bscale_table[32] =	{	6, 5, 4, 2, 0, 3, 1, -1, 2, 0, -2, 1, -1, -3, -2, -4, -3, -5,
								-4, -6, -4, -7, -5, -7, 0, 0, 0, 0, 0, 0, 0, 0};
bool bsupport_table[32] =	{	true, true, true, true, true, true, true, true, true, true, 
								true, true, true, true, true, true, true, true, true, true, 
								true, true, true, true, true, true, false, false, false, false,
								false, false};
#else
uint8_t bsel_table[32] =	{	12, 12, 12, 34, 138, 12, 34, 138, 12, 34, 137, 12, 34, 135, 33,
								131, 31, 123, 27, 107, 15, 121, 19, 75, 1, 3, 47, 19, 77, 11, 43, 0};
int8_t bscale_table[32] =	{	7, 6, 5, 3, 1, 4, 2, 0, 3, 1, -1, 2, 0, -2, -1, -3, -2, -4, -3, -5, 
								-3, -6, -4, -6, 0, -2, -6, -4, -7, -5, -7, 0};
bool bsupport_table[32] =	{	true, true, true, true, true, true, true, true, true, true,
								true, true, true, true, true, true, true, true, true, true,
								true, true, true, true, true, true, true, true, true, true,
								true, true};
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
	PORTD.DIRSET = PIN3_bm; // Set TX to output
	PORTD.DIRCLR = PIN2_bm; // Set RX to input
	
	PORTD.PIN2CTRL = PORT_OPC_TOTEM_gc;	// | PORT_SRLEN_bm;
	PORTD.PIN3CTRL = PORT_OPC_TOTEM_gc;	// | PORT_SRLEN_bm;
	
	// Turn on interrupts
	USARTD0.CTRLA = USART_RXCINTLVL_LO_gc | USART_TXCINTLVL_LO_gc/* | USART_DREINTLVL_LO_gc*/;
	
	// Enable RX, TX
	USARTD0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
	
	// Double transmission speed
#ifdef U2X
	USARTD0.CTRLB |=  (1 << USART_CLK2X_bm);
#else
	USARTD0.CTRLB &= ~(0 << USART_CLK2X_bm);
#endif

	// Parity param set
	uint8_t temp = 0;
	switch (parity)
	{
		case PARITY_DISABLE:
			temp |= USART_PMODE_DISABLED_gc;
		break;
		case PARITY_EVEN:
			temp |= USART_PMODE_EVEN_gc;
		break;
		case PARITY_ODD:
			temp |= USART_PMODE_ODD_gc;
		break;
	}
	
	// Stop bits param set
	switch (stopbits)
	{
		case STOPBITS_1BIT:
			temp &= ~USART_SBMODE_bm;
		break;
		case STOPBITS_2BIT:
			temp |=  USART_SBMODE_bm;
		break;
	}
	
	// USART mode asynchronous, 8bit
	USARTD0.CTRLC = (Async?USART_CMODE_ASYNCHRONOUS_gc:USART_CMODE_SYNCHRONOUS_gc) | USART_CHSIZE_8BIT_gc | temp;
	
	// Set baud rate
	USARTD0.BAUDCTRLA = bsel_table[baud];
	USARTD0.BAUDCTRLB = (((int8_t)(16 + bscale_table[baud])) << USART_BSCALE0_bp) | ((bsel_table[baud] >> 8) & 0x0F);	
} //CUSART

uint8_t CDGUSUSART::GetReceivedByte()
{
	return USARTD0.DATA;
}

void CDGUSUSART::SetTransmittingByte(uint8_t data)
{
	//while ( ( USARTD0.STATUS & (1<<USART_DREIF_bm)) );
	USARTD0.DATA = data;
}

bool CDGUSUSART::IsTransmitting()
{
	return !( USARTD0.STATUS & USART_TXCIF_bm);
}

bool CDGUSUSART::IsReceiving()
{
	return !( USARTD0.STATUS & USART_RXCIF_bm);
}

bool CDGUSUSART::IsDataEmpty()
{
	return !( USARTD0.STATUS & USART_DREIF_bm);
}

void CDGUSUSART::SetRxInterruptionCallback(void* sender, ISRCallback callback)
{
	InterruptSenderTable[USARTD0_RXC_vect_num] = sender;
	InterruptFuncTable[USARTD0_RXC_vect_num] = callback;
}

void CDGUSUSART::SetTxInterruptionCallback(void* sender, ISRCallback callback)
{
	InterruptSenderTable[USARTD0_TXC_vect_num] = sender;
	InterruptFuncTable[USARTD0_TXC_vect_num] = callback;
}

void CDGUSUSART::SetDMARxTrig(DMA_CH_t *dma_channel)
{
	dma_channel->TRIGSRC = DMA_CH_TRIGSRC_USARTD0_RXC_gc;
	dma_channel->SRCADDR0 = (int)&USARTD0.DATA;
	dma_channel->SRCADDR1 = (int)&USARTD0.DATA >> 8;
	dma_channel->SRCADDR2 = 0;
}

void CDGUSUSART::SetDMATxTrig(DMA_CH_t *dma_channel)
{
	dma_channel->TRIGSRC = DMA_CH_TRIGSRC_USARTD0_DRE_gc;
	dma_channel->DESTADDR0 = (int)&USARTD0.DATA;
	dma_channel->DESTADDR1 = (int)&USARTD0.DATA >> 8;
	dma_channel->DESTADDR2 = 0;
}
