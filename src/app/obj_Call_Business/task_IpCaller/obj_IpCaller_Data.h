/**
  ******************************************************************************
  * @file    obj_Caller_Data.h
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

#ifndef _obj_Caller_Data_H
#define _obj_Caller_Data_H


#include "../../../os/RTOS.h"


// Define Object Property-------------------------------------------------------
typedef struct CALLER_CONFIG_STRU	//bdu_zxj
{	
	uint8	limit_invite_time;
	uint16	limit_ringing_time;
	uint16	limit_ack_time;
	uint8	limit_bye_time;
}	CALLER_CONFIG;
extern CALLER_CONFIG	Caller_Config;

typedef struct
{
	uint8 	call_type;
	void 	(*callback)(CALLER_STRUCT *msg);
} STRUCT_IPCALLER_CALLTYPE_CALLBACK;

extern const STRUCT_IPCALLER_CALLTYPE_CALLBACK TABLE_CALLTYPE_CALLER_TOREDIAL_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_CALLER_TOREDIAL_CALLBACK_NUMBERS;

extern const STRUCT_IPCALLER_CALLTYPE_CALLBACK TABLE_CALLTYPE_CALLER_TOINVITE_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_CALLER_TOINVITE_CALLBACK_NUMBERS;

extern const STRUCT_IPCALLER_CALLTYPE_CALLBACK TABLE_CALLTYPE_CALLER_TORINGING_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_CALLER_TORINGING_CALLBACK_NUMBERS;

extern const STRUCT_IPCALLER_CALLTYPE_CALLBACK TABLE_CALLTYPE_CALLER_TOACK_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_CALLER_TOACK_CALLBACK_NUMBERS;

extern const STRUCT_IPCALLER_CALLTYPE_CALLBACK TABLE_CALLTYPE_CALLER_TOBYE_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_CALLER_TOBYE_CALLBACK_NUMBERS;

extern const STRUCT_IPCALLER_CALLTYPE_CALLBACK TABLE_CALLTYPE_CALLER_TOWAITING_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_CALLER_TOWAITING_CALLBACK_NUMBERS;

extern const STRUCT_IPCALLER_CALLTYPE_CALLBACK TABLE_CALLTYPE_CALLER_TOUNLOCK_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_CALLER_TOUNLOCK_CALLBACK_NUMBERS;

extern const STRUCT_IPCALLER_CALLTYPE_CALLBACK TABLE_CALLTYPE_CALLER_TOTIMEOUT_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_CALLER_TOTIMEOUT_CALLBACK_NUMBERS;

extern const STRUCT_IPCALLER_CALLTYPE_CALLBACK TABLE_CALLTYPE_CALLER_TOERROR_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_CALLER_TOERROR_CALLBACK_NUMBERS;

extern const STRUCT_IPCALLER_CALLTYPE_CALLBACK TABLE_CALLTYPE_CALLER_TOFORCECLOSE_CALLBACK[];
extern const uint8 TABLE_CALLTYPE_CALLER_TOFORCECLOSE_CALLBACK_NUMBERS;
// Define Object Function - Public---------------------------------------------
void Caller_Data_Init(void);
uint8 Get_Caller_ByType_CallbackID_Common(const STRUCT_IPCALLER_CALLTYPE_CALLBACK *pcallback,
										  const uint8 callback_length,
										  uint8 call_type);

#define Get_Caller_ToInvite_CallbackID(call_type)	\
	Get_Caller_ByType_CallbackID_Common(TABLE_CALLTYPE_CALLER_TOINVITE_CALLBACK,\
										TABLE_CALLTYPE_CALLER_TOINVITE_CALLBACK_NUMBERS,\
										call_type)

#define Get_Caller_ToRedial_CallbackID(call_type)	\
	Get_Caller_ByType_CallbackID_Common(TABLE_CALLTYPE_CALLER_TOREDIAL_CALLBACK,\
										TABLE_CALLTYPE_CALLER_TOREDIAL_CALLBACK_NUMBERS,\
										call_type)

#define Get_Caller_ToRinging_CallbackID(call_type)	\
	Get_Caller_ByType_CallbackID_Common(TABLE_CALLTYPE_CALLER_TORINGING_CALLBACK,\
										TABLE_CALLTYPE_CALLER_TORINGING_CALLBACK_NUMBERS,\
										call_type)

#define Get_Caller_ToAck_CallbackID(call_type)	\
	Get_Caller_ByType_CallbackID_Common(TABLE_CALLTYPE_CALLER_TOACK_CALLBACK,\
										TABLE_CALLTYPE_CALLER_TOACK_CALLBACK_NUMBERS,\
										call_type)


#define Get_Caller_ToBye_CallbackID(call_type)	\
	Get_Caller_ByType_CallbackID_Common(TABLE_CALLTYPE_CALLER_TOBYE_CALLBACK,\
										TABLE_CALLTYPE_CALLER_TOBYE_CALLBACK_NUMBERS,\
										call_type)

#define Get_Caller_ToTimeout_CallbackID(call_type)	\
	Get_Caller_ByType_CallbackID_Common(TABLE_CALLTYPE_CALLER_TOTIMEOUT_CALLBACK,\
										TABLE_CALLTYPE_CALLER_TOTIMEOUT_CALLBACK_NUMBERS,\
										call_type)		

#define Get_Caller_ToWaiting_CallbackID(call_type)	\
	Get_Caller_ByType_CallbackID_Common(TABLE_CALLTYPE_CALLER_TOWAITING_CALLBACK,\
										TABLE_CALLTYPE_CALLER_TOWAITING_CALLBACK_NUMBERS,\
										call_type)

#define Get_Caller_ToUnlock_CallbackID(call_type)	\
	Get_Caller_ByType_CallbackID_Common(TABLE_CALLTYPE_CALLER_TOUNLOCK_CALLBACK,\
										TABLE_CALLTYPE_CALLER_TOUNLOCK_CALLBACK_NUMBERS,\
										call_type)

#define Get_Caller_ToError_CallbackID(call_type)	\
	Get_Caller_ByType_CallbackID_Common(TABLE_CALLTYPE_CALLER_TOERROR_CALLBACK,\
										TABLE_CALLTYPE_CALLER_TOERROR_CALLBACK_NUMBERS,\
										call_type)

#define Get_Caller_ToForceClose_CallbackID(call_type)	\
	Get_Caller_ByType_CallbackID_Common(TABLE_CALLTYPE_CALLER_TOFORCECLOSE_CALLBACK,\
										TABLE_CALLTYPE_CALLER_TOFORCECLOSE_CALLBACK_NUMBERS,\
										call_type)
// Define Object Function - Private---------------------------------------------


// Define Object Function - Other-----------------------------------------------


#endif
