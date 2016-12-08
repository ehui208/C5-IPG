
#ifndef _IP_VIDEO_CONTROL_H
#define _IP_VIDEO_CONTROL_H

#include "video_multicast_client.h"
#include "video_linphone_client.h"

typedef enum
{
	ip_video_none,				// 无
	ip_video_multicast,			// 组播类型
	ip_video_linphone,			// linhpone类型
	ip_video_unicast,			// 点播类型
} ip_video_type;

typedef enum
{
	ip_video_idle,				// 无呼叫
	ip_video_caller,			// 主呼叫
	ip_video_becalled,			// 被呼叫
} ip_video_active;

typedef struct _ip_video_client_state_
{
	ip_video_type	type;
	ip_video_active	active;
	int 			server_ip;
	int 			server_dev_id;
} ip_video_client_state;

/*******************************************************************************************
 * @fn:		ip_video_control_init
 *
 * @brief:	视频客户端初始化
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int ip_video_control_init(void);

/*******************************************************************************************
 * @fn:		api_video_client_turn_on
 *
 * @brief:	申请加入视频服务的视频组
 *
 * @param:	trans_type	- 传输类型
 * @param:  	server_ip		- 服务器IP地址
 * @param:  	dev_id		- 服务器端设备id
 * @param:  	period		- 服务时间
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_client_turn_on( ip_video_type trans_type, int server_ip, int dev_id, int period );

/*******************************************************************************************
 * @fn:		api_video_client_turn_off
 *
 * @brief:	退出视频服务
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_client_turn_off(void);

/*******************************************************************************************
 * @fn:		api_video_client_multicast_notify_off
 *
 * @brief:	服务器通知关闭客户端组播服务
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_client_multicast_notify_off( int server_ip );


/*******************************************************************************************
 * @fn:		api_video_client_linphone_apply_on
 *
 * @brief:	服务器端呼叫linphonec - 服务器端linphonec呼叫成功后调用
 *
 * @param:	server_ip - 服务器端ip
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_client_linphone_apply_on( int server_ip );

/*******************************************************************************************
 * @fn:		api_video_client_linphone_apply_off
 *
 * @brief:	服务器端退出linphonec - 服务器端linphonec呼叫关闭后调用
 *
 * @param:	server_ip - 服务器端ip
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_client_linphone_apply_off( int server_ip );


int api_video_client_adjust_apply( UDP_Image_t* presult  );
int api_video_client_adjust_bright_apply( int inc  );
int api_video_client_adjust_color_apply( int inc  );
int api_video_client_adjust_contrast_apply( int inc  );

#endif





