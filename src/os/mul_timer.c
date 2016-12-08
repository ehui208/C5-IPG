
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h> 
#include "mul_timer.h"

static timer_manage_t timer_manage;
static void signal_func(int signo);

//czn_20160629_s
#ifdef SYNC_PROCESS
pthread_t task_Timer;
 
void vtk_TaskProcessEvent_Timer(void)
{
	//Loop_vdp_common_buffer	hal_timer;
	//p_vdp_common_buffer pdb 	= 0;
	//init_vdp_common_queue(&hal_timer, 100,NULL, NULL);
	sigset_t waisig;
	int sig;
	
	
	setitimer(ITIMER_REAL, &timer_manage.value, &timer_manage.ovalue);

	
	while(1)
	{
		sigemptyset(&waisig);
		sigaddset(&waisig,SIGALRM);
		 //pop_vdp_common_queue(&hal_timer, &pdb, 25) ;
		 sig = sigwaitinfo(&waisig,NULL);
		 if(sig == SIGALRM)
		 {
		 	signal_func(sig);
		 }
		 
	}
}
#endif

int init_mul_timer(void)
{
#ifndef SYNC_PROCESS
     int ret;
     
     memset(&timer_manage, 0, sizeof(timer_manage_t));
	 
     if( (timer_manage.old_sigfunc = signal(SIGALRM, signal_func)) == SIG_ERR )
     {
         return (-1);
     }
	 
     timer_manage.new_sigfunc = signal_func;
     
     timer_manage.value.it_value.tv_sec 	= 0;
     timer_manage.value.it_value.tv_usec 	= MUL_TIMER_BASE*1000;
     timer_manage.value.it_interval.tv_sec 	= 0;
     timer_manage.value.it_interval.tv_usec = MUL_TIMER_BASE*1000;
	 
     ret = setitimer(ITIMER_REAL, &timer_manage.value, &timer_manage.ovalue); 
     
     return (ret);
#else
	memset(&timer_manage, 0, sizeof(timer_manage_t));
	 
	timer_manage.new_sigfunc = signal_func;

	timer_manage.value.it_value.tv_sec 	= 0;
	timer_manage.value.it_value.tv_usec 	= MUL_TIMER_BASE*1000;
	timer_manage.value.it_interval.tv_sec 	= 0;
	timer_manage.value.it_interval.tv_usec = MUL_TIMER_BASE*1000;

	// ret = setitimer(ITIMER_REAL, &timer_manage.value, &timer_manage.ovalue); 
	if( pthread_create(&task_Timer,NULL,(void*)vtk_TaskProcessEvent_Timer,NULL) != 0 )
	{
		printf( "Create pthread task_Timer error! \n" );
		exit(1);
	}
	return (0);
#endif
}
//czn_20160629_e

int destroy_mul_timer(void)
{
     int ret;
     
     if( (signal(SIGALRM, timer_manage.old_sigfunc)) == SIG_ERR)
     {
         return (-1);
     }

     ret = setitimer(ITIMER_REAL, &timer_manage.ovalue, &timer_manage.value);
     if(ret < 0)
     {
         return (-1);
     } 
     memset(&timer_manage, 0, sizeof(struct _timer_manage));
     
     return(0);
}


int add_a_timer(timer_info_t* ptimer, timer_proc callback, int interval )
{
	if( ptimer == NULL || callback == NULL || interval <= 0 )
		return -1;

	if( timer_manage.timer_cnt >= MAX_TIMER_CNT )
		return -1;
	
    pthread_mutex_init( &ptimer->lock, 0 );

	ptimer->state		= 0;
	ptimer->counter		= 0;
	ptimer->interval	= interval;
	ptimer->callback	= callback;
	ptimer->handle		= timer_manage.timer_cnt;
	
	timer_manage.timer_handle[timer_manage.timer_cnt] = ptimer;
	timer_manage.timer_cnt++;

	return timer_manage.timer_cnt;
}


/* success, return 0; failed, return -1 */
int del_a_timer(timer_info_t* ptimer)
{
     return (0);
}

static void signal_func(int signo)
{
	int i;
	timer_info_t* ptimer;	 
	for(i = 0; i < timer_manage.timer_cnt; i++)
	{
		ptimer = timer_manage.timer_handle[i];
		
		pthread_mutex_lock( &ptimer->lock );
			
		if( ptimer->state )
		{
			ptimer->counter++;
			if( ptimer->counter >= ptimer->interval )
			{
				ptimer->counter 	= 0;
				ptimer->state 	= 0;		//cao20151118
				pthread_mutex_unlock( &ptimer->lock );
				(*ptimer->callback)();
				continue;
			}
			else
			{
				pthread_mutex_unlock( &ptimer->lock );				
			}
		}
		pthread_mutex_unlock( &ptimer->lock );
	}
}

void set_a_timer_period( timer_info_t* pTimer, int Period )
{
	pthread_mutex_lock( &pTimer->lock );
	pTimer->interval = Period;
	pthread_mutex_unlock( &pTimer->lock );
}

void start_a_timer( timer_info_t* pTimer ) 
{
	pthread_mutex_lock( &pTimer->lock );
	pTimer->counter	= 0;
	pTimer->state 	= 1;
	pthread_mutex_unlock( &pTimer->lock );
}

void stop_a_timer( timer_info_t* pTimer )
{
	pthread_mutex_lock( &pTimer->lock );
	pTimer->counter	= 0;
	pTimer->state = 0;	
	pthread_mutex_unlock( &pTimer->lock );
}


