/**
  ******************************************************************************
  * @file    obj_TableProcess.c
  * @author  czb
  * @version V00.01.00
  * @date    2016.5.31
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2016 V-Tec</center></h2>
  ******************************************************************************
  */ 
#include "obj_TableProcess.h"
#include "../task_survey/task_survey.h"


// 找到关键字会有一个分隔符,故有效的关键字个数就是分隔符的个数(最后一个无分隔符关键字)
int GetStringDelimCnt(char* pstr, char delim )
{
	int cnt;
	for( cnt = 0; *pstr != 0; pstr++)
	{
		if(*pstr == delim)
		{
			cnt++;
		}
	}
	return cnt;
}

one_vtk_table*  LoadVtkTableFileProperty(FILE *file)
{
	one_vtk_table *	table;
	char 			buff[BUFF_ONE_RECORD_LEN];
	char 			*pos1,*pos2;
	unsigned char 	processState;	//处理状态
	int 				i;
	
	fseek(file, 0, SEEK_SET);
	
	table = malloc(sizeof(one_vtk_table));
	// lzh_20160601_s
	if( table == NULL ) return NULL;	
	pthread_mutex_init( &table->lock, 0);
	// lzh_20160601_s

	for(processState = 0, memset(buff, 0, BUFF_ONE_RECORD_LEN); fgets(buff,BUFF_ONE_RECORD_LEN-1,file) != NULL; memset(buff, 0, BUFF_ONE_RECORD_LEN))
	{
		switch(processState)
		{
			case 0:
				pos1 = strstr( buff, "[TABLE_KEY_NAME]" );
				if( pos1 != NULL )
				{
					processState = 1;
				}
				break;
			case 1:
				#if 0
				pos1 = strstr( buff, "key" );
				if( pos1 != NULL )
				{
					processState = 2;
					
					pos2 = pos1;
					
					for(table->keyname_cnt = 1; *pos1 != 0; pos1++)
					{
						if(*pos1 == ',')
						{
							table->keyname_cnt++;
						}
					}		
					
					table->pkeyname = malloc(table->keyname_cnt*sizeof(one_vtk_dat));

					strtok(pos2,"=, ");
					for(i= 0; (pos1 = strtok(NULL,"=, ")) != NULL && i< table->keyname_cnt; i++)
					{
						table->pkeyname[i].len = strlen(pos1);
						table->pkeyname[i].pdat = malloc(table->pkeyname[i].len);
						strncpy((char*)table->pkeyname[i].pdat, pos1, table->pkeyname[i].len);
					}
				}
				#else
				pos1 = strstr( buff, "key" );
				if( pos1 != NULL )
				{
					processState = 2;
					
					pos2 = pos1;

					//去掉空格从前往后检查
					for(i = 0; pos1[i] != 0; )
					{
						if( isspace(pos1[i]) )	// 去掉空格和回车换行
						{
							strcpy(&pos1[i], &pos1[i+1]);
							continue;
						}
						i++;
					}
					
					table->keyname_cnt = GetStringDelimCnt(pos1,',');
					
					table->pkeyname = malloc(table->keyname_cnt*sizeof(one_vtk_dat));

					strtok(pos2,"=, ");
					for(i= 0; (pos1 = strtok(NULL,", ")) != NULL && i< table->keyname_cnt; i++)
					{
						table->pkeyname[i].len = strlen(pos1);
						table->pkeyname[i].pdat = malloc(table->pkeyname[i].len);
						strncpy((char*)table->pkeyname[i].pdat, pos1, table->pkeyname[i].len);
					}
				}				
				#endif
				break;
			case 2:
				pos1 = strstr( buff, "max" );
				if( pos1 != NULL )
				{
					processState = 3;
					
					table->pkeyname_len = malloc(table->keyname_cnt*sizeof(int));
					
					strtok(pos1,"=, ");
					for(i= 0; (pos2 = strtok(NULL,", ")) != NULL && i< table->keyname_cnt; i++)
					{
						table->pkeyname_len[i] = atoi(pos2);
					}
				}
				break;
			case 3:
				pos1 = strstr( buff, "[TABLE_KEY_VALUE]" );
				if( pos1 != NULL )
				{
					processState = 4;
				}
				break;
			case 4:
				#if 0
				table->record_cnt = 0;
				pos1 = strstr( buff, "value" );
				if( pos1 != NULL )
				{
					table->record_cnt++;
					processState = 5;
				}
				#else
				table->record_cnt = 0;
				pos1 = strstr( buff, "value" );
				if( pos1 != NULL )
				{
					if( table->keyname_cnt == GetStringDelimCnt(pos1,',') )
					{
						table->record_cnt++;
						processState = 5;
					}
				}
				#endif
				break;				
			case 5:
				#if 0
				pos1 = strstr( buff, "value" );
				if( pos1 != NULL )
				{
					table->record_cnt++;
				}
				else
				{
					pos2 = strstr( buff, "[TABLE_END]" );
					if(pos2 != NULL )
					{
						processState = 6;
					}
				}
				#else
				pos1 = strstr( buff, "value" );
				if( pos1 != NULL )
				{
					if( table->keyname_cnt == GetStringDelimCnt(pos1,',') )
					{
						table->record_cnt++;
					}
				}
				else
				{
					pos2 = strstr( buff, "[TABLE_END]" );
					if(pos2 != NULL )
					{
						processState = 6;
					}
				}				
				#endif
				break;				
		}
		if(processState == 6)
		{
			break;
		}
	}

	if(processState < 5)		//文件有错
	{
		if(processState >= 2)
		{
			for(i= i; i< table->keyname_cnt; i++)
			{
				free(table->pkeyname[i].pdat);
				table->pkeyname[i].pdat = NULL;
			}
		}
			
		if(processState >= 3)
		{
			free(table->pkeyname_len);
			table->pkeyname_len = NULL;
		}

		free(table);
		table = NULL;
		return NULL;
	}

	table->precord = malloc(table->record_cnt*sizeof(one_vtk_dat));
	
	return table;
}

