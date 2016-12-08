/**
  ******************************************************************************
  * @file    obj_Caller_IPGVtkCall_Callback.c
  * @author  czn
  * @version V00.01.00 
  * @date    2016.01.21
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2016 V-Tec</center></h2>
  ******************************************************************************
  */
  /*
#include "../../task_Phone.h"
//#include "obj_IPGVtkCall_Analyze.h"
#include "../task_Caller.h"
//#include "task_BeCalled.h"
//#include "obj_BeCalled_IPGCallIPG_Callback.h"
#include "obj_Caller_IPGVtkCall_Callback.h"
*/

#include "../../../task_survey/obj_CallServer_Virtual/obj_CallServer_Virtual.h"
#include "../task_IpCaller.h"
#include "../obj_IpCaller_State.h"

#include "obj_IpCaller_VtkUnicastCall_Callback.h"

/*------------------------------------------------------------------------
						重拨处理
------------------------------------------------------------------------*/
void Callback_Caller_ToRedial_VtkUnicastCall(CALLER_STRUCT *msg)
{
	;	//暂无处理
}

/*------------------------------------------------------------------------
						呼叫申请
------------------------------------------------------------------------*/
void Callback_Caller_ToInvite_VtkUnicastCall(CALLER_STRUCT *msg)	//R_
{
	//will_add_czn
	/*uint8 SR_apply;
	SR_apply = API_SR_Request(CT08_DS_CALL_ST); 
						//链路忙,不可剥夺
	if (SR_apply == SR_DISABLE)
	{
		//will_add	//对小区主机_增加系统忙的应答
		return;
	}
	//链路忙,可剥夺
	else if (SR_apply == SR_ENABLE_DEPRIVED)	
	{
		API_SR_Deprived();	//复位系统链路状态字
	}
	//链路可直接使用
	else	 
	{	
		API_SR_Deprived();	//复位系统链路状态字	
	}*/
	//API_Stack_APT_Without_ACK(Caller_Run.target_addr&0x00ff,MAIN_CALL);
	//printf("___________________Caller_Run.tdev.ip%08x\n",Caller_Run.tdev.ipdev.ip);
	
	_IP_Call_Link_Run_Stru partner_call_link;
	Global_Addr_Stru gaddr;
	
	IpCaller_Business_Rps(msg,0);
	printf("--------------czn test----------------\n");
	//return;
	
	//if(API_Send_CallDial_ByUdp(Caller_Run.call_type,Caller_Run.call_sub_type,Caller_Run.para_type,&Caller_Run.tdev) == 0)
	if(API_Get_Partner_Calllink_NewCmd(DsAndGl,&(msg->t_addr),&partner_call_link) == -1)
	{
		printf("-----------fail -------------API_Get_Partner_Calllink_NewCmd state %d\n",partner_call_link.Status);
		API_CallServer_InviteFail();
		return;
	}
	printf("----------ok----------------API_Get_Partner_Calllink_NewCmd state %d\n",partner_call_link.Status);
	if(partner_call_link.Status & Device_Offline_BitMask)
	{
		API_CallServer_InviteFail();
		return;
	}
	if(partner_call_link.Status != CLink_Idle)
	{
		if(partner_call_link.Status == CLink_Transferred)
		{
			//czn_will_add
		}
		else
		{
			if(Send_UnlinkCmd(DsAndGl,&(msg->t_addr),&partner_call_link) == -1)
			{
				API_CallServer_InviteFail();
				return;
			}
		}
	}

	if(Send_VtkUnicastCmd_Invite(DsAndGl,&(msg->s_addr),&(msg->t_addr)) == 0)
	{
		Set_VtkUnicastCmdAnalyze_AsCallSource(); 
		
		gaddr.gatewayid 	= 0;
		gaddr.ip			= GetLocalIp();
		gaddr.rt			= msg->s_addr.rt;
		gaddr.code		= msg->s_addr.code;
		
		Set_IPCallLink_CallerData(&msg->t_addr);
		Set_IPCallLink_BeCalledData(&gaddr);
		
		Set_IPCallLink_Status(CLink_AsCallServer);
		Caller_Run.checklink_error	= 0;
	}
	else
	{
		API_CallServer_InviteFail();
	}
	
}

/*------------------------------------------------------------------------
						呼叫成功,进入Ringing
------------------------------------------------------------------------*/
void Callback_Caller_ToRinging_VtkUnicastCall(CALLER_STRUCT *msg)	//R_
{
	IpCaller_Business_Rps(msg,0);
	//will_add_czn
	/*if(SR_State.in_use == FALSE)
	{
		//链路创建
		//API_SR_CallerCreate();
		SR_Routing_Create(Business_Run.call_sub_type);
		
	}*/
}

/*------------------------------------------------------------------------
						分机摘机,进入ACK
------------------------------------------------------------------------*/
void Callback_Caller_ToAck_VtkUnicastCall(CALLER_STRUCT *msg)	//R_
{

	IpCaller_Business_Rps(msg,0);

	printf("Callback_Caller_ToAck_VtkCdsCallSt==============\n");
	Send_VtkUnicastCmd_StateNoticeAck(DsAndGl,&Caller_Run.t_addr,1);

	API_talk_on_by_unicast(Caller_Run.t_addr.ip,AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);
}

/*------------------------------------------------------------------------
						主机挂机,返回Waiting
------------------------------------------------------------------------*/

// lzh_20160127_s
#include "../../../video_service/video_object.h"
#include "../../../video_service/ip_camera_control/ip_camera_control.h"
#include "../../../video_service/ip_video_control/ip_video_control.h"
// lzh_20160127_e

