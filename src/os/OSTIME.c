
#include "OSTIME.h"

/*********************************************************************
*
*       OS_CreateTimer
*/
void OS_CreateTimer( OS_TIMER* pTimer, OS_TIMERROUTINE* Hook, OS_TIME Period )
{	
#ifndef MULTI_TIMER
	memset( pTimer, 0, sizeof(OS_TIMER) );
	
	pTimer->Period = Period;

	pTimer->Evp.sigev_value.sival_ptr	= &pTimer->TimerID;
	pTimer->Evp.sigev_notify			= SIGEV_SIGNAL;//SIGEV_THREAD;//SIGEV_SIGNAL;
//	pTimer->Evp.sigev_notify_function 	= &Hook;
//	pTimer->Evp.sigev_notify_attributes = NULL;
	pTimer->Evp.sigev_signo			= SIGUSR2;

//	pTimer->Act.sa_handler 			= Hook;
//	pTimer->Act.sa_flags 			= 0;

	//sigemptyset( &pTimer->Act.sa_mask );

	//if( sigaction(SIGUSR2, &pTimer->Act, NULL) == -1 )
	{
	//	perror("fail to sigaction");
	}
	
	signal( SIGUSR2, Hook );

	if( timer_create(CLOCK_REALTIME, &pTimer->Evp, &pTimer->TimerID) == -1 )
	{
		perror(" OS_CreateTimer fail to timer_create");
	}
#else
	add_a_timer(&(pTimer->timer),(timer_proc)Hook,(int)Period);
#endif
}

/*********************************************************************
*
*       OS_RetriggerTimer
*/
void OS_RetriggerTimer( OS_TIMER* pTimer ) 
{
#ifndef MULTI_TIMER
	if( !pTimer->Active ) 
	{
		pTimer->Active					= 1;
		pTimer->It.it_interval.tv_sec	= 0;
		pTimer->It.it_interval.tv_nsec 	= pTimer->Period * 1000000;
		pTimer->It.it_value.tv_sec		= 0;
		pTimer->It.it_value.tv_nsec 	= pTimer->Period * 1000000;
		
		if( timer_settime(pTimer->TimerID, 0, &pTimer->It, NULL) == -1 )
		{
			perror("beep_timer_settime fail to timer_settime");
		}
	}
#else
	start_a_timer(&pTimer->timer);
#endif
}

/*********************************************************************
*
*       OS_StartTimer
*
*       Always ends with a call of OS_RESTORE_I().
*       OS_RetriggerTimer() relies on this call !
*/
 void OS_StartTimer( OS_TIMER* pTimer ) 
 {
#ifndef MULTI_TIMER 
	OS_RetriggerTimer( pTimer );
#else
	OS_RetriggerTimer( pTimer );
#endif
}

/*********************************************************************
*
*       OS_StopTimer()
*/
void OS_StopTimer( OS_TIMER* pTimer ) 
{
#ifndef MULTI_TIMER
	if( pTimer->Active ) 
	{
		pTimer->Active					= 0;
		pTimer->It.it_interval.tv_sec	= 0;
		pTimer->It.it_interval.tv_nsec 	= 0;
		pTimer->It.it_value.tv_sec		= 0;
		pTimer->It.it_value.tv_nsec 	= 0;
		
		if( timer_settime(pTimer->TimerID, 0, &pTimer->It, NULL) == -1 )
		{
			perror("beep_timer_settime fail to timer_settime");
		}
	}
#else
	stop_a_timer(&pTimer->timer);
#endif
}

/*********************************************************************
*
*       OS_SetTimerPeriod
*
**********************************************************************
*/
void OS_SetTimerPeriod( OS_TIMER* pTimer, OS_TIME Period )
{
#ifndef MULTI_TIMER
	pTimer->Period = Period;
#else
	set_a_timer_period(&pTimer->timer,Period);
#endif
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
void OS_DeleteTimer( OS_TIMER* pTimer )
{
#ifndef MULTI_TIMER
	timer_delete( &pTimer->TimerID );
#else
	del_a_timer(&pTimer->timer);
#endif
}


