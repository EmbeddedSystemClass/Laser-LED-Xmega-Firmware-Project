/* 
* CDS18B20.h
*
* Created: 08.06.2016 13:09:56
* Author: TPU_2
*/


#ifndef __CDS18B20_H__
#define __CDS18B20_H__


class CDS18B20
{
public:
	CDS18B20();
	~CDS18B20();
	
	void Initialize();
	bool w1_find();
	void w1_sendcmd(unsigned char cmd);
	unsigned char w1_receive_byte();
	int temp_18b20();
}; //CDS18B20

#endif //__CDS18B20_H__
