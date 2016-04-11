/*
 * ATxmegaLaserDiode.cpp
 *
 * Created: 01.04.2016 20:57:39
 * Author : Vladislav
 */

// Xmega
#include <avr/io.h>
#include <avr/interrupt.h>

// Application class
#include "DGUSGUI.h"
#include "CDGUSUSART.h"
#include "CLaserBoard.h"
#include "MODBUS/CMBSender.h"
#include "LaserLib/CUSART.h"
#include "LaserLib/sys_interrupts.h"

// Utils
#include <math.h>
#include <util/delay.h>

/* Global variables */
CDGUSUSART usart;
CMBSender sender;
CLaserBoard laserBoard;

uint16_t swap(uint16_t data)
{
	return (data >> 8) | (data << 8);
}

class CLaserControllerApp : public CMBEventsHandler
{
public :
	// CMBEventsHandler
	void OnTransactionCallback(uint8_t* data, uint16_t length)
	{
		//no code
	}
	
	void OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length)
	{
		uint16_t val = swap(*((uint16_t*)data));
		switch (addr)
		{
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
			break;
		}
	}
	
	void OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length)
	{
		if (addr == 0x03)	PIC_ID = data[1];
	}
	
	// Initialization
	void Initialize(CMBSender* sender)
	{
		m_cpSender = sender;
		
		// Set local variables
		PIC_ID = 0;
		Energy = 50;
		Frequency = 4;
		water_state_var = 1;
		lamp_state_var = 1;
		led_state_var = 1;
	}
	
	void Start()
	{
		uint16_t pic_id = swap(1);
		m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
		m_cpSender->WaitMODBUSTransmitter();
		
		//Setup variables
		m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_ENRG, &Energy, 2);
		m_cpSender->WaitMODBUSTransmitter();
		m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_FREQ, &Frequency, 2);
		m_cpSender->WaitMODBUSTransmitter();
		m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_WATER, &water_state_var, 2);
		m_cpSender->WaitMODBUSTransmitter();
		m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_LAMP, &lamp_state_var, 2);
		m_cpSender->WaitMODBUSTransmitter();
		m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_LED, &led_state_var, 2);
		m_cpSender->WaitMODBUSTransmitter();
	}
	
	// Process
	void Run()
	{
		// Get PIC ID
		m_cpSender->StartMODBUSRegisterTransaction(REGISTER_ADDR_PICID, 2);
		m_cpSender->WaitMODBUSTransmitter();
		
		// Get Variables
		m_cpSender->StartMODBUSRegisterTransaction(VARIABLE_ADDR_FREQ, 2);
		m_cpSender->WaitMODBUSTransmitter();
		m_cpSender->StartMODBUSRegisterTransaction(VARIABLE_ADDR_ENRG, 2);
		m_cpSender->WaitMODBUSTransmitter();
		m_cpSender->StartMODBUSRegisterTransaction(VARIABLE_ADDR_LED, 2);
		m_cpSender->WaitMODBUSTransmitter();
		
		// Process application
		/* place code here */
	}
	
private :
	// Registers
	uint8_t PIC_ID;
	
	// Variables
	uint16_t Energy;
	uint16_t Frequency;
	
	// State variables
	uint16_t water_state_var;
	uint16_t lamp_state_var;
	uint16_t led_state_var;
	
	// Modules
	CMBSender* m_cpSender;
};

CLaserControllerApp App;

void SystemInitialize()
{
	cli();	/* Disable global interrupts */
	
	// Initialize modules
	laserBoard.InitializeIO();
	usart.Initialize(BAUD_115200_ERM0P1, PARITY_DISABLE, STOPBITS_1BIT, true);
	sender.Initialize(&usart, &App, 256, 256);
	App.Initialize(&sender);
	laserBoard.InitializeClock();
	
	sei();	/* Enable global interrupts */
}
	 
int main(void)
{
	// Initialization system
	SystemInitialize();
	
	// Startup delay
	_delay_ms(2000);
	
	// Initialize application GUI
	App.Start();
	
	// Start main loop
    while (1) 
    {
		// Loop delay
		_delay_ms(200);
		
		// Process application
		App.Run();
    }
}

