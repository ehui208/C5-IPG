
#ifndef _VIDEO_MULTICAST_COMMON_H
#define _VIDEO_MULTICAST_COMMON_H

#include "video_object.h"
#include "../vtk_udp_stack/vtk_udp_stack_class.h"
#include "../vtk_udp_stack/vtk_udp_stack_c5_ipc_cmd.h"

#include "../task_survey/task_survey.h"
#include "../task_survey/sys_msg_process.h"
#include "../task_debug_sbu/task_debug_sbu.h"

#define	VDP_PRINTF_VIDEO

#ifdef	VDP_PRINTF_VIDEO
#define	vd_printf(fmt,...)	printf("[V]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)
#else
#define	vd_printf(fmt,...)
#endif

// C/S交互命令定义
typedef enum
{
	// communication ack
	COM_ACK=0,				//  client 	<-> server
	
	// subscribe couple
	SUBSCRIBE_REQ=0x0001,			//  client 	-> server
	SUBSCRIBE_RSP=0x0081,			//  server 	-> client
	
	// desubscribe couple
	DESUBSCRIBE_REQ=0x0002,		//  client 	-> server
	DESUBSCRIBE_RSP=0x0082,		//  server 	-> client
	
	// cancel couple
	CANCEL_REQ=0x0003,				//	server	-> client	
	CANCEL_RSP=0x0083,				//	client	-> server
} vd_subscribe_cmd;

// 服务器端response回复的result类型
typedef enum
{
	VD_RESPONSE_RESULT_NONE,		// 无效
	VD_RESPONSE_RESULT_ENABLE,		// 允许
	VD_RESPONSE_RESULT_DISABLE,		// 不允许
	//转到代理服务器
	VD_RESPONSE_RESULT_TO_PROXY,	// 允许到代理
} vd_response_result;

// 视频资源状态报告类型
typedef enum
{
	VD_NOTIFY_NORMAL,			// 视频信号正常
	VD_NOTIFY_NO_SIGNAL,		// 无视频信号	
	VD_NOTIFY_SIGNAL_RESET,		// 视频信号重启
} vd_notify_State;

// subscribe couple

// 客户端申请的数据包格式
typedef struct
{
	target_head			target;
	vd_response_result	result;				// 申请的回复结果
	int					vd_multicast_time;	// 视频组播的时间(s)
	int					dev_id;				// 申请视像头设备id
} vd_subscribe_req_pack;

// 服务器端响应客户端的申请数据包格式
typedef struct
{
	target_head			target;
	vd_response_result	result;				// 申请的回复结果
	int					vd_multicast_time;	// 视频组播的时间(s)
	short				vd_multicast_port;	// 视频组播地端口号
	int					vd_multicast_ip;	// 视频组播地址
} vd_subscribe_rsp_pack;

// desubscribe couple
typedef struct _vd_desubscribe_req_pack_t
{
	target_head			target;
	vd_response_result	result;
} vd_desubscribe_req_pack;

typedef struct _vd_desubscribe_rsp_pack_t
{
	target_head			target;
	vd_response_result	result;
} vd_desubscribe_rsp_pack;

// cancel couple
typedef struct _vd_cancel_req_pack_t
{
	target_head			target;
	vd_response_result	result;				// 申请的回复结果
} vd_cancel_req_pack;

typedef struct _vd_cancel_rsq_pack_t
{
	target_head			target;
	vd_response_result	result;				// 申请的回复结果
} vd_cancel_rsq_pack;


/////////////////////////////////////////////////////
#include "./ip_camera_control/encoder_vin/obj_adjust.h"

typedef enum
{
    ADJ_GET,
    ADJ_SET,
    ADJ_DEC,
    ADJ_INC,
}Dir_ype_t;

#if 0

typedef enum
{
    ADJ_CONTRAST,
    ADJ_BRIGHT,
    ADJ_COLOR,
    ADJ_ALL,
}AdjustType_t;

typedef struct
{
	unsigned char   	logContrastCnt;
	unsigned char 	logBrightCnt;
	unsigned char 	logColorCnt;
}ImagePara_t;
#endif

typedef struct
{
	Dir_ype_t		dir;
	AdjustType_t	type;
    ImagePara_s		data;
	int				state;  // lzh_20160530 state: 0/ok, 1/server close
} UDP_Image_t;

#endif


