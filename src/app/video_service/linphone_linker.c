
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>      /* basic system data types */
#include <error.h>

#include "./ip_camera_control/video_linphone_server.h"
#include "./ip_video_control/video_linphone_client.h"
#include "video_multicast_common.h"
#include "linphone_linker.h"

video_linphone_linker	one_linphone_linker;

Loop_vdp_common_buffer	linphone_msg_queue;

int 	linphone_linker_process_pid;

void* 	linphone_linker_msg_process( void* arg );
int 	linphone_linker_udp_recv_anaylasis( char* buf, int len );
int 	linphone_linker_inner_recv_anaylasis( char* buf, int len );

int init_one_linphone_linker( void )
{
	// init state
	one_linphone_linker.state 			= VD_LINPHONE_IDLE;
	one_linphone_linker.send_cmd_sn 	= 0;	
	// init udp
	init_one_udp_comm_rt_buff( &one_linphone_linker.udp, 500,linphone_linker_udp_recv_anaylasis, 500, NULL );
	init_one_udp_comm_rt_type( &one_linphone_linker.udp, "linphone_linker", UDP_RT_TYPE_UNICAST, LINPHONE_STARTER_RECV_PORT, LINPHONE_STARTER_RECV_PORT, NULL );	
	// init business rsp wait array
	init_one_send_array(&one_linphone_linker.waitrsp_array);

	// init inner msg queue
	init_vdp_common_queue(&linphone_msg_queue,500,(msg_process)linphone_linker_inner_recv_anaylasis,NULL);
	if( pthread_create(&linphone_linker_process_pid, 0, linphone_linker_msg_process, &linphone_msg_queue) )
	{
		vd_printf("Create task thread Failure,%s\n", strerror(errno));
	}
	vd_printf("init_one_linphone_linker ok!\n");	

	return 0;
}

#define LINPHNE_STARTER_POLLING_MS		100
void* linphone_linker_msg_process(void* arg )
{
	p_Loop_vdp_common_buffer	pmsgqueue 	= (p_Loop_vdp_common_buffer)arg;
	p_vdp_common_buffer pdb 	= 0;

	while( 1 )
	{	
		// 内部消息队列处理
		int size;
		size = pop_vdp_common_queue( pmsgqueue,&pdb,LINPHNE_STARTER_POLLING_MS );
		if( size > 0 )
		{
			(*pmsgqueue->process)(pdb,size);
			purge_vdp_common_queue( pmsgqueue );
		}
		// 100ms定时查询
		poll_all_business_recv_array( &one_linphone_linker.waitrsp_array, LINPHNE_STARTER_POLLING_MS );
	}	
	return 0;	
}

// inner接收命令业务分析
int linphone_linker_inner_recv_anaylasis( char* buf, int len )
{
	return 0;
}

