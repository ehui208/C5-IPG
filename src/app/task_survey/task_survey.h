
#ifndef _VDP_CONTROLLER_H
#define _VDP_CONTROLLER_H

#include "../utility.h"
#include "../unix_socket.h"
#include "../vtk_udp_stack/vtk_udp_stack_class.h"
#include "./obj_CallServer_Virtual/obj_IP_Call_Link.h"				//czn_20160427

#define		VDP_DAT_BUF_LEN					240
#define 	VDP_THREAD_CYCLE_TIME			600
#define 	VDP_QUEUE_POLLING_TIME			0  //1000		// 队列polling的等待时间

//消息目标对象定义：
#define MSG_ID_ACK				1

#define MSG_ID_UART				2

#define MSG_ID_survey			10		// 消息分发控制任务
#define MSG_ID_udp_stack		13		// udp stack

#define MSG_ID_NetManage		100		// 网络管理对象消息
#define MSG_ID_IOServer			101		// IO服务对象消息
#define MSG_ID_GOServer			102		// GO服务对象消息
#define MSG_ID_DeviceManage		105		// 设备管理对象消息
#define MSG_ID_CallSurvey		106		// 呼叫业务服务对象消息
#define MSG_ID_NamelistManage 	108

#define MSG_ID_DEBUG_SBU		110		// 调试服务对象消息

//czn_20160526_s
#define MSG_ID_CallServer		111		// 
#define MSG_ID_IpCalller			112		// 
#define MSG_ID_IpBeCalled		113
//czn_20160526_e

//czn_20160217_s
#define MSG_ID_CALL_LINK		114
#define MSG_ID_MON_LINK			115
#define MSG_ID_UNIT_LINK		116
#define MSG_ID_Phone_Rep		211
#define MSG_ID_CAMERA_CTRL		116
//czn_20160217_e

#define CALL_SURVEY_IDLE			0
#define CALL_SURVEY_ASCALLER		1
#define CALL_SURVEY_ASBECALLED		2
//lzh_20160503_s
#define CALL_SURVEY_MONITOR			3
//lzh_20160503_s


extern unsigned char Call_Survey_State;
//czn_20160418_e

typedef struct
{
	// 消息传输头
	unsigned char 	msg_target_id;		// 消息的目标对象
	unsigned char 	msg_source_id;		// 消息的源对象
	unsigned char 	msg_type;			// 消息的类型
	unsigned char  	msg_sub_type;		 //消息的子类型
} VDP_MSG_HEAD;

typedef struct 
{
	VDP_MSG_HEAD	head;
	int				target_ip;
	int 			cmd;
	int 			id;	
	int 			len;
	char 			pbuf[VDP_DAT_BUF_LEN];
} UDP_MSG_TYPE;

typedef struct 
{
	VDP_MSG_HEAD	head;
	int 			len;
	char 			pbuf[VDP_DAT_BUF_LEN];
} UART_MSG_TYPE;

void vtk_TaskInit_survey( void );

extern vdp_task_t	task_control;
extern vdp_task_t	task_debug_sbu;
extern vdp_task_t	task_io_server;
extern vdp_task_t	task_net_manange;
// czn_20160526_s
extern vdp_task_t	task_caller;
extern vdp_task_t	task_becalled;
// czn_20160526_e

vdp_task_t* GetTaskAccordingMsgID(unsigned char msg_id);
int GetMsgIDAccordingPid( pthread_t pid );

// 功能: 等待业务同步应答
// 参数: pBusiness - 同步等待时的挂起的队列，business_type - 等待的业务类型， data - 得到的数据，plen - 数据有效长度
// 返回: 0 - 返回超时，1 -  返回正常
int WaitForBusinessACK( p_Loop_vdp_common_buffer pBusiness, unsigned char business_type,  char* data, int* plen, int timeout );


/****************************************************************************************************************************
 * @fn:		API_add_message_to_suvey_queue
 *
 * @brief:	加入消息到分发服务队列
 *
 * @param:  pdata 			- 数据指针
 * @param:  len 			- 数据长度
 *
 * @return: 0/ok, 1/full
****************************************************************************************************************************/
int	API_add_message_to_suvey_queue( char* pdata, unsigned int len );


int API_one_message_to_suvey_queue( int msg, int sub_msg );

#define VIDEO_SEVICE_NOTIFY_REMOTE_ON()		API_one_message_to_suvey_queue(SURVEY_MSG_VIDEO_SERVICE_ON,0)
#define VIDEO_SEVICE_NOTIFY_REMOTE_OFF()	API_one_message_to_suvey_queue(SURVEY_MSG_VIDEO_SERVICE_OFF,0)

#endif

