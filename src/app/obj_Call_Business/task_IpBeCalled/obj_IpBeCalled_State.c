/**
  ******************************************************************************
  * @file    obj_BeCalled_State.c
  * @author  czn
  * @version V00.01.00 (basic on vsip)
  * @date    2014.11.07
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
#include "task_BeCalled.h"
//#include "task_Caller.h"
#include "obj_BeCalled_State.h"
#include "obj_BeCalled_Data.h"
//#include "define_Command.h"
*/
#include "../../task_survey/obj_CallServer_Virtual/obj_CallServer_Virtual.h"
#include "task_IpBeCalled.h"
//#include "task_Caller.h"
#include "obj_IpBeCalled_State.h"
#include "obj_IpBeCalled_Data.h"
#include "../../../os/OSTIME.h"



uint8 BeCalled_UnlockId;
uint8 BeCalled_ErrorCode;
/*------------------------------------------------------------------------
					BeCalled_To_Ringing
入口:  
	源地址，呼叫类型

处理:
	状态转移进入RINGING状态

返回: 
	1 = 处理失败 
	0 = 处理成功
------------------------------------------------------------------------*/
void BeCalled_To_Ringing(BECALLED_STRUCT *msg)	//R_
{
	uint8 callback_id; 

	printf("-------------BeCalled_To_Ringing-------------------\n");
		 
	//装载Config 
	BeCalled_Data_Init();

	//初始化运行参数
	BeCalled_Run.call_type		= msg->call_type;
	BeCalled_Run.s_addr 			= msg->s_addr;
	BeCalled_Run.t_addr			= msg->t_addr;
	BeCalled_Run.state 			= BECALLED_RINGING;
	
	//RINGING定时
	BeCalled_Run.timer = 0;
	OS_RetriggerTimer(&timer_becalled);

	//根据呼叫类型进行菜单,指示灯,提示音等的相关处理
	callback_id = Get_BeCalled_ToRinging_CallbackID(msg->call_type);
	printf("-------------BeCalled_To_Ringing-CallbackID--%d   %d\n",callback_id,msg->call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_BECALLED_TORINGING_CALLBACK[callback_id-1].callback)(msg);
	}
}

/*------------------------------------------------------------------------
					BeCalled_To_Redial
入口:  
	call类型，电话号码数组指针

处理:
	执行重拨号操作

返回: 
	1 =	处理失败 
	0 = 处理成功
------------------------------------------------------------------------*/
uint8 BeCalled_To_Redial(BECALLED_STRUCT *msg)
{
//will_add_czn
	/*uint8 callback_id; 
		 

	//初始化运行参数
	BeCalled_Run.call_type		= call_type;
	BeCalled_Run.partner_source = source_addr;
	BeCalled_Run.state 			= BECALLED_RINGING;
	
	//RINGING定时
	BeCalled_Run.timer = 0;
	OS_RetriggerTimer(&timer_becalled);

	//根据呼叫类型进行菜单,指示灯,提示音等的相关处理
	callback_id = Get_BeCalled_ToRedial_CallbackID(call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_BECALLED_TOREDIAL_CALLBACK[callback_id-1].callback)();
	}*/
	return 0;
}

/*------------------------------------------------------------------------
				BeCalled_To_Transfer
入口:  
	源地址,目标地址，call类型

处理:
	状态转移进入TRANSFER状态

返回: 
	无
------------------------------------------------------------------------*/
void BeCalled_To_Transfer(BECALLED_STRUCT *msg)
{
	/*uint8 callback_id;
	

	BeCalled_Run.partner_target = target_addr;
	BeCalled_Run.state 			= BECALLED_TRANSFER;
	
	//定时
	BeCalled_Run.timer = 0;
	//will_add_czn OS_RetriggerTimer(&timer_becalled);

	
	callback_id = Get_BeCalled_ToTransfer_CallbackID(call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_BECALLED_TOTRANSFER_CALLBACK[callback_id-1].callback)();
	}
	*/
}

