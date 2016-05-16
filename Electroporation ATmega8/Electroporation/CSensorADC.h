/* 
* CSensorADC.h
*
* Created: 28.03.2016 16:22:18
* Author: TPU_2
*/


#ifndef __CSENSORADC_H__
#define __CSENSORADC_H__

#include "LaserLib/sys_interrupts.h"
#include <stdlib.h>
#include <stdint.h>

typedef enum ADC_REFERENCE_ENUM
{
	REF_AREF = 0,
	REF_AVCC = 1,
	REF_INT2V56 = 3
} ADC_REFERENCE, *PADC_REFERENCE;

typedef enum ADC_CHANNEL_ENUM
{
	ADC_CHANNEL0 = 0,
	ADC_CHANNEL1 = 1,
	ADC_CHANNEL2 = 2,
	ADC_CHANNEL3 = 3,
	ADC_CHANNEL4 = 4,
	ADC_CHANNEL5 = 5,
	ADC_CHANNEL6 = 6,
	ADC_CHANNEL7 = 7,
	ADC_CHANNEL_1V3 = 14,
	ADC_CHANNEL_GND = 15
} ADC_CHANNEL, *PADC_CHANNEL;

typedef enum ADC_PRESCALER_ENUM
{
	ADC_PSCL_DIV2R = 0,
	ADC_PSCL_DIV2 = 1,
	ADC_PSCL_DIV4 = 2,
	ADC_PSCL_DIV8 = 3,
	ADC_PSCL_DIV16 = 4,
	ADC_PSCL_DIV32 = 5,
	ADC_PSCL_DIV64 = 6,
	ADC_PSCL_DIV128 = 7,
} ADC_PRESCALER, *PADC_PRESCALER;

class CSensorADC
{
public:
	CSensorADC();
	~CSensorADC();
	
	// Basic ADC function
	virtual void Initialize(ADC_REFERENCE ref, ADC_CHANNEL adc_channel, ADC_PRESCALER adc_pscl, bool LeftAdjust, bool freeRunning);
	virtual void Start();
	virtual void SetADCCallback(ISRCallback func, void* owner);
	virtual uint16_t GetValue();
	
	// ISR Callback
	static void StaticADCConvCompleteCallback(void* sender);
	virtual void ADCConvCompleteCallback();
protected:
private:
	uint16_t value;
}; //CSensorADC

#endif //__CSENSORADC_H__
