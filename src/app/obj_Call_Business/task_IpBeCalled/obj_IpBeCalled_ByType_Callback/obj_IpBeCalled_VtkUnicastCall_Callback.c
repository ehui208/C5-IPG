/**
  ******************************************************************************
  * @file    obj_BeCalled_CdsCallSt_Callback.c
  * @author  czn
  * @version V00.01.00 
  * @date    2015.01.06
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2014 V-Tec</center></h2>
  ******************************************************************************
  */ 
  /*
#include "../../task_Phone.h"
#include "../task_BeCalled.h"
#include "obj_BeCalled_IPGVtkCall_Callback.h"
//#include "obj_Caller_IPGCallIPG_Callback.h"
*/
#include "../../../task_survey/obj_CallServer_Virtual/obj_CallServer_Virtual.h"
#include "../task_IpBeCalled.h"
#include "../obj_IpBeCalled_State.h"
#include "obj_IpBeCalled_VtkUnicastCall_Callback.h"
#include "../../../video_service/ip_video_cs_control.h"
// lzh_20160127_s
#include "../../../video_service/video_object.h"
#include "../../../video_service/ip_camera_control/ip_camera_control.h"
#include "../../../video_service/ip_video_control/ip_video_control.h"
// lzh_20160127_e
//r_20150801
/*------------------------------------------------------------------------
						重拨处理
------------------------------------------------------------------------*/
void Callback_BeCalled_ToRedial_VtkUnicastCall(BECALLED_STRUCT *msg)
{
	;	//will_add
}

/*------------------------------------------------------------------------
						呼叫转译处理
------------------------------------------------------------------------*/
void Callback_BeCalled_ToTransfer_VtkUnicastCall(BECALLED_STRUCT *msg)
{
	;
}

/*------------------------------------------------------------------------
							进入Ringing
1. 对小区主机: 发送TRYING
2. 对Analyze:  向Caller申请呼叫
------------------------------------------------------------------------*/



void Callback_BeCalled_ToRinging_VtkUnicastCall(BECALLED_STRUCT *msg)	//R_
{
	printf("----------Callback_BeCalled_ToRinging_VtkUnicastCall-----\n");
	IpBeCalled_Business_Rps(msg,0);

	Set_VtkUnicastCmdAnalyze_AsCallTarget(); 

	Set_IPCallLink_Status(CLink_AsCallServer);
	
	Set_IPCallLink_CallerData(&msg->t_addr);
	Set_IPCallLink_BeCalledData(&msg->s_addr);
	
	Send_VtkUnicastCmd_InviteAck(DsAndGl,&msg->s_addr,1);
	
	
	//if(BeCalled_Run.s_addr.ip>>24 != 21 && BeCalled_Run.s_addr.ip>>24 != 22)
	//will_change if(BeCalled_Run.s_addr.rt == 10 || BeCalled_Run.s_addr.rt == 12)
	//API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(BeCalled_Run.s_addr.ip,150);

	BeCalled_Run.checklink_error = 0;
	// lzh_20160127_s

	// lzh_20160127_e	
}

/*------------------------------------------------------------------------
							进入ACK
1. 对小区主机: 发送OK_200(摘机)
------------------------------------------------------------------------*/
void Callback_BeCalled_ToAck_VtkUnicastCall(BECALLED_STRUCT *msg)		//R_
{
	//向小区主机发送OK_200指令(告知小区主机_呼叫成功)
	
	//IPG_Translation_ToDs(Business_Run.call_sub_type,Business_Run.para_type,
	//							 &Business_Run.sdev,&Business_Run.tdev,ST_TALK);
	//API_Send_CallState_ByUdp(PHONE_TYPE_BECALLED,BeCalled_Run.call_type,BeCalled_Run.call_sub_type,BeCalled_Run.para_type,&BeCalled_Run.sdev,CALL_STATE_TALK);
	IpBeCalled_Business_Rps(msg,0);
	
	Send_VtkUnicastCmd_StateNotice(DsAndGl,&BeCalled_Run.s_addr,&BeCalled_Run.t_addr,VTKU_CALL_STATE_TALK);

	API_talk_on_by_unicast(BeCalled_Run.s_addr.ip,AUDIO_SERVER_UNICAST_PORT,AUDIO_CLIENT_UNICAST_PORT );
	//音视频开启确认
//	if (GetPowerVideoState() != POWER_ON)
//	{
//		OS_Delay(100);		
//		Work_On();
//	}
}

