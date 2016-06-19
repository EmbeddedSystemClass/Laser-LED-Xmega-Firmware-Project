/* 
* CMBSender.cpp
*
* Created: 28.02.2016 22:01:08
* Author: Vladislav
*/


#include "CMBSender.h"
#include <avr/io.h>
#include <util/crc16.h>

// default constructor
CMBSender::CMBSender()
{
	
}


void CMBSender::Initialize(CTimer* timer, CUSART* usart, CMBEventsHandler *handler, uint16_t rx_bufSize, uint16_t tx_bufSize, uint16_t timeout)
{
	rx_bufferSize = rx_bufSize;
	tx_bufferSize = tx_bufSize;
	rx_buffer = (uint8_t*)malloc(rx_bufferSize);
	tx_buffer = (uint8_t*)malloc(tx_bufferSize);
	
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
	Timeout = timeout;
	
	// Set usart interface interrupts
	pUSART = usart;
	usart->SetRxInterruptionCallback((void*)this, OnUSARTRxInterrupt);
	usart->SetTxInterruptionCallback((void*)this, OnUSARTTxInterrupt);
	
	// Set event handler
	if (handler)
		CallbackHandler = handler;
	else
		CallbackHandler = this;
		
	// Set timeout timer
	pTimer = timer;
	if (pTimer)
	{
		pTimer->SetOVFCallback(OnTimeoutInterrupt, (void*)this, TC_OVFINTLVL_LO_gc);
	}
} //CMBSender

void CMBSender::Deinitialize()
{
	free(rx_buffer);
	free(tx_buffer);
}

// default destructor
CMBSender::~CMBSender()
{
} //~CMBSender

// MODBUS write data to register command
void CMBSender::WriteDataToRegisterAsync(uint8_t addr, uint8_t* data, uint8_t length)
{
	// Fill data transmit buffer
	FillRegDataTx(addr, data, length);
	
	// Start transition
	StartMODBUSTransmitter();
}

void CMBSender::WriteDataToSRAMAsync(uint16_t addr, uint16_t* data, uint16_t length)
{
	// Fill data transmit buffer
	FillVarDataTx(addr, data, length);
		
	// Start transition
	StartMODBUSTransmitter();
}

void CMBSender::RequestDataFromRegisterAsync(uint8_t addr, uint8_t length)
{
	// Fill data transmit buffer
	FillRegDataRq(addr, length);
	
	// Start transition
	StartMODBUSTransmitter();
}

void CMBSender::RequestDataFromSRAMAsync(uint16_t addr, uint8_t length)
{
	// Fill data transmit buffer
	FillVarDataRq(addr, length);
	
	// Start transition
	StartMODBUSTransmitter();
}

// Send data methods synchronous
void CMBSender::WriteDataToRegister(uint8_t addr, uint8_t* data, uint8_t length)
{
	// Fill data transmit buffer
	FillRegDataTx(addr, data, length);
	
	// Send data asynchronous
	for (uint16_t i = 0; i < tx_frame_length; i++)
	{
		while (pUSART->IsDataEmpty());
		pUSART->SetTransmittingByte(tx_buffer[i]);
	}
}

void CMBSender::WriteDataToSRAM(uint16_t addr, uint16_t* data, uint16_t length)
{
	// Fill data transmit buffer
	FillVarDataTx(addr, data, length);
	
	// Send data asynchronous
	for (uint16_t i = 0; i < tx_frame_length; i++)
	{
		while (pUSART->IsDataEmpty());
		pUSART->SetTransmittingByte(tx_buffer[i]);
	}
}

//void CMBSender::RequestDataFromRegister(uint8_t addr, uint8_t length);
//void CMBSender::RequestDataFromSRAM(uint16_t addr, uint8_t length);

