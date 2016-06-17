/*
 * ATxmegaLaserDiode.cpp
 *
 * Created: 01.04.2016 20:57:39
 * Author : Vladislav
 */

// Xmega
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

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
CTimerC timer;
CTimerD1 timeout;
CTimerF laserTimer;
CTimerC1 flowtimer;
CTimerD pwmtimer; // Cooling PWM
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
	//LASER_CNT_EEPROMADDR
	//eeprom_write_dword((uint32_t*)LASER_CNT_EEPROMADDR, 0);
	
	cli();	/* Disable global interrupts */
	
	EVSYS.CH0MUX = EVSYS_CHMUX_PORTC_PIN4_gc;
	
	// Initialize modules
	laserBoard.InitializeIO();
	D18B20.Initialize();
	// TimerE0, TimerE1
	player.Initialize();
	// TimerC0 Initialize prepare timer
	timer.Initialize(WGM_Normal, CS_DIV256);
	timer.SetPeriod(25000); // Every 10 ms
	timer.SetOVFCallback(App.OnTimerStatic, &App, TC_OVFINTLVL_LO_gc); // Enable interrupt
	// TimerD1
	timeout.Initialize(WGM_Normal, CS_DIV1024);
	timeout.SetPeriod(31250); //1s timeout
	// TimerC1
	flowtimer.Initialize(WGM_Normal, CS_EventChannel0);
	flowtimer.SetPeriod(65535);
	flowtimer.Start(65535);
	// TimerD0
	pwmtimer.Initialize(WGM_SingleSlopePWM, CS_DIV64);//CS_DIV256);
	pwmtimer.SetCOMPA(512);
	laserBoard.PWMOn();
	laserBoard.REDOn();
	laserBoard.GRNOn();
	laserBoard.BLUOn();
	//pwmtimer.EnableChannel(TIMER_CHANNEL_A);
	pwmtimer.SetOVFCallback(App.OnPWMTimerOVFStatic, &App, TC_OVFINTLVL_LO_gc);
	pwmtimer.SetCOMPACallback(App.OnPWMTimerCMPStatic, &App, TC_CCAINTLVL_LO_gc);
	// Set RGB Led control
	pwmtimer.SetCOMPB(256);
	pwmtimer.SetCOMPC(512);
	pwmtimer.SetCOMPD(768);
	pwmtimer.SetCOMPBCallback(App.OnPWMTimerREDStatic, &App, TC_CCBINTLVL_HI_gc);
	pwmtimer.SetCOMPCCallback(App.OnPWMTimerGRNStatic, &App, TC_CCCINTLVL_HI_gc);
	pwmtimer.SetCOMPDCallback(App.OnPWMTimerBLUStatic, &App, TC_CCDINTLVL_HI_gc);
	pwmtimer.Start(1024);
	
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

DGUS_PROFILE empty_record;
	 
int main(void)
{
	//eeprom_write_dword((uint32_t*)LASER_CNT_EEPROMADDR, 0);
	// Initialization system
	SystemInitialize();
	
	/*float x = 0.0f;
	char empty_name[] = "Empty\0";
	char empty_time[] = "00:00\0";
	
	empty_record.ID = 0;
	ConvertData(empty_record.Name, empty_name, 6, 0);
	ConvertData(empty_record.Time, empty_time, 6, 0);
	empty_record.Power = 0;
	
	uint32_t flash_addr = DGUS_DATABASE_ADDR;
	for (uint32_t i = 0; i < 16; i++)
	{
		// Initialize Empty database
		for (uint32_t j = 0; j < 64; j++)
		{
			empty_record.ID = (uint16_t)(i * 64 + j);
			sender.WriteDataToSRAM(0x0100 + j * 0x0100, (uint16_t*)&empty_record, (uint16_t)sizeof(empty_record));
		}
		
		_delay_ms(200);
		
		Database.MapDatabaseToWrite(0x0100, flash_addr, 0x4000);
		flash_addr += (uint32_t)0x4000;
		
		_delay_ms(1000);
		
		Database.UnMap();
		
		_delay_ms(200);
	}*/
	
	// Startup delay (Beep "Imperial March")
	//player.Play();
	_delay_ms(5000);
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
		//static uint16_t prs = 0;
		//if ((prs++ % 5) == 0)
		{
			App.Run();
			
			//while (dacSPI.transmitterState() > 0);
			//dacSPI.Deinitialize();
			//_delay_ms(1);
			
			static uint16_t prs = 0;
			if ((prs++ % 15) == 0)
				temperature = D18B20.temp_18b20();
			
			//_delay_ms(1);
			//dacSPI.Initialize(true, SPI_DORD_MSBtoLSB, SPI_MODE_LFSTP_TRSMP, false, SPI_PRESCALER_DIV128_gc);
			
			if (temperature > 265)
				laserBoard.Relay2On();
			
			if (temperature <255)
				laserBoard.Relay2Off();			
		}
    }
}