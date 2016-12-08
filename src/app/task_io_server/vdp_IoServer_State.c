/**
  ******************************************************************************
  * @file    obj_IoServer_State.c
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

#include "task_IoServer.h"	
#include "vdp_IoServer_State.h"
#include "vdp_IoServer_Data.h"
#include "../vtk_udp_stack/vtk_udp_stack_io_server.h"


#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<errno.h>

#define BUFF_LEN							100
#define ADDRESS								"addr"
#define CURRENT_VALUE						"value"
#define IO_DATA_DEFAULT_FILE_NAME			"/mnt/nand1-2/io_data_default_table.csv"
#define IO_DATA_FILE_NAME					"/mnt/nand1-2/io_data_table.csv"
#define IO_DATA_BAK_FILE_NAME				"/mnt/nand1-2/io_data_bak_table.csv"

IO_STATE_S	ioProperty;

//IO SERVER 参数表结构定义
const unsigned char* IO_DATA_KEYNAME_TAB[] = 
{
	ADDRESS,
	CURRENT_VALUE,
};
const int IO_DATA_KEYNAME_CNT = sizeof(IO_DATA_KEYNAME_TAB)/sizeof(unsigned char*);

//IO SERVER 状态初始化
void IoServerStateInit(void)
{
	ioProperty.mustSaveFlag 	= 0;
	ioProperty.pTable			= NULL;
	OS_CreateTimer(&ioProperty.saveTimer, SaveIoDataTimerCallback, 5000/25);
}

//定时存在参数值到文件中
void SaveIoDataTimerCallback(void)
{
	if(ioProperty.mustSaveFlag)
	{
		API_io_server_save_data_file();
		OS_RetriggerTimer(&ioProperty.saveTimer);
	}
	else
	{
		OS_StopTimer(&ioProperty.saveTimer);
	}
}

//交换两条记录值
void ExchangeRecord(one_vtk_dat* record1, one_vtk_dat* record2)
{
	one_vtk_dat oneRecord;

	oneRecord.len		= record1->len;
	oneRecord.pdat	= record1->pdat;
	
	record1->len 	= record2->len;
	record1->pdat 	= record2->pdat;
	
	record2->len 	= oneRecord.len;
	record2->pdat 	= oneRecord.pdat;
}

//加载参数值到内存中
int load_io_data_table(void)
{
	unsigned int i;
	unsigned int tempValue = 0;
	unsigned int minValue = 0;
	unsigned int maxValue = 0;
	char writeBuff[BUFF_LEN];
	char* post1;
	char* post2;
	
	ioProperty.pTable = load_vtk_table_file(IO_DATA_FILE_NAME);
	///*
	//加载失败
	if(ioProperty.pTable == NULL)
	{
		snprintf( writeBuff, BUFF_LEN, "cp %s %s", IO_DATA_DEFAULT_FILE_NAME, IO_DATA_FILE_NAME);
		system(writeBuff);
		sync(); 	//czn_20161012		
		ioProperty.pTable = load_vtk_table_file(IO_DATA_FILE_NAME);
		if(ioProperty.pTable == NULL)
		{
			return -1;
		}
	}
	else
	{
		one_vtk_table*	pDefaultTable;
		
		pDefaultTable = load_vtk_table_file(IO_DATA_DEFAULT_FILE_NAME);
		if(pDefaultTable == NULL)
		{
			return -2;
		}
		
		for( i = 0; i < ioProperty.pTable->record_cnt; i++ )
		{
			if(EEPROM_ADDR[i].address == BaseAddress)
			{
				ExchangeRecord(&ioProperty.pTable->precord[i], &pDefaultTable->precord[i]);
				ioProperty.mustSaveFlag = 1;
			}
			else
			{
				if(EEPROM_ADDR[i].dpt > 4)
				{	
					continue;
				}
				else
				{
					memset(writeBuff, 0, BUFF_LEN);
					memcpy(writeBuff, ioProperty.pTable->precord[i].pdat, ioProperty.pTable->precord[i].len);
					post1 = strchr(writeBuff,',');
					if(post1 != NULL)
					{
						post1++;
						post2 = strstr(post1,"0x");
						tempValue = 0;
						minValue = 0;
						maxValue = 0;
						if(post2 != NULL)
						{
							sscanf(post1, "%x", &tempValue);
						}
						else 
						{
							tempValue = atoi(post1);
						}
						
						//取出最大最小值判断值是否在范围内
						memcpy(&minValue, ptrRomTab[i], EEPROM_ADDR[i].dpt);
						memcpy(&maxValue, ptrRomTab[i]+EEPROM_ADDR[i].dpt, EEPROM_ADDR[i].dpt);

						//不在范围内，取默认值
						if(tempValue < minValue || tempValue > maxValue)
						{
							ExchangeRecord(&ioProperty.pTable->precord[i], &pDefaultTable->precord[i]);
							ioProperty.mustSaveFlag = 1;
						}
					}
					else
					{
						ExchangeRecord(&ioProperty.pTable->precord[i], &pDefaultTable->precord[i]);
						ioProperty.mustSaveFlag = 1;
					}
				}
			}
		}

		free_vtk_table_file_buffer(pDefaultTable);
		SaveIoDataTable();
	}
	//*/

	//printf_one_table(ioProperty.pTable);
	
	return 0;
}

