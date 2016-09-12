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

#define LASER_CNT_EEPROMADDR	0x0001

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
	APP_LOGO = 0x0000,
	APP_WORKIDLE = 0x0001,
	APP_WORKPREPARE = 0x0002,
	APP_WORKSETUP = 0x0004,
	APP_WORKREADY = 0x0008,
	APP_WORKPOWERON = 0x0010,
	APP_WORKLIGHT = 0x0020,
	APP_WORKTEMPERWAIT = 0x0040,
	
	// Events
	APP_WORKOnReady = 0x0100,
	APP_WORKOnPowerOn = 0x0200,
	APP_WORKOnPowerOff = 0x0300,
	APP_OnPhototypeSelect = 0x0400,
	
	// Database
	APP_DATABASE = 0x1000,
	APP_READPROFILE = 0x2000,
	APP_SAVEPROFILE = 0x3000,
	APP_UNMAPDATABASE = 0x4000,
	
	// Errors
	APP_POWERERR = 0x0500,
	APP_FLOWERR = 0x0600,
	APP_TEMPERERR = 0x0700
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
	void FastRun();
	void SetLaserDiodePower();
	
	// helper methods
	DGUS_LASERSETTINGS CalculateLaserSettings(DGUS_LASERPROFILE *profile);
	void SetPictureId(uint16_t pic_id);
	void SetPictureIdAsync(uint16_t pic_id);
	void GetVariable(uint16_t addr, uint16_t size);
	void SetVariable(uint16_t addr, uint16_t* data, uint16_t size);
	
	static void OnPWMTimerOVFStatic(void* sender);
	static void OnPWMTimerCMPStatic(void* sender);
	static void OnPWMTimerREDStatic(void* sender);
	static void OnPWMTimerGRNStatic(void* sender);
	static void OnPWMTimerBLUStatic(void* sender);
	static void OnINT0Static(void* sender);
	static void OnTimerStatic(void* sender);
	static void OnLaserTimerStopStatic(void* sender);
	static void OnLaserTimerStatic(void* sender);
	
protected :
	void OnTimer();
	void OnLaserTimer();
	void OnLaserTimerStop();
	void OnTimeout();
	void OnPWMTimerOVF();
	void OnPWMTimerCMP();
	void OnPWMTimerRED();
	void OnPWMTimerGRN();
	void OnPWMTimerBLU();
	void OnINT0();
	
	void MelaninPreset(uint16_t melanin);
	void PhototypePreset(uint16_t phototype);
	bool CheckLimits(uint16_t &freq, uint16_t &duration, APP_PROFILE mode);
	
private :
	// application state
	APP_STATE state;
	APP_PROFILE Profile;
	
	// Registers
	volatile uint8_t PIC_ID;
	volatile uint8_t PIC_ID_last;
	volatile bool update;
	volatile bool prepare;
	volatile bool peltier_en;
	//volatile bool isstarted;
	
	// Limits
	volatile uint16_t m_wMaxEnergy;
	volatile uint16_t m_wMaxEnergy_;
	volatile uint16_t m_wMaxDuration;
	
	// variables
	volatile DGUS_LASERPROFILE	m_structLaserProfile[4];
	volatile DGUS_LASERSETTINGS	m_structLaserSettings;
	
	// laser settings
	uint16_t laserTimerPeriod;
	uint16_t laserTimerDutyCycle;
	uint16_t laserTimerDutyCyclems;
	uint16_t laserPower;
	uint32_t laserCounter;
	uint32_t laserCounterSession;
	
	// all laser settings
	DGUS_LASERDIODE laserDiodeData;
	
	// timer settings
	volatile uint16_t m_wMinutes;
	volatile uint16_t m_wSeconds;
	volatile uint16_t m_wMillSec;
	volatile uint16_t m_wSetMin;
	volatile uint16_t m_wSetSec;
	
	// Flow control
	volatile uint16_t m_wFlow;
	// Single pulse dead time counter
	volatile uint16_t m_wDeadTime;
	
	// Modules
	CMBSender* m_cpSender;
}; //CLaserControlApp

#endif //__CLASERCONTROLAPP_H__
