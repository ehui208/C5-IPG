
#include "task_IoServer.h"
#include "../task_debug_sbu/task_debug_sbu.h"

#include "obj_TableProcess.h"
#include "obj_ResourceTable.h"

#define RESOURCE_TABLE_NAME		"/mnt/nand1-2/ResourceTable.csv"

ResourceTb_Run_Stru ResourceTb_Run;

one_vtk_table * presource_table 		= NULL;


const unsigned char* RESOURCETB_KEYNAME_TAB[] = 
{
	RESOURCE_ID,
	RESOURCE_FILE,
	RESOURCE_GETTIME,
	RESOURCE_ERRCODE,
	RESOURCE_LASTUPDATE,
	RESOURCE_UPDATERESULT,
};
const int RESOURCETB_KEYNAME_CNT = sizeof(RESOURCETB_KEYNAME_TAB)/sizeof(unsigned char*);

const int RESOURCETB_KEYNAME_LENGTH_TAB[] = 
{
	RESOURCE_ID_LEN,
	RESOURCE_FILE_LEN,
	RESOURCE_GETTIME_LEN,
	RESOURCE_ERRCODE_LEN,
	RESOURCE_LASTUPDATE_LEN,
	RESOURCE_UPDATERESULT_LEN,
};


int Init_ResourceTb_Head(void)
{
	presource_table = malloc(sizeof(one_vtk_table));
	
	if(presource_table == NULL)
	{
		return -1;
	}
	
	pthread_mutex_init( &presource_table->lock, 0);

	int i;

	presource_table->keyname_cnt = RESOURCETB_KEYNAME_CNT;	
	presource_table->pkeyname = malloc(presource_table->keyname_cnt * sizeof(one_vtk_dat));
	for( i = 0; i < presource_table->keyname_cnt; i++ )
	{
		presource_table->pkeyname[i].len = strlen(RESOURCETB_KEYNAME_TAB[i]);
		presource_table->pkeyname[i].pdat = malloc(presource_table->pkeyname[i].len);
		memcpy(presource_table->pkeyname[i].pdat, RESOURCETB_KEYNAME_TAB[i], presource_table->pkeyname[i].len);	
	}		

	presource_table->pkeyname_len = malloc(presource_table->keyname_cnt * sizeof(int));

	for( i = 0; i < presource_table->keyname_cnt; i++ )
	{
		presource_table->pkeyname_len[i] = RESOURCETB_KEYNAME_LENGTH_TAB[i];
	}

	presource_table->record_cnt = 0;
	presource_table->precord = NULL;

	//save_vtk_table_file_buffer( presource_table, 0, 0, RESOURCE_TABLE_NAME);

	return 0;
}

// 加载呼叫记录表
int API_Load_ResourceTable(void)
{
		
	// 加载基础数据表
	pthread_mutex_init(&ResourceTb_Run.lock,0);
	ResourceTb_Run.modified_add_cnt = 0;
	ResourceTb_Run.modified_pre_cnt = 0;

	OS_CreateTimer(&ResourceTb_Run.autosave_timer, ResourceTable_AutoSave_Timer_Callback, 60000/20);
	OS_RetriggerTimer(&ResourceTb_Run.autosave_timer);
	
	presource_table = load_vtk_table_file( RESOURCE_TABLE_NAME);
	if( presource_table != NULL  )
	{
		//printf("load table from \"/mnt/nand1-2/call_record_table.csv\".\n");
		API_ResourceTb_Verify();
		if( 1 )
		{
			printf_one_table(presource_table);
		}
	}
	else
	{
		Init_ResourceTb_Head();
	}
	return 0;
}



