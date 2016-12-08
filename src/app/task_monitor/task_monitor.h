

#ifndef _TASK_MONITOR_
#define _TASK_MONITOR_

#include "../task_survey/task_survey.h"

typedef enum
{
	MONITOR_IDLE,
	MONITOR_LOCAL,
	MONITOR_REMOTE,
} MONITOR_STATE;


typedef struct
{
	MONITOR_STATE	state;				// 状态
	int				target_ip;			// 远程监视ip
	int				device_id;			// 远程设备号
	int				period;				// 监视时间	
} monitor_sbu;

int open_monitor_local( void );
int open_monitor_remote( int ip );
int close_monitor( void );

#endif

