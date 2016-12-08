
#include "../task_survey/task_survey.h"
#include "../task_debug_sbu/task_debug_sbu.h"
#include "../task_io_server/task_IoServer.h"
#include "../task_io_server/vdp_IoServer_Data.h"
#include "../task_io_server/vdp_IoServer_State.h"
#include "../vdp_uart.h"
#include "vdp_net_manange.h"
#include "vdp_net_manange_ip_list.h"

Loop_vdp_common_buffer	vdp_net_manange_mesg_queue;
Loop_vdp_common_buffer	vdp_net_manange_sync_queue;
vdp_task_t				task_net_manange;

int net_manange_Uart_Dip_Set_IP_Reply(unsigned char msg_id, unsigned char result);

void vdp_net_manange_mesg_data_process(char* msg_data, int len);
void* vdp_net_manange_task( void* arg );
int ipg_reset_ok_notify(void);

void vtk_TaskInit_net_manang(void)
{
	init_vdp_common_queue(&vdp_net_manange_mesg_queue, 500, (msg_process)vdp_net_manange_mesg_data_process, &task_net_manange);
	init_vdp_common_queue(&vdp_net_manange_sync_queue, 100, NULL, 								  		&task_net_manange);
	init_vdp_common_task(&task_net_manange, MSG_ID_NetManage, vdp_net_manange_task, &vdp_net_manange_mesg_queue, &vdp_net_manange_sync_queue);
}

void exit_vdp_net_manange_task(void)
{
	exit_vdp_common_queue(&vdp_net_manange_mesg_queue);
	exit_vdp_common_queue(&vdp_net_manange_sync_queue);
	exit_vdp_common_task(&task_net_manange);
}

void* vdp_net_manange_task( void* arg )
{
	vdp_task_t*	 ptask 			= (vdp_task_t*)arg;
	p_vdp_common_buffer pdb 	= 0;
	int	size;

	usleep(30000);			// 延时一会，等待io线程ok	

	while( ptask->task_run_flag )
	{
		size = pop_vdp_common_queue( ptask->p_msg_buf, &pdb, VDP_QUEUE_POLLING_TIME);
		if( size > 0 )
		{
			(*ptask->p_msg_buf->process)(pdb,size);
			purge_vdp_common_queue( ptask->p_msg_buf );
		}
	}
	return 0;
}

