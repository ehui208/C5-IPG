/**
  ******************************************************************************
  * @file    obj_IoServer_Data.c
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

#include "vdp_IoServer_Data.h"
#include "task_IoServer.h"



//EEPROM ADDRESS TABLE
const INFO EEPROM_ADDR[] =
{
	{BaseAddress+1,0x04,0x02,FACTORY_INIT_ENABLE},		/* ID0*/
	{BaseAddress+2,0x00,0x0f,FACTORY_INIT_ENABLE},		/* ID1*/
	{BaseAddress+3,0x00,0x0f,FACTORY_INIT_ENABLE},		/* ID2*/
	{BaseAddress+4,0x00,0x0f,FACTORY_INIT_ENABLE},		/* ID3*/
	{BaseAddress+5,0x00,0x0f,FACTORY_INIT_ENABLE},		/* ID4*/
	{BaseAddress+6,0x00,0x02,FACTORY_INIT_ENABLE},		/* ID5*/
	{BaseAddress+7,0x00,0x02,FACTORY_INIT_ENABLE},		/* ID6*/
	{BaseAddress+8,0x01,0x04,FACTORY_INIT_ENABLE},		/* ID7*/
	{BaseAddress+9,0x01,0x03,FACTORY_INIT_ENABLE},		/* ID8*/
	{BaseAddress+10,0x01,0x02,FACTORY_INIT_ENABLE},	/* ID9*/
	{BaseAddress+11,0x00,0x0f,FACTORY_INIT_ENABLE},		/* ID10 */
	{BaseAddress+12,0x02,0x0f,FACTORY_INIT_ENABLE},		/* ID11 */
	{BaseAddress+13,0x02,0x0f,FACTORY_INIT_ENABLE},		/* ID12 */
	{BaseAddress+14,0x03,0x0f,FACTORY_INIT_ENABLE},		/* ID13 */
	{BaseAddress+15,0x03,0x0f,FACTORY_INIT_ENABLE},		/* ID14 */
	{BaseAddress+16,0x03,0x03,FACTORY_INIT_ENABLE},	/* ID15 */
	{BaseAddress+17,0x03,0x04,FACTORY_INIT_ENABLE},	/* ID16 */
	{BaseAddress+18,0x03,0x01,FACTORY_INIT_ENABLE},	/* ID17 */
	{BaseAddress+19,0x00,0x02,FACTORY_INIT_ENABLE},	/* ID18 */
	{BaseAddress+20,0x00,0x02,FACTORY_INIT_ENABLE},	/* ID19 */

	{BaseAddress+21,0x00,0x01,FACTORY_INIT_ENABLE},	/* ID20 */
	{BaseAddress+22,0x00,0x01,FACTORY_INIT_ENABLE},	/* ID21 */
	{BaseAddress+23,0x00,0x01,FACTORY_INIT_ENABLE},	/* ID22 */
};