// udp接收命令业务分析
int linphone_linker_udp_recv_anaylasis( char* buf, int len )
{
	// lzh_20160811_s
	// 包头需要去掉
	baony_head* pbaony_head = (baony_head*)buf;
	if( pbaony_head->type != IP8210_CMD_TYPE )
		return -1;
	len -= sizeof(baony_head);
	buf += sizeof(baony_head);
	// lzh_20160811_e
		
	// 接收命令业务分析, 需要过滤掉target ip 的4个字节
	vd_linphonec_linker_req_pack* 	prcvbuf = (vd_linphonec_linker_req_pack*)(buf);
	vd_linphonec_linker_rsp_pack*	prspbuf	= (vd_linphonec_linker_rsp_pack*)(buf);

	switch( prcvbuf->target.cmd )
	{			
		case LINPHONE_LINK_REQ:
			// 得到客户端的请求: 
			// 1. 若本地linphone未启动，则需要: 启动linphone为server类型; 登记状态; 并通知对方准备ok
			// 2. 若本机linphone已在server状态下，则不需要重启linphonae, 通知对方准备ok
			// 3. 若本地linphone已在client状态下，需要关闭linphone，并重启linphone为server类型; 登记状态; 并通知对方准备ok
			if( prcvbuf->vd_direct )
			{
				// 客户端请求开启服务器端
				if( api_linphone_server_becalled( prcvbuf->target.ip, prcvbuf->auto_talk ) < 0 )
				{
					api_one_linphone_linker_rsp( prcvbuf->target.ip, prcvbuf, VD_LINPHONE_RSP_RESULT_DISABLE);
				}
				else
				{
					api_one_linphone_linker_rsp( prcvbuf->target.ip, prcvbuf, VD_LINPHONE_RSP_RESULT_ENABLE);				
				}
			}
			else
			{
				// 服务器端请求开启客户端
				if( api_linphone_client_becalled( prcvbuf->target.ip, prcvbuf->auto_talk ) < 0)
				{
					api_one_linphone_linker_rsp( prcvbuf->target.ip, prcvbuf, VD_LINPHONE_RSP_RESULT_DISABLE);
				}
				else
				{
					api_one_linphone_linker_rsp( prcvbuf->target.ip, prcvbuf, VD_LINPHONE_RSP_RESULT_ENABLE);				
				}				
			}
			break;
			
		case LINPHONE_LINK_RSP:
			// 得到服务器端的应答: 
			// 1. 发送请求前已启动本地linphone客户端
			// 2. 登记状态，并触发申请者ok
			if( prspbuf->result == VD_LINPHONE_RSP_RESULT_ENABLE )
			{
				one_linphone_linker.msg = VD_LINPHONE_MSG_REQ_OK;		
			}
			else
			{
				one_linphone_linker.msg = VD_LINPHONE_MSG_REQ_UNALLOW;			
			}				
			// trigg rsp sem
			if( trig_one_send_array( &one_linphone_linker.waitrsp_array, prspbuf->target.id, prspbuf->target.cmd ) < 0 )
				vd_printf("recv one vd_rsp none-match command,id=%d\n",	prspbuf->target.id);		
			else
				vd_printf("recv one vd_rsp match command ok,id=%d\n",prspbuf->target.id);			
			break;
			
		case LINPHONE_QUIT_REQ:
			if( prcvbuf->vd_direct )
			{
				// 客户端请求关闭服务器端
				if( api_linphone_server_beclosed( prcvbuf->target.ip ) < 0 )
				{
					api_one_linphone_unlink_rsp( prcvbuf->target.ip, prcvbuf, VD_LINPHONE_RSP_RESULT_DISABLE);		
				}
				else
				{
					api_one_linphone_unlink_rsp( prcvbuf->target.ip, prcvbuf, VD_LINPHONE_RSP_RESULT_ENABLE); 			
				}
			}
			else
			{
				// 服务器端请求关闭客户端
				if( api_linphone_client_beclosed( prcvbuf->target.ip ) < 0 )
				{
					api_one_linphone_unlink_rsp( prcvbuf->target.ip, prcvbuf, VD_LINPHONE_RSP_RESULT_DISABLE);		
				}
				else
				{
					api_one_linphone_unlink_rsp( prcvbuf->target.ip, prcvbuf, VD_LINPHONE_RSP_RESULT_ENABLE); 			
				}				
			}
			break;
			
		case LINPHONE_QUIT_RSP:
			if( prspbuf->result == VD_LINPHONE_RSP_RESULT_ENABLE )
			{
				one_linphone_linker.msg = VD_LINPHONE_MSG_CLOSE_OK;		
			}
			else
			{
				one_linphone_linker.msg = VD_LINPHONE_MSG_CLOSE_ERR;
			}
			// trigg rsp sem
			if( trig_one_send_array( &one_linphone_linker.waitrsp_array, prspbuf->target.id, prspbuf->target.cmd) < 0 )
				vd_printf("recv one vd_rsp none-match command,id=%d\n",	prspbuf->target.id);		
			else
				vd_printf("recv one vd_rsp match command ok,id=%d\n",prspbuf->target.id);			
			break;			
	}
	return 0;
}

