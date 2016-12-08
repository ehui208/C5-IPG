

#ifndef _obj_VtkUnicastCommand_Analyze_h
#define _obj_VtkUnicastCommand_Analyze_h


#include "../../../os/RTOS.h"



// Define Task Vars and Structures----------------------------------------------
#define VTKUNICASTCMD_ANALYZE_ASCALLSOURCE		0
#define VTKUNICASTCMD_ANALYZE_ASCALLTARGET		1

#define CMD_SOURCE_UNICAST			0
#define CMD_SOURCE_MULTICAST			1
#define CMD_SOURCE_SIP					2

#define VTKU_CALL_STATE_TALK					0
#define VTKU_CALL_STATE_BYE					1
//联网呼叫类型定义

#define DsAndGl					0x30		//单元主机(小区主机)与中心	
#define DmAndSt					0x31		//小区主机与分机
#define GlAndSt					0x32		//中心与分机
#define StAndSt					0x33		//分机与分机
#define GlMonMr					0x34		//中心监视主机
#define GlAndGl					0x35		//中心与中心
#define GLMonMr_new			0x38		// 新增Mon_type,区别

#define VTK_CMD_LINK_1001			0x1001
#define VTK_CMD_UNLINK_1002		0x1002
#define VTK_CMD_MON_1004			0x1004
#define VTK_CMD_DIAL_1003			0x1003
#define VTK_CMD_STATE_1005		0x1005
#define VTK_CMD_NT_1006			0x1006
#define VTK_CMD_EXIT_1008			0x1008
#define VTK_CMD_UNLOCK_E003		0xE003
      
#define VTK_CMD_LINK_REP_1081		0x1081
#define VTK_CMD_UNLINK_REP_1082	0x1082
#define VTK_CMD_MON_REP_1084		0x1084
#define VTK_CMD_DIAL_REP_1083		0x1083
#define VTK_CMD_STATE_REP_1085	0x1085
#define VTK_CMD_NT_REP_1086		0x1086
#define VTK_CMD_EXIT_REP_1088		0x1088

typedef struct
{
	uint8 	state;
	int		save_sn;
}VTKUNICASTCMD_ANALYZE_RUN_STRU;

#pragma pack(1)
//czn_20160516
typedef struct
{
	uint8 call_type;
	union
	{
		uint8 rspstate;
		struct
		{
			uint8 source_idh;
			uint8 source_idl;
			uint8 call_code;
			uint8 target_idh;
			uint8 target_idl;
		};
	};
}VtkUnicastCmd_Stru;


typedef struct
{
	uint8 cmd_type;
	uint8 cmd_sub_type;
	uint8 call_type;
	union
	{
		uint8 rspstate;
		struct
		{
			uint8 source_idh;
			uint8 source_idl;
			uint8 call_code;
			uint8 target_idh;
			uint8 target_idl;
		};
	};
}VtkUnicastCmd_X_Stru;

#pragma pack()
// Define API-------------------------------------------------------------------
int API_VtkUnicastCmd_Analyzer(int source_ip,int cmd, int sn,uint8 *pkt , uint16 len);

//czn_020160422


void Set_VtkUnicastCmdAnalyze_State(uint8 newstate);
	#define Set_VtkUnicastCmdAnalyze_AsCallSource() 		Set_VtkUnicastCmdAnalyze_State(VTKUNICASTCMD_ANALYZE_ASCALLSOURCE)
	#define Set_VtkUnicastCmdAnalyze_AsCallTarget() 		Set_VtkUnicastCmdAnalyze_State(VTKUNICASTCMD_ANALYZE_ASCALLTARGET)


int Send_VtkUnicastCmd_Invite(unsigned char call_type,Global_Addr_Stru *s_addr,Global_Addr_Stru *t_addr);
int Send_VtkUnicastCmd_InviteAck(unsigned char call_type,Global_Addr_Stru *t_addr,unsigned char result);
int Send_VtkUnicastCmd_StateNotice(unsigned char call_type,Global_Addr_Stru *s_addr,Global_Addr_Stru *t_addr,unsigned char new_state);
int Send_VtkUnicastCmd_StateNoticeAck(unsigned char call_type,Global_Addr_Stru *t_addr,unsigned char result);

int API_Get_Partner_Calllink_NewCmd(unsigned char call_type,Global_Addr_Stru *target_addr,_IP_Call_Link_Run_Stru *partner_call_link);
int Send_UnlinkCmd(unsigned char call_type,Global_Addr_Stru *target_addr,_IP_Call_Link_Run_Stru *partner_call_link);
int Send_VtkUnicastCmd_TargetUnlockReq(unsigned char call_type,Global_Addr_Stru *t_addr,unsigned char locknum);


#endif
		
