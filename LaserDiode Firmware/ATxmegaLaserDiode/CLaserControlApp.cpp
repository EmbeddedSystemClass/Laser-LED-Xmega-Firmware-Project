/* 
* CLaserControlApp.cpp
*
* Created: 11.04.2016 15:27:29
* Author: TPU_2
*/


#include "CLaserControlApp.h"
#include "Periphery/CSoundPlayer.h"
#include "Periphery/CTimerC1.h"
#include "Periphery/CTimerD.h"
#include "CDGUSDatabase.h"
#include <string.h>
#include <util/delay.h>
#include <avr/eeprom.h>

//#define FLOW_CONTROL
#define LED_LASER_INDICATOR

extern CTimerC timer;
extern CTimerF laserTimer;
extern CTimerC1 flowtimer;
extern CTimerD pwmtimer;
extern CDGUSDatabase Database;
extern CLaserBoard laserBoard;
extern CSoundPlayer player;
extern CSPI dacSPI;
extern int temperature;

uint16_t MinDurationTable[11] = {100, 100,  20,  20, 20, 10, 10, 10, 10, 10, 10};
uint16_t MaxDurationTable[11] = {400, 400, 120, 120, 100, 80, 70, 70, 60, 50, 40};
uint16_t TableNum[11]         = {0, 7, 11, 11, 9, 8, 7, 7, 6, 5, 4};
uint16_t PowerTable[110]      ={500,	480,	450,	432,	420,	411,	405,	0,		0,		0,		0,
								500,	500,	500,	490,	480,	480,	460,	460,	450,	450,	440,
								500,	500,	500,	490,	490,	490,	480,	470,	465,	450,	440,
								500,	500,	500,	490,	485,	485,	465,	465,	455,	0,		0,
								500,	500,	500,	500,	490,	485,	480,	475,	0,		0,		0,
								500,	500,	500,	500,	490,	485,	480,	0,		0,		0,		0,
								500,	500,	500,	500,	490,	485,	480,	0,		0,		0,		0,
								500,	500,	500,	500,	490,	485,	0,		0,		0,		0,		0,
								500,	500,	500,	500,	490,	0,		0,		0,		0,		0,		0,
								500,	500,	500,	500,	0,		0,		0,		0,		0,		0,		0};
								
uint16_t tableRED[8] = {1000, 1000, 10, 10, 10, 1000, 10, 1000};
uint16_t tableGRN[8] = {10, 1000, 1000, 10, 1000, 10, 10, 1000};
uint16_t tableBLU[8] = {1000, 10, 1000, 1000, 10, 10, 1000, 1000};
								
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
			case VARIABLE_ADDR_LASERCNT:	laserCounter = swap32(*(uint32_t*)data);			break;
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
		break;
		case PICID_WORK_ERROR1:
			state = APP_POWERERR;
		break;
		case PICID_WORK_ERRORFLOW:
			state = APP_FLOWERR;
		break;
		case PICID_WORK_ERRORHEATING:
			state = APP_TEMPERERR;
		break;
		case PICID_WORK_TEMPERWAIT:
			state = APP_WORKTEMPERWAIT;
		break;
		case PICID_WORK_PHOTOTYPESEL:
			state = APP_OnPhototypeSelect;
		break;
		default:
			//state = APP_WORKIDLE;
		break;
	}
}

