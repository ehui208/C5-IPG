/**
  ******************************************************************************
  * @file    task_Caller.h
  * @author  czn
  * @version V00.02.00 (basic on vsip)
  * @date    2014.11.06
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2014 V-Tec</center></h2>
  ******************************************************************************
  */ 

#ifndef _task_Caller_H
#define _task_Caller_H

//#include "task_Survey.h"
#include "../../../os/RTOS.h"
#include "../../../os/OSTIME.h"

// Define Task Vars and Structures----------------------------------------------
	 //CALLER状态机
#define CALLER_WAITING			0
#define CALLER_INVITE			1
#define CALLER_RINGING			2
#define CALLER_ACK				3
#define CALLER_BYE				4
	//呼叫结束类型
#define	CALLER_INVITE_ERROR		1
#define	CALLER_INVITE_TIMEOUT	2
#define CALLER_INVITE_BYE		3
#define CALLER_INVITE_CANCEL	4
#define	CALLER_RINGING_TIMEOUT	5
#define CALLER_RINGING_BYE		6
#define CALLER_RINGING_CANCEL	7
#define	CALLER_ACK_TIMEOUT		8
#define CALLER_ACK_BYE			9
#define CALLER_ACK_CANCEL		10
#define CALLER_BYE_OK			11
#define CALLER_BYE_TIMEOUT		12
#define CALLER_BYE_BYE			13

	//第几把锁
#define	CALLER_UNLOCK1			1
#define	CALLER_UNLOCK2			2

extern vdp_task_t	task_caller;

#define MAX_CALLER_MSGBUF_LEN		20
// Define task interface-------------------------------------------------------------
#pragma pack(1)

typedef struct {
	VDP_MSG_HEAD 		msg_head;
	unsigned char			call_type;
	Global_Addr_Stru 		s_addr;
	Global_Addr_Stru 		t_addr;
	int			 	 	ext_len;
	unsigned char 		ext_buf[MAX_CALLER_MSGBUF_LEN];
} CALLER_STRUCT ;

#pragma pack()

//Caller_Run
typedef struct CALLER_RUN_STRU
{	
	unsigned char			state;					//呼叫状态
	unsigned char			call_type;				//呼叫类型
	Global_Addr_Stru 		s_addr;
	Global_Addr_Stru 		t_addr;
	unsigned short		timer;					//呼叫定时
	unsigned short		checklink_error;
}	CALLER_RUN;

extern CALLER_RUN Caller_Run;

#define CALLER_STRUCT_BASIC_LENGTH	((unsigned int)&(((CALLER_STRUCT*)0)->ext_buf[0]))
		//msg_type
typedef enum
{
	CALLER_MSG_INVITE	= 1,
	CALLER_MSG_RINGING,
	CALLER_MSG_ACK,
	CALLER_MSG_CANCEL,
	CALLER_MSG_BYE,
	CALLER_MSG_TRANSFER,
	CALLER_MSG_ERROR,
	CALLER_MSG_REDIAL,
	CALLER_MSG_UNLOCK1,
	CALLER_MSG_UNLOCK2,
	CALLER_MSG_TIMEOUT,
	CALLER_MSG_GETSTATE,
	CALLER_MSG_FORCECLOSE,
}IP_CALLER_MSG_TYPE;

typedef enum
{
	CALLER_TOUT_TIMEOVER = 0,
	CALLER_TOUT_CHECKLINK,
}IP_CALLER_TIMEOUTMSG_SUBTYPE;

typedef enum
{
	CALLER_ERROR_INVITEFAIL			= 1,
	CALLER_ERROR_DTCALLER_QUIT,
	CALLER_ERROR_DTBECALLED_QUIT,
	CALLER_ERROR_UINTLINK_CLEAR,
}IP_CALLER_ERROR_TYPE;

// Define Task 2 items----------------------------------------------------------
extern OS_TIMER timer_caller;	//软定时_Caller定时结束
void vtk_TaskInit_Caller(void);	
void vtk_TaskProcessEvent_Caller(CALLER_STRUCT	*msg_caller);
void init_vdp_caller_task(void);

// Define Task others-----------------------------------------------------------
void Caller_Timer_Callback(void);	
void Caller_Waiting_Process(CALLER_STRUCT *msg_caller);
void Caller_Invite_Process(CALLER_STRUCT *msg_caller);
void Caller_Ringing_Process(CALLER_STRUCT *msg_caller);
void Caller_Ack_Process(CALLER_STRUCT *msg_caller);
void Caller_Bye_Process(CALLER_STRUCT *msg_caller);
void Caller_StateInvalid_Process(CALLER_STRUCT *msg_caller);
void Caller_MsgInvalid_Process(CALLER_STRUCT *msg_caller);
uint8 Get_Caller_State(void);
uint16 Get_Caller_PartnerAddr(void);
void Get_Caller_State_Rsp(CALLER_STRUCT *msg_caller);
void IpCaller_Business_Rps(CALLER_STRUCT *msg,unsigned char result);
// Define API-------------------------------------------------------------------
/*uint8 API_Caller_Common(uint8 msg_type_temp , uint8 call_type_temp,uint16 target_addr_temp);

	//BDU(BeCalled)呼叫分机申请(小区主机呼叫)
	#define API_Caller_Invite(call_type_temp,target_addr_temp)		\
		API_Caller_Common(CALLER_MSG_INVITE, call_type_temp,target_addr_temp)

	//分机应答_呼叫成功 => Caller		
	#define API_Caller_Ringing(call_type_temp)	\
		API_Caller_Common(CALLER_MSG_RINGING, call_type_temp,NULL)
	
	//分机发送_摘机 => Caller		
	#define API_Caller_Ack(call_type_temp)		\
		API_Caller_Common(CALLER_MSG_ACK, call_type_temp,NULL)
	
	//结束呼叫(对链路操作) => Caller		
	#define API_Caller_Bye(call_type_temp)			\
		API_Caller_Common(CALLER_MSG_BYE, call_type_temp,NULL)

	//结束呼叫(对链路操作) => Caller		
	#define API_Caller_Close(call_type_temp)			\
		API_Caller_Common(CALLER_MSG_BYE, call_type_temp,NULL)

	//		
	#define API_Caller_Error(call_type_temp)		\
		API_Caller_Common(CALLER_MSG_ERROR, call_type_temp,NULL)

	//Caller呼叫超时(已在软件定时中处理)
	#define API_Caller_Timeout(call_type_temp)	API_Caller_Common(CALLER_MSG_TIMEOUT, call_type_temp,NULL)

	//取消呼叫(清除链路, 取消呼叫分机)=>Caller
	#define API_Caller_Cancel(call_type_temp)		\
		API_Caller_Common(CALLER_MSG_CANCEL ,call_type_temp,NULL)
			
	//分机发送_开锁1 => Caller
	#define API_Caller_Unlock1(call_type_temp)		\
		API_Caller_Common(CALLER_MSG_UNLOCK1 ,call_type_temp,NULL)	
	
	//分机发送_开锁2 => Caller
	#define API_Caller_Unlock2(call_type_temp)		\
		API_Caller_Common(CALLER_MSG_UNLOCK2 ,call_type_temp,NULL)	*/		
#endif
		