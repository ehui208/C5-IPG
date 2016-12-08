
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>      /* basic system data types */
#include <error.h>

#include "video_multicast_client.h"

video_multicast_client	one_multicast_client;

int 					client_inner_msg_process_pid; 				// 内部消息处理pid
void* 					client_inner_msg_process( void* arg );

int client_recv_busines_anaylasis( char* buf, int len );

int init_one_multicast_client( void )
{
	// init state
	one_multicast_client.state = VD_CLIENT_IDLE;
	// init udp
	init_one_udp_comm_rt_buff( &one_multicast_client.udp, 500,client_recv_busines_anaylasis, 500, NULL );
	init_one_udp_comm_rt_type( &one_multicast_client.udp, "multicast client", UDP_RT_TYPE_UNICAST, VIDEO_CLIENT_CMD_RECV_PORT, VIDEO_SERVER_CMD_RECV_PORT, NULL );
	// init business rsp wait array
	init_one_send_array(&one_multicast_client.waitrsp_array);
	if( pthread_create(&client_inner_msg_process_pid, 0, client_inner_msg_process, 0) )
	{
		return -1;
	}
	vd_printf("init_one_multicast_client ok!\n");
	
	one_multicast_client.send_cmd_sn = 0;

	return 0;
}

#define CLIENT_POLLING_MS		100
void* client_inner_msg_process(void* arg )
{
	// 100ms定时查询
	while( 1 )
	{
		usleep(CLIENT_POLLING_MS*1000);
		poll_all_business_recv_array( &one_multicast_client.waitrsp_array, CLIENT_POLLING_MS );
	}
	return 0;	
}

vd_client_msg_type api_one_video_client_cancel_rsp( int server_ip, vd_cancel_req_pack *pserver_request, vd_response_result result );

