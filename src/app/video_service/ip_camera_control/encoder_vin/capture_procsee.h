
#ifndef _MULTICAST_CAPTURE_PROCESS_H_
#define _MULTICAST_CAPTURE_PROCESS_H_

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <linux/videodev.h>
#include <errno.h>
#include <linux/fb.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <signal.h>
#include <dirent.h>

#include "obj_h264e.h"

typedef struct
{
	int32_t  Ip;
	int16_t  Port;
}SUB_MSG_TO_CAPTURE_t;

typedef enum
{
	 QVGA_ENC,
	 VGA_ENC,
} H264_ENC_VIDEO_TYPE;

typedef struct
{
	int width;
	int height;

} H264_ENC_VIDEO_PARA;

int SetH264EncVideo( H264_ENC_VIDEO_TYPE type );
H264_ENC_VIDEO_PARA GetH264EncVideo( void );
int SetH264EncVideoPara( ENCODE_PROFILE_t para );
ENCODE_PROFILE_t GetH264EncVideoPara( void );


void  EnableCapture( SUB_MSG_TO_CAPTURE_t Msg );
void  DisableCapture( void );

//////////////////////////////////////////////////////////////////////////////
typedef enum
{
	 CAPUTRE_IDLE,
	 CAPUTRE_START,
	 CAPUTRE_RUN,
	 CAPUTRE_STOP,
}
CAPUTRE_STATE_t;

int catpure_process_init( void );
CAPUTRE_STATE_t catpure_process_state_get( void );
int catpure_process_state_set_idle( void );
int catpure_process_state_set_start( void );
int catpure_process_state_set_run( void );
int catpure_process_state_set_stop( void );

#endif
