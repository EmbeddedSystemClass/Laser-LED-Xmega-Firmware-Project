/* 
* CMBSenderDMA.cpp
*
* Created: 12.04.2016 10:16:52
* Author: TPU_2
*/


#include "CMBSenderDMA.h"
#include <avr/io.h>
#include <util/crc16.h>

// default constructor
CMBSenderDMA::CMBSenderDMA()
{
} //CMBSenderDMA

// default destructor
CMBSenderDMA::~CMBSenderDMA()
{
} //~CMBSenderDMA

// MODBUS Sender initialization
void CMBSenderDMA::Initialize(CUSART* usart, CMBEventsHandler *handler, DMA_t *dma, DMA_CHANNEL rxDMAch, DMA_CHANNEL txDMAch, uint16_t rx_bufSize, uint16_t tx_bufSize)
{	
	AllocateBuffers(rx_bufSize, tx_bufSize);
	switch (rxDMAch)
	{
		case DMA_CHANNEL_0:	
			rx_dma = &dma->CH0; 
			InterruptSenderTable[DMA_CH0_vect_num] = this;
			InterruptFuncTable[DMA_CH0_vect_num] = OnUSARTRxInterrupt;
		break;
		case DMA_CHANNEL_1:	
			rx_dma = &dma->CH1; 
			InterruptSenderTable[DMA_CH1_vect_num] = this;
			InterruptFuncTable[DMA_CH1_vect_num] = OnUSARTRxInterrupt;
			break;
		case DMA_CHANNEL_2:	
			rx_dma = &dma->CH2; 
			InterruptSenderTable[DMA_CH2_vect_num] = this;
			InterruptFuncTable[DMA_CH2_vect_num] = OnUSARTRxInterrupt;
			break;
		case DMA_CHANNEL_3:	
			rx_dma = &dma->CH3; 
			InterruptSenderTable[DMA_CH3_vect_num] = this;
			InterruptFuncTable[DMA_CH3_vect_num] = OnUSARTRxInterrupt;
			break;
	}
	switch (txDMAch)
	{
		case DMA_CHANNEL_0:	
			tx_dma = &dma->CH0; 
			InterruptSenderTable[DMA_CH0_vect_num] = this;
			InterruptFuncTable[DMA_CH0_vect_num] = OnUSARTTxInterrupt;
			break;
		case DMA_CHANNEL_1:	
			tx_dma = &dma->CH1; 
			InterruptSenderTable[DMA_CH1_vect_num] = this;
			InterruptFuncTable[DMA_CH1_vect_num] = OnUSARTTxInterrupt;
			break;
		case DMA_CHANNEL_2:	
			tx_dma = &dma->CH2; 
			InterruptSenderTable[DMA_CH2_vect_num] = this;
			InterruptFuncTable[DMA_CH2_vect_num] = OnUSARTTxInterrupt;
			break;
		case DMA_CHANNEL_3:	
			tx_dma = &dma->CH3; 
			InterruptSenderTable[DMA_CH3_vect_num] = this;
			InterruptFuncTable[DMA_CH3_vect_num] = OnUSARTTxInterrupt;
			break;
	}
	
	// Initialize RX DMA
	rx_dma->CTRLA = DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm | DMA_CH_BURSTLEN_1BYTE_gc;
	rx_dma->CTRLB = DMA_CH_TRNINTLVL_LO_gc;
	rx_dma->ADDRCTRL = DMA_CH_SRCDIR_FIXED_gc | DMA_CH_DESTDIR_INC_gc | DMA_CH_DESTRELOAD_BLOCK_gc;
	rx_dma->REPCNT = 0;
	// Initialize TX DMA
	tx_dma->CTRLA = DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm | DMA_CH_BURSTLEN_1BYTE_gc;
	tx_dma->CTRLB = DMA_CH_TRNINTLVL_LO_gc;
	tx_dma->ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_SRCRELOAD_BLOCK_gc | DMA_CH_DESTDIR_FIXED_gc;
	tx_dma->REPCNT = 0;
	
	// Variables
	tx_buffer_pos    = 0x00;
	tx_frame_length  = 0x00;
	rx_buffer_pos    = 0x00;
	rx_frame_length  = 0x00;
	rx_frame_crc     = 0x00;
	rx_currt_crc     = 0x00;
	modbus_receiver_state  = rx_Idle;
	modbus_transmitter_state  = tx_Idle;
	isTransaction = false;
	
	// Set usart interface interrupts
	pUSART = usart;
	usart->SetRxInterruptionCallback(NULL, NULL);
	usart->SetTxInterruptionCallback(NULL, NULL);
	
	// Set usart interface DMA
	usart->SetDMARxTrig(rx_dma);
	usart->SetDMATxTrig(tx_dma);
	
	// Set event handler
	if (handler)
		CallbackHandler = handler;
	else
		CallbackHandler = this;
} //CMBSender