void  LoadVtkTableFileValue(FILE *file, one_vtk_table *	table)
{
	char 			buff[BUFF_ONE_RECORD_LEN];
	char 			tempbuff[BUFF_ONE_RECORD_LEN];
	char 			*pos1, *pos2;
	int			 	i;
	int				recordCnt;
	int				templenght;

	fseek(file, 0, SEEK_SET);
	
	for(recordCnt = 0, memset(buff, 0, BUFF_ONE_RECORD_LEN); fgets(buff,BUFF_ONE_RECORD_LEN-1,file) != NULL; memset(buff, 0, BUFF_ONE_RECORD_LEN))
	{
		pos1 = strstr( buff, "value" );
		if( pos1 != NULL )
		{
			// lzh_20160607_s
			pos2 = pos1;
			if( table->keyname_cnt != GetStringDelimCnt(pos2,',') )
				continue;
			// lzh_20160607_e
			
			//去掉空格从前往后检查
			for(i = 5; pos1[i] != 0; )
			{
				if(isspace(pos1[i]) && (pos1[i-1] == ',' || pos1[i-1] == '='))	//空格跟着","或者"="去掉
				{
					strcpy(&pos1[i], &pos1[i+1]);
					continue;
				}
				i++;
			}
			//去掉空格从后往前检查			
			for(i = strlen(pos1) - 1; i > 4; )
			{
				if(isspace(pos1[i]) && (pos1[i+1] == ',' || pos1[i+1] == '=' || pos1[i+1] == 0 ))	//空格在","或者"="或者"\0"前面去掉
				{
					strcpy(&pos1[i], &pos1[i+1]);
					continue;
				}
				i--;
			}

			// lzh_20160607_s			
			//strtok(pos1,"=");
			strtok(pos1,"=,");
			// lzh_20160607_e			
			for(i = 0, table->precord[recordCnt].len = 0, memset(tempbuff, 0, BUFF_ONE_RECORD_LEN); (pos2 = strtok(NULL,",")) != NULL && i< table->keyname_cnt; i++)
			{
				templenght = (strlen(pos2) < table->pkeyname_len[i]) ? strlen(pos2) : table->pkeyname_len[i];

				strncpy(tempbuff + table->precord[recordCnt].len, pos2, templenght);
				
				table->precord[recordCnt].len += templenght;
				
				tempbuff[table->precord[recordCnt].len++] = ',';
			}
			
			tempbuff[--(table->precord[recordCnt].len)] = 0;
			
			table->precord[recordCnt].pdat = malloc(table->precord[recordCnt].len);

			strncpy((char*)(table->precord[recordCnt].pdat), tempbuff, table->precord[recordCnt].len);
			
			if(++recordCnt == table->record_cnt)
			{
				break;
			}	
		}
		else
		{
			pos1 = strstr( buff, "[TABLE_END]" );
			if( pos1 != NULL )
			{
				break;
			}
		}
	}
	
}

