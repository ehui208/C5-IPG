
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>      /* basic system data types */
#include <error.h>

#include "../ip_video_cs_control.h"
#include "ip_video_control.h"
#include "../video_object.h"

ip_video_client_state	one_ip_video_client;

/*******************************************************************************************
 * @fn:		ip_video_control_init
 *
 * @brief:	视频客户端初始化
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int ip_video_control_init(void)
{
	one_ip_video_client.type 			= ip_video_none;
	one_ip_video_client.active			= ip_video_idle;
	one_ip_video_client.server_ip		= 0;
	one_ip_video_client.server_dev_id	= 0;
	
	init_one_multicast_client();
}

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
int api_video_client_turn_on( ip_video_type trans_type, int server_ip, int dev_id, int period )
{
	int ret = 0;
	
	vd_client_msg_type client_req_msg;
		
	vd_printf("send client request command to ip = 0x%08x, dev_id = %d\n",server_ip, dev_id);

	//switch( get_g_video_object_trans_type() )
	switch( trans_type )
	{
		case ip_video_multicast:
			// check if linphone or not
			if( one_ip_video_client.type == ip_video_linphone )
			{
				one_ip_video_client.type = ip_video_none;
				api_linphone_client_to_close( one_ip_video_client.server_ip );
			}			
			// check if multicast or not
			if( one_ip_video_client.type == ip_video_multicast )
			{
				// lzh_20160813_s - 服务器ip地址相同则不再申请
				if( one_ip_video_client.server_ip != server_ip )
				{
					one_ip_video_client.type = ip_video_none;
					api_one_video_client_desubscribe_req( one_ip_video_client.server_ip );
					usleep(100000);
				}
				else
				{
					return ret;
				}
				// lzh_20160813_e
			}
			
			// apply for multicast
			
			// logdoc&logview
			char detail[LOG_DESC_LEN+1];
			snprintf(detail,LOG_DESC_LEN+1,"subscribe_req_to_0x%08x,period=%d...",server_ip,period);
			API_add_log_item(0,"S_VSRV",detail,NULL);

			client_req_msg = api_one_video_client_subscribe_req( server_ip, dev_id, period );
			
			if( client_req_msg == VD_CLIENT_MSG_REQ_OK )
			{
				one_ip_video_client.type 			= ip_video_multicast;
				one_ip_video_client.active			= ip_video_caller;		
				one_ip_video_client.server_ip 		= server_ip;
				one_ip_video_client.server_dev_id	= dev_id;
				vd_printf("api_one_video_client_subscribe_req over!\n" );	

				// logdoc&logview
				snprintf(detail,LOG_DESC_LEN+1,"subscribe_req_to_0x%08x ok!",server_ip);
				API_add_log_item(0,"S_VSRV",detail,NULL);
			}
			else if( client_req_msg == VD_CLIENT_MSG_REQ_TO_PROXY )
			{			
				// 保存起始请求ip地址
				// 得到代理服务器的ip地址
				one_ip_video_client.server_ip = api_cur_video_server_proxy_ip();

				// logdoc&logview
				snprintf(detail,LOG_DESC_LEN+1,"subscribe_req_to_proxy:0x%08x",one_ip_video_client.server_ip);
				API_add_log_item(0,"S_VSRV",detail,NULL);
				
				// 向代理服务器发起申请
				if( one_ip_video_client.server_ip != 0 )
				{
					client_req_msg = api_one_video_client_subscribe_req( one_ip_video_client.server_ip, dev_id, period );
					if( client_req_msg == VD_CLIENT_MSG_REQ_OK )
					{
						one_ip_video_client.type			= ip_video_multicast;
						one_ip_video_client.active			= ip_video_caller;
						one_ip_video_client.server_dev_id	= dev_id;
						// logdoc&logview
						snprintf(detail,LOG_DESC_LEN+1,"subscribe_req_to_proxy:0x%08x ok",one_ip_video_client.server_ip);
						API_add_log_item(0,"S_VSRV",detail,NULL);
					}
					else
					{
						// logdoc&logview
						snprintf(detail,LOG_DESC_LEN+1,"subscribe_req_to_proxy:0x%08x er",one_ip_video_client.server_ip);
						API_add_log_item(0,"S_VSRV",detail,NULL);
						ret = -1;
					}
				}
			}
			else 
			{
				// logdoc&logview
				snprintf(detail,LOG_DESC_LEN+1,"subscribe_req_to_0x%08x er!",server_ip);
				API_add_log_item(0,"S_VSRV",detail,NULL);	
				ret = -1;
			}
			break;
		case ip_video_linphone:
			// check if multicast or not
			if( one_ip_video_client.type == ip_video_multicast )
			{
				one_ip_video_client.type 	= ip_video_none;
				api_one_video_client_desubscribe_req( one_ip_video_client.server_ip );
			}
			// check if linphone or not
			if( one_ip_video_client.type == ip_video_linphone )
			{
				one_ip_video_client.type 	= ip_video_none;
				api_linphone_client_to_close( one_ip_video_client.server_ip );
			}			
			// apply for multicast
			if( api_linphone_client_to_call(server_ip,get_g_video_object_auto_talk() ) >= 0 )
			{
				one_ip_video_client.server_ip 	= server_ip;
				one_ip_video_client.type 		= ip_video_linphone;
				one_ip_video_client.active		= ip_video_caller;				
			}
			break;
		default:
			break;
	}	
	return ret;	
}

/*******************************************************************************************
 * @fn:		api_video_client_turn_off
 *
 * @brief:	关闭视频服务
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_client_turn_off( void )
{
	vd_printf("send client close command to ip = 0x%08x\n",one_ip_video_client.server_ip );

	// logdoc&logview
	char detail[LOG_DESC_LEN+1];

	switch( one_ip_video_client.type )
	{
		case ip_video_multicast:
			if( api_one_video_client_desubscribe_req( one_ip_video_client.server_ip ) == VD_CLIENT_MSG_CLOSE_OK )
			{
				snprintf(detail,LOG_DESC_LEN+1,"unsubscribe_req_to_0x%08x ok!",one_ip_video_client.server_ip);
				API_add_log_item(0,"S_VSRV",detail,NULL);
			}
			else
			{
				snprintf(detail,LOG_DESC_LEN+1,"unsubscribe_req_to_0x%08x er!",one_ip_video_client.server_ip);
				API_add_log_item(0,"S_VSRV",detail,NULL);
			}
			
			vd_printf("api_one_video_client_desubscribe_req over!\n" );			
			break;
		case ip_video_linphone:
			api_linphone_client_to_close( one_ip_video_client.server_ip );
			break;
		default:
			break;
	}
	one_ip_video_client.type 	= ip_video_none;			
	one_ip_video_client.active	= ip_video_idle;
	return 0;		
}

/*******************************************************************************************
 * @fn:		api_video_client_multicast_notify_off
 *
 * @brief:	服务器通知关闭客户端组播服务
 *
 * @param:	none
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_client_multicast_notify_off( int server_ip )
{
	vd_printf("one multicast apply off, ip = 0x%08x\n",server_ip );

	// logdoc&logview
	char detail[LOG_DESC_LEN+1];
	snprintf(detail,LOG_DESC_LEN+1,"multicast_off_from_0x%08x",server_ip);
	API_add_log_item(0,"S_VSRV",detail,NULL);

	// 登记状态
	one_ip_video_client.type	= ip_video_none;
	one_ip_video_client.active	= ip_video_idle;
	
	// 发送通告命令到
	api_video_c_service_close_notify();
	return 0;
}

/*******************************************************************************************
 * @fn:		api_video_client_linphone_apply_on
 *
 * @brief:	服务器端呼叫linphonec - 服务器端linphonec呼叫成功后调用
 *
 * @param:	server_ip - 服务器端ip
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_client_linphone_apply_on( int server_ip )
{
	vd_printf("one linphone apply on, ip = 0x%08x\n",server_ip );

	if( api_video_c_service_start_apply(ip_video_linphone, server_ip) == 0 )
	{
		// 登记状态
		one_ip_video_client.type		= ip_video_linphone;
		one_ip_video_client.active		= ip_video_becalled;
		one_ip_video_client.server_ip	= server_ip;
		return 0;
	}	
	else
		return -1;
}

/*******************************************************************************************
 * @fn:		api_video_client_linphone_apply_off
 *
 * @brief:	服务器端退出linphonec - 服务器端linphonec呼叫关闭后调用
 *
 * @param:	server_ip - 服务器端ip
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_video_client_linphone_apply_off( int server_ip )
{
	vd_printf("one linphone apply off, ip = 0x%08x\n",server_ip );

	// 登记状态
	one_ip_video_client.type	= ip_video_none;
	one_ip_video_client.active	= ip_video_idle;
	
	// 发送通告命令到
	api_video_c_service_close_notify();
	return 0;
}


////////////////////////////////////////////////////////////
int api_video_client_adjust_apply( UDP_Image_t* presult  )
{
	int len = sizeof(UDP_Image_t);
	if( api_udp_c5_ipc_send_req( one_ip_video_client.server_ip ,CMD_CAM_REMOTE_ADJUST_REQ,(char*)&presult,sizeof(UDP_Image_t), presult, &len) == -1 )
	{
		vd_printf("api_video_c_service_adjust_apply fail\n");
		return -1;
	}
	else
	{
		vd_printf("api_video_c_service_adjust_apply ok\n");
		return 0;
	}		
}

int api_video_client_adjust_bright_apply( int inc  )
{
	UDP_Image_t apply;

	if( inc )
		apply.dir 	= ADJ_INC;
	else
		apply.dir 	= ADJ_DEC;
	apply.type		= ADJ_BRIGHT;
	
	return api_video_client_adjust_apply(&apply);
}

int api_video_client_adjust_color_apply( int inc  )
{
	UDP_Image_t apply;

	if( inc )
		apply.dir 	= ADJ_INC;
	else
		apply.dir 	= ADJ_DEC;
	apply.type		= ADJ_COLOR;
	
	return api_video_client_adjust_apply(&apply);
}

int api_video_client_adjust_contrast_apply( int inc  )
{
	UDP_Image_t apply;

	if( inc )
		apply.dir 	= ADJ_INC;
	else
		apply.dir 	= ADJ_DEC;
	apply.type		= ADJ_CONTRAST;
	
	return api_video_client_adjust_apply(&apply);
}