//DEFAULT PROPERY DEFINE	
const KNX_006_B2_USHORT		ID_0_DeviceAddr[3]			= {0x0000,0xFFFF,0x0037};
const KNX_018_B14_STR14		ID_1_DeviceName[1]		= {6,"C5 IPG"};
const KNX_018_B14_STR14		ID_2_DeviceModel[1]		= {6,"C5_IPG"};
const KNX_018_B14_STR14		ID_3_Description[1]		= {5,"C5 IPG"};
const KNX_018_B14_STR14		ID_4_FWRevision[1]		= {11,"C5_IPG a1.0"};
const KNX_009_B2_VERSION	ID_5_SWRevision[3]		= {0x0000,0xffff,0x0040};	//V000100
const KNX_009_B2_VERSION	ID_6_ProtocolVersion[3]	= {0x0000,0xffff,0x0040};
const KNX_015_B4_ACCESS		ID_7_SerialNumber[3]	= {0x00000000,0xffffffff,0x10000000};
const KNX_011_B3_DATE		ID_8_MFgDate[3]			= {{0x00,0x00,0x00},{0xff,0xff,0xff},{30,8,14}};	//KNX格式: 日/月/年	 => 2013年9月29日
const KNX_006_B2_USHORT		ID_9_ControlCode[3]		= {0x0000,0xffff,0x1234};
const KNX_018_B14_STR14		ID_10_DeviceTypeID[1]	= {5,"C5 IPG"};
const KNX_018_B14_STR14		ID_11_DisplayName[1]	= {6,"123456"};
const KNX_018_B14_STR14		ID_12_DisplayMode[1]	= {1,"-"};
const KNX_018_B14_STR14		ID_13_Installation[1]	= {1,"-"};
const KNX_018_B14_STR14		ID_14_Location[1]		= {1,"-"};
const KNX_011_B3_DATE		ID_15_UpdateDate[3]		= {{0x00,0x00,0x00},{0xff,0xff,0xff},{30,8,14}};
const KNX_012_B4_UINT		ID_16_AccessPassword[3]	= {0x00000000,0xffffffff,0x12345678};
const KNX_001_B1_BIT0		ID_17_EventUpload[3]	= {0,1,0};
const KNX_006_B2_USHORT		ID_18_WaitVoltage[3]		= {0x0000,0xffff,0x1234};
const KNX_006_B2_USHORT		ID_19_WorkVoltage[3]	= {0x0000,0xffff,0x0000};
//lzh
const KNX_004_B1_UCHAR		ID_20_CameraBright[3]	= {0,9,1};
const KNX_004_B1_UCHAR		ID_21_CameraColor[3]	= {0,9,4};
const KNX_004_B1_UCHAR		ID_22_CameraContrast[3]	= {0,9,5};

//DEFAULT PARAMETER POINTER TABLE
const unsigned char* const ptrRomTab[] =
{
	(const unsigned char*)ID_0_DeviceAddr,				/* ID0*/	
	(const unsigned char*)ID_1_DeviceName,				/* ID1*/
	(const unsigned char*)ID_2_DeviceModel,				/* ID2*/
	(const unsigned char*)ID_3_Description,				/* ID3*/
	(const unsigned char*)ID_4_FWRevision,				/* ID4*/
	(const unsigned char*)ID_5_SWRevision,				/* ID5*/
	(const unsigned char*)ID_6_ProtocolVersion,			/* ID6*/
	(const unsigned char*)ID_7_SerialNumber,			/* ID7*/
	(const unsigned char*)ID_8_MFgDate,				/* ID8*/
	(const unsigned char*)ID_9_ControlCode,				/* ID9*/
	(const unsigned char*)ID_10_DeviceTypeID,			/* ID10*/
	(const unsigned char*)ID_11_DisplayName,			/* ID11*/
	(const unsigned char*)ID_12_DisplayMode,			/* ID12*/
	(const unsigned char*)ID_13_Installation,				/* ID13*/
	(const unsigned char*)ID_14_Location,				/* ID14*/
	(const unsigned char*)ID_15_UpdateDate,			/* ID15*/
	(const unsigned char*)ID_16_AccessPassword,		/* ID16*/
	(const unsigned char*)ID_17_EventUpload,			/* ID17*/
	(const unsigned char*)ID_18_WaitVoltage,			/* ID18*/
	(const unsigned char*)ID_19_WorkVoltage,			/* ID19*/

	(const unsigned char*)ID_20_CameraBright,			/* ID20*/
	(const unsigned char*)ID_21_CameraColor,			/* ID21*/
	(const unsigned char*)ID_22_CameraContrast,			/* ID22*/
};

/*------------------------------------------------------------------------
						IoServer_Data_Init
入口:  
	无

处理:
	初始化
------------------------------------------------------------------------*/
void IoServer_Data_Init(void)	//read
{
	//flag_switch_io = IO_SWITCH_ALL_ENABLE;  //初始化IO接收消息的开关, 允许普通任务或协议栈发送消息到IO
}



/*********************************************************************************************************
**  End Of File
*********************************************************************************************************/
