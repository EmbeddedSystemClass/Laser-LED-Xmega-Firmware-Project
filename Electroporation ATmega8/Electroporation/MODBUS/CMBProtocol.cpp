/* 
* CMBProtocol.cpp
*
* Created: 04.03.2016 14:36:31
* Author: Vladislav
*/


#include "CMBProtocol.h"
#include <avr/io.h>
#include <util/crc16.h>

CMBEventsHandler::~CMBEventsHandler()
{
} //~CMBProtocol

void CMBEventsHandler::AllocateBuffers(uint16_t rx_bufSize, uint16_t tx_bufSize)
{
	rx_bufferSize = rx_bufSize;
	tx_bufferSize = tx_bufSize;
	rx_buffer = (uint8_t*)malloc(rx_bufferSize);
	tx_buffer = (uint8_t*)malloc(tx_bufferSize);
	
	// Variables
	tx_frame_length  = 0x00;
	rx_frame_length  = 0x00;
}

void CMBEventsHandler::FreeBuffers()
{
	free(rx_buffer);
	free(tx_buffer);
}

// local methods
void CMBEventsHandler::FillRegDataTx(uint8_t addr, uint8_t* data, uint8_t length)
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

void CMBEventsHandler::FillRegDataRq(uint8_t addr, uint8_t length)
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

void CMBEventsHandler::FillVarDataTx(uint16_t addr, uint16_t* data, uint16_t length)
{
	// Frame header
	tx_buffer[0] = FRAMEHEADER_LOWBYTE;
	tx_buffer[1] = FRAMEHEADER_HIGHBYTE;
		// Frame size
#ifdef USE_CRC
	tx_buffer[2] = length + 5; //CMD (1) + DATA(length * 2) + CRC(2) + ADDR(2)
#else
	tx_buffer[2] = length + 3; //CMD (1) + DATA(length * 2) + ADDR(2)
#endif
	tx_buffer[3] = 0x82;	// Frame command
	tx_buffer[4] = addr >> 8;
	tx_buffer[5] = addr & 0xff;
	tx_frame_length = length + 6;
		
	// Frame data
	for (uint16_t i = 0; i < length / 2; i++)
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

void CMBEventsHandler::FillVarDataRq(uint16_t addr, uint16_t length)
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
	tx_buffer[6] = length;				// Data
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