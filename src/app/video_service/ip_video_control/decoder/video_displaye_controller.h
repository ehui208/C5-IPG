
#ifndef _VIDEO_DISPLAYE_CONTROLLER_H_
#define _VIDEO_DISPLAYE_CONTROLLER_H_


#define H264_DEC_TEST_MODE_DISABLE		0
#define H264_DEC_TEST_MODE_ENABLE		1

#define  H264_DEC_TEST_MODE				H264_DEC_TEST_MODE_DISABLE

#if H264_DEC_TEST_MODE
#include "../../os/RTOS.h"
#include "../../os/OSQ.h"
#else
#include "../../../../os/RTOS.h"
#include "../../../../os/OSQ.h"
#endif


#include "video_process.h"



typedef struct
{
	int8_t  			Source;
	SUB_MSG_VIDEO_DISPLAY_t 	VideoDisplay;
} MsgVideoDisplayerController_t;

void vtk_VideoDisplayerController( void );
void API_FromMulticastJoin( int16_t port, int32_t mcg_lowbyte );
void API_FromMulticastLeave( int16_t port, int32_t mcg_lowbyte );	
void API_FromUnicastJoin( int16_t port, int32_t mcg_lowbyte );
void API_FromUnicastLeave( void );


#endif