// Initialization
void CLaserControlApp::Initialize(CMBSender* sender)
{
	// GUI class initialization
	m_cpSender = sender;
	
	// Set global variables
	PIC_ID = 0;
	update = false;
	prepare = false;
	peltier_en = false;
	m_wSetMin = 0;
	m_wSetSec = 10;
	m_wMillSec = 0;
	m_wMinutes = m_wSetMin;
	m_wSeconds = m_wSetSec;
	m_wDeadTime = 0;
	
	// Fast profile
	m_structLaserProfile[PROFILE_FAST].Frequency = 10;			// 10 Hz
	m_structLaserProfile[PROFILE_FAST].Duration  = 40;			// ms
	m_structLaserProfile[PROFILE_FAST].EnergyPercent = 100;		// W
	
	// Medium profile
	m_structLaserProfile[PROFILE_MEDIUM].Frequency = 5;			// 10 Hz
	m_structLaserProfile[PROFILE_MEDIUM].Duration  = 80;		// ms
	m_structLaserProfile[PROFILE_MEDIUM].EnergyPercent = 100;	// W
	
	// Slow profile
	m_structLaserProfile[PROFILE_SLOW].Frequency = 2;			// 10 Hz
	m_structLaserProfile[PROFILE_SLOW].Duration  = 120;			// ma
	m_structLaserProfile[PROFILE_SLOW].EnergyPercent = 100;		// W
	
	// Single profile
	m_structLaserProfile[PROFILE_SINGLE].Frequency = 1;			// 10 Hz
	m_structLaserProfile[PROFILE_SINGLE].Duration  = 100;		// ms
	m_structLaserProfile[PROFILE_SINGLE].EnergyPercent = 100;	// W
	
	// Current profile
	Profile = PROFILE_FAST;
	
	// Set all laser settings
	laserCounter = eeprom_read_dword((uint32_t*)LASER_CNT_EEPROMADDR);
	laserDiodeData.mode = Profile;
	memcpy((void*)&laserDiodeData.laserprofile, (void*)&m_structLaserProfile[PROFILE_FAST], sizeof(DGUS_LASERPROFILE));
	laserDiodeData.lasersettings = CalculateLaserSettings((DGUS_LASERPROFILE*)&m_structLaserProfile[PROFILE_FAST]);
	laserDiodeData.timer.timer_minutes = m_wSetMin;
	laserDiodeData.timer.timer_seconds = m_wSetSec;
	laserDiodeData.PulseCounter = swap32(laserCounter);
	laserDiodeData.melanin = 0;
	laserDiodeData.phototype = 1;
	laserDiodeData.temperature = temperature;
	laserDiodeData.cooling = 3;
	laserDiodeData.flow = 0;
	laserDiodeData.DatabasePageOffset = 0;
	laserDiodeData.DatabaseSelectionIndex = 13;
	laserDiodeData.SessionPulseCounter = 0;
	
	// Initialize Laser timer
	laserTimer.Initialize(WGM_SingleSlopePWM, CS_DIV1024);
	laserTimer.SetPeriod(laserTimerPeriod);		// 10 Hz
	laserTimer.SetCOMPA(laserTimerDutyCycle);	// 50 ms, 50% duty cycle
	laserTimer.SetOVFCallback(OnLaserTimerStopStatic, this, TC_OVFINTLVL_LO_gc);
	laserTimer.SetCOMPACallback(OnLaserTimerStatic, this, TC_CCAINTLVL_LO_gc);
	laserTimer.EnableChannel(TIMER_CHANNEL_A);	// Enable Laser TTL Gate
	laserTimer.ChannelSet(TIMER_CHANNEL_A);
#ifdef LED_LASER_INDICATOR
	laserTimer.SetCOMPB(laserTimerDutyCycle);	// 50 ms, 50% duty cycle
	laserTimer.EnableChannel(TIMER_CHANNEL_B);	// Enable Laser TTL Gate
	laserTimer.ChannelSet(TIMER_CHANNEL_B);
#endif
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
	
	timer.Start(25000);
	
	state = APP_WORKSETUP;
}

// Process GUI
void CLaserControlApp::FastRun()
{

}

