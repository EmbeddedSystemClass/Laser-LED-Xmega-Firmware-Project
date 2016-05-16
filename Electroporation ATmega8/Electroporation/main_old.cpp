/*
 * Electroporation.cpp
 *
 * Created: 22.03.2016 15:11:04
 * Author : TPU_2
 */ 

#define F_CPU	16000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "CDGUSUSART.h"
#include "CRelayTimer.h"
#include "CSensorADC.h"
#include "LaserLib/CUSART.h"
#include "LaserLib/sys_interrupts.h"
#include "MODBUS/CMBProtocol.h"
#include "MODBUS/CMBSender.h"

#define REGISTER_ADDR_PICID		0x03
#define VARIABLE_ADDR_MIN		0x1000
#define VARIABLE_ADDR_SEC		0x2000
#define VARIABLE_ADDR_MSC		0x3000
#define VARIABLE_ADDR_PWR		0x4000
#define VARIABLE_ADDR_BAR		0x5000

// Global variables
CDGUSUSART	usart;
CMBSender	sender;
CRelayTimer relayTimer;
CSensorADC	adc;

uint16_t swap(uint16_t data)
{
	return (data >> 8) | (data << 8);
}

// Initialize GPIO
void Init_Relay()
{
	PORTC = 0;
	DDRC = (1 << PC1);
}

// Relay
void Relay_On()
{
	PORTC |= (1 << PC1);
}

void Relay_Off()
{
	PORTC &= ~(1 << PC1);;
}

void SPI_MasterInit()
{
	DDRB = (1 << PB3) | (1 << PB2) | (1 << PB5);
	PORTB = 0;
	
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA) | (1 << SPR1) | (1 << SPR0) /*| (1 << DORD)*/;
}

void SPI_MasterTransmit(char data)
{
	/* Start transmission */
	SPDR = data;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
}

class CLaserControllerApp : public CMBEventsHandler
{
public :
	// CMBEventsHandler
	void OnTransactionCallback(uint8_t* data, uint16_t length)
	{
		//no code
	}
	
	void OnVariableReceived(uint16_t addr, uint16_t* data, uint16_t length)
	{
		uint16_t val = swap(*((uint16_t*)data));
		if (addr == 0x0010)	{MinVal = val; MinValSet = val;}
		if (addr == 0x0020)	{SecVal = val; SecValSet = val;}
		if (addr == 0x0030)	mScVal = val;
		if (addr == 0x0040)	PwrVal = val;
		if (addr == 0x0050)	BarVal = val;
		if (addr == 0x0000)	H_L = val;
	}
	
	void OnRegisterReceived(uint8_t addr, uint8_t* data, uint8_t length)
	{
		if (addr == 0x03)	PIC_ID = data[1];
	}
	
	// Initialization
	void Initialize(CMBSender* sender)
	{
		m_cpSender = sender;
	}
	
