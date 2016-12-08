
#include "call_trace.h"

#include "task_survey/uart_udp_msg_process.h"

// 根据输入数据包来过滤需要的类型，每个机型的过滤类型可能不同
int  call_trace_type_input_filer(unsigned char* pdat, int len, call_trace_data_type *pfiler_type)
{
	/*
	C5_IPG_PACK* 	pPack 	= (C5_IPG_PACK*)pdat;
	unsigned short* pcmd 	= (unsigned short*)pPack->dat;
	switch( pPack->target.type )
	{
		case DATA_SINGLE:			// 数据转发到udp
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			break;
		
		case AUDIO_RUN: 			// 启动音频
			break;
			
		case DATA_AUDIO_RUN:		// 启动音频+数据转发
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
			API_talk_on_by_unicast(pPack->target.ip,AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);
			break;
			
		case AUDIO_STOP:			// 停止音频
			API_talk_off(); 		
			break;
			
		case DATA_AUDIO_STOP:		// 停止音频+数据转发
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
			API_talk_off(); 		
			break;
			
		case VIDEO_REC_RUN: 		// 启动视频接收(解码)
			API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(pPack->target.ip,1000);
			break;
			
		case DATA_VIDEO_REC_RUN:	// 启动视频接收(解码)+数据转发
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
			API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(pPack->target.ip,1000);				
			break;
			
		case VIDEO_REC_STOP:		// 停止视频接收(解码)
			API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST();
			break;
			
		case DATA_VIDEO_REC_STOP:	// 停止视频接收(解码)+数据转发
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
			API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST();
			break;
			
		case VIDEO_TRA_RUN: 		// 启动视频发送(编码)
			break;
			
		case DATA_VIDEO_TRA_RUN:	// 启动视频发送(编码)+数据转发
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
			break;
			
		case VIDEO_TRA_STOP:		// 停止视频发送(编码)
			break;
			
		case DATA_VIDEO_TRA_STOP:	// 停止视频发送(编码)+数据转发
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
			break;
			
		case ARUN_VREC_RUN: 		// 启动音频+启动视频接收(解码)
			API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(pPack->target.ip,1000);
			API_talk_on_by_unicast(pPack->target.ip,AUDIO_SERVER_UNICAST_PORT, AUDIO_CLIENT_UNICAST_PORT);							
			break;
			
		case DATA_ARUN_VREC_RUN:	// 启动音频+启动视频接收(解码)+数据转发
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
			API_talk_on_by_unicast(pPack->target.ip,AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);
			API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(pPack->target.ip,1000);
			break;
			
		case ASTP_VREC_STP: 		// 停止音频+停止视频接收(解码)
			API_talk_off();
			API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST();
			break;
			
		case DATA_ASTP_VREC_STP:	// 停止音频+停止视频接收(解码)+数据转发
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
			API_talk_off();
			API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST();
			break;
			
		case ARUN_VTRA_RUN: 		// 启动音频+启动视频发送(编码)
			API_talk_on_by_unicast(pPack->target.ip,AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);
			break;
			
		case DATA_ARUN_VTRA_RUN:	// 启动音频+启动视频发送(编码)+数据转发
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
			API_talk_on_by_unicast(pPack->target.ip,AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);
			break;
			
		case ASTP_VTRA_STP: 		// 停止音频+停止视频发送(编码)
			API_talk_off();
			break;
			
		case DATA_ASTP_VTRA_STP:	// 停止音频+停止视频发送(编码)+数据转发
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
			API_talk_off();
			break;
														
		case DATA_ASTP_VTRA_RUN:	// 数据转发，同时音频停止、视频接收
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
			API_talk_off();
			break;
			
		case DATA_ASTP_VALL_STP:	// 数据转发，同时音频停止、视频接收发送停止
			api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
			api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
			API_talk_off();
			API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST();												
			break;	
	}	
	*/
	return 0;
}

// 根据输出数据包来过滤需要的类型，每个机型的过滤类型可能不同
int  call_trace_type_output_filer(unsigned char* pdat, int len, call_trace_data_type *pfiler_type)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
int m_CallTraceData[trace_type_max]={0};
int	m_CallTraceSuccStepCnt = 0;
int m_CallTraceSuccResult = 0;

const call_trace_data_type CALL_TRACE_BUSINESS_SUCC_STEP_LIST_TAB[]=
{
	trace_call_apply_number,
	trace_call_reply_number,
	
	trace_video_on_apply_number,
	trace_video_on_reply_number,
	
	trace_video_on_number,
	
	trace_talk_apply_number,
	trace_talk_reply_number,
	
	trace_audio_on_number,
	
	trace_unlock_apply_number,
	trace_unlock_reply_number,
	
	trace_close_apply_number,
	trace_close_reply_number,
	
	trace_video_off_apply_number,
	trace_video_off_reply_number,
	
	trace_video_off_number,
	trace_audio_off_number,
};

// 有效业务次数的分析过程
int  call_trace_anaysis_data_type_stream(call_trace_data_type  type)
{
	// 登记任意类型的数据包
	m_CallTraceData[type]++;

	// 分析正确的呼叫业务数据包，登记正确的呼叫业务次数
	if( type == CALL_TRACE_BUSINESS_SUCC_STEP_LIST_TAB[m_CallTraceSuccStepCnt] )
	{
		m_CallTraceSuccStepCnt++;
		if( m_CallTraceSuccStepCnt >= (sizeof(CALL_TRACE_BUSINESS_SUCC_STEP_LIST_TAB)/sizeof(call_trace_data_type)) )
		{
			m_CallTraceSuccResult++;
		}
	}
	else
	{
		m_CallTraceSuccStepCnt = 0;
	}	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 外部API
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 统计数器复位
int  call_trace_anaysis_data_reset(void)
{
	int i;
	for( i = 0; i < trace_type_max; i++ )
	{
		m_CallTraceData[i] = 0;
	}

	m_CallTraceSuccResult = 0;
	
	return 0;
}

// 输入数据流分析
int  call_trace_anaysis_data_input(unsigned char* pdat, int len)
{
	call_trace_data_type input_type;
	
	if( call_trace_type_input_filer(pdat,len,&input_type) == 0 )
	{
		call_trace_anaysis_data_type_stream(input_type);
	}
	
	return 0;
}

// 输出数据流分析
int  call_trace_anaysis_data_output(unsigned char* pdat, int len)
{
	call_trace_data_type output_type;
	
	if( call_trace_type_output_filer(pdat,len,&output_type) == 0 )
	{
		call_trace_anaysis_data_type_stream(output_type);
	}
	
	return 0;
}



