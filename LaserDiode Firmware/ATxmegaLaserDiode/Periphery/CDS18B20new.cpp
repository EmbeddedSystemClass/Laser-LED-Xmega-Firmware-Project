/* 
* CDS18B20new.cpp
*
* Created: 08.06.2016 13:09:52
* Author: TPU_2
*/


#include "CDS18B20new.h"
#include <avr/io.h>

#define F_CPU	32000000

#include <util/delay.h>

// default constructor
CDS18B20new::CDS18B20new()
{
} //CDS18B20newnew

// default destructor
CDS18B20new::~CDS18B20new()
{
} //~CDS18B20new

/*
bool CDS18B20new::w1_find()
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

void CDS18B20new::w1_sendcmd(unsigned char cmd)
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

unsigned char CDS18B20new::w1_receive_byte()
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

int CDS18B20new::temp_18b20()
{
	unsigned char data[2];
	int temp = 0;
	if(w1_find())
	{
		w1_sendcmd(0xcc);	//skip ROM
		w1_sendcmd(0x44);	//start conversion
		_delay_ms(750);		//wait for conversion 750 ms
		w1_find();			//send Presence & Reset
		w1_sendcmd(0xcc);
		w1_sendcmd(0xbe);	//read temperature
		data[0] = w1_receive_byte();
		data[1] = w1_receive_byte();
		
		temp = data[1];
		temp = temp<<8;
		temp |= data[0];
		
		temp *= 0.625;
	}
	
	return temp;
}*/

void CDS18B20new::Initialize()
{
	PORTD.OUTCLR = PIN7_bm;
	PORTD.PIN7CTRL = PORT_OPC_TOTEM_gc;// | PORT_SRLEN_bm;
}
void CDS18B20new::process_w1_find()
{
	switch (cmd_state)
	{
		case DS18B20_FIND_0_START:
			PORTD.DIRSET = PIN7_bm;	// pull down 1-wire bus
			cmd_state = DS18B20_FIND_1_PD;
			// set 485 us period
		break;
		case DS18B20_FIND_1_PD:		// 485 us
			PORTD.DIRCLR = PIN7_bm; // 1-wire bus pull up
			cmd_state = DS18B20_FIND_2_PU;
			// set 65 us period
		break;
		case DS18B20_FIND_2_PU:		// 65 us, sample after
			if((PORTD.IN & PIN7_bm) == 0x00)
				find = true;
			else
				find = false;
			cmd_state = DS18B20_FIND_COMPL;
			// set 480us period
		break;
		case DS18B20_FIND_COMPL:	// complete find 480 us
		break;
	}
}
void CDS18B20new::process_w1_send()
{
	switch (cmd_state)
	{
		case DS18B20_SEND_0_START:
			tx_pos = 0;
			if ((tx_data & (1 << tx_pos)) == (1 << tx_pos))
			{
				// set period 2 us
				cmd_state = DS18B20_SEND_1_SEND1;
			}
			else
			{
				// set period 65 us
				cmd_state = DS18B20_SEND_1_SEND0;
			}
			PORTD.DIRSET = PIN7_bm;
		break;
		case DS18B20_SEND_1_SEND1:	// 2 us
			PORTD.DIRCLR = PIN7_bm;
			cmd_state = DS18B20_SEND_2_SEND1;
		break;
		case DS18B20_SEND_2_SEND1:	// 65 us
			tx_pos++;
			if (tx_pos == 8) cmd_state = DS18B20_SEND_COMPL;
			else
			{
				
			}
		break;
		case DS18B20_SEND_1_SEND0:	// 65 us
			PORTD.DIRCLR = PIN7_bm;
			cmd_state = DS18B20_SEND_2_SEND0;
		break;
		case DS18B20_SEND_2_SEND0:	// 5 us
			tx_pos++;
			if (tx_pos == 8) cmd_state = DS18B20_SEND_COMPL;
			else
			{
				
			}
		break;
		case DS18B20_SEND_COMPL:	// complete send
		break;
	}
}


