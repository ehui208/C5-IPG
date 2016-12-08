
#ifndef _VIDEO_LINPHONE_LINKER_H
#define _VIDEO_LINPHONE_LINKER_H

#include "../vtk_udp_stack/vtk_udp_stack_class.h"

// linphone call接收端口号
#define LINPHONE_STARTER_RECV_PORT		28002

typedef enum
{
	VD_LINPHONE_IDLE,
	VD_LINPHONE_CLIENT,
	VD_LINPHONE_SERVER,
} vd_linphone_state;

typedef enum
{
	//request
	VD_LINPHONE_MSG_REQ_NONE,
	VD_LINPHONE_MSG_REQ_BUSY,
	VD_LINPHONE_MSG_REQ_NO_RSP,
	VD_LINPHONE_MSG_REQ_OK,
	VD_LINPHONE_MSG_REQ_UNALLOW,
	VD_LINPHONE_MSG_REQ_TIMEOUT,
	//close
	VD_LINPHONE_MSG_CLOSE_NONE,
	VD_LINPHONE_MSG_CLOSE_ERR,
	VD_LINPHONE_MSG_CLOSE_NO_RSP,
	VD_LINPHONE_MSG_CLOSE_OK,
	VD_LINPHONE_MSG_CLOSE_TIMEOUT,
} vd_linphone_msg_type;

typedef struct _video_linphone_linker_t_
{
	udp_comm_rt 			udp;				//	udp交互模板实例
	vd_linphone_state 		state;				//	状态机
	vd_linphone_msg_type	msg;				//	最新返回消息
	int 					send_cmd_sn;		//	发送命令包序列号
	send_sem_id_array		waitrsp_array;		// 	业务应答同步队列
}video_linphone_linker;

// 呼叫交互命令定义
typedef enum
{
	LINPHONEC_ACK,					// 0 just communication ack, must be reserved
	// linphone link req couple
	LINPHONE_LINK_REQ,				//  client 	-> server
	LINPHONE_LINK_RSP,				//  server 	-> client
	
	// linphone quit req couple
	LINPHONE_QUIT_REQ,				//	server	-> client	
	LINPHONE_QUIT_RSP,				//	client	-> server
	
} vd_linphone_cmd;

// linphone服务器端response回复的result类型
typedef enum
{
	VD_LINPHONE_RSP_RESULT_NONE,		// 无效
	VD_LINPHONE_RSP_RESULT_ENABLE,		// 允许
	VD_LINPHONE_RSP_RESULT_DISABLE,		// 不允许
} vd_linphone_rsp_result;

// 客户端申请的数据包格式
typedef struct
{
	target_head				target;
	int						vd_direct;	// 视频输入输出方向
	int						auto_talk;	// 呼叫自动应答
} vd_linphonec_linker_req_pack;

// 服务器端响应客户端的申请数据包格式
typedef struct
{
	target_head				target;
	int						result;		// 申请的回复结果
} vd_linphonec_linker_rsp_pack;


int init_one_linphone_linker( void );

vd_linphone_msg_type api_one_linphone_linker_req( int server_ip, int vd_dir, int auto_talk );
vd_linphone_msg_type api_one_linphone_linker_rsp( int client_ip, vd_linphonec_linker_req_pack* plinkreq, vd_linphone_rsp_result result);

vd_linphone_msg_type api_one_linphone_unlink_req( int target_ip, int vd_dir);
vd_linphone_msg_type api_one_linphone_unlink_rsp( int target_ip, vd_linphonec_linker_req_pack* punlinkreq, vd_linphone_rsp_result result);

#endif