void CLaserControlApp::Run()
{	
	// Get PIC ID
	m_cpSender->StartMODBUSRegisterTransaction(REGISTER_ADDR_PICID, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WaitMODBUSListener();
	
	_delay_ms(10);
	
	// show sensors
	if ((state & 0xFFF) != 0)
	{
		SetVariable(VARIABLE_ADDR_TEMPER, (uint16_t*)&temperature, 2);
		if (m_wFlow < 20) laserDiodeData.coolIcon = 1;
		if (m_wFlow >= 20 && m_wFlow <= 40) laserDiodeData.coolIcon = 2;
		if (m_wFlow > 40) laserDiodeData.coolIcon = 3;
		SetVariable(VARIABLE_ADDR_COOLICON, (uint16_t*)&laserDiodeData.coolIcon, 2);
		SetVariable(VARIABLE_ADDR_FLOW, (uint16_t*)&m_wFlow, 2);
	}
	
	// temperature check
	if ((state & (APP_WORKPOWERON | APP_WORKLIGHT | APP_WORKREADY)) != 0)
	{
		if (temperature > 300)
		{
			SetPictureId(PICID_WORK_ERRORHEATING);
			PIC_ID_last = PIC_ID;
		}
	}
	
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
				uint16_t melanin = laserDiodeData.melanin;
				uint16_t phototype = laserDiodeData.phototype;
				
				GetVariable(STRUCT_ADDR_LASERPROFILE_DATA, sizeof(DGUS_LASERPROFILE));
				_delay_ms(10);
				GetVariable(VARIABLE_ADDR_COOLING, 2);
				_delay_ms(10);
				GetVariable(VARIABLE_ADDR_MODE, 2);
				_delay_ms(10);
				GetVariable(VARIABLE_ADDR_MELANIN, 2);
				_delay_ms(10);
				GetVariable(VARIABLE_ADDR_PHOTOTYPE, 2);
				_delay_ms(10);
				
				laserDiodeData.PulseCounter = swap32(laserCounter);
				laserDiodeData.temperature = temperature;
				laserDiodeData.flow = m_wFlow;
				
				if (phototype != laserDiodeData.phototype)
				{
					PhototypePreset(laserDiodeData.phototype);
					update = true;
				}
				else
				if (melanin != laserDiodeData.melanin)
				{
					MelaninPreset(laserDiodeData.melanin);
					update = true;
				}
				else
				{
					APP_PROFILE prof = (APP_PROFILE)laserDiodeData.mode;
					if (Profile != prof)	{update = true; Profile = prof;}
				
					memcpy((void*)&laserDiodeData.laserprofile, (void*)&m_structLaserProfile[Profile], sizeof(laserDiodeData.laserprofile));
					laserDiodeData.lasersettings = CalculateLaserSettings((DGUS_LASERPROFILE*)&m_structLaserProfile[Profile]);
					laserPower = m_structLaserProfile[Profile].EnergyPercent;
				}
				
				update = CheckLimits(laserDiodeData.laserprofile.Frequency, laserDiodeData.laserprofile.Duration, Profile);
				
				laserPower = m_wMaxEnergy * laserPower / 100;
				/*if (laserDiodeData.laserprofile.EnergyPercent > m_wMaxEnergy)
				{
					laserDiodeData.laserprofile.EnergyPercent = m_wMaxEnergy;
					laserPower = m_wMaxEnergy;
					update = true;
				}*/
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
				{
					SetVariable(STRUCT_ADDR_LASERPROFILE_DATA, (uint16_t*)&m_structLaserProfile[Profile], sizeof(DGUS_LASERPROFILE));
					SetPictureId(PICID_WORK_READY);	
				}
			}
		break;
		case APP_WORKTEMPERWAIT:
			{
				DGUS_PREPARETIMER timervar;
				timervar.timer_minutes = temperature / 10;
				timervar.timer_seconds = temperature % 10;
				SetVariable(STRUCT_ADDR_PREPARETIMER_DATA, (uint16_t*)&timervar, sizeof(timervar));
				
				m_wMinutes = m_wSetMin;
				m_wSeconds = m_wSetSec;
				prepare = true;
				
				if (temperature < 290)
					SetPictureId(PICID_WORK_PREPARE);
			}
		break;
		case APP_WORKREADY:
			{				
				if (temperature > 300)
				{
					SetPictureId(PICID_WORK_ERRORHEATING);
					PIC_ID_last = PIC_ID;
				}
			}
		break;
		case APP_WORKLIGHT:
		case APP_WORKPOWERON:
			{				
				uint16_t data = ((uint16_t)((laserPower * 640) / 63)) << 2;  // (laserPower * 1024) / 1000)
				dacSPI.Send((uint8_t*)&data, sizeof(data));
							
				if (!laserBoard.Footswitch())
				{
					if (state != APP_WORKLIGHT)
					{
						SetPictureId(PICID_WORK_STARTED);
						state = APP_WORKLIGHT;
					}
				}
				else
				{
					if (state != APP_WORKPOWERON)
					{
						SetPictureId(PICID_WORK_POWERON);
						state = APP_WORKPOWERON;
					}
				}
			
				uint32_t cnt = swap32(laserCounter);
				SetVariable(VARIABLE_ADDR_LASERCNT, (uint16_t*)&cnt,  4);
				cnt = swap32(laserCounterSession);
				SetVariable(VARIABLE_ADDR_SESSNCNT, (uint16_t*)&cnt,  4);
				
#ifdef FLOW_CONTROL
				if (m_wFlow < 30)
				{
					SetPictureId(PICID_WORK_ERRORFLOW);
					PIC_ID_last = PIC_ID;
				}
#endif
			}
		break;
		case APP_FLOWERR:			
			laserBoard.LaserPowerOff();
			
			if (m_wFlow > 60)
				SetPictureId(PICID_WORKOnReady);
		break;
		case APP_TEMPERERR:			
			laserBoard.LaserPowerOff();
			
			prepare = false;
			if (temperature < 290)
			{
				prepare = true;
				SetPictureId(PIC_ID_last);
			}
		break;
		case APP_POWERERR:			
			laserBoard.LaserPowerOff();
			
			if ((PORTD.IN & PIN5_bm) != 0)
				SetPictureId(PIC_ID_last);
		break;
		
		// Commands
		case APP_WORKOnReady:
			{
				laserCounterSession = 0;
				
				//laserBoard.Relay2On();
				laserBoard.LaserPowerOn();
				
				uint16_t coolpwm = laserDiodeData.cooling * 204;
				pwmtimer.SetCOMPA(coolpwm);
				//pwmtimer.Start(1024);
				peltier_en = true;
				
				prepare = true;
				m_wMinutes = m_wSetMin;
				m_wSeconds = m_wSetSec;

#ifdef FLOW_CONTROL				
				if (m_wFlow < 30)
				{
					SetPictureId(PICID_WORK_ERRORFLOW);
				}
				else				
#endif
				if (temperature > 290)
				{
					SetPictureId(PICID_WORK_TEMPERWAIT);
				}
				else
					SetPictureId(PICID_WORK_PREPARE);
			}
		break;
		case APP_WORKOnPowerOn:
			{
				uint16_t data = ((uint16_t)((laserPower * 640) / 63)) << 2;  // (laserPower * 1024) / 1000)
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
				//pwmtimer.Stop();
				peltier_en = false;
				laserBoard.PWMOn(); // Cooling off
				eeprom_write_dword((uint32_t*)LASER_CNT_EEPROMADDR, laserCounter);
				//laserBoard.Relay2Off();
				
				uint16_t data = 0;
				dacSPI.Send((uint8_t*)&data, sizeof(data));
			}
		break;
		case APP_OnPhototypeSelect:
			{
				SetPictureId(PICID_WORK_IDLE);
			}
		break;
		
		// Database
		case APP_DATABASE:
			GetVariable(VARIABLE_ADDR_DATAOFFS, 2);
			_delay_ms(50);
			GetVariable(VARIABLE_ADDR_DATAINDEX, 2);
			_delay_ms(50);
			
			Database.MapDatabaseToRead(VARIABLE_ADDR_DATABASE, DGUS_DATABASE_ADDR + (uint32_t)laserDiodeData.DatabasePageOffset * (uint32_t)PROFILE_SIZE, 0x0C00);
			
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
				DGUS_DATABASE_ADDR + (uint32_t)(laserDiodeData.DatabasePageOffset + laserDiodeData.DatabaseSelectionIndex) * (uint32_t)PROFILE_SIZE, 0x0100);
		break;
		case APP_SAVEPROFILE:
			Database.MapDatabaseToWrite(
				VARIABLE_ADDR_PROFILE,
				DGUS_DATABASE_ADDR + (uint32_t)(laserDiodeData.DatabasePageOffset + laserDiodeData.DatabaseSelectionIndex) * (uint32_t)PROFILE_SIZE, 0x0100);
		break;
		case APP_UNMAPDATABASE:
			Database.UnMap();
		break;
		default:
			Database.UnMap();
		break;
	}
	
	if ((PORTD.IN & PIN6_bm) == 0)
		{
			PIC_ID_last = PIC_ID;
			SetPictureId(PICID_WORK_ERROR1);
		}
	
	if (update)
	{		
		SetVariable(STRUCT_ADDR_LASERDIODE_DATA, (uint16_t*)&laserDiodeData, sizeof(DGUS_LASERDIODE));
		update = false;
	}
}

