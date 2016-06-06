/* 
* CLaserControlApp.cpp
*
* Created: 11.04.2016 15:27:29
* Author: TPU_2
*/


#include "CLaserControlApp.h"
#include "Periphery/CSoundPlayer.h"
#include <string.h>
#include <util/delay.h>

CTimerC timer;
CTimerF laserTimer;
extern CLaserBoard laserBoard;
extern CSoundPlayer player;
extern CSPI dacSPI;

volatile DGUS_LASERPROFILE_STRUCT m_structDGUSDATA[4];

extern uint16_t swap(uint16_t data);

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
	uint16_t val = swap(*((uint16_t*)data));
	
	if (addr == VARIABLE_ADDR_DATAINDEX)
		profileIndex = val;
	
	if (addr == VARIABLE_ADDR_DATAOFFS)
		databaseOffset = val;
	
	if (addr == STRUCT_ADDR_LASERPROFILE_DATA)
	{
		if (profile <= 5)
			ConvertData((void*)&m_structDGUSDATA[profile], (void*)data, length);
		else
			CLaserBoard::Beep();
	}
}

volatile uint8_t DatabaseStatusRegister;

void CLaserControlApp::OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length)
{
	// Update GUI registers
	if (addr == 0x03)	PIC_ID = data[1];
	
	if (addr == REGISTER_DATABASE_EN)
	{ 
		DatabaseStatusRegister = data[0];
	}
	
	switch (PIC_ID)
	{
		case PICID_LOGO:
			state = APP_LOGO;
		break;
		case PICID_WORK_PREPARE:
			state = APP_WORKPREPARE;
		break;
		case PICID_WORK_ERROR1:
			// error
		break;
		case PICID_WORK_IDLE:
			state = APP_WORKSETUP;
		break;
		case PICID_WORK_READY:
			state = APP_WORKREADY;
		break;
		case PICID_WORK_POWERON:
			state = APP_WORKPOWERON;
		break;
		case PICID_WORK_STARTED:
			state = APP_WORKLIGHT;
		break;
		case PICID_WORK_NUMPAD:
		case PICID_WORK_PHOTOTYPE:
		break;
		case PICID_WORKOnLaserOff_:
			state = APP_WORKOnPowerOff;
			break;
		case PICID_WORKOnLaserOff:
			state = APP_WORKOnPowerOff;
		break;
		case PICID_WORKOnLaserOn:
			state = APP_WORKOnPowerOn;
		break;
		case PICID_WORKOnReady:
			state = APP_WORKOnReady;
		break;
		case PICID_DATABASE:
		case PICID_DATABASEOnRead:
		case PICID_NEWPROFILE:
		break;
		default:
			state = APP_WORKIDLE;
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
	
	// Set global variables
	PIC_ID = 0;
	update = false;
	
	// Fast profile
	m_structDGUSDATA[PROFILE_FAST].Frequency = 10; // 10 Hz
	m_structDGUSDATA[PROFILE_FAST].DutyCycle = 50; // 50%
	m_structDGUSDATA[PROFILE_FAST].Duration  = ((1000 / m_structDGUSDATA[PROFILE_FAST].Frequency) * m_structDGUSDATA[PROFILE_FAST].DutyCycle) / 100; // ms
	m_structDGUSDATA[PROFILE_FAST].EnergyPercent = 100; // W
	//m_structDGUSDATA[PROFILE_FAST].Power     = (m_structDGUSDATA[PROFILE_FAST].Intensity * m_structDGUSDATA[PROFILE_FAST].DutyCycle) / 100; // W
	//m_structDGUSDATA[PROFILE_FAST].Energy    = (m_structDGUSDATA[PROFILE_FAST].Intensity * m_structDGUSDATA[PROFILE_FAST].Duration) / 1000; // J
	
	// Slow profile
	m_structDGUSDATA[PROFILE_SLOW].Frequency = 1; // 10 Hz
	m_structDGUSDATA[PROFILE_SLOW].DutyCycle = 50; // 50%
	m_structDGUSDATA[PROFILE_SLOW].Duration  = ((1000 / m_structDGUSDATA[PROFILE_SLOW].Frequency) * m_structDGUSDATA[PROFILE_SLOW].DutyCycle) / 100; // ms
	m_structDGUSDATA[PROFILE_SLOW].EnergyPercent = 100; // W
	//m_structDGUSDATA[PROFILE_SLOW].Power     = (m_structDGUSDATA[PROFILE_SLOW].Intensity * m_structDGUSDATA[PROFILE_SLOW].DutyCycle) / 100; // W
	//m_structDGUSDATA[PROFILE_SLOW].Energy    = (m_structDGUSDATA[PROFILE_SLOW].Intensity * m_structDGUSDATA[PROFILE_SLOW].Duration) / 1000; // J
	
	// Medium profile
	m_structDGUSDATA[PROFILE_MEDIUM].Frequency = 5; // 10 Hz
	m_structDGUSDATA[PROFILE_MEDIUM].DutyCycle = 50; // 50%
	m_structDGUSDATA[PROFILE_MEDIUM].Duration  = ((1000 / m_structDGUSDATA[PROFILE_MEDIUM].Frequency) * m_structDGUSDATA[PROFILE_MEDIUM].DutyCycle) / 100; // ms
	m_structDGUSDATA[PROFILE_MEDIUM].EnergyPercent = 100; // W
	//m_structDGUSDATA[PROFILE_MEDIUM].Power     = (m_structDGUSDATA[PROFILE_MEDIUM].Intensity * m_structDGUSDATA[PROFILE_MEDIUM].DutyCycle) / 100; // W
	//m_structDGUSDATA[PROFILE_MEDIUM].Energy    = (m_structDGUSDATA[PROFILE_MEDIUM].Intensity * m_structDGUSDATA[PROFILE_MEDIUM].Duration) / 1000; // J
	
	// Initialize Laser timer
	laserTimerPeriod = (6250 / m_structDGUSDATA[PROFILE_FAST].Frequency) * 10;
	laserTimerDutyCycle = laserTimerPeriod - ((laserTimerPeriod / 100) * m_structDGUSDATA[PROFILE_FAST].DutyCycle);
	
	laserTimer.Initialize(WGM_SingleSlopePWM, CS_DIV1024);
	laserTimer.SetPeriod(laserTimerPeriod);	// 10 Hz
	laserTimer.SetCOMPA(laserTimerDutyCycle);	// 50 ms, 50% duty cycle
	laserTimer.SetCOMPB(laserTimerDutyCycle);	// 50 ms, 50% duty cycle
	laserTimer.SetOVFCallback(OnLaserTimerStatic, this, TC_OVFINTLVL_LO_gc);
	laserTimer.EnableChannel(TIMER_CHANNEL_A); // Enable Laser TTL Gate
	laserTimer.EnableChannel(TIMER_CHANNEL_B); // Enable Laser TTL Gate
	laserTimer.ChannelSet(TIMER_CHANNEL_A);
	laserTimer.ChannelSet(TIMER_CHANNEL_B);
	
	// Current profile
	profile = PROFILE_FAST;
}

void CLaserControlApp::Start()
{	
	// Startup DGUS initialization
	uint16_t pic_id = swap(PICID_WORK_IDLE);
	m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
	m_cpSender->WaitMODBUSTransmitter();
	
	//Setup variables
	m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_LASERPROFILE_DATA, (uint16_t*)&m_structDGUSDATA[PROFILE_FAST], sizeof(m_structDGUSDATA[PROFILE_FAST]));
	m_cpSender->WaitMODBUSTransmitter();
	
	state = APP_WORKSETUP;
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
		m_cpSender->StartMODBUSVariableTransaction(STRUCT_ADDR_LASERPROFILE_DATA, sizeof(DGUS_LASERPROFILE));
		m_cpSender->WaitMODBUSTransmitter();
		m_cpSender->WaitMODBUSListener();
		_delay_ms(50);
	}
	
	switch (state)
	{		
		// DGUS State
		case APP_LOGO:
			{
				uint16_t pic_id = swap(PICID_MAINMENU);
				m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
				m_cpSender->WaitMODBUSTransmitter();
				state = APP_WORKIDLE;
			}
		break;
		case APP_WORKLIGHT:
		case APP_WORKPOWERON:
			{
				uint16_t pic_id = 0;
				
				if (!laserBoard.Footswitch())
				{
					laserTimer.SetCOMPA(laserTimerDutyCycle);
					laserTimer.SetCOMPB(laserTimerDutyCycle);
					laserTimer.Start(laserTimerPeriod);
					// Show power on
					if (state != APP_WORKLIGHT)
					{
						pic_id = swap(PICID_WORK_STARTED);
						
						m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
						m_cpSender->WaitMODBUSTransmitter();
						state = APP_WORKLIGHT;
					}
				}
				else
				{
					laserTimer.Stop();
					laserTimer.ChannelSet(TIMER_CHANNEL_A);
					laserTimer.ChannelSet(TIMER_CHANNEL_B);
					// Show light
					pic_id = swap(PICID_WORK_POWERON);
					// Show power on
					if (state != APP_WORKPOWERON)
					{
						pic_id = swap(PICID_WORK_POWERON);
						
						m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
						m_cpSender->WaitMODBUSTransmitter();
						state = APP_WORKPOWERON;
					}
				}
			}
		break;
		
		// Commands
		case APP_WORKOnReady:
			{
				laserBoard.Relay2On();
				laserBoard.LaserPowerOn();
				
				uint16_t pic_id = swap(PICID_WORK_READY);
				m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
				m_cpSender->WaitMODBUSTransmitter();
				_delay_ms(50);
			}
		break;
		case APP_WORKOnPowerOn:
			{
				uint16_t data = ((uint16_t)((laserPower * 64) / 63)) << 2;  // (laserPower * 1024) / 1000)
				dacSPI.Send((uint8_t*)&data, sizeof(data));
				
				uint16_t pic_id = swap(PICID_WORK_POWERON);
				m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
				m_cpSender->WaitMODBUSTransmitter();
				_delay_ms(50);
			}
		break;
		case APP_WORKOnPowerOff:
			{
				uint16_t pic_id = swap(PICID_WORK_IDLE);
				m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
				m_cpSender->WaitMODBUSTransmitter();
				
				laserTimer.Stop();
				laserTimer.ChannelSet(TIMER_CHANNEL_A);
				laserTimer.ChannelSet(TIMER_CHANNEL_B);
				laserBoard.LaserPowerOff();
				laserBoard.Relay2Off();
				
				uint16_t data = 0;
				dacSPI.Send((uint8_t*)&data, sizeof(data));
			}
		break;
		default:
		break;
	}
	
	DGUS_LASERPROFILE DGUSDATA;
	
	if (update)
	{
		m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_LASERPROFILE_DATA, (uint16_t*)&m_structDGUSDATA[profile], sizeof(DGUS_LASERPROFILE));
		m_cpSender->WaitMODBUSTransmitter();
		update = false;
	}
	else
	{
		// Fast profile
		DGUSDATA.DutyCycle = m_structDGUSDATA[profile].Duration * m_structDGUSDATA[profile].Frequency / 10;
		//DGUSDATA.Power     = (m_structDGUSDATA[profile].Intensity * m_structDGUSDATA[profile].DutyCycle) / 100;
		//DGUSDATA.Energy    = m_structDGUSDATA[profile].Intensity * m_structDGUSDATA[profile].Duration / 1000;
		
		laserTimerPeriod = (6250 / m_structDGUSDATA[profile].Frequency) * 10;
		laserTimerDutyCycle = laserTimerPeriod - ((laserTimerPeriod / 100) * DGUSDATA.DutyCycle);
		laserTimerDutyCyclems = DGUSDATA.DutyCycle;
		laserPower = DGUSDATA.EnergyPercent * 10; //m_structDGUSDATA[profile].Intensity;
		
		m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_LASERPROFILE_DATA, (uint16_t*)&DGUSDATA, sizeof(DGUS_LASERPROFILE));
		m_cpSender->WaitMODBUSTransmitter();
	}
}

void CLaserControlApp::OnTimer()
{
}

void CLaserControlApp::OnLaserTimer()
{
	player.SoundStart(1000, laserTimerDutyCyclems/2, 0);
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
