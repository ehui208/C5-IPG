/**
  ******************************************************************************
  * @file    obj_BeCalled_State.h
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

#ifndef _obj_BeCalled_State_H
#define _obj_BeCalled_State_H

#include "../../../os/RTOS.h"
//#include "BSP.h"

#include "task_IpBeCalled.h"
#include "../../vtk_udp_stack/vtk_udp_stack_class.h"

// Define Object Property-------------------------------------------------------
	//Callback





	//BeCalled_Run

extern uint8 BeCalled_UnlockId;
extern uint8 BeCalled_ErrorCode;
	//Õª»úÄ£Ê½
#define BECALLED_MN_PICKUP	0
#define BECALLED_SJ_PICKUP	1


// Define Object Function - Public---------------------------------------------
uint8 BeCalled_To_Redial(BECALLED_STRUCT *msg);
void BeCalled_To_Ringing(BECALLED_STRUCT *msg);
void BeCalled_To_Transfer(BECALLED_STRUCT *msg);
void BeCalled_To_Ack(BECALLED_STRUCT *msg);
void BeCalled_To_Bye(BECALLED_STRUCT *msg);
void BeCalled_To_Waiting(BECALLED_STRUCT *msg);
void BeCalled_To_Unlock(BECALLED_STRUCT *msg);
void BeCalled_To_Timeout(BECALLED_STRUCT *msg);
void BeCalled_To_Error(BECALLED_STRUCT *msg);
void BeCalled_To_ForceClose(BECALLED_STRUCT *msg);
// Define Object Function - Private---------------------------------------------


// Define Object Function - Other-----------------------------------------------



#endif
