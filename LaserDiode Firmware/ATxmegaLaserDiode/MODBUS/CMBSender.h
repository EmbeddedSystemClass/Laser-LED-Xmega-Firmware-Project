/* 
* CMBSender.h
*
* Created: 28.02.2016 22:01:08
* Author: Vladislav
*/


#ifndef __CMBSENDER_H__
#define __CMBSENDER_H__

#include <stdint.h>
#include <stdlib.h>
#include "CMBProtocol.h"
#include "../LaserLib/CUSART.h"
#include "../LaserLib/CTimer.h"

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

class CMBSender : public CMBEventsHandler
{
public:
	CMBSender();
	~CMBSender();
	
	void Initialize(CTimer* timer, CUSART* usart, CMBEventsHandler *handler, uint16_t rx_bufSize, uint16_t tx_bufSize, uint16_t timeout);
	void Deinitialize();
	
	// Send data methods asynchronous
	void WriteDataToRegisterAsync(uint8_t addr, uint8_t* data, uint8_t length);
	void WriteDataToSRAMAsync(uint16_t addr, uint16_t* data, uint16_t length);
	void RequestDataFromRegisterAsync(uint8_t addr, uint8_t length);
	void RequestDataFromSRAMAsync(uint16_t addr, uint8_t length);
	
	// Send data methods synchronous
	void WriteDataToRegister(uint8_t addr, uint8_t* data, uint8_t length);
	void WriteDataToSRAM(uint16_t addr, uint16_t* data, uint16_t length);
	//void RequestDataFromRegister(uint8_t addr, uint8_t length);
	//void RequestDataFromSRAM(uint16_t addr, uint8_t length);
	
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
	static void OnTimeoutInterrupt(void* sender);
	
	// CMBEventsHandler overload methods
	virtual void OnTransactionCallback(uint8_t* data, uint16_t length);
	virtual void OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length);
	virtual void OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length);
	
private:
	
	// main receive byte FSM function
	void OnReceiveByte(uint8_t data);
	void OnTransmitByte();
	void OnTimeout();
	void ProcessTransaction(uint8_t* data, uint16_t length);
	
protected:
	CUSART* pUSART;
	CTimer* pTimer;
	
	// Receiver variables
	volatile MODBUS_RECEIVER_STATE modbus_receiver_state;
	volatile uint8_t  rx_buffer_pos;
	uint16_t Timeout;
	// Variables for final state machine runtime CRC check
	uint16_t rx_frame_crc;
	uint16_t rx_currt_crc;
	
	// transmitter variables
	volatile MODBUS_TRANSMITTER_STATE modbus_transmitter_state;
	volatile uint8_t  tx_buffer_pos;
	
	// MODBUS state
	bool isTransaction;
	
	// Callback owner
	CMBEventsHandler *CallbackHandler;
}; //CMBSender

#endif //__CMBSENDER_H__
