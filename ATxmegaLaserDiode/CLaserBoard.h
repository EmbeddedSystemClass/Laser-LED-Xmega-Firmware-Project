/* 
* CLaserBoard.h
*
* Created: 09.04.2016 15:23:56
* Author: Vladislav
*/


#ifndef __CLASERBOARD_H__
#define __CLASERBOARD_H__

#define F_CPU 32000000

#include <avr/io.h>

typedef enum LASERBOARD_PINS_ENUM
{
	// Analog port
	PIN_ADC0,				// = PIN0_bp, PORTB
	PIN_ADC1,				// = PIN1_bp, PORTA
	PIN_ADC2,				// = PIN2_bp, PORTA
	PIN_ADC3,				// = PIN3_bp, PORTA
	PIN_ADC4,				// = PIN4_bp, PORTA
	PIN_ADC5,				// = PIN5_bp, PORTA
	PIN_ADC6,				// = PIN6_bp, PORTA
	PIN_ADC7,				// = PIN7_bp, PORTA
	PIN_ATRIG,				// = PIN1_bp, PORTB
	PIN_DAC0,				// = PIN2_bp, PORTB
	PIN_DAC1,				// = PIN3_bp, PORTB
	// IN Port
	PIN_IN0,				// = PIN0_bp, PORTC
	PIN_IN1,				// = PIN1_bp, PORTC
	PIN_IN2,				// = PIN2_bp, PORTC
	PIN_IN3,				// = PIN3_bp, PORTC
	PIN_IN4,				// = PIN4_bp, PORTC
	PIN_IN5,				// = PIN5_bp, PORTC
	PIN_IN6,				// = PIN6_bp, PORTC
	PIN_IN7,				// = PIN7_bp, PORTC // Laser tattoo interface
	PIN_IN8,				// = PIN5_bp, PORTD // Laser tattoo interface
	PIN_IN9,				// = PIN6_bp, PORTD // Laser tattoo interface
	// Fast OUT Port
	PIN_LASERGATE,			// = PIN0_bp, PORTF // Laser TTL out
	PIN_OUT1,				// = PIN1_bp, PORTF
	PIN_OUT2,				// = PIN2_bp, PORTF
	PIN_OUT3,				// = PIN3_bp, PORTF
	PIN_OUT4,				// = PIN4_bp, PORTF
	PIN_OUT5,				// = PIN5_bp, PORTF
	PIN_OUT6,				// = PIN6_bp, PORTF
	PIN_OUT7,				// = PIN7_bp, PORTF
	// Slow OUT Port
	PIN_SLOWOUT0,			// = PIN1_bp, PORTE
	PIN_SLOWOUT1,			// = PIN2_bp, PORTE
	PIN_SLOWOUT2,			// = PIN4_bp, PORTD // Relay 1
	PIN_SLOWOUT3			// = PIN0_bp, PORTD // Relay 2
} LASERBOARD_PINS, *PLASERBOARD_PINS;

class CLaserBoard
{
//functions
public:
	CLaserBoard();
	~CLaserBoard();
	
	void InitializeIO();
	void InitializeClock();
	
	void Beep();
	void BeepClassError();
protected:
private:

}; //CLaserBoard

#endif //__CLASERBOARD_H__
