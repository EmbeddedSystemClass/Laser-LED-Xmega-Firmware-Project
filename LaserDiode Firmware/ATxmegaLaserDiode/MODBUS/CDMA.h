/* 
* CDMA.h
*
* Created: 12.04.2016 13:22:01
* Author: TPU_2
*/


#ifndef __CDMA_H__
#define __CDMA_H__

#include <avr/io.h>
#include <stdbool.h>

typedef enum DMA_CHANNEL_ENUM
{
	DMA_CHANNEL_0 = 1,
	DMA_CHANNEL_1 = 2,
	DMA_CHANNEL_2 = 3,
	DMA_CHANNEL_3 = 4
} DMA_CHANNEL, *PDMA_CHANNEL;

typedef enum DMA_DOUBLEBUF_MODE_ENUM
{
	DMA_DBUFMODE_DISABLED = 0,
	DMA_DBUFMODE_CH01 = 1,
	DMA_DBUFMODE_CH23 = 2,
	DMA_DBUFMODE_CH01CH23 = 3
} DMA_DOUBLEBUF_MODE, *PDMA_DOUBLEBUF_MODE;

typedef enum DMA_CHANNEL_PRIORITY_ENUM
{
	RoundRobin0123 = 0,
	Channel0RoundRobin123 = 1,
	Channel01RoundRobin23 = 2,
	Channel0123 = 3
} DMA_CHANNEL_PRIORITY, *PDMA_CHANNEL_PRIORITY;

class CDMA_Channel
{
public:
	CDMA_Channel(DMA_CH_t* hDMA_CH);
	~CDMA_Channel();
private:
	DMA_CH_t* m_hDMA_CH;
};

class CDMA
{
public:
	CDMA(DMA_t* hdma);
	~CDMA();
	
	// Initialize DMA
	void Initialize(DMA_DOUBLEBUF_MODE dbuf_mode, DMA_CHANNEL_PRIORITY chprior);
	// Reset DMA
	void Reset();
	// Start DMA
	void Start();
	// Stop DMA
	void Stop();
	
	// DMA status
	bool IsChannelBusy(DMA_CHANNEL channel);
	bool IsChannelPending(DMA_CHANNEL channel);
	CDMA_Channel* getChannel(DMA_CHANNEL channel);
protected:
private:
	DMA_t* m_hDMA;
}; //CDMA

#endif //__CDMA_H__
