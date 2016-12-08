#ifndef _H264D_DEFINE_H_
#define _H264D_DEFINE_H_

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

#include "favc_avcodec.h"

//-------------------------------------
// Decoder part
//-------------------------------------
#define FAVC_DECODER_DEV  		"/dev/w55fa92_264dec"

//-------------------------------------
// Encoder part
//-------------------------------------
#define FAVC_ENCODER_DEV  		"/dev/w55fa92_264enc"

#define FIX_QUANT				0
#define IPInterval					31

#define FB_PINGPONG_BUF     		1

#define MAX_IMG_WIDTH                   			640//640//320
#define MAX_IMG_HEIGHT                  			480//480//240

#define DEC_SIZE							MAX_IMG_WIDTH*MAX_IMG_HEIGHT*3/2



//-------------------------------------
// Data structure
//-------------------------------------

typedef struct _Decode_
{
	void* Data;
}DECODE_OBJ_t;

typedef struct AVFrame 
{
    u_int8_t *data[4];
} AVFrame;

typedef struct video_profile
{
    u_int32_t bit_rate;
    u_int32_t width;   //length per dma buffer
    u_int32_t height;
    u_int32_t framerate;
    u_int32_t frame_rate_base;
    u_int32_t gop_size;
    u_int32_t qmax;
    u_int32_t qmin;
    u_int32_t quant;
    u_int32_t intra;
    AVFrame coded;
    int8_t *priv;
} video_profile;

typedef struct _H264DecInfo_
{
	int32_t FD;
	int8_t* Name;
	int32_t Bitstreamsize;
	u_int8_t* OutPutVirBuf;
	avc_workbuf_t OutPutBuf;
	FAVC_DEC_PARAM Param;
	video_profile Profile;
}H264DecInfo;


#define DecodeNew0( type, count )  (type*)H264DecMalloc0( sizeof(type)*count )

void* H264DecMalloc0( size_t size );

//void vtk_TaskInit_H264_dec( unsigned char priority );

int CloseH264Dec( DECODE_OBJ_t* f );
int InitH264Dec( DECODE_OBJ_t* f, int Srcwidth, int Srcheight );
int DecodeH264( DECODE_OBJ_t* f , unsigned char *frame,  int size );

#endif //#ifndef _H264_DEFINE_H_