void vdp_net_manange_mesg_data_process(char* msg_data,int len)
{
	int ret;
	int ip;
	NetManange_Para_type*			pNetManange 		= (NetManange_Para_type*)msg_data;
	NetManange_Link_type*	 		pNetLink 			= (NetManange_Link_type*)msg_data;
	NetManange_ipg_list_update*		pNetIPGListUpdate 	= (NetManange_ipg_list_update*)msg_data;
	NetManange_ipg_list_read*		pNetIpgListRead 	= (NetManange_ipg_list_read*)msg_data;
	NetManange_ipg_online_check*	pNetOnLineCheck 	= (NetManange_ipg_online_check*)msg_data; 
	NetManange_net_test*			pNetTest 			= (NetManange_net_test*)msg_data;
	NetManange_ipg_repeat_check* 	pNetIPGRepeat 		= (NetManange_ipg_repeat_check*)msg_data;
	NetManange_ipg_list_report*		pIPGListReport 		= (NetManange_ipg_list_report*)msg_data;	
	
	switch( pNetManange->head.msg_type&(~COMMON_RESPONSE_BIT) )
	{
		// uart: 请求设置dip开关命令
		case VDP_NET_MANANGE_MSG_DIP_SET_IP:
			break;

		// udp: 请求或回复更新ipg列表
		case VDP_NET_MANANGE_REMOTE_IPG_LIST_UPDATE_REQ:
			// udp cmd process
			if( !(pNetLink->head.msg_type&COMMON_RESPONSE_BIT) )
			{
				bprintf("rec udp request ipg list update\n");
			
				// 接收到ipg list的请求命令后给出应答
				API_IPG_List_Update_Reply(pNetIPGListUpdate->ip,pNetIPGListUpdate->head.msg_source_id,pNetIPGListUpdate->head.msg_target_id,pNetIPGListUpdate->head.msg_type,pNetIPGListUpdate->head.msg_sub_type);
				
				bprintf("snd udp reply ipg list update, target ip = %08x\n",pNetIPGListUpdate->ip);
			}
			else
			{
				bprintf("get udp reply ipg list update, target ip = %08x, source ip = %s\n",pIPGListReport->asker, pIPGListReport->ip);
				
				// 接收到ipg list的回复数据包后添加到数据库中
				API_IPG_List_Update_Get_Node( pIPGListReport );
				
				bprintf("add one ipg list node\n");
			}
			
			
			break;
			
		// uart: 请求更新ipg列表
		case VDP_NET_MANANGE_MSG_IPG_LIST_UPDATE_REQ:
			// uart cmd process
			bprintf("rec uart request ipg list update\n");

			bprintf("snd boardcast cmd to udp, wait for 2s...\n");
			
			if( API_IPG_List_Update_Request( pNetIPGListUpdate->head.msg_target_id, pNetIPGListUpdate->head.msg_source_id, VDP_NET_MANANGE_MSG_IPG_LIST_UPDATE_REQ, 0 ) >= 0 )
			{						
				bprintf("request list cmd boardcast ok\n");
				
				//API_net_manange_Uart_IPG_List_Update_Reply(pNetIPGListUpdate->head.msg_source_id, 1);	
				
				bprintf("reply uart ipg list update ok\n");
			}			
			else
			{
				//API_net_manange_Uart_IPG_List_Update_Reply(pNetIPGListUpdate->head.msg_source_id, 0);	
				bprintf("reply uart ipg list already in updating\n");
			}	
			break;

		// uart: 请求读取ipg列表			
		case VDP_NET_MANANGE_MSG_IPG_LIST_READ_REQ:
			
			bprintf("rec uart request ipg list node, list off = %d, list cnt = %d\n", pNetIpgListRead->list_off, pNetIpgListRead->list_cnt);

			API_net_manange_Uart_IPG_List_Read_Reply( pNetIpgListRead );
				
			bprintf("snd uart reply ipg list node,  list off = %d, list cnt = %d\n", pNetIpgListRead->list_off, pNetIpgListRead->list_cnt);
			break;

		// udp: 请求ipg在线检测:
		case VDP_NET_MANANGE_REMOTE_IPG_ONLINE_CHECK:
			// udp cmd process			
			if( !(pNetLink->head.msg_type&COMMON_RESPONSE_BIT) )
			{
				bprintf("rec udp request ip = %08x on line check\n",pNetOnLineCheck->targetIP);
			
				//  回复在线情况
				API_net_manange_Udp_OnLine_Check_Reply(pNetOnLineCheck->head.msg_source_id, pNetOnLineCheck->targetIP, 1);

				bprintf("snd udp reply ip = %08x on line check ok, result = %d\n",pNetOnLineCheck->targetIP, 1);
				
				// 同步发送请求到stm32l, 通知其亮灯指示等操作			
				//API_net_manange_Uart_OnLine_Check_Request(MSG_ID_NetManage, pNetOnLineCheck->targetIP);

				bprintf("notify stm32l flash on line lamp\n");
			}
			break;
			
		// uart: 请求ipg在线检测:
		case VDP_NET_MANANGE_MSG_IPG_ONLINE_CHECK:
			// uart cmd process
			bprintf("rec uart request ip = %08x on line check\n",pNetOnLineCheck->targetIP);
			
			if( !(pNetManange->head.msg_type&COMMON_RESPONSE_BIT) ) 
			{
				// 判断是否为本机地址
				if( pNetOnLineCheck->targetIP == GetLocalIp() )
				{
					API_net_manange_Uart_OnLine_Check_Reply( pNetOnLineCheck->head.msg_source_id, pNetOnLineCheck->targetIP, 1 );
					
					bprintf("uart reply ip = %08x on line check err, ip just local!\n", pNetOnLineCheck->targetIP);
				}
				else
				{
					bprintf("start udp request ip = %08x on line check\n",pNetOnLineCheck->targetIP);
					
					//  send udp data package
					ret =  API_net_manange_Udp_OnLine_Check_Request(pNetOnLineCheck->head.msg_target_id, pNetOnLineCheck->targetIP );

					bprintf("get udp reply on line check result = %d\n", ret );
					
					if( ret >0 )
					{
						if( ret == 1 )							
						{						
							API_net_manange_Uart_OnLine_Check_Reply( pNetOnLineCheck->head.msg_source_id, pNetOnLineCheck->targetIP, 1 );
							
							bprintf("uart reply ip = %08x on line check ok, result = %d\n",pNetOnLineCheck->targetIP, 1);
						}
						else
						{
							API_net_manange_Uart_OnLine_Check_Reply( pNetOnLineCheck->head.msg_source_id, pNetOnLineCheck->targetIP, 0 );
							
							bprintf("uart reply ip = %08x on line check er, result = %d\n",pNetOnLineCheck->targetIP, 0);
						}
					}
					else
					{
						API_net_manange_Uart_OnLine_Check_Reply( pNetOnLineCheck->head.msg_source_id, pNetOnLineCheck->targetIP, 0 );

						bprintf("uart reply ip = %08x on line check timeout, result = %d\n",pNetOnLineCheck->targetIP, 0);
					}
				}
			}
			break;

		case VDP_NET_MANANGE_REMOTE_IPG_REPEAT_CHECK:
			// udp cmd process
			if( !(pNetLink->head.msg_type&COMMON_RESPONSE_BIT) )
			{
				// 判断广播命令是否与本机地址相同, 相等的回复数据1
				ip = GetLocalIp();
				if( pNetIPGRepeat->targetIP == ip )
				{
					API_net_manange_Udp_IPG_Repeat_Reply( pNetIPGRepeat->head.msg_source_id, pNetIPGRepeat->targetIP, 1 );
				}
			}
			break;
			
		case VDP_NET_MANANGE_MSG_IPG_REPEAT_CHECK:
			// uart cmd process
			ret = API_net_manange_Udp_IPG_Repeat_Request(pNetIPGRepeat->head.msg_target_id);
			if( ret >0 )
			{
				if( ret == 1 )
					API_net_manange_Uart_IPG_Repeat_Reply( pNetOnLineCheck->head.msg_source_id, pNetOnLineCheck->targetIP, 1 );
				else
					API_net_manange_Uart_IPG_Repeat_Reply( pNetOnLineCheck->head.msg_source_id, pNetOnLineCheck->targetIP, 0 );
			}
			else
			{
				API_net_manange_Uart_IPG_Repeat_Reply( pNetOnLineCheck->head.msg_source_id, pNetOnLineCheck->targetIP, 0 );				
			}
			break;

		case VDP_NET_MANANGE_MSG_NET_TEST:
			bprintf("rec uart request network quality\n");			
			break;	

		case VDP_NET_MANANGE_MSG_NET_RESET:			
			break;
		
		// udp接收到link请求或link回复
		case VDP_NET_MANANGE_MSG_COMMON_LINK:
			#if 0
			// 判断link是请求还是应答
			if( pNetLink->head.msg_type&COMMON_RESPONSE_BIT )
			{
				bprintf("net manange: rec link reply = %d\n", pNetLink->link_state );
			}
			else
			{
				bprintf("becalled get request link start...\n" );
		
				device_check.head.msg_source_id = MSG_ID_NetManage;
				device_check.head.msg_sub_type	= pNetLink->head.msg_sub_type;
		
				device_check.call_type			= pNetLink->call_type;
				device_check.para_type			= PARA_TYPE_T_IPDEVID_S_IPDEVID;
				
				device_check.tdev.tipdev.tip	= pNetLink->targetIP;
				device_check.tdev.tipdev.tdev	= pNetLink->targetDevID;
				device_check.sdev.sipdev.sip	= pNetLink->sourcetIP;
				device_check.sdev.sipdev.sdev	= pNetLink->targetDevID;
				
				// 必须申请设备的状态 0/device err, 1/device ok
		
				bprintf("becalled request device check = %02x\n", device_check.tdev.tipdev.tdev );
				
				if( API_call_survey_device_check_request( &device_check ) > 0 )
				{
					bprintf("get device check result = %d\n", device_check.result);
					
					API_net_manange_Udp_Link_Reply(pNetLink->head.msg_source_id, pNetLink->call_type, pNetLink->sourcetIP, pNetLink->sourceDevID, pNetLink->targetDevID, device_check.result);
					
					bprintf("reply link result = %d\n", device_check.result);
				}
				else
				{
					bprintf("get device check timeout!\n");
					
					// 2/timeout
					API_net_manange_Udp_Link_Reply(pNetLink->head.msg_source_id, pNetLink->call_type, pNetLink->sourcetIP, pNetLink->sourceDevID, pNetLink->targetDevID, 2);
		
					bprintf("reply link result = 2\n");
				}
			}
			#endif
			break;			
	}
}

