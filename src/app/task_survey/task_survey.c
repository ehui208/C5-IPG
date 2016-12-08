

#include "task_survey.h"
#include "obj_multi_timer.h"
#include "sys_msg_process.h"
#include "./obj_CallServer_Virtual/obj_CallServer_Virtual.h"	//czn_20160526
#include "obj_DeviceLinking.h"		//cao_20160601
//czn_20160705_s
#include "../vtk_udp_stack/vtk_udp_stack_c5_ipc_cmd.h"
#include "../task_debug_sbu/task_debug_sbu.h"
//czn_20160705_e

//czn_20160418_s
unsigned char Call_Survey_State = CALL_SURVEY_IDLE;
//czn_20160418_e

Loop_vdp_common_buffer	vdp_controller_mesg_queue;
Loop_vdp_common_buffer	vdp_controller_sync_queue;
vdp_task_t				task_control;

void survey_mesg_data_process(char* msg_data, int len);
void* survey_mesg_task( void* arg );

void vtk_TaskInit_survey( void )
{
	init_vdp_common_queue(&vdp_controller_mesg_queue, 1000, (msg_process)survey_mesg_data_process, &task_control);
	init_vdp_common_queue(&vdp_controller_sync_queue, 100, NULL, 								  &task_control);
	init_vdp_common_task(&task_control, MSG_ID_survey, survey_mesg_task, &vdp_controller_mesg_queue, &vdp_controller_sync_queue);

	dprintf("survey task starting............\n");
}

void vtk_TaskExit_survey(void)
{
	exit_vdp_common_queue(&vdp_controller_mesg_queue);
	exit_vdp_common_queue(&vdp_controller_sync_queue);
	exit_vdp_common_task(&task_control);
}

void* survey_mesg_task( void* arg )
{
	vdp_task_t*	 ptask 			= (vdp_task_t*)arg;
	p_vdp_common_buffer pdb 	= 0;
	int	size;
	//czn_20160705_s
	int ip_addr;
	fw_download_update_response	fw_download_update_rsp;
	if(IsHaveUpdate(&ip_addr) == 0)
	{
		fw_download_update_rsp.result = 0;
		bprintf("HaveUpdate serverip=%08x %s %s\n",ip_addr,__DATE__,__TIME__);
		api_fireware_upgrade_cmd_send(ip_addr,CMD_DEVICE_TYPE_UPDATE_START_RSP, (char*)&fw_download_update_rsp );
	}
	//czn_20160705_e
	while( ptask->task_run_flag )
	{
		size = pop_vdp_common_queue(ptask->p_msg_buf, &pdb, VDP_QUEUE_POLLING_TIME);
		if( size > 0 )
		{
			(*ptask->p_msg_buf->process)(pdb,size);
			purge_vdp_common_queue( ptask->p_msg_buf );
		}
	}
	return 0;
}

void survey_mesg_data_process(char* msg_data,int len)
{
	VDP_MSG_HEAD* pVdpUdp = (VDP_MSG_HEAD*)msg_data;

	// 判断是否为系统服务
	if( pVdpUdp->msg_target_id == MSG_ID_survey )
	{
		survey_sys_message_processing(msg_data,len);
	}
	else
	{
		vdp_task_t* pVdpTask = GetTaskAccordingMsgID(pVdpUdp->msg_target_id);
		if( pVdpTask != NULL )
		{
			push_vdp_common_queue(pVdpTask->p_msg_buf,msg_data,len);
			dprintf("msg_t_id=%d ok,msg_s_id=%d,msg_len=%d\n",pVdpUdp->msg_target_id,pVdpUdp->msg_source_id,len);			
		}
		else
		{
			dprintf("msg_t_id=%d er,msg_s_id=%d,msg_len=%d\n",pVdpUdp->msg_target_id,pVdpUdp->msg_source_id,len);			
		}
	}
}

// 功能: 等待业务同步应答
// 参数: pBusiness - 同步等待时的挂起的队列，business_type - 等待的业务类型， data - 得到的数据，plen - 数据有效长度
// 返回: 0 - 返回超时，1 -  返回正常
int WaitForBusinessACK( p_Loop_vdp_common_buffer pBusiness, unsigned char business_type,  char* data, int* plen, int timeout )
{
	VDP_MSG_HEAD* pbusiness_buf;
	p_vdp_common_buffer pdb = 0;
	int size;
	vdp_task_t*	powner;	
	
	int ret = 0;

	size = pop_vdp_common_queue( pBusiness,&pdb,timeout);
	if( size > 0  )
	{		
		pbusiness_buf = (VDP_MSG_HEAD*)pdb;

		powner = (vdp_task_t*)pBusiness->powner;
			
		if( pbusiness_buf->msg_type  == (business_type|COMMON_RESPONSE_BIT) )
		{				
			//dprintf("task %d wait ack ok!\n",powner->task_id);
			// 得到数据
			*plen = size;
			memcpy( data, pdb, size );
			ret = 1;
		}
		purge_vdp_common_queue( pBusiness );
	}
	return ret;
}

vdp_task_t* GetTaskAccordingMsgID(unsigned char msg_id)		//cao_20160429
{
	vdp_task_t* ptaskobj = NULL;
	
	switch( msg_id )
	{
		case MSG_ID_DEBUG_SBU:
			ptaskobj = &task_debug_sbu;
			break;
		case MSG_ID_IOServer:
			ptaskobj = &task_io_server;				
			break;
		case MSG_ID_survey:
			ptaskobj = &task_control; 			
			break;
		case MSG_ID_NetManage:
			ptaskobj = &task_net_manange; 			
			break;
		//czn_20160526_s
		case MSG_ID_IpCalller:
			ptaskobj = &task_caller;			
			break;	
		case MSG_ID_IpBeCalled:
			ptaskobj = &task_becalled;			
			break;
		//czn_20160526_e				
	}	
	return ptaskobj;
}

const vdp_task_t* 	task_tab[] =
{
	&task_debug_sbu,
	&task_control,
	&task_io_server,
	&task_net_manange,
	//czn_20160526_s
	&task_caller,
	&task_becalled,
	//czn_20160526_e				
	NULL,
};

int GetMsgIDAccordingPid( pthread_t pid )
{
	int i;
	for( i = 0; ; i++ )
	{
		if( task_tab[i]->task_pid == NULL )
			return 0;
		if( task_tab[i]->task_pid == pid )
		{
			dprintf("get pid=%lu msg id=%d\n",task_tab[i]->task_pid,task_tab[i]->task_id);
			return task_tab[i]->task_id;
		}
	}
	return 0;
}

/****************************************************************************************************************************
 * @fn:		API_add_message_to_suvey_queue
 *
 * @brief:	加入消息到分发服务队列
 *
 * @param:  pdata 			- 数据指针
 * @param:  len 			- 数据长度
 *
 * @return: 0/ok, 1/full
****************************************************************************************************************************/
int	API_add_message_to_suvey_queue( char* pdata, unsigned int len )
{
	return push_vdp_common_queue(task_control.p_msg_buf, pdata, len);
}

int API_one_message_to_suvey_queue( int msg, int sub_msg )
{
	VDP_MSG_HEAD	head;
	head.msg_source_id	= MSG_ID_survey;
	head.msg_target_id	= MSG_ID_survey;
	head.msg_type		= msg;
	head.msg_sub_type	= sub_msg;	
	return push_vdp_common_queue(task_control.p_msg_buf, &head, sizeof(VDP_MSG_HEAD) );
}

