/* 
* CLaserControlApp.cpp
*
* Created: 11.04.2016 15:27:29
* Author: TPU_2
*/


#include "CLaserControlApp.h"
#include "CSoundPlayer.h"
#include <string.h>
#include <util/delay.h>

CTimerC timer;
CTimerF laserTimer;
extern CLaserBoard laserBoard;
extern CSoundPlayer player;

volatile DGUS_DATA m_structDGUSDATA_Fast;
volatile DGUS_DATA m_structDGUSDATA_Medium;
volatile DGUS_DATA m_structDGUSDATA_Slow;

uint16_t swap(uint16_t data)
{
	return (data >> 8) | (data << 8);
}

void ConvertData(void* dst, void* src, uint16_t size)
{
	uint16_t  length = size / 2;
	uint16_t* source = (uint16_t*)src;
	uint16_t* dest = (uint16_t*)dst;
	
	// swap bytes in words
	for (uint16_t i = 0; i < length; i++)
		dest[i] = swap(source[i]);
}

uint16_t min(uint16_t x, uint16_t y)
{
	if (x < y)
		return x;
	else
		return y;
}

uint16_t max(uint16_t x, uint16_t y)
{
	if (x > y)
		return x;
	else
		return y;
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
	//uint16_t val = swap(*((uint16_t*)data));
	
	if (addr == STRUCT_ADDR_DATA)
	{
		//memcpy((void*)&m_structDGUSDATA_Fast, (void*)data, length);
		//ConvertData((void*)&m_structDGUSDATA_Fast, (void*)data, length);
		switch (profile)
		{
			case WorkFast:
				ConvertData((void*)&m_structDGUSDATA_Fast, (void*)data, length/2);
			break;
			case WorkSlow:
				ConvertData((void*)&m_structDGUSDATA_Slow, (void*)data, length/2);
			break;
			case WorkMedium:
				ConvertData((void*)&m_structDGUSDATA_Medium, (void*)data, length/2);
			break;
			default:
				// Error
				CLaserBoard::Beep();
			break;
		}
	}
}

void CLaserControlApp::OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length)
{
	// Update GUI registers
	if (addr == 0x03)	PIC_ID = data[1];
	
	switch (PIC_ID)
	{
		case PICID_LOGO:
			state = APP_LOGO;
		break;
		case PICID_WORKFAST:
			state = APP_WORKFAST;
			if (profile != WorkFast)
			{
				profile = WorkFast;
				update = true;
			}
		break;
		case PICID_WORKMEDIUM:
			state = APP_WORKMEDIUM;
			if (profile != WorkMedium)
			{
				profile = WorkMedium;
				update = true;
			}
		break;
		case PICID_WORKSLOW:
			state = APP_WORKSLOW;
			if (profile != WorkSlow)
			{
				profile = WorkSlow;
				update = true;
			}
		break;
		case PICID_WORKOnReady:
			state = APP_WORKOnReady;
		break;
		case PICID_WORKSTART:
			state = APP_WORKSTART;
		break;
		case PICID_WORKOnStart:
			state = APP_WORKOnStart;
		break;
		case PICID_WORKSTARTED:
			state = APP_WORKSTARTED;
		break;
		case PICID_PHOTOTYPESELECT:
			state = APP_PHOTOTYPESELECT;
		break;
		case PICID_PHOTOTYPE1:
			state = APP_PHOTOTYPE1;
		break;
		case PICID_PHOTOTYPE2:
			state = APP_PHOTOTYPE2;
		break;
		case PICID_PHOTOTYPE3:
			state = APP_PHOTOTYPE3;
		break;
		case PICID_PHOTOTYPE4:
			state = APP_PHOTOTYPE4;
		break;
		case PICID_PHOTOTYPE5:
			state = APP_PHOTOTYPE5;
		break;
		case PICID_PHOTOTYPE6:
			state = APP_PHOTOTYPE6;
		break;
		default:
			//state = APP_SETUPtoRUN_ANIM;
		break;
	}
}

