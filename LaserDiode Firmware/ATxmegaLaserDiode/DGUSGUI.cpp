/*
 * DGUSGUI.cpp
 *
 * Created: 03.06.2016 11:24:02
 *  Author: TPU_2
 */ 

#include <stdint.h>

void ConvertData(void* dst, void* src, uint16_t size, uint16_t offset = 0)
{
	uint16_t  length = size / 2;
	uint16_t* source = (uint16_t*)src;
	uint16_t* dest = (uint16_t*)dst;
	
	// swap bytes in words
	/*for (uint16_t i = 0; i < length; i++)
		dest[(i + offset) % length] = swap(source[i]);*/
		
	for (uint16_t i = 0; i < size; i++)
		((uint8_t*)dst)[((i + offset) % size) ^ 1] = ((uint8_t*)src)[i];
}

uint16_t min(uint16_t x, uint16_t y)
{
	if (x < y)
		return x;
	else
		return y;
}

uint16_t max(uint16_t x, uint16_t y)
{
	if (x > y)
		return x;
	else
		return y;
}

uint16_t swap(uint16_t data)
{
	return (data >> 8) | (data << 8);
}

uint32_t swap32(uint32_t data)
{
	return ((data & 0xffff) << 16) | ((data & 0xffff0000) >> 16);
}
