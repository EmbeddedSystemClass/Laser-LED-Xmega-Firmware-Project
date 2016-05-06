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

#define PICID_DATABASE_MIN		0x0018
#define PICID_DATABASE_MAX		0x0029

#define PICID_MAINMENU			0x0014

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

#define STRUCT_ADDR_LINESDATA1	0x0010

typedef struct DGUS_LINESDATA1_STRUCT
{
	char line1 [32];
	char line2 [32];
	char line3 [32];
	char line4 [32];
	char line5 [32];
	char line6 [32];
	char line7 [32];
} DGUS_LINESDATA1, *PDGUS_LINESDATA1;

#define STRUCT_ADDR_LINESDATA2	0x0080

typedef struct DGUS_LINESDATA2_STRUCT
{
	char line8 [32];
	char line9 [32];
	char line10[32];
	char line11[32];
	char line12[32];
	char line13[32];
	char line14[32];
} DGUS_LINESDATA2, *PDGUS_LINESDATA2;

#define STRUCT_ADDR_VALUESDATA1	0x0310
#define STRUCT_ADDR_VALUESDATA2	0x0380

typedef struct DGUS_VALUESDATA_STRUCT
{
	char value1[32];
	char value2[32];
	char value3[32];
	char value4[32];
	char value5[32];
	char value6[32];
	char value7[32];
} DGUS_VALUESDATA, *PDGUS_VALUESDATA;

#endif /* DGUSGUI_H_ */