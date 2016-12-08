
#ifndef _VTK_UDP_STACK_C5_IPC2_CMD_H
#define _VTK_UDP_STACK_C5_IPC2_CMD_H

#include "vtk_udp_stack_class.h"

#define	C5_IPC2_CMD_RCV_PORT	25001
#define C5_IPC2_CMD_TRS_PORT	25001

#define	VDP_PRINTF_C5_IPC2

#ifdef	VDP_PRINTF_C5_IPC2
#define	c5_ipc2_printf(fmt,...)	printf("[C5-IPC2]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)
#else
#define	c5_ipc2_printf(fmt,...)
#endif

typedef struct
{
	udp_comm_rt 		udp;				//	udp交互模板实例
	int 				send_cmd_sn;		//	发送命令包序列号
	send_sem_id_array	waitrsp_array;		// 	业务应答同步队列	
}c5_ipc2_instance_t;

// 初始化udp:25001的服务实例
int init_c5_ipc2_instance( void );

// 通过udp:25001端口发送数据，不等待业务应答
int api_udp_c5_ipc2_send_data( int target_ip, int cmd, const char* pbuf, unsigned int len );

// 通过udp:25001端口发送数据包后，并等待业务应答，得到业务应答的数据
int api_udp_c5_ipc2_send_req(int target_ip, int cmd, char* psbuf, int slen , char *prbuf, unsigned int *prlen);

// 接收到udp:25001端口数据包后给出的业务应答
int api_udp_c5_ipc2_send_rsp( int target_ip, int cmd, int id, const char* pbuf, unsigned int len );

// 接收到udp:25001端口数据包的回调函数 - 发送到survey处理
int api_udp_c5_ipc2_recv_callback( int target_ip, int cmd, int sn, char* pbuf, unsigned int len );

// 接收到udp:25001端口数据包的回调函数 - 发送到survey处理
int api_udp_transfer2_recv_callback( char* pbuf, unsigned int len );
int api_udp_transfer2_send_data2( int target_ip, const char* pbuf, unsigned int len );

#endif

