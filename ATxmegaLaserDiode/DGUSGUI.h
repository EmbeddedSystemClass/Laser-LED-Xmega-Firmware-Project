/*
 * DGUSGUI.h
 *
 * Created: 11.04.2016 15:07:36
 *  Author: TPU_2
 */ 


#ifndef DGUSGUI_H_
#define DGUSGUI_H_

/*
// Variables & registers addresses
#define REGISTER_ADDR_PICID		0x03
#define VARIABLE_ADDR_ENRG		0x0000
#define VARIABLE_ADDR_FREQ		0x0100
#define VARIABLE_ADDR_WATER		0x0300
#define VARIABLE_ADDR_LAMP		0x0400
#define VARIABLE_ADDR_LED		0x0500


// Picture ids
#define PICID_FASTMENU			0x0001
#define PICID_MEDIUMMENU		0x0002
#define PICID_SLOWMENU			0x0003
#define PICID_READYMENU			0x0005*/

// Electroporation GUI
#define REGISTER_ADDR_PICID		0x03
#define VARIABLE_ADDR_MIN		0x0000
#define VARIABLE_ADDR_SEC		0x0010
#define VARIABLE_ADDR_MSC		0x0020
#define VARIABLE_ADDR_PWR		0x0030
#define VARIABLE_ADDR_BAR1		0x0040
#define VARIABLE_ADDR_BAR2		0x0050
#define VARIABLE_ADDR_BAR3		0x0060

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