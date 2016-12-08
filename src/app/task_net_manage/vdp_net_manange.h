
#ifndef _VDP_NET_MANANGE_H
#define  _VDP_NET_MANANGE_H

#include "vdp_net_manange_process.h"
#include "../vtk_udp_stack/vtk_udp_stack_c5_ipc_cmd.h"

#define VDP_NET_MANANGE_MSG_DIP_SET_IP						0		//dip开关设置
#define VDP_NET_MANANGE_MSG_NET_TEST						1		//网络质量检测
#define VDP_NET_MANANGE_MSG_NET_RESET						2

#define VDP_NET_MANANGE_MSG_IPG_RESET_OK					80		// n329复位ok通告

#define VDP_NET_MANANGE_MSG_COMMON_LINK						100		// uart申请设备link
#define VDP_NET_MANANGE_MSG_IPG_LIST_UPDATE_REQ				101		// uart申请更新list申请
#define VDP_NET_MANANGE_MSG_IPG_LIST_READ_REQ				102		// uart申请读取list申请
#define VDP_NET_MANANGE_MSG_IPG_ONLINE_CHECK				103		// uart申请网络在线监测
#define VDP_NET_MANANGE_MSG_IPG_REPEAT_CHECK				104		// uart申请本机ip重复检查

#define VDP_NET_MANANGE_REMOTE_COMMON_LINK					110		// udp申请设备link
#define VDP_NET_MANANGE_REMOTE_IPG_LIST_UPDATE_REQ			111		// udp申请更新list申请
#define VDP_NET_MANANGE_REMOTE_IPG_ONLINE_CHECK				113		// udp申请网络在线监测
#define VDP_NET_MANANGE_REMOTE_IPG_REPEAT_CHECK				114		// udp申请本机ip重复检查


#pragma pack(1)

/*------------------dip 等参数设置-----------------------*/
typedef struct 
{
	VDP_MSG_HEAD	head;
	union 
	{
		unsigned char data;				// DIP开关设置时的参数
		unsigned char macadd[18];			// MAC设置参数
		unsigned char ipadd[18];			// IP地址设置参数
		unsigned char maskadd[18];		// 掩码设置参数
		unsigned char gwadd[18];			// 网关设置参数
	}msg_data;
} NetManange_Para_type;

/*-------------------网络质量检测------------------------*/
#define TIME_OUT				0
#define GOOD					1
#define GENERAL 					2
#define BAD 						3

typedef struct 
{
	VDP_MSG_HEAD	head;
	unsigned char 	net_state;			// 0/ 
} NetManange_net_test;

/*-------------------ipg 列表更新--------------------------*/
typedef struct 
{
	VDP_MSG_HEAD	head;
	int				ip;
	unsigned char		state;
} NetManange_ipg_list_update;

/*-------------------ipg 目标报告--------------------------*/
typedef struct 
{
	VDP_MSG_HEAD	head;
	int				asker;
	char 			ip[16];			// IP地址字符串，eg: [192.168.010.009\0]
	char 			mac[18];		// MAC设置参数，eg: [00:12:48:17:55:66\0]	
} NetManange_ipg_list_report;

/*-------------------ipg复位iok------------------------------*/
typedef struct 
{
	VDP_MSG_HEAD	head;
	unsigned char 	msg_data;			// reset ok msg data
} NetManange_reset_ok;

/*-------------------ipg设备连接-----------------------------*/
typedef struct 
{
	VDP_MSG_HEAD	head;
	
	int		  		targetIP;			//目标IP地址
	int		  		sourcetIP;			//源IP地址
	unsigned short	targetDevID;		//目标设备地址
	unsigned short	sourceDevID;		//源设备地址

	unsigned char		call_type;			// 呼叫类型
	
	//link result
	unsigned char		link_state;
	
} NetManange_Link_type;

/*--------------------ipg list记录读取-----------------------*/
#define IPG_NUM_ONE_PACKAGE	5
#define IPG_IP_STR_BUF_LEN		16
#define IPG_MAC_STR_BUF_LEN	18

typedef	struct 
{
	unsigned char		attr;			// ipg list 节点的属性值:   0/正常，1/ip 地址重复，2/mac地址重复, 3/ both repeat
	char 			ip[16];			// IP地址字符串，eg: [192.168.010.009\0]
	char 			mac[18];		// MAC设置参数，eg: [00:12:48:17:55:66\0]
}ipg_list_node;

typedef struct 
{
	VDP_MSG_HEAD	head;	
	unsigned char		list_max;		//ipg list 的总个数
	unsigned char		list_off;			//ipg list 当前传输包的偏移量
	unsigned char		list_cnt;			//ipg list 当前传输包的个数
	ipg_list_node		list_node[IPG_NUM_ONE_PACKAGE];		//ipg list 当前传输包数据区
} NetManange_ipg_list_read;

