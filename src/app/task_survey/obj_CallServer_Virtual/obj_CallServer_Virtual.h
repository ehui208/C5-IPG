/**
  ******************************************************************************
  * @file    task_BusinessAnalyze.h
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
#ifndef _obj_CallServer_Virtual_h
#define _obj_CallServer_Virtual_h

#include "obj_IP_Call_Link.h"
#include "../task_survey.h"
#include "../obj_CommandAnalyze/obj_VtkUnicastCommand_Analyze.h"
#include "../../obj_Call_Business/task_IpBeCalled/task_IpBeCalled.h"
#include "../../obj_Call_Business/task_IpCaller/task_IpCaller.h"

typedef enum
{
	IpCallType_VtkUnicast = 0,
	IpCallType_VtkMulticast,
	IpCallType_Sip,
}IpCallType_Type;

typedef enum
{
	DtCallType_CdsVsipCall = 0,
	DtCallType_CallIM,
	DtCallType_ForceCallDs,
}DtCallType_Type;

typedef enum
{
	CallServer_Wait = 0,
	CallServer_Invite,
	CallServer_Ring,
	CallServer_Ack,
	CallServer_SourceBye,
	CallServer_TargetBye,
}CallServer_State;

typedef enum
{
	CallServer_AsCallSource = 0,
	CallServer_AsCallTarget,
	CallServer_AsForceCallSource,
}CallServer_WorkMode;

typedef enum
{
	CallServer_Msg_DtStartCall = 0,
	CallServer_Msg_IpStartCall,
	CallServer_Msg_IpgForceStartCall,
	CallServer_Msg_InviteOk,
	CallServer_Msg_InviteFail,
	CallServer_Msg_TargetOffhook,
	CallServer_Msg_TargetCancel,
	CallServer_Msg_SourceCancel,
	CallServer_Msg_DtStateNoticeAckOk,
	CallServer_Msg_DtStateNoticeAckFail,
	CallServer_Msg_IpStateNoticeAckOk,
	CallServer_Msg_IpStateNoticeAckFail,
	CallServer_Msg_DtCallerQuit,
	CallServer_Msg_IpCallerQuit,
	CallServer_Msg_DtBeCalledQuit,
	CallServer_Msg_IpBeCalledQuit,
	CallServer_Msg_GetCallLink,
	CallServer_Msg_GetMonLink,
	CallServer_Msg_Unlink,
	CallServer_Msg_UnitLinkClear,
	CallServer_Msg_TargetUnlockReq,
}CallServer_AbstractMsg_Type;

typedef struct
{
	unsigned char 		state;
	unsigned char 		call_mode;
	unsigned char			dtcall_type;
	unsigned char	 		ipcall_type;
	Global_Addr_Stru 		s_addr;
	Global_Addr_Stru 		t_addr;
}CallServer_Run_Stru;

extern CallServer_Run_Stru CallServer_Run;

#define MAX_CALLSERVER_MSGBUF_LEN		20
#pragma pack(1)
typedef struct
{
	VDP_MSG_HEAD 		msg_head;
	unsigned char 		dtcall_type;
	unsigned char 		ipcall_type;
	Global_Addr_Stru 		s_addr;
	Global_Addr_Stru 		t_addr;
	int			 	 	ext_len;
	unsigned char ext_buf[MAX_CALLSERVER_MSGBUF_LEN];
}CALLSERVER_STRU;

#define CALLSERVER_STRU_HEAD	((unsigned int)&(((CALLSERVER_STRU*)0)->ext_buf[0]))




int API_CallServer_Common(unsigned char msg_type,unsigned char dtcall_type,unsigned char ipcall_type,
							Global_Addr_Stru *s_addr,Global_Addr_Stru *t_addr,unsigned char *ext_buf,int ext_len);

#define API_CallServer_IpStartCall(dtcall_type,ipcall_type,s_addr,t_addr)	\
	API_CallServer_Common(CallServer_Msg_IpStartCall,dtcall_type,ipcall_type,s_addr,t_addr,NULL,NULL)

#define API_CallServer_IpStartCall(dtcall_type,ipcall_type,s_addr,t_addr)	\
	API_CallServer_Common(CallServer_Msg_IpStartCall,dtcall_type,ipcall_type,s_addr,t_addr,NULL,NULL)

#define API_CallServer_InviteOk()	\
		API_CallServer_Common(CallServer_Msg_InviteOk,NULL,NULL,NULL,NULL,NULL,NULL)
			
#define API_CallServer_InviteFail()	\
		API_CallServer_Common(CallServer_Msg_InviteFail,NULL,NULL,NULL,NULL,NULL,NULL)

#define API_CallServer_TargetOffhook()	\
		API_CallServer_Common(CallServer_Msg_TargetOffhook,NULL,NULL,NULL,NULL,NULL,NULL)

#define API_CallServer_SourceCancel()	\
		API_CallServer_Common(CallServer_Msg_SourceCancel,NULL,NULL,NULL,NULL,NULL,NULL)

#define API_CallServer_TargetCancel()	\
		API_CallServer_Common(CallServer_Msg_TargetCancel,NULL,NULL,NULL,NULL,NULL,NULL)
			
#define API_CallServer_DtStateNoticeAckOk()	\
		API_CallServer_Common(CallServer_Msg_DtStateNoticeAckOk,NULL,NULL,NULL,NULL,NULL,NULL)
			
#define API_CallServer_DtStateNoticeAckFail()	\
		API_CallServer_Common(CallServer_Msg_DtStateNoticeAckFail,NULL,NULL,NULL,NULL,NULL,NULL)

#define API_CallServer_IpStateNoticeAckOk()	\
		API_CallServer_Common(CallServer_Msg_IpStateNoticeAckOk,NULL,NULL,NULL,NULL,NULL,NULL)
			
#define API_CallServer_IpStateNoticeAckFail()	\
		API_CallServer_Common(CallServer_Msg_IpStateNoticeAckFail,NULL,NULL,NULL,NULL,NULL,NULL)

#define API_CallServer_GetCallLink(s_addr)	\
		API_CallServer_Common(CallServer_Msg_GetCallLink,NULL,NULL,s_addr,NULL,NULL,NULL)			

#define API_CallServer_GetMonLink(s_addr)	\
		API_CallServer_Common(CallServer_Msg_GetMonLink,NULL,NULL,s_addr,NULL,NULL,NULL)
			
#define API_CallServer_Unlink()	\
		API_CallServer_Common(CallServer_Msg_Unlink,NULL,NULL,NULL,NULL,NULL,NULL)

#define API_CallServer_UnitLinkClear()	\
		API_CallServer_Common(CallServer_Msg_UnitLinkClear,NULL,NULL,NULL,NULL,NULL,NULL)

#define API_CallServer_DtBeCalledQuit()	\
		API_CallServer_Common(CallServer_Msg_DtBeCalledQuit,NULL,NULL,NULL,NULL,NULL,NULL)

#define API_CallServer_DtCallerQuit()	\
		API_CallServer_Common(CallServer_Msg_DtCallerQuit,NULL,NULL,NULL,NULL,NULL,NULL)

#define API_CallServer_IpBeCalledQuit()	\
		API_CallServer_Common(CallServer_Msg_IpBeCalledQuit,NULL,NULL,NULL,NULL,NULL,NULL)

#define API_CallServer_IpCallerQuit()	\
		API_CallServer_Common(CallServer_Msg_IpCallerQuit,NULL,NULL,NULL,NULL,NULL,NULL)

#define API_CallServer_TargetUnlockReq(locknum)	\
		API_CallServer_Common(CallServer_Msg_TargetUnlockReq,NULL,NULL,NULL,NULL,locknum,1)

#endif