vd_linphone_msg_type api_one_linphone_linker_req( int server_ip, int vd_dir, int auto_talk )
{
	vd_linphonec_linker_req_pack	client_request; 

	// 命令组包
	client_request.target.ip		= server_ip;
	client_request.target.cmd		= LINPHONE_LINK_REQ;
	client_request.target.id		= ++one_linphone_linker.send_cmd_sn;
	client_request.vd_direct		= vd_dir;
	client_request.auto_talk		= auto_talk;

	one_linphone_linker.msg = VD_LINPHONE_MSG_REQ_NONE;

	vd_printf("api_one_linphone_linker_req to 0x%08x,cmd =%d, id =%d, dir = %d, auto = %d ...\n", 
		client_request.target.ip,client_request.target.cmd,client_request.target.id,client_request.vd_direct,client_request.auto_talk);
		
	// 加入等待业务应答队列	
	sem_t *pwaitrsp_sem = join_one_send_array(&one_linphone_linker.waitrsp_array,client_request.target.id, 
													client_request.target.cmd+1,10000, 0, NULL, 0 );
	// 发送数据
	sem_t *presend_sem = one_udp_comm_trs_api( &one_linphone_linker.udp, client_request.target.ip, client_request.target.cmd, client_request.target.id, 
			1, (char*)&client_request+sizeof(target_head), sizeof(vd_linphonec_linker_req_pack)-sizeof(target_head));

	// 等待服务器通信应答
	if( sem_wait_ex2(presend_sem, ACK_RESPONSE_TIMEOUT) != 0 )
	{
		vd_printf("api_one_linphone_linker_req,wait ack --- timeout ---\n");
		one_linphone_linker.msg	= VD_LINPHONE_MSG_REQ_TIMEOUT;		
		return one_linphone_linker.msg;
	}
		
	// 等待服务器业务回复2s
	if( sem_wait_ex2(pwaitrsp_sem, 12000) != 0 )
	{
		vd_printf("api_one_linphone_linker_req,wait rsp ---have no rsp---\n");		
		one_linphone_linker.msg	= VD_LINPHONE_MSG_REQ_NO_RSP;		
		return one_linphone_linker.msg;
	}

	if( one_linphone_linker.msg == VD_LINPHONE_MSG_REQ_OK )
		vd_printf("api_one_linphone_linker_req ok!\n" );
	else
		vd_printf("api_one_linphone_linker_req er!\n" );

	return one_linphone_linker.msg;
}

vd_linphone_msg_type api_one_linphone_linker_rsp( int client_ip, vd_linphonec_linker_req_pack* plinkreq, vd_linphone_rsp_result result)
{
	vd_linphonec_linker_rsp_pack	client_rsp; 

	// 命令组包
	client_rsp.target.ip		= client_ip;
	client_rsp.target.cmd		= LINPHONE_LINK_RSP;
	client_rsp.target.id		= plinkreq->target.id;
	client_rsp.result			= result;

	// 发送数据
	sem_t *presend_sem = one_udp_comm_trs_api( &one_linphone_linker.udp, client_rsp.target.ip, client_rsp.target.cmd, client_rsp.target.id,
												1,(char*)&client_rsp+sizeof(target_head), sizeof(vd_linphonec_linker_rsp_pack)-sizeof(target_head));
	
	vd_printf("api_one_linphone_linker_rsp...\n");
	
	// 等待服务器通信应答
	if( sem_wait_ex2(presend_sem, 2000) != 0 )
	{
		vd_printf("api_one_linphone_linker_rsp,wait ack --- timeout ---\n");
		one_linphone_linker.msg	= VD_LINPHONE_MSG_REQ_TIMEOUT;		
		return one_linphone_linker.msg;
	}
	
	vd_printf("api_one_linphone_linker_rsp ok!\n" );

	one_linphone_linker.msg	= VD_LINPHONE_MSG_REQ_OK;	
	return one_linphone_linker.msg;	
}