/*******************************************************************************************
 * @fn:		ipg_reset_ok_notify
 *
 * @brief:	ipg上电复位后发送ok命令到stm32l
 *
 * @param:   	none
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int ipg_reset_ok_notify(void)
{		
	int len;
	int ret;
	int sendcnt;

	NetManange_reset_ok	NetManange;
	
	NetManange.head.msg_target_id	= MSG_ID_NetManage;
	NetManange.head.msg_source_id	= MSG_ID_NetManage;
	NetManange.head.msg_type 		= VDP_NET_MANANGE_MSG_IPG_RESET_OK;
	NetManange.head.msg_sub_type 	= 0;	
	NetManange.msg_data				= 1;
	
	vdp_task_t* pTask = GetTaskAccordingMsgID(MSG_ID_NetManage);

	dprintf("net manange inner send reset ok start = %d\n",pTask->task_id);

	if( pTask != NULL )
	{
		for( sendcnt = 2; sendcnt >0; sendcnt-- )
		{	
			vdp_uart_send_data((char*)&NetManange, sizeof(NetManange_reset_ok));
			// 等待业务应答
			ret = WaitForBusinessACK( pTask->p_syc_buf, VDP_NET_MANANGE_MSG_IPG_RESET_OK, (char*)&NetManange, &len, 5000 );
			if( ret > 0 )
			{		
				break;
			}
		}
	}
	
	dprintf("net manange inner send reset ok over, ret = %d!\n", ret);
	
	return ret;	
}

/*******************************************************************************************
 * @fn:		net_manange_Uart_Dip_Set_IP_Reply
 *
 * @brief:	通过uart端口回复请求的结果
 *
 * @param:  	result - 0/ok, 1/err
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int net_manange_Uart_Dip_Set_IP_Reply(unsigned char msg_id, unsigned char result)
{		
	NetManange_Para_type	 NetManange;
	
	NetManange.head.msg_target_id 		= msg_id;
	NetManange.head.msg_source_id		= MSG_ID_NetManage;
	NetManange.head.msg_type			= (VDP_NET_MANANGE_MSG_DIP_SET_IP|COMMON_RESPONSE_BIT);
	NetManange.head.msg_sub_type		= 0;
	
	NetManange.msg_data.data			= result;
	
	vdp_uart_send_data((char*)&NetManange, sizeof(NetManange_Para_type));
	return 0;	
}



/*******************************************************************************************
 * @fn:		API_net_manange_Udp_Link_Request
 *
 * @brief:	发送通用link指令到网络
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 	- 目标ip地址
 * @param:  	tdev_id 		- 目标设备地址
 * @param:  	sdev_id 		- 源设备地址
 *
 * @return: 	-1/err, other/state		// 连接设备0/ok, 1/ipg rsp err, 2/dev rsp err, 3/data err
*******************************************************************************************/
int API_net_manange_Udp_Link_Request( unsigned char call_type, int target_ip, unsigned char tdev_id, unsigned char sdev_id )
{
	NetManange_Link_type NetManange;
	NetManange_Link_type RspNetManange;
	unsigned int len = sizeof(NetManange_Link_type);
	int ret;

	NetManange.head.msg_source_id 	= GetMsgIDAccordingPid(pthread_self());
	NetManange.head.msg_target_id	= MSG_ID_NetManage;
	NetManange.head.msg_type		= VDP_NET_MANANGE_REMOTE_COMMON_LINK;
	NetManange.head.msg_sub_type	= 0;	

	NetManange.targetIP 			= target_ip;
	NetManange.sourcetIP			= GetLocalIp();
	NetManange.targetDevID			= tdev_id;
	NetManange.sourceDevID			= sdev_id;
	
	NetManange.call_type			= call_type;
	NetManange.link_state			= 0;
	
	dprintf("net manange udp link to 0x%08x start...\n",target_ip);
	
	// 发送link消息
	ret = api_udp_c5_ipc_send_req( NetManange.targetIP, CMD_NET_MANAGE_REQ, (char*)&NetManange, len, (char*)&RspNetManange, &len );
	if( ret == 0 )
	{				
		if( RspNetManange.head.msg_type == (VDP_NET_MANANGE_REMOTE_COMMON_LINK|COMMON_RESPONSE_BIT) )
		{
			// 0/device err, 1/device ok, 2/timeout
			dprintf("net manange link state = %d\n",RspNetManange.link_state);
			if( RspNetManange.link_state == 0 )
				ret = 2;	//ipg ok, device err
			else if( RspNetManange.link_state == 1 )
				ret = 0;	//ipg ok, device ok
			else
				ret = 2;	//ipg ok, device err
		}
		else
			ret = 1;	// data err
	}
	else
	{
		ret = -1;	// ipg rsp timeout err
		dprintf("net manange link state err!\n");
	}
	
	dprintf("net manange udp link over!\n");
	
	return ret;	
	
}

