/**
  ******************************************************************************
  * @file    obj_DeviceLinking.c
  * @author  czb
  * @version V00.01.00
  * @date    2016.5.31
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2016 V-Tec</center></h2>
  ******************************************************************************
  */ 
#include "obj_DeviceLinking.h"

LINK_PROPERTY linkRun;

void DeviceLinkingInit(void)
{
	linkRun.CheckStopFlag = 1;
	linkRun.CheckState = CHECK_STATE_IDLE;
	linkRun.deviceAddr = NULL;
	
	OS_CreateTimer(&linkRun.checkNextTimer, CheckNextDeviceTimerCallBack, 100/25);
}

void CheckNextDeviceTimerCallBack(void)
{
	if(++linkRun.currentDevice < linkRun.deviceNum)
	{
		if(linkRun.CheckStopFlag)
		{
			linkRun.CheckState = CHECK_STATE_IDLE;
			free(linkRun.deviceAddr);
			linkRun.deviceAddr = NULL;
		}
		else
		{
			DeviceLinking_Standard_Processing(linkRun.deviceAddr[linkRun.currentDevice]);
		}
	}
	else
	{
		linkRun.CheckStopFlag = 1;
		linkRun.CheckState = CHECK_STATE_IDLE;
		free(linkRun.deviceAddr);
		linkRun.deviceAddr = NULL;
	}
}


void DeviceLinking_Standard_Processing(uint16 logicAddr)
{
	UART_MSG_TYPE msg;

	msg.head.msg_target_id= MSG_ID_survey;
	msg.head.msg_source_id = MSG_ID_survey;
	msg.head.msg_type = UATR_MSG_TYPE_LINKING;
	msg.head.msg_sub_type = 0;

	msg.len = 2;
	msg.pbuf[0] = logicAddr&0x00FF;
	msg.pbuf[1] = logicAddr>>8;

	vdp_uart_send_data((char*)&msg, 10);
	printf("check logicAddr = %d device start.......................................................................\n", logicAddr);
}

void LinkingMultipleDevices(uint16 deviceNum, uint16 *logicAddr)
{
	if(linkRun.CheckState == CHECK_STATE_BUSY)
	{
		return;
	}
	
	linkRun.currentDevice = 0;
	linkRun.CheckState = CHECK_STATE_BUSY;
	linkRun.CheckStopFlag = 0;
	
	linkRun.deviceNum = deviceNum;
	
	linkRun.deviceAddr = malloc(deviceNum*sizeof(uint16));
	memcpy(linkRun.deviceAddr, logicAddr, deviceNum*sizeof(uint16));
	
	DeviceLinking_Standard_Processing(linkRun.deviceAddr[linkRun.currentDevice]);
}

void StopLinking(void)
{
	linkRun.CheckStopFlag = 1;
}

uint8 GetCheckState(void)
{
	return linkRun.CheckState;
}

LINK_RESULT LinkingDeviceResponse(uint16 logicAddr, uint8 result)
{
	LINK_RESULT linkResult;
	
	if(logicAddr == linkRun.deviceAddr[linkRun.currentDevice])
	{
		linkResult.device= linkRun.deviceAddr[linkRun.currentDevice];
		linkResult.result = result;

		if(linkRun.CheckStopFlag)
		{
			linkRun.CheckState = CHECK_STATE_IDLE;
			free(linkRun.deviceAddr);
			linkRun.deviceAddr = NULL;
			OS_StopTimer(&linkRun.checkNextTimer);
		}
		else
		{
			OS_RetriggerTimer(&linkRun.checkNextTimer);
		}
	}
	else
	{
		linkResult.result = 0xFF;		//ÎÞÐ§Ó¦´ð
	}

	printf("check device rusult logicAddr = %d, result = %d------------------------------------------------\n", logicAddr, result);
	
	return linkResult;
}

//czn_20160629
int Device_SyncLinking(uint16 logicAddr)
{
	UART_MSG_TYPE msg;
	char rsp_msg[10];
	int rlen;
	unsigned short rsp_logicAddr;
	p_vdp_common_buffer pdb = 0;
	vdp_task_t *pthread;

	msg.head.msg_target_id	= MSG_ID_survey;
	msg.head.msg_source_id 	= GetMsgIDAccordingPid(pthread_self());
	msg.head.msg_type 		= UATR_MSG_TYPE_SYNCLINKING;
	msg.head.msg_sub_type 	= 0;

	msg.len = 2;
	msg.pbuf[0] = logicAddr&0x00FF;
	msg.pbuf[1] = logicAddr>>8;

	pthread = GetTaskAccordingMsgID(msg.head.msg_source_id);
	
	if(pthread == NULL)
	{
		return -1;
	}

	while(pop_vdp_common_queue( pthread->p_syc_buf,&pdb,1)>0)
	{
		purge_vdp_common_queue(pthread->p_syc_buf);
	}
	
	vdp_uart_send_data((char*)&msg, 10);
	printf("sync check logicAddr = %d device start.......................................................................\n", logicAddr);
	
	if(WaitForBusinessACK( pthread->p_syc_buf, msg.head.msg_type, (char*)&rsp_msg, &rlen, 3000 ) == 0)
	{
		return -1;
	}
	
	rsp_logicAddr = rsp_msg[5];
	rsp_logicAddr <<= 8;
	rsp_logicAddr += rsp_msg[4];
			
	
	if(logicAddr != rsp_logicAddr || rsp_msg[6] != 1)
	{
		return -1;
	}

	return 0;
}
/*********************************************************************************************************
**  End Of File
*********************************************************************************************************/