vd_linphone_msg_type api_one_linphone_unlink_req( int target_ip,  int vd_dir)
{
	vd_linphonec_linker_req_pack	client_request; 

	// 命令组包
	client_request.target.ip		= target_ip;
	client_request.target.cmd		= LINPHONE_QUIT_REQ;
	client_request.target.id		= ++one_linphone_linker.send_cmd_sn;
	client_request.vd_direct		= vd_dir;
	client_request.auto_talk		= 0;

	one_linphone_linker.msg = VD_LINPHONE_MSG_CLOSE_NONE;

	vd_printf("api_one_linphone_unlink_req to 0x%08x,cmd =%d, id =%d, dir = %d, auto = %d ...\n", 
		client_request.target.ip,client_request.target.cmd,client_request.target.id,client_request.vd_direct,client_request.auto_talk);
		
	// 加入等待业务应答队列	
	sem_t *pwaitrsp_sem = join_one_send_array(&one_linphone_linker.waitrsp_array,client_request.target.id, 
													client_request.target.cmd+1,10000, 0, NULL, 0 );
	// 发送数据
	sem_t *presend_sem = one_udp_comm_trs_api( &one_linphone_linker.udp, client_request.target.ip, client_request.target.cmd, client_request.target.id, 
				1,(char*)&client_request+sizeof(target_head), sizeof(vd_linphonec_linker_req_pack)-sizeof(target_head));
	
	// 等待服务器通信应答
	if( sem_wait_ex2(presend_sem, ACK_RESPONSE_TIMEOUT) != 0 )
	{
		vd_printf("api_one_linphone_unlink_req,wait ack --- timeout ---\n");
		one_linphone_linker.msg	= VD_LINPHONE_MSG_CLOSE_TIMEOUT;		
		return one_linphone_linker.msg;
	}

	// 等待服务器业务回复2s
	if( sem_wait_ex2(pwaitrsp_sem, 12000) != 0 )
	{
		vd_printf("api_one_linphone_unlink_req,wait rsp ---have no rsp---\n");		
		one_linphone_linker.msg	= VD_LINPHONE_MSG_CLOSE_NO_RSP;		
		return one_linphone_linker.msg;
	}

	if( one_linphone_linker.msg == VD_LINPHONE_MSG_CLOSE_OK	)
		vd_printf("api_one_linphone_unlink_req ok!\n" );
	else
		vd_printf("api_one_linphone_unlink_req er!\n" );

	return one_linphone_linker.msg;
}

vd_linphone_msg_type api_one_linphone_unlink_rsp( int target_ip, vd_linphonec_linker_req_pack* punlinkreq, vd_linphone_rsp_result result)
{
	vd_linphonec_linker_rsp_pack	client_rsp; 

	// 命令组包
	client_rsp.target.ip		= target_ip;
	client_rsp.target.cmd		= LINPHONE_QUIT_RSP;
	client_rsp.target.id		= punlinkreq->target.id;
	client_rsp.result			= result;

	// 发送数据
	sem_t *presend_sem = one_udp_comm_trs_api( &one_linphone_linker.udp, client_rsp.target.ip, client_rsp.target.cmd, client_rsp.target.id,
												1,(char*)&client_rsp+sizeof(target_head), sizeof(vd_linphonec_linker_rsp_pack)-sizeof(target_head));
	
	vd_printf("api_one_linphone_unlink_rsp...\n");
	
	// 等待服务器通信应答
	if( sem_wait_ex2(presend_sem, 2000) != 0 )
	{
		vd_printf("api_one_linphone_unlink_rsp,wait ack --- timeout ---\n");
		one_linphone_linker.msg	= VD_LINPHONE_MSG_CLOSE_TIMEOUT;		
		return one_linphone_linker.msg;
	}
	
	vd_printf("api_one_linphone_unlink_rsp ok!\n" );

	one_linphone_linker.msg	= VD_LINPHONE_MSG_CLOSE_OK;	
	return one_linphone_linker.msg;	
}

