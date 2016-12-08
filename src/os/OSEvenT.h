
#ifndef OSEVENT_H
#define OSEVENT_H

#include "RTOS.h"

#define OS_TASK_EVENT		OS_U8
#define OS_TS_WAIT_EVENT	1

typedef struct OS_E_STRUCT OS_E;
struct OS_E_STRUCT
{
	pthread_mutex_t Emutex;
	pthread_cond_t  Econd;
	OS_TASK_EVENT	Events;
	OS_TASK_EVENT	EventMask;
	OS_STAT			Estat;
};

void OS_E_Create( OS_E* pE );
void OS_SignalEvent( OS_TASK_EVENT Event, OS_E* pE );
OS_TASK_EVENT OS_WaitEvent( OS_TASK_EVENT EventMask, OS_E* pE );
OS_TASK_EVENT OS_WaitEventTimed( OS_TASK_EVENT EventMask, OS_TIME Timeout ,OS_E* pE );
OS_TASK_EVENT OS_WaitSingleEvent( OS_TASK_EVENT EventMask, OS_E* pE );
OS_TASK_EVENT OS_WaitSingleEventTimed( OS_TASK_EVENT EventMask, OS_TIME Timeout, OS_E* pE );

#endif
