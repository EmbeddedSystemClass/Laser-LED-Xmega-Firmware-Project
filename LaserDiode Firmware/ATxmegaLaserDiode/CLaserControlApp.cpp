/* 
* CLaserControlApp.cpp
*
* Created: 11.04.2016 15:27:29
* Author: TPU_2
*/


#include "CLaserControlApp.h"
#include "Periphery/CSoundPlayer.h"
#include "CDGUSDatabase.h"
#include <string.h>
#include <util/delay.h>

CTimerC timer;
CTimerF laserTimer;
extern CDGUSDatabase Database;
extern CLaserBoard laserBoard;
extern CSoundPlayer player;
extern CSPI dacSPI;
extern int temperature;

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
	
	if (length == 2)
	{
		switch (addr)
		{
			case VARIABLE_ADDR_MODE:		laserDiodeData.mode = val;							break;
			case VARIABLE_ADDR_FREQ:		laserDiodeData.laserprofile.Frequency = val;		break;
			case VARIABLE_ADDR_DURATION:	laserDiodeData.laserprofile.Duration = val;			break;
			case VARIABLE_ADDR_ENERGYPCT:	laserDiodeData.laserprofile.EnergyPercent = val;	break;
			case VARIABLE_ADDR_POWER:		laserDiodeData.lasersettings.Power = val;			break;
			case VARIABLE_ADDR_ENERGY:		laserDiodeData.lasersettings.Energy = val;			break;
			case VARIABLE_ADDR_DUTYCYCLE:	laserDiodeData.lasersettings.DutyCycle = val;		break;
			case VARIABLE_ADDR_LASERCNT:	laserDiodeData.PulseCounter = val;					break;
			case VARIABLE_ADDR_MELANIN:		laserDiodeData.melanin = val;						break;
			case VARIABLE_ADDR_PHOTOTYPE:	laserDiodeData.phototype = val;						break;
			case VARIABLE_ADDR_TEMPER:		laserDiodeData.temperature = val;					break;
			case VARIABLE_ADDR_COOLING:		laserDiodeData.cooling = val;						break;
			case VARIABLE_ADDR_FLOW:		laserDiodeData.flow = val;							break;
			case VARIABLE_ADDR_TIMMIN:		laserDiodeData.timer.timer_minutes = val;			break;
			case VARIABLE_ADDR_TIMSEC:		laserDiodeData.timer.timer_seconds = val;			break;
			case VARIABLE_ADDR_DATAOFFS:	laserDiodeData.DatabasePageOffset = val;			break;
			case VARIABLE_ADDR_DATAINDEX:	laserDiodeData.DatabaseSelectionIndex = val;		break;
		}
	} else
	{
		if (addr == STRUCT_ADDR_LASERDIODE_DATA)
			ConvertData((void*)&laserDiodeData, (void*)data, length);
		
		if (addr == STRUCT_ADDR_LASERPROSETTINGS_DATA)
			ConvertData((void*)&m_structLaserSettings, (void*)data, length);
		
		if (addr == STRUCT_ADDR_LASERPROFILE_DATA)
		{
			if (Profile <= 5)
				ConvertData((void*)&m_structLaserProfile[Profile], (void*)data, length);
			else
				CLaserBoard::Beep();
		}
	}
}

volatile uint8_t DatabaseStatusRegister;

