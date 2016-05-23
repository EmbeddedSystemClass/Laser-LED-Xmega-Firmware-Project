/* 
* CLaserControlApp.cpp
*
* Created: 11.04.2016 15:27:29
* Author: TPU_2
*/


#include "CLaserControlApp.h"
#include "CSoundPlayer.h"
#include <util/delay.h>


CTimerC timer;
extern CLaserBoard laserBoard;
extern CSoundPlayer player;
extern CDGUSDatabase Database;

uint16_t swap(uint16_t data)
{
	return (data >> 8) | (data << 8);
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
	uint16_t val = swap(*((uint16_t*)data));
	switch (addr)
	{
		case VARIABLE_ADDR_MIN:
			m_wSetMin = val;
			m_wMinutes = val;
		break;
		case VARIABLE_ADDR_SEC:
			m_wSetSec = val;
			m_wSeconds = val;
		break;
		case VARIABLE_ADDR_MSC:
			// Not used
		break;
		case VARIABLE_ADDR_PWR:
			m_wPower = val;
		break;
		case VARIABLE_ADDR_DATAPAGE:
			m_wDatabasePage = val;
		break;
		case VARIABLE_ADDR_SEL:
			m_wDatabaseSelInx = val;
		break;
		case VARIABLE_ADDR_DATABASE:
			Database.OnVariableReceived(addr, data, length);
		break;
		// Input password
		case VARIABLE_ADDR_PASSWORD:
			m_wPassword[0] = ((char*)data)[0];
			m_wPassword[1] = ((char*)data)[1];
			m_wPassword[2] = ((char*)data)[2];
			m_wPassword[3] = ((char*)data)[3];
		break;
		// Input secure settings
		case VARIABLE_ADDR_DACOFFSET:
			m_wDAC_Offset = val;
		break;
		case VARIABLE_ADDR_DACP:
			m_wDAC_P = val;
		break;
		case VARIABLE_ADDR_DACM:
			m_wDAC_M = val;
		break;
		
		default:
			// Error
			CLaserBoard::Beep();
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
		case PICID_SETUP:
			state = APP_SETUP;
		break;
		case PICID_TIMER:
			state = APP_RUN;
		break;
		case PICID_OnTimerStart:
			state = APP_OnTimerResume;
		break;
		case PICID_OnStart:
			state = APP_OnTimerStart;
		break;
		case PICID_OnPause:
			state = APP_OnTimerPause;
		break;
		case PICID_OnRestart:
			state = APP_OnTimerRestart;
		break;
		case PICID_OnStop:
			state = APP_OnTimerStop;
		break;
		case PICID_OnH_L:
			state = APP_OnHL;
		break;
		case PICID_OnTimerSave:
			state = APP_OnSaveSetup;
		break;
		
		case PICID_PROFILEPOP:
			state = APP_READPROFILE;
		break;
		case PICID_PROFILEPUSH:
			state = APP_SAVEPROFILE;
		break;
		case PICID_DATABASE:
			state = APP_SHOWDATABASE;
		break;
		case PICID_EDITPROFILE:
		case PICID_NEWPROFILE:
			state = APP_UNMAPDATABASE;
		break;
		
		case PICID_Password:
			state = APP_PASSWORD;
		break;
		case PICID_Service:
			state = APP_SERVICE;
		break;
		
		default:
			//state = APP_SETUPtoRUN_ANIM;
			state = APP_IDLE;
		break;
	}
}

// Initialization
void CLaserControlApp::Initialize(CMBSender* sender)
{
	// GUI class initialization
	m_cpSender = sender;
	timer.Initialize(WGM_Normal, CS_DIV256);
	timer.SetPeriod(25000); // Every 10 ms
	timer.SetOVFCallback(OnTimerStatic, this, TC_OVFINTLVL_LO_gc); // Enable interrupt
	
	// Set global variables
	PIC_ID = 0;
	m_wSetMin = 1;
	m_wSetSec = 30;
	m_wMillSec = 0;
	m_wMinutes = m_wSetMin;
	m_wSeconds = m_wSetSec;
	m_wPower = 0;
	m_wDatabasePage = 0;
	m_wDatabaseSelInx = 0;
}