/*------------------------------------------------------------------------
				接收到消息(分机): 结束呼叫
1. 对小区主机: 发送呼叫结束指令
------------------------------------------------------------------------*/
void Callback_BeCalled_ToBye_VtkUnicastCall(BECALLED_STRUCT *msg)		//R_
{
	//向小区主机发送BYE_BECALLED指令(结束呼叫)
	
	//IPG_Translation_ToDs(Business_Run.call_sub_type,Business_Run.para_type,
	//							 &Business_Run.sdev,&Business_Run.tdev,ST_CLOSE);
	//API_Send_CallState_ByUdp(PHONE_TYPE_BECALLED,BeCalled_Run.call_type,BeCalled_Run.call_sub_type,BeCalled_Run.para_type,&BeCalled_Run.sdev,CALL_STATE_BYE);
	IpBeCalled_Business_Rps(msg,0);
	Send_VtkUnicastCmd_StateNotice(DsAndGl,&BeCalled_Run.s_addr,&BeCalled_Run.t_addr,VTKU_CALL_STATE_BYE);
	//BeCalled_Run.state = BECALLED_WAITING;
	
	//BeCalled_Run.timer = 0;

	Set_IPCallLink_Status(CLink_Idle);
	
	api_video_c_service_turn_off(); 

	API_talk_off();
	//will_add_czn OS_StopTimer(&timer_becalled);
//	
//	if(GetPowerVideoTxState() == POWER_ON)
//	{
//		OS_Delay(100);
//		API_POWER_VIDEOTX_OFF();
//	}
	
	// lzh_20160127_s
	//api_video_client_turn_off();
	// lzh_20160127_e	

}

/*------------------------------------------------------------------------
				接收到消息(): 结束呼叫					
------------------------------------------------------------------------*/
void Callback_BeCalled_ToWaiting_VtkUnicastCall(BECALLED_STRUCT *msg)	//R_
{
	
//	if(GetPowerVideoTxState() == POWER_ON)
//	{
//		OS_Delay(100);
//		API_POWER_VIDEOTX_OFF();
//	}
	IpBeCalled_Business_Rps(msg,0);
	
	Send_VtkUnicastCmd_StateNoticeAck(DsAndGl,&BeCalled_Run.s_addr,1);

	Set_IPCallLink_Status(CLink_Idle);

	api_video_c_service_turn_off(); 

	API_talk_off();
	
}

/*------------------------------------------------------------------------
						开锁处理
1. 对小区主机: 发送开锁指令
------------------------------------------------------------------------*/
void Callback_BeCalled_ToUnlock_VtkUnicastCall(BECALLED_STRUCT *msg)	//R-
{
	IpBeCalled_Business_Rps(msg,0);
 	if(msg->msg_head.msg_type == BECALLED_MSG_UNLOCK1)
	{
		//向小区主机发送开锁1指令
		//Gateway_SendDS_Command(CdsCallIPG_Run.cds_addr, CdsCallIPG_Run.st_addr, ST_UNLOCK);
		//API_Send_Unlock_ByUdp(BeCalled_Run.call_type,BeCalled_Run.call_sub_type,BeCalled_Run.para_type,&BeCalled_Run.sdev,0);
		//printf("Callback_BeCalled_ToUnlock1_IPGVtkCall---------------------\n");
		Send_VtkUnicastCmd_TargetUnlockReq(DsAndGl,&BeCalled_Run.s_addr,0);
	}
	else if(msg->msg_head.msg_type == BECALLED_MSG_UNLOCK2)
	{
		//向小区主机发送开锁2指令
		//Gateway_SendDS_Command(CdsCallIPG_Run.cds_addr, CdsCallIPG_Run.st_addr, ST_UNLOCK_SECOND);
		//API_Stack_APT_Without_ACK(CdsCallIPG_Run.cds_addr,ST_UNLOCK_SECOND);
		//API_Send_Unlock_ByUdp(BeCalled_Run.call_type,BeCalled_Run.call_sub_type,BeCalled_Run.para_type,&BeCalled_Run.sdev,1); 
		Send_VtkUnicastCmd_TargetUnlockReq(DsAndGl,&BeCalled_Run.s_addr,1);
	}
}