void CLaserControlApp::OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length)
{
	// Update GUI registers
	if (addr == 0x03)	PIC_ID = data[1];
	
	if (addr == REGISTER_DATABASE_EN)
		DatabaseStatusRegister = data[0];
	
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
		case PICID_DATABASEOnRead:
			state = APP_READPROFILE;
		break;
		case PICID_DATABASEOnSave:
			state = APP_SAVEPROFILE;
		break;
		case PICID_DATABASE:
			state = APP_DATABASE;
		break;
		case PICID_NEWPROFILE:
			state = APP_UNMAPDATABASE;
		break;;
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
	
	// Initialize prepare timer
	timer.Initialize(WGM_Normal, CS_DIV256);
	timer.SetPeriod(25000); // Every 10 ms
	timer.SetOVFCallback(OnTimerStatic, this, TC_OVFINTLVL_LO_gc); // Enable interrupt
	timer.Start(25000);
	
	// Set global variables
	PIC_ID = 0;
	update = false;
	prepare = true;
	m_wSetMin = 0;
	m_wSetSec = 5;
	m_wMillSec = 0;
	m_wMinutes = m_wSetMin;
	m_wSeconds = m_wSetSec;
	
	// Fast profile
	m_structLaserProfile[PROFILE_FAST].Frequency = 10;			// 10 Hz
	m_structLaserProfile[PROFILE_FAST].Duration  = 50;			// ms
	m_structLaserProfile[PROFILE_FAST].EnergyPercent = 100;		// W
	
	// Medium profile
	m_structLaserProfile[PROFILE_MEDIUM].Frequency = 5;			// 10 Hz
	m_structLaserProfile[PROFILE_MEDIUM].Duration  = 100;		// ms
	m_structLaserProfile[PROFILE_MEDIUM].EnergyPercent = 100;	// W
	
	// Slow profile
	m_structLaserProfile[PROFILE_SLOW].Frequency = 2;			// 10 Hz
	m_structLaserProfile[PROFILE_SLOW].Duration  = 200;			// ma
	m_structLaserProfile[PROFILE_SLOW].EnergyPercent = 100;		// W
	
	// Single profile
	m_structLaserProfile[PROFILE_SINGLE].Frequency = 1;			// 10 Hz
	m_structLaserProfile[PROFILE_SINGLE].Duration  = 400;		// ms
	m_structLaserProfile[PROFILE_SINGLE].EnergyPercent = 100;	// W
	
	// Current profile
	Profile = PROFILE_FAST;
	
	// Set all laser settings
	laserDiodeData.mode = Profile;
	// Ебаный GCC со своей строгой типизацией для volatile просто достал!!!
	memcpy((void*)&laserDiodeData.laserprofile, (void*)&m_structLaserProfile[PROFILE_FAST], sizeof(DGUS_LASERPROFILE));
	laserDiodeData.lasersettings = CalculateLaserSettings((DGUS_LASERPROFILE*)&m_structLaserProfile[PROFILE_FAST]);
	laserDiodeData.timer.timer_minutes = m_wSetMin;
	laserDiodeData.timer.timer_seconds = m_wSetSec;
	laserDiodeData.PulseCounter = swap32(10000000);
	laserDiodeData.melanin = 0;
	laserDiodeData.phototype = 0;
	laserDiodeData.temperature = 24;
	laserDiodeData.cooling = 6;
	laserDiodeData.flow = 10;
	laserDiodeData.DatabasePageOffset = 0;
	laserDiodeData.DatabaseSelectionIndex = 13;
	
	// Initialize Laser timer
	laserTimer.Initialize(WGM_SingleSlopePWM, CS_DIV1024);
	laserTimer.SetPeriod(laserTimerPeriod);		// 10 Hz
	laserTimer.SetCOMPA(laserTimerDutyCycle);	// 50 ms, 50% duty cycle
	laserTimer.SetCOMPB(laserTimerDutyCycle);	// 50 ms, 50% duty cycle
	laserTimer.SetOVFCallback(OnLaserTimerStatic, this, TC_OVFINTLVL_LO_gc);
	laserTimer.EnableChannel(TIMER_CHANNEL_A);	// Enable Laser TTL Gate
	laserTimer.EnableChannel(TIMER_CHANNEL_B);	// Enable Laser TTL Gate
	laserTimer.ChannelSet(TIMER_CHANNEL_A);
	laserTimer.ChannelSet(TIMER_CHANNEL_B);
}