/*******************************************************************************************
 * @fn:		api_join_video_multicast_group
 *
 * @brief:	加入视频服务的视频组组
 *
 * @param:  	mcg_addr 		- 组播地址的低字节 (高字节固定为224.0.2.)
 * @param:  	port 		- 组播输出的端口号
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_join_video_multicast_group( int mcg_addr, short port )
{
	vd_printf("join_video_multicast_group addr = 0x%08x, port = %d\n",mcg_addr,port);

	API_FromMulticastJoin( port, mcg_addr );

	one_multicast_client.state	= VD_CLIENT_ACTIVE;

	return 0;
}

/*******************************************************************************************
 * @fn:		leave_video_multicast_group
 *
 * @brief:	离开视频服务的视频组组
 *
 * @param:  	mcg_addr 		- 组播地址的低字节 (高字节固定为224.0.2.)
 * @param:  	port 		- 组播输出的端口号
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
int api_leave_video_multicast_group( int mcg_addr, unsigned short port )
{
	API_FromMulticastLeave( port, mcg_addr );	

	one_multicast_client.state	= VD_CLIENT_IDLE;

	return 0;
}

// 客户端接收命令业务分析
int client_recv_busines_anaylasis( char* buf, int len )
{
	// lzh_20160811_s
	// 包头需要去掉
	baony_head* pbaony_head = (baony_head*)buf;
	if( pbaony_head->type != IP8210_CMD_TYPE )
		return -1;
	len -= sizeof(baony_head);
	buf += sizeof(baony_head);
	// lzh_20160811_e

	// 客户端接收命令业务分析, 需要过滤掉target ip 的4个字节
	vd_subscribe_req_pack* 	prcvbuf 		= (vd_subscribe_req_pack*)(buf);
	vd_subscribe_rsp_pack*	prspbuf			= (vd_subscribe_rsp_pack*)(buf);
	vd_cancel_req_pack*		prcvcacelbuf	= (vd_cancel_req_pack*)(buf);

	// 业务处理
	switch( prcvbuf->target.cmd )
	{			
		case CANCEL_REQ:
			// send rsp command
			api_one_video_client_cancel_rsp(prcvcacelbuf->target.ip,prcvcacelbuf,VD_RESPONSE_RESULT_ENABLE);
			api_leave_video_multicast_group( one_multicast_client.vd_multicast_ip, one_multicast_client.vd_multicast_port);
			api_video_client_multicast_notify_off(prcvcacelbuf->target.ip);
			break;

		case SUBSCRIBE_RSP:
			if( one_multicast_client.state	== VD_CLIENT_SUBSCRIBE && one_multicast_client.send_cmd_sn == prspbuf->target.id )
			{
				// 业务应答为允许
				if( prspbuf->result == VD_RESPONSE_RESULT_ENABLE )
				{
					one_multicast_client.state				= VD_CLIENT_ACTIVE;
					one_multicast_client.msg				= VD_CLIENT_MSG_REQ_OK; 

					// 保存数据
					one_multicast_client.vd_ask_server_ip	= prspbuf->target.ip;
					one_multicast_client.vd_proxy_server_ip	= 0;
					one_multicast_client.vd_multicast_ip 	= prspbuf->vd_multicast_ip;					
					one_multicast_client.vd_multicast_port 	= prspbuf->vd_multicast_port;
					one_multicast_client.vd_multicast_time 	= prspbuf->vd_multicast_time;

					vd_printf("recv one multi ip = %0x08x\n", prspbuf->vd_multicast_ip);		
					// 启动组播接收
					api_join_video_multicast_group( prspbuf->vd_multicast_ip, prspbuf->vd_multicast_port);
				}
				// 业务应答为代理 - 得到代理服务器的IP地址
				else if( prspbuf->result == VD_RESPONSE_RESULT_TO_PROXY )
				{
					one_multicast_client.state				= VD_CLIENT_IDLE;
					one_multicast_client.msg				= VD_CLIENT_MSG_REQ_TO_PROXY;					
					
					// 保存代理服务器的IP和PORT
					one_multicast_client.vd_ask_server_ip	= prspbuf->target.ip;
					one_multicast_client.vd_proxy_server_ip	= prspbuf->vd_multicast_ip;					
					one_multicast_client.vd_multicast_ip 	= 0;
					one_multicast_client.vd_multicast_port 	= prspbuf->vd_multicast_port;
					one_multicast_client.vd_multicast_time 	= prspbuf->vd_multicast_time;
					vd_printf("recv one proxy ip = %0x08x\n", prspbuf->vd_multicast_ip);		
				}				
				// 检测应答信号为不允许
				else
				{					
					one_multicast_client.state				= VD_CLIENT_IDLE;
					one_multicast_client.msg				= VD_CLIENT_MSG_REQ_UNALLOW;
				}
			}
			// 触发业务应答
			if( trig_one_send_array( &one_multicast_client.waitrsp_array, prspbuf->target.id, prspbuf->target.cmd ) < 0 )
				vd_printf("recv one vd_rsp none-match command,id=%d\n",	prspbuf->target.id);		
			else
				vd_printf("recv one vd_rsp match command ok,id=%d\n",prspbuf->target.id);
			break;
		
		case DESUBSCRIBE_RSP:
			//if( one_multicast_client.state	== VD_CLIENT_SUBSCRIBE && one_multicast_client.send_cmd_sn == prspbuf->target.rsp_id )
			{
				one_multicast_client.state	= VD_CLIENT_IDLE;
				one_multicast_client.msg	= VD_CLIENT_MSG_REQ_OK; 
				
				// 关闭组播接收
				api_leave_video_multicast_group( prspbuf->vd_multicast_ip, prspbuf->vd_multicast_port);					
			}			
			// 触发业务应答
			if( trig_one_send_array( &one_multicast_client.waitrsp_array, prspbuf->target.id, prspbuf->target.cmd ) < 0 )
				vd_printf("recv one vd_rsp none-match command,id=%d\n",	prspbuf->target.id);		
			else
				vd_printf("recv one vd_rsp match command ok,id=%d\n",prspbuf->target.id);			
			break;			
		default:
			break;
	}
	return 0;
}

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
vd_client_msg_type api_one_video_client_subscribe_req( int server_ip, int dev_id, int second )
{
	vd_subscribe_req_pack	client_request; 

	// 状态判断
	if( one_multicast_client.state != VD_CLIENT_IDLE )
		return VD_CLIENT_MSG_REQ_BUSY;

	// 状态初始化
	one_multicast_client.state		= VD_CLIENT_SUBSCRIBE;
	
	// 命令组包
	client_request.target.ip		= server_ip;
	client_request.target.cmd		= SUBSCRIBE_REQ;
	client_request.target.id		= ++one_multicast_client.send_cmd_sn;
	client_request.result			= VD_RESPONSE_RESULT_NONE;
	client_request.vd_multicast_time= second;
	client_request.dev_id			= dev_id;

	vd_printf("api_one_video_client_subscribe_req to 0x%08x,cmd =%d, id =%d ...\n", client_request.target.ip,client_request.target.cmd,client_request.target.id);
		
	// 加入等待业务应答队列	
	//sem_t *pwaitrsp_sem = join_one_send_array(&one_multicast_client.waitrsp_array,client_request.target.id, client_request.target.cmd+1,BUSINESS_RESEND_TIME, 0, NULL, 0 );
	sem_t *pwaitrsp_sem = join_one_send_array(&one_multicast_client.waitrsp_array,client_request.target.id, client_request.target.cmd+0x80,BUSINESS_RESEND_TIME, 0, NULL, 0 );
	
	// 发送数据
	sem_t *presend_sem = one_udp_comm_trs_api( &one_multicast_client.udp, client_request.target.ip,client_request.target.cmd,client_request.target.id,
			1, (char*)&client_request+sizeof(target_head), sizeof(vd_subscribe_req_pack)-sizeof(target_head));
	
	vd_printf("api_one_video_client_subscribe_req...\n");

	// 等待服务器通信应答
	if( sem_wait_ex2(presend_sem, ACK_RESPONSE_TIMEOUT) != 0 )
	{
		dprintf("api_one_video_client_subscribe_req,wait ack --- timeout ---\n");
		one_multicast_client.state	= VD_CLIENT_IDLE;
		one_multicast_client.msg	= VD_CLIENT_MSG_REQ_TIMEOUT;		
		return one_multicast_client.msg;
	}
		
	// 等待服务器业务回复2s
	if( sem_wait_ex2(pwaitrsp_sem, 5000) != 0 )
	{
		dprintf("api_one_video_client_subscribe_req,wait rsp ---have no rsp---\n");		
		one_multicast_client.state	= VD_CLIENT_IDLE;
		one_multicast_client.msg	= VD_CLIENT_MSG_REQ_NO_RSP;		
		return one_multicast_client.msg;
	}
	
	vd_printf("api_one_video_client_subscribe_req ok!\n" );

	return one_multicast_client.msg;
}


/*******************************************************************************************
 * @fn:		api_one_video_client_desubscribe_req
 *
 * @brief:	申请加入视频服务的视频组
 *
 * @param:  	server_ip		- 服务器IP地址
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
vd_client_msg_type api_one_video_client_desubscribe_req( int server_ip )
{
	vd_desubscribe_req_pack	client_request; 

	if( one_multicast_client.state == VD_CLIENT_IDLE )
		return VD_CLIENT_MSG_REQ_NONE;

	if( one_multicast_client.state == VD_CLIENT_ACTIVE )
	{
		// 无条件关闭组播接收
		api_leave_video_multicast_group( one_multicast_client.vd_multicast_ip, one_multicast_client.vd_multicast_port); 				
	}
	
	one_multicast_client.state		= VD_CLIENT_IDLE;
		
	// 命令组包
	client_request.target.ip		= server_ip;
	client_request.target.cmd		= DESUBSCRIBE_REQ;
	client_request.target.id		= ++one_multicast_client.send_cmd_sn;
	client_request.result			= VD_RESPONSE_RESULT_NONE;

	vd_printf("api_one_video_client_desubscribe_req to 0x%08x,cmd =%d, id =%d ...\n", client_request.target.ip,client_request.target.cmd,client_request.target.id);
		
	// 加入等待业务应答队列
	//sem_t *pwaitrsp_sem = join_one_send_array(&one_multicast_client.waitrsp_array,client_request.target.id, client_request.target.cmd+1,BUSINESS_RESEND_TIME, 0, NULL, 0 );
	sem_t *pwaitrsp_sem = join_one_send_array(&one_multicast_client.waitrsp_array,client_request.target.id, client_request.target.cmd+0x80,BUSINESS_RESEND_TIME, 0, NULL, 0 );

	// 发送数据
	sem_t *presend_sem = one_udp_comm_trs_api( &one_multicast_client.udp, client_request.target.ip,client_request.target.cmd,client_request.target.id,
			1, (char*)&client_request+sizeof(target_head), sizeof(vd_subscribe_req_pack)-sizeof(target_head));
	
	vd_printf("api_one_video_client_desubscribe_req...\n");
	
	// 等待服务器通信应答
	if( sem_wait_ex2(presend_sem, ACK_RESPONSE_TIMEOUT) != 0 )
	{
		vd_printf("api_one_video_client_desubscribe_req,wait ack --- timeout ---\n");
		one_multicast_client.msg	= VD_CLIENT_MSG_CLOSE_TIMEOUT;		
		return one_multicast_client.msg;
	}
	
	// 等待服务器业务回复2s
	if( sem_wait_ex2(pwaitrsp_sem, BUSINESS_WAIT_TIMEPUT) != 0 )
	{
		vd_printf("api_one_video_client_desubscribe_req,wait rsp ---have no rsp---\n");		
		one_multicast_client.msg	= VD_CLIENT_MSG_CLOSE_NO_RSP;		
		return one_multicast_client.msg;
	}

	one_multicast_client.msg	= VD_CLIENT_MSG_CLOSE_OK;	
	
	vd_printf("api_one_video_client_desubscribe_req ok!\n" );

	return one_multicast_client.msg;	
}


/*******************************************************************************************
 * @fn:		api_one_video_client_cancel_rsp
 *
 * @brief:	相应cancel的应答
 *
 * @param:  	server_ip			- 服务器IP地址
 * @param:	pserver_request	- 服务器请求命令
 * @param:	result			- 服务器请求结果
 *
 * @return: 	-1/err, 0/ok
*******************************************************************************************/
vd_client_msg_type api_one_video_client_cancel_rsp( int server_ip, vd_cancel_req_pack *pserver_request, vd_response_result result )
{
	vd_cancel_rsq_pack	client_rsp; 

	// 状态初始化
	one_multicast_client.state	= VD_CLIENT_IDLE;
	
	// 命令组包
	client_rsp.target.ip		= server_ip;
	client_rsp.target.cmd		= CANCEL_RSP;
	client_rsp.target.id		= pserver_request->target.id;
	client_rsp.result			= result;

	// 发送数据
	// 发送数据
	sem_t *presend_sem = one_udp_comm_trs_api( &one_multicast_client.udp, client_rsp.target.ip,client_rsp.target.cmd,client_rsp.target.id,
			1, (char*)&client_rsp+sizeof(target_head), sizeof(vd_subscribe_req_pack)-sizeof(target_head));
	
	vd_printf("api_one_video_client_cancel_rsp...\n");
	
	// 等待服务器通信应答
	if( sem_wait_ex2(presend_sem, 5000) != 0 )
	{
		dprintf("api_one_video_client_cancel_rsp,wait ack --- timeout ---\n");
		one_multicast_client.msg	= VD_CLIENT_MSG_CLOSE_TIMEOUT;		
		return one_multicast_client.msg;
	}
	
	vd_printf("api_one_video_client_cancel_rsp ok!\n" );

	one_multicast_client.msg	= VD_CLIENT_MSG_CLOSE_OK;	
	return one_multicast_client.msg;	
}

/*******************************************************************************************
 * @fn:		api_cur_video_server_proxy_ip
 *
 * @brief:	得到当前申请的代理服务器IP - 调用api_one_video_client_subscribe_req的结果为VD_CLIENT_MSG_REQ_TO_PROXY时有效
 *
 * @param:	none
 *
 * @return: 	0 - none， other - proxy ip
*******************************************************************************************/
int	api_cur_video_server_proxy_ip(void)
{
	return one_multicast_client.vd_proxy_server_ip;
}


