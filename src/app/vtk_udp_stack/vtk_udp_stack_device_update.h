
#ifndef _VTK_UDP_STACK_DEVICE_UPDATE_H
#define _VTK_UDP_STACK_DEVICE_UPDATE_H

#include "vtk_udp_stack_class_ext.h"

#define	VDP_PRINTF_DEVICE_UPDATE

#ifdef	VDP_PRINTF_DEVICE_UPDATE
#define	dev_update_printf(fmt,...)	printf("[DECVICE_UPDATE]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)
#else
#define	dev_update_printf(fmt,...)
#endif

#define DEVICE_SEARCH_MULTICAST_ADDR			"236.6.6.1"		// 组播地址
#define DEVICE_SEARCH_UDP_BOARDCAST_PORT		25007			// 组播播搜索所有设备

#define DEVICE_SEARCH_CMD_READ_REQ				0x9A01			// 设备查找命令请求
#define DEVICE_SEARCH_CMD_READ_RSP				0x9A02			// 设备查找命令应答
#define DEVICE_SEARCH_CMD_WRITE_REQ				0x9B01			// 设备更新命令请求
#define DEVICE_SEARCH_CMD_WRITE_RSP				0x9B02			// 设备更新命令应答


#pragma pack(1)

//ip head struct define
typedef struct
{
	char 	s;
	char 	w;
	char 	i;
	char 	t;
	char 	c;
	char 	h;
} device_search_head;

typedef struct
{
	unsigned short	cmd;
} device_search_cmd;

// 设备发现请求数据包
typedef struct
{
	char		req_source_zero[6];		// 保留6个字节0
	int			req_source_ip;			// 请求PC端的ip地址
	char		req_target_zero[6];		// 保留6个字节0
	int			req_target_ip_seg;		// 请求PC端的ip地址所在的网段
} device_search_read_req;

// 设备发现应答数据包
typedef struct
{
	char		req_source_zero[6];		// 保留6个字节0
	int			req_source_ip;			// 请求PC端的ip地址
	char		req_target_zero[6];		// 保留6个字节0
	int			rsp_target_ip;			// 应答设备的ip地址
	int			rsp_target_mask;		// 应答设备的mask掩码
	int			rsp_target_gateway;		// 应答设备的网关
	char		rsp_target_mac[6];		// 应答设备的mac地址
	char		rsp_target_sn[10];		// 应答设备的序列号（不确定？）
} device_search_read_rsp;

// 设备更新请求数据包
typedef struct
{
	int			req_target_old_ip;		// 指定设备的原ip地址
	int			req_target_new_ip;		// 指定设备的新ip地址
	int			req_target_new_mask;	// 指定设备的新mask掩码
	int			req_target_new_gateway;	// 指定设备的新网关
	char		req_target_new_mac[6];	// 指定设备的新mac
} device_search_write_req;

// 设备更新应答数据包
typedef struct
{
	int			rsp_target_old_ip;		// 应答设备的原ip地址
	int			rsp_target_new_ip;		// 应答设备的新ip地址
	int			rsp_target_new_mask;	// 应答设备的新mask掩码
	int			rsp_target_new_gateway;	// 应答设备的新网关
	char		rsp_target_new_mac[6];	// 应答设备的新mac
} device_search_write_rsp;

typedef struct
{
	device_search_head	head;
	device_search_cmd	cmd;
	union
	{
		char						dat[1];
		device_search_read_req		read_req;
		device_search_read_rsp		read_rsp;
		device_search_write_req		write_req;
		device_search_write_rsp		write_rsp;		
	}buf;
} device_search_package;


#pragma pack()

typedef struct
{
	udp_comm_rt 		udp;				//	udp交互模板实例
	send_sem_id_array	waitrsp_array;		// 	业务应答同步队列	
}device_update_instance_t;

// 初始化udp:25000的服务实例
int init_device_update_instance( void );

// 通过udp:25007端口发送数据
int api_udp_device_update_send_data( int target_ip, unsigned short cmd, const char* pbuf, unsigned int len );

// 通过udp:25007端口发送数据包后，并等待业务应答，得到业务应答的数据
int api_udp_device_update_send_req(int target_ip, unsigned short cmd, char* psbuf, int slen , char *prbuf, unsigned int *prlen);

// 接收到udp:25007端口数据包后给出的业务应答
int api_udp_device_update_send_rsp( int target_ip, unsigned short cmd, int id, const char* pbuf, unsigned int len );

// 接收到udp:25007端口数据包的回调函数
int api_udp_device_update_recv_callback( int target_ip, unsigned short cmd, int sn, char* pbuf, unsigned int len );

// 接收到udp:25007端口数据包的回调函数 - 发送到survey处理
int api_uddevice_update_recv_callback( int target_ip, unsigned short cmd, char* pbuf, unsigned int len );

#endif

