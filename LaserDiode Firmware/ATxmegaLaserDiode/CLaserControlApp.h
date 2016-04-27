/* 
* CLaserControlApp.h
*
* Created: 11.04.2016 15:27:29
* Author: TPU_2
*/


#ifndef __CLASERCONTROLAPP_H__
#define __CLASERCONTROLAPP_H__

#include <stdbool.h>
#include "CTimerC.h"
#include "CTimerF.h"
#include "CDGUSUSART.h"

#include "DGUSGUI.h"
#include "CLaserBoard.h"

#include "MODBUS/CMBSender.h"

#include "LaserLib/CSPI.h"
#include "LaserLib/CUSART.h"
#include "LaserLib/sys_interrupts.h"

typedef enum APP_STATE_ENUM
{
	// DGUS State
	APP_LOGO,
	APP_WORKFAST,
	APP_WORKMEDIUM,
	APP_WORKSLOW,
	APP_WORKSTART,
	APP_WORKSTARTED,
	
	// Commands
	APP_WORKOnReady,
	APP_WORKOnStart,
	APP_WORKOnStop,
	
	// Phototype selector state
	APP_PHOTOTYPESELECT,
	APP_PHOTOTYPE1,
	APP_PHOTOTYPE2,
	APP_PHOTOTYPE3,
	APP_PHOTOTYPE4,
	APP_PHOTOTYPE5,
	APP_PHOTOTYPE6
} APP_STATE, *PAPP_STATE;

typedef enum APP_PROFILE_ENUM
{
	WorkFast,
	WorkMedium,
	WorkSlow
} APP_PROFILE, *PAPP_PROFILE;

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
	
	// Modules
	CMBSender* m_cpSender;
}; //CLaserControlApp

#endif //__CLASERCONTROLAPP_H__
