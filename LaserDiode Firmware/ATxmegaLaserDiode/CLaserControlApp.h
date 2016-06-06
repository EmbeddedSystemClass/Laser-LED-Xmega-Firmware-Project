/* 
* CLaserControlApp.h
*
* Created: 11.04.2016 15:27:29
* Author: TPU_2
*/


#ifndef __CLASERCONTROLAPP_H__
#define __CLASERCONTROLAPP_H__

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
	
	// Phototype selector state
	APP_PHOTOTYPESELECT,
	APP_PHOTOTYPE1,
	APP_PHOTOTYPE2,
	APP_PHOTOTYPE3,
	APP_PHOTOTYPE4,
	APP_PHOTOTYPE5,
	APP_PHOTOTYPE6,
	
	APP_DATABASE,
	APP_DATABASE_START
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
	
protected :
	void OnTimer();
	void OnLaserTimer();
	void OnTimeout();
	static void OnTimerStatic(void* sender);
	static void OnLaserTimerStatic(void* sender);
	
private :
	// application state
	APP_STATE state;
	APP_PROFILE profile;
	
	// Registers
	volatile uint8_t PIC_ID;
	volatile bool update;
	
	// variables
	uint16_t laserTimerPeriod;
	uint16_t laserTimerDutyCycle;
	uint16_t laserTimerDutyCyclems;
	uint16_t laserPower;
	uint16_t profileIndex;
	uint16_t databaseOffset;
	
	// Modules
	CMBSender* m_cpSender;
}; //CLaserControlApp

#endif //__CLASERCONTROLAPP_H__
