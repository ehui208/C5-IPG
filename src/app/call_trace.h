
#ifndef _CALL_TRACE_H
#define _CALL_TRACE_H


// 分析数据类型
typedef enum
{
	trace_call_apply_number = 0,	// 呼叫申请次数
	trace_call_reply_number,		// 呼叫回复次数
	trace_video_on_apply_number,	// 视频开启申请次数
	trace_video_on_reply_number,	// 视频开启回复次数
	trace_video_on_number,			// 视频开启次数
	trace_talk_apply_number,		// 通话开启申请次数
	trace_talk_reply_number,		// 通话开启回复次数
	trace_audio_on_number,			// 音频开启次数
	trace_unlock_apply_number,		// 开锁申请次数
	trace_unlock_reply_number,		// 开锁回复次数
	trace_close_apply_number,		// 呼叫关闭申请次数
	trace_close_reply_number,		// 呼叫关闭回复次数
	trace_video_off_apply_number,	// 视频关闭申请次数
	trace_video_off_reply_number,	// 视频关闭回复次数
	trace_video_off_number,			// 视频关闭次数
	trace_audio_off_number, 		// 音频关闭次数
	trace_type_max					// 自动测试类型总数
} call_trace_data_type; 

// 统计数器复位
int  call_trace_anaysis_data_reset(void);
// 输入数据流分析
int  call_trace_anaysis_data_input(unsigned char* pdat, int len);
// 输出数据流分析
int  call_trace_anaysis_data_output(unsigned char* pdat, int len);


#endif


