#ifndef OSTIME_H
#define OSTIME_H

#include "RTOS.h"

#include "mul_timer.h"
#define MULTI_TIMER

/**********************************************************************
*
*       OS_TIMER
*/
typedef struct OS_timer OS_TIMER;
struct OS_timer 
{
#ifndef MULTI_TIMER
	OS_TIMER*			pNext;
	OS_TIME				Period;
	char    			Active;
	timer_t 			TimerID;	
	struct sigevent 	Evp;	
	struct itimerspec 	It;
	struct sigaction 	Act;
#else
	timer_info_t		timer;
#endif
};

typedef void OS_TIMERROUTINE( void );


void OS_CreateTimer( OS_TIMER* pTimer, OS_TIMERROUTINE* Hook, OS_TIME Period );
void OS_RetriggerTimer( OS_TIMER* pTimer );
void OS_StartTimer( OS_TIMER* pTimer );
void OS_StopTimer( OS_TIMER* pTimer );
void OS_SetTimerPeriod( OS_TIMER* pTimer, OS_TIME Period );



#endif
