/**
  ******************************************************************************
  * @file    obj_Caller_State.h
  * @author  czn
  * @version V00.01.00 (basic on vsip)
  * @date    2014.11.06
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2014 V-Tec</center></h2>
  ******************************************************************************
  */ 

#ifndef _obj_Caller_State_H
#define _obj_Caller_State_H


#include "../../../os/RTOS.h"
#include "../../vtk_udp_stack/vtk_udp_stack_class.h"

// Define Object Property-------------------------------------------------------
	//Caller_Run

extern uint8 Caller_UnlockId;
extern uint8 Caller_ErrorCode;
	//Õª»úÄ£Ê½
#define MN_PICKUP	0
#define SJ_PICKUP	1


// Define Object Function - Public---------------------------------------------
uint8 Caller_To_Invite(CALLER_STRUCT *msg);
uint8 Caller_To_Redial(CALLER_STRUCT *msg);
void Caller_To_Ringing(CALLER_STRUCT *msg);
void Caller_To_Ack(CALLER_STRUCT *msg);
void Caller_To_Bye(CALLER_STRUCT *msg);
void Caller_To_Waiting(CALLER_STRUCT *msg);
void Caller_To_Unlock(CALLER_STRUCT *msg);
void Caller_To_Timeout(CALLER_STRUCT *msg);
void Caller_To_Error(CALLER_STRUCT *msg);
void Caller_To_ForceClose(CALLER_STRUCT *msg);
// Define Object Function - Private---------------------------------------------


// Define Object Function - Other-----------------------------------------------


#endif