// MODBUS Sender deinitialization (for reallocation)
void CMBSenderDMA::Deinitialize()
{
	FreeBuffers();
}

// MODBUS write data to register command
void CMBSenderDMA::WriteDataToRegisterAsync(uint8_t addr, uint8_t* data, uint8_t length)
{
	// Fill data transmit buffer
	FillRegDataTx(addr, data, length);
	
	// Start transition
	StartMODBUSTransmitter();
}

// MODBUS write data to SRAM command
void CMBSenderDMA::WriteDataToSRAMAsync(uint16_t addr, uint16_t* data, uint16_t length)
{
	// Fill data transmit buffer
	FillVarDataTx(addr, data, length);
	
	// Start transition
	StartMODBUSTransmitter();
}

// MODBUS request data from register command
void CMBSenderDMA::RequestDataFromRegisterAsync(uint8_t addr, uint8_t length)
{
	// Fill data transmit buffer
	FillRegDataRq(addr, length);
	
	// Start transition
	StartMODBUSTransmitter();
}

// MODBUS request data from SRAM command
void CMBSenderDMA::RequestDataFromSRAMAsync(uint16_t addr, uint8_t length)
{
	// Fill data transmit buffer
	FillVarDataRq(addr, length);
	
	// Start transition
	StartMODBUSTransmitter();
}

// MODBUS Receive byte event handler
void CMBSenderDMA::OnReceiveBlock(uint8_t data)
{
	switch (modbus_receiver_state)
	{
		case rx_Start :
		case rx_FrameHeader0 :
			break;
		case rx_FrameHeader1 :
			break;
		case rx_FrameLength :
			break;
		case rx_FrameReceive :
			{
// 				OnTransactionCallback(rx_buffer, rx_frame_length);
// 				if (CallbackHandler != 0)
// 					CallbackHandler->OnTransactionCallback(rx_buffer, rx_frame_length);
			}
			break;
		case rx_Error :
		case rx_TimeOut :
			isTransaction = false;
		case rx_Complete :
		case rx_Idle :
		default:
		break;
	};
}

// MODBUS Transmit byte event handler
void CMBSenderDMA::OnTransmitBlock()
{
	switch (modbus_transmitter_state)
	{
		case tx_Start :
			break;
		case tx_Transmitting :
			break;
		case tx_Idle :
		case tx_TimeOut :
		case tx_Complete :
			if (isTransaction)
				StartMODBUSListener();
			isTransaction = false;
		default :
		break;
	}
}

void CMBSenderDMA::StartRxDMA(uint8_t* dstmem, uint16_t length)
{
	// Set transaction size
	rx_dma->TRFCNT = length;
	
	// Set data address destination
	rx_dma->DESTADDR0 = (int)dstmem;
	rx_dma->DESTADDR1 = (int)dstmem >> 8;
	rx_dma->DESTADDR2 = 0;
	
	// Start DMA channel
	rx_dma->CTRLA |= DMA_CH_ENABLE_bm;
}

void CMBSenderDMA::StopRxDMA()
{
	// Stop DMA channel
	rx_dma->CTRLA &= ~DMA_CH_ENABLE_bm;
}

void CMBSenderDMA::StartTxDMA(uint8_t* srcmem, uint16_t length)
{
	// Set transaction size
	tx_dma->TRFCNT = length;
	
	// Set data address destination
	tx_dma->SRCADDR0 = (int)srcmem;
	tx_dma->SRCADDR1 = (int)srcmem >> 8;
	tx_dma->SRCADDR2 = 0;
	
	// Start DMA channel
	tx_dma->CTRLA |= DMA_CH_ENABLE_bm;
}

