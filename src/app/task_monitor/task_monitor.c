

#include "../vtk_udp_stack/vtk_udp_stack_c5_ipc_cmd.h"
#include "../task_debug_sbu/task_debug_sbu.h"
#include "../video_service/ip_video_cs_control.h"
#include "task_monitor.h"

monitor_sbu	one_monitor = {0};

//czn_20160614_s
int API_DS_Camera_On(void)
{
	char msg_buf[4];
	
	
	msg_buf[0] = 116;
	msg_buf[1] = 0;
	msg_buf[2] = 0;
	msg_buf[3] = 0;
	vdp_uart_send_data((char*)msg_buf,4);
	return 0;
}

int API_DS_Camera_Off(void)
{
	char msg_buf[4];
	
	
	msg_buf[0] = 116;
	msg_buf[1] = 0;
	msg_buf[2] = 1;
	msg_buf[3] = 0;
	vdp_uart_send_data((char*)msg_buf,4);
	return 0;
}
//czn_20160614_e

//czn_20160516
int ip_mon_command_process(UDP_MSG_TYPE *pUdpType)
{
	// 解析监视业务
	switch(pUdpType->cmd)
	{
		default:
			break;
	}
	return 0;
}

int get_monitor_state(void)
{
	return one_monitor.state;
}

// return: 0/ok, -1/state err, 1/start video service err
int open_monitor_local(void)
{
	int monitor_time;
	// 读取监视时间参数
	monitor_time = 100;

	if( one_monitor.state == MONITOR_IDLE )
	{
		// 启动设备
		// one_monitor.device_id
	
		if( API_VIDEO_S_SERVICE_TURN_ON_TEST(monitor_time) == 0 )
		{
			one_monitor.state = MONITOR_LOCAL;
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
		return -1;
}

// return: 0/ok, -1/state err, 1/start video service err
int open_monitor_remote( int ip )
{
	int monitor_time;
	// 读取监视时间参数
	monitor_time = 100;

	if( one_monitor.state == MONITOR_IDLE )
	{
		one_monitor.target_ip	= ip;	
	
		// 启动设备
		// one_monitor.device_id

		if( api_video_c_service_turn_on( ip_video_multicast, ip, 0, monitor_time ) == 0 )
		{
			one_monitor.state 		= MONITOR_REMOTE;
			one_monitor.target_ip	= ip;			
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
		return -1;
}

// return: 0/ok, -1/state err, 1/stop video service err
int close_monitor( void )
{
	if( one_monitor.state != MONITOR_IDLE )
	{
		if( one_monitor.state == MONITOR_LOCAL )
		{
			if( api_video_s_service_close() == 0 )
			{
				one_monitor.state = MONITOR_IDLE;
				return 0;
			}
			else
			{
				return 1;
			}			
		}
		else
		{
			// 关闭设备状态
			//VtkUnicastRsp.code
		
			if( api_video_c_service_turn_off() == 0 )
			{
				one_monitor.state = MONITOR_IDLE;
				return 0;
			}
			else
			{
				return 1;
			}						
		}
	}
	else
		return -1;
}

