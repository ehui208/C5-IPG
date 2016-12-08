/**
  ******************************************************************************
  * @file    task_IoServer.h
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

#ifndef VDP_IO_SERVER_H
#define VDP_IO_SERVER_H

#include "../task_survey/task_survey.h"

// Define task interface-------------------------------------------------------------
#define IO_DATA_LENGTH				28

#define _OK							1
#define _ERROR						2

#pragma pack(1)

typedef struct 
{
	uint16 	property_id;
	int 	result;
	uint16 	len;
	uint8	ptr_data[IO_DATA_LENGTH];
} IoServer_STRUCT ;

typedef struct 
{
	VDP_MSG_HEAD	head;
	IoServer_STRUCT	msg_dat;
} io_server_type;

#pragma pack()

#define IO_SERVER_READ_LOCAL					1
#define IO_SERVER_WRITE_LOCAL					2
#define IO_SERVER_SAVE_DATA_TO_FILE				3

void vtk_TaskInit_io_server(void);
int API_io_server_read_local(unsigned char sourceTaskId, uint16 property_id, uint8 *ptr_data );
int API_io_server_write_local(unsigned char sourceTaskId, uint16 property_id, uint8 *ptr_data );
int API_io_server_save_data_file(void);
	
#define API_io_server_InnerRead( sourceId, property_id, ptr_data)	API_io_server_read_local(sourceId, property_id, ptr_data)
#define API_io_server_InnerWrite( sourceId, property_id, ptr_data)	API_io_server_write_local(sourceId, property_id, ptr_data)

#endif

