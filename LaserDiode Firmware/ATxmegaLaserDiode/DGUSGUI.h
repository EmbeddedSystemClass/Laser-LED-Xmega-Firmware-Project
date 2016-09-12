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

#define VARIABLE_ADDR_MODE		0x0001
#define VARIABLE_ADDR_FREQ		0x0002
#define VARIABLE_ADDR_DURATION	0x0003
#define VARIABLE_ADDR_ENERGYPCT	0x0004
#define VARIABLE_ADDR_POWER		0x0005
#define VARIABLE_ADDR_ENERGY	0x0006 // Duration
#define VARIABLE_ADDR_DUTYCYCLE	0x0007 // Energy
#define VARIABLE_ADDR_LASERCNT	0x0008
#define VARIABLE_ADDR_MELANIN	0x000a
#define VARIABLE_ADDR_PHOTOTYPE	0x000b
#define VARIABLE_ADDR_TEMPER	0x000c
#define VARIABLE_ADDR_COOLING	0x000d
#define VARIABLE_ADDR_FLOW		0x000e
#define VARIABLE_ADDR_TIMMIN	0x000f
#define VARIABLE_ADDR_TIMSEC	0x0010
#define VARIABLE_ADDR_DATAOFFS	0x0011
#define VARIABLE_ADDR_DATAINDEX	0x0012
#define VARIABLE_ADDR_COOLICON	0x0013
#define VARIABLE_ADDR_SESSNCNT	0x0014

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
#define PICID_DATABASEOnSave	37
#define PICID_WORK_TEMPERWAIT	39
#define PICID_WORK_ERRORFLOW	40
#define PICID_WORK_ERRORHEATING	41
#define PICID_WORK_PHOTOTYPESEL	47

// Data structures
#define STRUCT_ADDR_LASERDIODE_DATA			0x0001
#define STRUCT_ADDR_LASERPROFILE_DATA		0x0002
#define STRUCT_ADDR_LASERPROSETTINGS_DATA	0x0005
#define STRUCT_ADDR_PREPARETIMER_DATA		0x000f

// Database
#define VARIABLE_ADDR_DATABASE	0x0100
#define VARIABLE_ADDR_PROFILE	0x0D00
#define PROFILE_SIZE			0x0100
#define DGUS_DATABASE_ADDR		0x00F00000

typedef struct DGUS_PREPARETIMER_STRUCT
{
	uint16_t timer_minutes;
	uint16_t timer_seconds;
} DGUS_PREPARETIMER, *PDGUS_PREPARETIMER;

typedef struct DGUS_LASERPROFILE_STRUCT
{
	// Basic laser settings
	uint16_t Frequency;		// Frequency of laser pulses
	uint16_t Duration;		// Duration of laser pulse
	uint16_t EnergyPercent; // Energy in percentage of one pulse
} DGUS_LASERPROFILE, *PDGUS_LASERPROFILE;

typedef struct DGUS_LASERSETTINGS_STRUCT
{
	// Service settings
	uint16_t Power;			// Power of laser light
	uint16_t Energy;		// Energy in J
	uint16_t DutyCycle;		// Duty cycle	
} DGUS_LASERSETTINGS, *PDGUS_LASERSETTINGS;

typedef struct DGUS_LASERDIODE_STRUCT
{
	// Laser mode
	uint16_t mode;
	
	// Basic laser settings
	DGUS_LASERPROFILE laserprofile;
	
	// Service settings
	DGUS_LASERSETTINGS lasersettings;
	
	// Pulse counter
	uint32_t PulseCounter;
	
	// Phototype
	uint16_t melanin;
	uint16_t phototype;
	
	// Cooling settings
	uint16_t temperature;
	uint16_t cooling;
	uint16_t flow;
	
	// Timer
	DGUS_PREPARETIMER timer;
	
	// Database variables
	uint16_t DatabasePageOffset;
	uint16_t DatabaseSelectionIndex;
	
	// Icons
	uint16_t coolIcon;
	
	// Pulse counter
	uint32_t SessionPulseCounter;
} DGUS_LASERDIODE, *PDGUS_LASERDIODE;

typedef struct DGUS_PROFILE_STRUCT
{
	uint16_t ID;
	char Name[32];
	char Time[32];
	uint16_t Power;
} DGUS_PROFILE, *PDGUS_PROFILE;

void ConvertData(void* dst, void* src, uint16_t size, uint16_t offset = 0);
uint16_t min(uint16_t x, uint16_t y);
uint16_t max(uint16_t x, uint16_t y);
uint16_t swap(uint16_t data);
uint32_t swap32(uint32_t data);

#endif /* DGUSGUI_H_ */