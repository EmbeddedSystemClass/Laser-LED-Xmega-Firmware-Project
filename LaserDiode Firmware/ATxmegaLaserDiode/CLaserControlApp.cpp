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

extern uint16_t DATA[1024];

volatile DGUS_DATA m_structDGUSDATA_Fast;
volatile DGUS_DATA m_structDGUSDATA_Medium;
volatile DGUS_DATA m_structDGUSDATA_Slow;

extern uint16_t swap(uint16_t data);

void ConvertData(void* dst, void* src, uint16_t size, uint16_t offset = 0)
{
	uint16_t  length = size / 2;
	uint16_t* source = (uint16_t*)src;
	uint16_t* dest = (uint16_t*)dst;
	
	// swap bytes in words
	/*for (uint16_t i = 0; i < length; i++)
		dest[(i + offset) % length] = swap(source[i]);*/
		
	for (uint16_t i = 0; i < size; i++)
		((uint8_t*)dst)[((i + offset) % size) ^ 1] = ((uint8_t*)src)[i];
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
	if (addr == 0x0400)
	{
		ConvertData((void*)&DATA, (void*)data, length);
	}
	
	if (addr == VARIABLE_ADDR_PROFINDEX)
	{
		profileIndex = swap(*((uint16_t*)data));
	}
	
	if (addr == STRUCT_ADDR_DATA)
	{
		//memcpy((void*)&m_structDGUSDATA_Fast, (void*)data, length);
		//ConvertData((void*)&m_structDGUSDATA_Fast, (void*)data, length);
		switch (profile)
		{
			case WorkFast:
				ConvertData((void*)&m_structDGUSDATA_Fast, (void*)data, length);
			break;
			case WorkSlow:
				ConvertData((void*)&m_structDGUSDATA_Slow, (void*)data, length);
			break;
			case WorkMedium:
				ConvertData((void*)&m_structDGUSDATA_Medium, (void*)data, length);
			break;
			default:
				// Error
				CLaserBoard::Beep();
			break;
		}
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
		case PICID_WORKOnStart:
			state = APP_WORKOnStart;
		break;
		case PICID_WORKOnStop:
			state = APP_WORKOnStop;
		break;
		case PICID_WORKSTART:
			state = APP_WORKSTART;
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
			if (PIC_ID >= PICID_DATABASE_MIN && PIC_ID < PICID_DATABASE_MAX)
			{
				state = APP_DATABASE;
			}
			if (PIC_ID == PICID_DATABASE_MAX)
			{
				state = APP_DATABASE_START;
			}
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
	m_structDGUSDATA_Fast.Frequency = 10; // 10 Hz
	m_structDGUSDATA_Fast.DutyCycle = 50; // 50%
	m_structDGUSDATA_Fast.Duration  = ((1000 / m_structDGUSDATA_Fast.Frequency) * m_structDGUSDATA_Fast.DutyCycle) / 100; // ms
	m_structDGUSDATA_Fast.Intensity = 200; // W
	m_structDGUSDATA_Fast.Power     = (m_structDGUSDATA_Fast.Intensity * m_structDGUSDATA_Fast.DutyCycle) / 100; // W
	m_structDGUSDATA_Fast.Energy    = (m_structDGUSDATA_Fast.Intensity * m_structDGUSDATA_Fast.Duration) / 1000; // J
	
	// Slow profile
	m_structDGUSDATA_Slow.Frequency = 1; // 10 Hz
	m_structDGUSDATA_Slow.DutyCycle = 50; // 50%
	m_structDGUSDATA_Slow.Duration  = ((1000 / m_structDGUSDATA_Slow.Frequency) * m_structDGUSDATA_Slow.DutyCycle) / 100; // ms
	m_structDGUSDATA_Slow.Intensity = 200; // W
	m_structDGUSDATA_Slow.Power     = (m_structDGUSDATA_Slow.Intensity * m_structDGUSDATA_Slow.DutyCycle) / 100; // W
	m_structDGUSDATA_Slow.Energy    = (m_structDGUSDATA_Slow.Intensity * m_structDGUSDATA_Slow.Duration) / 1000; // J
	
	// Medium profile
	m_structDGUSDATA_Medium.Frequency = 5; // 10 Hz
	m_structDGUSDATA_Medium.DutyCycle = 50; // 50%
	m_structDGUSDATA_Medium.Duration  = ((1000 / m_structDGUSDATA_Medium.Frequency) * m_structDGUSDATA_Medium.DutyCycle) / 100; // ms
	m_structDGUSDATA_Medium.Intensity = 200; // W
	m_structDGUSDATA_Medium.Power     = (m_structDGUSDATA_Medium.Intensity * m_structDGUSDATA_Medium.DutyCycle) / 100; // W
	m_structDGUSDATA_Medium.Energy    = (m_structDGUSDATA_Medium.Intensity * m_structDGUSDATA_Medium.Duration) / 1000; // J
	
	// Initialize Laser timer
	laserTimerPeriod = (6250 / m_structDGUSDATA_Fast.Frequency) * 10;
	laserTimerDutyCycle = laserTimerPeriod - ((laserTimerPeriod / 100) * m_structDGUSDATA_Fast.DutyCycle);
	
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
	profile = WorkFast;
}

void CLaserControlApp::Start()
{	
	// Startup DGUS initialization
	uint16_t pic_id = swap(PICID_MAINMENU);
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
	static uint8_t DatabaseSelectedProfile = 0;
	
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
			{
				uint16_t pic_id = swap(PICID_MAINMENU);
				m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
				m_cpSender->WaitMODBUSTransmitter();
				state = APP_WORKFAST;
			}
		break;
		case APP_WORKFAST:
			//state = APP_WORKFAST;
		break;
		case APP_WORKMEDIUM:
			//state = APP_WORKFAST;
		break;
		case APP_WORKSLOW:
			//state = APP_WORKFAST;
		break;
		case APP_WORKSTART:
			//state = APP_WORKFAST;
		break;
		case APP_WORKSTARTED:
			//state = APP_WORKFAST;
			
			if (!laserBoard.Footswitch())
			{
				laserTimer.SetCOMPA(laserTimerDutyCycle);
				laserTimer.SetCOMPB(laserTimerDutyCycle);
				//laserTimer.SetPeriod(laserTimerPeriod);
				laserTimer.Start(laserTimerPeriod);
			}
			else
			{
				laserTimer.Stop();
				laserTimer.ChannelSet(TIMER_CHANNEL_A);
				laserTimer.ChannelSet(TIMER_CHANNEL_B);
			}
		break;
		
		// Commands
		case APP_WORKOnReady:
			{
				laserBoard.Relay2On();
				laserBoard.LaserPowerOn();
				
				uint16_t pic_id = swap(PICID_WORKSTART);
				m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
				m_cpSender->WaitMODBUSTransmitter();
				state = APP_WORKSTART;
			}
		break;
		case APP_WORKOnStart:
			{
				uint16_t data = ((uint16_t)((laserPower * 64) / 63)) << 2;  // (laserPower * 1024) / 1000)
				dacSPI.Send((uint8_t*)&data, sizeof(data));
				
				uint16_t pic_id = swap(PICID_WORKSTARTED);
				m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
				m_cpSender->WaitMODBUSTransmitter();
				state = APP_WORKSTARTED;

				//laserTimer.Start(12500);
			}
		break;
		case APP_WORKOnStop:
			{
				uint16_t data = 0;
				dacSPI.Send((uint8_t*)&data, sizeof(data));
				
				laserTimer.Stop();
				laserTimer.ChannelSet(TIMER_CHANNEL_A);
				laserTimer.ChannelSet(TIMER_CHANNEL_B);
				laserBoard.LaserPowerOff();
				laserBoard.Relay2Off();
				
				uint16_t pic_id = 0;
				switch (profile)
				{
					case WorkFast: 
						pic_id = swap(PICID_WORKFAST); 
						state = APP_WORKFAST;
						break;
					case WorkSlow: 
						pic_id = swap(PICID_WORKSLOW); 
						state = APP_WORKSLOW;
						break;
					case WorkMedium: 
						pic_id = swap(PICID_WORKMEDIUM); 
						state = APP_WORKMEDIUM;
						break;
				}
				m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
				m_cpSender->WaitMODBUSTransmitter();
			}
		break;
		
		// Phototype selector state
		case APP_PHOTOTYPESELECT:
			//state = APP_WORKFAST;
		break;
		case APP_PHOTOTYPE1:
			//state = APP_WORKFAST;
		break;
		case APP_PHOTOTYPE2:
			//state = APP_WORKFAST;
		break;
		case APP_PHOTOTYPE3:
			//state = APP_WORKFAST;
		break;
		case APP_PHOTOTYPE4:
			//state = APP_WORKFAST;
		break;
		case APP_PHOTOTYPE5:
			//state = APP_WORKFAST;
		break;
		case APP_PHOTOTYPE6:
			//state = APP_WORKFAST;
		break;
		case APP_DATABASE_START:
			{
				uint16_t pic_id = swap(27 + 14 - 3);
				m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
				m_cpSender->WaitMODBUSTransmitter();
				state = APP_DATABASE;
			}
		break;
		case APP_DATABASE:
			{
				//VARIABLE_ADDR_PROFINDEX
				m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_PROFINDEX, 2);
				m_cpSender->WaitMODBUSTransmitter();
				m_cpSender->WaitMODBUSListener();
				_delay_ms(50);
				
				DGUS_LINESDATA1 lines1 = {0};
				DGUS_LINESDATA2 lines2 = {0};
				DGUS_VALUESDATA value1 = {0};
				DGUS_VALUESDATA value2 = {0};
				char empty[33] = "Hello world!                    ";
				
				                //-------|-------|-------|-------|
				char value[33] = "  4321    1234    1234    1234  ";
				
				static int cnt = 0;
				static int lastprofile = 0;
				cnt+=1;
				if (cnt >= 32) cnt = 0;
				
				if (profileIndex != lastprofile) cnt = 0;
				
				lastprofile = profileIndex;
				
				// Names
				ConvertData((void*) lines1.line1 , (void*)empty, 32);
				ConvertData((void*) lines1.line2 , (void*)empty, 32);
				ConvertData((void*) lines1.line3 , (void*)empty, 32);
				ConvertData((void*) lines1.line4 , (void*)empty, 32);
				ConvertData((void*) lines1.line5 , (void*)empty, 32);
				ConvertData((void*) lines1.line6 , (void*)empty, 32);
				ConvertData((void*) lines1.line7 , (void*)empty, 32);
				ConvertData((void*) lines2.line8 , (void*)empty, 32);
				ConvertData((void*) lines2.line9 , (void*)empty, 32);
				ConvertData((void*) lines2.line10, (void*)empty, 32);
				ConvertData((void*) lines2.line11, (void*)empty, 32);
				ConvertData((void*) lines2.line12, (void*)empty, 32);
				ConvertData((void*) lines2.line13, (void*)empty, 32);
				ConvertData((void*) lines2.line14, (void*)empty, 32);
				
				switch (profileIndex)
				{
					case 1 : ConvertData((void*) lines1.line1 , (void*)empty, 32, cnt); break;
					case 2 : ConvertData((void*) lines1.line2 , (void*)empty, 32, cnt); break;
					case 3 : ConvertData((void*) lines1.line3 , (void*)empty, 32, cnt); break;
					case 4 : ConvertData((void*) lines1.line4 , (void*)empty, 32, cnt); break;
					case 5 : ConvertData((void*) lines1.line5 , (void*)empty, 32, cnt); break;
					case 6 : ConvertData((void*) lines1.line6 , (void*)empty, 32, cnt); break;
					case 7 : ConvertData((void*) lines1.line7 , (void*)empty, 32, cnt); break;
					case 8 : ConvertData((void*) lines2.line8 , (void*)empty, 32, cnt); break;
					case 9 : ConvertData((void*) lines2.line9 , (void*)empty, 32, cnt); break;
					case 10: ConvertData((void*) lines2.line10, (void*)empty, 32, cnt); break;
					case 11: ConvertData((void*) lines2.line11, (void*)empty, 32, cnt); break;
					case 12: ConvertData((void*) lines2.line12, (void*)empty, 32, cnt); break;
					case 13: ConvertData((void*) lines2.line13, (void*)empty, 32, cnt); break;
					case 14: ConvertData((void*) lines2.line14, (void*)empty, 32, cnt); break;
				}
				
				// Parameters
				ConvertData((void*)&value1.value1, (void*)value, 32);
				ConvertData((void*)&value1.value2, (void*)value, 32);
				ConvertData((void*)&value1.value3, (void*)value, 32);
				ConvertData((void*)&value1.value4, (void*)value, 32);
				ConvertData((void*)&value1.value5, (void*)value, 32);
				ConvertData((void*)&value1.value6, (void*)value, 32);
				ConvertData((void*)&value1.value7, (void*)value, 32);
				ConvertData((void*)&value2.value1, (void*)value, 32);
				ConvertData((void*)&value2.value2, (void*)value, 32);
				ConvertData((void*)&value2.value3, (void*)value, 32);
				ConvertData((void*)&value2.value4, (void*)value, 32);
				ConvertData((void*)&value2.value5, (void*)value, 32);
				ConvertData((void*)&value2.value6, (void*)value, 32);
				ConvertData((void*)&value2.value7, (void*)value, 32);
				
				// Param transfer
				m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_LINESDATA1 , (uint16_t*)&lines1, sizeof(lines1));
				m_cpSender->WaitMODBUSTransmitter();
				m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_LINESDATA2 , (uint16_t*)&lines2, sizeof(lines2));
				m_cpSender->WaitMODBUSTransmitter();
				m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_VALUESDATA1, (uint16_t*)&value1, sizeof(value1));
				m_cpSender->WaitMODBUSTransmitter();
				m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_VALUESDATA2, (uint16_t*)&value2, sizeof(value2));
				m_cpSender->WaitMODBUSTransmitter();
			}
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
				
				laserTimerPeriod = (6250 / m_structDGUSDATA_Fast.Frequency) * 10;
				laserTimerDutyCycle = laserTimerPeriod - ((laserTimerPeriod / 100) * DGUSDATA.DutyCycle);
				laserTimerDutyCyclems = DGUSDATA.DutyCycle;
				laserPower = m_structDGUSDATA_Fast.Intensity;
				DatabaseSelectedProfile = DGUSDATA.DatabaseSelectedProfile;
				
				m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_WRITEDATA, (uint16_t*)&DGUSDATA.Power, sizeof(DGUS_WRITEDATA));
			break;
			case WorkSlow:
				// Fast profile
				DGUSDATA.DutyCycle = m_structDGUSDATA_Slow.Duration * m_structDGUSDATA_Slow.Frequency / 10;
				DGUSDATA.Power     = (m_structDGUSDATA_Slow.Intensity * m_structDGUSDATA_Slow.DutyCycle) / 100;
				DGUSDATA.Energy    = m_structDGUSDATA_Slow.Intensity * m_structDGUSDATA_Slow.Duration / 1000;
				
				laserTimerPeriod = (6250 / m_structDGUSDATA_Slow.Frequency) * 10;
				laserTimerDutyCycle = laserTimerPeriod - ((laserTimerPeriod / 100) * DGUSDATA.DutyCycle);
				laserTimerDutyCyclems = DGUSDATA.DutyCycle;
				laserPower = m_structDGUSDATA_Slow.Intensity;
				DatabaseSelectedProfile = DGUSDATA.DatabaseSelectedProfile;
				
				m_cpSender->WriteDataToSRAMAsync(STRUCT_ADDR_WRITEDATA, (uint16_t*)&DGUSDATA.Power, sizeof(DGUS_WRITEDATA));
			break;
			case WorkMedium:
				// Fast profile
				DGUSDATA.DutyCycle = m_structDGUSDATA_Medium.Duration * m_structDGUSDATA_Medium.Frequency / 10;
				DGUSDATA.Power     = (m_structDGUSDATA_Medium.Intensity * m_structDGUSDATA_Medium.DutyCycle) / 100;
				DGUSDATA.Energy    = m_structDGUSDATA_Medium.Intensity * m_structDGUSDATA_Medium.Duration / 1000;
				
				laserTimerPeriod = (6250 / m_structDGUSDATA_Medium.Frequency) * 10;
				laserTimerDutyCycle = laserTimerPeriod - ((laserTimerPeriod / 100) * DGUSDATA.DutyCycle);
				laserTimerDutyCyclems = DGUSDATA.DutyCycle;
				laserPower = m_structDGUSDATA_Medium.Intensity;
				DatabaseSelectedProfile = DGUSDATA.DatabaseSelectedProfile;
				
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
