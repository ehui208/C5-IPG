

#include "task_debug_sbu.h"
#include "logdoc.h"
#include "../task_io_server/task_IoServer.h"

#include "../vtk_udp_stack/vtk_udp_stack_c5_ipc_cmd.h"

int debug_log_doc_level = 8;

Loop_vdp_common_buffer	vdp_debug_sbu_inner_queue;
Loop_vdp_common_buffer	vdp_debug_sbu_sync_queue;
vdp_task_t				task_debug_sbu;

void vdp_debug_sbu_inner_data_process(char* msg_data, int len);
void* vdp_debug_sbu_task( void* arg );
	
void vtk_TaskInit_debug_sbu(void)
{	
	InitialLogFileLines();

	init_vdp_common_queue(&vdp_debug_sbu_inner_queue, 500, (msg_process)vdp_debug_sbu_inner_data_process, &task_debug_sbu);
	init_vdp_common_queue(&vdp_debug_sbu_sync_queue, 200, NULL, 								  		&task_debug_sbu);
	init_vdp_common_task(&task_debug_sbu, MSG_ID_DEBUG_SBU, vdp_debug_sbu_task, &vdp_debug_sbu_inner_queue, &vdp_debug_sbu_sync_queue);

	dprintf("vdp_debug_sbu_task starting............\n");
}

void exit_vdp_debug_sbu_task(void)
{
	exit_vdp_common_queue(&vdp_debug_sbu_inner_queue);
	exit_vdp_common_task(&task_debug_sbu);
}

//#define WRITE_LOG_DOC_ENABLE

void* vdp_debug_sbu_task( void* arg )
{
	vdp_task_t*	 ptask 			= (vdp_task_t*)arg;
	p_vdp_common_buffer pdb 	= 0;
	int	size;

	while( ptask->task_run_flag )
	{
		size = pop_vdp_common_queue( ptask->p_msg_buf, &pdb, VDP_QUEUE_POLLING_TIME);
		if( size > 0 )
		{
			#ifdef WRITE_LOG_DOC_ENABLE
			// 检测到有数据，则打开log文件
			OpenLogFile();
			#endif
			
			(*ptask->p_msg_buf->process)(pdb,size);
			purge_vdp_common_queue( ptask->p_msg_buf );
			
			#ifdef WRITE_LOG_DOC_ENABLE
			while(1)
			{
				size = pop_vdp_common_queue( ptask->p_msg_buf, &pdb, 10);
				if( size > 0 )
				{
					(*ptask->p_msg_buf->process)(pdb,size);
					purge_vdp_common_queue( ptask->p_msg_buf );
				}
				else
					break;
			}
			// 写入全部缓冲数据后关闭文件
			CloseLogFile();
			// 判断文件是否超长
			JustifyLogFileLength();			
			#endif
		}
	}
	return 0;
}

int send_ip_mon_link_req( int target_ip, unsigned char dev_id, int* premote_ip, unsigned char* pdev_id  );
int API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(int ip);
int API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(int ip);