void CLaserControlApp::Start()
{	
	// Startup DGUS initialization
	uint16_t pic_id = swap(1);
	m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
	m_cpSender->WaitMODBUSTransmitter();
	
	//Setup variables
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_MIN, (uint16_t*)&m_wSetMin, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_SEC, (uint16_t*)&m_wSetSec, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_MSC, (uint16_t*)&m_wMillSec, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_PWR, (uint16_t*)&m_wPower, 2);
	m_cpSender->WaitMODBUSTransmitter();
	
	uint16_t bar = 0;
	uint16_t bar1 = min(bar, 12);
	uint16_t bar2 = min(max(bar, 11), 24);
	uint16_t bar3 = max(bar, 23);
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR1, (uint16_t*)&bar1, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR2, (uint16_t*)&bar2, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR3, (uint16_t*)&bar3, 2);
	m_cpSender->WaitMODBUSTransmitter();
	
	state = APP_SETUP;
}

// Process GUI
void CLaserControlApp::Run()
{
	// Get PIC ID
	m_cpSender->StartMODBUSRegisterTransaction(REGISTER_ADDR_PICID, 2);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WaitMODBUSListener();
	_delay_ms(50);
	
	static uint16_t anim = 5;
	uint16_t pic_id;
	
	static uint16_t bar = 0;
	uint16_t bar1 = 0;
	uint16_t bar2 = 0;
	uint16_t bar3 = 0;
	
	switch (state)
	{
		case APP_LOGO:
			Start(); // Start if logo
		break;
		case APP_SETUP:
			// Get Variables
			m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_MIN, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WaitMODBUSListener();
			_delay_ms(50);
			
			m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_SEC, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WaitMODBUSListener();
			_delay_ms(50);
			
			m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_PWR, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WaitMODBUSListener();
			_delay_ms(50);
			
			bar = (m_wPower * 34) / 100;
			bar1 = min(bar, 12);
			bar2 = min(max(bar, 11), 24);
			bar3 = min(max(bar, 23), 34);
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR1, (uint16_t*)&bar1, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR2, (uint16_t*)&bar2, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR3, (uint16_t*)&bar3, 2);
			m_cpSender->WaitMODBUSTransmitter();
		break;
		case APP_SETUPtoRUN_ANIM:
			// Play animation
			anim++;
			if (anim > 9) {
				anim = 9;
				state = APP_RUN;
			}
			pic_id = swap(anim);
			
			// Timer setup to timer run animation
			m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
			m_cpSender->WaitMODBUSTransmitter();
		break;
		case APP_RUN:
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_MIN, (uint16_t*)&m_wMinutes, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_SEC, (uint16_t*)&m_wSeconds, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_MSC, (uint16_t*)&m_wMillSec, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			/*m_wPower++;
			if (m_wPower > 100) m_wPower = 0;
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_PWR, (uint16_t*)&m_wPower, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			bar++;
			if (bar > 34) bar = 0;
			bar1 = min(bar, 12);
			bar2 = min(max(bar, 11), 24);
			bar3 = max(bar, 23);
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR1, (uint16_t*)&bar1, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR2, (uint16_t*)&bar2, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR3, (uint16_t*)&bar3, 2);
			m_cpSender->WaitMODBUSTransmitter();*/
		break;
		case APP_OnTimerStart:
			// Set Run state
			anim = 5;
			pic_id = swap(PICID_TIMER);
			m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			m_wMinutes = m_wSetMin;
			m_wSeconds = m_wSetSec;
			m_wMillSec = 0;
			
			// Start timer
			timer.Start(25000);
			laserBoard.Relay1On();
			
			state = APP_RUN;
		break;
		case APP_OnTimerResume:
			// Set Run state
			//m_wPower = 0;
			anim = 5;
			pic_id = swap(PICID_TIMER);
			m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			// Start timer
			timer.Start(25000);
			laserBoard.Relay1On();
			
			state = APP_RUN;
		break;
		case APP_OnTimerStop:
			// Set Stop (return to Setup) state
			pic_id = swap(PICID_SETUP);
			m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			// Stop timer
			timer.Stop();
			laserBoard.Relay1Off();
			
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_MIN, (uint16_t*)&m_wSetMin, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_SEC, (uint16_t*)&m_wSetSec, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_MSC, (uint16_t*)&m_wMillSec, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			m_wMinutes = m_wSetMin;
			m_wSeconds = m_wSetSec;
			m_wMillSec = 0;
			
			state = APP_SETUP;
		break;
		case APP_OnTimerPause:
			// Pause (return to Run) state
			pic_id = swap(PICID_TIMERPAUSED);
			m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
			m_cpSender->WaitMODBUSTransmitter();
			timer.Stop();
			
			laserBoard.Relay1Off();
			
			state = APP_RUN;
		break;
		case APP_OnTimerRestart:
			// Restart (return to Run) state
			pic_id = swap(PICID_TIMER);
			m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			// Reset
			m_wMinutes = m_wSetMin;
			m_wSeconds = m_wSetSec;
			m_wMillSec = 0;
			
			timer.Stop();
			
			player.SoundStart(1000, 50, 1);
			player.SoundStop();
			_delay_ms(50);
			player.SoundStart(1000, 50, 1);
			player.SoundStop();
			_delay_ms(50);
			
			// Stop timer
			timer.Start(25000);
			laserBoard.Relay1On();
			
			state = APP_RUN;
		break;
		case APP_OnHL:
			// Turn HL (return to Setup) state
			pic_id = swap(PICID_SETUP);
			m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			static bool hl = true;
			
			if (hl)
				laserBoard.Relay2On();
			else
				laserBoard.Relay2Off();
				
			hl = !hl;
			
			state = APP_SETUP;
		break;
		case APP_OnSaveSetup :
			pic_id = swap(PICID_SETUP);
			m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
			m_cpSender->WaitMODBUSTransmitter();
		break;
		
		case APP_SHOWDATABASE:
			//Database.UnMap();
			
			m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_DATAPAGE, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WaitMODBUSListener();
			_delay_ms(50);
			m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_SEL, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WaitMODBUSListener();
			_delay_ms(50);
		
			Database.MapDatabaseToRead(VARIABLE_ADDR_DATABASE, DGUS_DATABASE_ADDR + m_wDatabasePage*PROFILE_SIZE, 0x0C00);
		break;
		case APP_READPROFILE:
			Database.MapDatabaseToRead(VARIABLE_ADDR_PROFILE, DGUS_DATABASE_ADDR + (m_wDatabasePage + m_wDatabaseSelInx)*PROFILE_SIZE, 0x0100);
		break;
		case APP_SAVEPROFILE:
			Database.MapDatabaseToWrite(VARIABLE_ADDR_PROFILE, DGUS_DATABASE_ADDR + (m_wDatabasePage + m_wDatabaseSelInx)*PROFILE_SIZE, 0x0100);
		break;
		case APP_UNMAPDATABASE:
			Database.UnMap();
		break;
		
		case APP_PASSWORD:
			m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_PASSWORD, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WaitMODBUSListener();
			_delay_ms(50);
			
			if (strcmp((char*)m_wPassword, "1234") == 0)
			{
				pic_id = swap(PICID_Service);
				m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
				m_cpSender->WaitMODBUSTransmitter();
			}/* else
			{
				pic_id = swap(PICID_MAINMENU);
				m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
				m_cpSender->WaitMODBUSTransmitter();
			}*/
		break;
		
		default:
			Database.UnMap();
			m_wPassword[0] = 0;
			m_wPassword[1] = 0;
			m_wPassword[2] = 0;
			m_wPassword[3] = 0;
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_PASSWORD, (uint16_t*)&m_wPassword, 2);
			m_cpSender->WaitMODBUSTransmitter();
		break;
	}
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
				player.SoundStart(1000, 1000, 0);
				player.SoundStop();
				//player.beep(1000, 1000);
				return;
			}
			m_wSeconds = 60;
			m_wMinutes--;
		}
		m_wMillSec = 100; // Every 10 ms
		m_wSeconds--;
		if (m_wMinutes == 0 && m_wSeconds < 10)
		{
			if (m_wMinutes == 0 && m_wSeconds < 5)
			{
				player.SoundStart(1000, 100, 0);
				player.SoundStop();
				//player.beep(1000, 100);
			}
			else
			{
				player.SoundStart(1000, 50, 1);
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
	m_wMillSec-=10;
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
