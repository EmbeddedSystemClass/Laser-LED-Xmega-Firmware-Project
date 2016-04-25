/* 
* CDMA.cpp
*
* Created: 12.04.2016 13:22:01
* Author: TPU_2
*/


#include "CDMA.h"

// default constructor
CDMA_Channel::CDMA_Channel(DMA_CH_t* hDMA_CH) : m_hDMA_CH(hDMA_CH)
{
	
}

// default destructor
CDMA_Channel::~CDMA_Channel()
{
	
}

// default constructor
CDMA::CDMA(DMA_t* hdma) : m_hDMA(hdma)
{
} //CDMA

// default destructor
CDMA::~CDMA()
{
} //~CDMA

void CDMA::Initialize(DMA_DOUBLEBUF_MODE dbuf_mode, DMA_CHANNEL_PRIORITY chprior)
{
	m_hDMA->CTRL = (m_hDMA->CTRL & ~(DMA_DBUFMODE_gm | DMA_PRIMODE_gm)) 
				| (dbuf_mode << DMA_DBUFMODE0_bp) | (chprior << DMA_PRIMODE0_bp);
}

void CDMA::Reset()
{
	m_hDMA->CTRL |=	 DMA_RESET_bm;
	m_hDMA->CTRL &=	~DMA_RESET_bm;
}

void CDMA::Start()
{
	m_hDMA->CTRL |=  DMA_ENABLE_bm;
}

void CDMA::Stop()
{
	m_hDMA->CTRL &= ~DMA_ENABLE_bm;
}

bool CDMA::IsChannelBusy(DMA_CHANNEL channel)
{
	return (m_hDMA->STATUS & (1 << (channel + DMA_CH0BUSY_bp)));
}

bool CDMA::IsChannelPending(DMA_CHANNEL channel)
{
	return (m_hDMA->STATUS & (1 << (channel + DMA_CH0PEND_bp)));
}

CDMA_Channel* CDMA::getChannel(DMA_CHANNEL channel)
{
	CDMA_Channel* dma_channel;
	switch (channel)
	{
		case DMA_CHANNEL_0:
			dma_channel = new CDMA_Channel(&m_hDMA->CH0);
		break;
		case DMA_CHANNEL_1:
			dma_channel = new CDMA_Channel(&m_hDMA->CH1);
		break;
		case DMA_CHANNEL_2:
			dma_channel = new CDMA_Channel(&m_hDMA->CH2);
		break;
		case DMA_CHANNEL_3:
			dma_channel = new CDMA_Channel(&m_hDMA->CH3);
		break;
	}
	return dma_channel;
}