//加载一个表单文件
one_vtk_table*  load_vtk_table_file( const char* ptable_file_name)
{
	one_vtk_table *	table;
	FILE 			*file = NULL;

	if( (file=fopen(ptable_file_name,"r")) == NULL )
	{
		return NULL;
	}
	//加载表属性
	if((table = LoadVtkTableFileProperty(file)) == NULL)
	{
		fclose(file);
		return NULL;
	}

	//加载表记录
	LoadVtkTableFileValue(file, table);

	fclose(file);
	return table;
}

//释放一个表单文件
int free_vtk_table_file_buffer( one_vtk_table* ptable)
{
	int i;
	
	if(ptable != NULL)
	{
		// lzh_20160601_s	
		pthread_mutex_lock(&ptable->lock);
		// lzh_20160601_e
	
		for(i = 0; i< ptable->keyname_cnt; i++)
		{
			free(ptable->pkeyname[i].pdat);
			ptable->pkeyname[i].pdat = NULL;
		}
		
		free(ptable->pkeyname);
		ptable->pkeyname = NULL;
		
		free(ptable->pkeyname_len);
		ptable->pkeyname_len = NULL;
		
		for(i = 0; i< ptable->record_cnt; i++)
		{
			free(ptable->precord[i].pdat);
			ptable->precord[i].pdat = NULL;
		}
		
		free(ptable->precord);
		ptable->precord = NULL;
		
		// lzh_20160601_s		
		pthread_mutex_unlock(&ptable->lock);
		// lzh_20160601_e
		
		free(ptable);
		ptable = NULL;
		
	}

	return 0;
}

