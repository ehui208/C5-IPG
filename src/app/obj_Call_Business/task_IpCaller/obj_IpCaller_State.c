/**
  ******************************************************************************
  * @file    obj_Caller_State.c
  * @author  czn
  * @version V00.02.00 (basic on vsip
  * @date    2014.11.06
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2014 V-Tec</center></h2>
  ******************************************************************************
  */ 
#include "../../task_survey/obj_CallServer_Virtual/obj_CallServer_Virtual.h"
#include "task_IpCaller.h"
#include "obj_IpCaller_State.h"
#include "obj_IpCaller_Data.h"
//#include "define_Command.h"



uint8 Caller_UnlockId;
uint8 Caller_ErrorCode;
/*------------------------------------------------------------------------
						Caller_To_Invite
入口:  
	电话号码数组指针, 呼叫类型

处理:


返回: 
	1 = 处理失败 
	0 = 处理成功
------------------------------------------------------------------------*/
uint8 Caller_To_Invite(CALLER_STRUCT *msg)	//R_
{
	uint8 callback_id;
	
	
	//装载Config
	Caller_Data_Init();
	

	//运行参数初始化
	Caller_Run.state 			= CALLER_INVITE;					//状态机
	Caller_Run.call_type 		= msg->call_type;						//呼叫类型
	Caller_Run.s_addr		= msg->s_addr;						//目标地址
	Caller_Run.t_addr			= msg->t_addr;
	//启动INVITE定时	
	Caller_Run.timer = 0;
	OS_RetriggerTimer(&timer_caller);
	
	callback_id = Get_Caller_ToInvite_CallbackID(msg->call_type);
	
	if (callback_id)
	{
		(*TABLE_CALLTYPE_CALLER_TOINVITE_CALLBACK[callback_id-1].callback)(msg);
	}
	
	return 0;


}

/*------------------------------------------------------------------------
						Caller_To_Redial
入口:  
	电话号码数组指针, 呼叫类型

处理:
	执行重拨号操作

返回: 
	1 =	处理失败 
	0 = 处理成功
------------------------------------------------------------------------*/
uint8 Caller_To_Redial(CALLER_STRUCT *msg)	//R	//实际无使用
{
	//will_add_czn
	/*uint8 callback_id;
	
	
	//状态机
	Caller_Run.state 		= CALLER_INVITE;
	Caller_Run.call_type 	= call_type;						//呼叫类型
	Caller_Run.target_addr	= target_addr;						//目标地址
	
	Caller_Run.timer = 0;
	OS_RetriggerTimer(&timer_caller);
	
	callback_id = Get_Caller_ToRedial_CallbackID(call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_CALLER_TOREDIAL_CALLBACK[callback_id-1].callback)();
	}*/
	return 0;
}


/*------------------------------------------------------------------------
					Caller_To_Ringing
入口:  
	电话号码数组指针, 呼叫类型

处理:
	进入RINGING状态

返回: 
	无
------------------------------------------------------------------------*/
void Caller_To_Ringing(CALLER_STRUCT *msg)	//R_
{
	uint8 callback_id;


	//状态机
	Caller_Run.state = CALLER_RINGING;

	//RINGING定时
	Caller_Run.timer = 0;
	OS_RetriggerTimer(&timer_caller);
	
	callback_id = Get_Caller_ToRinging_CallbackID(msg->call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_CALLER_TORINGING_CALLBACK[callback_id-1].callback)(msg);
	}
}

/*------------------------------------------------------------------------
					Caller_To_Ack
入口:  
	电话号码数组指针, control类型

处理:
	状态转移进入ACK状态

返回: 
	无
------------------------------------------------------------------------*/
void Caller_To_Ack(CALLER_STRUCT *msg)	//R_
{
	uint8 callback_id;


	//状态机
	Caller_Run.state = CALLER_ACK;
	
	//ACK定时
	Caller_Run.timer = 0;
	OS_RetriggerTimer(&timer_caller);
	printf("Caller_To_Ack===============\n");
	callback_id = Get_Caller_ToAck_CallbackID(msg->call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_CALLER_TOACK_CALLBACK[callback_id-1].callback)(msg);
	}
	
}