void vdp_debug_sbu_inner_data_process(char* msg_data,int len)
{
	int desc_len,remain_len;
	char *pviewdat = (msg_data+1);
	desc_len = msg_data[0];			// 得到描叙的长度

	vdp_debug_type		*pDebug 	= (vdp_debug_type*)msg_data;
	vdp_debug_key_type 	*pDebugKey 	= (vdp_debug_key_type*)msg_data;
	vdp_debug_key_type 	rsp_msg;

	int					result;
	int 				remote_ip;
	unsigned char 		dev_id;

	fw_link_request			fw_link;
		
	switch( pDebug->head.msg_type&(~COMMON_RESPONSE_BIT) )
	{
		case SUB_ID_DEBUG_PH_KEY:						
			// reply
			rsp_msg.head.msg_target_id		= MSG_ID_DEBUG_SBU;
			rsp_msg.head.msg_source_id		= MSG_ID_DEBUG_SBU;
			rsp_msg.head.msg_type			= SUB_ID_DEBUG_PH_KEY|COMMON_RESPONSE_BIT;
			rsp_msg.head.msg_sub_type		= 0;			
			rsp_msg.key_dat 				= pDebugKey->key_dat;
			rsp_msg.key_state				= pDebugKey->key_state;			
			vdp_uart_send_data((char*)&rsp_msg, sizeof(vdp_debug_key_type));
			
			// key process
			if( pDebugKey->key_dat == KEY1_PRESS )		
			{
				bprintf("get one ph key = %d\n",pDebugKey->key_dat); 
				//result = send_ip_mon_link_req(inet_addr("192.168.1.8"),0,&remote_ip,&dev_id);
				//result = API_VIDEO_C_SERVICE_TURN_ON_MULTICAST( inet_addr("192.168.1.8"), 150 );
				bprintf("send_ip_mon_link_req: result=%d,ip=0x%08x,devid=%d\n",result,remote_ip,dev_id);
			}			
			if( pDebugKey->key_dat == KEY2_PRESS )
			{
				bprintf("get one ph key = %d\n",pDebugKey->key_dat); 		
				//API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST();
			}			
			if( pDebugKey->key_dat == KEY3_PRESS )
			{
				bprintf("get one ph key = %d\n",pDebugKey->key_dat); 		
			}
			if( pDebugKey->key_dat == KEY4_PRESS )
			{
				bprintf("get one ph key = %d\n",pDebugKey->key_dat); 

				fw_link.device_type_req 	= 1;
				fw_link.fireware_ver_req	= 2;
				api_fireware_upgrade_apply(inet_addr("192.168.1.242"),&fw_link);
				
			}			
			break;
			
		case SUB_ID_DEBUG_LOG_DOC:
			//写入到logFile
			#ifdef WRITE_LOG_DOC_ENABLE 		
			write_rec_to_log_doc(pviewdat);
			#endif
			break;
	}
}

/*
time_t long型，表示从1970年1月1日到现在经过的秒数。
struct tm 
{
	int tm_sec;      // 秒 – 取值区间为[0,59] 
	int tm_min;      // 分 - 取值区间为[0,59] 
	int tm_hour;     // 时 - 取值区间为[0,23] 
	int tm_mday;     // 一个月中的日期 - 取值区间为[1,31] 
	int tm_mon;      // 月份（从一月开始，0代表一月） - 取值区间为[0,11] 
	int tm_year;     // 年份，其值等于实际年份减去1900 
	int tm_wday;     // 星期 – 取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推 
	int tm_yday;     // 从每年的1月1日开始的天数 – 取值区间为[0,365]，其中0代表1月1日，1代表1月2日
};
*/
// 加入的log信息格式如下:
// 1、时间日期(15): 	160412_11:20:30
// 2、业务类型(6):		S_CAL0
// 3、描叙(64):		非通信-Call start, LINKING.../通信-[192.168.13.1];UDP;T
// 4、等级(1):		0-9，数字越大越高
// 5、细节:			通信类型的数据

/****************************************************************************************************************************************
 * @fn      API_add_log_item()
 *
 * @brief   加入一个log信息
 *
 * @param   level 		- 保存到文档或view的等级，0-9，数字越大登记越高
 * @param   pbusiness 	- 业务类型，固定为6个字符长度，如: 格式为S_XXXX
 * @param   pdes 		- 业务描叙，最长为64个字节，描叙中空格用'_'来替代，空格作为excel的分隔符 (若为通信格式，则格式为:[IP];UDP/TCP;T/R)
 * @param   pdetail	- 业务细节，一般作为通信数据包显示
 *
 * @return  0 - ok, other - err
 ***************************************************************************************************************************************/
