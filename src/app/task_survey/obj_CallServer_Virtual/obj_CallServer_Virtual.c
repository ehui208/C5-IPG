/**
  ******************************************************************************
  * @file    task_BusinessAnalyze.c
  * @author  czn
  * @version V00.01.00 
  * @date    2015.01.13
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2014 V-Tec</center></h2>
  ******************************************************************************
  */ 
/*
#include "../task_Phone.h"
#include "../task_BeCalled/obj_BeCalled_Data.h"
#include "../task_BeCalled/obj_BeCalled_State.h"
#include "../task_BeCalled/task_BeCalled.h"
#include "task_BusinessAnalyze_Virtaul.h"

#include "../task_Caller/task_Caller.h"
*/
#include <stdio.h>

#include "obj_CallServer_Virtual.h"

#include "../../vtk_udp_stack/vtk_udp_stack_c5_ipc_cmd.h"


int API_CallServer_Common(unsigned char msg_type,unsigned char dtcall_type,unsigned char ipcall_type,
							Global_Addr_Stru *s_addr,Global_Addr_Stru *t_addr,unsigned char *ext_buf,int ext_len)
{
	CALLSERVER_STRU msg,rsp_msg;
	int rlen;
	
	//uint8 vsiptype_s,vsiptype_d;
	msg.msg_head.msg_target_id	= MSG_ID_CallServer;
	msg.msg_head.msg_source_id	= GetMsgIDAccordingPid(pthread_self());;
	msg.msg_head.msg_type		= msg_type;
	msg.msg_head.msg_sub_type 	= 0;
	msg.dtcall_type 				= dtcall_type;
	msg.ipcall_type				= ipcall_type;
	
	if(s_addr != NULL)
	{
		msg.s_addr = *s_addr;
	}
	if(t_addr != NULL)
	{
		msg.t_addr = *t_addr;
	}

	msg.ext_len = 0;
	
	if(ext_len > 0 && ext_buf != NULL)
	{
		msg.ext_len = (ext_len > MAX_CALLSERVER_MSGBUF_LEN) ? MAX_CALLSERVER_MSGBUF_LEN:ext_len;
		memcpy(msg.ext_buf,ext_buf,msg.ext_len);
	}
	
	vdp_uart_send_data((char*)&msg,CALLSERVER_STRU_HEAD + msg.ext_len);

	if(msg_type == CallServer_Msg_Unlink)
	{
		if(WaitForBusinessACK( GetTaskAccordingMsgID(msg.msg_head.msg_source_id)->p_syc_buf, msg_type, (char*)&rsp_msg, &rlen, 2000 ) == 0)
		{
			return -1;
		}
		if(rsp_msg.ext_buf[0] != 0)
		{
			return -1;
		}
	}
	return 0;
}