// 保存一条呼叫记录数据
int API_ResourceTb_AddOneRecord( int rid,int errcode,char *rsname,char *prersname)
{
	one_vtk_dat	*pTempRecord = NULL;
	char			recordBuffer[BUFF_ONE_RECORD_LEN+1] = {0};
	char 		tempBuffer[RESOURCE_FILE_LEN+1];
	int			temp;
	int			i,recordlen;
	char 		*newrecord;
	

	time_t nowtime;

	if(presource_table == NULL)
	{
		return -1;
	}
	
	nowtime = time(NULL); 

	
	
	snprintf(recordBuffer,BUFF_ONE_RECORD_LEN, "%d,%s,%d,%d,0,0",rid,rsname,nowtime,errcode);

	recordlen = strlen(recordBuffer);
	
	pthread_mutex_lock(&ResourceTb_Run.lock);
	
	for(i = 0;i < presource_table->record_cnt; i++)
	{
		temp = 20;
		if(get_one_table_record_string(presource_table,i,RESOURCE_ID,tempBuffer,&temp) != 0)
		{
			pthread_mutex_unlock(&ResourceTb_Run.lock);
			return -1;
		}
		
		tempBuffer[temp] = 0;
		temp = atol(tempBuffer);
		
		if(temp == rid)
		{
			break;	
		}
	}
	
	if(i < presource_table->record_cnt)
	{
		
		if(prersname != NULL)
		{
			temp = RESOURCE_FILE_LEN;
			if(get_one_table_record_string(presource_table,i,RESOURCE_FILE,prersname,&temp) != 0)
			{
				pthread_mutex_unlock(&ResourceTb_Run.lock);
				return -1;
			}
			prersname[temp] = 0;
		}

		newrecord = malloc(recordlen);
		
		if(newrecord == NULL)
		{
			pthread_mutex_unlock(&ResourceTb_Run.lock);
			return -1;
		}
		
		memcpy(newrecord,recordBuffer,recordlen);
		free(presource_table->precord[i].pdat);
		
		presource_table->precord[i].len = recordlen;
		presource_table->precord[i].pdat = newrecord;

		ResourceTb_Run.modified_time = nowtime;
		ResourceTb_Run.modified_pre_cnt ++;
	}
	else
	{
		if(prersname != NULL)
		{
			*prersname = 0;
		}
		
		pTempRecord = malloc((presource_table->record_cnt+1) * sizeof(one_vtk_dat));
		
		if(pTempRecord == NULL)
		{
			pthread_mutex_unlock(&ResourceTb_Run.lock);
			return -1;
		}

		newrecord = malloc(recordlen);
		
		if(newrecord == NULL)
		{
			free(pTempRecord);
			pthread_mutex_unlock(&ResourceTb_Run.lock);
			return -1;
		}
		
		memcpy(newrecord,recordBuffer,recordlen);
		
		for(i = 0; i < presource_table->record_cnt; i++)
		{
			 pTempRecord[i] = presource_table->precord[i];;
		}

		 pTempRecord[i].len = recordlen;
		 pTempRecord[i].pdat = newrecord;
		 
		 if(presource_table->precord != NULL)
		 {
		 	free(presource_table->precord);
		 }

		 presource_table->record_cnt ++;
		 presource_table->precord = pTempRecord;

		 ResourceTb_Run.modified_time = nowtime;
		ResourceTb_Run.modified_add_cnt ++;
	}
	
	pthread_mutex_unlock(&ResourceTb_Run.lock);
	
	return 0;
}

//op:1-flush immediately,0-polling
int API_ResourceTb_Flush_Process(int op)
{
	time_t nowtime;
	
	pthread_mutex_lock(&ResourceTb_Run.lock);
	
	if(ResourceTb_Run.modified_add_cnt > 0 || ResourceTb_Run.modified_pre_cnt > 0)
	{	
		nowtime = time(NULL); 
		if(op == 1 || (nowtime -ResourceTb_Run.modified_time) > RESOURCETB_FLUSH_TIME)	
		{
			if(ResourceTb_Run.modified_add_cnt > 0 && ResourceTb_Run.modified_pre_cnt == 0)
			{
				save_vtk_table_file_buffer( presource_table, presource_table->record_cnt - ResourceTb_Run.modified_add_cnt, ResourceTb_Run.modified_add_cnt, RESOURCE_TABLE_NAME);
			}
			else
			{	
				FILE *pf = fopen(RESOURCE_TABLE_NAME,"r");
				if(pf != NULL)
				{
					fclose(pf);
					if(remove(RESOURCE_TABLE_NAME)  != 0)
					{
						pthread_mutex_unlock(&ResourceTb_Run.lock);
						return -1;
					}
				}
				
				if(save_vtk_table_file_buffer( presource_table, 0, presource_table->record_cnt, RESOURCE_TABLE_NAME) != 0)
				{
					pthread_mutex_unlock(&ResourceTb_Run.lock);
					return -1;
				}	
			}
			ResourceTb_Run.modified_add_cnt = 0;
			ResourceTb_Run.modified_pre_cnt = 0;
			sync();		//czn_20161012
		}
		
	}

	pthread_mutex_unlock(&ResourceTb_Run.lock);

	return 0;
}