void CMBSender::OnReceiveByte(uint8_t data)
{
	switch (modbus_receiver_state)
	{
		case rx_Start :
		case rx_FrameHeader0 :
			if (data == FRAMEHEADER_LOWBYTE)
				modbus_receiver_state = rx_FrameHeader1;
			else
				modbus_receiver_state = rx_FrameHeader0; // Skip
			break;
		case rx_FrameHeader1 :
			if (data == FRAMEHEADER_HIGHBYTE)
				modbus_receiver_state = rx_FrameLength;
			else
				modbus_receiver_state = rx_FrameHeader0; // Skip
			break;
		case rx_FrameLength :
			rx_frame_length = data;
			rx_buffer_pos = 0;
			rx_currt_crc = 0;
			modbus_receiver_state = rx_FrameReceive;
			break;
		case rx_FrameReceive :
			rx_buffer[rx_buffer_pos] = data;
			if (rx_buffer_pos > rx_bufferSize)
			{
				modbus_receiver_state = rx_Error;
				break;
			}
			rx_currt_crc = _crc16_update(rx_currt_crc, data);
			rx_buffer_pos++;
			if (rx_buffer_pos == rx_frame_length)
			{
#ifdef USE_CRC
				modbus_receiver_state = rx_CRC0;
#else
				modbus_receiver_state = rx_Complete;
#endif
				OnTransactionCallback(rx_buffer, rx_frame_length);
				if (CallbackHandler != 0)
					CallbackHandler->OnTransactionCallback(rx_buffer, rx_frame_length);
			}
			break;
		case rx_CRC0 :
			rx_frame_crc = data;
			modbus_receiver_state = rx_CRC1;
			break;
		case rx_CRC1 :
			rx_frame_crc |= (data << 8);
			if (rx_currt_crc == rx_frame_crc)
			{
				modbus_receiver_state = rx_Complete;
				
				OnTransactionCallback(rx_buffer, rx_frame_length);
				if (CallbackHandler != 0)
					CallbackHandler->OnTransactionCallback(rx_buffer, rx_frame_length);
			}
			else
				modbus_receiver_state = rx_Error;
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

void CMBSender::OnTransmitByte()
{
	switch (modbus_transmitter_state)
	{
		case tx_Start :
			modbus_transmitter_state = tx_Transmitting; // Start receiving packet
			tx_buffer_pos = 0;
			pUSART->SetTransmittingByte(tx_buffer[tx_buffer_pos]);
			tx_buffer_pos++;
			break;
		case tx_Transmitting :
			pUSART->SetTransmittingByte(tx_buffer[tx_buffer_pos]);
			tx_buffer_pos++;
			if (tx_buffer_pos == tx_frame_length)
				modbus_transmitter_state = tx_Complete;
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

void CMBSender::OnTimeout()
{
	pTimer->Stop();
	if (modbus_receiver_state != rx_Complete)
		modbus_receiver_state = rx_TimeOut;
}

void CMBSender::StartMODBUSListener()
{
	rx_buffer_pos    = 0x00;
	rx_frame_length  = 0x00;
	rx_frame_crc     = 0x00;
	rx_currt_crc     = 0x00;
	modbus_receiver_state  = rx_Start;
	pTimer->Start(Timeout);
}

MODBUS_STATE CMBSender::WaitMODBUSListener()
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

void CMBSender::StopMODBUSListener()
{
	modbus_receiver_state  = rx_Idle;
}

void CMBSender::StartMODBUSTransmitter()
{
	tx_buffer_pos = 0x00;
	modbus_transmitter_state  = tx_Start;
	OnTransmitByte();
	pTimer->Start(Timeout);
}

MODBUS_STATE CMBSender::WaitMODBUSTransmitter()
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

void CMBSender::StopMODBUSTransmitter()
{
	modbus_transmitter_state = tx_Idle;
}

void CMBSender::OnUSARTRxInterrupt(void* sender)
{
	CMBSender* owner = (CMBSender*)sender;
	uint8_t data = owner->pUSART->GetReceivedByte();
	owner->OnReceiveByte(data);
}

void CMBSender::OnUSARTTxInterrupt(void* sender)
{
	CMBSender* owner = (CMBSender*)sender;
	owner->OnTransmitByte();
}

void CMBSender::OnTimeoutInterrupt(void* sender)
{
	CMBSender* owner = (CMBSender*)sender;
	owner->OnTimeout();
}

void CMBSender::StartMODBUSRegisterTransaction(uint8_t addr, uint8_t length)
{
	isTransaction = true;
	RequestDataFromRegisterAsync(addr, length);
}

void CMBSender::StartMODBUSVariableTransaction(uint16_t addr, uint8_t length)
{
	isTransaction = true;
	RequestDataFromSRAMAsync(addr, length);
}

void CMBSender::ProcessTransaction(uint8_t* data, uint16_t length)
{
	if (data[0] == 0x81)
	{
		OnRegisterReceived(data[1], &data[3], data[2]);
	}
	if (data[0] == 0x83)
	{
		OnVariableReceived((data[1] << 8) | data[2], (uint16_t*)&data[4], data[3]);
	}
}

void CMBSender::OnTransactionCallback(uint8_t* data, uint16_t length)
{
	CMBSender::ProcessTransaction(data, length);
}

void CMBSender::OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length)
{
	if (CallbackHandler != 0)
		CallbackHandler->OnVariableReceived(addr, data, length);
}


void CMBSender::OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length)
{
	if (CallbackHandler != 0)
		CallbackHandler->OnRegisterReceived(addr, data, length);
}
