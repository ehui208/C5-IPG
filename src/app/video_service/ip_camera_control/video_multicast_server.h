
#ifndef _VIDEO_MULTICAST_SERVR_H
#define _VIDEO_MULTICAST_SERVR_H

#include "../video_multicast_common.h"

#include "subscriber_list.h"
#include "video_source_map_tab.h"
#include "video_source_proxy.h"

#include "./encoder_vin/video_capture_controller.h"
#include "./encoder_vin/obj_adjust.h"

typedef enum
{
	VD_SERVER_IDLE,
	VD_SERVER_ACTIVE,
} vd_server_state;


typedef enum
{
	// response
	VD_SERVER_MSG_RSP_OK,
	VD_SERVER_MSG_RSP_ERR,
	VD_SERVER_MSG_RSP_NO_PERFORMAT,	
	VD_SERVER_MSG_RSP_TIMEOUT,	
	// cancel
	VD_SERVER_MSG_CANCEL_OK,
	VD_SERVER_MSG_CANCEL_ERR,
	VD_SERVER_MSG_CANCEL_NO_RSP,	
	VD_SERVER_MSG_CANCEL_TIMEOUT,		
} vd_server_msg_type;

typedef struct _video_multicast_server_t_
{
	udp_comm_rt 			udp;					//  	udp交互模板实例
	vd_server_state 		state;					//	状态机
	vd_server_msg_type		msg;					//	最新返回消息
	send_sem_id_array		waitrsp_array;			//	业务应答同步队列
	int 					send_cmd_sn;			//	发送命令包序列号
}video_multicast_server;



int init_one_multicast_server( void );


/*******************************************************************************************
 * @fn:		api_one_video_server_cancel_req
 *
 * @brief:	通知指定客户端退出视频组播组
 *
 * @param:  	client_ip			- 客户端IP地址
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
vd_server_msg_type api_one_video_server_cancel_req( int client_ip );

#endif


