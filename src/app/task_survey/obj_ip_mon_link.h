
#ifndef _OBJ_IP_MON_LINK_H
#define _OBJ_IP_MON_LINK_H

#include "task_survey.h"

typedef enum
{
	MON_SERVICE_ON,
	MON_SERVICE_NG,
	MON_SERVICE_PROXY,
} mon_link_rsp_state;

typedef enum
{
	MON_LINK_NG_NONE,
	MON_LINK_NG_GATEWAY_BUSY,
	MON_LINK_NG_TARGET_BUSY,
	MON_LINK_NG_NO_TARGET,
} mon_link_rsp_ng_reason;

#pragma pack(1)

typedef struct
{
	//通用头
	uint8 	mon_type;		// 监视类型
	uint8 	device_id;		// 设备id
	//请求数据内容
	uint16 	apply_type;		// 请求类型
}mon_link_request;

typedef struct
{
	mon_link_rsp_state 		state;			// 0-OK; 1-NG; 2-Proxy
	mon_link_rsp_ng_reason	reason;			// 不允许的原因 : 1-Gateway_BUSY; 2-Target_BUSY; 3-NO_TARGET
	uint32					ipaddr;	
	uint32					gateway;	
}mon_link_result;

typedef struct
{
	//通用头
	uint8 					mon_type;		// 监视类型
	uint8 					device_id;		// 设备id
	//应答数据内容
	mon_link_result			result;			// 返回结果
}mon_link_response;

#pragma pack()

int send_ip_mon_link_req( int target_ip, unsigned char dev_id, int* premote_ip, unsigned char* pdev_id  );
int recv_ip_mon_link_req( UDP_MSG_TYPE* psource );

#endif

