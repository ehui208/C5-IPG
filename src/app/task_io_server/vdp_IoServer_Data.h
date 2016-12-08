/**
  ******************************************************************************
  * @file    obj_IoServer_Data.h
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

#ifndef _obj_IoServer_Data_H
#define _obj_IoServer_Data_H

#define INIT_DISABLE				0x00
#define INSTALLER_INIT_ENABLE		0x01
#define FACTORY_INIT_ENABLE		0x02
#define USER_INIT_ENABLE			0x04


// Define Object Property-------------------------------------------------------
#define KNX_000	0
#define KNX_001	1
#define KNX_002	2
#define KNX_003	3
#define KNX_004	4
#define KNX_005	5
#define KNX_006	6
#define KNX_007	7
#define KNX_008	8
#define KNX_009	9
#define KNX_010	10
#define KNX_011	11
#define KNX_012	12
#define KNX_013	13
#define KNX_014	14
#define KNX_015	15
#define KNX_016	16
#define KNX_017	17
#define KNX_018	18
#define KNX_019	19
#define KNX_020	20
#define KNX_021	21
#define KNX_022	22
#define KNX_023	23



//KNX基本类型定义
typedef unsigned char	KNX_000_B1_BOOL;	//0 Boolean
typedef unsigned char	KNX_001_B1_BIT0;	//1 1-Bit Controled
typedef unsigned char	KNX_002_B1_BIT012;	//2 3-Bit Controled
typedef unsigned char	KNX_003_B1_CHARSET;	//3 Char Set
typedef unsigned char	KNX_004_B1_UCHAR;	//4 8-Bit Unsigned
typedef signed char		KNX_005_B1_CHAR;	//5 8-Bit Signed

typedef unsigned short	KNX_006_B2_USHORT;	//6 Unsigned Value
typedef signed short	KNX_007_B2_SHORT;	//7 Signed Value
typedef unsigned short	KNX_008_B2_FLOAT;	//8 Floated Value
typedef unsigned short	KNX_009_B2_VERSION;	//9 Version

typedef struct _KNX_010_B3_TIME_			//10 Time
{
	unsigned char TIME[3];
} KNX_010_B3_TIME;

typedef struct _KNX_011_B3_DATE_			//11 Date
{
	unsigned char DATE[3];
} KNX_011_B3_DATE;

typedef unsigned long	KNX_012_B4_UINT;	//12 Unsigned Value
typedef long			KNX_013_B4_INT;		//13 Signed Value
typedef long			KNX_014_B4_FLOAT;	//14 Float Value
typedef long			KNX_015_B4_ACCESS;	//15 Access_6-digits_option

typedef struct _KNX_016_B8_DATETIME_		//16 DateTime
{
	unsigned char actual_byte;	//存储空间(包含本byte)
	unsigned char date_time[8];
} _KNX_016_B8_DATETIME_;
typedef struct _KNX_017_B8_STR8_			//17 ShortString
{
	unsigned char actual_byte;	//存储空间(包含本byte)
	unsigned char STR8[8];
} KNX_017_B8_STR8;

typedef struct _KNX_018_B14_STR14_			//18 String
{
	unsigned char actual_byte;	//存储空间(包含本byte)
	unsigned char STR14[14];
} KNX_018_B14_STR14;

typedef struct _KNX_019_B24_NAMELIST_		//19 data
{
	unsigned char actual_byte;		//存储空间(包含本byte)
	
	unsigned char valid_data;		//字符数
	unsigned char flag_valid;		//有效标志: 0=有效, 0x20=无效
	unsigned char dip_address;		//DIP设置的地址: 0~31
	unsigned char data[21];			//实际最多存储20byte字符,余1byte余留
} KNX_019_B24_NAMELIST;

typedef struct _KNX_020_B19_STR19_
{
	unsigned char actual_byte;	//存储空间(包含本byte)
	unsigned char STR19[19];
} KNX_020_B19_STR19;

typedef struct _KNX_021_B21_STR21_
{
	unsigned char actual_byte;	//存储空间(包含本byte)
	unsigned char STR21[21];	
} KNX_021_B21_STR21;

typedef struct _KNX_022_B5_DATA_
{
	unsigned char actual_byte;	//存储空间(包含本byte)
	unsigned char DATA[5];
} KNX_022_B5_DATA;

typedef struct _KNX_023_B220_STR220_
{
	unsigned char actual_byte;	//存储空间(包含本byte)
	unsigned char STR220[220];
} KNX_023_B220_STR220;


/*
 *文件功能说明：
 *  该文件为工具软件生成的参数表的头文件
 *
 *文件字段解析：
 *	 ADDR_DPT:	
 *  DESCRIPTION:为参数带描叙的类型
 *  BaseAddress:为参数的起始地址
 *  XXX_xxxxxxx:为参数的功能编号,有3个功能：
 *    1.得到参数当前值：作为EEPROM_ADDR表中的索引来得到EEPROM地址，从而读写该参数的当前值
 *    2.得到参数的描叙值：作为EEPROM_ADDR表中的索引来得到该参数是否带有描叙，然后来查找Description_TAB中的描叙
 *    3.得到参数的最大值、最小值、缺省值（最大和最小值针对不同的类型固定）
 */

//扩展类型定义
typedef struct _INFO_
{
	unsigned int address;
	unsigned char level;
	unsigned char dpt;               //data length
	unsigned char init_type;
} INFO;

typedef struct _DESCRIPTION_
{
	unsigned short id;				
	unsigned char actual_byte;		
	unsigned char STR8[8];
} DESCRIPTION;



#define BaseAddress 		0x0808000	

#define DeviceAdd				0						//设备地址						//出厂初始化
#define DeviceName				1						//设备名称						//code区 无需初始化						
#define DeviceModel				2						//设备型号						//code区 无需初始化							
#define Description				3						//设备描述						//code区 无需初始化						
#define FWRevision				4						//硬件版本						//code区 无需初始化						
#define SWRevision				5						//软件版本						//code区 无需初始化						
#define ProtocolVersion			6						//协议版本						//code区 无需初始化							
#define SerialNumber			7						//生产唯一序列号				//出厂初始化
#define MFgDate					8						//生产日期						//出厂初始化
#define ControlCode				9						//区位控制码					//出厂初始化
#define DeviceTypeID			10						//设备类型						//code区 无需初始化						
#define DisplayName				11						//销售商名称					//出厂初始化
#define DisplayMode				12						//销售商型号					//出厂初始化
#define Installation			13						//工程名称						//出厂初始化
#define Location				14						//安装位置						//出厂初始化
#define UpdateDate				15						//软件升级时间					//出厂初始化
#define AccessPassword			16						//访问密码						//出厂初始化
#define EventUpload				17						//事件上报						//出厂初始化
#define WaitVoltage				18						//待机电压						//出厂初始化
#define WorkVoltage				19						//工作电压						//出厂初始化

#define IO_CameraBright			20
#define IO_CameraColor			21
#define IO_CameraContrast		22

#define PROPERTY_NUMBS 			(IO_CameraContrast + 1)


// Define Object Function - Public----------------------------------------------
extern const INFO EEPROM_ADDR[];
extern const unsigned char* const ptrRomTab[];


// Define Object Function - Private---------------------------------------------
void IoServer_Data_Init(void);	


// Define Object Function - other-----------------------------------------------


#endif
