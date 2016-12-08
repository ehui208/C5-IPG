/**
  ******************************************************************************
  * @file    obj_BeCalled_Data.h
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

#ifndef _obj_BeCalled_Data_H
#define _obj_BeCalled_Data_H

#include "../../../os/RTOS.h"
//#include "BSP.h"


// Define Object Property-------------------------------------------------------
typedef struct BECALLED_CONFIG_STRU	//bdu_zxj
{	
  	uint16	limit_ringing_time;	//呼叫等待定时
	uint16	limit_ack_time;		//呼叫通话定时
	uint8	limit_bye_time;
	uint16	limit_transfer_time;
	uint16	limit_autoredial_time;
}  BECALLED_CONFIG;

extern BECALLED_CONFIG BeCalled_Config;

typedef struct
{
	unsigned char call_type;
	void 		(*callback)(BECALLED_STRUCT *msg);
} STRUCT_IPBECALLED_CALLTYPE_CALLBACK;

extern const STRUCT_IPBECALLED_CALLTYPE_CALLBACK TABLE_CALLTYPE_BECALLED_TOACK_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_BECALLED_TOACK_CALLBACK_NUMBERS;

extern const STRUCT_IPBECALLED_CALLTYPE_CALLBACK TABLE_CALLTYPE_BECALLED_TOBYE_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_BECALLED_TOBYE_CALLBACK_NUMBERS;

extern const STRUCT_IPBECALLED_CALLTYPE_CALLBACK TABLE_CALLTYPE_BECALLED_TOREDIAL_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_BECALLED_TOREDIAL_CALLBACK_NUMBERS;

extern const STRUCT_IPBECALLED_CALLTYPE_CALLBACK TABLE_CALLTYPE_BECALLED_TORINGING_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_BECALLED_TORINGING_CALLBACK_NUMBERS;

extern const STRUCT_IPBECALLED_CALLTYPE_CALLBACK TABLE_CALLTYPE_BECALLED_TOTRANSFER_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_BECALLED_TOTRANSFER_CALLBACK_NUMBERS;

extern const STRUCT_IPBECALLED_CALLTYPE_CALLBACK TABLE_CALLTYPE_BECALLED_TOUNLOCK_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_BECALLED_TOUNLOCK_CALLBACK_NUMBERS;

extern const STRUCT_IPBECALLED_CALLTYPE_CALLBACK TABLE_CALLTYPE_BECALLED_TOTIMEOUT_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_BECALLED_TOTIMEOUT_CALLBACK_NUMBERS;

extern const STRUCT_IPBECALLED_CALLTYPE_CALLBACK TABLE_CALLTYPE_BECALLED_TOWAITING_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_BECALLED_TOWAITING_CALLBACK_NUMBERS;

extern const STRUCT_IPBECALLED_CALLTYPE_CALLBACK TABLE_CALLTYPE_BECALLED_TOERROR_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_BECALLED_TOERROR_CALLBACK_NUMBERS;

extern const STRUCT_IPBECALLED_CALLTYPE_CALLBACK TABLE_CALLTYPE_BECALLED_TOFORCECLOSE_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_BECALLED_TOFORCECLOSE_CALLBACK_NUMBERS;

// Define Object Function - Public---------------------------------------------
uint8 Get_BeCalled_ByType_CallbackID_Common(const STRUCT_IPBECALLED_CALLTYPE_CALLBACK *pcallback,
										  const uint8 callback_length,
										  uint8 call_type);

#define Get_BeCalled_ToRedial_CallbackID(call_type)	\
	Get_BeCalled_ByType_CallbackID_Common(TABLE_CALLTYPE_BECALLED_TOREDIAL_CALLBACK,\
										TABLE_CALLTYPE_BECALLED_TOREDIAL_CALLBACK_NUMBERS,\
										call_type)

#define Get_BeCalled_ToAck_CallbackID(call_type)	\
	Get_BeCalled_ByType_CallbackID_Common(TABLE_CALLTYPE_BECALLED_TOACK_CALLBACK,\
										TABLE_CALLTYPE_BECALLED_TOACK_CALLBACK_NUMBERS,\
										call_type)

#define Get_BeCalled_ToTimeout_CallbackID(call_type)	\
	Get_BeCalled_ByType_CallbackID_Common(TABLE_CALLTYPE_BECALLED_TOTIMEOUT_CALLBACK,\
										TABLE_CALLTYPE_BECALLED_TOTIMEOUT_CALLBACK_NUMBERS,\
										call_type)
		
#define Get_BeCalled_ToBye_CallbackID(call_type)	\
	Get_BeCalled_ByType_CallbackID_Common(TABLE_CALLTYPE_BECALLED_TOBYE_CALLBACK,\
										TABLE_CALLTYPE_BECALLED_TOBYE_CALLBACK_NUMBERS,\
										call_type)

#define Get_BeCalled_ToRinging_CallbackID(call_type)	\
	Get_BeCalled_ByType_CallbackID_Common(TABLE_CALLTYPE_BECALLED_TORINGING_CALLBACK,\
										TABLE_CALLTYPE_BECALLED_TORINGING_CALLBACK_NUMBERS,\
										call_type)

#define Get_BeCalled_ToTransfer_CallbackID(call_type)	\
	Get_BeCalled_ByType_CallbackID_Common(TABLE_CALLTYPE_BECALLED_TOTRANSFER_CALLBACK,\
										TABLE_CALLTYPE_BECALLED_TOTRANSFER_CALLBACK_NUMBERS,\
										call_type)

#define Get_BeCalled_ToUnlock_CallbackID(call_type)	\
	Get_BeCalled_ByType_CallbackID_Common(TABLE_CALLTYPE_BECALLED_TOUNLOCK_CALLBACK,\
										TABLE_CALLTYPE_BECALLED_TOUNLOCK_CALLBACK_NUMBERS,\
										call_type)

#define Get_BeCalled_ToWaiting_CallbackID(call_type)	\
	Get_BeCalled_ByType_CallbackID_Common(TABLE_CALLTYPE_BECALLED_TOWAITING_CALLBACK,\
										TABLE_CALLTYPE_BECALLED_TOWAITING_CALLBACK_NUMBERS,\
										call_type)

#define Get_BeCalled_ToError_CallbackID(call_type)	\
	Get_BeCalled_ByType_CallbackID_Common(TABLE_CALLTYPE_BECALLED_TOERROR_CALLBACK,\
										TABLE_CALLTYPE_BECALLED_TOERROR_CALLBACK_NUMBERS,\
										call_type)

#define Get_BeCalled_ToForceClose_CallbackID(call_type)	\
	Get_BeCalled_ByType_CallbackID_Common(TABLE_CALLTYPE_BECALLED_TOFORCECLOSE_CALLBACK,\
										TABLE_CALLTYPE_BECALLED_TOFORCECLOSE_CALLBACK_NUMBERS,\
										call_type)	

void BeCalled_Data_Init(void);


// Define Object Function - Private---------------------------------------------


// Define Object Function - Other-----------------------------------------------


#endif
