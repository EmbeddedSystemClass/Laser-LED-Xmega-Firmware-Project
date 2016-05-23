/*
 * ATxmegaLaserDiode.cpp
 *
 * Created: 01.04.2016 20:57:39
 * Author : Vladislav
 */

// Xmega
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

// DGUS
#include "DGUSGUI.h"
#include "CDGUSUSART.h"
#include "CDGUSDatabase.h"

// Application class
#include "CLaserBoard.h"
#include "CLaserControlApp.h"
#include "CSoundPlayer.h"

// MODBUS Lib
#include "MODBUS/CMBSender.h"

// Laser Lib
#include "LaserLib/CSPI.h"
#include "LaserLib/CUSART.h"
#include "LaserLib/sys_interrupts.h"

// Utils
#include <math.h>
#include <util/delay.h>

/* Global variables */
CSPI dacSPI;
CDGUSUSART usart;
CMBSender sender;
CLaserBoard laserBoard;
CSoundPlayer player;
CLaserControlApp App;
CDGUSDatabase Database;

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
	player.Initialize();
	dacSPI.Initialize(true, SPI_DORD_MSBtoLSB, SPI_MODE_LFSTP_TRSMP, false, SPI_PRESCALER_DIV128_gc);
	usart.Initialize(BAUD_115200_ERM0P1, PARITY_DISABLE, STOPBITS_1BIT, true);
	sender.Initialize(&usart, &App, 256, 256);
	App.Initialize(&sender);
	laserBoard.InitializeClock();
	Database.Initialize(&sender, VARIABLE_ADDR_DATABASE);
	
	sei();	/* Enable global interrupts */
}

volatile char str[14];
volatile char strxxxx[14] = "Empty";

DGUS_PROFILE empty_record = {0, "mEtp\0y", "000:\00", 0};
	 
int main(void)
{
	float x = 0.0f;
	
	// Initialization system
	SystemInitialize();
	
	// Startup delay (Beep "Imperial March")
	//player.Play();
	_delay_ms(2000);
	
	// Initialize application GUI
	App.Start();
	
	// Initialize Empty database
	/*for (int i = 0; i < 64; i++)
	{
		sender.WriteDataToSRAM(0x0D00 + i * 0x0100, (uint16_t*)&empty_record, (uint16_t)sizeof(empty_record));
	}*/
	
	// Start main loop
    while (1) 
    {
		// Loop delay
		_delay_ms(1);
		
		// Process application
		static uint16_t prs = 0;
		if ((prs++ % 200) == 0)
			App.Run();
		
		/*for (uint32_t i = 0; i < 64; i++)
		{	
			Database.MapDatabaseToWrite(0x0D00, 0x00900000 + (uint32_t)0x4000 * i, sizeof(empty_record) * 64);
			_delay_ms(1000);
			Database.UnMap();
		}*/

		/*
		// Sine waveform generation
		x += 0.1f;
		if (x > 6.2831853) x = 0.0f;
		float y = 1023.0f * (1.0f + sinf(x)) * 0.5f;
		uint16_t data = ((uint16_t)y) << 2;
	
		// Send to DAC	
		dacSPI.Send((uint8_t*)&data, sizeof(data));*/
    }
}