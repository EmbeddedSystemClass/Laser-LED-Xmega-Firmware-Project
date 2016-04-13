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
private :
	// Registers
	volatile uint8_t PIC_ID;
	
	// Variables
	volatile uint16_t Energy;
	volatile uint16_t Frequency;
	
	// State variables
	volatile uint16_t water_state_var;
	volatile uint16_t lamp_state_var;
	volatile uint16_t led_state_var;
	
	// Modules
	CMBSender* m_cpSender;
}; //CLaserControlApp

#endif //__CLASERCONTROLAPP_H__
