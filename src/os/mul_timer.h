

#ifndef _MUL_TIMER_H_
#define _MUL_TIMER_H_

#include <sys/time.h>

#define  SYNC_PROCESS		//czn_20160629

#define MAX_TIMER_CNT 			20
#define MUL_TIMER_BASE 			25		// 25ms		if 10ms, sometime lose the SIGALARM
#define MUL_TIMER_SECOND		(1000/MUL_TIMER_BASE)

typedef void (*timer_proc)(void);

typedef struct _timer_info
{
	int 		   	state;
	int 		   	interval;
	int 		   	counter;
	int				handle;
	timer_proc     	callback;
	pthread_mutex_t	lock;
}timer_info_t;

typedef struct _timer_manage
{
	timer_info_t* 	timer_handle[MAX_TIMER_CNT];
	int				timer_cnt;
		
    void (* old_sigfunc)(int);
    void (* new_sigfunc)(int);
    struct itimerval value, ovalue;
}timer_manage_t;

int init_mul_timer(void);
int destroy_mul_timer(void);

int add_a_timer(timer_info_t* ptimer, timer_proc callback, int interval );
int del_a_timer(timer_info_t* ptimer);


void set_a_timer_period( timer_info_t* pTimer, int Period );
void start_a_timer( timer_info_t* pTimer );
void stop_a_timer( timer_info_t* pTimer );

#endif /* _MUL_TIMER_H_ */

