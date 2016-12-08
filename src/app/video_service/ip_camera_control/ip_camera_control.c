

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>      /* basic system data types */
#include <error.h>

#include "ip_camera_control.h"
#include "../video_object.h"

ip_camera_server_state	one_ip_camera_server;

/*******************************************************************************************
 * @fn:		ip_camera_control_init
 *
 * @brief:	视频服务器端初始化
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int ip_camera_control_init(void)
{
	one_ip_camera_server.type 	= ip_camera_none;
	one_ip_camera_server.active	= ip_camera_idle;
	one_ip_camera_server.ip		= 0;
	one_ip_camera_server.dev_id	= 0;
	init_one_multicast_server();
}

/*******************************************************************************************
 * @fn:		api_camera_server_turn_on
 *
 * @brief:	申请加入视频服务
 *
 * @param:	trans_type	- 传输类型
 * @param:  	client_ip		- 客户端IP地址
 * @param:  	period		- 服务时间
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_camera_server_turn_on( ip_camera_type trans_type, int client_ip, int period )
{
	int				mcg_addr;
	short			port;
	
	vd_printf("send server request command to ip = 0x%08x\n",client_ip);

	switch( trans_type )
	{
		case ip_camera_local:
			// 服务器端组播不能主动发出命令，作为调试模式无条件的打开组播发送
			get_server_multicast_addr_port( &mcg_addr, &port );
			// 调用开启指定客户端视频组播输出命令
			api_enable_video_server_multicast( mcg_addr, port );			
			break;
		case ip_camera_multicast:
			// lzh_20161119_s
			one_ip_camera_server.ip 	= client_ip;			
			// lzh_20161119_e
			// 服务器端组播不能主动发出命令，作为调试模式无条件的打开组播发送
			get_server_multicast_addr_port( &mcg_addr, &port );
			// 调用开启指定客户端视频组播输出命令
			api_enable_video_server_multicast( mcg_addr, port );
			break;
		case ip_camera_linphone:
			// check if multicast or not
			if( one_ip_camera_server.type == ip_camera_multicast )
			{
				one_ip_camera_server.type 	= ip_camera_none;
				api_one_video_server_cancel_req( one_ip_camera_server.ip );
			}
			// check if linphone or not
			if( one_ip_camera_server.type == ip_camera_linphone )
			{
				one_ip_camera_server.type 	= ip_camera_none;
				api_linphone_server_to_close( one_ip_camera_server.ip );
			}			
			// apply for multicast			
			if( api_linphone_server_to_call(client_ip,get_g_video_object_auto_talk() ) >= 0 )
			{
				one_ip_camera_server.ip 	= client_ip;
				one_ip_camera_server.type 	= ip_camera_linphone;
				one_ip_camera_server.active	= ip_camera_caller;				
			}
			break;
	}	
	return 0;	
}


/*******************************************************************************************
 * @fn:		api_camera_server_turn_off
 *
 * @brief:	退出视频服务 - (若为组播则全部退出)
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_camera_server_turn_off( void )
{
	vd_printf("send server close command to ip = 0x%08x\n",one_ip_camera_server.ip );

	switch( one_ip_camera_server.type )
	{
		case ip_camera_multicast:
			api_one_video_server_cancel_req( one_ip_camera_server.ip );
			break;
		case ip_camera_linphone:
			api_linphone_server_to_close( one_ip_camera_server.ip );
			break;
		default:
			break;
	}
	one_ip_camera_server.type 	= ip_camera_none;
	one_ip_camera_server.active	= ip_camera_idle;
	return 0;		
}

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
int api_camera_server_multicast_apply_on( int apply_ip, int dev_id )
{
	if( api_video_s_service_start_apply(ip_camera_multicast, apply_ip, dev_id ) == 0 )
	{
		one_ip_camera_server.type	= ip_camera_multicast;
		one_ip_camera_server.active = ip_camera_becalled; 
		one_ip_camera_server.dev_id	= dev_id;
		return 0;
	}
	else
		return -1;
}

/*******************************************************************************************
 * @fn:		api_camera_server_multicast_notify_off
 *
 * @brief:	无客户端后关闭组播发出通告 
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_camera_server_multicast_notify_off( void )
{
	one_ip_camera_server.type 	= ip_camera_none;			
	one_ip_camera_server.active	= ip_camera_idle;	
	// 发送通告命令
	api_video_s_service_close_notify();
	return 0;
}

/*******************************************************************************************
 * @fn:		api_camera_server_linphone_apply_on
 *
 * @brief:	客户端呼叫linphonec - 客户端linphonec呼叫成功后调用
 *
 * @param:	client_ip - 客户端ip
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_camera_server_linphone_apply_on( int client_ip )
{
	vd_printf("one linphone apply on, ip = 0x%08x\n",client_ip );

	if( api_video_s_service_start_apply(ip_camera_linphone, client_ip) )
	{
		// 登记状态
		one_ip_camera_server.type	= ip_camera_linphone;
		one_ip_camera_server.active = ip_camera_becalled; 	
		one_ip_camera_server.ip		= client_ip;
		return 0;
	}	
	else
		return -1;
}

/*******************************************************************************************
 * @fn:		api_camera_server_linphone_apply_off
 *
 * @brief:	客户端linphonec退出 - 客户端linphonec呼叫退出后调用
 *
 * @param:	client_ip - 客户端ip
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_camera_server_linphone_apply_off( int client_ip )
{
	vd_printf("one linphone apply off, ip = 0x%08x\n",client_ip );

	// 登记状态
	one_ip_camera_server.type	= ip_camera_none;
	one_ip_camera_server.active = ip_camera_idle;
	
	// 发送通告命令到
	api_video_s_service_close_notify();
	return 0;
}

int api_camera_server_adjust( UDP_Image_t* presource )
{
	if( presource == NULL )
		return -1;
	
	switch( presource->type )
	{
		case ADJ_CONTRAST:
			if( presource->dir == ADJ_DEC )
				presource->data = SetImageContrastDec();
			else if( presource->dir == ADJ_INC )
				presource->data = SetImageContrastInc();
			break;
	
		case ADJ_BRIGHT:
			if( presource->dir == ADJ_DEC )
				presource->data = SetImageBrightDec();
			else if( presource->dir == ADJ_INC )
				presource->data = SetImageBrightInc();		
			break;
	
		case ADJ_COLOR:
			if( presource->dir == ADJ_DEC )
				presource->data = SetImageColorDec();
			else if( presource->dir == ADJ_INC )
				presource->data = SetImageColorInc();			
			break;
	
		case ADJ_ALL:
			vd_printf(" ADJ_ALL  %d %d %d \n",presource->data.logContrastCnt, presource->data.logBrightCnt, presource->data.logColorCnt);
			if( presource->dir == ADJ_GET )
				ReadImagePara( &presource->data );
			else if( presource->dir == ADJ_SET )
				presource->data = SetImageAllValue( presource->data.logContrastCnt, presource->data.logBrightCnt, presource->data.logColorCnt );
			break;
	}
	return 0;
}

int api_video_server_adjust_bright( int inc  )
{
	UDP_Image_t apply;

	if( inc )
		apply.dir 	= ADJ_INC;
	else
		apply.dir 	= ADJ_DEC;
	apply.type		= ADJ_BRIGHT;
	
	return api_camera_server_adjust(&apply);
}

int api_video_server_adjust_color( int inc  )
{
	UDP_Image_t apply;

	if( inc )
		apply.dir 	= ADJ_INC;
	else
		apply.dir 	= ADJ_DEC;
	apply.type		= ADJ_COLOR;
	
	return api_camera_server_adjust(&apply);
}

int api_video_server_adjust_contrast( int inc )
{
	UDP_Image_t apply;

	if( inc )
		apply.dir 	= ADJ_INC;
	else
		apply.dir 	= ADJ_DEC;
	apply.type		= ADJ_CONTRAST;
	
	return api_camera_server_adjust(&apply);
}

int api_camera_server_adjust_reply( int client_ip, UDP_Image_t* presource )
{
	if( api_udp_c5_ipc_send_data( client_ip,CMD_CAM_REMOTE_ADJUST_RSP,(char*)&presource,sizeof(UDP_Image_t)) == -1 )
	{
		vd_printf("api_video_s_service_adjust_reply fail\n");
		return -1;
	}
	else
	{
		vd_printf("api_video_s_service_adjust_reply ok\n");
		return 0;
	}		
}


