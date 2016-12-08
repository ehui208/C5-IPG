
#include "task_survey.h"
#include "sys_msg_process.h"
#include "obj_ip_mon_link.h"
#include "../video_service/ip_video_cs_control.h"
#include "../task_monitor/task_monitor.h"

#include "../vtk_udp_stack/vtk_udp_stack_c5_ipc_cmd.h"

#include "./obj_CallServer_Virtual/obj_CallServer_Virtual.h"	//czn_20160526
#include "obj_DeviceLinking.h"				//cao_20160601

#include "uart_udp_msg_process.h"

/****************************************************************************************************************************
 * @fn:		survey_sys_message_processing
 *
 * @brief:	公共服务消息处理
 *
 * @param:  pdata 			- 数据指针
 * @param:  len 			- 数据长度
 *
 * @return: 0/ok, 1/full
****************************************************************************************************************************/
int	survey_sys_message_processing( char* pdata, int len )
{
	VDP_MSG_HEAD* 	pMsg 		= (VDP_MSG_HEAD*)pdata;			// 内部消息头
	UDP_MSG_TYPE*	pUdpType	= (UDP_MSG_TYPE*)pdata;
	UART_MSG_TYPE*	pUartMsg	= (UART_MSG_TYPE*)pdata;
		
	// 处理hal的相关命令
	switch( pMsg->msg_source_id )
	{
		case MSG_ID_udp_stack:
			if( (pMsg->msg_sub_type == 0) && (pMsg->msg_type == 0) )
			{
				// udp 数据过滤
				UdpSurvey_Filter_Prcessing(pUdpType);			
				UdpSurvey_idle_Prcessing( pUdpType );
			}
			else if( (pMsg->msg_sub_type == 1) && (pMsg->msg_type == 1) )
			{
				udp_message_process(pUdpType->pbuf,pUdpType->len);
			}
			break;
			
		case MSG_ID_UART:
			uart_message_process( pUartMsg->pbuf, pUartMsg->len );
			break;
			
		default:
			Survey_SundryMsg_Process(pdata,len);	//czn_20160601
			break;
	}			
}

//czn_20160526
void Call_Survey_Unlink_Deal(UDP_MSG_TYPE *pUdpType)
{
	uint8 rspbuf[2];

	rspbuf[1] = 2;
	
	//czn_20160531
	/*if(Call_Survey_State == CALL_SURVEY_IDLE)
	{
		rspbuf[1] = 1;
	}
	else*/
	{
		//czn_20160606
		if(API_CallServer_Unlink() == 0)
		{
			//usleep(500000);
			Set_IPCallLink_Status(CLink_Idle);
			rspbuf[1] = 1;
		}
	}

	api_udp_c5_ipc_send_rsp(pUdpType->target_ip,0x1082,pUdpType->id,rspbuf,2);
	
}

// lzh_20160503_s

// lzh_20160503_s

// return: 0/ok, -1/err
int Interface_monitor_local_video(void)
{
	if( open_monitor_local() == 0 )
	{
		Call_Survey_State = CALL_SURVEY_MONITOR;
		return 0;
	}
	else
		return -1;
}

// return: 0/ok, -1/state err, 1/link err, 2/ng, 3/start remote video err
int Interface_monitor_remote_video(int ip , int dev_id )
{	
	int link_result;	// 0/ok, -1/设备异常，1/proxy ok, 2/设备不允许
	int remote_ip;
	unsigned char remote_dev_id;
	
	if( Call_Survey_State == CALL_SURVEY_IDLE )
	{
		link_result = send_ip_mon_link_req( ip, dev_id, &remote_ip, &remote_dev_id );
		// 允许监视
		if( link_result == 0 )
		{
			if( open_monitor_remote(remote_ip) == 0 )
			{
				dprintf("open_monitor_remote: mon link 0x%08x:%d req ok!\n",remote_ip,remote_dev_id);			
				Call_Survey_State = CALL_SURVEY_MONITOR;
				return 0;
			}
			else
			{
				return 3;
			}
		}
		// 设备错误
		else if( link_result == -1 )
		{
			dprintf("Interface_monitor_remote_video: mon link req er!\n");
			return 1;
		}
		// 不允许监视
		else
		{
			dprintf("Interface_monitor_remote_video: mon link req ng!\n");
			return 2;			
		}
	}
	else
	{
		return -1;
	}
}

