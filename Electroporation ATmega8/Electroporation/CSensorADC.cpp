/* 
* CSensorADC.cpp
*
* Created: 28.03.2016 16:22:18
* Author: TPU_2
*/


#include "CSensorADC.h"
#include "LaserLib/int8a.h"

// default constructor
CSensorADC::CSensorADC()
{
} //CSensorADC

// default destructor
CSensorADC::~CSensorADC()
{
} //~CSensorADC

void CSensorADC::Initialize(ADC_REFERENCE ref, ADC_CHANNEL adc_channel, ADC_PRESCALER adc_pscl, bool LeftAdjust, bool freeRunning)
{
	// ADC multiplexer initialize
	ADMUX |= (ref << REFS0) | (adc_channel << MUX0) | ((LeftAdjust?1:0) << ADLAR);
	
	// ADC control setup
	ADCSRA |= (adc_pscl << ADPS0) | (1 << ADEN) | ((freeRunning?1:0) << ADFR) | (1 << ADIE);
	
	// ADC set interrupt
	InterruptFuncTable[ADC_vect_num] = &StaticADCConvCompleteCallback;
	InterruptSenderTable[ADC_vect_num] = this;
}

void CSensorADC::Start()
{
	ADCSRA |= (1 << ADSC);
}

void CSensorADC::SetADCCallback(ISRCallback func, void* owner)
{
	InterruptFuncTable[ADC_vect_num] = func;
	InterruptSenderTable[ADC_vect_num] = owner;
}

uint16_t CSensorADC::GetValue()
{
	return value;
}

void CSensorADC::StaticADCConvCompleteCallback(void* sender)
{
	CSensorADC* adc = (CSensorADC*)sender;
	adc->ADCConvCompleteCallback();
}

void CSensorADC::ADCConvCompleteCallback()
{
	value = ADC;
}
