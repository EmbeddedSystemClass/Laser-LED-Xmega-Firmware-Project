/* 
* CDS18B20.h
*
* Created: 08.06.2016 13:09:56
* Author: TPU_2
*/


#ifndef __CDS18B20NEW_H__
#define __CDS18B20NEW_H__

typedef enum DS18B20_STATE_ENUM
{
	DS18B20_IDLE,
	
	// reset states
	DS18B20_FIND_0_START,	// start find cmd
	DS18B20_FIND_1_PD,		// 485 us
	DS18B20_FIND_2_PU,		// 65 us, sample after
	DS18B20_FIND_COMPL,		// complete find 480 us
	
	// send states
	DS18B20_SEND_0_START,
	DS18B20_SEND_1_SEND1,	// 2 us
	DS18B20_SEND_2_SEND1,	// 65 us
	DS18B20_SEND_1_SEND0,	// 65 us
	DS18B20_SEND_2_SEND0,	// 5 us
	DS18B20_SEND_COMPL,		// complete send
	
	// Receive states
	DS18B20_RECV_1_PD,		// 2 us
	DS18B20_RECV_2_PU,		// 10 us, sample after
	DS18B20_RECV_3_COMPLBIT,// complete bit 50 us
	DS18B20_RECV_COMPL		// complete receive
} DS18B20_STATE, *PDS18B20_STATE;

typedef enum SENSOR_STATE_ENUM
{
	SENSOR_1_FIND,
	SENSOR_2_SEND_CC,
	SENSOR_3_SEND_44,
	SENSOR_4_DELAY_750ms,
	SENSOR_5_FIND,
	SENSOR_6_SEND_CC,
	SENSOR_7_SEND_BE,
	SENSOR_8_RECV_16bit,
	
	// stop states
	SENSOR_NOTFIND,
	SENSOR_IDLE
} SENSOR_STATE, *PSENSOR_STATE;

class CDS18B20new
{
public:
	CDS18B20new();
	~CDS18B20new();
	
	void Initialize();
	/*bool w1_find();
	void w1_sendcmd(unsigned char cmd);
	unsigned char w1_receive_byte();
	int temp_18b20();*/
	
	void process_w1_find();
	void process_w1_send();
private:
	DS18B20_STATE	cmd_state;
	SENSOR_STATE	sen_state;
	uint8_t			tx_pos;
	uint8_t			tx_data;
	uint16_t		rx_data;
	bool			find;	// false if not
}; //CDS18B20new

#endif //__CDS18B20NEW_H__
