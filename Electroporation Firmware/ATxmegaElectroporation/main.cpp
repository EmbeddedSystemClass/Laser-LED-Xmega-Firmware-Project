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

DGUS_PROFILE empty_record;
	 
int main(void)
{
	float x = 0.0f;
	char empty_name[] = "Empty\0";
	char empty_time[] = "00:00\0";
	
	empty_record.ID = 0;
	ConvertData(empty_record.Name, empty_name, 6, 0);
	ConvertData(empty_record.Time, empty_time, 6, 0);
	empty_record.Power = 0;
	
	
	// Initialization system
	SystemInitialize();
	
	
	uint32_t flash_addr = 0x00900000;
	for (uint32_t i = 0; i < 16; i++)
	{
		Database.MapDatabaseToWrite(0x0100, flash_addr, 0x4000);
		flash_addr += (uint32_t)0x4000;
		
		// Initialize Empty database
		for (uint32_t j = 0; j < 64; j++)
		{
			empty_record.ID = (uint16_t)(i * 64 + j);
			sender.WriteDataToSRAM(0x0100 + j * 0x0100, (uint16_t*)&empty_record, (uint16_t)sizeof(empty_record));
		}

		Database.UnMap();
		
		//_delay_ms(1000);
	}
	
	// Startup delay (Beep "Imperial March")
	//player.Play();
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
		if ((prs++ % 200) == 0)
			App.Run();
    }
}