void Callback_Caller_ToBye_VtkUnicastCall(CALLER_STRUCT *msg)	//R_
{

	IpCaller_Business_Rps(msg,0);
	
	Send_VtkUnicastCmd_StateNotice(DsAndGl,&Caller_Run.s_addr,&Caller_Run.t_addr,VTKU_CALL_STATE_BYE);

	//Caller_Run.state = CALLER_WAITING;
	
	//关闭定时
	//Caller_Run.timer = 0;
	//will_add_czn OS_StopTimer(&timer_caller);	
	
	// lzh_20160127_s	
	//api_camera_server_turn_off();

	API_talk_off();

	Set_IPCallLink_Status(CLink_Idle);
	// lzh_20160127_e

}

/*------------------------------------------------------------------------
				分机挂机(单元链路强制释放),返回Waitting
------------------------------------------------------------------------*/
void Callback_Caller_ToWaiting_VtkUnicastCall(CALLER_STRUCT *msg)	//R_
{
	IpCaller_Business_Rps(msg,0);
	
	Send_VtkUnicastCmd_StateNoticeAck(DsAndGl,&Caller_Run.t_addr,1);
	Set_IPCallLink_Status(CLink_Idle);

	API_talk_off();
}


/*------------------------------------------------------------------------
						开锁处理
------------------------------------------------------------------------*/
void Callback_Caller_ToUnlock_VtkUnicastCall(CALLER_STRUCT *msg)	//R_
{

}

/*------------------------------------------------------------------------
					Caller呼叫超时处理(仅自身退出呼叫状态)						
------------------------------------------------------------------------*/
void Callback_Caller_ToTimeout_VtkUnicastCall(CALLER_STRUCT *msg)	//R_
{
	_IP_Call_Link_Run_Stru partner_call_link;
	int  quit_flag = 0;
	
	if(msg->msg_head.msg_sub_type == CALLER_TOUT_TIMEOVER)
	{
		Send_VtkUnicastCmd_StateNotice(DsAndGl,&Caller_Run.s_addr,&Caller_Run.t_addr,VTKU_CALL_STATE_BYE);
		
		Caller_Run.timer = 0;
		OS_StopTimer(&timer_caller);
		Caller_Run.state = CALLER_WAITING;
		API_talk_off();

		Set_IPCallLink_Status(CLink_Idle);
		
		API_CallServer_IpCallerQuit();
	}
	else if(msg->msg_head.msg_sub_type == CALLER_TOUT_CHECKLINK)
	{		
		if((Caller_Run.state == CALLER_RINGING  || Caller_Run.state == CALLER_ACK) && Caller_Run.timer > 3)
		{
			Global_Addr_Stru gaddr;
			gaddr.gatewayid	= 0;
			gaddr.ip			= Caller_Run.t_addr.ip;
			gaddr.rt			= 0;
			gaddr.code		= 0;
			if(API_Get_Partner_Calllink_NewCmd(DsAndGl,&gaddr,&partner_call_link) == -1)
			{
				if(++Caller_Run.checklink_error	 >= 2)
				{
					quit_flag = 1;
					goto CTOUT_CHECKLINK_END;
				}
			}
			else
			{
				Caller_Run.checklink_error = 0;
				
				if(partner_call_link.Status != CLink_AsCallServer && partner_call_link.Status != CLink_AsBeCalled)
				{
					quit_flag = 1;
					goto CTOUT_CHECKLINK_END;
				}
				
				if(partner_call_link.BeCalled_Data.Call_Source.ip != GetLocalIp() || 
					partner_call_link.BeCalled_Data.Call_Source.rt != Caller_Run.s_addr.rt||
					partner_call_link.BeCalled_Data.Call_Source.code != Caller_Run.s_addr.code)
				{
					quit_flag = 1;
				}
			}
	CTOUT_CHECKLINK_END:
			if(quit_flag == 1)
			{
				Caller_Run.timer = 0;
				OS_StopTimer(&timer_caller);
				Caller_Run.state = CALLER_WAITING;
				API_talk_off();

				Set_IPCallLink_Status(CLink_Idle);
				
				API_CallServer_IpCallerQuit();
			}
			
		}
		
	}
	
}

/*------------------------------------------------------------------------
				接收到消息(): 错误处理					
------------------------------------------------------------------------*/
void Callback_Caller_ToError_VtkUnicastCall(CALLER_STRUCT *msg)	
{
	IpCaller_Business_Rps(msg,0);
	
	switch(Caller_ErrorCode)
	{
		case CALLER_ERROR_DTBECALLED_QUIT:
		case CALLER_ERROR_UINTLINK_CLEAR:
		case CALLER_ERROR_INVITEFAIL:
			
			
			//API_Send_BusinessRsp_ByUdp(PHONE_TYPE_CALLER,Caller_Run.call_type,Caller_Run.call_sub_type);
			if(Caller_Run.state != CALLER_WAITING)
			{
				Send_VtkUnicastCmd_StateNotice(DsAndGl,&Caller_Run.s_addr,&Caller_Run.t_addr,VTKU_CALL_STATE_BYE);
				Caller_Run.state = CALLER_WAITING;
		
				//关闭定时
				Caller_Run.timer = 0;
				OS_StopTimer(&timer_caller);
				Set_IPCallLink_Status(CLink_Idle);
				API_talk_off();
			}
			//will_add_czn OS_StopTimer(&timer_caller);
			break;
		
	}
}

void Callback_Caller_ToForceClose_VtkUnicastCall(CALLER_STRUCT *msg)
{
	
	
	//if(Caller_Run.state != CALLER_WAITING)
	{
		
		Caller_Run.state = CALLER_WAITING;

		//关闭定时
		Caller_Run.timer = 0;

		Set_IPCallLink_Status(CLink_Idle);
		API_talk_off();
	}
}