DGUS_LASERSETTINGS CLaserControlApp::CalculateLaserSettings(DGUS_LASERPROFILE *profile)
{
	DGUS_LASERSETTINGS result;
	
	uint16_t period = 1000 / profile->Frequency;									//  period [ms]
	uint32_t dutycycle = uint32_t(profile->Duration * 1000ul) / uint32_t(period);	//  duration [ms] / period [us]
	result.DutyCycle = uint16_t(dutycycle/10);										// (period [us] * 100) / (duration [ms] * 1000)
	result.Energy = uint16_t((dutycycle * (uint32_t)profile->EnergyPercent * (uint32_t)MAX_LASER_POWER) / 100000ul);
	result.Power = (profile->EnergyPercent * MAX_LASER_POWER) / 100;
	
	// calculate timer settings
	laserTimerPeriod = (6250 / profile->Frequency) * 10;
	laserTimerDutyCycle = laserTimerPeriod - uint16_t((uint32_t(laserTimerPeriod) * dutycycle) / 1000ul);
	laserTimerDutyCyclems = period;
	
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

void CLaserControlApp::SetPictureIdAsync(uint16_t pic_id)
{
	uint16_t pic = swap(pic_id);
	m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic, 2);
	m_cpSender->WaitMODBUSTransmitter();
}

