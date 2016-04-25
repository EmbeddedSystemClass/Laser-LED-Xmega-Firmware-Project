/* 
* CSoundPlayer.h
*
* Created: 14.04.2016 10:34:58
* Author: TPU_2
*/


#ifndef __CSOUNDPLAYER_H__
#define __CSOUNDPLAYER_H__

class CSoundPlayer
{
public:
	CSoundPlayer();
	~CSoundPlayer();
	
	// Initialize
	void Initialize();

	// Speaker control
	void beep(uint16_t note, uint16_t duration);
	void SoundStart(uint16_t freq, uint16_t time, uint8_t volume);
	void SoundStop();
	
	// Play imperial march
	void Play();
protected:
	//static void beep(unsigned int note, unsigned int duration);
	void InitializeToneTimer();
	void InitializeDurationTimer();
private:
}; //CSoundPlayer

#endif //__CSOUNDPLAYER_H__