int API_Resource_HaveUpdated(int rid,int result)
{
	one_vtk_dat	*pTempRecord = NULL;
	char			recordBuffer[BUFF_ONE_RECORD_LEN+1] = {0};
	char 		tempBuffer[RESOURCE_FILE_LEN+1];
	int			temp;
	int			i,j,recordlen;
	char 		*newrecord;
	time_t 		nowtime;
	
	if(presource_table == NULL)
	{
		return -1;
	}
	nowtime = time(NULL); 
	
	pthread_mutex_lock(&ResourceTb_Run.lock);
	for(i = 0;i<presource_table->record_cnt;i++)
	{
		temp = 20;
		if(get_one_table_record_string(presource_table,i,RESOURCE_ID,tempBuffer,&temp) != 0)
		{
			pthread_mutex_unlock(&ResourceTb_Run.lock);
			return -1;
		}
		
		tempBuffer[temp] = 0;
		
		if( atol(tempBuffer) == rid)
		{
			break;	
		}
	}

	if(i >= presource_table->record_cnt)
	{
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		return -1;
	}
	
	recordlen = 0;
	memcpy(recordBuffer + recordlen,tempBuffer,temp);
	recordlen += temp;
	recordBuffer[recordlen ++] = ',';
	
	temp = RESOURCE_FILE_LEN;
	if(get_one_table_record_string(presource_table,i,RESOURCE_FILE,tempBuffer,&temp) != 0)
	{
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		return -1;
	}
	memcpy(recordBuffer + recordlen,tempBuffer,temp);
	recordlen += temp;
	recordBuffer[recordlen ++] = ',';

	temp = 40;
	if(get_one_table_record_string(presource_table,i,RESOURCE_GETTIME,tempBuffer,&temp) != 0)
	{
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		return -1;
	}
	memcpy(recordBuffer + recordlen,tempBuffer,temp);
	recordlen += temp;
	recordBuffer[recordlen ++] = ',';

	temp = 40;
	if(get_one_table_record_string(presource_table,i,RESOURCE_ERRCODE,tempBuffer,&temp) != 0)
	{
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		return -1;
	}
	memcpy(recordBuffer + recordlen,tempBuffer,temp);
	recordlen += temp;
	
	recordBuffer[recordlen] = 0;
	snprintf(recordBuffer + recordlen,BUFF_ONE_RECORD_LEN - recordlen, ",%d,%d",nowtime,result);
	recordlen = strlen(recordBuffer);

	newrecord = malloc(recordlen);
		
	if(newrecord == NULL)
	{
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		return -1;
	}
	
	memcpy(newrecord,recordBuffer,recordlen);
	free(presource_table->precord[i].pdat);
	
	presource_table->precord[i].len = recordlen;
	presource_table->precord[i].pdat = newrecord;

	ResourceTb_Run.modified_time = nowtime;
	ResourceTb_Run.modified_pre_cnt ++;

	pthread_mutex_unlock(&ResourceTb_Run.lock);
	return 0;
}

int API_ResourceTb_DelOneRecord(int rid,char *delfilename)
{
	one_vtk_dat	*pTempRecord = NULL;
	char 		tempBuffer[RESOURCE_FILE_LEN + 1];
	int			temp;
	int			i,j,recordlen;
	time_t 		nowtime;

	if(presource_table == NULL)
	{
		return -1;
	}	
	nowtime = time(NULL);
	
	pthread_mutex_lock(&ResourceTb_Run.lock);
	for(i = 0;i<presource_table->record_cnt;i++)
	{
		temp = 20;
		if(get_one_table_record_string(presource_table,i,RESOURCE_ID,tempBuffer,&temp) != 0)
		{
			pthread_mutex_unlock(&ResourceTb_Run.lock);
			return -1;
		}
		
		tempBuffer[temp] = 0;
		temp = atol(tempBuffer);
		
		if(temp == rid)
		{
			break;	
		}
	}

	if(i >= presource_table->record_cnt)
	{
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		if(delfilename != NULL)
		{
			*delfilename = 0;
		}
	}
	else
	{
		if(delfilename != NULL)
		{
			temp = RESOURCE_FILE_LEN;
			if(get_one_table_record_string(presource_table,i,RESOURCE_FILE,delfilename,&temp) != 0)
			{
				pthread_mutex_unlock(&ResourceTb_Run.lock);
				return -1;
			}
			delfilename[temp] = 0;

			pTempRecord = malloc((presource_table->record_cnt-1) * sizeof(one_vtk_dat));
		
			if(pTempRecord == NULL)
			{
				pthread_mutex_unlock(&ResourceTb_Run.lock);
				return -1;
			}
			
			for(j = 0; j < i; j ++)
			{
				 pTempRecord[j] = presource_table->precord[j];
			}
			for(j += 1;j < presource_table->record_cnt;j ++)
			{
				pTempRecord[j-1] = presource_table->precord[j];
			}

			 free(presource_table->precord);

		 	presource_table->record_cnt --;
		 	presource_table->precord = pTempRecord;

			ResourceTb_Run.modified_time = nowtime;
			ResourceTb_Run.modified_pre_cnt ++;
		}
	}

	pthread_mutex_unlock(&ResourceTb_Run.lock);
	return 0;
}

