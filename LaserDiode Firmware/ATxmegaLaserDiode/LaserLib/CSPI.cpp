/* 
* CSPI.cpp
*
* Created: 13.04.2016 11:27:59
* Author: TPU_2
*/


#include "CSPI.h"
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

// default constructor
CSPI::CSPI()
{
} //CSPI

// default destructor
CSPI::~CSPI()
{
} //~CSPI

void CSPI::Initialize(bool Master, SPI_DATA_ORDER dord, SPI_MODE mode, bool doubleClk, SPI_PRESCALER_t prescaler)
{
	PORTE.DIRSET = PIN4_bm | PIN5_bm | PIN7_bm; // MOSI, nSS, SCK to out
	//PORTE.DIRCLR = PIN6_bm;						// MISO to in
	
	// All slew rate limit enable with inverting
	PORTE.PIN4CTRL = PORT_OPC_TOTEM_gc | PORT_SRLEN_bm | PORT_INVEN_bm;
	PORTE.PIN5CTRL = PORT_OPC_TOTEM_gc | PORT_SRLEN_bm | PORT_INVEN_bm;
	//PORTE.PIN6CTRL = PORT_OPC_TOTEM_gc | PORT_SRLEN_bm | PORT_INVEN_bm;
	PORTE.PIN7CTRL = PORT_OPC_TOTEM_gc | PORT_SRLEN_bm | PORT_INVEN_bm;
	
	// Set high
	PORTE.OUTSET = PIN4_bm | PIN5_bm | PIN7_bm;
	
	// SPI config
	SPIE.CTRL = 
		SPI_ENABLE_bm | 
		(Master?SPI_MASTER_bm:0) | 
		(doubleClk?SPI_CLK2X_bm:0) | 
		(dord?SPI_DORD_bm:0) | 
		(mode << SPI_MODE_gp) |
		prescaler;
		
	// Allocate transmit buffer
	buffer = (uint8_t*)malloc(256);
	
	// Enable interrupt
	SPIE.INTCTRL = SPI_INTLVL_LO_gc;
	
	// Set interrupts
	InterruptFuncTable[SPIE_INT_vect_num] = StaticOnTransmit;
	InterruptSenderTable[SPIE_INT_vect_num] = this;
}

void CSPI::Deinitialize()
{
	free(buffer);
	
	SPIE.CTRL &= ~SPI_ENABLE_bm;
}

void CSPI::Send(uint8_t* data, uint16_t length)
{
	rx_pos = length;
	rx_frame_length = length;
	
	// Copy data to transmit buffer
	memcpy(buffer, data, length);

	// Start transmit
	PORTE.OUTCLR = PIN4_bm;
	SPIE.DATA = buffer[--rx_pos];
}

void CSPI::SendByte(uint8_t data)
{
	SPIE.DATA = data;
}

void CSPI::StaticOnTransmit(void* sender)
{
	CSPI* spi = (CSPI*)sender;
	spi->OnTransmit();
}

void CSPI::OnTransmit()
{
	if (rx_pos != 0)
		SPIE.DATA = buffer[--rx_pos];
	else
		PORTE.OUTSET = PIN4_bm;
}
