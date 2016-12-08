#ifndef _H264E_DEFINE_H_
#define _H264E_DEFINE_H_

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

#define EncodeNew0( type, count )  (type*)H264EncMalloc0( sizeof(type)*count )

void* H264EncMalloc0( size_t size );


//-------------------------------------
// Decoder part
//-------------------------------------
#define FAVC_DECODER_DEV  		"/dev/w55fa92_264dec"

//-------------------------------------
// Encoder part
//-------------------------------------
#define FAVC_ENCODER_DEV  		"/dev/w55fa92_264enc"

#define RATE_CTL

#define TEST_ROUND				300
#define FIX_QUANT				0
#define IPInterval					31

#define ENC_IMG_WIDTH   			640//320//1280//640
#define ENC_IMG_HEIGHT  			480//240//720//480


//-------------------------------------
// Data structure
//-------------------------------------
typedef struct
{
	void* Data;
}ENCODE_OBJ_t;

typedef struct
{
    u_int8_t *data[4];
} ENCODE_FRAME_t;

typedef struct
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
	u_int32_t key_frame;	
//    u_int32_t intra;
//    ENCODE_FRAME_t *coded_frame;
//    int8_t *priv;
} ENCODE_PROFILE_t;

//void vtk_TaskInit_H264_enc( unsigned char priority );
int EncodeH264( ENCODE_OBJ_t* f, u_int8_t *frame, void * data );
int InitH264Enc( ENCODE_OBJ_t* f, ENCODE_PROFILE_t *video_setting );
int CloseH264Enc( ENCODE_OBJ_t* f );



#endif //#ifndef _H264_DEFINE_H_
