/**
  ******************************************************************************
  * @file    task_IoServer.c
  * @author  zxj
  * @version V00.01.00
  * @date    2012.11.01
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2012 V-Tec</center></h2>
  ******************************************************************************
  */ 
  
#include "task_IoServer.h"

#include "../task_debug_sbu/task_debug_sbu.h"
#include "obj_ResourceTable.h"		//czn_20160812

#include "task_IoServer.h"
#include "vdp_IoServer_Data.h"
#include "vdp_IoServer_State.h"


Loop_vdp_common_buffer	vdp_io_server_mesg_queue;
Loop_vdp_common_buffer	vdp_io_server_sync_queue;
vdp_task_t				task_io_server;

void vdp_io_server_mesg_data_process(char* msg_data, int len);
void* vdp_io_server_task( void* arg );
	
void vtk_TaskInit_io_server(void)
{
	init_vdp_common_queue(&vdp_io_server_mesg_queue, 500, (msg_process)vdp_io_server_mesg_data_process, &task_io_server);
	init_vdp_common_queue(&vdp_io_server_sync_queue, 500, NULL, 								  		&task_io_server);
	init_vdp_common_task(&task_io_server, MSG_ID_IOServer, vdp_io_server_task, &vdp_io_server_mesg_queue, &vdp_io_server_sync_queue);

	IoServerStateInit();
}


void exit_vdp_io_server_task(void)
{
	exit_vdp_common_queue(&vdp_io_server_mesg_queue);
	exit_vdp_common_queue(&vdp_io_server_sync_queue);
	exit_vdp_common_task(&task_io_server);
}

void* vdp_io_server_task( void* arg )
{
	vdp_task_t*	 ptask 			= (vdp_task_t*)arg;
	p_vdp_common_buffer pdb 	= 0;
	int	size;
	
	load_io_data_table();

	API_Load_ResourceTable();	//czn_20160812
	
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

void vdp_io_server_mesg_data_process(char* msg_data,int len)
{
	io_server_type*   pIoMsg;
	pIoMsg = (io_server_type*)msg_data;

	switch( pIoMsg->head.msg_type )
	{
		case IO_SERVER_READ_LOCAL:
			InnerRead(&pIoMsg->msg_dat);
			//printf("---pIoMsg->msg_dat---,len=%d,d0=%d,d1=%d\n",pIoMsg->msg_dat.len,pIoMsg->msg_dat.ptr_data[0],pIoMsg->msg_dat.ptr_data[1]);
			// 回复应答数据
			ReadWritelocalResponse( pIoMsg->head.msg_source_id, IO_SERVER_READ_LOCAL, &pIoMsg->msg_dat);
			break;
			
		case IO_SERVER_WRITE_LOCAL:
			InnerWrite(&pIoMsg->msg_dat);
			// 回复应答数据
			ReadWritelocalResponse( pIoMsg->head.msg_source_id, IO_SERVER_WRITE_LOCAL, &pIoMsg->msg_dat );
			break;
			
		case IO_SERVER_SAVE_DATA_TO_FILE:
			SaveIoDataTable();
			sync();		//czn_20161012			
			break;
			
		default:
			break;
	}
}

// msg_id - 申请读的id
int API_io_server_read_local(unsigned char sourceTaskId, uint16 property_id, uint8 *ptr_data )
{
	io_server_type  data;
	int len;
	
	// 组织io数据
	data.head.msg_target_id	= MSG_ID_IOServer;
	data.head.msg_source_id	= sourceTaskId;
	data.head.msg_type 		= IO_SERVER_READ_LOCAL;
	data.head.msg_sub_type 	= 0;
	data.msg_dat.property_id	= property_id;
	data.msg_dat.result		= -1;

	vdp_task_t* pTask = GetTaskAccordingMsgID(sourceTaskId);


	if( pTask != NULL )
	{
		push_vdp_common_queue(&vdp_io_server_mesg_queue, (char*)&data, sizeof(data) );
		// 等待ack应答
		if(WaitForBusinessACK( pTask->p_syc_buf, IO_SERVER_READ_LOCAL, (char*)&data, &len, 5000 ) > 0)
		{
			if(data.msg_dat.result)
			{
				return -2;
			}
			else
			{
				//printf("---ptr_data---,d0=%d,d1=%d\n",data.msg_dat.ptr_data[0],data.msg_dat.ptr_data[1]);
			
				memcpy( ptr_data, data.msg_dat.ptr_data, len-sizeof(VDP_MSG_HEAD)-8);	

				//printf("---ptr_data---,len=%d,d0=%d,d1=%d\n",len,ptr_data[0],ptr_data[1]);

				return 0;
			}
		}
		return -3;
	}
	else
	{
		return -1;
	}
}

int API_io_server_write_local(unsigned char sourceTaskId, uint16 property_id, uint8 *ptr_data )
{
	io_server_type  data;
	int len;
	

	// 组织io数据
	data.head.msg_target_id	= MSG_ID_IOServer;
	data.head.msg_source_id	= sourceTaskId;
	data.head.msg_type 		= IO_SERVER_WRITE_LOCAL;
	data.head.msg_sub_type 	= 0;
	data.msg_dat.property_id	= property_id;
	data.msg_dat.result		= -1;

	if(EEPROM_ADDR[property_id].dpt > 4)
	{
		len = ptr_data[0]+1;
	}
	else
	{
		len = EEPROM_ADDR[property_id].dpt;
	}
	memcpy(data.msg_dat.ptr_data, ptr_data, len);
	
	vdp_task_t* pTask = GetTaskAccordingMsgID(sourceTaskId);


	if( pTask != NULL )
	{
		push_vdp_common_queue(&vdp_io_server_mesg_queue, (char*)&data, sizeof(data) );
		// 等待ack应答
		if(WaitForBusinessACK( pTask->p_syc_buf, IO_SERVER_WRITE_LOCAL, (char*)&data, &len, 5000 ) > 0)
		{
			if(data.msg_dat.result)
			{
				return -2;
			}
			else
			{
				//memcpy( ptr_data, data.msg_dat.ptr_data, len-sizeof(VDP_MSG_HEAD)-8); 
				return 0;
			}
		}
		return -3;
	}
	else
	{
		return -1;
	}
}


int API_io_server_save_data_file(void)
{
	io_server_type  data;
	
	data.head.msg_target_id 	= MSG_ID_IOServer;
	data.head.msg_source_id	= MSG_ID_IOServer;			
	data.head.msg_type		= IO_SERVER_SAVE_DATA_TO_FILE;
	push_vdp_common_queue(&vdp_io_server_mesg_queue, (char*)&data, sizeof(data) );
	return 0;
}

/*********************************************************************************************************
**  End Of File
*********************************************************************************************************/
