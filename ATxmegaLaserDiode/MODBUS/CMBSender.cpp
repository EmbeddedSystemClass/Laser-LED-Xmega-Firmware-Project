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


void CMBSender::Initialize(CUSART* usart, CMBEventsHandler *handler, uint16_t rx_bufSize, uint16_t tx_bufSize)
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
	
	// Set usart interface interrupts
	pUSART = usart;
	usart->SetRxInterruptionCallback((void*)this, OnUSARTRxInterrupt);
	usart->SetTxInterruptionCallback((void*)this, OnUSARTTxInterrupt);
	
	// Set event handler
	if (handler)
		CallbackHandler = handler;
	else
		CallbackHandler = this;
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

// local methods
void CMBSender::FillRegDataTx(uint8_t addr, uint8_t* data, uint8_t length)
{
		// Frame header
	tx_buffer[0] = FRAMEHEADER_LOWBYTE;
	tx_buffer[1] = FRAMEHEADER_HIGHBYTE;
		// Frame size
#ifdef USE_CRC
	tx_buffer[2] = length + 4; //CMD (1) + DATA(length) + CRC(2) + ADDR(1)
#else
	tx_buffer[2] = length + 2; //CMD (1) + DATA(length) + ADDR(1)
#endif
	tx_buffer[3] = 0x80;	// Frame command
	tx_buffer[4] = addr;	// Register address
	tx_frame_length = length + 5;
		
	// Frame data
	for (uint16_t i = 0; i < length; i++)
		tx_buffer[i+5] = data[i];
		
#ifdef USE_CRC
		// Calculate CRC
	uint16_t crc = 0;
	for (uint16_t i = 5; i < tx_frame_length; i++)
		crc = _crc16_update(crc, tx_buffer[i]);
	tx_buffer[tx_frame_length]   = crc >> 8;
	tx_buffer[tx_frame_length+1] = crc & 0xff;
	tx_frame_length = length + 7;
#endif
}

void CMBSender::FillRegDataRq(uint8_t addr, uint8_t length)
{
	// Frame header
	tx_buffer[0] = FRAMEHEADER_LOWBYTE;
	tx_buffer[1] = FRAMEHEADER_HIGHBYTE;
	// Frame size
#ifdef USE_CRC
	tx_buffer[2] = 5; //CMD(1) + DATA(2) + CRC(2)
#else
	tx_buffer[2] = 3; //CMD(1) + DATA(2)
#endif
	tx_buffer[3] = 0x81;	// Frame command
	tx_buffer[4] = addr;	// Register address
	tx_buffer[5] = length;	// Data
	tx_frame_length = 6;
	
#ifdef USE_CRC
	// Calculate CRC
	uint16_t crc = 0;
	for (uint16_t i = 3; i < tx_frame_length; i++)
		crc = _crc16_update(crc, tx_buffer[i]);
	tx_buffer[tx_frame_length]   = crc >> 8;
	tx_buffer[tx_frame_length+1] = crc & 0xff;
	tx_frame_length = length + 8;
#endif
}

void CMBSender::FillVarDataTx(uint16_t addr, uint16_t* data, uint16_t length)
{
	// Frame header
	tx_buffer[0] = FRAMEHEADER_LOWBYTE;
	tx_buffer[1] = FRAMEHEADER_HIGHBYTE;
		// Frame size
#ifdef USE_CRC
	tx_buffer[2] = length * 2 + 5; //CMD (1) + DATA(length * 2) + CRC(2) + ADDR(2)
#else
	tx_buffer[2] = length * 2 + 3; //CMD (1) + DATA(length * 2) + ADDR(2)
#endif
	tx_buffer[3] = 0x82;	// Frame command
	tx_buffer[4] = addr >> 8;
	tx_buffer[5] = addr & 0xff;
	tx_frame_length = length * 2 + 6;
		
	// Frame data
	for (uint16_t i = 0; i < length; i++)
	{
		tx_buffer[6 + i*2 + 0] = data[i] >> 8;
		tx_buffer[6 + i*2 + 1] = data[i] & 0xff;
	}
		
#ifdef USE_CRC
	// Calculate CRC
	uint16_t crc = 0;
	for (int i = 6; i < tx_frame_length; i++)
		crc = _crc16_update(crc, tx_buffer[i]);
	tx_buffer[tx_frame_length] = crc >> 8;
	tx_buffer[tx_frame_length+1] = crc & 0xff;
	this->tx_frame_length = length * 2 + 8;
#endif
}

void CMBSender::FillVarDataRq(uint16_t addr, uint16_t length)
{
	// Frame header
	tx_buffer[0] = FRAMEHEADER_LOWBYTE;
	tx_buffer[1] = FRAMEHEADER_HIGHBYTE;
	// Frame size
#ifdef USE_CRC
	tx_buffer[2] = 6; //CMD(1) + DATA(3) + CRC(2)
#else
	tx_buffer[2] = 4; //CMD(1) + DATA(3)
#endif
	tx_buffer[3] = 0x83;				// Frame command
	tx_buffer[4] = addr >> 8;			// Register address
	tx_buffer[5] = addr & 0xff;			// Register address
	tx_buffer[6] = length * 2;			// Data
	tx_frame_length = 7;
	
#ifdef USE_CRC
	// Calculate CRC
	uint16_t crc = 0;
	for (uint16_t i = 3; i < tx_frame_length; i++)
		crc = _crc16_update(crc, tx_buffer[i]);
	tx_buffer[tx_frame_length]   = crc >> 8;
	tx_buffer[tx_frame_length+1] = crc & 0xff;
	tx_frame_length = 9;
#endif	
}

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
				/*if (isTransaction)
					StartMODBUSTransmitter();*/
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
				/*if (isTransaction)
					StartMODBUSTransmitter();*/
				
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
		OnVariableReceived(data[1], (uint16_t*)&data[4], (data[2] << 8) | data[3]);
	}
}

void CMBSender::OnTransactionCallback(uint8_t* data, uint16_t length)
{
	ProcessTransaction(data, length);
	// no code
}

void CMBSender::OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length)
{
	if (CallbackHandler != 0)
		CallbackHandler->OnVariableReceived(addr, data, length);
	// no code
}


void CMBSender::OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length)
{
	if (CallbackHandler != 0)
		CallbackHandler->OnRegisterReceived(addr, data, length);
	// no code
}