/*------------------------------------------------------------------------
					BeCalled呼叫超时(自动退出BeCalled)
------------------------------------------------------------------------*/
void Callback_BeCalled_ToTimeout_VtkUnicastCall(BECALLED_STRUCT *msg)	//R_
{
	_IP_Call_Link_Run_Stru partner_call_link;
	int quit_flag =0;
	
	if(msg->msg_head.msg_sub_type == BECALLED_TOUT_TIMEOVER)
	{	
		if(BeCalled_Run.state != BECALLED_WAITING)
		{
		
			Send_VtkUnicastCmd_StateNotice(DsAndGl,&BeCalled_Run.s_addr,&BeCalled_Run.t_addr,VTKU_CALL_STATE_BYE);

			BeCalled_Run.state = BECALLED_WAITING;
		
			BeCalled_Run.timer = 0;
			
			OS_StopTimer(&timer_becalled);

			Set_IPCallLink_Status(CLink_Idle);

			api_video_c_service_turn_off(); 

			API_talk_off();

			API_CallServer_IpBeCalledQuit();
		}
	}
	else if(msg->msg_head.msg_sub_type == BECALLED_TOUT_CHECKLINK)
	{
		if((BeCalled_Run.state == BECALLED_RINGING  || BeCalled_Run.state == BECALLED_ACK) && BeCalled_Run.timer > 3)
		{
			Global_Addr_Stru gaddr;
			gaddr.gatewayid 	= 0;
			gaddr.ip 			= BeCalled_Run.s_addr.ip;
			gaddr.rt			= 0;
			gaddr.code		= 0;
			if(API_Get_Partner_Calllink_NewCmd(DsAndGl,&gaddr,&partner_call_link) == -1)
			{
				if(++BeCalled_Run.checklink_error	 >= 2)
				{
					quit_flag = 1;
					bprintf("--------quit0----------------------\n");
					goto BTOUT_CHECKLINK_END;
				}
			}
			else
			{
				BeCalled_Run.checklink_error = 0;
				
				if(partner_call_link.Status != CLink_AsCallServer && partner_call_link.Status != CLink_AsCaller)
				{
					bprintf("--------quit1 %d-------------\n",partner_call_link.Status);
					quit_flag = 1;
					goto BTOUT_CHECKLINK_END;
				}
				
				if(partner_call_link.Caller_Data.Call_Target.ip != GetLocalIp() || 
					partner_call_link.Caller_Data.Call_Target.rt != BeCalled_Run.t_addr.rt||
					partner_call_link.Caller_Data.Call_Target.code != BeCalled_Run.t_addr.code)
				{
					bprintf("--------quit2 ip=%08x,rt= %d code= %d-------------\n",partner_call_link.Caller_Data.Call_Target.ip,partner_call_link.Caller_Data.Call_Target.rt,partner_call_link.Caller_Data.Call_Target.code);
					quit_flag = 1;
				}
			}
	BTOUT_CHECKLINK_END:
			if(quit_flag == 1)
			{
				BeCalled_Run.state = BECALLED_WAITING;
		
				BeCalled_Run.timer = 0;
				
				OS_StopTimer(&timer_becalled);

				Set_IPCallLink_Status(CLink_Idle);

				api_video_c_service_turn_off(); 

				API_talk_off();

				API_CallServer_IpBeCalledQuit();
			}
		}
	}
}

/*------------------------------------------------------------------------
				接收到消息(): 错误处理					
------------------------------------------------------------------------*/
void Callback_BeCalled_ToError_VtkUnicastCall(BECALLED_STRUCT *msg)	
{
	IpBeCalled_Business_Rps(msg,0);
	switch(BeCalled_ErrorCode)
	{
		case BECALLED_ERROR_INVITEFAIL:
			Send_VtkUnicastCmd_StateNotice(DsAndGl,&msg->s_addr,&msg->t_addr,VTKU_CALL_STATE_BYE);

			BeCalled_Run.state = BECALLED_WAITING;
	
			BeCalled_Run.timer = 0;
			OS_StopTimer(&timer_becalled);

			Set_IPCallLink_Status(CLink_Idle);

			api_video_c_service_turn_off(); 

			API_talk_off();
			break;
		case BECALLED_ERROR_UINTLINK_CLEAR:
		case BECALLED_ERROR_DTCALLER_QUIT:
			if(BeCalled_Run.state != BECALLED_WAITING)
			{
			
				Send_VtkUnicastCmd_StateNotice(DsAndGl,&BeCalled_Run.s_addr,&BeCalled_Run.t_addr,VTKU_CALL_STATE_BYE);

				BeCalled_Run.state = BECALLED_WAITING;
			
				BeCalled_Run.timer = 0;
				
				OS_StopTimer(&timer_becalled);

				Set_IPCallLink_Status(CLink_Idle);

				api_video_c_service_turn_off(); 

				API_talk_off();

			}
			break;
	
	}
}

void Callback_BeCalled_ToForceClose_VtkUnicastCall(BECALLED_STRUCT *msg)
{
	
	//if(BeCalled_Run.state != BECALLED_WAITING)
	{

		BeCalled_Run.state = BECALLED_WAITING;
	
		BeCalled_Run.timer = 0;

		OS_StopTimer(&timer_becalled);

		Set_IPCallLink_Status(CLink_Idle);

		api_video_c_service_turn_off(); 

		API_talk_off();

	}
}
