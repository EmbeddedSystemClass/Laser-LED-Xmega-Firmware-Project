/*
 * TimerDef.h
 *
 * Created: 10.06.2016 10:26:17
 *  Author: TPU_2
 */ 


#ifndef TIMERDEF_H_
#define TIMERDEF_H_


typedef enum TIMER_WAVEFORMGEN_MODE_ENUM
{
	WGM_Normal = 0,
	WGM_Frequency = 1,
	WGM_SingleSlopePWM = 3,
	WGM_DualSlopeTopEventPWM = 5,
	WGM_DualSlopeBothEventPWM = 6,
	WGM_DualSlopeBottomEventPWM = 7
} TIMER_WAVEFORMGEN_MODE, *PTIMER_WAVEFORMGEN_MODE;

typedef enum TIMER_CLOCKSELECT_ENUM
{
	CS_NoClock = 0,
	CS_NoPrescale = 1,
	CS_DIV2 = 2,
	CS_DIV4 = 3,
	CS_DIV8 = 4,
	CS_DIV64 = 5,
	CS_DIV256 = 6,
	CS_DIV1024 = 7,
	CS_EventChannel0 = 8,
	CS_EventChannel1 = 9,
	CS_EventChannel2 = 10,
	CS_EventChannel3 = 11,
	CS_EventChannel4 = 12,
	CS_EventChannel5 = 13,
	CS_EventChannel6 = 14,
	CS_EventChannel7 = 15,
} TIMER_CLOCKSELECT, *PTIMER_CLOCKSELECT;

typedef enum TIMER_CHANNELS_ENUM
{
	TIMER_CHANNEL_A	= 0x10,
	TIMER_CHANNEL_B	= 0x20,
	TIMER_CHANNEL_C	= 0x40,
	TIMER_CHANNEL_D	= 0x80
} TIMER_CHANNELS, *PTIMER_CHANNELS;


#endif /* TIMERDEF_H_ */