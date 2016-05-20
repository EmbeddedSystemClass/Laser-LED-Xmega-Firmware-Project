/* 
* CLaserControlApp.cpp
*
* Created: 11.04.2016 15:27:29
* Author: TPU_2
*/


#include "CLaserControlApp.h"
#include "CRelayTimer.h"
#include "CSensorADC.h"
#include <util/delay.h>
#include <avr/eeprom.h>

extern CSensorADC  adc;
extern CRelayTimer relayTimer;
extern CLaserBoard laserBoard;

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
		case VARIABLE_ADDR_DATABASE:
			Database.OnVariableReceived(addr, data, length);
		break;
		default:;
			// Error
			//CLaserBoard::Beep();
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
			ready = true;
		break;
		case PICID_SETUP:
			state = APP_SETUP;
			ready = false;
		break;
		case PICID_TIMER:
			state = APP_RUN;
			ready = false;
		break;
		case PICID_OnTimerStart:
			state = APP_OnTimerResume;
			ready = true;
		break;
		case PICID_OnStart:
			state = APP_OnTimerStart;
			ready = true;
		break;
		case PICID_OnPause:
			state = APP_OnTimerPause;
			ready = true;
		break;
		case PICID_OnRestart:
			state = APP_OnTimerRestart;
			ready = true;
		break;
		case PICID_OnStop:
			state = APP_OnTimerStop;
			ready = true;
		break;
		case PICID_OnH_L:
			state = APP_OnHL;
			ready = true;
		break;
		case PICID_OnTimerSave:
			state = APP_OnSaveSetup;
			ready = true;
		break;
		
		case PICID_PROFILEPOP:
			state = APP_READPROFILE;
			ready = true;
		break;
		case PICID_PROFILEPUSH:
			state = APP_SAVEPROFILE;
			ready = true;
		break;
		case PICID_DATABASE:
			state = APP_SHOWDATABASE;
			ready = true;
		break;
		
		default:
			//state = APP_SETUPtoRUN_ANIM;
			ready = true;
		break;
	}
}

