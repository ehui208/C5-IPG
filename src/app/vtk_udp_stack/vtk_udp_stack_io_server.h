
#ifndef _VTK_UDP_STACK_IO_SERVER_H
#define _VTK_UDP_STACK_IO_SERVER_H

#include "vtk_udp_stack_class_ext.h"

#define	VDP_PRINTF_IO_SERVER

#ifdef	VDP_PRINTF_IO_SERVER
#define	udp_io_server_printf(fmt,...)	printf("[udp_io_server]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)
#else
#define	udp_io_server_printf(fmt,...)
#endif

#define UDP_IO_SERVER_UDP_PORT					25008			// IO server 通信端口

#define UDP_IO_SERVER_CMD_RW				0				// IO server 通信指令
#define UDP_IO_SERVER_CMD_RW_RSP		1				// IO server 通信指令

typedef struct
{
	udp_comm_rt 		udp;				//	udp交互模板实例
	int 				send_cmd_sn;		//	发送命令包序列号
	send_sem_id_array	waitrsp_array;		// 	业务应答同步队列	
}udp_io_server_instance_t;


// 通过udp:25008端口发送数据，不等待业务应答
int api_udp_io_server_send_data( int target_ip, int cmd, const char* pbuf, unsigned int len );

// 通过udp:25008端口发送数据包后，并等待业务应答，得到业务应答的数据
int api_udp_io_server_send_req(int target_ip, int cmd, char* psbuf, int slen , char *prbuf, unsigned int *prlen);

// 接收到udp:25008端口数据包后给出的业务应答
int api_udp_io_server_send_rsp( int target_ip, int cmd, int id, const char* pbuf, unsigned int len );

// 接收到udp:25008端口数据包的回调函数 - 发送到survey处理
int api_udp_io_serverc_recv_callback( int target_ip, int cmd, int sn, char* pbuf, unsigned int len );

#endif

