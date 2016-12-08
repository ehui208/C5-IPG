
#ifndef _EXAMPLE_H
#define _EXAMPLE_H

//#define	EXAMPLE_EANBLE

#ifdef 	EXAMPLE_EANBLE

#include "vtk_udp_stack_class.h"

#define EXAMPLE_RCV_PORT	29000
#define EXAMPLE_TRS_PORT	29001

#define	VDP_PRINTF_EXAMPLE

#ifdef	VDP_PRINTF_EXAMPLE
#define	ext_printf(fmt,...)	printf("[EXP]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)
#else
#define	ext_printf(fmt,...)
#endif

typedef enum
{
	exp_cmd_req,
	exp_cmd_rsp,
}example_cmd_type;

typedef enum
{
	exp_idle,
	exp_req,
	exp_rsp,		
	exp_end,		
}example_state;

typedef enum
{
	exp_none,
	exp_ok,
	exp_timeout,
	exp_no_rsp,
	exp_err,		
}example_msg_type;

typedef struct
{
	udp_comm_rt 			udp;				//	udp交互模板实例
	int 					send_cmd_sn;		//	发送命令包序列号
	send_sem_id_array		waitrsp_array;		// 	业务应答同步队列
	example_msg_type		msg;				// 	消息类型
	example_state 			state;				//	状态机
	
} example_instance_t;

typedef struct
{
	target_head				target;
	int						para1;
	int						para2;	
} example_req_pack;

typedef struct
{
	target_head				target;
	int						result;
} example_rsp_pack;

int init_example_instance( void );

example_msg_type example_req( int server_ip, int para1, int para2 );
example_msg_type example_rsp( int client_ip, example_req_pack* plinkreq, int result);

#endif

#endif

