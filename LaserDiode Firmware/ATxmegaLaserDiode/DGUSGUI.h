/*
 * DGUSGUI.h
 *
 * Created: 11.04.2016 15:07:36
 *  Author: Vladislav Alex. O.
 */ 


#ifndef DGUSGUI_H_
#define DGUSGUI_H_

// Variables & registers addresses
#define REGISTER_ADDR_PICID		0x03
#define VARIABLE_ADDR_FREQ		0x0001
#define VARIABLE_ADDR_DURATION	0x0002
#define VARIABLE_ADDR_INTENSITY	0x0003
#define VARIABLE_ADDR_POWER		0x0004
#define VARIABLE_ADDR_ENERGY	0x0005
#define VARIABLE_ADDR_DUTYCYCLE	0x0006

// Picture ids
#define PICID_LOGO				0x0000
#define PICID_WORKFAST			0x0001
#define PICID_WORKMEDIUM		0x0002
#define PICID_WORKSLOW			0x0003

#define PICID_WORKOnReady		0x0007
#define PICID_WORKOnStart		0x0008
#define PICID_WORKOnStop		0x0009

#define PICID_WORKSTART			0x000a
#define PICID_WORKSTARTED		0x000b

#define PICID_PHOTOTYPESELECT	0x000c
#define PICID_PHOTOTYPE1		0x000e
#define PICID_PHOTOTYPE2		0x000f
#define PICID_PHOTOTYPE3		0x0010
#define PICID_PHOTOTYPE4		0x0011
#define PICID_PHOTOTYPE5		0x0012
#define PICID_PHOTOTYPE6		0x0013

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
	//uint16_t update;
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

#endif /* DGUSGUI_H_ */