/*------------------------------------------------------------------------
					BeCalled_To_Ack
入口:  
	源地址，目标地址，control_type(bit4-7：进入ACK类型，bit0-3：消息子类型)

处理:
	状态转移进入ACK状态

返回: 
	无
------------------------------------------------------------------------*/
void BeCalled_To_Ack(BECALLED_STRUCT *msg)	//R_
{
	uint8 callback_id;
	
	
	BeCalled_Run.state = BECALLED_ACK;
	
	//开定时
	BeCalled_Run.timer = 0;
	OS_RetriggerTimer(&timer_becalled);

	callback_id = Get_BeCalled_ToAck_CallbackID(msg->call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_BECALLED_TOACK_CALLBACK[callback_id-1].callback)(msg);
	}
}

/*------------------------------------------------------------------------
					BeCalled_To_Bye
入口:  
	control类型

处理:
	状态转移进入BYE状态

返回: 
	无
------------------------------------------------------------------------*/
void BeCalled_To_Bye(BECALLED_STRUCT *msg)	//R_
{
	uint8 callback_id;
	
	//进入BYE状态
	BeCalled_Run.state = BECALLED_BYE;
	
	BeCalled_Run.timer = 0;
	OS_RetriggerTimer(&timer_becalled);
	
	callback_id = Get_BeCalled_ToBye_CallbackID(msg->call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_BECALLED_TOBYE_CALLBACK[callback_id-1].callback)(msg);
	}
}

/*------------------------------------------------------------------------
					BeCalled_To_Timeout
入口:  
	control类型

处理:
	超时处理

返回: 
	无
------------------------------------------------------------------------*/
void BeCalled_To_Timeout(BECALLED_STRUCT *msg)	//R_
{
	uint8 callback_id;
	
	callback_id = Get_BeCalled_ToTimeout_CallbackID(BeCalled_Run.call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_BECALLED_TOTIMEOUT_CALLBACK[callback_id-1].callback)(msg);
	}
}

/*------------------------------------------------------------------------
					BeCalled_To_Waiting
入口:  
	control类型

处理:
	状态转移进入WAITING状态

返回: 
	无
------------------------------------------------------------------------*/
void BeCalled_To_Waiting(BECALLED_STRUCT *msg)	//R_
{
	uint8 callback_id;


	BeCalled_Run.state = BECALLED_WAITING;
	
	//关闭定时器
	BeCalled_Run.timer = 0;
	OS_StopTimer(&timer_becalled);
	
	callback_id = Get_BeCalled_ToWaiting_CallbackID(msg->call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_BECALLED_TOWAITING_CALLBACK[callback_id-1].callback)(msg);
	}
	
}

/*-----------------------------------------------
			BeCalled_To_Unlock
入口:
       call_type,	    lock_id:锁1 / 锁2

返回:
	无
-----------------------------------------------*/
void BeCalled_To_Unlock(BECALLED_STRUCT *msg)	//R_
{
	uint8 callback_id;


	//记录开锁id，以供回调使用
	//BeCalled_UnlockId = msg->ext_buf[0];

	callback_id = Get_BeCalled_ToUnlock_CallbackID(msg->call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_BECALLED_TOUNLOCK_CALLBACK[callback_id-1].callback)(msg);
	}
}
/*-----------------------------------------------
			BeCalled_To_Error
入口:
       call_type,	    error_code

返回:
	无
-----------------------------------------------*/
void BeCalled_To_Error(BECALLED_STRUCT *msg)	//R_
{
	uint8 callback_id;


	//记录开锁id，以供回调使用
	BeCalled_ErrorCode = msg->ext_buf[0];
	
	callback_id = Get_BeCalled_ToError_CallbackID(msg->call_type);
	if (callback_id)
	{
		(*TABLE_CALLTYPE_BECALLED_TOERROR_CALLBACK[callback_id-1].callback)(msg);
	}
}

void BeCalled_To_ForceClose(BECALLED_STRUCT *msg)
{
	uint8 callback_id;

	if(BeCalled_Run.state != BECALLED_WAITING)
	{
		//状态机
		BeCalled_Run.state = BECALLED_WAITING;
		
		//关闭定时
		BeCalled_Run.timer = 0;
		OS_StopTimer(&timer_becalled);
		
		callback_id = Get_BeCalled_ToForceClose_CallbackID(BeCalled_Run.call_type);
		if (callback_id)
		{
			(*TABLE_CALLTYPE_BECALLED_TOFORCECLOSE_CALLBACK[callback_id-1].callback)(msg);
		}
	}

	IpBeCalled_Business_Rps(msg,0);
}

/*********************************************************************************************************
**  End Of File
*********************************************************************************************************/
