/*
 * ATxmegaLaserDiode.cpp
 *
 * Created: 01.04.2016 20:57:39
 * Author : Vladislav
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "CDGUSUSART.h"
#include "CLaserBoard.h"
#include "LaserLib/CUSART.h"
#include "LaserLib/sys_interrupts.h"

#include <math.h>
#include <util/delay.h>

/* Global variables */
CDGUSUSART usart;
CLaserBoard laserBoard;
	 
int main(void)
{
	cli();	/* Disable global interrupts */
	
	laserBoard.InitializeIO();
	laserBoard.InitializeClock();
	usart.Initialize(BAUD_115200_ERM0P1, PARITY_DISABLE, STOPBITS_1BIT, true);
	
	sei();	/* Enable global interrupts */
	
    /* Replace with your application code */
    while (1) 
    {
		_delay_ms(1);
    }
}

