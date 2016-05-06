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
	
	sei();	/* Enable global interrupts */
}

uint16_t DATA[1024];
	 
int main(void)
{
	float x = 0.0f;
	
	// Initialization system
	SystemInitialize();
	
	// Startup delay (Beep "Imperial March")
	player.Play();
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
		
		// Process application
		static uint16_t prs = 0;
		if ((prs++ % 200) == 0)
			App.Run();
			
		laserBoard.PortCheck();
		
		uint8_t	 database_en   = 0x5A;
		uint8_t  database_op   = 0xA0;
		uint32_t database_addr = 0x00002C00; //0x002C0000; //300 Database.DAT
		uint16_t database_vp   = 0x0400;
		uint16_t database_len  = 1024;
		
		sender.WriteDataToRegister(REGISTER_DATABASE_EN,   (uint8_t*)&database_en, 1);
		sender.WriteDataToRegister(REGISTER_DATABASE_OP,  (uint8_t*) &database_op, 1);
		sender.WriteDataToRegister(REGISTER_DATABASE_ADDR, (uint8_t*)&database_addr, 4);
		sender.WriteDataToRegister(REGISTER_DATABASE_VP,   (uint8_t*)&database_vp, 2);
		sender.WriteDataToRegister(REGISTER_DATABASE_LEN,  (uint8_t*)&database_len, 2);
		
		sender.RequestDataFromSRAMAsync(0x0400, 16);
		
		database_en = 0;
		sender.WriteDataToRegister(REGISTER_DATABASE_EN,   (uint8_t*)&database_en, 1);

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