// Initialization
void CLaserControlApp::Initialize(CMBSender* sender)
{
	// GUI class initialization
	m_cpSender = sender;
	
	// Initialize timer
	timer.Initialize(WGM_Normal, CS_DIV256);
	timer.SetPeriod(25000); // Every 10 ms
	timer.SetOVFCallback(OnTimerStatic, this, TC_OVFINTLVL_LO_gc); // Enable interrupt
	
	// Initialize Laser timer
	laserTimer.Initialize(WGM_SingleSlopePWM, CS_DIV1024);
	laserTimer.SetPeriod(12500);	// 10 Hz
	laserTimer.SetCOMPA(3125);	// 50 ms, 50% duty cycle
	laserTimer.SetOVFCallback(OnLaserTimerStatic, this, TC_OVFINTLVL_LO_gc);
	laserTimer.EnableChannel(TIMER_CHANNEL_A); // Enable Laser TTL Gate
	
	// Set global variables
	PIC_ID = 0;
	update = false;
	
	// Fast profile
	m_structDGUSDATA_Fast.Frequency = 10; // 10 Hz
	m_structDGUSDATA_Fast.DutyCycle = 50; // 50%
	m_structDGUSDATA_Fast.Duration  = ((1000 / m_structDGUSDATA_Fast.Frequency) * m_structDGUSDATA_Fast.DutyCycle) / 100; // ms
	m_structDGUSDATA_Fast.Intensity = 200; // W
	m_structDGUSDATA_Fast.Power     = (m_structDGUSDATA_Fast.Intensity * m_structDGUSDATA_Fast.DutyCycle) / 100; // W
	m_structDGUSDATA_Fast.Energy    = (m_structDGUSDATA_Fast.Intensity * m_structDGUSDATA_Fast.Duration) / 1000; // J
	
	// Slow profile
	m_structDGUSDATA_Slow.Frequency = 10; // 10 Hz
	m_structDGUSDATA_Slow.DutyCycle = 50; // 50%
	m_structDGUSDATA_Slow.Duration  = ((1000 / m_structDGUSDATA_Slow.Frequency) * m_structDGUSDATA_Slow.DutyCycle) / 100; // ms
	m_structDGUSDATA_Slow.Intensity = 200; // W
	m_structDGUSDATA_Slow.Power     = (m_structDGUSDATA_Slow.Intensity * m_structDGUSDATA_Slow.DutyCycle) / 100; // W
	m_structDGUSDATA_Slow.Energy    = (m_structDGUSDATA_Slow.Intensity * m_structDGUSDATA_Slow.Duration) / 1000; // J
	
	// Medium profile
	m_structDGUSDATA_Medium.Frequency = 10; // 10 Hz
	m_structDGUSDATA_Medium.DutyCycle = 50; // 50%
	m_structDGUSDATA_Medium.Duration  = ((1000 / m_structDGUSDATA_Medium.Frequency) * m_structDGUSDATA_Medium.DutyCycle) / 100; // ms
	m_structDGUSDATA_Medium.Intensity = 200; // W
	m_structDGUSDATA_Medium.Power     = (m_structDGUSDATA_Medium.Intensity * m_structDGUSDATA_Medium.DutyCycle) / 100; // W
	m_structDGUSDATA_Medium.Energy    = (m_structDGUSDATA_Medium.Intensity * m_structDGUSDATA_Medium.Duration) / 1000; // J
	
	// Current profile
	profile = WorkFast;
}

void CLaserControlApp::Start()
{	
	// Startup DGUS initialization
	uint16_t pic_id = swap(1);
	m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
	m_cpSender->WaitMODBUSTransmitter();
	
	//Setup variables
	m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_DATA, (uint16_t*)&m_structDGUSDATA_Fast, sizeof(m_structDGUSDATA_Fast));
	m_cpSender->WaitMODBUSTransmitter();
	
	state = APP_WORKFAST;
}