void CLaserControlApp::OnTimer()
{
	if (m_wDeadTime != 0)
		m_wDeadTime--;
		
	if (prepare)
	{
		if (m_wMillSec == 0)
		{
			m_wFlow = (TCC1.CNT * 10) / 8;
			flowtimer.Reset();
			
			if (m_wSeconds == 0)
			{
				if (m_wMinutes == 0)
				{
					if (state == APP_WORKPREPARE)
					{
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
					}
					OnTimeout();
					
					//player.beep(1000, 1000);
					return;
				}
				m_wSeconds = 60;
				m_wMinutes--;
			}
			m_wMillSec = 100; // Every 10 ms
			m_wSeconds--;
			
			// ****************** Tick sound
			if (m_wMinutes == 0 && m_wSeconds < 10 && state == APP_WORKPREPARE)
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
			}
			// ************************************
		}
		m_wMillSec-=10;
	}
	else
	{
		if (m_wMillSec == 0)
		{
			m_wMillSec = 100;
			
			m_wFlow = (TCC1.CNT * 10) / 8;
			flowtimer.Reset();
		}
		m_wMillSec-=10;
	}
}

void CLaserControlApp::OnLaserTimer()
{
	laserCounter++;
	laserCounterSession++;
	
	if (laserTimerDutyCyclems > 100)
		player.SoundStart(500, 50, 0);
	else
		player.SoundStart(500, laserTimerDutyCyclems/2, 0);
	player.SoundStop();
}

void CLaserControlApp::OnLaserTimerStop()
{	
	if (Profile == PROFILE_SINGLE)
	{
		laserTimer.Stop();
		laserTimer.ChannelSet(TIMER_CHANNEL_A);
		laserTimer.ChannelSet(TIMER_CHANNEL_B);
	}
}

void CLaserControlApp::OnTimeout()
{
	//timer.Stop();
	prepare = false;
	//laserBoard.Relay1Off();
}

void CLaserControlApp::OnPWMTimerOVF()
{
	if (peltier_en)
	{
		laserBoard.PWMOff();
	}

#ifndef LED_LASER_INDICATOR
	laserBoard.REDOff();
	laserBoard.GRNOff();
	laserBoard.BLUOff();
	
	static int pos = 0;
	static int delay = 1000;
	static uint16_t red = 0;
	static uint16_t grn = 0;
	static uint16_t blu = 0;
	
	if (tableGRN[pos] > grn) grn++;
	if (tableGRN[pos] < grn) grn--;
	
	if (tableRED[pos] > red) red++;
	if (tableRED[pos] < red) red--;
	
	if (tableBLU[pos] > blu) blu++;
	if (tableBLU[pos] < blu) blu--;
	
	if (tableBLU[pos] == blu && tableRED[pos] == red && tableGRN[pos] == grn)
	{
		if (delay == 0)
		{
			pos++;
			if (pos == 8) pos = 0;
			delay = 1000;
		}
		delay --;
	}
	
	pwmtimer.SetCOMPB(red);
	pwmtimer.SetCOMPC(grn);
	pwmtimer.SetCOMPD(blu);
#endif
}