//搜索一个表单中键值对匹配的所有记录
one_vtk_table* search_vtk_table_with_key_value( one_vtk_table* ptable, unsigned char* key_name, unsigned char* key_value, int whole_word_only )
{
	int 			i, j;
	int				keyNameNum;
	int				valueNum;
	int 			tempTableRecordCnt;
	char			inputUpper[BUFF_ONE_KEY_NAME_LEN];
	char			recordUpper[BUFF_ONE_KEY_NAME_LEN];
	char			tempRecord[BUFF_ONE_RECORD_LEN];
	char			*pos1;
	one_vtk_table 	*table;
	
	if(ptable == NULL)
	{
		return NULL;
	}

	// lzh_20160601_s	
	pthread_mutex_lock(&ptable->lock);
	// lzh_20160601_e
	
	if(ptable->keyname_cnt == 0 || ptable->record_cnt == 0)
	{
		// lzh_20160601_s	
		pthread_mutex_unlock(&ptable->lock);
		// lzh_20160601_e	
		return NULL;
	}
	
	for(i = 0, memset(inputUpper, 0, BUFF_ONE_KEY_NAME_LEN); i < strlen(key_name); i++)
	{
		inputUpper[i] = toupper(key_name[i]);
	}
	
	for(keyNameNum = 0; keyNameNum < ptable->keyname_cnt; keyNameNum++)
	{
		for(i = 0, memset(recordUpper, 0, BUFF_ONE_KEY_NAME_LEN); i < ptable->pkeyname[keyNameNum].len; i++)
		{
			recordUpper[i] = toupper(ptable->pkeyname[keyNameNum].pdat[i]);
		}
		if(!memcmp(inputUpper, recordUpper, ptable->pkeyname[keyNameNum].len))
		{
			break;
		}
	}

	if(keyNameNum >= ptable->keyname_cnt)	//没有该key_name
	{
		// lzh_20160601_s	
		pthread_mutex_unlock(&ptable->lock);
		// lzh_20160601_e	
		return NULL;
	}
	
	table = malloc(sizeof(one_vtk_table));
	// lzh_20160601_s
	if( table == NULL ) return NULL;	
	pthread_mutex_init( &table->lock, 0);
	// lzh_20160601_s
	
	table->keyname_cnt = ptable->keyname_cnt;
	
	table->pkeyname_len= ptable->pkeyname_len;
	
	table->pkeyname = ptable->pkeyname;
	
	for(i = 0, memset(inputUpper, 0, BUFF_ONE_KEY_NAME_LEN); i < strlen(key_value); i++)
	{
		inputUpper[i] = toupper(key_value[i]);
	}

	for(valueNum = 0, table->record_cnt = 0, memset(tempRecord, 0, BUFF_ONE_RECORD_LEN); valueNum < ptable->record_cnt; valueNum++)
	{
		strncpy(tempRecord, ptable->precord[valueNum].pdat, ptable->precord[valueNum].len);
		// lzh_20160607_s
		tempRecord[ptable->precord[valueNum].len] = ',';
		// lzh_20160607_e
		for(i = 0,pos1 = strtok(tempRecord,","); i < ptable->keyname_cnt && pos1 != NULL; i++, pos1 = strtok(NULL,","))
		{
			if(i == keyNameNum)
			{
				// lzh_20160601_s				
				memset(recordUpper, 0, BUFF_ONE_KEY_NAME_LEN);
				// lzh_20160601_e				
				for(j = 0; j< strlen(pos1); j++)
				{
					recordUpper[j] = toupper(pos1[j]);
				}
				break;
			}
		}
		if( whole_word_only )
		{		
			if( !strcmp( recordUpper, inputUpper ) )
			{
				table->record_cnt++;
			}
		}
		else
		{
			if(strstr( recordUpper, inputUpper ) != NULL)
			{
				table->record_cnt++;
			}			
		}
	}
	
	table->precord = malloc(table->record_cnt*sizeof(one_vtk_dat));

	for(valueNum = 0, tempTableRecordCnt = 0, memset(tempRecord, 0, BUFF_ONE_RECORD_LEN); valueNum < ptable->record_cnt; valueNum++)
	{
		strncpy(tempRecord, ptable->precord[valueNum].pdat, ptable->precord[valueNum].len);
		// lzh_20160607_s
		tempRecord[ptable->precord[valueNum].len] = ',';
		// lzh_20160607_e
		
		for(i = 0,pos1 = strtok(tempRecord,","); i < ptable->keyname_cnt && pos1 != NULL; i++, pos1 = strtok(NULL,","))
		{
			if(i == keyNameNum)
			{
				// lzh_20160601_s				
				memset(recordUpper, 0, BUFF_ONE_KEY_NAME_LEN);
				// lzh_20160601_e	
				for(j = 0; j< strlen(pos1); j++)
				{
					recordUpper[j] = toupper(pos1[j]);
				}
				break;
			}
		}

		if( whole_word_only )
		{
			if( !strcmp( recordUpper, inputUpper ) )
			{
				table->precord[tempTableRecordCnt].len = ptable->precord[valueNum].len;
				table->precord[tempTableRecordCnt++].pdat = ptable->precord[valueNum].pdat;
			}
		}
		else
		{
			if(strstr( recordUpper, inputUpper ) != NULL)
			{
				table->precord[tempTableRecordCnt].len = ptable->precord[valueNum].len;
				table->precord[tempTableRecordCnt++].pdat = ptable->precord[valueNum].pdat;
			}
		}
	}

	// lzh_20160601_s	
	pthread_mutex_unlock(&ptable->lock);
	// lzh_20160601_e

	return table;
}
//得到一个表单的一条记录记录信息，index为键值对匹配的offset
one_vtk_dat* get_one_vtk_record( one_vtk_table* ptable, unsigned char* key_name, unsigned char* key_value, int index)
{
	int 	i, j;
	int		keyNameNum;
	int		valueNum;
	int		tempTableRecordCnt;
	char	inputUpper[BUFF_ONE_KEY_NAME_LEN];
	char	recordUpper[BUFF_ONE_KEY_NAME_LEN];
	char	tempRecord[BUFF_ONE_RECORD_LEN];
	char	*pos1;
	
	if(ptable == NULL)
	{
		return NULL;
	}
	// lzh_20160601_s	
	pthread_mutex_lock(&ptable->lock);
	// lzh_20160601_e

	if(ptable->keyname_cnt == 0 || ptable->record_cnt == 0)
	{
		// lzh_20160601_s	
		pthread_mutex_unlock(&ptable->lock);
		// lzh_20160601_e		
		return NULL;
	}

	for(i = 0, memset(inputUpper, 0, BUFF_ONE_KEY_NAME_LEN); i < strlen(key_name); i++)
	{
		inputUpper[i] = toupper(key_name[i]);
	}
	
	for(keyNameNum = 0; keyNameNum < ptable->keyname_cnt; keyNameNum++)
	{
		for(i = 0, memset(recordUpper, 0, BUFF_ONE_KEY_NAME_LEN); i < ptable->pkeyname[keyNameNum].len; i++)
		{
			recordUpper[i] = toupper(ptable->pkeyname[keyNameNum].pdat[i]);
		}
		if(!memcmp(inputUpper, recordUpper, ptable->pkeyname[keyNameNum].len))
		{
			break;
		}
	}

	if(keyNameNum >= ptable->keyname_cnt)	//没有该key_name
	{
		// lzh_20160601_s	
		pthread_mutex_unlock(&ptable->lock);
		// lzh_20160601_e	
		return NULL;
	}
	
	for(i = 0, memset(inputUpper, 0, BUFF_ONE_KEY_NAME_LEN); i < strlen(key_value); i++)
	{
		inputUpper[i] = toupper(key_value[i]);
	}
	
	for(valueNum = 0, tempTableRecordCnt = 0, memset(tempRecord, 0, BUFF_ONE_RECORD_LEN); valueNum < ptable->record_cnt; valueNum++)
	{
		strncpy(tempRecord, ptable->precord[valueNum].pdat, ptable->precord[valueNum].len);
		
		for(i = 0,pos1 = strtok(tempRecord,","); i < ptable->keyname_cnt && pos1 != NULL; i++, pos1 = strtok(NULL,","))
		{
			if(i == keyNameNum)
			{
				for(j = 0, memset(recordUpper, 0, BUFF_ONE_KEY_NAME_LEN); j< strlen(pos1); j++)	//cao_20160708
				{
					recordUpper[j] = toupper(pos1[j]);
				}
				break;
			}
		}
		if(strstr( recordUpper, inputUpper ) != NULL)
		{
			if(tempTableRecordCnt++ == index)
			{
				// lzh_20160601_s
				//return &(ptable->precord[valueNum]);
				one_vtk_dat* prec = &(ptable->precord[valueNum]);
				pthread_mutex_unlock(&ptable->lock);
				return prec;
				// lzh_20160601_e			
			}
		}
	}

	// lzh_20160601_s	
	pthread_mutex_unlock(&ptable->lock);
	// lzh_20160601_e

	return NULL;
}

