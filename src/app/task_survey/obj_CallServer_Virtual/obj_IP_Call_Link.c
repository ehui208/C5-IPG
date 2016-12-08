#include <stdio.h>
#include "obj_CallServer_Virtual.h"
#include "obj_IP_Call_Link.h"

_IP_Call_Link_Run_Stru IP_Call_Link_Run = {.Status = CLink_Idle};


//czn_020160422_s
void Set_IPCallLink_Status(unsigned char newstatus)
{
	IP_Call_Link_Run.Status = newstatus ;
}

unsigned char Get_IPCallLink_Status(void)
{
	return IP_Call_Link_Run.Status;
}

void Set_IPCallLink_Data(unsigned char data_type,Global_Addr_Stru *data)
{
	if(data_type == 0)
	{
		IP_Call_Link_Run.Caller_Data.Call_Target= *data;
	}
	else if(data_type == 1)
	{
		IP_Call_Link_Run.BeCalled_Data.Call_Source = *data;
	}
	if(data_type == 2)
	{
		IP_Call_Link_Run.Transferred_Data.Transferred_Device = *data;
	}
}
//czn_020160422_e

//czn_20160516
int Get_Call_Link_Respones(UDP_MSG_TYPE *pUdpType)
{
	_IP_Call_Link_Run_Stru *rspbuf = (_IP_Call_Link_Run_Stru*)malloc(sizeof(_IP_Call_Link_Run_Stru) );
	VtkUnicastCmd_Stru *pcmd = (VtkUnicastCmd_Stru *)pUdpType->pbuf;
	
	if(rspbuf == NULL)
	{
		return -1;
	}
	//rspbuf[0]  = VTK_CMD_LINK_REP_1081 >> 8;
	//rspbuf[1]  = VTK_CMD_LINK_REP_1081 & 0xff; 
	//rspbuf[2]  = pUdpType->pbuf[2];
	//rspbuf[3]  = pUdpType->pbuf[3];
	
	memcpy(rspbuf,&IP_Call_Link_Run,sizeof(_IP_Call_Link_Run_Stru));
	
	if(pcmd->target_idh != 0 || pcmd->target_idl != 0)
	{
		rspbuf->Status |= ((Judge_TargetDevice_Online(pcmd->target_idh,pcmd->target_idl) == 0) ? 0 : Device_Offline_BitMask);
	}
	

	//int id = (rspbuf[2]<<8) | rspbuf[3];
	
	if(api_udp_c5_ipc_send_rsp(pUdpType->target_ip,VTK_CMD_LINK_REP_1081,pUdpType->id,(char*)rspbuf,sizeof(_IP_Call_Link_Run_Stru)) == -1)
	{
		free(rspbuf);
		return -1;
	}

	free(rspbuf);
	return 0;
}
