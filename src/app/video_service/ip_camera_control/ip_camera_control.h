
#ifndef _IP_CAMERA_CONTROL_H
#define _IP_CAMERA_CONTROL_H

#include "video_multicast_server.h"
#include "video_linphone_server.h"

typedef enum
{
	ip_camera_none,				// 无
	ip_camera_local,			// 本地视频开启
	ip_camera_multicast,		// 组播类型
	ip_camera_linphone,			// linhpone类型
	ip_camera_unicast,			// 点播类型
} ip_camera_type;

typedef enum
{
	ip_camera_idle,				// 无呼叫
	ip_camera_caller,			// 主呼叫
	ip_camera_becalled,			// 被呼叫
} ip_camera_active;

typedef struct _ip_camera_server_state_
{
	ip_camera_type		type;
	ip_camera_active	active;
	int 				ip;
	int					dev_id;
} ip_camera_server_state;

/*******************************************************************************************
 * @fn:		ip_camera_control_init
 *
 * @brief:	视频服务器端初始化
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int ip_camera_control_init(void);

/*******************************************************************************************
 * @fn:		api_camera_server_turn_on
 *
 * @brief:	申请加入视频服务 - 若为组播则为测试模式，直接启动组播；若是linphone则启动呼叫过程
 *
 * @param:	trans_type	- 传输类型
 * @param:  	client_ip		- 客户端IP地址
 * @param:  	period		- 服务时间
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_camera_server_turn_on( ip_camera_type trans_type, int client_ip, int period );

/*******************************************************************************************
 * @fn:		api_camera_server_turn_off
 *
 * @brief:	退出视频服务 - (若为组播则全部退出)
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_camera_server_turn_off(void);

/*******************************************************************************************
 * @fn: 	api_camera_server_multicast_apply_on
 *
 * @brief:	组播开启申请 - 仅仅第一次开启组播服务时，需要向前端申请
 *
 * @param:	apply_ip 	- 请求方的IP
 * @param:	dev_id 	- 请求的设备id
 *
 * @return: 	-1/err, 0/ok, 1/转呼叫
*******************************************************************************************/
int api_camera_server_multicast_apply_on( int apply_ip, int dev_id );

/*******************************************************************************************
 * @fn:		api_camera_server_multicast_notify_off
 *
 * @brief:	无客户端后关闭组播发出通告 
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_camera_server_multicast_notify_off( void );


/*******************************************************************************************
 * @fn:		api_camera_server_linphone_apply_on
 *
 * @brief:	客户端呼叫linphonec - 客户端linphonec呼叫成功后调用
 *
 * @param:	client_ip - 客户端ip
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_camera_server_linphone_apply_on( int client_ip );

/*******************************************************************************************
 * @fn:		api_camera_server_linphone_apply_off
 *
 * @brief:	客户端linphonec退出 - 客户端linphonec呼叫退出后调用
 *
 * @param:	client_ip - 客户端ip
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_camera_server_linphone_apply_off( int client_ip );

int api_camera_server_adjust( UDP_Image_t* presource );
int api_video_server_adjust_bright( int inc  );
int api_video_server_adjust_color( int inc  );
int api_video_server_adjust_contrast( int inc  );
int api_camera_server_adjust_reply( int client_ip, UDP_Image_t* presource );

#endif