/*******************************************************************************************
 * @fn:		API_net_manange_Udp_Link_Reply
 *
 * @brief:	发送通用link回复指令到网络
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 	- 目标ip地址
 * @param:  	tdev_id 		- 目标设备地址
 * @param:  	sdev_id 		- 源设备地址
 * @param:  	state 		- 连接状态1/ok, 0/no device
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Udp_Link_Reply( unsigned char msg_id, unsigned char call_type,  int target_ip, unsigned char tdev_id, unsigned char sdev_id, unsigned char state )
{
	NetManange_Link_type NetManange;
	NetManange.head.msg_target_id	= msg_id;
	NetManange.head.msg_source_id	= MSG_ID_NetManage;
	NetManange.head.msg_type		= VDP_NET_MANANGE_REMOTE_COMMON_LINK|COMMON_RESPONSE_BIT;
	NetManange.head.msg_sub_type	= 0;
		
	NetManange.targetIP 			= target_ip;
	NetManange.sourcetIP			= GetLocalIp();
	NetManange.targetDevID			= tdev_id;
	NetManange.sourceDevID			= sdev_id;

	NetManange.call_type			= call_type;	
	NetManange.link_state			= state;
	
	if( api_udp_c5_ipc_send_data(target_ip, CMD_NET_MANAGE_RSP, (char*)&NetManange, sizeof(NetManange) ) == 0 )	
	{
		dprintf("net manange link reply\n");
		return 0;
	}
	else
	{
		dprintf("net manange link reply er\n");		
		return -1;
	}
}


/*******************************************************************************************
 * @fn:		API_net_manange_Uart_IPG_List_Read_Request
 *
 * @brief:	回复发送请求ipg list的数据
 *
 * @param:  	pNetManage - 请求方的消息
 *
 * @return: 	-1/err, other/state
*******************************************************************************************/
int API_net_manange_Uart_IPG_List_Read_Reply( NetManange_ipg_list_read* pNetManage )
{
	//ipg_list_node list_tab[IPG_NUM_ONE_PACKAGE];
	
	NetManange_ipg_list_read NetManage;

	memcpy( (char*)&NetManage, (char*)pNetManage, sizeof(NetManange_ipg_list_read) );

	if( NetManage.list_cnt > IPG_NUM_ONE_PACKAGE ) NetManage.list_cnt = IPG_NUM_ONE_PACKAGE;
	
	NetManage.head.msg_target_id	= pNetManage->head.msg_source_id;
	NetManage.head.msg_source_id	= MSG_ID_NetManage;
	NetManage.head.msg_type			= VDP_NET_MANANGE_MSG_IPG_LIST_READ_REQ|COMMON_RESPONSE_BIT;
	NetManage.list_max				= API_IPG_List_Report_Max_Number();
	NetManage.list_cnt 				= API_IPG_List_Report_Data(NetManage.list_off, NetManage.list_cnt,NetManage.list_node);
	
	//if( NetManage.list_max < (NetManage.list_off + NetManage.list_cnt ) )
	//	NetManage.list_cnt = NetManage.list_max - NetManage.list_off;

	dprintf("net manange ipg list read reply\n");

	vdp_uart_send_data((char*)&NetManage, sizeof(NetManange_ipg_list_read) - sizeof(ipg_list_node)*(IPG_NUM_ONE_PACKAGE -NetManage.list_cnt) );
	
	return 0;	
	
}