// Initialization
void CLaserControlApp::Initialize(CMBSender* sender)
{
	// GUI class initialization
	m_cpSender = sender;
	//relayTimer.Initialize(WGM_CTC, COM_Clear, CS_DIV64, false);
	//timer.SetPeriod(25000); // Every 10 ms
	//relayTimer.SetOVFCallback(OnTimerStatic, this); // Enable interrupt
	
	// Set global variables
	PIC_ID = 0;
	m_wSetMin = 1;
	m_wSetSec = 30;
	m_wMillSec = 0;
	m_wMinutes = m_wSetMin;
	m_wSeconds = m_wSetSec;
	m_wPower = 0;
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
	m_wPower = 0;
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
	
	laserBoard.SetDACValue(0);
	
	state = APP_SETUP;
	
	m_wEncoder = 0;
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
	uint16_t msec;
	
	static uint16_t bar = 0;
	uint16_t bar1 = 0;
	uint16_t bar2 = 0;
	uint16_t bar3 = 0;
	
	if (state != APP_SETUP) m_wEncoder = 0;
	
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
			_delay_ms(10);
			
			m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_SEC, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WaitMODBUSListener();
			_delay_ms(10);
			
			m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_PWR, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WaitMODBUSListener();
			_delay_ms(10);
			
			bar = (adc.GetValue() * 34) / 1024;
			//bar = (m_wPower * 34) / 100;
			bar1 = min(bar, 12);
			bar2 = min(max(bar, 11), 24);
			bar3 = min(max(bar, 23), 34);
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR1, (uint16_t*)&bar1, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR2, (uint16_t*)&bar2, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR3, (uint16_t*)&bar3, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			laserBoard.SetDACValue((m_wPower * 512) / 25); // (Power * 2048) / 100
			
			if (m_wEncoder != 0)
			{
				if (int16_t(m_wPower) < -m_wEncoder)
				{
					m_wPower = 0;
				}
				else
				{
					m_wPower += m_wEncoder;
					if (m_wPower > 100) m_wPower = 100;
				}
				
				m_wEncoder = 0;
			
				m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_PWR, (uint16_t*)&m_wPower, 2);
				m_cpSender->WaitMODBUSTransmitter();
			}
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
			msec = m_wMillSec / 10;
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_MSC, (uint16_t*)&msec, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_PWR, (uint16_t*)&m_wPower, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			/*m_wPower++;
			if (m_wPower > 100) m_wPower = 0;
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_PWR, (uint16_t*)&m_wPower, 2);
			m_cpSender->WaitMODBUSTransmitter();*/
			
			bar = (m_wPower * 34) / 100;
			bar1 = min(bar, 12);
			bar2 = min(max(bar, 11), 24);
			bar3 = max(bar, 23);
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR1, (uint16_t*)&bar1, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR2, (uint16_t*)&bar2, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_BAR3, (uint16_t*)&bar3, 2);
			m_cpSender->WaitMODBUSTransmitter();
		break;
		case APP_OnTimerStart:
			// Set Run state
			//m_wPower = 0;
			anim = 5;
			pic_id = swap(PICID_TIMER);
			m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			m_wMinutes = m_wSetMin;
			m_wSeconds = m_wSetSec;
			m_wMillSec = 0;
			
			// Start timer
			relayTimer.Start((uint8_t)125);
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
			relayTimer.Start((uint8_t)125);
			laserBoard.Relay1On();
		
			state = APP_RUN;
		break;
		case APP_OnTimerStop:
			// Set Stop (return to Setup) state
			pic_id = swap(PICID_SETUP);
			m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			// Stop timer
			relayTimer.Stop();
			laserBoard.Relay1Off();
			
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_MIN, (uint16_t*)&m_wSetMin, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_SEC, (uint16_t*)&m_wSetSec, 2);
			m_cpSender->WaitMODBUSTransmitter();
			msec = m_wMillSec / 10;
			m_cpSender->WriteDataToSRAMAsync(VARIABLE_ADDR_MSC, (uint16_t*)&msec, 2);
			m_cpSender->WaitMODBUSTransmitter();
			
			m_wMinutes = m_wSetMin;
			m_wSeconds = m_wSetSec;
			m_wMillSec = 0;
			m_wPower = 0;
			
			state = APP_SETUP;
		break;
		case APP_OnTimerPause:
			// Pause (return to Run) state
			pic_id = swap(PICID_TIMERPAUSED);
			m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
			m_cpSender->WaitMODBUSTransmitter();
			relayTimer.Stop();
			
			laserBoard.Relay1Off();
			
			state = APP_RUN;
		break;
		case APP_OnTimerRestart:
			// Restart (return to Run) state
			pic_id = swap(PICID_TIMER);
			m_cpSender->WriteDataToRegisterAsync(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
			m_cpSender->WaitMODBUSTransmitter();
			relayTimer.Stop();
			
			// Reset
			m_wMinutes = m_wSetMin;
			m_wSeconds = m_wSetSec;
			m_wMillSec = 0;
			
			// Stop timer
			relayTimer.Start((uint8_t)125);
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
				return;
			}
			m_wSeconds = 60;
			m_wMinutes--;
		}
		m_wMillSec = 1000; // Every 10 ms
		m_wSeconds--;
	}
	m_wMillSec -= 8;
}

void CLaserControlApp::OnEncoder()
{
	if ((PIND & (1 << PD6)) != 0)
	{
		//if (m_wEncoder != 100)
			m_wEncoder += 1;
	}
	else
	{
		//if (m_wEncoder != 0)
			m_wEncoder -= 1;
	}
}

void CLaserControlApp::OnPWM()
{
	static int16_t pwm_pulse = 0;
	static int16_t dx = 1;
	static uint8_t prescale = 0;
	
	prescale++;
	TIFR &= (1 << TOIE1);
	
	if (ready)
	{		
		if (prescale % 20 == 0)
		{
			if (pwm_pulse == 1023) dx = -1;
			if (pwm_pulse < 1) dx = 1;
			
			pwm_pulse+= dx;
			
			OCR1A = pwm_pulse;
		}
	}
	else
	{
		uint16_t power = m_wPower;
		
		if (int16_t(power) < -m_wEncoder)
		{
			power = 0;
		}
		else
		{
			power += m_wEncoder;
			if (power > 100) power = 100;
		}
		
		OCR1A = (power * 255)/25;  //m_wPower * 1024 / 100
	}
}

void CLaserControlApp::OnTimeout()
{
	relayTimer.Stop();
	laserBoard.Relay1Off();
}

void CLaserControlApp::OnPWMStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnPWM();
}

void CLaserControlApp::OnTimerStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnTimer();
}

void CLaserControlApp::OnEncoderStatic(void* sender)
{
	CLaserControlApp* app = (CLaserControlApp*)sender;
	app->OnEncoder();
}
