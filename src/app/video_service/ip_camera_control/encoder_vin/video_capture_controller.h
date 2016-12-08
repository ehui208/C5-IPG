
#ifndef _VIDEO_CAPTURE_CONTROLLER_H_
#define _VIDEO_CAPTURE_CONTROLLER_H_


#include "capture_procsee.h"



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

#define H264_ENC_TEST_MODE_DISABLE		0
#define H264_ENC_TEST_MODE_ENABLE		1

#define  H264_ENC_TEST_MODE				H264_ENC_TEST_MODE_DISABLE

#if H264_ENC_TEST_MODE
#include "../../os/RTOS.h"
#include "../../os/OSQ.h"

#else
#include "../../../../os/RTOS.h"
#include "../../../../os/OSQ.h"

#endif


typedef struct
{
	int8_t  			Source;
	SUB_MSG_TO_CAPTURE_t	ToCapture;
} MsgVideoCaptureController_t;

void vtk_VideoCaptureController( void );
void API_CaptureStart( int16_t port, int32_t mcg_lowbyte );
void API_CaptureStop( void );
void API_ToMulticastLeave( int16_t port, int32_t mcg_lowbyte );
void API_ToMulticastJoin( int16_t port, int32_t mcg_lowbyte );

#endif

