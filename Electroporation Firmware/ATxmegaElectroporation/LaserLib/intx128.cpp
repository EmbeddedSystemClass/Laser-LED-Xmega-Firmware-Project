/*
 * intx128.cpp
 *
 * Created: 04.03.2016 15:02:29
 *  Author: Vladislav
 */ 

#ifndef INTX128_CPP_
#define INTX128_CPP_

#include "intx128.h"
#include <avr/interrupt.h>
#include <avr/io.h>

void* InterruptSenderTable[NUM_INERRUPTS+1] = {0};
ISRCallback InterruptFuncTable[NUM_INERRUPTS+1] = {0};

#define SYS_INTERRUPT_VECTOR_HANDLER_MACRO(vector_id)										\
	ISR(vector_id)																			\
	{																						\
		if (InterruptFuncTable[vector_id##_num] != 0)										\
		(*(InterruptFuncTable[vector_id##_num]))(InterruptSenderTable[vector_id##_num]);	\
	}

/* OSC interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(OSC_OSCF_vect)      /* External Oscillator Failure Interrupt (NMI) */

/* PORTC interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTC_INT0_vect)      /* External Interrupt 0 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTC_INT1_vect)      /* External Interrupt 1 */

/* PORTR interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTR_INT0_vect)      /* External Interrupt 0 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTR_INT1_vect)      /* External Interrupt 1 */

/* DMA interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(DMA_CH0_vect)      /* Channel 0 Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(DMA_CH1_vect)      /* Channel 1 Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(DMA_CH2_vect)      /* Channel 2 Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(DMA_CH3_vect)      /* Channel 3 Interrupt */

/* RTC interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(RTC_OVF_vect)      /* Overflow Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(RTC_COMP_vect)      /* Compare Interrupt */

/* TWIC interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TWIC_TWIS_vect)      /* TWI Slave Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TWIC_TWIM_vect)      /* TWI Master Interrupt */

/* TCC0 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCC0_OVF_vect)      /* Overflow Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCC0_ERR_vect)      /* Error Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCC0_CCA_vect)      /* Compare or Capture A Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCC0_CCB_vect)      /* Compare or Capture B Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCC0_CCC_vect)      /* Compare or Capture C Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCC0_CCD_vect)      /* Compare or Capture D Interrupt */

/* TCC1 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCC1_OVF_vect)      /* Overflow Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCC1_ERR_vect)      /* Error Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCC1_CCA_vect)      /* Compare or Capture A Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCC1_CCB_vect)      /* Compare or Capture B Interrupt */

/* SPIC interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(SPIC_INT_vect)      /* SPI Interrupt */

/* USARTC0 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTC0_RXC_vect)      /* Reception Complete Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTC0_DRE_vect)      /* Data Register Empty Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTC0_TXC_vect)      /* Transmission Complete Interrupt */

/* USARTC1 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTC1_RXC_vect)      /* Reception Complete Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTC1_DRE_vect)      /* Data Register Empty Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTC1_TXC_vect)      /* Transmission Complete Interrupt */

/* AES interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(AES_INT_vect)      /* AES Interrupt */

/* NVM interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(NVM_EE_vect)      /* EE Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(NVM_SPM_vect)      /* SPM Interrupt */

/* PORTB interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTB_INT0_vect)      /* External Interrupt 0 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTB_INT1_vect)      /* External Interrupt 1 */

/* ACB interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ACB_AC0_vect)      /* AC0 Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ACB_AC1_vect)      /* AC1 Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ACB_ACW_vect)      /* ACW Window Mode Interrupt */

/* ADCB interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ADCB_CH0_vect)      /* Interrupt 0 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ADCB_CH1_vect)      /* Interrupt 1 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ADCB_CH2_vect)      /* Interrupt 2 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ADCB_CH3_vect)      /* Interrupt 3 */

/* PORTE interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTE_INT0_vect)      /* External Interrupt 0 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTE_INT1_vect)      /* External Interrupt 1 */

/* TWIE interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TWIE_TWIS_vect)      /* TWI Slave Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TWIE_TWIM_vect)		/* TWI Master Interrupt */

/* TCE0 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCE0_OVF_vect)      /* Overflow Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCE0_ERR_vect)      /* Error Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCE0_CCA_vect)      /* Compare or Capture A Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCE0_CCB_vect)      /* Compare or Capture B Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCE0_CCC_vect)      /* Compare or Capture C Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCE0_CCD_vect)      /* Compare or Capture D Interrupt */

/* TCE1 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCE1_OVF_vect)      /* Overflow Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCE1_ERR_vect)      /* Error Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCE1_CCA_vect)      /* Compare or Capture A Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCE1_CCB_vect)      /* Compare or Capture B Interrupt */

/* SPIE interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(SPIE_INT_vect)      /* SPI Interrupt */

/* USARTE0 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTE0_RXC_vect)      /* Reception Complete Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTE0_DRE_vect)      /* Data Register Empty Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTE0_TXC_vect)      /* Transmission Complete Interrupt */

/* USARTE1 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTE1_RXC_vect)      /* Reception Complete Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTE1_DRE_vect)      /* Data Register Empty Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTE1_TXC_vect)      /* Transmission Complete Interrupt */

/* PORTD interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTD_INT0_vect)      /* External Interrupt 0 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTD_INT1_vect)      /* External Interrupt 1 */

/* PORTA interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTA_INT0_vect)      /* External Interrupt 0 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTA_INT1_vect)      /* External Interrupt 1 */

/* ACA interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ACA_AC0_vect)      /* AC0 Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ACA_AC1_vect)      /* AC1 Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ACA_ACW_vect)      /* ACW Window Mode Interrupt */

/* ADCA interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ADCA_CH0_vect)      /* Interrupt 0 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ADCA_CH1_vect)      /* Interrupt 1 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ADCA_CH2_vect)      /* Interrupt 2 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(ADCA_CH3_vect)      /* Interrupt 3 */

/* TCD0 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCD0_OVF_vect)      /* Overflow Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCD0_ERR_vect)      /* Error Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCD0_CCA_vect)      /* Compare or Capture A Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCD0_CCB_vect)      /* Compare or Capture B Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCD0_CCC_vect)      /* Compare or Capture C Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCD0_CCD_vect)      /* Compare or Capture D Interrupt */

/* TCD1 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCD1_OVF_vect)      /* Overflow Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCD1_ERR_vect)      /* Error Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCD1_CCA_vect)      /* Compare or Capture A Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCD1_CCB_vect)      /* Compare or Capture B Interrupt */

/* SPID interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(SPID_INT_vect)      /* SPI Interrupt */

/* USARTD0 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTD0_RXC_vect)      /* Reception Complete Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTD0_DRE_vect)      /* Data Register Empty Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTD0_TXC_vect)      /* Transmission Complete Interrupt */

/* USARTD1 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTD1_RXC_vect)      /* Reception Complete Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTD1_DRE_vect)      /* Data Register Empty Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTD1_TXC_vect)      /* Transmission Complete Interrupt */

/* PORTF interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTF_INT0_vect)      /* External Interrupt 0 */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(PORTF_INT1_vect)      /* External Interrupt 1 */

/* TCF0 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCF0_OVF_vect)      /* Overflow Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCF0_ERR_vect)      /* Error Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCF0_CCA_vect)      /* Compare or Capture A Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCF0_CCB_vect)      /* Compare or Capture B Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCF0_CCC_vect)      /* Compare or Capture C Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(TCF0_CCD_vect)      /* Compare or Capture D Interrupt */

/* USARTF0 interrupt vectors */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTF0_RXC_vect)      /* Reception Complete Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTF0_DRE_vect)      /* Data Register Empty Interrupt */
SYS_INTERRUPT_VECTOR_HANDLER_MACRO(USARTF0_TXC_vect)      /* Transmission Complete Interrupt */

#endif