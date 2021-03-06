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

#define FLOW_CONTROL
//#define POWER_CHECK
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

uint16_t MinDurationTable[11] = {60, 60,  20,  20, 20, 10, 10, 10, 10, 10, 10};
uint16_t MaxDurationTable[11] = {400, 400, 120, 120, 100, 80, 65, 57, 50, 44, 40};
uint16_t TableNum[11]         = {0, 11, 11, 11, 9, 8, 7, 7, 6, 5, 4};
uint16_t EnergyTable[110]     ={21,		32,		44,		54,		63,		72,		83,		87,		95,		103,	111, // Updated
								7,		10,		14,		17,		20,		23,		26,		29,		31,		34,		37,
								7,		10,		14,		17,		20,		24,		27,		29,		32,		34,		37,
								7,		10,		13,		17,		20,		24,		26,		29,		32,		0,		0,
								4,		7,		10,		14,		17,		20,		23,		26,		0,		0,		0,
								4,		7,		10,		14,		17,		20,		23,		0,		0,		0,		0,
								4,		7,		10,		14,		17,		20,		23,		0,		0,		0,		0,
								4,		7,		10,		14,		17,		20,		0,		0,		0,		0,		0,
								4,		7,		10,		14,		17,		0,		0,		0,		0,		0,		0,
								4,		7,		10,		14,		0,		0,		0,		0,		0,		0,		0};
								
uint16_t step_table[10] = {1, 2, 5, 10, 15, 20, 50, 100, 150, 200};
								
uint16_t tableRED[8] = {1000, 1000, 10, 10, 10, 1000, 10, 1000};
uint16_t tableGRN[8] = {10, 1000, 1000, 10, 1000, 10, 10, 1000};
uint16_t tableBLU[8] = {1000, 10, 1000, 1000, 10, 10, 1000, 1000};
	
void NormalizeStep(uint16_t &min, uint16_t &max, uint16_t &step, uint16_t threshold_numsteps, uint16_t step_tbl[])
{
	uint16_t offset = (min / step) * step;
	uint16_t delta = max - min;
	uint16_t i = 0;
	
	step = step_tbl[i++];
	while ((delta / step) > threshold_numsteps)
	{
		step = step_tbl[i++];
	}
		
	min = offset;
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
	uint16_t val = swap(*((uint16_t*)data));
	
	uint16_t energy = laserDiodeData.lasersettings.Energy;
	
	if (length == 2)
	{
		switch (addr)
		{
			case VARIABLE_ADDR_MODE:		laserDiodeData.mode = val;							break;
			case VARIABLE_ADDR_FREQ:		laserDiodeData.laserprofile.Frequency = val;		break;
			case VARIABLE_ADDR_DURATION:	laserDiodeData.laserprofile.DurationCnt = val;		break;
			case VARIABLE_ADDR_ENERGYPCT:	laserDiodeData.laserprofile.EnergyCnt = val;		break;
			case VARIABLE_ADDR_POWER:		laserDiodeData.lasersettings.FlushesLimit = val;	break;
			case VARIABLE_ADDR_ENERGY:		laserDiodeData.lasersettings.Energy = val;			break;
			case VARIABLE_ADDR_DUTYCYCLE:	laserDiodeData.lasersettings.Duration = val;		break;
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
			//ConvertData((void*)&m_structLaserSettings, (void*)data, length);
			ConvertData((void*)&laserDiodeData.lasersettings, (void*)data, length);
		
		if (addr == STRUCT_ADDR_LASERPROFILE_DATA)
		{
			if (Profile <= 5)
				ConvertData((void*)&laserDiodeData.laserprofile, (void*)data, length);
			else
				CLaserBoard::Beep();
		}
	}
	
	laserDiodeData.lasersettings.Energy = energy;
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
		case PICID_MAINMENU:
			state = APP_MENU;
		break;
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
	//isstarted = false;
	m_wSetMin = 2;
	m_wSetSec = 10;
	m_wMillSec = 0;
	m_wMinutes = m_wSetMin;
	m_wSeconds = m_wSetSec;
	m_wDeadTime = 0;
	
	uint16_t freq = 10;
	uint16_t duration = 40;
	uint16_t energy = 14;
	
	// Fast profile
	LaserPreset(freq, duration, energy, PROFILE_FAST);
	// Medium profile
	freq = 5; duration = 80; energy = 26;
	LaserPreset(freq, duration, energy, PROFILE_MEDIUM);
	// Slow profile
	freq = 2; duration = 120; energy = 36;
	LaserPreset(freq, duration, energy, PROFILE_SLOW);
	// Single profile
	freq = 1; duration = 100; energy = 38;
	LaserPreset(freq, duration, energy, PROFILE_SINGLE);
	
	// Current profile
	Profile = PROFILE_FAST;
	
	// Set all laser settings
	laserCounter = eeprom_read_dword((uint32_t*)LASER_CNT_EEPROMADDR);
	laserDiodeData.mode = Profile;
	memcpy((void*)&laserDiodeData.laserprofile, (void*)&m_structLaserProfile[PROFILE_FAST], sizeof(DGUS_LASERPROFILE));
	memcpy((void*)&laserDiodeData.lasersettings, (void*)&m_structLaserSettings[PROFILE_FAST], sizeof(DGUS_LASERSETTINGS));
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
	
	// Preset hardware to FAST mode
	freq = 10;
	duration = 40;
	energy = 14;
	LaserPreset(freq, duration, energy, PROFILE_FAST);
	
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
	DGUS_LASERDIODE data;
	memcpy(&data, &laserDiodeData, sizeof(data));
	data.lasersettings.Energy = laserDiodeData.lasersettings.Energy + laserDiodeData.lasersettings.Energy / 5;
	m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_LASERDIODE_DATA, (uint16_t*)&data, sizeof(DGUS_LASERDIODE));
	m_cpSender->WaitMODBUSTransmitter();
	
	timer.Start(25000);
	
	state = APP_WORKSETUP;
}

