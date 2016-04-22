/* 
* CLaserControlApp.cpp
*
* Created: 11.04.2016 15:27:29
* Author: TPU_2
*/


#include "CLaserControlApp.h"
#include <util/delay.h>

uint16_t swap(uint16_t data)
{
	return (data >> 8) | (data << 8);
}

// default constructor
CLaserControlApp::CLaserControlApp()
{
} //CLaserControlApp

// default destructor
CLaserControlApp::~CLaserControlApp()
{
} //~CLaserControlApp

// CMBEventsHandler
void CLaserControlApp::OnTransactionCallback(uint8_t* data, uint16_t length)
{
	//no code
}

void CLaserControlApp::OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length)
{
	// Update GUI variables	
	uint16_t val = swap(*((uint16_t*)data));
	switch (swap(addr))
	{
		/*
		case VARIABLE_ADDR_ENRG:
			Energy = val;
		break;
		case VARIABLE_ADDR_FREQ:
			Frequency = val;
		break;
		case VARIABLE_ADDR_LAMP:
			lamp_state_var = val;
		break;
		case VARIABLE_ADDR_LED:
			led_state_var = val;
		break;
		case VARIABLE_ADDR_WATER:
			water_state_var = val;
		break;*/
		case VARIABLE_ADDR_MIN:
		break;
		case VARIABLE_ADDR_SEC:
		break;
		case VARIABLE_ADDR_MSC:
		break;
		case VARIABLE_ADDR_PWR:
		break;
	}
}

void CLaserControlApp::OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length)
{
	// Update GUI registers
	if (addr == 0x03)	PIC_ID = data[1];
}

// Initialization
void CLaserControlApp::Initialize(CMBSender* sender)
{
	// GUI class initialization
	m_cpSender = sender;
	
	// Set global variables
	PIC_ID = 0;
	Energy = 50;
	Frequency = 4;
	water_state_var = 1;
	lamp_state_var = 1;
	led_state_var = 1;
}

void CLaserControlApp::Start()
{	
	// Startup DGUS initialization
	uint16_t pic_id = swap(1);
	m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
	m_cpSender->WaitMODBUSTransmitter();
	
	//Setup variables
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_ENRG, (uint16_t*)&Energy, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_FREQ, (uint16_t*)&Frequency, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_WATER, (uint16_t*)&water_state_var, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_LAMP, (uint16_t*)&lamp_state_var, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_LED, (uint16_t*)&led_state_var, 2);
	m_cpSender->WaitMODBUSTransmitter();
}

// Process GUI
void CLaserControlApp::Run()
{
	// Get PIC ID
	m_cpSender->StartMODBUSRegisterTransaction(REGISTER_ADDR_PICID, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WaitMODBUSListener();
	//_delay_ms(50);
	
	// Get Variables
	m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_FREQ, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WaitMODBUSListener();
	//_delay_ms(50);
	
	m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_ENRG, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WaitMODBUSListener();
	//_delay_ms(50);
	
	m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_LED, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WaitMODBUSListener();
	//_delay_ms(50);
	
	// Timer setup to timer run animation
	static uint16_t anim = 5;
	anim++;
	if (anim > 9) anim = 5;
	uint16_t pic_id = swap(anim);
	m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
	m_cpSender->WaitMODBUSTransmitter();
	
	/*
	Energy++;
	if (Energy > 100) Energy = 0;
	
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_ENRG, (uint16_t*)&Energy, 2);
	m_cpSender->WaitMODBUSTransmitter();
	_delay_ms(50);*/
}
