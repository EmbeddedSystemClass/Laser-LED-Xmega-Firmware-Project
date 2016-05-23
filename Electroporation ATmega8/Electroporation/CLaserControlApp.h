/* 
* CLaserControlApp.h
*
* Created: 11.04.2016 15:27:29
* Author: TPU_2
*/


#ifndef __CLASERCONTROLAPP_H__
#define __CLASERCONTROLAPP_H__


#include "DGUSGUI.h"
#include "CDGUSUSART.h"
#include "CLaserBoard.h"
#include "MODBUS/CMBSender.h"
#include "LaserLib/CUSART.h"
#include "LaserLib/sys_interrupts.h"
#include "CDGUSDatabase.h"

typedef enum APP_STATE_ENUM
{
	APP_LOGO,
	APP_SETUP,
	APP_SETUPtoRUN_ANIM,
	APP_RUN,
	APP_OnTimerStart,
	APP_OnTimerResume,
	APP_OnTimerPause,
	APP_OnTimerStop,
	APP_OnTimerRestart,
	APP_OnHL,
	APP_OnSaveSetup,
	
	APP_SAVEPROFILE,
	APP_READPROFILE,
	APP_SHOWDATABASE
} APP_STATE, *PAPP_STATE;

class CLaserControlApp : public CMBEventsHandler
{
public:
	CLaserControlApp();
	~CLaserControlApp();

	// CMBEventsHandler
	virtual void OnTransactionCallback(uint8_t* data, uint16_t length);
	virtual void OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length);
	virtual void OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length);
	
	// Initialization
	void Initialize(CMBSender* sender);
	void Start();
	
	// Process GUI
	void Run();
	
	static void OnPWMStatic(void* sender);
	static void OnTimerStatic(void* sender);
	static void OnEncoderStatic(void* sender);
	
protected :
	void OnPWM();
	void OnTimer();
	void OnEncoder();
	void OnTimeout();
	
private :
	// application state
	bool ready;
	APP_STATE state;
	
	// Registers
	volatile uint8_t PIC_ID;
	
	// Variables
	volatile uint16_t m_wMinutes;
	volatile uint16_t m_wSeconds;
	volatile uint16_t m_wMillSec;
	volatile uint16_t m_wSetMin;
	volatile uint16_t m_wSetSec;
	volatile uint16_t m_wPower;
	
	volatile int16_t m_wEncoder;
	
	// Modules
	CMBSender* m_cpSender;
}; //CLaserControlApp

#endif //__CLASERCONTROLAPP_H__