
#ifndef _SUBSCRIBER_LIST_H
#define _SUBSCRIBER_LIST_H

#include <pthread.h>

// 订阅状态机
typedef enum
{
	IDLE,
	ACTIVATE_SREVICE,
	IN_SERVICE,
	DEACTIVATE_SERVICE,
} subscriber_state;

typedef struct	_subscriber_data_t
{
	int				reg_ip;			// 登记的客户端IP
	struct timeval	reg_tv;			// 登记的客户端申请时间 	
	int				reg_period;		// 登记的客户端申请播放时间 (s)
	int				reg_timer;		// 登记的客户端定时器
} subscriber_data;

#define MAX_SUBUSCRIBER_LIST_COUNT		50

typedef struct	_subscriber_list_t
{	
	subscriber_data		dat[MAX_SUBUSCRIBER_LIST_COUNT];
	int					counter;
    pthread_mutex_t 	lock;	
} subscriber_list;

int init_subscriber_list( void );
int activate_subscriber_list( int reg_ip, int reg_period );
int deactivate_subscriber_list( int reg_ip );
int get_total_subscriber_list(void);


#endif

