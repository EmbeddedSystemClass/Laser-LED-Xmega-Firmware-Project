/* 
* CLaserBoard.h
*
* Created: 09.04.2016 15:23:56
* Author: Vladislav
*/


#ifndef __CLASERBOARD_H__
#define __CLASERBOARD_H__

#define F_CPU 32000000

class CLaserBoard
{
//functions
public:
	CLaserBoard();
	~CLaserBoard();
	
	void InitializeIO();
	void InitializeClock();
protected:
private:

}; //CLaserBoard

#endif //__CLASERBOARD_H__
