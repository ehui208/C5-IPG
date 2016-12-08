

#ifndef _VIDEO_MULTICAST_CLIENT_H
#define _VIDEO_MULTICAST_CLIENT_H

#include "../video_multicast_common.h"

#include "./decoder/video_displaye_controller.h"

typedef enum
{
	VD_CLIENT_IDLE,				// 客户端空闲状态
	VD_CLIENT_SUBSCRIBE,		// 客户端申请状态
	VD_CLIENT_ACTIVE,			// 客户端接收组播状态
} vd_client_state;

typedef enum
{
	//request
	VD_CLIENT_MSG_REQ_NONE,
	VD_CLIENT_MSG_REQ_BUSY,
	VD_CLIENT_MSG_REQ_NO_RSP,
	VD_CLIENT_MSG_REQ_OK,
	VD_CLIENT_MSG_REQ_UNALLOW,
	VD_CLIENT_MSG_REQ_TIMEOUT,
	VD_CLIENT_MSG_REQ_TO_PROXY,
	//close
	VD_CLIENT_MSG_CLOSE_NONE,
	VD_CLIENT_MSG_CLOSE_ERR,
	VD_CLIENT_MSG_CLOSE_NO_RSP,
	VD_CLIENT_MSG_CLOSE_OK,
	VD_CLIENT_MSG_CLOSE_TIMEOUT,
} vd_client_msg_type;

typedef struct _video_multicast_client_t_
{
	udp_comm_rt 			udp;				//	udp交互模板实例
	vd_client_state 		state;				//	状态机
	vd_client_msg_type		msg;				//	最新返回消息
	int 					send_cmd_sn;		//	发送命令包序列号
	send_sem_id_array		waitrsp_array;		// 业务应答同步队列	
	// registor data
	int						vd_multicast_time;	// 视频组播的时间(s)
	unsigned short			vd_multicast_port;	// 视频组播发送端口号
	int						vd_multicast_ip;	// 视频组播发送地址	
	int						vd_ask_server_ip;	// 视频组播请求服务器地址	
	int						vd_proxy_server_ip;	// 视频组播代理服务器地址	
}video_multicast_client;


int init_one_multicast_client( void );

/*******************************************************************************************
 * @fn:		api_one_video_client_subscribe_req
 *
 * @brief:	申请加入对应ip的视频组播服务
 *
 * @param:  	server_ip		- 服务器IP地址
 * @param:  	dev_id		- 服务器端设备id 
 * @param:  	second	 	- 视频播放时间
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
vd_client_msg_type api_one_video_client_subscribe_req( int server_ip, int dev_id, int second );

/*******************************************************************************************
 * @fn:		api_one_video_client_desubscribe_req
 *
 * @brief:	申请加入视频服务的视频组
 *
 * @param:  	server_ip		- 服务器IP地址
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
vd_client_msg_type api_one_video_client_desubscribe_req( int server_ip );

/*******************************************************************************************
 * @fn:		api_cur_video_server_proxy_ip
 *
 * @brief:	得到当前申请的代理服务器IP - 调用api_one_video_client_subscribe_req的结果为VD_CLIENT_MSG_REQ_TO_PROXY时有效
 *
 * @param:	none
 *
 * @return: 	0 - none， other - proxy ip
*******************************************************************************************/
int	api_cur_video_server_proxy_ip(void);

#endif


