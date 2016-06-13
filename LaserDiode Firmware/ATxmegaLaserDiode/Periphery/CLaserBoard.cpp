/* 
* CLaserBoard.cpp
*
* Created: 09.04.2016 15:23:56
* Author: Vladislav
*/


#include "CLaserBoard.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

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
	PORTC.DIRSET = 0x00; //0xFF;
	
	// Configure all pins to "wired and"
	PORTC.PIN0CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTC.PIN1CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN2CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN3CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN4CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN5CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN6CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	PORTC.PIN7CTRL = PORT_OPC_WIREDAND_gc | PORT_SRLEN_bm;
	
	// Set all down
	PORTC.OUT = 0xFF;
	
	// Buzzer pin initialize
	PORTE.DIRSET = PIN3_bm;
	
	// Configure
	PORTE.PIN3CTRL = PORT_OPC_TOTEM_gc;
	PORTE.OUT = 0;
	
	// Configure all pins of PWM to inverted
	PORTF.PIN0CTRL = PORT_INVEN_bm | PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;
	PORTF.PIN1CTRL = /*PORT_INVEN_bm |*/ PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;
	PORTF.PIN2CTRL = PORT_INVEN_bm | PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;
	PORTF.PIN3CTRL = PORT_INVEN_bm | PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;
	PORTF.PIN4CTRL = PORT_INVEN_bm | PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;
	PORTF.PIN5CTRL = PORT_INVEN_bm | PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;
	PORTF.PIN6CTRL = PORT_INVEN_bm | PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;
	PORTF.PIN7CTRL = PORT_INVEN_bm | PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;

	// Set all down
	PORTF.OUT = 0x00;

	// Set all output
	PORTF.DIRSET = 0xFF;
	
	// Set PD0 and PD4
	PORTD.DIRSET = PIN0_bm | PIN4_bm;
	
	// Configure all pins to "wired and"
	PORTD.PIN0CTRL = PORT_INVEN_bm | PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;
	PORTD.PIN4CTRL = PORT_INVEN_bm | PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;
	
	PORTD.OUT = 0;//PIN0_bm;
	
	PORTE.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm;
	
	// Configure all pins to "wired and"
	PORTE.PIN0CTRL = PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;
	PORTE.PIN1CTRL = PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;
	PORTE.PIN2CTRL = PORT_OPC_TOTEM_gc | PORT_SRLEN_bm;
	
	PORTE.OUT = 0;//PIN0_bm;
	
	// Enable low level interrupts
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
	PMIC.CTRL |= PMIC_HILVLEN_bm;
	
	for (uint8_t i = 0; i < 8; i++)
		PIN_Cnt[i] = 1;
		
	PINThreshold = 3;
	
	PORTC.INT0MASK = PIN0_bm;
	PORTC.INTCTRL = PORT_INT0LVL_LO_gc;
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
	unsigned char n = (OSC.PLLCTRL & (~(OSC_PLLSRC_gm | OSC_PLLFAC_gm))) |    OSC_PLLSRC_XOSC_gc | OSC_PLLFAC2_bm;
	CCP = CCP_IOREG_gc;
	OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | OSC_PLLFAC2_bm;//n;
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
	CLK.PSCTRL = CLK_PSADIV_1_gc | CLK_PSBCDIV_1_1_gc;//n;
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
	CLK.LOCK = CLK_LOCK_bm;//n;
	// Peripheral Clock output: Disabled
	PORTCFG.CLKEVOUT = (PORTCFG.CLKEVOUT & (~PORTCFG_CLKOUT_gm)) | PORTCFG_CLKOUT_OFF_gc;
	//-----------------------
}

void CLaserBoard::PWMOn()
{
	PORTE.OUTSET = PIN2_bm;
}

void CLaserBoard::PWMOff()
{
	PORTE.OUTCLR = PIN2_bm;
}

void CLaserBoard::Relay1On()
{
	PORTD.DIRCLR = PIN0_bm;
}

void CLaserBoard::Relay2On()
{
	PORTD.DIRCLR = PIN4_bm;
}

void CLaserBoard::Relay1Off()
{
	PORTD.DIRSET = PIN0_bm;
}

void CLaserBoard::Relay2Off()
{
	PORTD.DIRSET = PIN4_bm;
}

void CLaserBoard::LaserPowerOn()
{
	PORTE.OUTSET = PIN1_bm;
}

void CLaserBoard::LaserPowerOff()
{
	PORTE.OUTCLR = PIN1_bm;
}

void CLaserBoard::Beep()
{
	cli();
	
	for (int i = 0; i < 100; i++)
	{
		PORTE.OUTSET = PIN3_bm;
		_delay_us(500);
		PORTE.OUTCLR = PIN3_bm;
		_delay_us(500);
	}
	
	sei();
}

void CLaserBoard::BeepClassError()
{
	cli();
	
	for (int j = 0; j < 5; j++)
	{
		for (int i = 0; i < 100; i++)
		{
			PORTE.OUTSET = PIN3_bm;
			_delay_us(500);
			PORTE.OUTCLR = PIN3_bm;
			_delay_us(500);
		}
		_delay_ms(100);
	}
	
	sei();
}

void CLaserBoard::PortCheck()
{
	/*uint8_t inport = PORTC.IN;
	uint8_t bitpos = 1;
	for (uint8_t i = 0; i < 8; i++)
	{
		if (inport & bitpos)
		{
			PIN_Cnt[i]++;
			if (PIN_Cnt[i] > 4)
				PIN_Cnt[i] = 4;
		}
		else
		{
			PIN_Cnt[i]--;
			if (PIN_Cnt[i] < 1)
				PIN_Cnt[i] = 1;
		}
		
		if (PIN_Cnt[i] > PINThreshold)
			Port |= bitpos;
		else
			Port &= ~bitpos;
		
		bitpos <<= 1;
	}*/
	Port = PORTC.IN;
}

bool CLaserBoard::Footswitch()
{
	return Port & 0x01;
}