void CLaserControlApp::Start()
{	
	// Startup DGUS initialization
	uint16_t pic_id = swap(PICID_MAINMENU);
	m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
	m_cpSender->WaitMODBUSTransmitter();
	
	// Setup variables
	m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_LASERDIODE_DATA, (uint16_t*)&laserDiodeData, sizeof(DGUS_LASERDIODE));
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
	
	SetVariable(VARIABLE_ADDR_TEMPER, (uint16_t*)&temperature, 2);
	
	switch (state)
	{		
		// DGUS State
		case APP_LOGO:
			{
				SetPictureId(PICID_MAINMENU);
				state = APP_WORKIDLE;
			}
		break;
		case APP_WORKSETUP:
			{
				GetVariable(STRUCT_ADDR_LASERPROFILE_DATA, sizeof(DGUS_LASERPROFILE));
				_delay_ms(50);
				GetVariable(VARIABLE_ADDR_MODE, 2);
				_delay_ms(50);
				
				APP_PROFILE prof = (APP_PROFILE)laserDiodeData.mode;
				if (Profile != prof)	{update = true; Profile = prof;}
				
				laserDiodeData.lasersettings = CalculateLaserSettings((DGUS_LASERPROFILE*)&m_structLaserProfile[Profile]);
				SetVariable(STRUCT_ADDR_LASERPROSETTINGS_DATA, (uint16_t*)&laserDiodeData.lasersettings,  sizeof(DGUS_LASERSETTINGS));
			}
		break;
		case APP_WORKPREPARE:
			{
				DGUS_PREPARETIMER timervar;
				if (prepare)
				{
					timervar.timer_minutes = m_wMinutes;
					timervar.timer_seconds = m_wSeconds;
					SetVariable(STRUCT_ADDR_PREPARETIMER_DATA, (uint16_t*)&timervar, sizeof(timervar));
				}
				else
					SetPictureId(PICID_WORK_IDLE);
			}
		break;
		case APP_WORKLIGHT:
		case APP_WORKPOWERON:
			{				
				if (!laserBoard.Footswitch())
				{
					laserTimer.SetCOMPA(laserTimerDutyCycle);
					laserTimer.SetCOMPB(laserTimerDutyCycle);
					laserTimer.Start(laserTimerPeriod);
					// Show power on
					if (state != APP_WORKLIGHT)
					{
						SetPictureId(PICID_WORK_STARTED);
						state = APP_WORKLIGHT;
					}
				}
				else
				{
					laserTimer.Stop();
					laserTimer.ChannelSet(TIMER_CHANNEL_A);
					laserTimer.ChannelSet(TIMER_CHANNEL_B);
					// Show power on
					if (state != APP_WORKPOWERON)
					{
						SetPictureId(PICID_WORK_POWERON);
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
				
				SetPictureId(PICID_WORK_READY);
			}
		break;
		case APP_WORKOnPowerOn:
			{
				uint16_t data = ((uint16_t)((laserPower * 64) / 63)) << 2;  // (laserPower * 1024) / 1000)
				dacSPI.Send((uint8_t*)&data, sizeof(data));
				
				SetPictureId(PICID_WORK_POWERON);
			}
		break;
		case APP_WORKOnPowerOff:
			{
				SetPictureId(PICID_WORK_IDLE);
				
				laserTimer.Stop();
				laserTimer.ChannelSet(TIMER_CHANNEL_A);
				laserTimer.ChannelSet(TIMER_CHANNEL_B);
				laserBoard.LaserPowerOff();
				laserBoard.Relay2Off();
				
				uint16_t data = 0;
				dacSPI.Send((uint8_t*)&data, sizeof(data));
			}
		break;
		
		// Database
		case APP_DATABASE:
			GetVariable(VARIABLE_ADDR_DATAOFFS, 2);
			_delay_ms(50);
			GetVariable(VARIABLE_ADDR_DATAINDEX, 2);
			_delay_ms(50);
			
			Database.MapDatabaseToRead(VARIABLE_ADDR_DATABASE, DGUS_DATABASE_ADDR + laserDiodeData.DatabasePageOffset * PROFILE_SIZE, 0x0C00);
			
			/*if (laserDiodeData.DatabaseSelectionIndex != 13)
			{				
				SetPictureId(PICID_DATABASEOnRead);
				
				_delay_ms(500);
				
				uint16_t index = 13;
				SetVariable(VARIABLE_ADDR_DATAINDEX, &index, 2);
			}*/
		break;
		case APP_READPROFILE:
			_delay_ms(1000);
			
			GetVariable(VARIABLE_ADDR_DATAINDEX, 2);
			
			Database.MapDatabaseToRead(
				VARIABLE_ADDR_PROFILE, 
				DGUS_DATABASE_ADDR + (laserDiodeData.DatabasePageOffset + laserDiodeData.DatabaseSelectionIndex) * PROFILE_SIZE, 0x0100);
		break;
		case APP_SAVEPROFILE:
			Database.MapDatabaseToWrite(
				VARIABLE_ADDR_PROFILE,
				DGUS_DATABASE_ADDR + (laserDiodeData.DatabasePageOffset + laserDiodeData.DatabaseSelectionIndex) * PROFILE_SIZE, 0x0100);
		break;
		case APP_UNMAPDATABASE:
			Database.UnMap();
		break;
		default:
			Database.UnMap();
		break;
	}
	
	if (update)
	{		
		SetVariable(STRUCT_ADDR_LASERPROFILE_DATA, (uint16_t*)&m_structLaserProfile[Profile], sizeof(DGUS_LASERPROFILE));
		update = false;
	}
}

DGUS_LASERSETTINGS CLaserControlApp::CalculateLaserSettings(DGUS_LASERPROFILE *profile)
{
	DGUS_LASERSETTINGS result;
	
	uint32_t period = 1000000ul / (uint32_t)profile->Frequency;	//  period [us]
	uint32_t dutycycle = period / profile->Duration;			//  period [us] / duration [ms]
	result.DutyCycle = uint16_t(dutycycle / 10);				// (period [us] * 100) / (duration [ms] * 1000)
	result.Energy = uint16_t((dutycycle * (uint32_t)profile->EnergyPercent * (uint32_t)MAX_LASER_POWER) / 100000ul);
	result.Power = (profile->EnergyPercent * MAX_LASER_POWER) / 100;
	
	// calculate timer settings
	laserTimerPeriod = (6250 / profile->Frequency) * 10;
	laserTimerDutyCycle = laserTimerPeriod - ((laserTimerPeriod / 100) * result.DutyCycle);
	laserTimerDutyCyclems = uint16_t(period / 1000);
	
	return result;
}

void CLaserControlApp::GetVariable(uint16_t addr, uint16_t size)
{
	m_cpSender->StartMODBUSVariableTransaction(addr, size);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WaitMODBUSListener();
}

void CLaserControlApp::SetVariable(uint16_t addr, uint16_t* data, uint16_t size)
{
	m_cpSender->WriteDataToSRAMAsync(addr, data, size);
	m_cpSender->WaitMODBUSTransmitter();
}

void CLaserControlApp::SetPictureId(uint16_t pic_id)
{
	uint16_t pic = swap(pic_id);
	m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic, 2);
	m_cpSender->WaitMODBUSTransmitter();
}