// lzh_20160601_s
//生成一个新表单: keyname_cnt - keyname个数，pkeyname - keyname名称，pkeyvalue_len - keyvalue的最大值
one_vtk_table* create_one_vtk_table( int keyname_cnt, one_vtk_dat* pkeyname, int* pkeyvalue_len )
{
	one_vtk_table * table;

	if( keyname_cnt == 0 || pkeyname == NULL || pkeyvalue_len == NULL )
		return NULL;

	table = malloc(sizeof(one_vtk_table));
	if( table == NULL ) 
		return NULL;
	
	pthread_mutex_init( &table->lock, 0);

	table->keyname_cnt 	= keyname_cnt;
	table->pkeyname_len	= pkeyvalue_len;
	table->pkeyname		= pkeyname;
	
	table->record_cnt	= 0;
	table->precord		= NULL;
	
	return table;
}

//向一个表单写入一条记录，若表单不存在则新建表单，将记录追加在表单的最后
//return: 当前记录的条数
int add_one_vtk_record( one_vtk_table* ptable, one_vtk_dat* precord )
{
	if(ptable == NULL || precord == NULL )
	{
		return -1;
	}

	pthread_mutex_lock(&ptable->lock);
	
	if(ptable->keyname_cnt == 0)
	{
		pthread_mutex_unlock(&ptable->lock);
		return -1;
	}

#if 0
	int i;
	one_vtk_dat* pold = ptable->precord;
	ptable->precord = malloc( (ptable->record_cnt+1)*sizeof(one_vtk_dat) );		// 多申请一个空间
	if( ptable->precord == NULL )
	{
		ptable->precord = pold;	// 恢复指针
		pthread_mutex_unlock(&ptable->lock);
		return -1;		
	}
	// copy old all node
	if( pold != NULL )
	{
		memcpy( ptable->precord, pold, ptable->record_cnt*sizeof(one_vtk_dat) );
	}
	// copy new one node
	ptable->precord[ptable->record_cnt].len		= precord->len;
	ptable->precord[ptable->record_cnt].pdat 	= precord->pdat;
	// table record cnt increase 1
	ptable->record_cnt++;
	// realse old node
	if( pold != NULL ) free(pold);
#else
	int i;
	one_vtk_dat* pold = ptable->precord;
	if( ptable->record_cnt == 0 )
	{
		ptable->precord = malloc( sizeof(one_vtk_dat) ); 	// 申请一个空间
		if( ptable->precord == NULL )
		{
			ptable->precord = pold; // 恢复指针
			pthread_mutex_unlock(&ptable->lock);
			return -1;		
		}
	}
	else
	{
		ptable->precord = realloc( pold, (ptable->record_cnt+1)*sizeof(one_vtk_dat) ); 	// 多申请一个空间
		if( ptable->precord == NULL )
		{
			ptable->precord = pold; // 恢复指针
			pthread_mutex_unlock(&ptable->lock);
			return -1;		
		}		
	}
	// copy new one node
	ptable->precord[ptable->record_cnt].len 	= precord->len;
	ptable->precord[ptable->record_cnt].pdat	= precord->pdat;
	// table record cnt increase 1
	ptable->record_cnt++;
#endif
	// lzh_20160601_s	
	pthread_mutex_unlock(&ptable->lock);
	// lzh_20160601_e

	return 0;

}

