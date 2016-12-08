/**
  ******************************************************************************
  * @file    task_CommandInterface.c
  * @author  czn
  * @version V00.01.00 
  * @date    2016.04.19
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2016 V-Tec</center></h2>
  ******************************************************************************
  */ 

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>

#include "../obj_CallServer_Virtual/obj_CallServer_Virtual.h"
#include "../../vtk_udp_stack/vtk_udp_stack_c5_ipc_cmd.h"
#include "obj_VtkUnicastCommand_Analyze.h"
#include "../../obj_Call_Business/task_IpBeCalled/task_IpBeCalled.h"
#include "../../obj_Call_Business/task_IpCaller/task_IpCaller.h"

VTKUNICASTCMD_ANALYZE_RUN_STRU	VtkUnicastCmdAnalyze_Run;
//czn_020160422
//czn_20160516

void Set_VtkUnicastCmdAnalyze_State(uint8 newstate)
{
	VtkUnicastCmdAnalyze_Run.state = newstate;
}

int API_VtkUnicastCmd_Analyzer(int source_ip,int cmd, int sn,uint8 *pkt , uint16 len)
{
	
	int rev = -1;
	VtkUnicastCmd_Stru *VtkUnicastCmd;
	unsigned char dtcall_type,ipcall_type;
	Global_Addr_Stru s_addr,t_addr;
	
	VtkUnicastCmd = (VtkUnicastCmd_Stru *)pkt;
	//vtkcmd = (VtkUnicastCmd->cmd_type<<8) |VtkUnicastCmd->cmd_sub_type;
	switch(cmd)
	{
		case VTK_CMD_DIAL_1003:
			ipcall_type 			= IpCallType_VtkUnicast;
			s_addr.gatewayid 		= 0;
			s_addr.ip				= source_ip;
			s_addr.rt 			= VtkUnicastCmd->source_idh;
			s_addr.code 			= VtkUnicastCmd->source_idl;
			t_addr.gatewayid		= 0;
			t_addr.ip				= GetLocalIp();
			t_addr.rt 				= VtkUnicastCmd->target_idh;
			t_addr.code 			= VtkUnicastCmd->target_idl;

			if(t_addr.rt == 0 && t_addr.code <= 32)
			{
				dtcall_type 	= DtCallType_CallIM;
				VtkUnicastCmdAnalyze_Run.save_sn = sn;
				API_CallServer_IpStartCall(dtcall_type, ipcall_type, &s_addr, &t_addr);
			}

			rev = 0;
			break;
			
		case VTK_CMD_DIAL_REP_1083:
			if(VtkUnicastCmdAnalyze_Run.state == VTKUNICASTCMD_ANALYZE_ASCALLSOURCE)
			{
				if(source_ip == Caller_Run.t_addr.ip)
				{
					API_CallServer_InviteOk();
					rev = 0;
				}
			}
			break;

		case VTK_CMD_STATE_1005:
			if(VtkUnicastCmd->call_code == VTKU_CALL_STATE_TALK)
			{
				if(VtkUnicastCmdAnalyze_Run.state == VTKUNICASTCMD_ANALYZE_ASCALLSOURCE)
				{
					//if(source_ip == Caller_Run.t_addr.ip && VtkUnicastCmd->target_idh == Caller_Run.t_addr.rt && VtkUnicastCmd->target_idl== Caller_Run.t_addr.code)
					if(source_ip == Caller_Run.t_addr.ip)//czn_20160613
					{
						VtkUnicastCmdAnalyze_Run.save_sn = sn;
						API_CallServer_TargetOffhook();
						rev = 0;
					}
					
				}
			}
			else if(VtkUnicastCmd->call_code == VTKU_CALL_STATE_BYE)
			{
				
				if(VtkUnicastCmdAnalyze_Run.state == VTKUNICASTCMD_ANALYZE_ASCALLSOURCE)
				{
					//if(source_ip == Caller_Run.t_addr.ip && VtkUnicastCmd->target_idh == Caller_Run.t_addr.rt && VtkUnicastCmd->target_idl== Caller_Run.t_addr.code)
					if(source_ip == Caller_Run.t_addr.ip)//czn_20160613
					{
						VtkUnicastCmdAnalyze_Run.save_sn = sn;
						API_CallServer_TargetCancel();
						rev = 0;
					}
					
				}
				else if(VtkUnicastCmdAnalyze_Run.state == VTKUNICASTCMD_ANALYZE_ASCALLTARGET)
				{
					//if(source_ip == BeCalled_Run.s_addr.ip && VtkUnicastCmd->source_idh == BeCalled_Run.s_addr.rt && VtkUnicastCmd->source_idl == BeCalled_Run.s_addr.code)
					if(source_ip == BeCalled_Run.s_addr.ip)//czn_20160613
					{
						VtkUnicastCmdAnalyze_Run.save_sn = sn;
						API_CallServer_SourceCancel();
						rev = 0;
					}
				}
			}
			break;
			
		case VTK_CMD_STATE_REP_1085:
			printf("--------recv VTK_CMD_STATE_REP_1085\n");
			if(VtkUnicastCmdAnalyze_Run.state == VTKUNICASTCMD_ANALYZE_ASCALLSOURCE)
			{
				if(source_ip == Caller_Run.t_addr.ip)
				{
					printf("--------source---API_CallServer_IpStateNoticeAckOk\n");
					API_CallServer_IpStateNoticeAckOk();
					rev = 0;
				}
			}
			else if(VtkUnicastCmdAnalyze_Run.state == VTKUNICASTCMD_ANALYZE_ASCALLTARGET)
			{
				if(source_ip == BeCalled_Run.s_addr.ip)
				{

					printf("--------target---API_CallServer_IpStateNoticeAckOk\n");
					API_CallServer_IpStateNoticeAckOk();
					rev = 0;
				}
			}
			break;

		case VTK_CMD_LINK_1001:
			
			break;
			
		case VTK_CMD_LINK_REP_1081:
			
			break;

		case VTK_CMD_UNLINK_1002:
			
			break;
			
		case VTK_CMD_UNLINK_REP_1082:
			
			break;	

		case VTK_CMD_UNLOCK_E003:
			if(source_ip == Caller_Run.t_addr.ip)
			{
				API_CallServer_TargetUnlockReq(&VtkUnicastCmd->call_code);
			}
			break;
			
			
			
	}
			
	return rev;
}

