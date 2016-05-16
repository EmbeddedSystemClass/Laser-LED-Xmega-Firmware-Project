/* 
* CLaserBoard.cpp
*
* Created: 09.04.2016 15:23:56
* Author: Vladislav
*/


#include "CLaserBoard.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

// default constructor
CLaserBoard::CLaserBoard()
{
} //CLaserBoard

// default destructor
CLaserBoard::~CLaserBoard()
{
} //~CLaserBoard

// Initialize GPIO
void CLaserBoard::Init_Relay()
{
	PORTC = 0;
	PORTD = 0;
	DDRC = (1 << PC1);
	DDRD = (1 << PD7);
	
	SPI_MasterInit();
}

void CLaserBoard::Relay1On()
{
	PORTC |= (1 << PC1);
}

void CLaserBoard::Relay2On()
{
	PORTD |= (1 << PD7);
}

void CLaserBoard::Relay1Off()
{
	PORTC &= ~(1 << PC1);
}

void CLaserBoard::Relay2Off()
{
	PORTD &= ~(1 << PD7);
}

void CLaserBoard::SPI_MasterInit()
{
	DDRB = (1 << PB3) | (1 << PB2) | (1 << PB5);
	PORTB = 0;
	
	SPSR = 0;
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) /*| (1 << CPHA)*/ | (1 << SPR1) | (1 << SPR0) /*| (1 << DORD)*/;
}

void CLaserBoard::SPI_MasterTransmit(char data)
{
	/* Start transmission */
	SPDR = data;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
}

void CLaserBoard::SetDACValue(uint16_t data)
{
	PORTB |= (1 << PB2);
	
	SPI_MasterTransmit(~(uint8_t)(data >> 8));
	SPI_MasterTransmit(~(uint8_t)(data & 0xff));
	
	PORTB &= ~(1 << PB2);
}