void CMBSenderDMA::StopTxDMA()
{
	// Stop DMA channel
	tx_dma->CTRLA &= ~DMA_CH_ENABLE_bm;
}

// MODBUS Timeout transaction handler
void CMBSenderDMA::OnTimeout()
{
	if (modbus_receiver_state != rx_Complete)
		modbus_receiver_state = rx_TimeOut;
}

// MODBUS Start receiver
void CMBSenderDMA::StartMODBUSListener()
{
	rx_buffer_pos    = 0x00;
	rx_frame_length  = 0x00;
	rx_frame_crc     = 0x00;
	rx_currt_crc     = 0x00;
	modbus_receiver_state  = rx_Start;
	
	StartRxDMA(rx_buffer, rx_frame_length);
}

// MODBUS Wait for receiver
MODBUS_STATE CMBSenderDMA::WaitMODBUSListener()
{
	while (modbus_receiver_state != rx_Complete)
	{
		if (modbus_receiver_state == rx_Error)
			return Error;
		if (modbus_receiver_state == rx_TimeOut)
			return TimeOut;
	}
	return Complete;
}

// MODBUS Stop receiver
void CMBSenderDMA::StopMODBUSListener()
{
	modbus_receiver_state  = rx_Idle;
	
	StopRxDMA();
}

// MODBUS Start transmitter
void CMBSenderDMA::StartMODBUSTransmitter()
{
	tx_buffer_pos = 0x00;
	modbus_transmitter_state  = tx_Start;
	
	StartTxDMA(tx_buffer, tx_frame_length);
}

// MODBUS Wait for transmitter
MODBUS_STATE CMBSenderDMA::WaitMODBUSTransmitter()
{
	while (modbus_transmitter_state != tx_Complete)
	{
		if (modbus_transmitter_state == tx_Error)
			return Error;
		if (modbus_transmitter_state == tx_TimeOut)
			return TimeOut;
	}
	return Complete;
}

// MODBUS Stop transmitter
void CMBSenderDMA::StopMODBUSTransmitter()
{
	modbus_transmitter_state = tx_Idle;
	
	StopTxDMA();
}

// Static method for Rx interrupt vector
void CMBSenderDMA::OnUSARTRxInterrupt(void* sender)
{
	CMBSenderDMA* owner = (CMBSenderDMA*)sender;
	uint8_t data = owner->pUSART->GetReceivedByte();
	owner->OnReceiveBlock(data);
}

// Static method for Tx interrupt vector
void CMBSenderDMA::OnUSARTTxInterrupt(void* sender)
{
	CMBSenderDMA* owner = (CMBSenderDMA*)sender;
	owner->OnTransmitBlock();
}

// Request data from register
void CMBSenderDMA::StartMODBUSRegisterTransaction(uint8_t addr, uint8_t length)
{
	isTransaction = true;
	RequestDataFromRegisterAsync(addr, length);
}

// Request data from SRAM
void CMBSenderDMA::StartMODBUSVariableTransaction(uint16_t addr, uint8_t length)
{
	isTransaction = true;
	RequestDataFromSRAMAsync(addr, length);
}

// Process data transaction
void CMBSenderDMA::ProcessTransaction(uint8_t* data, uint16_t length)
{
	if (data[0] == 0x81)
	{
		OnRegisterReceived(data[1], &data[3], data[2]);
	}
	if (data[0] == 0x83)
	{
		OnVariableReceived(data[1], (uint16_t*)&data[4], (data[2] << 8) | data[3]);
	}
}

// On Transaction event handler
void CMBSenderDMA::OnTransactionCallback(uint8_t* data, uint16_t length)
{
	ProcessTransaction(data, length);
}

// On data received event handler
void CMBSenderDMA::OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length)
{
	if (CallbackHandler != 0)
		CallbackHandler->OnVariableReceived(addr, data, length);
}

// On data received event handler
void CMBSenderDMA::OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length)
{
	if (CallbackHandler != 0)
		CallbackHandler->OnRegisterReceived(addr, data, length);
}