int Send_VtkUnicastCmd_Invite(unsigned char call_type,Global_Addr_Stru *s_addr,Global_Addr_Stru *t_addr)
{
	VtkUnicastCmd_Stru VtkUnicastCmd;
	int target_ip = t_addr->ip;
	
	//VtkUnicastCmd.cmd_type 		= VTK_CMD_DIAL_1003>>8;
	//VtkUnicastCmd.cmd_sub_type 	= VTK_CMD_DIAL_1003 & 0xff; 
	VtkUnicastCmd.call_type		= call_type;
	VtkUnicastCmd.source_idh		= s_addr->rt;
	VtkUnicastCmd.source_idl		= s_addr->code;
	VtkUnicastCmd.call_code		= 0;
	VtkUnicastCmd.target_idh		= t_addr->rt;
	VtkUnicastCmd.target_idl		= t_addr->code;
	
	
	return api_udp_c5_ipc_send_data(target_ip,VTK_CMD_DIAL_1003,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru));
	
}

int Send_VtkUnicastCmd_InviteAck(unsigned char call_type,Global_Addr_Stru *t_addr,unsigned char result)
{
	VtkUnicastCmd_Stru VtkUnicastCmd;
	int target_ip;
	
	target_ip = t_addr->ip;
	VtkUnicastCmd.call_type		= call_type;
	VtkUnicastCmd.rspstate			= result;
	printf("Send_VtkUnicastCmd_InviteAck------%08x\n",target_ip);
	return api_udp_c5_ipc_send_rsp(target_ip,VTK_CMD_DIAL_REP_1083,VtkUnicastCmdAnalyze_Run.save_sn,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru));
}




