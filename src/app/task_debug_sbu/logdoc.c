
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>

#include <time.h>   
#include <sys/types.h>   
#include <sys/socket.h>   
#include <sys/ioctl.h>   
                  
#include <netinet/in.h>    
#include <netinet/if_ether.h>   
#include <net/if.h>   
#include <net/if_arp.h>   
#include <arpa/inet.h>     

#include <errno.h>

#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../task_survey/task_survey.h"
#include "logdoc.h"


static char logfile_name[MAX_PATH_LENGTH];
static int	logfile_line;

FILE* logfile = NULL;

/****************************************************************************************************************************************
 * @fn      OpenLogFile()
 *
 * @brief   打开logDoc文件，task_debug_sbu会打开该文件后写入记录
 *
 * @param   none
 *
 * @return  0 - ok, other - err
 ***************************************************************************************************************************************/
int OpenLogFile(void)
{	
	if( ( logfile = fopen(logfile_name,"a+") ) == NULL )
	{
		eprintf( "open logfile error:%s\n",strerror(errno) );
		return -1;
	}
	return 0;
}

/****************************************************************************************************************************************
 * @fn      CloseLogFile()
 *
 * @brief   关闭logDoc文件，task_debug_sbu写入记录后，会关闭该文件
 *
 * @param   none
 *
 * @return  0 - ok, other - err
 ***************************************************************************************************************************************/
int CloseLogFile(void)
{	
	fclose( logfile );
	return 0;
}

/****************************************************************************************************************************************
 * @fn      InitialLogFileLines()
 *
 * @brief   得到logDoc的文档条数，若logDoc文档不存在，则自动生成该文件，若存在，则分析得到该文档有效的记录条数，在main初始化时调用
 *
 * @param   none
 *
 * @return  有效记录条数
 ***************************************************************************************************************************************/
int InitialLogFileLines( void )
{
	char buff[MAX_PATH_LENGTH] = {'\0'};
	FILE *file = NULL;
	
	snprintf(logfile_name, MAX_PATH_LENGTH, "%smnt/nand1-2/business.log",getenv("HOME"));

	logfile_line = 0;

	if( (file=fopen(logfile_name,"a+")) == NULL )
	{
		eprintf( "parse error:%s\n",strerror(errno) );
		return -1;
	}

	while( fgets(buff,MAX_PATH_LENGTH,file) != NULL )
	{
		logfile_line++;
	}
	
	fclose( file );

	printf("log file lines = %d\n", logfile_line);
	return 0;
}

/****************************************************************************************************************************************
 * @fn      JustifyLogFileLength()
 *
 * @brief   调整文件长度
 *
 * @param   none
 *
 * @return  有效记录条数
 ***************************************************************************************************************************************/
int JustifyLogFileLength( void )
{
	char buff[MAX_PATH_LENGTH] = {'\0'};
	char logfile_temp[MAX_PATH_LENGTH];	
	
	FILE *file_temp = NULL;
	FILE *file_log = NULL;
	
	int logfile_line_temp;

	if( logfile_line >= MAX_FILE_LINES )
	{
		//	open log file
		if( (file_log = fopen( logfile_name, "r" )) == NULL )
		{
			eprintf( "open logfile error:%s\n",strerror(errno) );
			return -1;
		}

		snprintf(logfile_temp, MAX_PATH_LENGTH, "%s.tmp",logfile_name);
		
		//	open temp file and empty file
		if( (file_temp = fopen( logfile_temp, "a+" )) == NULL )
		{
			eprintf( "open logfile temp error:%s\n",strerror(errno) );
			return -1;
		}

		//move top
		fseek( file_log, 0, SEEK_SET);
		fseek( file_temp, 0, SEEK_SET);

		logfile_line_temp = 0;
		//read line
		while( fgets(buff, MAX_PATH_LENGTH, file_log) != NULL )
		{
			logfile_line_temp++;
			if( logfile_line_temp > (MAX_FILE_LINES/2) )
			{
				// write temp file
				fprintf( file_temp, "%s", buff );
			}
		}
		// 重新开始计数
		logfile_line = 0;
			
		fclose(file_log);
		fclose(file_temp);
		
		// 删除旧的log file
		if( unlink(logfile_name) < 0 )
		{
			eprintf("unlink error !\n");
		}
		// 更改临时文件为log file名称
		if( rename( logfile_temp, logfile_name ) < 0 )
		{
			eprintf("rename error !\n");			
		}
	}
	return logfile_line;
}

/****************************************************************************************************************************************
 * @fn      write_rec_to_log_doc()
 *
 * @brief   写入一条记录到文档
 *
 * @param   prec_str - 写入记录的字符串指针
 *
 * @return  -1 - err，有效纪录长度
 ***************************************************************************************************************************************/
int write_rec_to_log_doc( char* prec_str )
{
	if( logfile != NULL )
	{
		fprintf( logfile, "%04d %s", logfile_line+1, prec_str );
		// line increase
		logfile_line++;
		return logfile_line;
	}
	else
		return -1;
}


/****************************************************************************************************************************************
 * @fn      read_rec_fm_log_doc()
 *
 * @brief   读取log档的记录数据
 *
 * @param   index 	- 记录索引号
 * @param   pDat 	- 记录数据指针
 *
 * @return  0 - ok, other - err
 ***************************************************************************************************************************************/
int read_rec_fm_log_doc( int index, char* pDat )
{
	return 0;
}