// return: 0/ok, -1/state err, 1/stop remote video err
int Interface_monitor_video_close(void)
{
	if( Call_Survey_State != CALL_SURVEY_IDLE )
	{
		if( close_monitor() == 0 )
		{
			Call_Survey_State = CALL_SURVEY_IDLE;
			return 0;
		}
		else 
			return 1;
	}
	else
		return -1;
}

int API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(int ip)
{
	int 			mcg_addr;
	unsigned short  port;	
	get_server_multicast_addr_port2( ip, &mcg_addr, &port );	
	API_FromMulticastJoin( port, mcg_addr );
	return 0;
}

int API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(int ip)
{
	int 			mcg_addr;
	unsigned short  port;	
	get_server_multicast_addr_port2( ip, &mcg_addr, &port );	
	API_FromMulticastLeave( port, mcg_addr );
	return 0;
}

int API_VIDEO_S_SERVICE_TURN_ON_MULTICAST(int ip)
{
	int 			mcg_addr;
	unsigned short  port;
	get_server_multicast_addr_port( &mcg_addr, &port );	
	API_ToMulticastJoin( port, mcg_addr );	
	return 0;
}

int API_VIDEO_S_SERVICE_TURN_OFF_MULTICAST(int ip)
{
	int 			mcg_addr;
	unsigned short  port;
	get_server_multicast_addr_port( &mcg_addr, &port );	
	API_ToMulticastLeave( port, mcg_addr );	
	return 0;
}

//czn_20160526
int UdpSurvey_idle_Prcessing(UDP_MSG_TYPE *pUdpType)
{
	switch(pUdpType->cmd)
	{
		// 启动呼叫
		case 0x1003:
			//API_VtkUnicastCmd_Analyzer(pUdpType->target_ip,pUdpType->cmd,pUdpType->id,pUdpType->pbuf,pUdpType->len);
			break;
			
		// 启动监视
		case 0x1004:
			break;
			
		default:
			//API_VtkUnicastCmd_Analyzer(pUdpType->target_ip,pUdpType->cmd,pUdpType->id,pUdpType->pbuf,pUdpType->len);
			break;
	}
	return 0;
}

const char STR_FOR_DEVICE_TYPE[] = {"C5_IPG_1.0"};
const char STR_FOR_CODE_VERSION[] = {"V1.0.0"};

const char UadateFileDownloadPath[] = {"/mnt/nand1-1/"};	//czn_20160704

//czn_20160705_s
//czn_20160827_s
Updater_Run_Stru 	Updater_Run = 
{
	.configfile_cnt = 0,
	//.filename[0]={0}		
};
//czn_20160827_e
//czn_20160705_e

