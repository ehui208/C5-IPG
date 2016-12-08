
#ifndef _IP_VIDEO_CS_CONTORL_H_
#define _IP_VIDEO_CS_CONTORL_H_

#include "video_multicast_common.h"
#include "./ip_video_control/ip_video_control.h"
#include "./ip_camera_control/ip_camera_control.h"

typedef enum
{
	VIDEO_CS_IDLE,
	VIDEO_CS_CLIENT,
	VIDEO_CS_SERVER,
	VIDEO_CS_BOTH,
} VIDEO_CS_STATE;

/*******************************************************************************************
 * @fn:		init_video_cs_service
 *
 * @brief:	初始化视频CS服务
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int init_video_cs_service(void);


/*******************************************************************************************
 * @fn:		api_get_video_cs_service_state
 *
 * @brief:	得到当前视频CS的状态 – 0-空闲/1-客户端/2-服务器端/3-C&S
 *
 * @param:	none
 *
 * @return: 	VIDEO_CS_STATE
*******************************************************************************************/
VIDEO_CS_STATE api_get_video_cs_service_state(void);


/*******************************************************************************************
 * @fn:		api_video_s_service_turn_on
 *
 * @brief:	开启服务器端视频输出服务 – 若为组播则为测试服务，强制打开视频组播输出
 *
 * @param:	trans_type	- 传输类型
 * @param:  	client_ip		- 客户端IP地址
 * @param:  	period		- 服务时间
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_s_service_turn_on( ip_camera_type trans_type, int client_ip, int period );

/*******************************************************************************************
 * @fn:		api_video_s_service_turn_off
 *
 * @brief:	关闭服务器端视频输出服务
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_s_service_turn_off( void );

/*******************************************************************************************
 * @fn:		api_video_s_service_start_apply
 *
 * @brief:	服务器端接收到视频输出申请 -（由后端调用）
 *
 * @param:  	trans_type	- 传输类型
 * @param:  	client_ip		- 客户端IP地址
 * @param:  	dev_id		- 客户端请求的摄像头id
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_s_service_start_apply( ip_camera_type trans_type, int client_ip, int dev_id );

// 得到当前服务的视频源设备id
int api_get_video_s_service_dev_id( void );

int api_video_s_service_close( void );

/*******************************************************************************************
 * @fn:		api_video_s_service_close_notify
 *
 * @brief:	服务器端接收到视频关闭通告 -（由后端调用）
 *
 * @param:  	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_s_service_close_notify( void );

/*******************************************************************************************
 * @fn:		api_video_c_service_turn_on
 *
 * @brief:	申请服务器端视频输出服务
 *
 * @param:	trans_type	- 传输类型
 * @param:  	server_ip	- 服务器端IP地址
 * @param:  	dev_id	- 服务器端设备id
 * @param:  	period		- 服务时间
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_c_service_turn_on( ip_video_type trans_type, int server_ip, int dev_id, int period );


/*******************************************************************************************
 * @fn:		api_video_c_service_turn_off
 *
 * @brief:	关闭客户器端视频输入服务
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_c_service_turn_off( void );


/*******************************************************************************************
 * @fn:		api_video_c_service_start_apply
 *
 * @brief:	客户端接收到视频播放申请 -（由后端调用）
 *
 * @param:  	trans_type - 传输类型
 * @param:  	server_ip - 服务器端IP地址
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_c_service_start_apply( ip_video_type trans_type, int server_ip );

/*******************************************************************************************
 * @fn:		api_video_c_service_close_notify
 *
 * @brief:	客户端接收到视频关闭通告 -（由后端调用）
 *
 * @param:  	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_c_service_close_notify( void );


#define API_VIDEO_S_SERVICE_TURN_ON_TEST(t)	\
	api_video_s_service_turn_on( ip_camera_multicast, 0, t )

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int api_video_c_service_adjust_apply( UDP_Image_t* presult );
int api_video_c_service_adjust_bright_apply( int inc  );
int api_video_c_service_adjust_color_apply( int inc );
int api_video_c_service_adjust_contrast_apply( int inc  );

int api_video_s_service_adjust( UDP_Image_t* presource );

int api_video_s_service_adjust_bright( int inc  );
int api_video_s_service_adjust_color( int inc  );
int api_video_s_service_adjust_contrast( int inc  );
int api_video_s_service_adjust_reply( int client_ip, UDP_Image_t* presource );
//int api_video_s_service_initial( int inc  );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif

