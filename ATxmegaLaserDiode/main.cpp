/*
 * ATxmegaLaserDiode.cpp
 *
 * Created: 01.04.2016 20:57:39
 * Author : Vladislav
 */ 

#define F_CPU 32000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <util/delay.h>

#define LEDS_NUM 8

uint16_t sine_wave[256] = 
	{
	8000	,		11061	,		13657	,		15391	,		16000	,		15391	,		13657	,		11061	,
	8000	,		4939	,		2343	,		609	,		0	,		609	,		2343	,		4939	,		8000	,
	11061	,	13657	,	15391	,	16000	,	15391	,	13657	,	11061	,	8000	,	4939	,	2343	,
	609	,	0	,	609	,	2343	,	4939	,	8000	,	11061	,	13657	,	15391	,	16000	,	15391	,
	13657	,	11061	,	8000	,	4939	,	2343	,	609	,	0	,	609	,	2343	,	4939	,	8000	,
	11061	,	13657	,	15391	,	16000	,	15391	,	13657	,	11061	,	8000	,	4939	,	2343	,
	609	,	0	,	609	,	2343	,	4939	,	8000	,	11061	,	13657	,	15391	,	16000	,	15391	,
	13657	,	11061	,	8000	,	4939	,	2343	,	609	,	0	,	609	,	2343	,	4939	,	8000	,
	11061	,	13657	,	15391	,	16000	,	15391	,	13657	,	11061	,	8000	,	4939	,	2343	,
	609	,	0	,	609	,	2343	,	4939	,	8000	,	11061	,	13657	,	15391	,	16000	,	15391	,
	13657	,	11061	,	8000	,	4939	,	2343	,	609	,	0	,	609	,	2343	,	4939	,	8000	,
	11061	,	13657	,	15391	,	16000	,	15391	,	13657	,	11061	,	8000	,	4939	,	2343	,
	609	,	0	,	609	,	2343	,	4939	,	8000	,	11061	,	13657	,	15391	,	16000	,	15391	,
	13657	,	11061	,	8000	,	4939	,	2343	,	609	,	0	,	609	,	2343	,	4939	,	8000	,
	11061	,	13657	,	15391	,	16000	,	15391	,	13657	,	11061	,	8000	,	4939	,	2343	,
	609	,	0	,	609	,	2343	,	4939	,	8000	,	11061	,	13657	,	15391	,	16000	,	15391	,
	13657	,	11061	,	8000	,	4939	,	2343	,	609	,	0	,	609	,	2343	,	4939	,	8000	,
	11061	,	13657	,	15391	,	16000	,	15391	,	13657	,	11061	,	8000	,	4939	,	2343	,
	609	,	0	,	609	,	2343	,	4939	,	8000	,	11061	,	13657	,	15391	,	16000	,	15391	,
	13657	,	11061	,	8000	,	4939	,	2343	,	609	,	0	,	609	,	2343	,	4939	,	8000	,
	11061	,	13657	,	15391	,	16000	,	15391	,	13657	,	11061	,8000	,	4939	,	2343	,	609	,
	0	,	609	,	2343	,	4939	,	8000	,	11061	,	13657	,	15391	,	16000	,	15391	,	13657	,
	11061	,	8000	,	4939	,	2343	,	609	,0	,	609	,	2343	,	4939	,	8000	,	11061	,	
	13657	,	15391	,	16000	,	15391	,	13657	,	11061	,	8000	,	4939	,	2343	,	609	,	0	,
	609	,	2343	,	4939};
	 
uint8_t leds[LEDS_NUM] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

ISR(TCC0_CCA_vect)
{
	PORTC.OUTCLR = 0xff;
}
	 
ISR(TCC0_OVF_vect)
{
	PORTC.OUTSET = 0xff;
}

ISR(TCE0_CCD_vect)
{
}

ISR(TCE0_OVF_vect)
{
}
	 
void InitializeIO()
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
}

void InitializeClockSystem()
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

void InitializeTIME()
{
	// Start timer 4MHz tick rate
	TCE0.CTRLA = TC_CLKSEL_DIV1_gc;
	
	// Enable output compare channel D, PWM with single slope waveform generation mode
	TCE0.CTRLB = TC0_CCDEN_bm | TC_WGMODE_SS_gc;
	
	// No events
	TCE0.CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;
	
	// Set period for 62.500 KHz PWM
	TCE0.PER = 16000; //512;
	
	// Set 50% duty cycle
	TCE0.CCD =  8000;
	
	// Enable interrupt
	TCE0.INTCTRLA = TC_OVFINTLVL_LO_gc;
	TCE0.INTCTRLB = TC_CCDINTLVL_LO_gc;
}

void InitializeTIMC()
{
	// Start timer 32MHz tick rate
	TCC0.CTRLA = TC_CLKSEL_DIV1024_gc;
	
	// Enable output compare channel D, PWM with single slope waveform generation mode
	TCC0.CTRLB = TC_WGMODE_NORMAL_gc;
	
	// No events
	TCC0.CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;
	
	// Set period for 1 Hz PWM
	TCC0.PER = 15625/15;
	
	// Enable interrupt
	TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc;
}