/*******************************************************************************************
 * @fn:		API_net_manange_Udp_OnLine_Check_Reply
 *
 * @brief:	uart 回复设备检测在线
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 		- 目标ip地址
 * @param:  	state 		- 在线状态
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Uart_OnLine_Check_Reply( unsigned char msg_id, int target_ip, unsigned char state )
{
	NetManange_ipg_online_check NetManange;

	NetManange.head.msg_target_id	= msg_id;
	NetManange.head.msg_source_id	= MSG_ID_NetManage;
	NetManange.head.msg_type		= VDP_NET_MANANGE_MSG_IPG_ONLINE_CHECK|COMMON_RESPONSE_BIT;
	NetManange.head.msg_sub_type	= 0;
		
	NetManange.targetIP 			= target_ip;	
	NetManange.online_state			= state;
	
	dprintf("net manange online check  uart reply\n");

	vdp_uart_send_data((char*)&NetManange, sizeof(NetManange_ipg_online_check) );
}

/*******************************************************************************************
 * @fn:		API_net_manange_Udp_OnLine_Check_Request
 *
 * @brief:	发送通用在线监测命令
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 		- 目标ip地址
 *
 * @return: 	-1/err, other/state
*******************************************************************************************/
int API_net_manange_Udp_OnLine_Check_Request( unsigned char msg_id,   int target_ip )
{
	NetManange_ipg_online_check NetManange;
	int len = sizeof(NetManange_ipg_online_check);
	int ret;

	NetManange.head.msg_target_id	= MSG_ID_NetManage;
	NetManange.head.msg_source_id	= msg_id;
	NetManange.head.msg_type		= VDP_NET_MANANGE_REMOTE_IPG_ONLINE_CHECK;
	NetManange.head.msg_sub_type	= 0;	
	
	NetManange.targetIP 			= GetLocalIp();	// 加上发送端地址
	NetManange.online_state			= 0;
	
	vdp_task_t* pTask = GetTaskAccordingMsgID(msg_id);

	dprintf("net manange udp online check task id = %d\n",pTask->task_id);

	ret = -1;
	
	if( pTask != NULL )
	{
		// 发送link消息
		if( api_udp_c5_ipc_send_req(target_ip, CMD_NET_MANAGE_RSP, (char*)&NetManange, len, (char*)&NetManange, &len) == 0 )
		{				
			dprintf("net manange online udp check state = %d\n",NetManange.online_state);
			ret = NetManange.online_state;
		}
		else
		{
			ret = -1;
			dprintf("net manange online udp check state err!\n");
		}
	}
	
	dprintf("net manange udp oneline check over!\n");
	
	return ret;		
}

