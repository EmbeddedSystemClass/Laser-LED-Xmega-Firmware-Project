/* 
* CDS18B20.cpp
*
* Created: 08.06.2016 13:09:52
* Author: TPU_2
*/


#include "CDS18B20.h"
#include <avr/io.h>

#define F_CPU	32000000

#include <util/delay.h>

// default constructor
CDS18B20::CDS18B20()
{
} //CDS18B20

// default destructor
CDS18B20::~CDS18B20()
{
} //~CDS18B20

bool CDS18B20::w1_find()
{
	PORTD.DIRSET = PIN7_bm;	// pull down 1-wire bus
	_delay_us(485);			// wait for 480 us
	PORTD.DIRCLR = PIN7_bm; // 1-wire bus pull up
	_delay_us(65);			// wait for 65 us
	
	bool result;
	if((PORTD.IN & PIN7_bm) == 0x00)
		result = true;
	else
		result = false;
		
	_delay_us(420);			// wait 480 us
	return result;
}

void CDS18B20::w1_sendcmd(unsigned char cmd)
{
	for(unsigned char i = 0; i < 8; i++)
	{
		if((cmd & (1<<i)) == 1<<i)
		{
			PORTD.DIRSET = PIN7_bm;
			_delay_us(2);
			PORTD.DIRCLR = PIN7_bm;
			_delay_us(65);
		}
		else
		{
			
			PORTD.DIRSET = PIN7_bm;
			_delay_us(65);
			PORTD.DIRCLR = PIN7_bm;
			_delay_us(5);
		}
	}
}

unsigned char CDS18B20::w1_receive_byte()
{
	unsigned char data=0;
	for(unsigned char i = 0; i < 8; i++)
	{
		PORTD.DIRSET = PIN7_bm;
		_delay_us(2);
		PORTD.DIRCLR = PIN7_bm;
		_delay_us(10);
		if((PORTD.IN & PIN7_bm) == 0x00)
			data &= ~(1<<i);
		else
			data |= 1<<i;
		_delay_us(50);
	}
	return data;
}

int CDS18B20::temp_18b20()
{
	unsigned char data[2];
	int temp = 0;
	
	if (req)
	{
		if(w1_find())
		{
			w1_sendcmd(0xcc);	//skip ROM
			w1_sendcmd(0x44);	//start conversion
			find = true;
		}
		else
			find = false;
	}
	else
	{
		if (find)
		{
			w1_find();
			w1_sendcmd(0xcc);
			w1_sendcmd(0xbe);	//read temperature
			data[0] = w1_receive_byte();
			data[1] = w1_receive_byte();
			
			temp = data[1];
			temp = temp<<8;
			temp |= data[0];
			
			temp *= 0.625;
			
			temperature = temp;
		}
	}
	
	req = !req;
	
	return temperature;
}

void CDS18B20::Initialize()
{
	PORTD.OUTCLR = PIN7_bm;
	PORTD.PIN7CTRL = PORT_OPC_TOTEM_gc;// | PORT_SRLEN_bm;
	
	temperature = 0;
	find = false;
	req = true;
}
