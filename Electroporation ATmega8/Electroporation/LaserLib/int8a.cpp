/*
 * int128a.cpp
 *
 * Created: 04.03.2016 15:02:29
 *  Author: Vladislav
 */ 

#ifndef INT128A_CPP_
#define INT128A_CPP_

#include "int8a.h"
#include <avr/io.h>
#include <stdlib.h>

#if defined (__AVR_ATmega8__)

void* InterruptSenderTable[NUM_INERRUPTS+1];
ISRCallback InterruptFuncTable[NUM_INERRUPTS+1];

/* External Interrupt Request 0 */
ISR(INT0_vect)
{
	if (InterruptFuncTable[INT0_vect_num] != NULL)
		(*(InterruptFuncTable[INT0_vect_num]))(InterruptSenderTable[INT0_vect_num]);
}

/* External Interrupt Request 1 */
ISR(INT1_vect)
{
	if (InterruptFuncTable[INT1_vect_num] != NULL)
		(*(InterruptFuncTable[INT1_vect_num]))(InterruptSenderTable[INT1_vect_num]);
}

/* Timer/Counter2 Compare Match */
ISR(TIMER2_COMP_vect)
{
	if (InterruptFuncTable[TIMER2_COMP_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER2_COMP_vect_num]))(InterruptSenderTable[TIMER2_COMP_vect_num]);
}

/* Timer/Counter2 Overflow */
ISR(TIMER2_OVF_vect)
{
	if (InterruptFuncTable[TIMER2_OVF_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER2_OVF_vect_num]))(InterruptSenderTable[TIMER2_OVF_vect_num]);
}

/* Timer/Counter1 Capture Event */
ISR(TIMER1_CAPT_vect)
{
	if (InterruptFuncTable[TIMER1_CAPT_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER1_CAPT_vect_num]))(InterruptSenderTable[TIMER1_CAPT_vect_num]);
}

/* Timer/Counter1 Compare Match A */
ISR(TIMER1_COMPA_vect)
{
	if (InterruptFuncTable[TIMER1_COMPA_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER1_COMPA_vect_num]))(InterruptSenderTable[TIMER1_COMPA_vect_num]);
}

/* Timer/Counter Compare Match B */
ISR(TIMER1_COMPB_vect)
{
	if (InterruptFuncTable[TIMER1_COMPB_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER1_COMPB_vect_num]))(InterruptSenderTable[TIMER1_COMPB_vect_num]);
}

/* Timer/Counter1 Overflow */
ISR(TIMER1_OVF_vect)
{
	if (InterruptFuncTable[TIMER1_OVF_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER1_OVF_vect_num]))(InterruptSenderTable[TIMER1_OVF_vect_num]);
}

/* Timer/Counter0 Overflow */
ISR(TIMER0_OVF_vect)
{
	if (InterruptFuncTable[TIMER0_OVF_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER0_OVF_vect_num]))(InterruptSenderTable[TIMER0_OVF_vect_num]);
}

/* SPI Serial Transfer Complete */
ISR(SPI_STC_vect)
{
	if (InterruptFuncTable[SPI_STC_vect_num] != NULL)
		(*(InterruptFuncTable[SPI_STC_vect_num]))(InterruptSenderTable[SPI_STC_vect_num]);
}

/* USART, Rx Complete */
ISR(USART_RXC_vect)
{
	if (InterruptFuncTable[USART_RXC_vect_num] != NULL)
		(*(InterruptFuncTable[USART_RXC_vect_num]))(InterruptSenderTable[USART_RXC_vect_num]);
}

/* USART Data Register Empty */
ISR(USART_UDRE_vect)
{
	if (InterruptFuncTable[USART_UDRE_vect_num] != NULL)
		(*(InterruptFuncTable[USART_UDRE_vect_num]))(InterruptSenderTable[USART_UDRE_vect_num]);
}

/* USART, Tx Complete */
ISR(USART_TXC_vect)
{
	if (InterruptFuncTable[USART_TXC_vect_num] != NULL)
		(*(InterruptFuncTable[USART_TXC_vect_num]))(InterruptSenderTable[USART_TXC_vect_num]);
}

/* ADC Conversion Complete */
ISR(ADC_vect)
{
	if (InterruptFuncTable[ADC_vect_num] != NULL)
		(*(InterruptFuncTable[ADC_vect_num]))(InterruptSenderTable[ADC_vect_num]);
}

/* EEPROM Ready */
ISR(EE_RDY_vect)
{
	if (InterruptFuncTable[EE_RDY_vect_num] != NULL)
		(*(InterruptFuncTable[EE_RDY_vect_num]))(InterruptSenderTable[EE_RDY_vect_num]);
}

/* Analog Comparator */
ISR(ANA_COMP_vect)
{
	if (InterruptFuncTable[ANA_COMP_vect_num] != NULL)
		(*(InterruptFuncTable[ANA_COMP_vect_num]))(InterruptSenderTable[ANA_COMP_vect_num]);
}

/* 2-wire Serial Interface */
ISR(TWI_vect)
{
	if (InterruptFuncTable[TWI_vect_num] != NULL)
		(*(InterruptFuncTable[TWI_vect_num]))(InterruptSenderTable[TWI_vect_num]);
}

/* Store Program Memory Ready */
ISR(SPM_RDY_vect)
{
	if (InterruptFuncTable[SPM_RDY_vect_num] != NULL)
		(*(InterruptFuncTable[SPM_RDY_vect_num]))(InterruptSenderTable[SPM_RDY_vect_num]);
}

#endif
#endif
