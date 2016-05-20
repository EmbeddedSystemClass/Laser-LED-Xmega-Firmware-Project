/* 
* CDGUSDatabase.cpp
*
* Created: 17.05.2016 14:37:33
* Author: Vladislav
*/


#include "CDGUSDatabase.h"

#define F_CPU 32000000

#include "util/delay.h"

// default constructor
CDGUSDatabase::CDGUSDatabase()
{
} //CDGUSDatabase

// default destructor
CDGUSDatabase::~CDGUSDatabase()
{
} //~CDGUSDatabase

void CDGUSDatabase::Initialize(CMBSender* sender, uint16_t vp)
{
	// GUI class initialization
	m_cpSender = sender;
	vp_addr = (vp >> 8) | (vp << 8);
	vp_addres = vp;
	data_transaction = 0;
}

void CDGUSDatabase::WriteToDatabase(void* data, uint16_t length, uint32_t addr)
{
	DGUS_WRITETOFLASH FLASH;
	
	FLASH.en = 0x5A;
	FLASH.op = 0x50;
	FLASH.addr = ((addr & 0xff) << 24) | ((addr & 0xff00) << 8) | ((addr & 0xff0000) >> 8) | ((addr & 0xff000000) >> 24);
	FLASH.vp = vp_addr;
	FLASH.len = (length >> 8) | (length << 8);
	
	// Write to SRAM
	m_cpSender->WriteDataToSRAMAsync(vp_addres, (uint16_t*)data, length);
	m_cpSender->WaitMODBUSTransmitter();
	
	// Database enable for writing
	m_cpSender->WriteDataToRegisterAsync(STRUCT_ADDR_WRITETOFLASH, (uint8_t*)&FLASH, sizeof(FLASH));
	m_cpSender->WaitMODBUSTransmitter();
	
	_delay_ms(400);
	
	//WriteDisable();
}

void CDGUSDatabase::ReadFromDatabase(void* data, uint16_t length, uint32_t addr)
{
	DGUS_WRITETOFLASH FLASH;
	
	FLASH.en = 0x5A;
	FLASH.op = 0xA0;
	FLASH.addr = ((addr & 0xff) << 24) | ((addr & 0xff00) << 8) | ((addr & 0xff0000) >> 8) | ((addr & 0xff000000) >> 24);
	FLASH.vp = vp_addr;
	FLASH.len = (length >> 8) | (length << 8);
	
	// Database enable for reading
	m_cpSender->WriteDataToRegisterAsync(STRUCT_ADDR_WRITETOFLASH, (uint8_t*)&FLASH, sizeof(FLASH));
	m_cpSender->WaitMODBUSTransmitter();
	
	// Read from SRAM
	m_cpSender->StartMODBUSVariableTransaction(vp_addres, length);
	m_cpSender->WaitMODBUSTransmitter();
	m_cpSender->WaitMODBUSListener();
	data_transaction = (uint16_t*)data;
}

void CDGUSDatabase::MapDatabaseToWrite(uint16_t vp, uint32_t addr, uint16_t length)
{
	DGUS_WRITETOFLASH FLASH;
	
	FLASH.en = 0x5A;
	FLASH.op = 0x50;
	FLASH.addr = ((addr & 0xff) << 24) | ((addr & 0xff00) << 8) | ((addr & 0xff0000) >> 8) | ((addr & 0xff000000) >> 24);
	FLASH.vp = (vp >> 8) | (vp << 8);
	FLASH.len = (length >> 8) | (length << 8);
	
	// Database enable for reading
	m_cpSender->WriteDataToRegisterAsync(STRUCT_ADDR_WRITETOFLASH, (uint8_t*)&FLASH, sizeof(FLASH));
	m_cpSender->WaitMODBUSTransmitter();
}

void CDGUSDatabase::MapDatabaseToRead(uint16_t vp, uint32_t addr, uint16_t length)
{
	DGUS_WRITETOFLASH FLASH;
	
	FLASH.en = 0x5A;
	FLASH.op = 0xA0;
	FLASH.addr = ((addr & 0xff) << 24) | ((addr & 0xff00) << 8) | ((addr & 0xff0000) >> 8) | ((addr & 0xff000000) >> 24);
	FLASH.vp = (vp >> 8) | (vp << 8);
	FLASH.len = (length >> 8) | (length << 8);
	
	// Database enable for reading
	m_cpSender->WriteDataToRegisterAsync(STRUCT_ADDR_WRITETOFLASH, (uint8_t*)&FLASH, sizeof(FLASH));
	m_cpSender->WaitMODBUSTransmitter();
}

void CDGUSDatabase::UnMap()
{
	uint8_t en = 0x00;
	m_cpSender->WriteDataToRegisterAsync(STRUCT_ADDR_WRITETOFLASH, (uint8_t*)&en, sizeof(en));
	m_cpSender->WaitMODBUSTransmitter();
}

void CDGUSDatabase::WriteEnable()
{
	uint8_t en = 0x5A;
	m_cpSender->WriteDataToRegisterAsync(STRUCT_ADDR_WRITETOFLASH, (uint8_t*)&en, sizeof(en));
	m_cpSender->WaitMODBUSTransmitter();
}

void CDGUSDatabase::WriteDisable()
{
	uint8_t en = 0x00;
	m_cpSender->WriteDataToRegisterAsync(STRUCT_ADDR_WRITETOFLASH, (uint8_t*)&en, sizeof(en));
	m_cpSender->WaitMODBUSTransmitter();
}

void CDGUSDatabase::OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length)
{
	if (data_transaction != 0)
		memcpy(data_transaction, data, length);
	data_transaction = 0;
}