/*******************************************************************************************
 * @fn:		API_net_manange_Udp_OnLine_Check_Reply
 *
 * @brief:	发送通用link回复指令到网络
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 		- 目标ip地址
 * @param:  	state 		- 在线状态
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Udp_OnLine_Check_Reply( unsigned char msg_id, int target_ip, unsigned char state )
{
	NetManange_ipg_online_check NetManange;

	NetManange.head.msg_target_id	= msg_id;
	NetManange.head.msg_source_id	= MSG_ID_NetManage;
	NetManange.head.msg_type		= VDP_NET_MANANGE_REMOTE_IPG_ONLINE_CHECK|COMMON_RESPONSE_BIT;
	NetManange.head.msg_sub_type	= 0;
		
	NetManange.targetIP 			= GetLocalIp();	// 加上发送端地址
	NetManange.online_state			= state;
	
	dprintf("net manange online check udp reply\n");

	return api_udp_c5_ipc_send_data(target_ip, CMD_NET_MANAGE_RSP, (char*)&NetManange, sizeof(NetManange));
}

/*******************************************************************************************
 * @fn:		API_net_manange_Udp_OnLine_Check_Reply
 *
 * @brief:	uart 回复设备检测在线
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 		- 目标ip地址
 * @param:  	state 		- 在线状态
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Uart_IPG_Repeat_Reply( unsigned char msg_id, int target_ip, unsigned char state )
{
	NetManange_ipg_repeat_check NetManange;

	NetManange.head.msg_target_id	= msg_id;
	NetManange.head.msg_source_id	= MSG_ID_NetManage;
	NetManange.head.msg_type		= VDP_NET_MANANGE_MSG_IPG_REPEAT_CHECK|COMMON_RESPONSE_BIT;
	NetManange.head.msg_sub_type	= 0;
		
	NetManange.targetIP 			= target_ip;	
	NetManange.repeat_state			= state;
	
	dprintf("net manange ipg repeat check uart reply\n");

	vdp_uart_send_data((char*)&NetManange, sizeof(NetManange_ipg_online_check) );
}

/*******************************************************************************************
 * @fn:		API_net_manange_Udp_IPG_Repeat_Request
 *
 * @brief:	发送ipg 重复在线检测
 *
 * @param:  	msg_id 		- 请求方的消息id
 *
 * @return: 	-1/err, other/state
*******************************************************************************************/
int API_net_manange_Udp_IPG_Repeat_Request( unsigned char msg_id )
{
	NetManange_ipg_repeat_check NetManange;
	int len;
	int ret;

	NetManange.head.msg_target_id	= MSG_ID_NetManage;
	NetManange.head.msg_source_id	= msg_id;
	NetManange.head.msg_type		= VDP_NET_MANANGE_REMOTE_IPG_REPEAT_CHECK;
	NetManange.head.msg_sub_type	= 0;	
	
	NetManange.targetIP 			= GetLocalIp(); 	//target_ip;
	NetManange.repeat_state		= 0;
	
	if( check_ip_repeat(NetManange.targetIP) == 1 )
		ret = 1;
	else
		ret = 0;
	
	dprintf("net manange udp oneline check over!\n");
	
	return ret;		
}

