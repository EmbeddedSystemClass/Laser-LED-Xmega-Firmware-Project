/*
 * ATxmegaLaserDiode.cpp
 *
 * Created: 01.04.2016 20:57:39
 * Author : Vladislav
 */

// Xmega
#include <avr/io.h>
#include <avr/interrupt.h>

// DGUS
#include "DGUSGUI.h"
#include "CDGUSUSART.h"
#include "CDGUSDatabase.h"

// Application class
#include "CLaserBoard.h"
#include "CLaserControlApp.h"

// MODBUS Lib
#include "MODBUS/CMBSender.h"

// Laser Lib
#include "CRelayTimer.h"
#include "CSensorADC.h"
#include "LaserLib/CUSART.h"
#include "LaserLib/sys_interrupts.h"

// Utils
#include <math.h>
#include <util/delay.h>

/* Global variables */
CDGUSUSART usart;
CMBSender sender;
CRelayTimer relayTimer;
CSensorADC	adc;
CLaserBoard laserBoard;
CLaserControlApp App;
CDGUSDatabase Database;

extern "C" void __cxa_pure_virtual()
{	
	while(1)
	{
	}
}

void SystemInitialize()
{
	cli();	/* Disable global interrupts */
	
	// Initialize modules
	laserBoard.Init_Relay();
	laserBoard.Relay1Off();
	laserBoard.Relay2Off();
	laserBoard.SetINT1Callback(&CLaserControlApp::OnEncoderStatic, &App);
	laserBoard.SetTIM1Callback(&CLaserControlApp::OnPWMStatic, &App);
	
	// Initialize USART
	usart.Initialize(BAUD_115200, DISABLE, STOP_1BIT, true);
	
	// Initialize timer
	relayTimer.Initialize(WGM_CTC, COM_Clear, CS_DIV1024, false);
	relayTimer.AsyncMode(false);
	relayTimer.SetCOMPACallback(&CLaserControlApp::OnTimerStatic, &App);
	relayTimer.SetCOMPA((uint8_t)125);
	
	// Initialize ADC
	adc.Initialize(REF_AVCC, ADC_CHANNEL0, ADC_PSCL_DIV128, false, true);
	adc.Start();
	
	// Construct MODBUS protocol module
	sender.Initialize(&usart, &App, 256, 256);
	App.Initialize(&sender);
	
	sei();	/* Enable global interrupts */
}
	 
int main(void)
{	
	// Initialization system
	SystemInitialize();
	
	// Startup delay (Beep "Imperial March")
	_delay_ms(2000);
	
	// Initialize application GUI
	App.Start();
	
	// Start main loop
    while (1) 
    {
		// Loop delay
		_delay_ms(1);
		
		// Process application
		static uint16_t prs = 0;
		if ((prs++ % 50) == 0)
			App.Run();
		
		/*	
		{
			static uint16_t cnt = 0;
			if (cnt > 2048) cnt = 0;
			
			PORTB |= (1 << PB2);
			
			_delay_ms(1);
			
			laserBoard.SPI_MasterTransmit(~(uint8_t)(cnt >> 8));
			laserBoard.SPI_MasterTransmit(~(uint8_t)(cnt & 0xff));
			
			PORTB &= ~(1 << PB2);
			
			_delay_ms(1);
			
			cnt++;
		}*/
    }
}