/*------------------------------------------------------------------------
					Caller_To_Bye
入口:  
	control类型

处理:
	状态转移进入BYE状态

返回: 
	无
------------------------------------------------------------------------*/
void Caller_To_Bye(CALLER_STRUCT *msg)	//R_
{
	uint8 callback_id;


	//状态机
	Caller_Run.state = CALLER_BYE;

	//BYE定时
	Caller_Run.timer = 0;
	OS_RetriggerTimer(&timer_caller);

	callback_id = Get_Caller_ToBye_CallbackID(msg->call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_CALLER_TOBYE_CALLBACK[callback_id-1].callback)(msg);
	}
}

/*------------------------------------------------------------------------
					Caller_To_Timeout
入口:  
	control类型

处理:
	超时处理

返回: 
	无
------------------------------------------------------------------------*/
void Caller_To_Timeout(CALLER_STRUCT *msg)	//R_
{
	uint8 callback_id;


	callback_id = Get_Caller_ToTimeout_CallbackID(Caller_Run.call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_CALLER_TOTIMEOUT_CALLBACK[callback_id-1].callback)(msg);
	}
}
/*------------------------------------------------------------------------
					Caller_To_Waiting
入口:  
       control类型

处理:
	状态转移进入WAITING状态

返回: 
	无
------------------------------------------------------------------------*/
void Caller_To_Waiting(CALLER_STRUCT *msg)	//R_
{
	uint8 callback_id;


	//状态机
	Caller_Run.state = CALLER_WAITING;
	
	//关闭定时
	Caller_Run.timer = 0;
	OS_StopTimer(&timer_caller);
	
	callback_id = Get_Caller_ToWaiting_CallbackID(msg->call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_CALLER_TOWAITING_CALLBACK[callback_id-1].callback)(msg);
	}
}

/*-----------------------------------------------
			Caller_Unlock
入口:
       call_type,	    lock_id:锁1 / 锁2

返回:
	无
-----------------------------------------------*/
void Caller_To_Unlock(CALLER_STRUCT *msg)	//R_
{
	uint8 callback_id;


	//记录开锁ID
	Caller_UnlockId = msg->ext_buf[0];
	
	callback_id = Get_Caller_ToUnlock_CallbackID(msg->call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_CALLER_TOUNLOCK_CALLBACK[callback_id-1].callback)(msg);
	}
	
}

/*-----------------------------------------------
			BeCalled_To_Error
入口:
       call_type,	    error_code

返回:
	无
-----------------------------------------------*/
void Caller_To_Error(CALLER_STRUCT *msg)	
{
	uint8 callback_id;


	//记录开锁id，以供回调使用
	Caller_ErrorCode = msg->ext_buf[0];
	
	callback_id = Get_Caller_ToError_CallbackID(msg->call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_CALLER_TOERROR_CALLBACK[callback_id-1].callback)(msg);
	}
}

void Caller_To_ForceClose(CALLER_STRUCT *msg)
{
	uint8 callback_id;

	if(Caller_Run.state != CALLER_WAITING)
	{
		//状态机
		Caller_Run.state = CALLER_WAITING;
		
		//关闭定时
		Caller_Run.timer = 0;
		OS_StopTimer(&timer_caller);
		
		callback_id = Get_Caller_ToForceClose_CallbackID(Caller_Run.call_type);
		if (callback_id)
		{
			(*TABLE_CALLTYPE_CALLER_TOFORCECLOSE_CALLBACK[callback_id-1].callback)(msg);
		}
	}
	IpCaller_Business_Rps(msg,0);
}
/*********************************************************************************************************
**  End Of File
*********************************************************************************************************/