int Send_VtkUnicastCmd_StateNotice(unsigned char call_type,Global_Addr_Stru *s_addr,Global_Addr_Stru *t_addr,unsigned char new_state)
{
	VtkUnicastCmd_Stru VtkUnicastCmd;
	int target_ip;
	if(VtkUnicastCmdAnalyze_Run.state == VTKUNICASTCMD_ANALYZE_ASCALLSOURCE)
	{
		target_ip = t_addr->ip;
	}
	else
	{
		target_ip = s_addr->ip;
	}
	VtkUnicastCmd.call_type		= call_type;
	VtkUnicastCmd.source_idh		= s_addr->rt;
	VtkUnicastCmd.source_idl		= s_addr->code;
	VtkUnicastCmd.call_code		= new_state;
	VtkUnicastCmd.target_idh		= t_addr->rt;
	VtkUnicastCmd.target_idl		= t_addr->code;
			
	return api_udp_c5_ipc_send_data(target_ip,VTK_CMD_STATE_1005,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru));
}

int Send_VtkUnicastCmd_StateNoticeAck(unsigned char call_type,Global_Addr_Stru *t_addr,unsigned char result)
{
	VtkUnicastCmd_Stru VtkUnicastCmd;
	int target_ip;
	
	target_ip = t_addr->ip;
	VtkUnicastCmd.call_type		= call_type;
	VtkUnicastCmd.rspstate			= result;
	
	return api_udp_c5_ipc_send_rsp(target_ip,VTK_CMD_STATE_REP_1085,VtkUnicastCmdAnalyze_Run.save_sn,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru));
}


//czn_020160422_s
int API_Get_Partner_Calllink_NewCmd(unsigned char call_type,Global_Addr_Stru *target_addr,_IP_Call_Link_Run_Stru *partner_call_link)
{
	VtkUnicastCmd_Stru VtkUnicastCmd;
	int rlen = sizeof(_IP_Call_Link_Run_Stru);
	int target_ip;

	target_ip = target_addr->ip;

	//VtkUnicastCmd.cmd_type 		= VTK_CMD_LINK_1001>> 8;
	//VtkUnicastCmd.cmd_sub_type 	= VTK_CMD_LINK_1001& 0xff; 

	// lzh_20160503_s
	VtkUnicastCmd.call_type		= call_type;		// 不同于 GlMonMr 即可，监视和呼叫的link需要区分
	// lzh_20160503_e
	VtkUnicastCmd.target_idh		= target_addr->rt;
	VtkUnicastCmd.target_idl		= target_addr->code;
	
	char *rbuf = (char *)malloc(rlen);

	if(rbuf == NULL)
	{
		return -1;
	}
	//if(api_udp_c5_ipc_send_req1(target_ip,VTK_CMD_LINK_1001,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru))==-1)
	// lzh_20160512_s
	//if(api_udp_c5_ipc_send_req(target_ip,VTK_CMD_LINK_1001,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru),ptask,rbuf,&rlen) == -1)
	bprintf("Get_Partner %08x\n",target_ip);
	if(api_udp_c5_ipc_send_req(target_ip,VTK_CMD_LINK_1001,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru),rbuf,&rlen) == -1)
	// lzh_20160512_e
	{
		bprintf("api_udp_c5_ipc_send_req\n");
		free(rbuf);
		return -1;
	}
	
	memcpy(partner_call_link,rbuf,sizeof(_IP_Call_Link_Run_Stru));

	free(rbuf);

	return 0;
	
	
}

