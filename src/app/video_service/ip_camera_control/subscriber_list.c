
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>      /* basic system data types */
#include <error.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#include "subscriber_list.h"
#include "video_multicast_server.h"

subscriber_list	server_userlist;

pthread_t server_userlist_monitor_pid;

int api_disable_video_server_multicast( int mcg_addr, short port );

void* server_userlist_monitor_process(void* arg)
{
	int i;
	// 定时查询user list
	while( 1 )
	{
		usleep(100000);
		pthread_mutex_lock( &server_userlist.lock );

		for( i = 0; i < server_userlist.counter; i++ )
		{
			server_userlist.dat[i].reg_timer += 100;
			if( server_userlist.dat[i].reg_timer >= (server_userlist.dat[i].reg_period*1000) )
			{
				server_userlist.dat[i].reg_timer = 0;
				
				vd_printf("server_userlist_monitor_process deactivate one client ip = 0x%08x\n",server_userlist.dat[i].reg_ip);
				
				api_one_video_server_cancel_req(server_userlist.dat[i].reg_ip);				
				// 删除计时时间到的客户端
				server_userlist.dat[i].reg_tv		= server_userlist.dat[server_userlist.counter-1].reg_tv;
				server_userlist.dat[i].reg_ip		= server_userlist.dat[server_userlist.counter-1].reg_ip;
				server_userlist.dat[i].reg_period	= server_userlist.dat[server_userlist.counter-1].reg_period;
				server_userlist.dat[i].reg_timer	= server_userlist.dat[server_userlist.counter-1].reg_timer;
				server_userlist.counter--;
				//无user则关闭视频服务
				if( server_userlist.counter == 0 )
				{				
					int		mcg_addr;
					short	port;					
					if( get_server_multicast_addr_port( &mcg_addr, &port ) == 0 )
					{
						// 调用关闭指定客户端视频组播输出命令				
						api_disable_video_server_multicast( mcg_addr, port );
					}
				}					
			}
		}		
		pthread_mutex_unlock( &server_userlist.lock );
	}
	return 0;
}

int init_subscriber_list( void )
{
	pthread_mutex_lock( &server_userlist.lock );	
	server_userlist.counter	= 0;
	pthread_mutex_unlock( &server_userlist.lock );
	
	if( pthread_create(&server_userlist_monitor_pid, 0, server_userlist_monitor_process, 0) )
	{
		return -1;
	}
	vd_printf("init_subscriber_list ok...\n");
	return 0;
}

int activate_subscriber_list( int reg_ip, int reg_period )
{
	struct timeval tv;	
	int i;

	pthread_mutex_lock( &server_userlist.lock );
	
	if( server_userlist.counter >= MAX_SUBUSCRIBER_LIST_COUNT )
	{
		pthread_mutex_unlock( &server_userlist.lock );	
		return -1;
	}
	
	for( i = 0; i < server_userlist.counter; i++ )
	{
		if( server_userlist.dat[i].reg_ip == reg_ip )
		{
			pthread_mutex_unlock( &server_userlist.lock );	
			return -1;
		}
	}
	
	gettimeofday(&tv, 0);
	server_userlist.dat[server_userlist.counter].reg_tv		= tv;
	server_userlist.dat[server_userlist.counter].reg_ip		= reg_ip;
	server_userlist.dat[server_userlist.counter].reg_period	= reg_period;
	server_userlist.dat[server_userlist.counter].reg_timer	= 0;

	server_userlist.counter++;
	i = server_userlist.counter;

	pthread_mutex_unlock( &server_userlist.lock );	
	
	return i;
	
}

int deactivate_subscriber_list( int reg_ip )
{
	int i;
	
	pthread_mutex_lock( &server_userlist.lock );	
	
	for( i = 0; i < server_userlist.counter; i++ )
	{
		if( server_userlist.dat[i].reg_ip == reg_ip )
		{
			server_userlist.dat[i].reg_tv 		= server_userlist.dat[server_userlist.counter-1].reg_tv;
			server_userlist.dat[i].reg_ip 		= server_userlist.dat[server_userlist.counter-1].reg_ip;
			server_userlist.dat[i].reg_period	= server_userlist.dat[server_userlist.counter-1].reg_period;
			server_userlist.dat[i].reg_timer	= server_userlist.dat[server_userlist.counter-1].reg_timer;
			server_userlist.counter--;
			break;
		}
	}
	i = server_userlist.counter;
	
	pthread_mutex_unlock( &server_userlist.lock );	
	
	return i;
}

int get_total_subscriber_list(void)
{
	int i;
	
	pthread_mutex_lock( &server_userlist.lock );	
	
	i = server_userlist.counter;
	
	pthread_mutex_unlock( &server_userlist.lock );	

	return i;
}