int API_add_log_item( int level, char* pbusiness, char* pdes, char* pdetail )
{
	VDP_MSG_HEAD 	msg_head;
	int				offset;
		
	char strout[MAX_PATH_LENGTH+1];
	char strtime[MAX_PATH_LENGTH];

	time_t t;
	struct tm *tblock; 	
	t = time(NULL); 
	tblock=localtime(&t);

	msg_head.msg_source_id	= MSG_ID_DEBUG_SBU;
	msg_head.msg_target_id	= MSG_ID_DEBUG_SBU;
	msg_head.msg_type		= SUB_ID_DEBUG_LOG_DOC;
	msg_head.msg_sub_type	= SUB_ID_DEBUG_LOG_DOC;

	offset = sizeof(VDP_MSG_HEAD);
	memcpy( strout, (char*)&msg_head, offset );
		
	sprintf( strtime,"%02d%02d%02d_%02d:%02d:%02d",tblock->tm_year-100,tblock->tm_mon+1,tblock->tm_mday,tblock->tm_hour,tblock->tm_min,tblock->tm_sec);
	strout[offset] = strlen(pdes); // 保存描述的长度
	snprintf( strout+offset+1, MAX_PATH_LENGTH-offset, "%s %s %s %d %s \n",strtime,pbusiness,pdes,level,(pdetail==NULL)?" ":pdetail);

	// 写入到logdoc
	if( level >= debug_log_doc_level )
	{	
		push_vdp_common_queue(&vdp_debug_sbu_inner_queue, strout, offset+1+strlen(strout+offset+1)+1 );
	}
	return 0;	
}



///////////////////////////////////////////////////////////////
//lzh_20160704_s
int get_upgrade_data_package_len( int cmd )
{
	int len = 0;
	switch( cmd )
	{
	case CMD_DEVICE_TYPE_CODE_VER_REQ:
		len = sizeof(fw_link_request);
		break;
	case CMD_DEVICE_TYPE_CODE_VER_RSP:
		len = sizeof(fw_link_response);
		break;
	case CMD_DEVICE_TYPE_DOWNLOAD_START_REQ:
		len = sizeof(fw_download_start_request);
		break;
	case CMD_DEVICE_TYPE_DOWNLOAD_START_RSP:
		len = sizeof(fw_download_start_response);
		break;
	case CMD_DEVICE_TYPE_DOWNLOAD_VERIFY_REQ:
		len = sizeof(fw_download_verify_request);
		break;
	case CMD_DEVICE_TYPE_DOWNLOAD_VERIFY_RSP:
		len = sizeof(fw_download_verify_response);
		break;
	case CMD_DEVICE_TYPE_UPDATE_START_REQ:
		len = sizeof(fw_download_update_request);
		break;
	case CMD_DEVICE_TYPE_UPDATE_START_RSP:
		len = sizeof(fw_download_update_response);
		break;
	}
	return len;
}

int api_fireware_upgrade_cmd_send( int target_ip, int cmd, char* pdat  )
{
	int len = get_upgrade_data_package_len(cmd);
	
	if( api_udp_c5_ipc_send_data( target_ip ,cmd, pdat, len) == -1 )
	{
		bprintf("api_fireware_upgrade_cmd_send fail\n");
		return -1;
	}
	else
	{
		bprintf("api_fireware_upgrade_cmd_send ok\n");
		return 0;
	}		
}


int api_fireware_upgrade_apply( int target_ip, fw_link_request* presult  )
{
	fw_link_response	fw_link_rsp;
	int len = sizeof(fw_link_request);
	if( api_udp_c5_ipc_send_req( target_ip,CMD_DEVICE_TYPE_CODE_VER_REQ,(char*)presult,sizeof(fw_link_request), &fw_link_rsp, &len) == -1 )
	{
		bprintf("api_fireware_upgrade_apply fail\n");
		return -1;
	}
	else
	{
		bprintf("api_fireware_upgrade_apply ok\n");
		return 0;
	}		
}
//lzh_20160704_e