//读取本地参数值
int InnerRead(IoServer_STRUCT *Msg_IoServer)
{
	char tempBuff[BUFF_LEN] = {0};
	char value[BUFF_LEN] = {0};
	unsigned int valueLen = BUFF_LEN;
	unsigned int tempValue = 0;
	unsigned int minValue = 0;
	unsigned int maxValue = 0;
	char* tempPos;
	
	if(EEPROM_ADDR[Msg_IoServer->property_id].address == BaseAddress)
	{
		if(EEPROM_ADDR[Msg_IoServer->property_id].dpt > 4)
		{	
			Msg_IoServer->len = *ptrRomTab[Msg_IoServer->property_id];
			strcpy(Msg_IoServer->ptr_data, ptrRomTab[Msg_IoServer->property_id]+1);
		}
		else
		{
			Msg_IoServer->len = EEPROM_ADDR[Msg_IoServer->property_id].dpt;
			memcpy(&tempValue, ptrRomTab[Msg_IoServer->property_id]+Msg_IoServer->len*2, Msg_IoServer->len);
			memcpy(Msg_IoServer->ptr_data, &tempValue, Msg_IoServer->len);
		}
	}
	else
	{
		one_vtk_dat* oneRecord;
		snprintf( tempBuff, BUFF_LEN, "0x%08x", EEPROM_ADDR[Msg_IoServer->property_id].address);
		oneRecord = get_one_vtk_record(ioProperty.pTable, ADDRESS, tempBuff,0);
		if(oneRecord == NULL)
		{
			Msg_IoServer->result = -1;
			return -1;
		}
		get_one_record_string(oneRecord,1, value, &valueLen);
		if(EEPROM_ADDR[Msg_IoServer->property_id].dpt>4)
		{
			Msg_IoServer->len = valueLen;
			memcpy(Msg_IoServer->ptr_data, value, Msg_IoServer->len);
		}
		else
		{
			//格式----------|数值|
			tempPos = strstr(value,"0x");
			if(tempPos != NULL)
			{
				sscanf(tempPos, "%x", &tempValue);
			}
			else 
			{
				tempValue = atoi(value);
			}
			
			//取出最大最小值判断值是否在范围内
			memcpy(&minValue, ptrRomTab[Msg_IoServer->property_id], EEPROM_ADDR[Msg_IoServer->property_id].dpt);
			memcpy(&maxValue, ptrRomTab[Msg_IoServer->property_id]+EEPROM_ADDR[Msg_IoServer->property_id].dpt, EEPROM_ADDR[Msg_IoServer->property_id].dpt);

			//不在范围内，取默认值
			if(tempValue < minValue || tempValue > maxValue)
			{
				Msg_IoServer->result = -2;
				return -2;
			}
			Msg_IoServer->len = EEPROM_ADDR[Msg_IoServer->property_id].dpt;
			memcpy(Msg_IoServer->ptr_data, &tempValue, Msg_IoServer->len);
		}
	}

	Msg_IoServer->result = 0;
	return 0;
}

