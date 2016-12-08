
#ifndef _SYS_MSG_PROCESS_H
#define _SYS_MSG_PROCESS_H

typedef struct
{
	int type;				//消息功能类型
	int status;				//消息状态
	int wparam;				//消息功能键值1
	int lparam;				//消息功能键值2
} SYS_WIN_MSG;

//所有的消息类型
#define MSG_0_PKEY			0		//机械式按键消息类型
#define MSG_1_IKEY			1		//遥控器按键消息类型
#define MSG_2_TKEY			2		//触摸屏按键消息类新
#define MSG_3_VKEY			3		//虚拟按键消息类型
#define MSG_4_TOUT			4		//超时消息类型
#define MSG_5_MCMD			5		//UDP命令消息类型
#define MSG_6_HOOK			5		//插簧消息类型

// 虚拟按键消息
#define VKEY_MENU			1
#define VKEY_UP				2
#define VKEY_DOWN			3
#define VKEY_LEFT			4
#define VKEY_RIGHT			5
#define VKEY_UNLOCK1		6
#define VKEY_UNLOCK2		7
#define VKEY_POWER			8
#define VKEY_MESG			9
#define VKEY_ALARM			10
#define VKEY_CALL			11
#define VKEY_LOGVIEW		12
#define VKEY_HOOK			13

//czn_20160601_s
#define SURVEY_MSG_CALLLINK_MANAGE				100
	#define SURVEY_SUBMSG_CALLLINK_WRITE_STATE					0
	#define SURVEY_SUBMSG_CALLLINK_READ_STATE					1
	#define SURVEY_SUBMSG_CALLLINK_WRITE_CALLERDATA			2
	#define SURVEY_SUBMSG_CALLLINK_WRITE_BECALLEDDATA			3
	#define SURVEY_SUBMSG_CALLLINK_WRITE_TRANSFERDATA			4
	
#define SURVEY_MSG_LOG_WRITE					101
#define SURVEY_MSG_PRINTF_LOG_ON_DS				102
#define SURVEY_MSG_GETLOCALIP					103

#define SURVEY_MSG_VIDEO_SERVICE_ON				105
#define SURVEY_MSG_VIDEO_SERVICE_OFF			106

void Survey_SundryMsg_Process(char* pdata, int len);
//czn_20160601_e

void Call_Survey_Unlink_Deal(UDP_MSG_TYPE *pUdpType);//czn_20160422

int	survey_sys_message_processing( char* pdata, int len );

// lzh_20160503_s
int Interface_monitor_local_video(void);
int Interface_monitor_remote_video(int ip,int dev_id);
int Interface_monitor_video_close(void);

int API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(int ip);
int API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(int ip);

int API_VIDEO_S_SERVICE_TURN_ON_MULTICAST(int ip);
int API_VIDEO_S_SERVICE_TURN_OFF_MULTICAST(int ip);

int UdpSurvey_idle_Prcessing(UDP_MSG_TYPE *pUdpType);
int UdpSurvey_Filter_Prcessing( UDP_MSG_TYPE *pUdpType );

// lzh_20160503_e

#endif