void CLaserControlApp::OnPWMTimerCMP()
{
	if (peltier_en)
		laserBoard.PWMOn();
}

void CLaserControlApp::OnPWMTimerRED()
{
#ifndef LED_LASER_INDICATOR
	laserBoard.REDOn();
#endif
}
void CLaserControlApp::OnPWMTimerGRN()
{
#ifndef LED_LASER_INDICATOR
	laserBoard.GRNOn();
#endif
}
void CLaserControlApp::OnPWMTimerBLU()
{
#ifndef LED_LASER_INDICATOR
	laserBoard.BLUOn();
#endif
}

void CLaserControlApp::OnINT0()
{
	static bool isstarted = false;
	
	switch (state)
	{
		case APP_WORKLIGHT:
		case APP_WORKPOWERON:
		{
			if ((PORTC.IN & 0x01) == 0)
			{				
				if ((!isstarted) && (m_wDeadTime == 0))
				{
					if (Profile == PROFILE_SINGLE)
					{
						m_wDeadTime = 10;
						
						laserCounter++;
						laserCounterSession++;
						
						if (laserTimerDutyCyclems > 100)
							player.SoundStart(500, 50, 0);
						else
							player.SoundStart(500, laserTimerDutyCyclems/2, 0);
						player.SoundStop();
					}
						
					if (laserTimerDutyCycle != 0)
						TCF0.CNT = laserTimerDutyCycle - 1;
						
					laserTimer.SetCOMPA(laserTimerDutyCycle);
					laserTimer.SetCOMPB(laserTimerDutyCycle);
					laserTimer.Start(laserTimerPeriod);
					
					if (Profile == PROFILE_SINGLE)
						_delay_ms(200);
					
					PORTC.INTFLAGS &= ~0x01;
					
					isstarted = true;
				}
				
				/*//sei();
				if (state != APP_WORKLIGHT)
				{
					SetPictureIdAsync(PICID_WORK_STARTED);
					state = APP_WORKLIGHT;
				}*/
			}
			else
			{
				isstarted = false;
				
				laserTimer.Stop();
				laserTimer.ChannelSet(TIMER_CHANNEL_A);
				laserTimer.ChannelSet(TIMER_CHANNEL_B);
				
				/*//sei();
				if (state != APP_WORKPOWERON)
				{
					SetPictureIdAsync(PICID_WORK_POWERON);
					state = APP_WORKPOWERON;
				}*/
			}
		}
		break;
		default:
		break;
	}
}

void CLaserControlApp::OnTimerStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnTimer();
}

void CLaserControlApp::OnLaserTimerStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnLaserTimerStop();
}

void CLaserControlApp::OnLaserTimerStopStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnLaserTimer();
}

void CLaserControlApp::OnPWMTimerOVFStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnPWMTimerOVF();
}

void CLaserControlApp::OnPWMTimerCMPStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnPWMTimerCMP();
}

void CLaserControlApp::OnPWMTimerREDStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnPWMTimerRED();
}

void CLaserControlApp::OnPWMTimerGRNStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnPWMTimerGRN();
}
void CLaserControlApp::OnPWMTimerBLUStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnPWMTimerBLU();
}

void CLaserControlApp::OnINT0Static(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnINT0();
}

