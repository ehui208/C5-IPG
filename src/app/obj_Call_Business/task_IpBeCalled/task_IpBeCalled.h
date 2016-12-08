/**
  ******************************************************************************
  * @file    task_BeCalled.h
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

#ifndef _task_BeCalled_H
#define _task_BeCalled_H

//#include "task_Survey.h"
#include "../../../os/RTOS.h"
#include "../../../os/OSTIME.h"

//#include "BSP.h"
//#include "task_Survey.h"

// Define Task Vars and Structures----------------------------------------------
	 //run.state
#define BECALLED_WAITING		0
#define BECALLED_RINGING		1
		#define BECALLED_RINGING_SUB_READY		10
		#define BECALLED_RINGING_SUB_DIAL		11
		#define BECALLED_RINGING_SUB_RINGING	12
#define BECALLED_ACK			2
#define BECALLED_BYE			3
#define BECALLED_TRANSFER      	4

	//第几把锁
#define	BECALLED_UNLOCK1		1
#define	BECALLED_UNLOCK2		2

#define BECALLED_MAX_SUBMSG		16

#define MAX_BECALLED_MSGBUF_LEN		20
// Define task interface-------------------------------------------------------------
#pragma pack(1)

typedef struct {
	VDP_MSG_HEAD 		msg_head;
	unsigned char			call_type;
	Global_Addr_Stru 		s_addr;
	Global_Addr_Stru 		t_addr;
	int			 	 	ext_len;
	unsigned char 		ext_buf[MAX_BECALLED_MSGBUF_LEN];
}BECALLED_STRUCT;

#pragma pack()

#define BECALLED_STRUCT_BASIC_LENGTH  (uint16)&(((BECALLED_STRUCT*)0)->ext_buf[0])

typedef struct BECALLED_RUN_STRU	//Caller_run
{	
	unsigned char			state;					//呼叫状态
	unsigned char			call_type;				//呼叫类型
	Global_Addr_Stru 		s_addr;
	Global_Addr_Stru 		t_addr;
	unsigned short		timer;					//呼叫定时
	unsigned short		checklink_error;
}BECALLED_RUN;

extern BECALLED_RUN BeCalled_Run;
	//msg_type
typedef enum
{
	BECALLED_MSG_INVITE	= 1,
	BECALLED_MSG_RINGING,
	BECALLED_MSG_ACK,
	BECALLED_MSG_CANCEL,
	BECALLED_MSG_BYE,
	BECALLED_MSG_TRANSFER,
	BECALLED_MSG_ERROR,
	BECALLED_MSG_REDIAL,
	BECALLED_MSG_UNLOCK1,
	BECALLED_MSG_UNLOCK2,
	BECALLED_MSG_TIMEOUT,
	BECALLED_MSG_GETSTATE,
	BECALLED_MSG_FORCECLOSE,
}IP_BECALLED_MSG_TYPE;

typedef enum
{
	BECALLED_TOUT_TIMEOVER = 0,
	BECALLED_TOUT_CHECKLINK,
}IP_BECALLED_TIMEOUTMSG_SUBTYPE;

typedef enum
{
	BECALLED_ERROR_INVITEFAIL			= 1,
	BECALLED_ERROR_DTCALLER_QUIT,
	BECALLED_ERROR_DTBECALLED_QUIT,
	BECALLED_ERROR_UINTLINK_CLEAR,
}IP_BECALLED_ERROR_TYPE;

		//结束呼叫_类型


#define	BECALLED_UNLOCK1				1
#define	BECALLED_UNLOCK2				2

extern vdp_task_t	task_becalled;
// Define Task 2 items----------------------------------------------------------
extern OS_TIMER timer_becalled;

void vtk_TaskInit_BeCalled(void);	
void vtk_TaskProcessEvent_BeCalled(BECALLED_STRUCT  *msg_becalled);
void init_vdp_becalled_task(void);

//Define the functions
void BeCalled_Waiting_Process(BECALLED_STRUCT  *msg_becalled);
void BeCalled_Ringing_Process(BECALLED_STRUCT  *msg_becalled);
void BeCalled_Ack_Process(BECALLED_STRUCT  *msg_becalled);
void BeCalled_Bye_Process(BECALLED_STRUCT  *msg_becalled);
void BeCalled_Transfer_Process(BECALLED_STRUCT  *msg_becalled);
void BeCalled_StateInvalid_Process(BECALLED_STRUCT *msg_becalled);
void BeCalled_MsgInvalid_Process(BECALLED_STRUCT *msg_becalled);
uint8 Get_BeCalled_State(void);
uint16 Get_BeCalled_PartnerAddr(void);
void Get_BeCalled_State_Rsp(BECALLED_STRUCT *msg_becalled);
void IpBeCalled_Business_Rps(BECALLED_STRUCT *msg,unsigned char result);

// Define Task others-----------------------------------------------------------
void BeCalled_Timer_Callback(void);


// Define API-------------------------------------------------------------------
/*uint8 API_BeCalled_Common(uint8 msg_type_temp, uint8 call_type_temp, 
						  uint16 address_s_temp,uint16 address_t_temp);

	//小区主机_呼叫申请 => BeCalled
	#define API_BeCalled_Invite(call_type_temp,address_s_temp, address_t_temp)	\
		API_BeCalled_Common(BECALLED_MSG_INVITE, call_type_temp,address_s_temp,address_t_temp)

	//小区主机_呼叫错误 => BeCalled
	#define API_BeCalled_Error(void)	\
		API_BeCalled_Common(BECALLED_MSG_ERROR,NULL,NULL,NULL)

	//分机发送_摘机 => BeCalled
	#define API_BeCalled_Ack(call_type_temp)	\
		API_BeCalled_Common(BECALLED_MSG_ACK,call_type_temp,NULL,NULL)

	//取消呼叫(关闭音视频,不对外设操作)=>BeCalled
	#define API_BeCalled_Cancel(call_type_temp)			\
		API_BeCalled_Common(BECALLED_MSG_CANCEL,call_type_temp,NULL,NULL)

	//结束呼叫(对小区主机_结束呼叫) => BeCalled
	#define API_BeCalled_Bye(void)	\
		API_BeCalled_Common(BECALLED_MSG_BYE,NULL,NULL,NULL)

	//分机应答_呼叫成功 => BeCalled
	#define API_BeCalled_Ringing(void)	\
		API_BeCalled_Common(BECALLED_MSG_RINGING,NULL,NULL,NULL)

	//BeCalled呼叫超时(在软定时中已处理)
	#define API_BeCalled_TimeOut(void)	\
		API_BeCalled_Common(BECALLED_MSG_TIMEOUT,NULL,NULL,NULL)
			
	//分机发送_开锁1 => BeCalled
	#define API_BeCalled_Unlock1(void)	\
		API_BeCalled_Common(BECALLED_MSG_UNLOCK1,NULL,NULL,NULL)
			
	//分机发送_开锁2 => BeCalled
	#define API_BeCalled_Unlock2(void)	\
		API_BeCalled_Common(BECALLED_MSG_UNLOCK2,NULL,NULL,NULL)		

*/
#endif