	void Start()
	{
		uint16_t pic_id = swap(0);
		m_cpSender->WriteDataToRegister(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
		m_cpSender->WaitMODBUSTransmitter();
		MinVal = 10;
		MinValSet = MinVal;
		m_cpSender->WriteDataToSRAM(VARIABLE_ADDR_MIN, (uint16_t*)&MinVal, 1);
		m_cpSender->WaitMODBUSTransmitter();
		SecVal = 0;
		SecValSet = SecVal;
		m_cpSender->WriteDataToSRAM(VARIABLE_ADDR_SEC, (uint16_t*)&SecVal, 1);
		m_cpSender->WaitMODBUSTransmitter();
		PwrVal = 0;
		m_cpSender->WriteDataToSRAM(VARIABLE_ADDR_PWR, (uint16_t*)&PwrVal, 1);
		m_cpSender->WaitMODBUSTransmitter();
		stop = false;
	}
	
	// Timer
	bool ProcessTimer()
	{
		bool res = false;
		if (mScVal == 0)
		{
			mScVal = 100;
			if (SecVal == 0)
			{
				SecVal = 60;
				if (MinVal == 0)
				{
					res = true;
					MinVal = 60;
				}
				MinVal--;
			}
			SecVal--;
		}
		mScVal--;
		return res;
	}
	
	// Static method callback
	static void ProcessTimerCallback(void* owner)
	{
		CLaserControllerApp* app = (CLaserControllerApp*)owner;
		
		static uint8_t cnt = 0;
		if ((cnt++) % 10 == 0)
		{
			if (app->ProcessTimer())
				app->OnTimerStop();
		}
	}
	
	// Process
	void Run()
	{
		// Get PIC ID
		m_cpSender->StartMODBUSRegisterTransaction(REGISTER_ADDR_PICID, 2);
		m_cpSender->WaitMODBUSListener();

		if (PIC_ID == 0)
		{
			if (stop)
			{
				MinVal = MinValSet;
				m_cpSender->WriteDataToSRAM(VARIABLE_ADDR_MIN, (uint16_t*)&MinVal, 1);
				m_cpSender->WaitMODBUSTransmitter();
				SecVal = SecValSet;
				m_cpSender->WriteDataToSRAM(VARIABLE_ADDR_SEC, (uint16_t*)&SecVal, 1);
				m_cpSender->WaitMODBUSTransmitter();
				
				relayTimer.Stop();
				
				stop = false;
			}
			
			// Get data variables
			_delay_ms(50);
			m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_MIN, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WaitMODBUSListener();
			_delay_ms(50);
			m_cpSender->StartMODBUSVariableTransaction(VARIABLE_ADDR_SEC, 2);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WaitMODBUSListener();
			_delay_ms(50);
			
			Relay_Off();
		}
		
		if (PIC_ID == 2)
		{
			// Show timer
			relayTimer.Start((uint8_t)0);
			stop = true;
			
			PwrVal = (adc.GetValue() * 25 / 256);	// value * 100 / 1024
			BarVal = (adc.GetValue() * 25 / 1024);	// value * 25 / 1024
			
			m_cpSender->WriteDataToSRAM(VARIABLE_ADDR_MIN, (uint16_t*)&MinVal, 1);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAM(VARIABLE_ADDR_SEC, (uint16_t*)&SecVal, 1);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAM(VARIABLE_ADDR_MSC, (uint16_t*)&mScVal, 1);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAM(VARIABLE_ADDR_PWR, (uint16_t*)&PwrVal, 1);
			m_cpSender->WaitMODBUSTransmitter();
			m_cpSender->WriteDataToSRAM(VARIABLE_ADDR_BAR, (uint16_t*)&BarVal, 1);
			m_cpSender->WaitMODBUSTransmitter();
			
			Relay_On();
		}
	}
	
	void OnTimerStop()
	{
		uint16_t pic_id = swap(0);
		m_cpSender->WriteDataToRegister(REGISTER_ADDR_PICID, (uint8_t*)&pic_id, 2);
		//m_cpSender->WaitMODBUSTransmitter();
	}
	
private :
	// Variables
	volatile uint16_t MinVal;
	volatile uint16_t SecVal;
	volatile uint16_t MinValSet;
	volatile uint16_t SecValSet;
	volatile uint16_t mScVal;
	volatile uint16_t PwrVal;
	volatile uint16_t BarVal;
	volatile uint16_t H_L;
	bool     stop;
	
	// Registers
	uint8_t PIC_ID;
	
	// Modules
	CMBSender* m_cpSender;
};

// Global variables
CLaserControllerApp App;

void Initialize()
{
	Init_Relay();
	
	// Initialize USART
	usart.Initialize(BAUD_115200, DISABLE, STOP_1BIT, true);
	
	// Initialize timer
	relayTimer.Initialize(WGM_CTC, COM_Clear, CS_DIV64, false);
	relayTimer.AsyncMode(false);
	relayTimer.SetCOMPACallback(&CLaserControllerApp::ProcessTimerCallback, &App);
	relayTimer.SetCOMPA((uint8_t)250);
	
	// Initialize ADC
	adc.Initialize(REF_AVCC, ADC_CHANNEL0, ADC_PSCL_DIV128, false, true);
	adc.Start();
	
	// Construct MODBUS protocol module
	sender.Initialize(&usart, &App, 32, 32);
	
	// Initialize application module
	App.Initialize(&sender);
	
	// Global interrupt enable
	sei();
}

int main(void)
{
	SPI_MasterInit();
	Initialize();
	
	Relay_Off();
	
	_delay_ms(1000);
	
	App.Start();
	
    /* Replace with your application code */
    while (1) 
    {			
		_delay_ms(1);
		
		static uint8_t prescale = 0;
		if ((prescale % 100) == 0)
			App.Run();
		prescale++;
		
		/*static uint16_t cnt = 0;
		if (cnt > 4095) cnt = 0;
		
		PORTB |= (1 << PB2);
		
		SPI_MasterTransmit(~(uint8_t)(cnt >> 8));
		SPI_MasterTransmit(~(uint8_t)(cnt & 0xff));
		
		PORTB &= ~(1 << PB2);
		
		cnt++;*/
    }
}

