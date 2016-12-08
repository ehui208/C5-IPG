#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include "obj_multi_timer.h"

//-------------------------------------------------------------------------------------------
OS_TIMER t_power_down;

void Power_Down_Timer_Process(void)
{
	OS_StopTimer(&t_power_down);
}

int Power_Down_Timer_Init(void)
{
	OS_CreateTimer( &t_power_down, Power_Down_Timer_Process, 100 );	
	return 0;
}

int Power_Down_Timer_Set( int sec )
{
	OS_SetTimerPeriod( &t_power_down, sec*MUL_TIMER_SECOND );
	OS_RetriggerTimer( &t_power_down );	
	return 0;
}

/********************************************************************************************/
//initial all timer
/********************************************************************************************/
void init_all_timer(void)
{
	init_mul_timer();
	Power_Down_Timer_Init();
}


