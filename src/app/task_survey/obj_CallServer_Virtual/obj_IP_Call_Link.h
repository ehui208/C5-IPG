#ifndef _OBJ_IP_CALL_LINK_H
#define _OBJ_IP_CALL_LINK_H



typedef enum
{
    CLink_Idle = 0,
    CLink_AsCallServer,
	CLink_AsBeCalled,
	CLink_AsCaller,
	CLink_Transferred,
	CLink_HaveLocalCall,
} Ip_Call_Link_Staus_e;

#define Device_Offline_BitMask		0x80
#define Judge_TargetDevice_Online(rt,code)		(Device_SyncLinking(rt*32+code))		//czn_20160629

#pragma pack(1)
//czn_020160422
typedef struct _Global_Addr_Stru
{
	unsigned short gatewayid;
	//union
	//{
		//unsigned char ip_addr[4];
	int 			ip;		
	//};
	unsigned char rt;
	unsigned char code;
}Global_Addr_Stru;

typedef struct _BeCalled_Data_Stru
{
	Global_Addr_Stru Call_Source;
}BeCalled_Data_Stru;


typedef struct _Caller_Data_Stru
{
	Global_Addr_Stru Call_Target;
}Caller_Data_Stru;

typedef struct _Transferred_Data_Stru
{
	Global_Addr_Stru Transferred_Device;
}Transferred_Data_Stru;

typedef struct _IP_Call_Link_Run_Stru
{
	unsigned char 			Status;
	BeCalled_Data_Stru 		BeCalled_Data;
	Caller_Data_Stru 		Caller_Data;
	Transferred_Data_Stru	Transferred_Data;
}_IP_Call_Link_Run_Stru;

#pragma pack()



//设置Calllink为空闲
void Set_IPCallLink_Idle(void);
//设置Calllink为CallServer
void Set_IPCallLink_AsCallServer(BeCalled_Data_Stru *becalled_data,Caller_Data_Stru *caller_data);
//设置Calllink为Becalled
void Set_IPCallLink_AsBeCalled(BeCalled_Data_Stru *becalled_data);
//设置Calllink为Caller
void Set_IPCallLink_AsCaller(Caller_Data_Stru *caller_data);
//设置Calllink为transferred
void Set_IPCallLink_ToTransferred(Transferred_Data_Stru *transferred_data);
//设置Calllink为HaveLocalCall
void Set_IPCallLink_ToHaveLocalCall(BeCalled_Data_Stru *becalled_data,Caller_Data_Stru *caller_data);
//czn_020160422_s
//int Get_Call_Link_Respones(UDP_MSG_TYPE *pUdpType);
void Set_IPCallLink_Status(unsigned char newstatus);
unsigned char Get_IPCallLink_Status(void);
void Set_IPCallLink_Data(unsigned char data_type,Global_Addr_Stru *data);
	#define Set_IPCallLink_CallerData(CallerData)				Set_IPCallLink_Data(0,CallerData)
	#define Set_IPCallLink_BeCalledData(BeCalledData)			Set_IPCallLink_Data(1,BeCalledData)
	#define Set_IPCallLink_TransferredData(TransferredData)	Set_IPCallLink_Data(2,TransferredData)
//czn_020160422_e
#endif
