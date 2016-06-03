/*
 * DGUSGUI.h
 *
 * Created: 11.04.2016 15:07:36
 *  Author: Vladislav Alex. O.
 */ 


#ifndef DGUSGUI_H_
#define DGUSGUI_H_

#include <stdint.h>

// Variables & registers addresses
#define REGISTER_ADDR_PICID		0x03
#define REGISTER_DATABASE_EN	0x56	// 1 bytes
#define REGISTER_DATABASE_OP	0x57	// 1 bytes
#define REGISTER_DATABASE_ADDR	0x58	// 4 bytes
#define REGISTER_DATABASE_VP	0x5C	// 2 bytes
#define REGISTER_DATABASE_LEN	0x5E	// 2 bytes

#define VARIABLE_ADDR_FREQ		0x0001
#define VARIABLE_ADDR_DURATION	0x0002
#define VARIABLE_ADDR_INTENSITY	0x0003
#define VARIABLE_ADDR_POWER		0x0004
#define VARIABLE_ADDR_ENERGY	0x0005
#define VARIABLE_ADDR_DUTYCYCLE	0x0006
#define VARIABLE_ADDR_PROFINDEX	0x0007

// Picture ids
/*#define PICID_LOGO				0x0000
#define PICID_WORKFAST			0x0001
#define PICID_WORKMEDIUM		0x0002
#define PICID_WORKSLOW			0x0003
// Events
#define PICID_WORKOnReady		0x0007
#define PICID_WORKOnStart		0x0008
#define PICID_WORKOnStop		0x0009
// Work States
#define PICID_WORKSTART			0x000a
#define PICID_WORKSTARTED		0x000b
// Phototype
#define PICID_PHOTOTYPESELECT	0x000c
#define PICID_PHOTOTYPE1		0x000e
#define PICID_PHOTOTYPE2		0x000f
#define PICID_PHOTOTYPE3		0x0010
#define PICID_PHOTOTYPE4		0x0011
#define PICID_PHOTOTYPE5		0x0012
#define PICID_PHOTOTYPE6		0x0013
// Database
#define PICID_DATABASE_MIN		0x0018
#define PICID_DATABASE_MAX		0x0029
// Main menu
#define PICID_MAINMENU			0x0014*/

#define PICID_LOGO				0
#define PICID_LOGIN				1
#define PICID_MAINMENU			3
#define PICID_LANGUAGEMENU		5
#define PICID_WORK_PREPARE		7
#define PICID_WORK_ERROR1		8
#define PICID_WORK_IDLE			10
#define PICID_WORK_READY		12
#define PICID_WORK_POWERON		14
#define PICID_WORK_STARTED		16
#define PICID_WORK_NUMPAD		18
#define PICID_WORK_PHOTOTYPE	20
#define PICID_WORKOnLaserOff_	22
#define PICID_WORKOnLaserOff	23
#define PICID_WORKOnLaserOn		24
#define PICID_WORKOnReady		25
#define PICID_DATABASE			26
#define PICID_DATABASEOnRead	28
#define PICID_NEWPROFILE		30
#define PICID_PROFILEKBRD1		32
#define PICID_PROFILEKBRD2		33
#define PICID_PROFILEKBRD3		34
#define PICID_PROFILEKBRD4		35
#define PICID_DATABASEOnSave	27

// Data structures
#define STRUCT_ADDR_DATA		0x0001

typedef struct DGUS_DATA_STRUCT
{
	uint16_t Frequency;
	uint16_t Duration;
	uint16_t Intensity;
	uint16_t Power;
	uint16_t Energy;
	uint16_t DutyCycle;
	uint16_t DatabaseSelectedProfile;
} DGUS_DATA, *PDGUS_DATA;

typedef struct DGUS_READDATA_STRUCT
{
	uint16_t Frequency;
	uint16_t Duration;
	uint16_t Intensity;
} DGUS_READDATA, *PDGUS_READDATA;

#define STRUCT_ADDR_WRITEDATA	0x0004

typedef struct DGUS_WRITEDATA_STRUCT
{
	uint16_t Power;
	uint16_t Energy;
	uint16_t DutyCycle;
} DGUS_WRITEDATA, *PDGUS_WRITEDATA;

void ConvertData(void* dst, void* src, uint16_t size, uint16_t offset = 0);
uint16_t min(uint16_t x, uint16_t y);
uint16_t max(uint16_t x, uint16_t y);
uint16_t swap(uint16_t data);

#endif /* DGUSGUI_H_ */