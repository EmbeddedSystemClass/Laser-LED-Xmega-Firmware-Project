/* 
* CMBSenderDMA.h
*
* Created: 12.04.2016 10:16:53
* Author: TPU_2
*/


#ifndef __CMBSENDERDMA_H__
#define __CMBSENDERDMA_H__

#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include "CMBProtocol.h"
#include "CDMA.h"
#include "../LaserLib/CUSART.h"

typedef enum MODBUS_RECEIVER_STATE_ENUM
{
	rx_Start,
	rx_FrameHeader0,
	rx_FrameHeader1,
	rx_FrameLength,
	rx_FrameReceive,
	rx_CRC0,
	rx_CRC1,
	rx_Idle,
	rx_Complete,
	rx_Error,
	rx_TimeOut
} MODBUS_RECEIVER_STATE, *PMODBUS_RECEIVER_STATE;

typedef enum MODBUS_TRANSMITTER_STATE_ENUM
{
	tx_Start,
	tx_Transmitting,
	tx_Idle,
	tx_Complete,
	tx_Error,
	tx_TimeOut
} MODBUS_TRANSMITTER_STATE, *PMODBUS_TRANSMITTER_STATE;

typedef enum MODBUS_STATE_ENUM
{
	Idle,
	Complete,
	Error,
	TimeOut
} MODBUS_STATE, *PMODBUS_STATE;

typedef void (*FPModbusCallback)(CMBEventsHandler *handler, uint8_t* data, uint16_t length);

class CMBSenderDMA : public CMBEventsHandler
{
public:
	CMBSenderDMA();
	~CMBSenderDMA();
	
	void Initialize(CUSART* usart, CMBEventsHandler *handler, DMA_t *dma, DMA_CHANNEL rxDMAch, DMA_CHANNEL txDMAch, uint16_t rx_bufSize, uint16_t tx_bufSize);
	void Deinitialize();
	
	// Send data methods asynchronous
	void WriteDataToRegisterAsync(uint8_t addr, uint8_t* data, uint8_t length);
	void WriteDataToSRAMAsync(uint16_t addr, uint16_t* data, uint16_t length);
	void RequestDataFromRegisterAsync(uint8_t addr, uint8_t length);
	void RequestDataFromSRAMAsync(uint16_t addr, uint8_t length);
	
	// Receive data methods
	void StartMODBUSListener();
	MODBUS_STATE WaitMODBUSListener();
	void StopMODBUSListener();
	
	// Transmit data methods
	void StartMODBUSTransmitter();
	MODBUS_STATE WaitMODBUSTransmitter();
	void StopMODBUSTransmitter();
	
	// Transaction methods
	void StartMODBUSRegisterTransaction(uint8_t addr, uint8_t length);
	void StartMODBUSVariableTransaction(uint16_t addr, uint8_t length);
	
protected:
	// ISR Callback
	static void OnUSARTRxInterrupt(void* sender);
	static void OnUSARTTxInterrupt(void* sender);
	
	// CMBEventsHandler overload methods
	virtual void OnTransactionCallback(uint8_t* data, uint16_t length);
	virtual void OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length);
	virtual void OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length);
	
private:	
	// main receive byte FSM function
	void OnReceiveBlock(uint8_t data);
	void OnTransmitBlock();
	void OnTimeout();
	void ProcessTransaction(uint8_t* data, uint16_t length);
	
	// DMA functions
	void StartRxDMA(uint8_t* dstmem, uint16_t length);
	void StopRxDMA();
	void StartTxDMA(uint8_t* srcmem, uint16_t length);
	void StopTxDMA();
	
protected:
	CUSART*  pUSART;
	DMA_CH_t* rx_dma;
	DMA_CH_t* tx_dma;
	
	// Receiver variables
	volatile MODBUS_RECEIVER_STATE modbus_receiver_state;
	uint8_t  rx_buffer_pos;
	// Variables for final state machine runtime CRC check
	uint16_t rx_frame_crc;
	uint16_t rx_currt_crc;
	
	// transmitter variables
	volatile MODBUS_TRANSMITTER_STATE modbus_transmitter_state;
	uint8_t  tx_buffer_pos;
	
	// MODBUS state
	bool isTransaction;
	
	// Callback owner
	CMBEventsHandler *CallbackHandler;
}; //CMBSenderDMA

#endif //__CMBSENDERDMA_H__
