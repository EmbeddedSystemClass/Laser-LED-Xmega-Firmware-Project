/* 
* CLaserControlApp.h
*
* Created: 11.04.2016 15:27:29
* Author: TPU_2
*/


#ifndef __CLASERCONTROLAPP_H__
#define __CLASERCONTROLAPP_H__

#define MAX_LASER_POWER		200 //W

// Standard libraries
#include <stdbool.h>

// DGUS
#include "DGUSGUI.h"

// MODBUS
#include "MODBUS/CMBSender.h"

// Periphery
#include "Periphery/CTimerC.h"
#include "Periphery/CTimerF.h"
#include "Periphery/CDGUSUSART.h"
#include "Periphery/CLaserBoard.h"

// Laser libraries
#include "LaserLib/CSPI.h"
#include "LaserLib/CUSART.h"
#include "LaserLib/sys_interrupts.h"

typedef enum APP_PROFILE_ENUM
{
	PROFILE_FAST	= 4,
	PROFILE_MEDIUM	= 3,
	PROFILE_SLOW	= 2,
	PROFILE_SINGLE	= 1,
	PROFILE_DEFAULT = 0
} APP_PROFILE, *PAPP_PROFILE;

typedef enum APP_STATE_ENUM
{
	// DGUS State
	APP_LOGO,
	APP_WORKIDLE,
	APP_WORKPREPARE,
	APP_WORKSETUP,
	APP_WORKREADY,
	APP_WORKPOWERON,
	APP_WORKLIGHT,
	
	// Events
	APP_WORKOnReady,
	APP_WORKOnPowerOn,
	APP_WORKOnPowerOff,
	
	// Database
	APP_DATABASE,
	APP_READPROFILE,
	APP_SAVEPROFILE,
	APP_UNMAPDATABASE
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
	
	// helper methods
	DGUS_LASERSETTINGS CalculateLaserSettings(DGUS_LASERPROFILE *profile);
	void SetPictureId(uint16_t pic_id);
	void GetVariable(uint16_t addr, uint16_t size);
	void SetVariable(uint16_t addr, uint16_t* data, uint16_t size);
	
protected :
	void OnTimer();
	void OnLaserTimer();
	void OnTimeout();
	static void OnTimerStatic(void* sender);
	static void OnLaserTimerStatic(void* sender);
	
private :
	// application state
	APP_STATE state;
	APP_PROFILE Profile;
	
	// Registers
	volatile uint8_t PIC_ID;
	volatile bool update;
	volatile bool prepare;
	
	// variables
	volatile DGUS_LASERPROFILE	m_structLaserProfile[4];
	volatile DGUS_LASERSETTINGS	m_structLaserSettings;
	
	// laser settings
	uint16_t laserTimerPeriod;
	uint16_t laserTimerDutyCycle;
	uint16_t laserTimerDutyCyclems;
	uint16_t laserPower;
	
	// all laser settings
	DGUS_LASERDIODE laserDiodeData;
	
	// timer settings
	volatile uint16_t m_wMinutes;
	volatile uint16_t m_wSeconds;
	volatile uint16_t m_wMillSec;
	volatile uint16_t m_wSetMin;
	volatile uint16_t m_wSetSec;
	
	// Modules
	CMBSender* m_cpSender;
}; //CLaserControlApp

#endif //__CLASERCONTROLAPP_H__