//得到一个表单的一条记录记录信息
one_vtk_dat* get_one_vtk_record_without_keyvalue( one_vtk_table* ptable, int index )
{
	pthread_mutex_lock(&ptable->lock);
	
	if( ptable == NULL || ptable->keyname_cnt == 0 || ptable->record_cnt == 0 || ptable->record_cnt <= index )
	{
		// lzh_20160601_s	
		pthread_mutex_unlock(&ptable->lock);
		// lzh_20160601_e		
		return NULL;
	}

	one_vtk_dat* prec = &(ptable->precord[index]);
	
	pthread_mutex_unlock(&ptable->lock);
	
	return prec;
}

// lzh_20160601_e

//得到一个记录的字段字符串信息: char* pstring, int* plen(为业务buf的最大长度，操作完成后会改写) 
int get_one_record_string( one_vtk_dat* prec, int key_index, char* pstring, int* plen )
{
	char	tempRecord[BUFF_ONE_RECORD_LEN];
	char	*pos1;
	int 	i;

	strncpy(tempRecord, prec->pdat, prec->len);
	// lzh_20160607_s
	tempRecord[prec->len] = '\0';
	// lzh_20160607_e

	for(i = 0,pos1 = strtok(tempRecord,","); i < key_index && pos1 != NULL; i++, pos1 = strtok(NULL,","));
	if(i == key_index && pos1 != NULL)
	{
		if(*plen > strlen(pos1))
		{
			*plen = strlen(pos1);
		}
		strncpy(pstring, pos1, *plen);
		return 0;
	}
	return -1;
}

