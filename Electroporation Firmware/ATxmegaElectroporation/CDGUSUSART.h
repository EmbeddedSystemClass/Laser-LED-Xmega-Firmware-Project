/* 
* CDGUSUSART.h
*
* Created: 23.03.2016 13:12:15
* Author: TPU_2
*/


#ifndef __CDGUSUSART_H__
#define __CDGUSUSART_H__

#include "LaserLib/CUSART.h"

class CDGUSUSART : public CUSART
{
//functions
public:
	CDGUSUSART();
	~CDGUSUSART();

	virtual void Initialize(BAUDRATE baud, PARITY parity, STOPBITS stopbits, bool Async);

	virtual uint8_t GetReceivedByte();
	virtual void SetTransmittingByte(uint8_t data);
	virtual bool IsTransmitting();
	virtual bool IsReceiving();
	virtual bool IsDataEmpty();

	virtual void SetRxInterruptionCallback(void* sender, ISRCallback callback);
	virtual void SetTxInterruptionCallback(void* sender, ISRCallback callback);
	
	// DMA control
	virtual void SetDMARxTrig(DMA_CH_t *dma_channel);
	virtual void SetDMATxTrig(DMA_CH_t *dma_channel);
protected:
private:

}; //CDGUSUSART

#endif //__CDGUSUSART_H__