/*******************************************************************************************
 * @fn:		API_net_manange_Udp_IPG_Repeat_Reply
 *
 * @brief:	回复ipg repeat
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	target_ip 		- 目标ip地址
 * @param:  	state 		- 在线状态
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Udp_IPG_Repeat_Reply( unsigned char msg_id, int target_ip, unsigned char state )
{
	NetManange_ipg_repeat_check NetManange;

	NetManange.head.msg_target_id	= msg_id;
	NetManange.head.msg_source_id	= MSG_ID_NetManage;
	NetManange.head.msg_type		= VDP_NET_MANANGE_REMOTE_IPG_REPEAT_CHECK|COMMON_RESPONSE_BIT;
	NetManange.head.msg_sub_type	= 0;
		
	NetManange.targetIP 			= GetLocalIp(); 	//target_ip;	
	NetManange.repeat_state			= state;
	
	dprintf("net manange ipg repeatcheck udp reply\n");

	api_udp_c5_ipc_send_data(target_ip, CMD_NET_MANAGE_RSP, (char*)&NetManange, sizeof(NetManange) );

	return 0;
}

/*******************************************************************************************
 * @fn:		API_net_manange_Uart_Net_Test_Reply
 *
 * @brief:	uart 回复设备检测在线
 *
 * @param:  	msg_id 		- 请求方的消息id
 * @param:  	state 		- 网络质量
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_net_manange_Uart_Net_Test_Reply( unsigned char msg_id, unsigned char state )
{
	NetManange_net_test NetManange;

	NetManange.head.msg_target_id	= msg_id;
	NetManange.head.msg_source_id	= MSG_ID_NetManage;
	NetManange.head.msg_type		= VDP_NET_MANANGE_MSG_NET_TEST|COMMON_RESPONSE_BIT;
	NetManange.head.msg_sub_type	= 0;
		
	NetManange.net_state			= state;
	
	dprintf("net manange net test uart reply\n");

	vdp_uart_send_data((char*)&NetManange, sizeof(NetManange_net_test) );
}