int API_ResourceTb_GetOneRecord(int rid,ResourceTb_OneRecord_Stru *precord)
{
	char 		tempBuffer[RESOURCE_FILE_LEN + 1];
	int			temp;
	int			i,j,recordlen;
	char 		*newrecord;
	
	if(presource_table == NULL)
	{
		return -1;
	}
	
	pthread_mutex_lock(&ResourceTb_Run.lock);
	for(i = 0;i<presource_table->record_cnt;i++)
	{
		temp = 20;
		if(get_one_table_record_string(presource_table,i,RESOURCE_ID,tempBuffer,&temp) != 0)
		{
			pthread_mutex_unlock(&ResourceTb_Run.lock);
			return -1;
		}
		
		tempBuffer[temp] = 0;
		temp = atol(tempBuffer);
		
		if(temp == rid)
		{
			break;	
		}
	}

	if(i >= presource_table->record_cnt)
	{
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		return -1;
	}

	precord->rid = rid;

	temp = RESOURCE_FILE_LEN;
	if(get_one_table_record_string(presource_table,i,RESOURCE_FILE,tempBuffer,&temp) != 0)
	{
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		return -1;
	}	
	tempBuffer[temp] = 0;
	strcpy(precord->filename,tempBuffer);

	temp = 20;
	if(get_one_table_record_string(presource_table,i,RESOURCE_GETTIME,tempBuffer,&temp) != 0)
	{
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		return -1;
	}
	tempBuffer[temp] = 0;
	precord->get_time = atol(tempBuffer);

	temp = 20;
	if(get_one_table_record_string(presource_table,i,RESOURCE_ERRCODE,tempBuffer,&temp) != 0)
	{
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		return -1;
	}
	tempBuffer[temp] = 0;
	precord->err_code = atol(tempBuffer);

	temp = 20;
	if(get_one_table_record_string(presource_table,i,RESOURCE_LASTUPDATE,tempBuffer,&temp) != 0)
	{
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		return -1;
	}
	tempBuffer[temp] = 0;
	precord->last_update = atol(tempBuffer);

	temp = 20;
	if(get_one_table_record_string(presource_table,i,RESOURCE_UPDATERESULT,tempBuffer,&temp) != 0)
	{
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		return -1;
	}
	tempBuffer[temp] = 0;
	precord->update_result = atol(tempBuffer);
	
	pthread_mutex_unlock(&ResourceTb_Run.lock);
	return 0;
}

int API_ResourceTb_Verify(void)
{
	one_vtk_dat	*pTempRecord = NULL;
	char 		tempBuffer[RESOURCE_FILE_LEN + 1];
	int			temp;
	int			i,j,k,recordlen;
	char 		*newrecord;
	FILE			*pfile;
	
	if(presource_table == NULL)
	{
		return -1;
	}

	pthread_mutex_lock(&ResourceTb_Run.lock);
	for(i = 0,j = 0;i < (presource_table->record_cnt-j);)
	{
		temp = RESOURCE_FILE_LEN;
		if(get_one_table_record_string(presource_table,i,RESOURCE_FILE,tempBuffer,&temp) != 0)
		{
			continue;
		}
		
		tempBuffer[temp] = 0;

		pfile = fopen(tempBuffer,"r");
		
		if(pfile == NULL)
		{
			free(presource_table->precord[i].pdat);
			ResourceTb_Run.modified_pre_cnt ++;

			for(k = i+1;k < (presource_table->record_cnt-j);k++)
			{
				presource_table->precord[k-1] = presource_table->precord[k];
			}

			j ++;
			
		}
		else
		{
			i ++;
			fclose(pfile);
		}
	}
	if(j > 0)
	{
		presource_table->record_cnt -=j;

		pTempRecord = malloc((presource_table->record_cnt) * sizeof(one_vtk_dat));
			
		if(pTempRecord == NULL)
		{
			pthread_mutex_unlock(&ResourceTb_Run.lock);
			return -1;
		}
		
		for(i = 0; i < presource_table->record_cnt;i ++)
		{
			pTempRecord[i] = presource_table->precord[i];
		}
		if(presource_table->precord != NULL)
		{
			free(presource_table->precord);
		}
		presource_table->precord = pTempRecord;
		
		pthread_mutex_unlock(&ResourceTb_Run.lock);
		return API_ResourceTb_Flush_Process(1);

	}	
	pthread_mutex_unlock(&ResourceTb_Run.lock);
	return 0;
}

void ResourceTable_AutoSave_Timer_Callback(void)
{
	API_ResourceTb_Flush_Polling();
	OS_RetriggerTimer(&ResourceTb_Run.autosave_timer);
}