int UdpSurvey_Filter_Prcessing( UDP_MSG_TYPE *pUdpType )
{
	mon_link_request* 	pMonLink = (mon_link_request*)pUdpType->pbuf;
	UDP_Image_t*		pImageAdjust = (UDP_Image_t*)pUdpType->pbuf;

	// lzh_20160704_s
	fw_link_response			fw_link_rsp;	
	fw_download_start_request*	pdownload_start_req = (fw_download_start_request*)pUdpType->pbuf;
	fw_download_start_response	fw_download_start_rsp;

	fw_download_verify_request* pfw_download_verify_req = (fw_download_verify_request*)pUdpType->pbuf;
	fw_download_verify_response	fw_download_verify_rsp;
	
	fw_download_update_response	fw_download_update_rsp;
	int tftp_file_len,tftp_file_cks,update_return;	//czn_20160708
	
	// lzh_20160704_e
		
	switch( pUdpType->cmd )
	{
		// task_net_manage process
		case CMD_NET_MANAGE_RSP:
			push_vdp_common_queue(task_net_manange.p_msg_buf,pUdpType->pbuf,pUdpType->len);			
			break;

		case CMD_CAM_REMOTE_ADJUST_REQ:
			if( api_video_s_service_adjust(pImageAdjust) != -1 )
				pImageAdjust->state = 0;
			else
				pImageAdjust->state = 1;
			api_video_s_service_adjust_reply(pUdpType->target_ip,pImageAdjust);
			break;

#if 1			
		case CMD_CALL_MON_LINK_REQ:
			if( pMonLink->mon_type== 0x34 )
			{
				bprintf("Get_mon_Link_request\n");
				if( recv_ip_mon_link_req(pUdpType) == 0 )
				{
					bprintf("recv_ip_mon_link_req, send rsp ok!\n");
					API_DS_Camera_On();
				}
			}
			break;
		case CMD_CALL_MON_LINK_RSP: 		
			bprintf("Get_Call_Link_Respones\n");
			//Get_Call_Link_Respones(pUdpType);					
			// lzh_20160503_e
			break;
			
		case CMD_CALL_MON_UNLINK_REQ:
			bprintf("Call_Survey_Unlink_Deal\n");
			// lzh_20160503_s
			if( pMonLink->mon_type== 0x34 )
			{
				//ip_mon_command_process( pUdpType ); 			
			}
			else
			{
				//Call_Survey_Unlink_Deal(pUdpType);					
			}
			// lzh_20160503_e
			break;
		case CMD_CALL_MON_UNLINK_RSP:
			break;
#else
		case CMD_CALL_MON_LINK_REQ:
		case CMD_CALL_MON_LINK_RSP: 		
			if( pMonLink->mon_type== 0x34 )
			{
				bprintf("Get_mon_Link_request\n");
				if( recv_ip_mon_link_req(pUdpType) == 0 )
				{
					bprintf("recv_ip_mon_link_req, send rsp ok!\n");
					API_DS_Camera_On();
				}
			}
			else
			{
				bprintf("Get_Call_Link_Respones\n");
				Get_Call_Link_Respones(pUdpType); 				
			}
			// lzh_20160503_e
			break;
			
		case CMD_CALL_MON_UNLINK_REQ:
		case CMD_CALL_MON_UNLINK_RSP:
			bprintf("Call_Survey_Unlink_Deal\n");
			// lzh_20160503_s
			if( pMonLink->mon_type== 0x34 )
			{
				ip_mon_command_process( pUdpType );			
			}
			else
			{
				Call_Survey_Unlink_Deal(pUdpType);					
			}
			// lzh_20160503_e
			break;
#endif

			// lzh_20160704_s
			case CMD_DEVICE_TYPE_CODE_VER_REQ:
				bprintf("response CMD_DEVICE_TYPE_CODE_VER_REQ\n");
				fw_link_rsp.device_type_rsp 	= 1;
				fw_link_rsp.fireware_ver_rsp	= 1;
				memcpy( fw_link_rsp.device_type, STR_FOR_DEVICE_TYPE, sizeof(STR_FOR_DEVICE_TYPE)+1 );
				memcpy( fw_link_rsp.fireware_ver, STR_FOR_CODE_VERSION,sizeof(STR_FOR_CODE_VERSION)+1 );
				api_fireware_upgrade_cmd_send(pUdpType->target_ip,CMD_DEVICE_TYPE_CODE_VER_RSP,(char*)&fw_link_rsp);
				//czn_20160708_s
				Updater_Run.configfile_cnt = 0;
				//Updater_Run.filename[0]	= 0;	//czn_20160827
				system("rm -r /mnt/nand1-2/tftp");
				usleep(50000);
				system("mkdir /mnt/nand1-2/tftp");
				//czn_20160708_e
				break;
			
			case CMD_DEVICE_TYPE_DOWNLOAD_START_REQ:
				bprintf("response CMD_DEVICE_TYPE_DOWNLOAD_START_REQ\n");
				bprintf("tftp: ip = 0x%08x, filename = %s\n",pdownload_start_req->server_ip_addr,pdownload_start_req->filename);
				//fw_download_start_rsp.result = 0; 
				//api_fireware_upgrade_cmd_send(pUdpType->target_ip,CMD_DEVICE_TYPE_DOWNLOAD_START_RSP,(char*)&fw_download_start_rsp);
				// 启动tftp -g 192..... filename
				//czn_20160827_s
				if(Updater_Run.configfile_cnt == 0)
				{
					//API_Led_NormalLongIrregularFlash(LED_POWER);
				}
				if(start_tftp_download( pdownload_start_req->server_ip_addr, pdownload_start_req->filename ) != 0)
				{
					
					//API_Led_NormalOn(LED_POWER);
				}
				else
				{
					Updater_Run.resource_record[Updater_Run.configfile_cnt].rid = pdownload_start_req->file_type;
					strncpy(Updater_Run.resource_record[Updater_Run.configfile_cnt].filename,pdownload_start_req->filename,SERVER_FILE_NAME-1);
					Updater_Run.configfile_cnt ++;
				}
				//czn_20160827_e
				//czn_20160708_e
				break;
			case CMD_DEVICE_TYPE_DOWNLOAD_VERIFY_REQ:
				bprintf("response CMD_DEVICE_TYPE_DOWNLOAD_VERIFY_REQ\n");
				if( get_tftp_file_checksum( &tftp_file_len,&tftp_file_cks) == -1 )
				{
					bprintf("R-verifyE: len=0x%08x,cks=0x%08x\n",pfw_download_verify_req->file_len,pfw_download_verify_req->checksum);
					bprintf("L-verifyE: len=0x%08x,cks=0x%08x\n",tftp_file_len,tftp_file_cks);
					fw_download_verify_rsp.result = 1;
					api_fireware_upgrade_cmd_send(pUdpType->target_ip,CMD_DEVICE_TYPE_DOWNLOAD_VERIFY_RSP, (char*)&fw_download_verify_rsp );
				}
				else
				{
					bprintf("R-verify: len=0x%08x,cks=0x%08x\n",pfw_download_verify_req->file_len,pfw_download_verify_req->checksum);
					bprintf("L-verify: len=0x%08x,cks=0x%08x\n",tftp_file_len,tftp_file_cks);
					if( (pfw_download_verify_req->checksum == tftp_file_cks) && (pfw_download_verify_req->file_len== tftp_file_len) )
					{
						fw_download_verify_rsp.result = 0;
						api_fireware_upgrade_cmd_send(pUdpType->target_ip,CMD_DEVICE_TYPE_DOWNLOAD_VERIFY_RSP, (char*)&fw_download_verify_rsp );
					}
					else
					{
						fw_download_verify_rsp.result = 1;
						//API_Led_NormalOn(LED_POWER);		//czn_20160708
						api_fireware_upgrade_cmd_send(pUdpType->target_ip,CMD_DEVICE_TYPE_DOWNLOAD_VERIFY_RSP, (char*)&fw_download_verify_rsp );
					}
				}
				break;
				//czn_20160827_s
			case CMD_DEVICE_TYPE_UPDATE_START_REQ:	//czn_20160708
				bprintf("response CMD_DEVICE_TYPE_UPDATE_START_REQ\n");
				//czn_20160705_s
				//czn_20160708_s
				if(UpdateResourceTable() != 0)
				{
					//API_Led_NormalOn(LED_POWER);
					fw_download_update_rsp.result = -1;
					api_fireware_upgrade_cmd_send(pUdpType->target_ip,CMD_DEVICE_TYPE_UPDATE_START_RSP, (char*)&fw_download_update_rsp );
					break;
				}
				
				Updater_Run.server_ip_addr = pUdpType->target_ip;
				update_return = UpdateFwAndResource();
				
				//API_Led_NormalOn(LED_POWER);
				if( update_return == -1 )
				{	
					fw_download_update_rsp.result = -1;
					api_fireware_upgrade_cmd_send(pUdpType->target_ip,CMD_DEVICE_TYPE_UPDATE_START_RSP, (char*)&fw_download_update_rsp );
				}
				else if(update_return == 1) //reset_disable
				{	
					fw_download_update_rsp.result = 0;
					api_fireware_upgrade_cmd_send(pUdpType->target_ip,CMD_DEVICE_TYPE_UPDATE_START_RSP, (char*)&fw_download_update_rsp );
				}
				break;
		
		default:
			break;
	}
}
// lzh_20160503_e
//czn_20160601
void Survey_SundryMsg_Process(char* pdata, int len)
{
	VDP_MSG_HEAD* 	pMsg  = (VDP_MSG_HEAD*)pdata;
	int log_level;
	int log_char_offset1,log_char_offset2,log_char_offset3;
	char *pdetail;
	int	local_ip;
	//cao_20160601
	uint16	logicAddr;
	switch(pMsg->msg_type)
	{
		case SURVEY_MSG_CALLLINK_MANAGE:
			if(pMsg->msg_sub_type == SURVEY_SUBMSG_CALLLINK_WRITE_STATE)
			{
				Set_IPCallLink_Status(pdata[sizeof(VDP_MSG_HEAD)]);
				API_Send_BusinessRsp_ByUart(pMsg->msg_source_id,pMsg->msg_target_id,((pMsg->msg_type|0x80)<<8)|pMsg->msg_sub_type,NULL,0);
			}
			if(pMsg->msg_sub_type == SURVEY_SUBMSG_CALLLINK_READ_STATE)
			{
				unsigned char ipcalllink_status;
				ipcalllink_status = Get_IPCallLink_Status();
				API_Send_BusinessRsp_ByUart(pMsg->msg_source_id,pMsg->msg_target_id,((pMsg->msg_type|0x80)<<8)|pMsg->msg_sub_type,&ipcalllink_status,1);
			}
			if(pMsg->msg_sub_type == SURVEY_SUBMSG_CALLLINK_WRITE_CALLERDATA)
			{
				Set_IPCallLink_CallerData((Global_Addr_Stru *)(pdata+sizeof(VDP_MSG_HEAD)));
				API_Send_BusinessRsp_ByUart(pMsg->msg_source_id,pMsg->msg_target_id,((pMsg->msg_type|0x80)<<8)|pMsg->msg_sub_type,NULL,0);
			}
			if(pMsg->msg_sub_type == SURVEY_SUBMSG_CALLLINK_WRITE_BECALLEDDATA)
			{
				Set_IPCallLink_BeCalledData((Global_Addr_Stru *)(pdata+sizeof(VDP_MSG_HEAD)));
				API_Send_BusinessRsp_ByUart(pMsg->msg_source_id,pMsg->msg_target_id,((pMsg->msg_type|0x80)<<8)|pMsg->msg_sub_type,NULL,0);
			}
			if(pMsg->msg_sub_type == SURVEY_SUBMSG_CALLLINK_WRITE_TRANSFERDATA)
			{
				Set_IPCallLink_TransferredData((Global_Addr_Stru *)(pdata+sizeof(VDP_MSG_HEAD)));
				API_Send_BusinessRsp_ByUart(pMsg->msg_source_id,pMsg->msg_target_id,((pMsg->msg_type|0x80)<<8)|pMsg->msg_sub_type,NULL,0);
			}
			break;

		case SURVEY_MSG_LOG_WRITE:
			log_level = *((int *)(pdata+sizeof(VDP_MSG_HEAD)));
			
			log_char_offset1 = sizeof(VDP_MSG_HEAD) + 4;
			log_char_offset2 = strlen(pdata+log_char_offset1) + log_char_offset1 + 1;
			log_char_offset3 = strlen(pdata+log_char_offset2) + log_char_offset2 + 1;
			//printf("--------1LOG_WRITE %d %s %s--%d %d %d-------\n",log_level,pdata + log_char_offset1,pdata + log_char_offset2,pdata[log_char_offset3],log_char_offset3,len);
			
			if(pdata[log_char_offset3] == '\0')
			{
				if(log_char_offset3 >= len)
				{
					break;
				}
				pdetail = NULL;
			}
			else
			{
				if((strlen(pdata+log_char_offset3) + log_char_offset3 + 1) > len)
				{
					break;
				}
				pdetail = pdata+log_char_offset3;
			}

			
			bprintf("write log:level=%d,%s:%s\n",log_level,pdata + log_char_offset1,pdata + log_char_offset2);

			API_add_log_item(log_level,pdata + log_char_offset1,pdata + log_char_offset2,pdetail);
			
			break;
		case SURVEY_MSG_GETLOCALIP:
			local_ip = GetLocalIp();
			API_Send_BusinessRsp_ByUart(pMsg->msg_source_id,pMsg->msg_target_id,((pMsg->msg_type|0x80)<<8)|pMsg->msg_sub_type,(unsigned char*)&local_ip,4);
			break;
		//cao_20160601			
		case UATR_MSG_TYPE_LINKING:
			logicAddr = pdata[5];
			logicAddr <<= 8;
			logicAddr += pdata[4];
			
			LinkingDeviceResponse(logicAddr, pdata[6]);
			break;	

		case SURVEY_MSG_VIDEO_SERVICE_OFF:
			API_DS_Camera_Off();
			break;
	}
}