void InitializeTIMC_PWM()
{
	// Start timer 32MHz tick rate
	TCC0.CTRLA = TC_CLKSEL_DIV2_gc;
	
	// Enable output compare channel D, PWM with single slope waveform generation mode
	TCC0.CTRLB = /*TC0_CCAEN_bm |*/ TC_WGMODE_SS_gc;
	
	// No events
	TCC0.CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;
	
	// Set period for 1 Hz PWM
	TCC0.PER = 4096;
	
	TCC0.CCA = 0;
	
	// Enable interrupt
	TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc;
	TCC0.INTCTRLB = TC_CCAINTLVL_LO_gc;
}

void initDMA_MEMtoTCE()
{
	/*// Channel 1 reset
	DMA.CH0.CTRLA = DMA_CH_RESET_bm;*/
	
	// Channel 1 burst len - 2 byte, repeat mode
	DMA.CH0.CTRLA = DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm | DMA_CH_BURSTLEN_2BYTE_gc;
	
	// Source address increment not reload, destination address fixed not reloaded
	DMA.CH0.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_SRCRELOAD_BLOCK_gc | DMA_CH_DESTDIR_INC_gc | DMA_CH_DESTRELOAD_BURST_gc;
	
	// Set trigger source to TCE0
	DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_TCE0_CCD_gc;
	
	// Set destination address
	DMA.CH0.DESTADDR0 = (int)&TCE0.CCDBUF;
	DMA.CH0.DESTADDR1 = (int)&TCE0.CCDBUF >> 8;
	DMA.CH0.DESTADDR2 = 0;
	
	// Set source address
	DMA.CH0.SRCADDR0 = (int)&sine_wave;
	DMA.CH0.SRCADDR1 = (int)&sine_wave >> 8;
	DMA.CH0.SRCADDR2 = 0;
	
	// Set transfer count
	DMA.CH0.TRFCNT = 512;
	
	// Set infinite repeat
	DMA.CH2.REPCNT = 0;
	
	// Channel 1 enable
	DMA.CH0.CTRLA |= DMA_CH_ENABLE_bm;
}

void initDMA_MEMtoLED()
{
	/*// Channel 1 reset
	DMA.CH1.CTRLA = DMA_CH_RESET_bm;*/
	
	// Channel 1 burst len - 2 byte, repeat mode
	DMA.CH1.CTRLA = DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm | DMA_CH_BURSTLEN_1BYTE_gc;
	
	// Source address increment not reload, destination address fixed not reloaded
	DMA.CH1.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_SRCRELOAD_BLOCK_gc | DMA_CH_DESTDIR_FIXED_gc | DMA_CH_DESTRELOAD_BLOCK_gc;
	
	// Set trigger source to TCE0
	DMA.CH1.TRIGSRC = DMA_CH_TRIGSRC_TCC0_OVF_gc;
	
	// Set destination address
	DMA.CH1.DESTADDR0 = (int)&PORTC.OUT;
	DMA.CH1.DESTADDR1 = (int)&PORTC.OUT >> 8;
	DMA.CH1.DESTADDR2 = 0;
	
	// Set source address
	DMA.CH1.SRCADDR0 = (int)&leds;
	DMA.CH1.SRCADDR1 = (int)&leds >> 8;
	DMA.CH1.SRCADDR2 = 0;
	
	// Set transfer count
	DMA.CH1.TRFCNT = LEDS_NUM;
	
	// Set infinite repeat
	DMA.CH1.REPCNT = 0;
	
	// Channel 1 enable
	DMA.CH1.CTRLA |= DMA_CH_ENABLE_bm;
}

void InitializeDMA()
{
	/*// Reset DMA
	DMA.CTRL = DMA_RESET_bm;*/
	
	// Enable DMA
	DMA.CTRL = DMA_ENABLE_bm | DMA_DBUFMODE_DISABLED_gc | DMA_PRIMODE_CH0123_gc;
	
	initDMA_MEMtoTCE();
	//initDMA_MEMtoLED();
}

int main(void)
{
	cli();
	
	InitializeClockSystem();
	InitializeIO();
	InitializeTIMC_PWM();
	//InitializeDMA();
	//InitializeTIME();
	
	// Enable low level interrupts
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
	PMIC.CTRL |= PMIC_HILVLEN_bm;
	sei();
	
	float x = 0.0f;
	float A = 0.0f;
	
    /* Replace with your application code */
    while (1) 
    {
		x += 0.001f;
		A = 0.5f * sinf(x) + 0.5f;
		/*if (A > 1.0f) A = 1.0f;
		if (A < 0.0f) A = 0.0f;*/
		
		uint16_t data = (uint16_t)(3000.0f * A); 
		
		TCC0.CCABUF = data;
		
		_delay_ms(1);
    }
}

