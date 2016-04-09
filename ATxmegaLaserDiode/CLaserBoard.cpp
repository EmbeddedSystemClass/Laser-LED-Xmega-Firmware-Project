/* 
* CLaserBoard.cpp
*
* Created: 09.04.2016 15:23:56
* Author: Vladislav
*/


#include "CLaserBoard.h"
#include <avr/io.h>

// default constructor
CLaserBoard::CLaserBoard()
{
} //CLaserBoard

// default destructor
CLaserBoard::~CLaserBoard()
{
} //~CLaserBoard

void CLaserBoard::InitializeIO()
{
	// Set all output
	PORTC.DIRSET = 0xFF;
	
	// Configure all pins to "wired and"
	PORTC.PIN0CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN1CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN2CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN3CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN4CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN5CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN6CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN7CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	
	// Set all down
	PORTC.OUT = 0x00;
	
	// Buzzer pin initialize
	PORTE.DIRSET = PIN3_bm;
	
	// Configure
	PORTE.PIN3CTRL = PORT_OPC_TOTEM_gc;
	PORTE.OUT = 0;
	
	// Enable low level interrupts
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
	PMIC.CTRL |= PMIC_HILVLEN_bm;
}

void CLaserBoard::InitializeClock()
{
		//-----------------------
		// External 16000.000 kHz oscillator initialization
		OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;
		// Enable the external oscillator
		OSC.CTRL |= OSC_XOSCEN_bm;
		// Wait for the external oscillator to stabilize
		while ((OSC.STATUS & OSC_XOSCRDY_bm) == 0);
		// PLL initialization
		// PLL clock source: External Osc. or Clock
		// PLL multiplication factor: 2
		// PLL frequency: 32.000000 MHz
		// Set the PLL clock source and multiplication factor
		unsigned char n = (OSC.PLLCTRL & (~(OSC_PLLSRC_gm | OSC_PLLFAC_gm))) |    OSC_PLLSRC_XOSC_gc | 2;
		CCP = CCP_IOREG_gc;
		OSC.PLLCTRL = n;
		// Enable the PLL
		OSC.CTRL |= OSC_PLLEN_bm;
		// System Clock prescaler A division factor: 1
		// System Clock prescalers B & C division factors: B:1, C:1
		// ClkPer4: 32000.000 kHz
		// ClkPer2: 32000.000 kHz
		// ClkPer:  32000.000 kHz
		// ClkCPU:  32000.000 kHz
		n = (CLK.PSCTRL & (~(CLK_PSADIV_gm | CLK_PSBCDIV1_bm | CLK_PSBCDIV0_bm))) |
		CLK_PSADIV_1_gc | CLK_PSBCDIV_1_1_gc;
		CCP = CCP_IOREG_gc;
		CLK.PSCTRL = n;
		// Wait for the PLL to stabilize
		while ((OSC.STATUS & OSC_PLLRDY_bm)==0);
		// Select the system clock source: Phase Locked Loop
		n = (CLK.CTRL & (~CLK_SCLKSEL_gm)) | CLK_SCLKSEL_PLL_gc;
		CCP = CCP_IOREG_gc;
		CLK.CTRL = n;
		// Disable the unused oscillators: 2 MHz, 32 MHz, internal 32 kHz
		OSC.CTRL &= ~(OSC_RC2MEN_bm | OSC_RC32MEN_bm | OSC_RC32KEN_bm);
		// Lock the CLK.CTRL and CLK.PSCTRL registers
		n = CLK.LOCK | CLK_LOCK_bm;
		CCP = CCP_IOREG_gc;
		CLK.LOCK = n;
		// Peripheral Clock output: Disabled
		PORTCFG.CLKEVOUT = (PORTCFG.CLKEVOUT & (~PORTCFG_CLKOUT_gm)) | PORTCFG_CLKOUT_OFF_gc;
		//-----------------------
}