//得到一个表单的指定一条记录的字段字符串信息: char* pstring, int* plen(为业务buf的最大长度，操作完成后会改写) 
int get_one_table_record_string( one_vtk_table* ptable, int rec_idx, unsigned char* key_name, char* pstring, int* plen)
{
	int 	i;
	int	keyNameNum;
	char	inputUpper[BUFF_ONE_KEY_NAME_LEN];
	char	recordUpper[BUFF_ONE_KEY_NAME_LEN];
	
	if(ptable == NULL)
	{
		return -1;
	}

	// lzh_20160601_s	
	pthread_mutex_lock(&ptable->lock);
	// lzh_20160601_e
	
	if(ptable->keyname_cnt == 0 || ptable->record_cnt == 0)
	{
		// lzh_20160601_s	
		pthread_mutex_unlock(&ptable->lock);
		// lzh_20160601_e	
		return -2;
	}
	
	for(i = 0, memset(inputUpper, 0, BUFF_ONE_KEY_NAME_LEN); i < strlen(key_name); i++)
	{
		inputUpper[i] = toupper(key_name[i]);
	}
	
	for(keyNameNum = 0; keyNameNum < ptable->keyname_cnt; keyNameNum++)
	{
		for(i = 0, memset(recordUpper, 0, BUFF_ONE_KEY_NAME_LEN); i < ptable->pkeyname[keyNameNum].len; i++)
		{
			recordUpper[i] = toupper(ptable->pkeyname[keyNameNum].pdat[i]);
		}
		if(!memcmp(inputUpper, recordUpper, ptable->pkeyname[keyNameNum].len))
		{
			break;
		}
	}

	if(keyNameNum >= ptable->keyname_cnt)	//没有该key_name
	{
		// lzh_20160601_s	
		pthread_mutex_unlock(&ptable->lock);
		// lzh_20160601_e	
		return -3;
	}
	
	// lzh_20160601_s	
	i = get_one_record_string( &(ptable->precord[rec_idx]), keyNameNum, pstring, plen );
	pthread_mutex_unlock(&ptable->lock);
	return i;
	// lzh_20160601_e
}

//删除并释放一个表单返回0删除成功
int release_one_vtk_tabl( one_vtk_table* ptable )
{
	if(ptable != NULL)
	{
		// lzh_20160601_s	
		pthread_mutex_lock(&ptable->lock);
		// lzh_20160601_e
		
		free(ptable->precord);
		ptable->pkeyname_len = NULL;
		ptable->pkeyname = NULL;
		ptable->precord = NULL;
		
		// lzh_20160601_s	
		pthread_mutex_unlock(&ptable->lock);
		// lzh_20160601_e
		
		free(ptable);
		ptable = NULL;
	}

	return 0;
}

// lzh_20160601_s
int printf_one_table( one_vtk_table* ptable )
{
#if 1
	int i;
	char buf[BUFF_ONE_KEY_NAME_LEN];

	if( ptable == NULL )
	{
		printf("null table can not to print\n");
		return -1;
	}
	// printf table key_name
	printf("table keyname numbers = %d\n", ptable->keyname_cnt);
	for(i = 0;i < ptable->keyname_cnt; i++)
	{
		memcpy(buf,ptable->pkeyname[i].pdat,ptable->pkeyname[i].len);
		buf[ptable->pkeyname[i].len] = 0;
		printf("keyname[%d].dat=%s,len=%d,max=%d\n", i, buf,ptable->pkeyname[i].len,ptable->pkeyname_len[i]);
	}
	// printf table key_name
	printf("table record numbers = %d\n", ptable->record_cnt);	
	for(i = 0;i < ptable->record_cnt; i++ )
	{
		memcpy(buf,ptable->precord[i].pdat,ptable->precord[i].len);
		buf[ptable->precord[i].len] = 0;
		printf("record[%d].dat=%s,len=%d\n", i, buf,ptable->precord[i].len);
	}
	printf("\n\n");
#endif

	return 0;
}

// 得到keyname在表中的偏移索引值
int get_keyname_index( one_vtk_table* ptable, unsigned char *keyname )
{
	int 	i;
	int 	keyNameNum;
	char	recordUpper[BUFF_ONE_KEY_NAME_LEN];	
	for(keyNameNum = 0; keyNameNum < ptable->keyname_cnt; keyNameNum++)
	{
		for(i = 0, memset(recordUpper, 0, BUFF_ONE_KEY_NAME_LEN); i < ptable->pkeyname[keyNameNum].len; i++ )
		{
			recordUpper[i] = toupper(ptable->pkeyname[keyNameNum].pdat[i]);
		}
		//bprintf("str1=%s,str2=%s,len=%d\n",keyname,recordUpper,ptable->pkeyname[keyNameNum].len);
		if( !memcmp(keyname, recordUpper, ptable->pkeyname[keyNameNum].len) )
		{
			break;
		}
	}
	if( keyNameNum == ptable->keyname_cnt )
	{
		bprintf("get_keyname_index err,num=%d,str1=%s,str2=%s\n",keyNameNum,keyname,recordUpper);
		return -1;
	}
	else
		return keyNameNum;
}
// lzh_20160601_e