//写本地参数值
int InnerWrite(IoServer_STRUCT *Msg_IoServer)
{
	char tempBuff[BUFF_LEN] = {0};
	char value[BUFF_LEN] = {0};
	unsigned int tempValue = 0;
	unsigned int minValue = 0;
	unsigned int maxValue = 0;
			
	if(EEPROM_ADDR[Msg_IoServer->property_id].address == BaseAddress)
	{
		Msg_IoServer->result = -1;
		return -1;
	}
	else
	{
		one_vtk_dat* oneRecord;
		snprintf( tempBuff, BUFF_LEN, "0x%08x", EEPROM_ADDR[Msg_IoServer->property_id].address);
		oneRecord = get_one_vtk_record(ioProperty.pTable, ADDRESS, tempBuff,0);
		if(oneRecord == NULL)
		{
			Msg_IoServer->result = -2;
			return -2;
		}

		//格式----------|addr,value|
		snprintf( value, BUFF_LEN, "0x%08x", EEPROM_ADDR[Msg_IoServer->property_id].address);
		strcat(value,",");
		
		if(EEPROM_ADDR[Msg_IoServer->property_id].dpt>4)
		{
			memcpy(value+strlen(value), Msg_IoServer->ptr_data, Msg_IoServer->len);
		}
		else
		{
			//需要存储的值
			memcpy(&tempValue, Msg_IoServer->ptr_data, EEPROM_ADDR[Msg_IoServer->property_id].dpt);
			
			//取出最大最小值判断值是否在范围内
			memcpy(&minValue, ptrRomTab[Msg_IoServer->property_id], EEPROM_ADDR[Msg_IoServer->property_id].dpt);
			memcpy(&maxValue, ptrRomTab[Msg_IoServer->property_id]+EEPROM_ADDR[Msg_IoServer->property_id].dpt, EEPROM_ADDR[Msg_IoServer->property_id].dpt);

			//不在范围内
			if(tempValue < minValue || tempValue > maxValue)
			{
				Msg_IoServer->result = -3;
				return -3;
			}
			snprintf(value+strlen(value), BUFF_LEN-strlen(value), "0x%.*x", EEPROM_ADDR[Msg_IoServer->property_id].dpt*2, tempValue);
		}
		
		oneRecord->len = strlen(value);
		oneRecord->pdat = realloc(oneRecord->pdat, oneRecord->len);
		memcpy(oneRecord->pdat, value, oneRecord->len);

		Msg_IoServer->result = 0;
		ioProperty.mustSaveFlag = 1;
		OS_RetriggerTimer(&ioProperty.saveTimer);
		
		return 0;
	}
}

//保存参数值到本地文件处理
int SaveIoDataTable(void)
{
	if(ioProperty.mustSaveFlag)
	{
		if(rename(IO_DATA_FILE_NAME,IO_DATA_BAK_FILE_NAME))
		{
			return -1;
		}
		
		if(save_vtk_table_file_buffer( ioProperty.pTable, 0, ioProperty.pTable->record_cnt, IO_DATA_FILE_NAME))
		{
			return -2;
		}
		dprintf("save io server data sucess --------------------\n");
	}
	
	ioProperty.mustSaveFlag = 0;
	OS_StopTimer(&ioProperty.saveTimer);
	
	return 0;
}

//读写本地参数回应
int ReadWritelocalResponse( unsigned char msg_id, unsigned char msg_type, IoServer_STRUCT* pIoDat )
{
	io_server_type  data;
	int len;

	vdp_task_t* pTask = GetTaskAccordingMsgID(msg_id);
	
	data.head.msg_target_id 	= msg_id;
	data.head.msg_source_id	= MSG_ID_IOServer;			
	data.head.msg_type		= (msg_type|COMMON_RESPONSE_BIT);
	data.head.msg_sub_type	= 0;
	
	data.msg_dat.property_id	= pIoDat->property_id;
	data.msg_dat.result		= pIoDat->result;
	data.msg_dat.len			= pIoDat->len;

	memcpy( data.msg_dat.ptr_data, pIoDat->ptr_data, data.msg_dat.len);

	//printf("---data.msg_dat---,len=%d,d0=%d,d1=%d\n",data.msg_dat.len,data.msg_dat.ptr_data[0],data.msg_dat.ptr_data[1]);
	
	push_vdp_common_queue(pTask->p_syc_buf, (char*)&data,12+data.msg_dat.len);
	return 0;
}

/*********************************************************************************************************
**  End Of File
*********************************************************************************************************/
