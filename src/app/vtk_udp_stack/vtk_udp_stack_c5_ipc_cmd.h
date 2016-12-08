
#ifndef _VTK_UDP_STACK_C5_IPC_CMD_H
#define _VTK_UDP_STACK_C5_IPC_CMD_H

#include "vtk_udp_stack_class.h"

#define	C5_IPC_CMD_RCV_PORT	25000
#define C5_IPC_CMD_TRS_PORT	25000

#define	VDP_PRINTF_C5_IPC

#ifdef	VDP_PRINTF_C5_IPC
#define	c5_ipc_printf(fmt,...)	printf("[C5-IPC]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)
#else
#define	c5_ipc_printf(fmt,...)
#endif

#define CMD_CALL_MON_LINK_REQ			0x1001
#define CMD_CALL_MON_LINK_RSP			0x1081

#define CMD_CALL_MON_UNLINK_REQ			0x1002
#define CMD_CALL_MON_UNLINK_RSP			0x1082

#define CMD_NET_MANAGE_REQ				0x8001		// 网络管理命令请求
#define CMD_NET_MANAGE_RSP				0x8081		// 网络管理命令应答

// 摄像机调节指令
#define CMD_CAM_REMOTE_ADJUST_REQ		0x0010		// 远程调节摄像头申请
#define CMD_CAM_REMOTE_ADJUST_RSP		0x0090		// 远程调节摄像头应答

// lzh_20160622_s
#define CMD_DEVICE_TYPE_CODE_VER_REQ		0x9001		// 远程查询设备类型，及code版本信息
#define CMD_DEVICE_TYPE_CODE_VER_RSP		0x9081		// 远程查回复设备类型，及code版本信息

#define CMD_DEVICE_TYPE_DOWNLOAD_START_REQ	0x9002		// 
#define CMD_DEVICE_TYPE_DOWNLOAD_START_RSP	0x9082		// 

#define CMD_DEVICE_TYPE_DOWNLOAD_STOP_REQ	0x9003		// 
#define CMD_DEVICE_TYPE_DOWNLOAD_STOP_RSP	0x9083		// 

#define CMD_DEVICE_TYPE_DOWNLOAD_VERIFY_REQ	0x9004		// 
#define CMD_DEVICE_TYPE_DOWNLOAD_VERIFY_RSP	0x9084		// 

#define CMD_DEVICE_TYPE_UPDATE_START_REQ	0x9005		// 
#define CMD_DEVICE_TYPE_UPDATE_START_RSP	0x9085		// 

#define CMD_DEVICE_TYPE_UPDATE_OVER_REQ		0x9006		// 
#define CMD_DEVICE_TYPE_UPDATE_OVER_RSP		0x9086		// 
// lzh_20160622_e

typedef struct
{
	udp_comm_rt 		udp;				//	udp交互模板实例
	int 				send_cmd_sn;		//	发送命令包序列号
	send_sem_id_array	waitrsp_array;		// 	业务应答同步队列	
}c5_ipc_instance_t;

// 初始化udp:25000的服务实例
int init_c5_ipc_instance( void );

// 通过udp:25000端口发送数据，不等待业务应答
int api_udp_c5_ipc_send_data( int target_ip, int cmd, const char* pbuf, unsigned int len );

// 通过udp:25000端口发送数据包后，并等待业务应答，得到业务应答的数据
int api_udp_c5_ipc_send_req(int target_ip, int cmd, char* psbuf, int slen , char *prbuf, unsigned int *prlen);

// 接收到udp:25000端口数据包后给出的业务应答
int api_udp_c5_ipc_send_rsp( int target_ip, int cmd, int id, const char* pbuf, unsigned int len );

// 接收到udp:25000端口数据包的回调函数 - 发送到survey处理
int api_udp_c5_ipc_recv_callback( int target_ip, int cmd, int sn, char* pbuf, unsigned int len );

// 接收到udp:25000端口数据包的回调函数 - 发送到survey处理
int api_udp_transfer_recv_callback( char* pbuf, unsigned int len );
int api_udp_transfer_send_data( int target_ip, const char* pbuf, unsigned int len );

#endif