int Send_UnlinkCmd(unsigned char call_type,Global_Addr_Stru *target_addr,_IP_Call_Link_Run_Stru *partner_call_link)
{
	VtkUnicastCmd_Stru VtkUnicastCmd;
	
	if(partner_call_link->Status == CLink_Idle)
	{
		return -1;
	}
	
	//VtkUnicastCmd.cmd_type 		= VTK_CMD_UNLINK_1002>> 8;
	//VtkUnicastCmd.cmd_sub_type 	= VTK_CMD_UNLINK_1002& 0xff;
	VtkUnicastCmd.call_type		= call_type;
	// lzh_20160512_s
	//if(api_udp_c5_ipc_send_req(target_addr->ip,VTK_CMD_UNLINK_1002,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru),ptask,NULL,NULL) == -1)
	if(api_udp_c5_ipc_send_req(target_addr->ip,VTK_CMD_UNLINK_1002,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru),NULL,NULL) == -1)
	// lzh_20160512_e
	{
		return -1;
	}
	if(partner_call_link->Status == CLink_AsBeCalled)
	{
		bprintf("Status == CLink_AsBeCalled %08x,%08x\n",partner_call_link->BeCalled_Data.Call_Source.ip,GetLocalIp());
		if(partner_call_link->BeCalled_Data.Call_Source.ip !=GetLocalIp())
		{
			// lzh_20160512_s		
			//if(api_udp_c5_ipc_send_req(partner_call_link->BeCalled_Data.Call_Source.ip,VTK_CMD_UNLINK_1002,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru),ptask,NULL,NULL) == -1)
			if(api_udp_c5_ipc_send_req(partner_call_link->BeCalled_Data.Call_Source.ip,VTK_CMD_UNLINK_1002,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru),NULL,NULL) == -1)
			// lzh_20160512_e			
			{
				return -1;
			}

		}
	}

	if(partner_call_link->Status == CLink_AsCaller)
	{
		bprintf("Status == CLink_AsCaller %08x,%08x\n",partner_call_link->Caller_Data.Call_Target.ip,GetLocalIp());
		if(partner_call_link->Caller_Data.Call_Target.ip !=GetLocalIp())
		{
			// lzh_20160512_s			
			//if(api_udp_c5_ipc_send_req(partner_call_link->Caller_Data.Call_Target.ip,VTK_CMD_UNLINK_1002,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru),ptask,NULL,NULL) == -1)
			if(api_udp_c5_ipc_send_req(partner_call_link->Caller_Data.Call_Target.ip,VTK_CMD_UNLINK_1002,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru),NULL,NULL) == -1)
			// lzh_20160512_e				
			{
				return -1;
			}

		}
	}
	
	if(partner_call_link->Status == CLink_AsCallServer)
	{
		bprintf("Status == CLink_AsCallServer %08x,%08x\n",partner_call_link->Caller_Data.Call_Target.ip,GetLocalIp());
		if(partner_call_link->BeCalled_Data.Call_Source.ip !=GetLocalIp() && partner_call_link->BeCalled_Data.Call_Source.ip != target_addr->ip)
		{
			// lzh_20160512_s		
			//if(api_udp_c5_ipc_send_req(partner_call_link->BeCalled_Data.Call_Source.ip,VTK_CMD_UNLINK_1002,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru),ptask,NULL,NULL) == -1)
			if(api_udp_c5_ipc_send_req(partner_call_link->BeCalled_Data.Call_Source.ip,VTK_CMD_UNLINK_1002,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru),NULL,NULL) == -1)
			// lzh_20160512_e			
			{
				return -1;
			}

		}
		if(partner_call_link->Caller_Data.Call_Target.ip !=GetLocalIp() && partner_call_link->Caller_Data.Call_Target.ip != target_addr->ip)
		{
			if(api_udp_c5_ipc_send_req(partner_call_link->Caller_Data.Call_Target.ip,VTK_CMD_UNLINK_1002,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru),NULL,NULL) == -1)		
			{
				return -1;
			}

		}

		
	}

	if(partner_call_link->Status == CLink_HaveLocalCall)
	{
		//czn_will_add
	}
	return 0;
}

int Send_VtkUnicastCmd_TargetUnlockReq(unsigned char call_type,Global_Addr_Stru *t_addr,unsigned char locknum)
{
	VtkUnicastCmd_Stru VtkUnicastCmd;
	int target_ip;
	
	target_ip = t_addr->ip;
	VtkUnicastCmd.call_type		= call_type;
	VtkUnicastCmd.call_code		= locknum;
	
	return api_udp_c5_ipc_send_data(target_ip,VTK_CMD_UNLOCK_E003,(char*)&VtkUnicastCmd,sizeof(VtkUnicastCmd_Stru));
}

//czn_020160422_e
/*********************************************************************************************************
**  End Of File
*********************************************************************************************************/

