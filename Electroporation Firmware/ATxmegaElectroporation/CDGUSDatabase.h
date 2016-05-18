/* 
* CDGUSDatabase.h
*
* Created: 17.05.2016 14:37:33
* Author: Vladislav
*/


#ifndef __CDGUSDATABASE_H__
#define __CDGUSDATABASE_H__

// DGUS
#include "DGUSGUI.h"
#include "CDGUSUSART.h"
#include "MODBUS/CMBSender.h"
#include <string.h>

#define STRUCT_ADDR_WRITETOFLASH	0x56	// 1 bytes

typedef struct DGUS_WRITETOFLASH_STRUCT
{
	uint8_t en;
	uint8_t op;
	uint32_t addr;
	uint16_t vp;
	uint16_t len;
} DGUS_WRITETOFLASH;

class CDGUSDatabase
{
public:
	CDGUSDatabase();
	~CDGUSDatabase();
	
	// Initialization
	void Initialize(CMBSender* sender, uint16_t vp);
	
	// Database functions
	void WriteEnable();
	void WriteDisable();
	void WriteToDatabase(void* data, uint16_t length, uint32_t addr);
	void ReadFromDatabase(void* data, uint16_t length, uint32_t addr);
	
	// Callback functions
	void OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length);
protected:
private:

	// Modules
	uint16_t* data_transaction;
	CMBSender* m_cpSender;
	uint16_t vp_addr;
	uint16_t vp_addres;
}; //CDGUSDatabase

#endif //__CDGUSDATABASE_H__
