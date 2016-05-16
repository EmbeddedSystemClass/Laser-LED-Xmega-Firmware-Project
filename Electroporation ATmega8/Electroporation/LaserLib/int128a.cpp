/*
 * int128a.cpp
 *
 * Created: 04.03.2016 15:02:29
 *  Author: Vladislav
 */ 

#ifndef INT128A_CPP_
#define INT128A_CPP_

#include "int128a.h"
#include <stdlib.h>
#include <avr/io.h>

#if defined (__AVR_ATmega128__)

void* InterruptSenderTable[35];
ISRCallback InterruptFuncTable[35];

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

/* External Interrupt Request 2 */
ISR(INT2_vect)
{
	if (InterruptFuncTable[INT2_vect_num] != NULL)
		(*(InterruptFuncTable[INT2_vect_num]))(InterruptSenderTable[INT2_vect_num]);
}

/* External Interrupt Request 3 */
ISR(INT3_vect)
{
	if (InterruptFuncTable[INT3_vect_num] != NULL)
		(*(InterruptFuncTable[INT3_vect_num]))(InterruptSenderTable[INT3_vect_num]);
}

/* External Interrupt Request 4 */
ISR(INT4_vect)
{
	if (InterruptFuncTable[INT4_vect_num] != NULL)
		(*(InterruptFuncTable[INT4_vect_num]))(InterruptSenderTable[INT4_vect_num]);
}

/* External Interrupt Request 5 */
ISR(INT5_vect)
{
	if (InterruptFuncTable[INT5_vect_num] != NULL)
		(*(InterruptFuncTable[INT5_vect_num]))(InterruptSenderTable[INT5_vect_num]);
}

/* External Interrupt Request 6 */
ISR(INT6_vect)
{
	if (InterruptFuncTable[INT6_vect_num] != NULL)
		(*(InterruptFuncTable[INT6_vect_num]))(InterruptSenderTable[INT6_vect_num]);
}

/* External Interrupt Request 7 */
ISR(INT7_vect)
{
	if (InterruptFuncTable[INT7_vect_num] != NULL)
		(*(InterruptFuncTable[INT7_vect_num]))(InterruptSenderTable[INT7_vect_num]);
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

/* Timer/Counter0 Compare Match */
ISR(TIMER0_COMP_vect)
{
	if (InterruptFuncTable[TIMER0_COMP_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER0_COMP_vect_num]))(InterruptSenderTable[TIMER0_COMP_vect_num]);
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

/* USART0, Rx Complete */
ISR(USART0_RX_vect)
{
	if (InterruptFuncTable[USART0_RX_vect_num] != NULL)
		(*(InterruptFuncTable[USART0_RX_vect_num]))(InterruptSenderTable[USART0_RX_vect_num]);
}

/* USART0 Data Register Empty */
ISR(USART0_UDRE_vect)
{
	if (InterruptFuncTable[USART0_UDRE_vect_num] != NULL)
		(*(InterruptFuncTable[USART0_UDRE_vect_num]))(InterruptSenderTable[USART0_UDRE_vect_num]);
}

/* USART0, Tx Complete */
ISR(USART0_TX_vect)
{
	if (InterruptFuncTable[USART0_TX_vect_num] != NULL)
		(*(InterruptFuncTable[USART0_TX_vect_num]))(InterruptSenderTable[USART0_TX_vect_num]);
}

/* ADC Conversion Complete */
ISR(ADC_vect)
{
	if (InterruptFuncTable[ADC_vect_num] != NULL)
		(*(InterruptFuncTable[ADC_vect_num]))(InterruptSenderTable[ADC_vect_num]);
}

/* EEPROM Ready */
ISR(EE_READY_vect)
{
	if (InterruptFuncTable[EE_READY_vect_num] != NULL)
		(*(InterruptFuncTable[EE_READY_vect_num]))(InterruptSenderTable[EE_READY_vect_num]);
}

/* Analog Comparator */
ISR(ANALOG_COMP_vect)
{
	if (InterruptFuncTable[ANALOG_COMP_vect_num] != NULL)
		(*(InterruptFuncTable[ANALOG_COMP_vect_num]))(InterruptSenderTable[ANALOG_COMP_vect_num]);
}

/* Timer/Counter1 Compare Match C */
ISR(TIMER1_COMPC_vect)
{
	if (InterruptFuncTable[TIMER1_COMPC_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER1_COMPC_vect_num]))(InterruptSenderTable[TIMER1_COMPC_vect_num]);
}

/* Timer/Counter3 Capture Event */
ISR(TIMER3_CAPT_vect)
{
	if (InterruptFuncTable[TIMER3_CAPT_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER3_CAPT_vect_num]))(InterruptSenderTable[TIMER3_CAPT_vect_num]);
}

/* Timer/Counter3 Compare Match A */
ISR(TIMER3_COMPA_vect)
{
	if (InterruptFuncTable[TIMER3_COMPA_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER3_COMPA_vect_num]))(InterruptSenderTable[TIMER3_COMPA_vect_num]);
}

/* Timer/Counter3 Compare Match B */
ISR(TIMER3_COMPB_vect)
{
	if (InterruptFuncTable[TIMER3_COMPB_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER3_COMPB_vect_num]))(InterruptSenderTable[TIMER3_COMPB_vect_num]);
}

/* Timer/Counter3 Compare Match C */
ISR(TIMER3_COMPC_vect)
{
	if (InterruptFuncTable[TIMER3_COMPC_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER3_COMPC_vect_num]))(InterruptSenderTable[TIMER3_COMPC_vect_num]);
}

/* Timer/Counter3 Overflow */
ISR(TIMER3_OVF_vect)
{
	if (InterruptFuncTable[TIMER3_OVF_vect_num] != NULL)
		(*(InterruptFuncTable[TIMER3_OVF_vect_num]))(InterruptSenderTable[TIMER3_OVF_vect_num]);
}

/* USART1, Rx Complete */
ISR(USART1_RX_vect)
{
	if (InterruptFuncTable[USART1_RX_vect_num] != NULL)
		(*(InterruptFuncTable[USART1_RX_vect_num]))(InterruptSenderTable[USART1_RX_vect_num]);
}

/* USART1, Data Register Empty */
ISR(USART1_UDRE_vect)
{
	if (InterruptFuncTable[USART1_UDRE_vect_num] != NULL)
		(*(InterruptFuncTable[USART1_UDRE_vect_num]))(InterruptSenderTable[USART1_UDRE_vect_num]);
}

/* USART1, Tx Complete */
ISR(USART1_TX_vect)
{
	if (InterruptFuncTable[USART1_TX_vect_num] != NULL)
		(*(InterruptFuncTable[USART1_TX_vect_num]))(InterruptSenderTable[USART1_TX_vect_num]);
}

/* 2-wire Serial Interface */
ISR(TWI_vect)
{
	if (InterruptFuncTable[TWI_vect_num] != NULL)
		(*(InterruptFuncTable[TWI_vect_num]))(InterruptSenderTable[TWI_vect_num]);
}

/* Store Program Memory Read */
ISR(SPM_READY_vect)
{
	if (InterruptFuncTable[SPM_READY_vect_num] != NULL)
		(*(InterruptFuncTable[SPM_READY_vect_num]))(InterruptSenderTable[SPM_READY_vect_num]);
}

#endif
#endif