// Process GUI
void CLaserControlApp::FastRun()
{

}

void CLaserControlApp::SetLaserDiodePower()
{
	uint16_t data = min(uint16_t((uint32_t(laserPower) * 1024ul) / MAX_LASER_POWER) << 2, 4095);  // (laserPower * 640) / 63)
	dacSPI.Send((uint8_t*)&data, sizeof(data));
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
			
			//if (isstarted)
			{
				StopLaser();
				laserBoard.LaserPowerOff();
				PIC_ID_last = PICID_WORK_PREPARE;
			}
		}
	}
	
	/*if ((state & (APP_WORKPOWERON | APP_WORKLIGHT)) == 0)
		isstarted = false;*/
	
	switch (state)
	{		
		// DGUS State
		case APP_LOGO:
			{
				SetPictureId(PICID_MAINMENU);
				state = APP_WORKIDLE;
			}
		break;
		case APP_MENU:
			{
				prepare = false;
				StopLaser();
				laserBoard.LaserPowerOff();
				//pwmtimer.Stop();
				peltier_en = false;
				laserBoard.PWMOn(); // Cooling off
			}
		break;
		case APP_WORKSETUP:
			{
				prepare = false;
				StopLaser();
				//pwmtimer.Stop();
				peltier_en = false;
				laserBoard.PWMOn(); // Cooling off
				
				uint16_t melanin     = laserDiodeData.melanin;
				uint16_t phototype   = laserDiodeData.phototype;
				uint16_t freq        = laserDiodeData.laserprofile.Frequency;
				uint16_t durationCnt = laserDiodeData.laserprofile.DurationCnt;
				uint16_t energyCnt   = laserDiodeData.laserprofile.EnergyCnt;
				uint16_t flushesLimit	= laserDiodeData.lasersettings.FlushesLimit;
				
				GetVariable(STRUCT_ADDR_LASERPROFILE_DATA, sizeof(DGUS_LASERPROFILE));
				_delay_ms(10);
				GetVariable(STRUCT_ADDR_LASERPROSETTINGS_DATA, sizeof(DGUS_LASERSETTINGS));
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
				laserDiodeData.temperature  = temperature;
				laserDiodeData.flow         = m_wFlow;
				
				if (flushesLimit != laserDiodeData.lasersettings.FlushesLimit)
				{	
					if ((laserDiodeData.lasersettings.FlushesLimit != 4) && (Profile != PROFILE_FAST))
					{
						laserDiodeData.lasersettings.FlushesLimit = 4;
						update = true;
						break;
					}
					
					if (Profile == PROFILE_FAST)
					{
						if (laserDiodeData.lasersettings.FlushesLimit == 3) laserDiodeData.lasersettings.FlushesLimit = 0;
						laserLimitMode = laserDiodeData.lasersettings.FlushesLimit;
						update = true;
						break;
					}
				}
				
				if (phototype != laserDiodeData.phototype)
				{
					PhototypePreset(laserDiodeData.phototype);
					memcpy((void*)&laserDiodeData.laserprofile , (void*)&m_structLaserProfile [Profile], sizeof(laserDiodeData.laserprofile));
					memcpy((void*)&laserDiodeData.lasersettings, (void*)&m_structLaserSettings[Profile], sizeof(laserDiodeData.lasersettings));
					update = true;
					break;
				}
				else
				if (melanin != laserDiodeData.melanin)
				{
					MelaninPreset(laserDiodeData.melanin);
					memcpy((void*)&laserDiodeData.laserprofile , (void*)&m_structLaserProfile [Profile], sizeof(laserDiodeData.laserprofile));
					memcpy((void*)&laserDiodeData.lasersettings, (void*)&m_structLaserSettings[Profile], sizeof(laserDiodeData.lasersettings));
					update = true;
					break;
				}
				else
				if (Profile != (APP_PROFILE)laserDiodeData.mode)
				{
					Profile = (APP_PROFILE)laserDiodeData.mode;
					// Update profile
					memcpy((void*)&laserDiodeData.laserprofile , (void*)&m_structLaserProfile [Profile], sizeof(laserDiodeData.laserprofile));
					memcpy((void*)&laserDiodeData.lasersettings, (void*)&m_structLaserSettings[Profile], sizeof(laserDiodeData.lasersettings));
					
					update = true;
					break;
				}
				
				uint16_t duration = durationCnt * pstGUI[Profile].m_wDurationStep;// + pstGUI[Profile].m_wDurationOffset;
				uint16_t energy   = energyCnt   * pstGUI[Profile].m_wEnergyStep;//   + pstGUI[Profile].m_wEnergyOffset;
				
				if (freq != laserDiodeData.laserprofile.Frequency)
				{
					freq = laserDiodeData.laserprofile.Frequency;
					
					if (!FreqLimits(freq, (APP_PROFILE)laserDiodeData.mode))
					{
						//laserDiodeData.laserprofile.Frequency = freq;
						//update = true;
					}
					
					update = true;
				}
				
				UpdateLimits(freq, duration, energy, Profile);
				
				if (durationCnt != laserDiodeData.laserprofile.DurationCnt)
				{					
					if (Profile == PROFILE_FAST)
						laserDiodeData.laserprofile.DurationCnt = durationCnt; // Disable duration when FAST mode
					else
					{
						duration = laserDiodeData.laserprofile.DurationCnt * pstGUI[Profile].m_wDurationStep;// + pstGUI[Profile].m_wDurationOffset;
						
						// Check limit
						if ((duration <= pstGUI[Profile].m_wMaxDuration) && (duration >= pstGUI[Profile].m_wMinDuration))
						{
							if (pstGUI[Profile].updateDuration)
								CalculateDurationSteps(freq, duration);
							laserDiodeData.lasersettings.Duration = duration;
						}
						else
							laserDiodeData.laserprofile.DurationCnt = durationCnt; // Cancel change if out
					}
					update = true;
				}
				if (energyCnt != laserDiodeData.laserprofile.EnergyCnt)
				{
					energy = laserDiodeData.laserprofile.EnergyCnt * pstGUI[Profile].m_wEnergyStep;// + pstGUI[Profile].m_wEnergyOffset;
					
					if (Profile == PROFILE_FAST)
						CheckLimitsFastMode(freq, duration, energy);
					else
					{
						// Check limit
						if ((energy <= pstGUI[Profile].m_wMaxEnergy_) && (energy >= pstGUI[Profile].m_wMinEnergy_))
						{
							if (pstGUI[Profile].updateEnergy)
								CalculateEnergySteps(freq, energy);
							laserDiodeData.lasersettings.Energy = energy;
						}
						else
							laserDiodeData.laserprofile.EnergyCnt = energyCnt; // Cancel change if out
					}
					update = true;
				}
				
				if (Profile != PROFILE_FAST)
					CheckLimits(freq, duration, energy, Profile);
				else
					CheckLimitsFastMode(freq, duration, energy);
				
				if (Profile == PROFILE_MEDIUM)
					CalculateMultiPulseLaserSettings(&laserDiodeData.laserprofile, &laserDiodeData.lasersettings);
				else
					CalculateLaserSettings(&laserDiodeData.laserprofile, &laserDiodeData.lasersettings);
					
				laserPower = min((uint32_t)(laserDiodeData.lasersettings.Energy * 1440ul) / (uint32_t)laserDiodeData.lasersettings.Duration, MAX_LASER_POWER); // Convert Energy J/cm2 to Power in Watts
				
				// Copy data to profiles
				if (update)
				{
					memcpy((void*)&m_structLaserProfile [Profile], (void*)&laserDiodeData.laserprofile , sizeof(laserDiodeData.laserprofile));
					memcpy((void*)&m_structLaserSettings[Profile], (void*)&laserDiodeData.lasersettings, sizeof(laserDiodeData.lasersettings));
				}
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
				
				/*m_wMinutes = m_wSetMin;
				m_wSeconds = m_wSetSec;*/
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
				SetLaserDiodePower();
							
				if (!laserBoard.Footswitch())
				{
					//isstarted = true;
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
					//StopLaser();
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
				/*m_wMinutes = m_wSetMin;
				m_wSeconds = m_wSetSec;*/

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
				SetLaserDiodePower();
				
				SetPictureId(PICID_WORK_POWERON);
			}
		break;
		case APP_WORKOnPowerOff:
			{
				SetPictureId(PICID_WORK_IDLE);
				
				StopLaser();
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

#ifdef POWER_CHECK
	if ((PORTD.IN & PIN6_bm) != 0)
		{
			PIC_ID_last = PIC_ID;
			SetPictureId(PICID_WORK_ERROR1);
		}
#endif
	
	if (update)
	{		
		DGUS_LASERDIODE data;
		memcpy(&data, &laserDiodeData, sizeof(data));
		data.lasersettings.Energy = laserDiodeData.lasersettings.Energy + laserDiodeData.lasersettings.Energy / 5;
		SetVariable(STRUCT_ADDR_LASERDIODE_DATA, (uint16_t*)&data, sizeof(DGUS_LASERDIODE));
		update = false;
	}
}

DGUS_LASERSETTINGS CLaserControlApp::CalculateLaserSettings(DGUS_LASERPROFILE *profile, DGUS_LASERSETTINGS *settings)
{
	DGUS_LASERSETTINGS result;
	
	uint16_t period = 1000 / profile->Frequency;									//  period [ms]
	uint32_t dutycycle = uint32_t(settings->Duration * 1000ul) / uint32_t(period);	//  duration [ms] / period [us]
	
	// calculate timer settings
	laserTimerPeriod = (6250 / profile->Frequency) * 10;
	laserTimerDutyCycle = laserTimerPeriod - uint16_t((uint32_t(laserTimerPeriod) * dutycycle) / 1000ul);
	laserTimerDutyCyclems = period;
	
	laserTimer.EnableChannel(TIMER_CHANNEL_A);
	laserTimer.EnableChannel(TIMER_CHANNEL_B);
	laserTimer.ChannelSet(TIMER_CHANNEL_A);
	laserTimer.ChannelSet(TIMER_CHANNEL_B);
	
	return result;
}

DGUS_LASERSETTINGS CLaserControlApp::CalculateMultiPulseLaserSettings(DGUS_LASERPROFILE *profile, DGUS_LASERSETTINGS *settings)
{
	DGUS_LASERSETTINGS result;
	
	uint16_t period = 1000 / profile->Frequency;									//  period [ms]
	uint16_t duration1 = settings->Duration / 2;
	uint16_t duration2 = settings->Duration / 2;
	uint16_t period1;
	uint16_t period2;
	uint16_t period3;
	uint16_t period4;
	
	if (profile->Frequency >= 2)
	{
		period3 = period - (settings->Duration - 5);
		period4 = duration1;
		period1 = 5;
		period2 = duration2;
	}
	else
	{
		period3 = period - (settings->Duration - 10);
		period4 = duration1;
		period1 = 10;
		period2 = duration2;
	}
	
	// calculate multi pulse
	laserMultiPulsePeriod[0] = uint32_t(625ul * (uint32_t)period1) / 10ul;
	laserMultiPulsePeriod[1] = uint32_t(625ul * (uint32_t)period2) / 10ul;
	laserMultiPulsePeriod[2] = uint32_t(625ul * (uint32_t)period3) / 10ul;
	laserMultiPulsePeriod[3] = uint32_t(625ul * (uint32_t)period4) / 10ul;
	
	laserTimer.DisableChannel(TIMER_CHANNEL_A);
	laserTimer.DisableChannel(TIMER_CHANNEL_B);
	
	return result;
}

void CLaserControlApp::LaserPreset(uint16_t &freq, uint16_t &duration, uint16_t &energy, APP_PROFILE mode)
{
	UpdateLimits(freq, duration, energy, mode);
	CalculateAllSteps(freq, duration, mode);
	
	m_structLaserProfile[mode].Frequency = freq;
	m_structLaserProfile[mode].EnergyCnt = (energy/* - pstGUI[mode].m_wEnergyOffset*/) / pstGUI[mode].m_wEnergyStep;
	m_structLaserProfile[mode].DurationCnt = (duration/* - pstGUI[mode].m_wDurationOffset*/) / pstGUI[mode].m_wDurationStep;
	m_structLaserSettings[mode].Duration = duration;
	m_structLaserSettings[mode].Energy = energy;
	m_structLaserSettings[mode].FlushesLimit = 4; // deprecated
	
	if (mode == PROFILE_FAST)
	{
		m_structLaserSettings[mode].FlushesLimit = 0;
	}
	
	// Set settings to hardware
	uint16_t period = 1000 / freq;											//  period [ms]
	uint32_t dutycycle = uint32_t(duration * 1000ul) / uint32_t(period);	//  duration [ms] / period [us]
	// calculate timer settings
	laserTimerPeriod = (6250 / freq) * 10;
	laserTimerDutyCycle = laserTimerPeriod - uint16_t((uint32_t(laserTimerPeriod) * dutycycle) / 1000ul);
	laserTimerDutyCyclems = period;
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
		
	if (m_wMillSec > 100) m_wMillSec = 0;
		
	if (prepare)
	{
		if (m_wMillSec == 0)
		{			
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
			if (state == APP_WORKPREPARE)
			{
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
			}
			// ************************************
		}
		m_wMillSec-=10;
	}
	
	if (!peltier_en)
	{
		if (m_wMillSec >= 70)
		{
			if ((m_wSeconds >= m_wSetSec) && (m_wMinutes >= m_wSetMin))
				m_wSeconds = m_wSetSec;
			else
			{
				if (m_wSeconds == 60)
				{
					if (m_wMinutes >= m_wSetMin)
						m_wMinutes = m_wSetMin;
					m_wSeconds = 0;
					m_wMinutes++;
				}
				m_wSeconds++;
			}
			m_wMillSec = 0; // Every 10 ms
		}
		m_wMillSec += 10;
	}
	
	// Flow sensor
	{
		static uint16_t millsec = 100;
		if (millsec == 0)
		{
			millsec = 100;
			
			m_wFlow = (TCC1.CNT * 73) / 104; // ((cnt * 10) / 8) * (7,3 / 13)
			flowtimer.Reset();
		}
		millsec-=10;
	}
}

void CLaserControlApp::OnLaserTimer()
{
}

void CLaserControlApp::OnLaserTimerStop()
{			
	if (Profile == PROFILE_SINGLE)
	{
		StopLaser();
		
		if (laserTimerDutyCyclems > 100)
			player.SoundStart(500, 50, 0);
		else
			player.SoundStart(500, laserTimerDutyCyclems/2, 0);
		player.SoundStop();
		return;
	}
	
	laserCounter++;
	laserCounterSession++;
	
	if (Profile == PROFILE_FAST)
	{
		laserLimitCnt ++;
		if ((laserLimitMode == 0) && ((laserCounterSession%300) == 0) && (laserCounterSession != 0))
		{
			laserLimitCnt = 0;
			StopLaser();
			player.SoundStart(500, 1000, 0);
			player.SoundStop();
			//StartLaser();
		}
		
		if ((laserLimitMode == 1) && ((laserCounterSession%400) == 0) && (laserCounterSession != 0))
		{
			laserLimitCnt = 0;
			StopLaser();
			player.SoundStart(500, 1000, 0);
			player.SoundStop();
			//StartLaser();
		}
		
		if ((laserLimitMode == 2) && ((laserCounterSession%500) == 0) && (laserCounterSession != 0))
		{
			laserLimitCnt = 0;
			StopLaser();
			player.SoundStart(500, 1000, 0);
			player.SoundStop();
			//StartLaser();
		}
	}
	
	if (Profile == PROFILE_MEDIUM)
	{	
		if 	((laserMultiPulseState%2) == 0)
			PORTF.OUTCLR = PIN0_bm;
		else
			PORTF.OUTSET = PIN0_bm;
			
		if 	((laserMultiPulseState%2) == 0)
			PORTF.OUTCLR = PIN1_bm;
		else
			PORTF.OUTSET = PIN1_bm;
			
		if (laserMultiPulseState >= NUM_PULSES * 2) laserMultiPulseState = 0;
		TCF0.PERBUF = laserMultiPulsePeriod[laserMultiPulseState];
		
		if (laserMultiPulseState == 3)
		{
			player.SoundStart(500, 100, 0);
			player.SoundStop();
		}
		
		laserMultiPulseState++;
	}
	else
	{
		if (laserTimerDutyCyclems > 100)
			player.SoundStart(500, 50, 0);
		else
			player.SoundStart(500, laserTimerDutyCyclems/2, 0);
		player.SoundStop();
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
						
						/*if (laserTimerDutyCyclems > 100)
							player.SoundStart(500, 50, 0);
						else
							player.SoundStart(500, laserTimerDutyCyclems/2, 0);
						player.SoundStop();*/
					}
						
					if (laserTimerDutyCycle != 0)
						TCF0.CNT = laserTimerDutyCycle - 1;
						
					StartLaser();
					
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
				
				StopLaser();
				
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
	app->OnLaserTimer();
}

void CLaserControlApp::OnLaserTimerStopStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnLaserTimerStop();
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
	uint16_t freq = 3;
	uint16_t duration = 65;
	uint16_t energy = 22;
	
	Profile = PROFILE_SLOW;
	laserDiodeData.mode = Profile;
	if (melanin < 10)
	{
		laserDiodeData.phototype = 1;
		freq = 3; duration = 75; energy = 21;
	}
	else
	if (melanin < 20)
	{
		laserDiodeData.phototype = 2;
		freq = 3; duration = 70; energy = 21;
	}
	else
	if (melanin < 35)
	{
		laserDiodeData.phototype = 3;
		freq = 3; duration = 70; energy = 21;
	}
	else
	if (melanin < 49)
	{
		laserDiodeData.phototype = 4;
		freq = 3; duration = 65; energy = 21;
	}
	else
	if (melanin < 72)
	{
		laserDiodeData.phototype = 5;
		freq = 3; duration = 100; energy = 20;
	}
	else
	{
		laserDiodeData.phototype = 6;
		freq = 1; duration = 200; energy = 30;
	}
	
	LaserPreset(freq, duration, energy, Profile);
	
	update = true;
	laserDiodeData.lasersettings = CalculateLaserSettings((DGUS_LASERPROFILE*)&m_structLaserProfile[Profile], (DGUS_LASERSETTINGS*)&m_structLaserSettings[Profile]);
}

void CLaserControlApp::PhototypePreset(uint16_t phototype)
{
	uint16_t freq = 3;
	uint16_t duration = 65;
	uint16_t energy = 22;
	
	Profile = PROFILE_SLOW;
	laserDiodeData.mode = Profile;
	laserDiodeData.phototype = phototype;
	
	switch (phototype)
	{
		case 1:
		{
			laserDiodeData.melanin = 5;
			freq = 3; duration = 75; energy = 21;
		}
		break;
		case 2:
		{
			laserDiodeData.melanin = 14;
			freq = 3; duration = 70; energy = 21;
		}
		break;
		case 3:
		{
			laserDiodeData.melanin = 27;
			freq = 3; duration = 70; energy = 21;
		}
		break;
		case 4:
		{
			laserDiodeData.melanin = 41;
			freq = 3; duration = 65; energy = 21;
		}
		break;
		case 5:
		{
			laserDiodeData.melanin = 61;
			freq = 3; duration = 100; energy = 20;
		}
		break;
		case 6:
		{
			laserDiodeData.melanin = 85;
			freq = 1; duration = 200; energy = 30;
		}
	}
	
	LaserPreset(freq, duration, energy, Profile);
	
	update = true;	
	laserDiodeData.lasersettings = CalculateLaserSettings((DGUS_LASERPROFILE*)&m_structLaserProfile[Profile], (DGUS_LASERSETTINGS*)&m_structLaserSettings[Profile]);
}

void CLaserControlApp::UpdateLimits(uint16_t freq, uint16_t duration, uint16_t energy, APP_PROFILE mode)
{
	bool UpdateFreq = false;
	switch (mode)
	{
		case PROFILE_DEFAULT:break;
		case PROFILE_SINGLE:		
			pstGUI[mode].m_wMinFreq = 1;
			pstGUI[mode].m_wMaxFreq = 1;
			UpdateFreq = true;
			break;
		case PROFILE_SLOW:			
			pstGUI[mode].m_wMinFreq = PROFILE_SLOW_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_SLOW_MAX_FREQ;
			break;
		case PROFILE_MEDIUM:		
			pstGUI[mode].m_wMinFreq = PROFILE_MEDIUM_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_MEDIUM_MAX_FREQ;
			break;
		case PROFILE_FAST:
			pstGUI[mode].m_wMinFreq = PROFILE_FAST_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_FAST_MAX_FREQ;
			break;
	}
	if (mode != PROFILE_SINGLE)	
	{
		if (freq < pstGUI[mode].m_wMinFreq)	UpdateFreq = true;
		if (freq > pstGUI[mode].m_wMaxFreq)	UpdateFreq = true;
	}
	
	// Update Frequency
	if (UpdateFreq) 
		laserDiodeData.laserprofile.Frequency = freq;
	
	//freq = min(max(1, freq), 10);
	uint16_t MinD = MinDurationTable[freq];
	uint16_t MaxD = MaxDurationTable[freq];
	uint16_t Delta = (MaxD - MinD) / (TableNum[freq]-1);
	
	if (mode == PROFILE_MEDIUM)
	{
		MinD = 10;
		MaxD = 100;
	}
	
	// Update duration helpers
	if (duration < MinD)	duration = MinD;
	if (duration > MaxD)	duration = MaxD;
		
	if ((pstGUI[mode].m_wMinDuration != MinD) || (pstGUI[mode].m_wMaxDuration != MaxD))
	{
		pstGUI[mode].m_wMinDuration = MinD;
		pstGUI[mode].m_wMaxDuration = MaxD;	
		pstGUI[mode].updateDuration = true;
	}

	uint16_t index = (duration - MinD) / Delta;
	index = min(max(0, index), TableNum[freq] - 1);
	
	pstGUI[mode].m_wMaxEnergy_ = EnergyTable[11 * (freq - 1) + index];
		
	uint16_t MaxEnergy = pstGUI[mode].m_wMaxEnergy_;
	//uint16_t Offset = pstGUI[mode].m_wEnergyOffset;
		
	if (/*(pstGUI[mode].m_wMinEnergy_ != Offset) ||*/ (pstGUI[mode].m_wMaxEnergy_ != MaxEnergy))
	{
		pstGUI[mode].m_wMinEnergy_ = 0;//Offset;
		pstGUI[mode].m_wMaxEnergy_ = MaxEnergy;
		pstGUI[mode].updateEnergy = true;
	}
}

bool CLaserControlApp::CheckLimits(uint16_t &freq, uint16_t &duration, uint16_t &energy, APP_PROFILE mode)
{
	bool UpdateFreq = false;
	switch (mode)
	{
		case PROFILE_DEFAULT:break;
		case PROFILE_SINGLE:
			pstGUI[mode].m_wMinFreq = 1;
			pstGUI[mode].m_wMaxFreq = 1;
			if (freq != 1)
			{
				freq = 1;
				UpdateFreq = true;
			}
		break;
		case PROFILE_SLOW:
			pstGUI[mode].m_wMinFreq = PROFILE_SLOW_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_SLOW_MAX_FREQ;
		break;
		case PROFILE_MEDIUM:
			pstGUI[mode].m_wMinFreq = PROFILE_MEDIUM_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_MEDIUM_MAX_FREQ;
		break;
		case PROFILE_FAST:
			pstGUI[mode].m_wMinFreq = PROFILE_FAST_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_FAST_MAX_FREQ;
		break;
	}
	
	if (mode != PROFILE_SINGLE)
	{
		if (freq < pstGUI[mode].m_wMinFreq)	{	freq = pstGUI[mode].m_wMinFreq;	UpdateFreq = true;	}
		if (freq > pstGUI[mode].m_wMaxFreq)	{	freq = pstGUI[mode].m_wMaxFreq;	UpdateFreq = true;	}
	}
	
	if (UpdateFreq)
	{
		// Update Frequency
		laserDiodeData.laserprofile.Frequency = freq;
		update = true;
	}
	
	//freq = min(max(1, freq), 10);
	uint16_t MinD = MinDurationTable[freq];
	uint16_t MaxD = MaxDurationTable[freq];
	uint16_t Delta = (MaxD - MinD) / (TableNum[freq]-1);
	uint16_t step ;
	
	if (mode == PROFILE_MEDIUM)
	{
		MinD = 10;
		MaxD = 100;	
	}
	
	bool UpdateDuration = false;
	if (duration < MinD)	{	duration = MinD;	UpdateDuration = true;	}
	if (duration > MaxD)	{	duration = MaxD;	UpdateDuration = true;	}
	
	if (UpdateDuration)
	{			
		// Update duration helpers	
		step = 1;
		NormalizeStep(MinD, MaxD, step, 20, step_table);
		pstGUI[mode].m_wDurationStep = step;
		pstGUI[mode].m_wDurationNumSteps = (MaxD - MinD)/step;
		//pstGUI[mode].m_wDurationOffset = MinD;
		pstGUI[mode].m_wMinDuration = MinD;
		pstGUI[mode].m_wMaxDuration = MaxD;
		
		// Update Duration
		laserDiodeData.laserprofile.DurationCnt = (duration/* - pstGUI[mode].m_wDurationOffset*/) / pstGUI[mode].m_wDurationStep;
		laserDiodeData.lasersettings.Duration = duration;
		
		update = true;
	}
	
	uint16_t index = (duration - MinD) / Delta;
	index = min(max(0, index), TableNum[freq] - 1);
	
	pstGUI[mode].m_wMaxEnergy_ = EnergyTable[11 * (freq - 1) + index];
	
	if (energy > pstGUI[mode].m_wMaxEnergy_)
	{
		energy = pstGUI[mode].m_wMaxEnergy_;
		
		// Update energy helpers
		step = 1;
		uint16_t MaxEnergy = pstGUI[mode].m_wMaxEnergy_;
		uint16_t Offset = 0;//pstGUI[mode].m_wEnergyOffset;
		NormalizeStep(Offset, MaxEnergy, step, 20, step_table);
		//pstGUI[mode].m_wEnergyOffset = Offset;
		pstGUI[mode].m_wEnergyStep = step;
		pstGUI[mode].m_wEnergyNumSteps = (pstGUI[mode].m_wMaxEnergy_/* - Offset*/)/step;
		pstGUI[mode].m_wMinEnergy_ = 0;//Offset;
		pstGUI[mode].m_wMaxEnergy_ = MaxEnergy;
		
		// Update Energy
		laserDiodeData.laserprofile.EnergyCnt = (energy/* - pstGUI[mode].m_wEnergyOffset*/) / pstGUI[mode].m_wEnergyStep;
		laserDiodeData.lasersettings.Energy = energy;
		
		update = true;
	}
	
	return update;
}

void CLaserControlApp::CalculateDurationSteps(uint16_t &freq, uint16_t &duration)
{
	uint16_t step = 1;
	uint16_t MinD = pstGUI[Profile].m_wMinDuration;
	uint16_t MaxD = pstGUI[Profile].m_wMaxDuration;
	NormalizeStep(MinD, MaxD, step, 20, step_table);
	pstGUI[Profile].m_wDurationStep = step;
	pstGUI[Profile].m_wDurationNumSteps = (MaxD - MinD)/step;
	//pstGUI[Profile].m_wDurationOffset = MinD;
	pstGUI[Profile].updateDuration = false;
	// Update Duration
	laserDiodeData.laserprofile.DurationCnt = (duration/* - pstGUI[Profile].m_wDurationOffset*/) / pstGUI[Profile].m_wDurationStep;
	laserDiodeData.lasersettings.Duration = duration;
}

void CLaserControlApp::CalculateEnergySteps(uint16_t &freq, uint16_t &energy)
{
	uint16_t step = 1;
	uint16_t MaxEnergy = pstGUI[Profile].m_wMaxEnergy_;
	uint16_t MinEnergy = pstGUI[Profile].m_wMinEnergy_;
	NormalizeStep(MinEnergy, MaxEnergy, step, 20, step_table);
	pstGUI[Profile].m_wEnergyStep = step;
	pstGUI[Profile].m_wEnergyNumSteps = (pstGUI[Profile].m_wMaxEnergy_ - MinEnergy)/step;
	//pstGUI[Profile].m_wEnergyOffset = MinEnergy;
	pstGUI[Profile].updateEnergy = false;
	// Update Energy
	laserDiodeData.laserprofile.EnergyCnt = (energy/* - pstGUI[Profile].m_wEnergyOffset*/) / pstGUI[Profile].m_wEnergyStep;
	laserDiodeData.lasersettings.Energy = energy;
}

void CLaserControlApp::CalculateAllSteps(uint16_t &freq, uint16_t &duration, APP_PROFILE mode)
{
	uint16_t MinD = MinDurationTable[freq];
	uint16_t MaxD = MaxDurationTable[freq];
	if (mode == PROFILE_MEDIUM)
	{
		MinD = 10;
		MaxD = 100;
	}
	uint16_t Delta = (MaxD - MinD) / (TableNum[freq]-1);
	uint16_t step = 1;
	
	if (mode == PROFILE_FAST)
	{
		// Update energy helpers
		//pstGUI[PROFILE_FAST].m_wEnergyOffset = 6;
		pstGUI[PROFILE_FAST].m_wEnergyStep = 1;
		pstGUI[PROFILE_FAST].m_wEnergyNumSteps = 13 - 6;
		pstGUI[PROFILE_FAST].m_wMinEnergy_ = 6;
		pstGUI[PROFILE_FAST].m_wMaxEnergy_ = 12;
		
		// Update duration helpers
		//pstGUI[PROFILE_FAST].m_wDurationOffset = 18;
		pstGUI[PROFILE_FAST].m_wDurationStep = 1;
		pstGUI[PROFILE_FAST].m_wDurationNumSteps = 35 - 18;
		pstGUI[PROFILE_FAST].m_wMinDuration = 18;
		pstGUI[PROFILE_FAST].m_wMaxDuration = 35;
		return;
	}
	
	// Update duration helpers
	NormalizeStep(MinD, MaxD, step, 20, step_table);
	pstGUI[mode].m_wDurationStep = step;
	pstGUI[mode].m_wDurationNumSteps = (MaxD - MinD)/step;
	//pstGUI[mode].m_wDurationOffset = MinD;
	pstGUI[mode].updateDuration = false;
	
	uint16_t index = (duration - MinD) / Delta;
	index = min(max(0, index), TableNum[freq] - 1);
	
	//pstGUI[mode].m_wMaxEnergy = PowerTable[11 * (freq - 1) + index] / 5;
	pstGUI[mode].m_wMaxEnergy_ = EnergyTable[11 * (freq - 1) + index];
	
	// Update energy helpers
	step = 1;
	uint16_t MaxEnergy = pstGUI[mode].m_wMaxEnergy_;
	uint16_t Offset = 0;//pstGUI[mode].m_wEnergyOffset;
	NormalizeStep(Offset, MaxEnergy, step, 20, step_table);
	//pstGUI[mode].m_wEnergyOffset = Offset;
	pstGUI[mode].m_wEnergyStep = step;
	pstGUI[mode].m_wEnergyNumSteps = (pstGUI[mode].m_wMaxEnergy_ - Offset)/step;
	pstGUI[mode].updateEnergy = false;
}

bool CLaserControlApp::FreqLimits(uint16_t &freq, APP_PROFILE mode)
{
	bool UpdateFreq = false;
	switch (mode)
	{
		case PROFILE_DEFAULT:break;
		case PROFILE_SINGLE:
			pstGUI[mode].m_wMinFreq = 1;
			pstGUI[mode].m_wMaxFreq = 1;
			if (freq != 1)
			{
				freq = 1;
				UpdateFreq = true;
				}
		break;
		case PROFILE_SLOW:
			pstGUI[mode].m_wMinFreq = PROFILE_SLOW_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_SLOW_MAX_FREQ;
		break;
		case PROFILE_MEDIUM:
			pstGUI[mode].m_wMinFreq = PROFILE_MEDIUM_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_MEDIUM_MAX_FREQ;
		break;
		case PROFILE_FAST:
			pstGUI[mode].m_wMinFreq = PROFILE_FAST_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_FAST_MAX_FREQ;
		break;
	}
	if (mode != PROFILE_SINGLE)
	{
		if (freq < pstGUI[mode].m_wMinFreq)	UpdateFreq = true;
		if (freq > pstGUI[mode].m_wMaxFreq)	UpdateFreq = true;
	}
	
	return UpdateFreq;
}

bool CLaserControlApp::CheckLimitsFastMode(uint16_t &freq, uint16_t &duration, uint16_t &energy)
{
	pstGUI[PROFILE_FAST].m_wMinFreq = PROFILE_FAST_MIN_FREQ;
	pstGUI[PROFILE_FAST].m_wMaxFreq = PROFILE_FAST_MAX_FREQ;
	
	if (freq < pstGUI[PROFILE_FAST].m_wMinFreq)	{	freq = pstGUI[PROFILE_FAST].m_wMinFreq;	}
	if (freq > pstGUI[PROFILE_FAST].m_wMaxFreq)	{	freq = pstGUI[PROFILE_FAST].m_wMaxFreq;	}
	
	laserDiodeData.laserprofile.Frequency = freq;
	
	if (energy > 13) { energy = 13;  }
	if (energy <  6) { energy = 6; }
		
	// Update energy helpers
	//pstGUI[PROFILE_FAST].m_wEnergyOffset = 6;
	pstGUI[PROFILE_FAST].m_wEnergyStep = 1;
	pstGUI[PROFILE_FAST].m_wEnergyNumSteps = 13 - 6;
	pstGUI[PROFILE_FAST].m_wMinEnergy_ = 6;
	pstGUI[PROFILE_FAST].m_wMaxEnergy_ = 13;
	
	// Update Energy
	laserDiodeData.laserprofile.EnergyCnt = (energy/* - pstGUI[PROFILE_FAST].m_wEnergyOffset*/) / pstGUI[PROFILE_FAST].m_wEnergyStep;
	laserDiodeData.lasersettings.Energy = energy;	
	
	duration = (energy * 1440) / 500; // Calculate max duration
	
	/*if (duration >= 35) duration = 35;	else
	if (duration >= 24) duration = 24;	else
	if (duration >= 18) duration = 18;*/
	
	// Update duration helpers
	//pstGUI[PROFILE_FAST].m_wDurationOffset = 18;
	pstGUI[PROFILE_FAST].m_wDurationStep = 1;
	pstGUI[PROFILE_FAST].m_wDurationNumSteps = 35 - 18;
	pstGUI[PROFILE_FAST].m_wMinDuration = 18;
	pstGUI[PROFILE_FAST].m_wMaxDuration = 35;
		
	// Update Duration
	laserDiodeData.laserprofile.DurationCnt = (duration/* - pstGUI[PROFILE_FAST].m_wDurationOffset*/) / pstGUI[PROFILE_FAST].m_wDurationStep;
	laserDiodeData.lasersettings.Duration = duration;
	
	return true;
}

void CLaserControlApp::StartLaser()
{
	laserLimitCnt = 0;
	if (Profile == PROFILE_MEDIUM)
	{
		laserTimer.Stop();
		laserMultiPulseState = 2;
		PORTF.OUTSET = PIN0_bm;
		PORTF.OUTSET = PIN1_bm;
		laserTimer.SetCOMPA(laserMultiPulsePeriod[0]/2);
		laserTimer.SetCOMPB(laserMultiPulsePeriod[0]/2);
		TCF0.PERBUF = laserMultiPulsePeriod[1];
		TCF0.CCABUF = laserMultiPulsePeriod[1]/2;
		TCF0.CCBBUF = laserMultiPulsePeriod[1]/2;
		TCF0.CNT = 0;
		laserTimer.Start(laserMultiPulsePeriod[0]);
	} else
	{
		TCF0.PERBUF = laserTimerPeriod;
		TCF0.CCABUF = laserTimerDutyCycle;
		TCF0.CCBBUF = laserTimerDutyCycle;
		laserTimer.SetCOMPA(laserTimerDutyCycle);
		laserTimer.SetCOMPB(laserTimerDutyCycle);
		//TCF0.CNT = 0;
		//laserTimer.ChannelReset(TIMER_CHANNEL_A);
		//laserTimer.ChannelReset(TIMER_CHANNEL_B);
		laserTimer.Start(laserTimerPeriod);
	}
}

void CLaserControlApp::StopLaser()
{
	laserTimer.Stop();
	laserTimer.ChannelSet(TIMER_CHANNEL_A);
	laserTimer.ChannelSet(TIMER_CHANNEL_B);
	PORTF.OUTSET = PIN0_bm;
	PORTF.OUTSET = PIN1_bm;
}