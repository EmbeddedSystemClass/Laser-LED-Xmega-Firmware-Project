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

// Application class
#include "Periphery/CLaserBoard.h"
#include "CLaserControlApp.h"
#include "Periphery/CSoundPlayer.h"

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

extern void ConvertData(void* dst, void* src, uint16_t size, uint16_t offset = 0);

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
	// TimerE0, TimerE1
	player.Initialize();
	dacSPI.Initialize(true, SPI_DORD_MSBtoLSB, SPI_MODE_LFSTP_TRSMP, false, SPI_PRESCALER_DIV128_gc);
	usart.Initialize(BAUD_115200_ERM0P1, PARITY_DISABLE, STOPBITS_1BIT, true);
	sender.Initialize(&usart, &App, 256, 256);
	// TimerC0, TimerF0
	App.Initialize(&sender);
	laserBoard.InitializeClock();
	
	sei();	/* Enable global interrupts */
}

uint16_t DATA[1024];

uint16_t swap(uint16_t data)
{
	return (data >> 8) | (data << 8);
}
	 
int main(void)
{
	float x = 0.0f;
	
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
		
		laserBoard.PortCheck();
		
		// Process application
		static uint16_t prs = 0;
		if ((prs++ % 200) == 0)
		{
			App.Run();
			
			DGUS_WRITETOFLASH flash_cmd;
			flash_cmd.en = 0x5A;
			flash_cmd.op = 0x50;
			flash_cmd.addr = 0x00009000; //400 Database.DAT
			flash_cmd.vp = swap(0x0400);
			flash_cmd.len = swap(32);
			
			              //                                *
			char str[33] = "Hello world! Vlad!Maya! 1234567!";
			char dst[33];
			
			ConvertData(dst, str, 33);
			
			sender.WriteDataToRegister(STRUCT_ADDR_WRITETOFLASH,  (uint8_t*)&flash_cmd, sizeof(flash_cmd));
			sender.WaitMODBUSTransmitter();
			
			sender.WriteDataToSRAMAsync(0x0400, (uint16_t*)dst, 32);
			sender.WaitMODBUSTransmitter();
			
			/*flash_cmd.en = 0;
			sender.WriteDataToRegister(STRUCT_ADDR_WRITETOFLASH,  (uint8_t*)&flash_cmd, 1);
			sender.WaitMODBUSTransmitter();*/
					
			sender.StartMODBUSRegisterTransaction(STRUCT_ADDR_WRITETOFLASH,  1);
			sender.WaitMODBUSTransmitter();
			sender.WaitMODBUSListener();
		}

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