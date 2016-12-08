
#ifndef _MULTICAST_VIDEO_PROCESS_H_
#define _MULTICAST_VIDEO_PROCESS_H_

//#include "multicast_data_process.h"
//#include "video_displaye_controller.h"

// lzh_20161015_s
#define SUPPORT_PACK_FRAGMENT	1
// lzh_20161015_e

typedef enum
{
	 VIDEO_DISPLAY_JOIN,
	 VIDEO_DISPLAY_LEAVE,
}
VIDEO_DISPLAY_CMD_t;

typedef enum
{
	 VIDEO_DISPLAY_IDLE,
	 VIDEO_DISPLAY_START,
	 VIDEO_DISPLAY_RUN,
	 VIDEO_DISPLAY_STOP,
}
VIDEO_DISPLAY_STATE_t;

typedef struct
{
	int32_t  Ip;
	int16_t  Port;;	
	int8_t 	 Cmd;
}SUB_MSG_VIDEO_DISPLAY_t;

typedef struct
{
	int32_t Sd;
	int32_t addr;
	int32_t Port;
} H264_RECEIVE_t;

typedef enum
{
	QVGA_DEC,
	VGA_DEC,
} H264_DEC_VIDEO_TYPE;

typedef struct
{
	int width;
	int height;

} H264_DEC_VIDEO_PARA;

int SetH264DecVideo( H264_DEC_VIDEO_TYPE type );
H264_DEC_VIDEO_PARA GetH264DecVideo( void );


int EnableVideoDisplay( int32_t type, int16_t port, int32_t mcg_lowbyte );
int DisableVideoDisplay( int32_t type, int16_t port, int32_t mcg_lowbyte );


// lzh_20161015_s
#ifdef SUPPORT_PACK_FRAGMENT
int start_udp_recv_task( void );
int close_udp_recv_task( void );
#endif


int video_process_init( void );
VIDEO_DISPLAY_STATE_t video_process_state_get( void );
int video_process_state_set_idle( void );
int video_process_state_set_start( void );
int video_process_state_set_run( void );
int video_process_state_set_stop( void );

#endif