//保存一个表单几个记录到文件
int save_vtk_table_file_buffer( one_vtk_table* ptable, int start_index, int write_num, char* ptable_file_name )
{					
	FILE* file 		= NULL;
	char 	fileFlag	= 0;
	char writeBuffer[BUFF_ONE_RECORD_LEN];
	char tempBuffer[BUFF_ONE_KEY_NAME_LEN];
	int	i;
	int	bufferIndex;
	int	temp;
	
	pthread_mutex_lock(&ptable->lock);
	
	if(ptable == NULL)
	{
		pthread_mutex_unlock(&ptable->lock);
		return -1;
	}

	if(ptable->record_cnt <  start_index + write_num)
	{
		pthread_mutex_unlock(&ptable->lock);
		return -2;
	}

	file = fopen( ptable_file_name, "r+" );
	if( file == NULL )
	{
		fileFlag = 1;
		file = fopen( ptable_file_name, "a+" );
		if( file == NULL )
		{
			pthread_mutex_unlock(&ptable->lock);
			printf( "open %s error.\n",ptable_file_name );
			return -3;
		}
	}

	if(fileFlag == 1)
	{	
		fputs("[TABLE_KEY_NAME]\r\n", file);
		//fprintf(file,"[TABLE_KEY_NAME]\r\n");
		
		memset(writeBuffer, 0, BUFF_ONE_RECORD_LEN);
		memcpy(writeBuffer, "key=,", 5);
		for(i = 0, bufferIndex = 5; i< ptable->keyname_cnt; i++)
		{
			memcpy(writeBuffer + bufferIndex, ptable->pkeyname[i].pdat, ptable->pkeyname[i].len);
			bufferIndex += ptable->pkeyname[i].len;
			if(i != ptable->keyname_cnt - 1)
			{
				writeBuffer[bufferIndex++] = ',';
			}
			else
			{
				writeBuffer[bufferIndex++] = '\r';
				writeBuffer[bufferIndex++] = '\n';
			}
		}
		fputs(writeBuffer, file);
		
		//fprintf(file,writeBuffer);
		memset(writeBuffer, 0, BUFF_ONE_RECORD_LEN);
		memcpy(writeBuffer, "max=,", 5);
		for(i = 0, bufferIndex = 5; i< ptable->keyname_cnt; i++)
		{
			temp = getNumberCount(ptable->pkeyname_len[i]);
			memset(tempBuffer, 0, BUFF_ONE_RECORD_LEN);
			gcvt(ptable->pkeyname_len[i], temp, tempBuffer);
			
			memcpy(writeBuffer + bufferIndex, tempBuffer, temp);
			bufferIndex += temp;
			if(i != ptable->keyname_cnt - 1)
			{
				writeBuffer[bufferIndex++] = ',';
			}
			else
			{
				writeBuffer[bufferIndex++] = '\r';
				writeBuffer[bufferIndex++] = '\n';
			}
		}
		fputs(writeBuffer, file);
		fputs("[TABLE_KEY_VALUE]\r\n", file);
		//fprintf(file,writeBuffer);
		//fprintf(file,"[TABLE_KEY_VALUE]\r\n");
	}

	fseek(file, 0, SEEK_END);
	
	for(i = 0; i < write_num; i++)
	{
		memset(writeBuffer, 0, BUFF_ONE_RECORD_LEN);
		memcpy(writeBuffer, "value=,", 7);
		memcpy(writeBuffer + 7, ptable->precord[start_index + i].pdat, ptable->precord[start_index + i].len);
		bufferIndex = 7 + ptable->precord[start_index + i].len;
		writeBuffer[bufferIndex++] = '\r';
		writeBuffer[bufferIndex++] = '\n';
		fputs(writeBuffer, file);
		//fprintf(file,writeBuffer);
	}
	
	pthread_mutex_unlock(&ptable->lock);
	//if(fflush(NULL) == 0)
	{
		bprintf("fflush ok!\n");		
	}
	fclose(file);
	return 0;
}


int getNumberCount(int num)
{
	int i;
	while(num  = num / 10)	i++;
	return i + 1;
}

/*********************************************************************************************************
**  End Of File
**********************************************************************************************************/
