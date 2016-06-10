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
#include "Periphery/CTimerC1.h"
#include "Periphery/CTimerD1.h"
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
CTimerC1 flowtimer;
CTimerD1 pwmtimer;
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
	
	EVSYS.CH0MUX = EVSYS_CHMUX_PORTC_PIN4_gc;
	
	// Initialize modules
	laserBoard.InitializeIO();
	D18B20.Initialize();
	// TimerE0, TimerE1
	player.Initialize();
	// TimerD0
	timeout.Initialize(WGM_Normal, CS_DIV1024);
	timeout.SetPeriod(31250); //1s timeout
	// TimerC1
	flowtimer.Initialize(WGM_Normal, CS_EventChannel0);
	flowtimer.SetPeriod(65535);
	flowtimer.Start(65535);
	// TimerD1
	pwmtimer.Initialize(WGM_SingleSlopePWM, CS_DIV64);
	pwmtimer.Start(1024);
	pwmtimer.SetCOMPA(512);
	pwmtimer.EnableChannel(TIMER_CHANNEL_A);
	pwmtimer.SetOVFCallback(App.OnPWMTimerOVFStatic, &App, TC_OVFINTLVL_LO_gc);
	pwmtimer.SetCOMPACallback(App.OnPWMTimerCMPStatic, &App, TC_CCAINTLVL_LO_gc);
	
	dacSPI.Initialize(true, SPI_DORD_MSBtoLSB, SPI_MODE_LFSTP_TRSMP, false, SPI_PRESCALER_DIV128_gc);
	usart.Initialize(BAUD_115200_ERM0P1, PARITY_DISABLE, STOPBITS_1BIT, true);
	sender.Initialize(&timeout, &usart, &App, 256, 256, 31250);
	Database.Initialize(&sender, VARIABLE_ADDR_DATABASE);
	// TimerC0, TimerF0
	App.Initialize(&sender);
	laserBoard.InitializeClock();
	
	InterruptSenderTable[PORTC_INT0_vect_num] = &App;
	InterruptFuncTable[PORTC_INT0_vect_num] = App.OnINT0Static;
	
	sei();	/* Enable global interrupts */
}
	 
int main(void)
{	
	// Initialization system
	SystemInitialize();
	
	// Startup delay (Beep "Imperial March")
	//player.Play();
	_delay_ms(3000);
	laserBoard.Relay1On();
	_delay_ms(100);
	//laserBoard.Relay1Off();
	
	uint16_t data = 0;
	dacSPI.Send((uint8_t*)&data, sizeof(data));
	
	// Initialize application GUI
	App.Start();
	
	// Start main loop
    while (1) 
    {
		// Loop delay
		//_delay_ms(1);
		
		laserBoard.PortCheck();
		//App.FastRun();
		
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
			
			if (temperature > 265)
				laserBoard.Relay2On();
			else
				laserBoard.Relay2Off();			
		}
    }
}