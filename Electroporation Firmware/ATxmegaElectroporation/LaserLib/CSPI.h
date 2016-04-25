/* 
* CSPI.h
*
* Created: 13.04.2016 11:28:00
* Author: TPU_2
*/


#ifndef __CSPI_H__
#define __CSPI_H__

#include <stdbool.h>
#include <avr/io.h>
#include "intx128.h"

typedef enum SPI_MODE_ENUM
{
	SPI_MODE_LRSMP_TFSTP = 0, /* Leading edge rising (sample), trailing edge falling (setup) */
	SPI_MODE_LRSTP_TFSMP = 1, /* Leading edge rising (setup), trailing edge falling (sample) */
	SPI_MODE_LFSMP_TRSTP = 2, /* Leading edge falling (sample), trailing edge rising (setup) */
	SPI_MODE_LFSTP_TRSMP = 3, /* Leading edge falling (setup), trailing edge rising (sample) */
} SPI_MODE, *PSPI_MODE;

typedef enum SPI_DATA_ORDER_ENUM
{
	SPI_DORD_LSBtoMSB = true,
	SPI_DORD_MSBtoLSB = false
} SPI_DATA_ORDER, *PSPI_DATA_ORDER;

class CSPI
{
public:
	CSPI();
	~CSPI();
	
	// Initialization
	void Initialize(bool Master, SPI_DATA_ORDER dord, SPI_MODE mode, bool doubleClk, SPI_PRESCALER_t prescaler);
	// Free buffer
	void Deinitialize();
	
	// Data send
	void Send(uint8_t* data, uint16_t length);
	void SendByte(uint8_t data);
	
	// ISR callback
	static void StaticOnTransmit(void* sender);
	void OnTransmit();
protected:
private:
	uint8_t* buffer;
	volatile uint16_t rx_pos;
	volatile uint16_t rx_frame_length;
}; //CSPI

#endif //__CSPI_H__