// Process GUI
void CLaserControlApp::Run()
{
	// Get PIC ID
	m_cpSender->StartMODBUSRegisterTransaction(REGISTER_ADDR_PICID, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WaitMODBUSListener();
	_delay_ms(50);
	
	if (!update)
	{
		// Get variables 
		m_cpSender->StartMODBUSVariableTransaction(STRUCT_ADDR_DATA, sizeof(DGUS_DATA));
		m_cpSender->WaitMODBUSTransmitter();
		m_cpSender->WaitMODBUSListener();
		_delay_ms(50);
	}
	
	switch (state)
	{		
		// DGUS State
		case APP_LOGO:
			state = APP_WORKFAST;
		break;
		case APP_WORKFAST:
			state = APP_WORKFAST;
		break;
		case APP_WORKMEDIUM:
			state = APP_WORKFAST;
		break;
		case APP_WORKSLOW:
			state = APP_WORKFAST;
		break;
		case APP_WORKSTART:
			state = APP_WORKFAST;
		break;
		case APP_WORKSTARTED:
			state = APP_WORKFAST;
		break;
		
		// Commands
		case APP_WORKOnReady:
			state = APP_WORKFAST;
		break;
		case APP_WORKOnStart:
			state = APP_WORKFAST;
		break;
		
		// Phototype selector state
		case APP_PHOTOTYPESELECT:
			state = APP_WORKFAST;
		break;
		case APP_PHOTOTYPE1:
			state = APP_WORKFAST;
		break;
		case APP_PHOTOTYPE2:
			state = APP_WORKFAST;
		break;
		case APP_PHOTOTYPE3:
			state = APP_WORKFAST;
		break;
		case APP_PHOTOTYPE4:
			state = APP_WORKFAST;
		break;
		case APP_PHOTOTYPE5:
			state = APP_WORKFAST;
		break;
		case APP_PHOTOTYPE6:
			state = APP_WORKFAST;
		break;
		default:
		break;
	}
	
	DGUS_DATA DGUSDATA;
	
	if (update)
	{
		switch (profile)
		{
			case WorkFast:
				m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_DATA, (uint16_t*)&m_structDGUSDATA_Fast, sizeof(DGUS_DATA));
			break;
			case WorkSlow:
				m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_DATA, (uint16_t*)&m_structDGUSDATA_Slow, sizeof(DGUS_DATA));
			break;
			case WorkMedium:
				m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_DATA, (uint16_t*)&m_structDGUSDATA_Medium, sizeof(DGUS_DATA));
			break;
		}
		m_cpSender->WaitMODBUSTransmitter();
		update = false;
	}
	else
	{
		switch (profile)
		{
			case WorkFast:
				// Fast profile			
				DGUSDATA.DutyCycle = m_structDGUSDATA_Fast.Duration * m_structDGUSDATA_Fast.Frequency / 10;
				DGUSDATA.Power     = (m_structDGUSDATA_Fast.Intensity * m_structDGUSDATA_Fast.DutyCycle) / 100;
				DGUSDATA.Energy    = m_structDGUSDATA_Fast.Intensity * m_structDGUSDATA_Fast.Duration / 1000;
				
				m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_WRITEDATA, (uint16_t*)&DGUSDATA.Power, sizeof(DGUS_WRITEDATA));
			break;
			case WorkSlow:
				// Fast profile
				DGUSDATA.DutyCycle = m_structDGUSDATA_Slow.Duration * m_structDGUSDATA_Slow.Frequency / 10;
				DGUSDATA.Power     = (m_structDGUSDATA_Slow.Intensity * m_structDGUSDATA_Slow.DutyCycle) / 100;
				DGUSDATA.Energy    = m_structDGUSDATA_Slow.Intensity * m_structDGUSDATA_Slow.Duration / 1000;
				
				m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_WRITEDATA, (uint16_t*)&DGUSDATA.Power, sizeof(DGUS_WRITEDATA));
			break;
			case WorkMedium:
				// Fast profile
				DGUSDATA.DutyCycle = m_structDGUSDATA_Medium.Duration * m_structDGUSDATA_Medium.Frequency / 10;
				DGUSDATA.Power     = (m_structDGUSDATA_Medium.Intensity * m_structDGUSDATA_Medium.DutyCycle) / 100;
				DGUSDATA.Energy    = m_structDGUSDATA_Medium.Intensity * m_structDGUSDATA_Medium.Duration / 1000;
				
				m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_WRITEDATA, (uint16_t*)&DGUSDATA.Power, sizeof(DGUS_WRITEDATA));
			break;
		}
		m_cpSender->WaitMODBUSTransmitter();
	}
}

void CLaserControlApp::OnTimer()
{
}

void CLaserControlApp::OnLaserTimer()
{
	player.SoundStart(1000, 5, 0);
	player.SoundStop();
}

void CLaserControlApp::OnTimeout()
{
	timer.Stop();
	laserBoard.Relay1Off();
}

void CLaserControlApp::OnTimerStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnTimer();
}

void CLaserControlApp::OnLaserTimerStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnLaserTimer();
}