void CLaserControlApp::OnTimer()
{
	if (m_wMillSec == 0)
	{
		if (m_wSeconds == 0)
		{
			if (m_wMinutes == 0)
			{
				OnTimeout();
				/*player.SoundStart(1000, 1000, 2);
				player.SoundStop();*/
				player.SoundStart(261, 100, 2);
				player.SoundStop();
				player.SoundStart(294, 100, 2);
				player.SoundStop();
				player.SoundStart(329, 100, 2);
				player.SoundStop();
				player.SoundStart(349, 100, 2);
				player.SoundStop();
				
				//player.beep(1000, 1000);
				return;
			}
			m_wSeconds = 60;
			m_wMinutes--;
		}
		m_wMillSec = 100; // Every 10 ms
		m_wSeconds--;
		/*if (m_wMinutes == 0 && m_wSeconds < 10)
		{
			if (m_wMinutes == 0 && m_wSeconds < 5)
			{
				player.SoundStart(1000, 100, 2);
				player.SoundStop();
				//player.beep(1000, 100);
			}
			else
			{
				player.SoundStart(1000, 50, 2);
				player.SoundStop();
				//player.beep(1000, 50);
			}
		}
		else
		{
			player.SoundStart(1000, 25, 2);
			player.SoundStop();
			//player.beep(1000, 25);
		}*/
	}
	m_wMillSec-=10;
}

void CLaserControlApp::OnLaserTimer()
{
	if (laserTimerDutyCyclems > 100)
		player.SoundStart(1000, 50, 0);
	else
		player.SoundStart(1000, laserTimerDutyCyclems/2, 0);
	player.SoundStop();
}

void CLaserControlApp::OnTimeout()
{
	timer.Stop();
	prepare = false;
	//laserBoard.Relay1Off();
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