void CLaserControlApp::MelaninPreset(uint16_t melanin)
{
	Profile = PROFILE_DEFAULT;
	laserDiodeData.mode = Profile;
	laserDiodeData.laserprofile.Frequency = 3;
	if (melanin < 10)
	{
		laserDiodeData.phototype = 1;
		m_wMaxDuration = 60;
		m_wMaxEnergy = 97;
	}
	else
	if (melanin < 20)
	{
		laserDiodeData.phototype = 2;
		m_wMaxDuration = 60;
		m_wMaxEnergy = 97;
	}
	else
	if (melanin < 35)
	{
		laserDiodeData.phototype = 3;
		m_wMaxDuration = 60;
		m_wMaxEnergy = 97;
	}
	else
	if (melanin < 49)
	{
		laserDiodeData.phototype = 4;
		m_wMaxDuration = 100;
		m_wMaxEnergy = 97;
	}
	else
	if (melanin < 72)
	{
		laserDiodeData.phototype = 5;
		m_wMaxDuration = 100;
		m_wMaxEnergy = 90;
	}
	else
	{
		laserDiodeData.phototype = 6;
		m_wMaxDuration = 100;
		m_wMaxEnergy = 87;
	}
	
	laserDiodeData.laserprofile.EnergyPercent = 100;//m_wMaxEnergy;
	laserDiodeData.laserprofile.Duration = m_wMaxDuration;
	
	laserDiodeData.lasersettings = CalculateLaserSettings((DGUS_LASERPROFILE*)&m_structLaserProfile[Profile]);
	laserPower = m_structLaserProfile[Profile].EnergyPercent;
}

void CLaserControlApp::PhototypePreset(uint16_t phototype)
{
	Profile = PROFILE_DEFAULT;
	laserDiodeData.mode = Profile;
	laserDiodeData.laserprofile.Frequency = 3;
	laserDiodeData.phototype = phototype;
	
	switch (phototype)
	{
		case 1:
		{
			laserDiodeData.melanin = 5;
			m_wMaxDuration = 60;
			m_wMaxEnergy = 97;
		}
		break;
		case 2:
		{
			laserDiodeData.melanin = 14;
			m_wMaxDuration = 60;
			m_wMaxEnergy = 97;
		}
		break;
		case 3:
		{
			laserDiodeData.melanin = 27;
			m_wMaxDuration = 60;
			m_wMaxEnergy = 97;
		}
		break;
		case 4:
		{
			laserDiodeData.melanin = 41;
			m_wMaxDuration = 100;
			m_wMaxEnergy = 97;
		}
		break;
		case 5:
		{
			laserDiodeData.melanin = 61;
			m_wMaxDuration = 100;
			m_wMaxEnergy = 90;
		}
		break;
		case 6:
		{
			laserDiodeData.melanin = 85;
			m_wMaxDuration = 100;
			m_wMaxEnergy = 87;
		}
	}
	
	laserDiodeData.laserprofile.EnergyPercent = 100;//m_wMaxEnergy;
	laserDiodeData.laserprofile.Duration = m_wMaxDuration;
	
	laserDiodeData.lasersettings = CalculateLaserSettings((DGUS_LASERPROFILE*)&m_structLaserProfile[Profile]);
	laserPower = m_structLaserProfile[Profile].EnergyPercent;
}

bool CLaserControlApp::CheckLimits(uint16_t &freq, uint16_t &duration, APP_PROFILE mode)
{
	//MinDurationTable
	//MaxDurationTable
	//TableNum
	//PowerTable
	//bool update = false;
	
	switch (mode)
	{
		case PROFILE_DEFAULT:
		break;
		case PROFILE_SINGLE:
			if (freq != 1)
			{
				freq = 1;
				update = true;
			}
			break;
		case PROFILE_SLOW:
			if (freq > 3)
			{
				freq = 3;
				update = true;
			}
			if (freq < 1)
			{
				freq = 1;
				update = true;
			}
			break;
		case PROFILE_MEDIUM:
			if (freq > 6)
			{
				freq = 6;
				update = true;
			}
			if (freq < 4)
			{
				freq = 4;
				update = true;
			}
			break;
		case PROFILE_FAST:
			if (freq > 10)
			{
				freq = 10;
				update = true;
			}
			if (freq < 7)
			{
				freq = 7;
				update = true;
			}
			break;
	}
	
	//freq = min(max(1, freq), 10);
	uint16_t MinD = MinDurationTable[freq];
	uint16_t MaxD = MaxDurationTable[freq];
	uint16_t Delta = (MaxD - MinD) / (TableNum[freq] - 1);
	
	if (duration < MinD)
	{
		duration = MinD;
		update = true;
	}
	
	if (duration > MaxD)
	{
		duration = MaxD;
		update = true;
	}
	
	uint16_t index = (duration - MinD) / Delta;
	index = min(max(0, index), TableNum[freq] - 1);
	
	m_wMaxEnergy = PowerTable[11 * (freq - 1) + index] / 5;
	
	return update;
}