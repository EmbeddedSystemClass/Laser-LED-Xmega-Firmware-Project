/*
 * DGUSGUI.h
 *
 * Created: 11.04.2016 15:07:36
 *  Author: TPU_2
 */ 


#ifndef DGUSGUI_H_
#define DGUSGUI_H_

#include <stdint.h>

// Electroporation GUI
#define REGISTER_ADDR_PICID		0x03
#define VARIABLE_ADDR_MIN		0x0000
#define VARIABLE_ADDR_SEC		0x0001
#define VARIABLE_ADDR_MSC		0x0002
#define VARIABLE_ADDR_PWR		0x0003
#define VARIABLE_ADDR_BAR1		0x0004
#define VARIABLE_ADDR_BAR2		0x0005
#define VARIABLE_ADDR_BAR3		0x0006

// DGUS data struct
#define DGUS_DATA_ADDR			0x0000

typedef struct DGUS_DATA_STRUCT
{
	uint16_t min;
	uint16_t sec;
	uint16_t msc;
	uint16_t Pwr;
	uint16_t Bar1;
	uint16_t Bar2;
	uint16_t Bar3;
} DGUS_DATA, *PDGUS_DATA;

#define PICID_LOGO				0x0000
#define PICID_SETUP				0x0001
#define PICID_OnStart			0x0003
#define PICID_OnH_L				0x0004
#define PICID_OnAnim1			0x0005
#define PICID_OnAnim2			0x0006
#define PICID_OnAnim3			0x0007
#define PICID_OnAnim4			0x0008
#define PICID_TIMER				0x0009
#define PICID_OnRestart			0x000a
#define PICID_OnStop			0x000b

#endif /* DGUSGUI_H_ */