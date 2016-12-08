/**
  ******************************************************************************
  * @file    obj_TableProcess.h
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

#ifndef _obj_TableProcess_H
#define _obj_TableProcess_H

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <pthread.h>
#include <netinet/in.h>    
#include <netinet/if_ether.h>   
#include <net/if.h>   
#include <net/if_arp.h>   
#include <arpa/inet.h>     

#define BUFF_ONE_RECORD_LEN			200		// lzh_20160601
#define BUFF_ONE_KEY_NAME_LEN		200		// lzh_20160601

// lzh_20160601_e

// Define Object Property-------------------------------------------------------

// KEY值得基本定义结构，pdat为malloc得到的数据指针
typedef struct
{
	int				len;			// 数据区长度
	unsigned char*	pdat;			// 数据区指针
}one_vtk_dat;

// 一个表的数据结构
typedef struct
{
	// key name zone
	int				keyname_cnt;	// 表单的key name个数
	int*			pkeyname_len;	// 表单的key name长度限制
	one_vtk_dat* 	pkeyname;		// 表单的key name数据指针
	
	// record zone
	int				record_cnt;		// 表单的记录个数
	one_vtk_dat*	precord;		// 表单记录的数据去指针	
	// lzh_20160601_s
	pthread_mutex_t	lock;			// 并发处理数据保护锁
	// lzh_20160601_e	
} one_vtk_table;



// Define Object Function - Public----------------------------------------------

//加载一个表单文件
one_vtk_table*  load_vtk_table_file( const char* ptable_file_name);	
//释放一个表单文件
int free_vtk_table_file_buffer( one_vtk_table* ptable);


//搜索一个表单中键值对匹配的所有记录
one_vtk_table* search_vtk_table_with_key_value( one_vtk_table* ptable, unsigned char* key_name, unsigned char* key_value, int whole_word_only);	
//得到一个表单的一条记录记录信息，从键值对匹配的offset偏移量开始获取
one_vtk_dat* get_one_vtk_record( one_vtk_table* ptable, unsigned char* key_name, unsigned char* key_value, int index);	
//得到一个记录的字段字符串信息: char* pstring, int* plen(为业务buf的最大长度，操作完成后会改写) 
int get_one_record_string( one_vtk_dat* prec, int key_index, char* pstring, int* plen );
int set_one_record_string( one_vtk_dat* prec, int key_index, char* pstring );

//得到一个表单的指定一条记录的字段字符串信息: char* pstring, int* plen(为业务buf的最大长度，操作完成后会改写) 
int get_one_table_record_string( one_vtk_table* ptable, int rec_idx, unsigned char* key_name, char* pstring, int* plen);

//删除并释放一个表单
int release_one_vtk_tabl( one_vtk_table* ptable);	


// lzh_20160601_s
// 打印一个表单
int printf_one_table( one_vtk_table* ptable );
// 生成一个临时表单
one_vtk_table* create_one_vtk_table( int keyname_cnt, one_vtk_dat* pkeyname, int* pkeyvalue_len );
// 增加一条记录到表单最后
int add_one_vtk_record( one_vtk_table* ptable, one_vtk_dat* precord );
//得到一个表单的一条记录记录信息
one_vtk_dat* get_one_vtk_record_without_keyvalue( one_vtk_table* ptable, int index );
// 得到keyname在表中的偏移索引值
int get_keyname_index( one_vtk_table* ptable, unsigned char *keyname );

// lzh_20160601_e

// Define Object Function - Private---------------------------------------------
// 保存一个表单文件
int save_vtk_table_file_buffer( one_vtk_table* ptable, int start_index, int write_num, char* ptable_file_name );
int getNumberCount(int num);

#endif