/*----------------------ipg 在线检测-------------------------*/
typedef struct 
{
	VDP_MSG_HEAD	head;	
	int		  		targetIP;			//目标IP地址
	unsigned char		online_state;		//在线状态
} NetManange_ipg_online_check;

/*----------------------ipg 重复检测-------------------------*/
typedef struct 
{
	VDP_MSG_HEAD	head;	
	int		  		targetIP;			//目标IP地址
	unsigned char		repeat_state;		//在线状态
} NetManange_ipg_repeat_check;

/*-----------------------------------------------------------*/

#pragma pack()

void vtk_TaskInit_net_manang(void);
void exit_vdp_net_manange_task(void);


unsigned char ResetNetWork( void );


/*******************************************************************************************
 * @fn:		API_net_manange_Udp_Link_Request
 *
 * @brief:	发送通用link指令到网络
 *
 * @param:  	target_ip 	- 目标ip地址
 * @param:  	tdev_id 		- 目标设备地址
 * @param:  	sdev_id 		- 源设备地址
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Udp_Link_Request( unsigned char call_type, int target_ip, unsigned char tdev_id, unsigned char sdev_id );

/*******************************************************************************************
 * @fn:		API_net_manange_Udp_Link_Reply
 *
 * @brief:	发送通用link回复指令到网络
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 		- 目标ip地址
 * @param:  	tdev_id 		- 目标设备地址
 * @param:  	sdev_id 		- 源设备地址
 * @param:  	state 		- 连接状态
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Udp_Link_Reply( unsigned char msg_id, unsigned char call_type,  int target_ip, unsigned char tdev_id, unsigned char sdev_id, unsigned char state );

/*******************************************************************************************
 * @fn:		API_net_manange_Uart_IPG_List_Read_Request
 *
 * @brief:	回复发送请求ipg list的数据
 *
 * @param:  	pNetManage - 请求方的消息
 *
 * @return: 	-1/err, other/state
*******************************************************************************************/
int API_net_manange_Uart_IPG_List_Read_Reply( NetManange_ipg_list_read* pNetManage );


/*******************************************************************************************
 * @fn:		API_net_manange_Udp_OnLine_Check_Reply
 *
 * @brief:	uart 回复设备检测在线
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 		- 目标ip地址
 * @param:  	state 		- 在线状态
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Uart_OnLine_Check_Reply( unsigned char msg_id, int target_ip, unsigned char state );

/*******************************************************************************************
 * @fn:		API_net_manange_Udp_OnLine_Check_Request
 *
 * @brief:	发送通用在线监测命令
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 		- 目标ip地址
 *
 * @return: 	-1/err, other/state
*******************************************************************************************/
int API_net_manange_Udp_OnLine_Check_Request( unsigned char msg_id,   int target_ip );

/*******************************************************************************************
 * @fn:		API_net_manange_Udp_OnLine_Check_Reply
 *
 * @brief:	发送通用link回复指令到网络
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 		- 目标ip地址
 * @param:  	state 		- 在线状态
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Udp_OnLine_Check_Reply( unsigned char msg_id, int target_ip, unsigned char state );

/*******************************************************************************************
 * @fn:		API_net_manange_Udp_OnLine_Check_Reply
 *
 * @brief:	uart 回复设备检测在线
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 		- 目标ip地址
 * @param:  	state 		- 在线状态
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Uart_IPG_Repeat_Reply( unsigned char msg_id, int target_ip, unsigned char state );

/*******************************************************************************************
 * @fn:		API_net_manange_Udp_IPG_Repeat_Request
 *
 * @brief:	发送ipg 重复在线检测
 *
 * @param:  	msg_id 		- 请求方的消息id
 *
 * @return: 	-1/err, other/state
*******************************************************************************************/
int API_net_manange_Udp_IPG_Repeat_Request( unsigned char msg_id );

/*******************************************************************************************
 * @fn:		API_net_manange_Udp_IPG_Repeat_Reply
 *
 * @brief:	回复ipg repeat
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 		- 目标ip地址
 * @param:  	state 		- 在线状态
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Udp_IPG_Repeat_Reply( unsigned char msg_id, int target_ip, unsigned char state );

/*******************************************************************************************
 * @fn:		API_net_manange_Uart_Net_Test_Reply
 *
 * @brief:	uart 回复设备检测在线
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	state 		- 网络质量
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Uart_Net_Test_Reply( unsigned char msg_id, unsigned char state );

#endif


