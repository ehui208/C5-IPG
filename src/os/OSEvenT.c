
#include "OSEvenT.h"



void OS_E_Create( OS_E* pE )
{
	memset( pE, 0, sizeof(OS_E) );

    pthread_mutex_init( &pE->Emutex, NULL );
    pthread_cond_init( &pE->Econd, NULL );
}

void OS_SignalEvent( OS_TASK_EVENT Event, OS_E* pE ) 
{
	OS_TASK_EVENT EventMask;

	pthread_mutex_lock( &pE->Emutex );

	pE->Events = Event;

	if( pE->Estat == OS_TS_WAIT_EVENT )
	{
		EventMask = pE->EventMask;

		if( EventMask & pE->Events )
			pthread_cond_signal( &pE->Econd );
		else
			printf("no wait Events \n");
	}

	pthread_mutex_unlock( &pE->Emutex );
}

OS_TASK_EVENT OS_WaitEvent( OS_TASK_EVENT EventMask, OS_E* pE )
{
    OS_TASK_EVENT c;
    
    pthread_mutex_lock( &pE->Emutex );

    if( !(pE->Events & EventMask) )
    {   
        pE->EventMask = EventMask;
        pE->Estat     = OS_TS_WAIT_EVENT; 
        pthread_cond_wait( &pE->Econd, &pE->Emutex );
    }
    
    c  = pE->Events;
    pE->Events = 0;

    pthread_mutex_unlock( &pE->Emutex );

    return c;
}

OS_TASK_EVENT OS_WaitEventTimed( OS_TASK_EVENT EventMask, OS_TIME Timeout ,OS_E* pE ) 
{
    OS_TASK_EVENT c;
    struct timespec abstime;
    struct timeval now;
    int nsec;

    pthread_mutex_lock( &pE->Emutex );

    gettimeofday(&now, NULL);
    nsec = now.tv_usec * 1000 + (Timeout % 1000) * 1000000;
    abstime.tv_nsec = nsec % 1000000000;
    abstime.tv_sec = now.tv_sec + nsec / 1000000000 + Timeout / 1000;

    if( Timeout > 0 )
    {
        if( !(pE->Events & EventMask) )
        {
            pE->EventMask = EventMask;
            pE->Estat     = OS_TS_WAIT_EVENT;
            pthread_cond_timedwait( &pE->Econd, &pE->Emutex, &abstime );
        }
    }

    c  = pE->Events;
    pE->Events = 0;  /* Cast to unsigned long to avoid warning */

    pthread_mutex_unlock( &pE->Emutex );

    return c;
}

OS_TASK_EVENT OS_WaitSingleEvent( OS_TASK_EVENT EventMask, OS_E* pE )
{
	OS_TASK_EVENT c;

	pthread_mutex_lock( &pE->Emutex );

	if( !(pE->Events & EventMask) )
	{
		pE->EventMask = EventMask;
		pE->Estat	  = OS_TS_WAIT_EVENT;	
		pthread_cond_wait( &pE->Econd, &pE->Emutex );
	}

	c  = pE->Events & EventMask;
	pE->Events &= (OS_TASK_EVENT) ~(unsigned long)EventMask;

	pthread_mutex_unlock( &pE->Emutex );

	return c;	
}

OS_TASK_EVENT OS_WaitSingleEventTimed( OS_TASK_EVENT EventMask, OS_TIME Timeout, OS_E* pE )
{
    OS_TASK_EVENT c;
	struct timespec abstime;
	struct timeval now;
	int	nsec;

	pthread_mutex_lock( &pE->Emutex );

	gettimeofday(&now, NULL);
	nsec = now.tv_usec * 1000 + (Timeout % 1000) * 1000000;
	abstime.tv_nsec = nsec % 1000000000;
	abstime.tv_sec = now.tv_sec + nsec / 1000000000 + Timeout / 1000;

	if( Timeout > 0 )
	{
		if( !(pE->Events & EventMask) )
		{
			pE->EventMask = EventMask;
			pE->Estat     = OS_TS_WAIT_EVENT;
			pthread_cond_timedwait( &pE->Econd, &pE->Emutex, &abstime );
		}
	}

    c  = pE->Events & EventMask;
    pE->Events &= (OS_TASK_EVENT) ~(unsigned long)EventMask;  /* Cast to unsigned long to avoid warning */ 

    pthread_mutex_unlock( &pE->Emutex );

    return c;
}

