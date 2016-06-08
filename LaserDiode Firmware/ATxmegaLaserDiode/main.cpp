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
#include "Periphery/CDGUSUSART.h"
#include "CDGUSDatabase.h"

// Application class
#include "Periphery/CLaserBoard.h"
#include "CLaserControlApp.h"
#include "Periphery/CSoundPlayer.h"
#include "Periphery/CTimerD.h"
#include "Periphery/CDS18B20.h"

// MODBUS Lib
#include "MODBUS/CMBSender.h"

// Laser Lib
#include "LaserLib/CSPI.h"
#include "LaserLib/CUSART.h"
#include "LaserLib/CTimer.h"
#include "LaserLib/sys_interrupts.h"

//#include "avr/boot.h"

// Utils
#include <math.h>
#include <util/delay.h>

/* Global variables */
CSPI dacSPI;
CTimerD timeout;
CDGUSUSART usart;
CDGUSDatabase Database;
CMBSender sender;
CLaserBoard laserBoard;
CSoundPlayer player;
CLaserControlApp App;
CDS18B20 D18B20;

int temperature = 0;

extern "C" void __cxa_pure_virtual()
{	
	while(1)
	{
		laserBoard.BeepClassError();
	}
}

void SystemInitialize()
{
	cli();	/* Disable global interrupts */
	
	// Initialize modules
	laserBoard.InitializeIO();
	D18B20.Initialize();
	// TimerE0, TimerE1
	player.Initialize();
	// TimerD0
	timeout.Initialize(WGM_Normal, CS_DIV1024);
	timeout.SetPeriod(31250); //1s timeout
	
	dacSPI.Initialize(true, SPI_DORD_MSBtoLSB, SPI_MODE_LFSTP_TRSMP, false, SPI_PRESCALER_DIV128_gc);
	usart.Initialize(BAUD_115200_ERM0P1, PARITY_DISABLE, STOPBITS_1BIT, true);
	sender.Initialize(&timeout, &usart, &App, 256, 256, 31250);
	Database.Initialize(&sender, VARIABLE_ADDR_DATABASE);
	// TimerC0, TimerF0
	App.Initialize(&sender);
	laserBoard.InitializeClock();
	
	sei();	/* Enable global interrupts */
}
	 
int main(void)
{	
	// Initialization system
	SystemInitialize();
	
	// Startup delay (Beep "Imperial March")
	//player.Play();
	_delay_ms(1000);
	laserBoard.Relay1On();
	_delay_ms(100);
	laserBoard.Relay1Off();
	
	uint16_t data = 0;
	dacSPI.Send((uint8_t*)&data, sizeof(data));
	
	// Initialize application GUI
	App.Start();
	
	// Start main loop
    while (1) 
    {
		// Loop delay
		_delay_ms(1);
		
		//laserBoard.PortCheck();
		
		// Process application
		static uint16_t prs = 0;
		if ((prs++ % 100) == 0)
		{
			App.Run();
			
			while (dacSPI.transmitterState() > 0);
			dacSPI.Deinitialize();
			//_delay_ms(1);
			temperature = D18B20.temp_18b20();
			//_delay_ms(1);
			dacSPI.Initialize(true, SPI_DORD_MSBtoLSB, SPI_MODE_LFSTP_TRSMP, false, SPI_PRESCALER_DIV128_gc);
		}
    }
}