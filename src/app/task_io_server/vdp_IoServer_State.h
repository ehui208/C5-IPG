/**
  ******************************************************************************
  * @file    obj_IoServer_State.h
  * @author  zxj
  * @version V00.01.00
  * @date    2012.11.01
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2012 V-Tec</center></h2>
  ******************************************************************************
  */ 

#ifndef _obj_IoServer_State_H
#define _obj_IoServer_State_H

#include "task_IoServer.h"
#include "obj_TableProcess.h"

// Define Object Property-------------------------------------------------------
typedef struct
{
	OS_TIMER		saveTimer;
	uint8 			mustSaveFlag;
	one_vtk_table*	pTable;
} IO_STATE_S;



// Define Object Function - Public----------------------------------------------
void IoServerStateInit(void);
int InnerRead(IoServer_STRUCT *Msg_IoServer);
int InnerWrite(IoServer_STRUCT *Msg_IoServer);

// Define Object Function - Private---------------------------------------------
void SaveIoDataTimerCallback(void);

int load_io_data_table(void);
int SaveIoDataTable(void);
int ReadWritelocalResponse( unsigned char msg_id, unsigned char msg_type, IoServer_STRUCT* pIoDat );

// Define Object Function - Other-----------------------------------------------


#endif
