/* 
    sample code for H264 for pattern 720x480 input and bitstream output
    This sample code is to do encode 1000 stream frames named "/tmp/dev0.264"
    #./h264_main test.yuv
 */

#include <unistd.h>  
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/videodev.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <errno.h>
#include <inttypes.h>

#include "favc_avcodec.h"
#include "ratecontrol.h"
#include "favc_version.h"
#include "obj_h264e.h"

#include "obj_V4L.h"


//#include "../../../../os/RTOS.h"
//#include "../../../../os/OSQ.h"




#define VIDEO_PALETTE_YUV420P_MACRO		50		/* YUV 420 Planar Macro */
	

typedef struct _H264EncInfo_
{
	int32_t FD;
	int8_t* Name;
	u_int32_t key_frame;
	u_int8_t* OutVirtBuffer;
	avc_workbuf_t OutPutBuf;
	FAVC_ENC_PARAM Param;
	H264RateControl Ratec;
}H264EncInfo;
H264EncInfo H264Enc;


/*------------------------------------------------------------------------
									封装:  内存申请
入口:  
	

处理:
	  	

出口:
		无
------------------------------------------------------------------------*/
void* H264EncMalloc0( size_t size )
{
	void* ptr = malloc( size );

	memset( ptr, 0, size );

	return ptr;
}

/*------------------------------------------------------------------------
									关闭:  H264_Encoder
入口:  
	

处理:
	  	

出口:
		无
------------------------------------------------------------------------*/
int CloseH264Enc( ENCODE_OBJ_t* f )
{
	H264EncInfo *H264Enc = (H264EncInfo*)f->Data;

	if( H264Enc->OutVirtBuffer )
		munmap( (void*)H264Enc->OutVirtBuffer, H264Enc->OutPutBuf.size );	
		
	if( H264Enc->FD )
		close( H264Enc->FD  );

	H264Enc->FD	= 0;
	
	free( H264Enc );

	return 0;
}

/*------------------------------------------------------------------------
									初始化:  H264_Encoder
入口:  
	

处理:
	  	

出口:
		无
------------------------------------------------------------------------*/
int InitH264Enc( ENCODE_OBJ_t* f, ENCODE_PROFILE_t* video_setting )
{
	H264EncInfo *H264Enc  = EncodeNew0( H264EncInfo, 1 );
	int32_t ret;

	H264Enc->FD = open( FAVC_ENCODER_DEV, O_RDWR );
	if( H264Enc->FD <= 0 )
	{
		printf( "open H264 Enc device fail \n" );
		goto fail;
	}

	//-----------------------------------------------
	//  driver handle 1
	//-----------------------------------------------    
	// Get Bitstream Buffer Information
	ret = ioctl( H264Enc->FD, FAVC_IOCTL_ENCODE_GET_BSINFO, &H264Enc->OutPutBuf );
	if( ret < 0)		
	{
		printf( "Get avc Enc bitstream info fail\n" );
		//goto fail;
	}
	printf("%s----->%d\n",__func__,__LINE__);
	printf( "H1 Get Enc BS Buffer Physical addr = 0x%x, size = 0x%x,\n", H264Enc->OutPutBuf.addr, H264Enc->OutPutBuf.size );

	H264Enc->OutVirtBuffer = mmap( NULL, H264Enc->OutPutBuf.size, PROT_READ|PROT_WRITE, MAP_SHARED, H264Enc->FD, H264Enc->OutPutBuf.offset );

	if( H264Enc->OutVirtBuffer == MAP_FAILED )
	{
		printf( "Map ENC Bitstream Buffer Failed!\n" );
		goto fail;
	}	
	//printf( "H1 Mapped ENC Bitstream Buffer Virtual addr = 0x%x\n", H264Enc->OutVirtBuffer );

	//-----------------------------------------------
	//  Issue Encode parameter to driver handle 1 & 2
	//-----------------------------------------------  	          

	memset( &H264Enc->Param, 0, sizeof(FAVC_ENC_PARAM) );

	H264Enc->Param.u32API_version 	= H264VER;

	H264Enc->Param.u32FrameWidth	= video_setting->width;
	H264Enc->Param.u32FrameHeight	= video_setting->height;

	H264Enc->Param.fFrameRate 		= video_setting->framerate;
	H264Enc->Param.u32IPInterval 	= video_setting->gop_size; // IDR 0x65输出的间隔, IPPPP.... I, next I frame interval
	H264Enc->Param.u32MaxQuant      = video_setting->qmax;
	H264Enc->Param.u32MinQuant      = video_setting->qmin;
	H264Enc->Param.u32Quant 		= video_setting->quant; //32
	H264Enc->Param.u32BitRate 		= video_setting->bit_rate;
	H264Enc->key_frame				= video_setting->key_frame;
	H264Enc->Param.ssp_output 		= 1;
	H264Enc->Param.intra 			= 1;
	H264Enc->Param.bROIEnable 		= 0;
	H264Enc->Param.u32ROIX 			= 0;
	H264Enc->Param.u32ROIY 			= 0;
	H264Enc->Param.u32ROIWidth 		= 0;
	H264Enc->Param.u32ROIHeight		= 0;
	H264Enc->Param.img_fmt			= 2;

#ifdef RATE_CTL
	memset( &H264Enc->Ratec, 0, sizeof(H264RateControl) );

	H264RateControlInit( &H264Enc->Ratec, H264Enc->Param.u32BitRate,		\
	RC_DELAY_FACTOR,RC_AVERAGING_PERIOD, RC_BUFFER_SIZE_BITRATE, 	\
	(int)H264Enc->Param.fFrameRate,									\
	(int) H264Enc->Param.u32MaxQuant, 								\
	(int)H264Enc->Param.u32MinQuant,									\
	(unsigned int)H264Enc->Param.u32Quant, 							\
	H264Enc->Param.u32IPInterval );
#endif

	ret = ioctl( H264Enc->FD, FAVC_IOCTL_ENCODE_INIT, &H264Enc->Param );
	if( ret < 0)
	{
		printf( "Handler_1 Error to set FAVC_IOCTL_ENCODE_INIT\n" );
		goto fail;
	}

	f->Data = H264Enc;
	
	return 0;

	fail:
	{
		CloseH264Enc( f );
		return -1;
	}	
}


/*------------------------------------------------------------------------
									运行:  H264_Encoder
入口:  
	

处理:
	  	

出口:
		无
------------------------------------------------------------------------*/
unsigned char cnt_ = 0;
int EncodeH264( ENCODE_OBJ_t* f, u_int8_t* frame, void* data )
{
	H264EncInfo* H264Enc = (H264EncInfo*)f->Data;
	ENCODE_FRAME_t* pict = (ENCODE_FRAME_t*)data;

	H264Enc->Param.pu8YFrameBaseAddr	= (unsigned char *)pict->data[0];   //input user continued virtual address (Y), Y=0 when NVOP
	H264Enc->Param.pu8UFrameBaseAddr	= (unsigned char *)pict->data[1];   //input user continued virtual address (U)
	H264Enc->Param.pu8VFrameBaseAddr	= (unsigned char *)pict->data[2];   //input user continued virtual address (V)

	H264Enc->Param.bitstream				= frame;  //output User Virtual address 
	H264Enc->Param.u32IPInterval 			= 0; 		// use default IPInterval that set in INIT
	H264Enc->Param.bitstream_size 		= 0;

	if( ++cnt_ >= H264Enc->key_frame )		// 输出关键帧 SPS + PPS + IDR
	{
		cnt_ = 0;
		H264Enc->Param.ssp_output 		= 1;
		H264Enc->Param.intra	 		= 1;
	}
	else									// 不输出关键帧 但间隔输出IDR
	{
		H264Enc->Param.ssp_output 		= -1;	
		H264Enc->Param.intra	 		= -1;
	}

	if( ioctl( H264Enc->FD, FAVC_IOCTL_ENCODE_FRAME, &H264Enc->Param) < 0 )
	{
		printf( "Error to set FAVC_IOCTL_ENCODE_FRAME\n" );
		return 0;
	}

#ifdef RATE_CTL
	if( H264Enc->Param.keyframe == 0 ) 
	{
	    //printf("%d %d %d\n", enc_param.u32Quant, enc_param.bitstream_size, 0);
	    H264RateControlUpdate( &H264Enc->Ratec, H264Enc->Param.u32Quant, H264Enc->Param.bitstream_size , 0 );
	} 
	else  
	{
	    //printf("%d %d %d\n", enc_param.u32Quant, enc_param.bitstream_size, 1);
	    H264RateControlUpdate( &H264Enc->Ratec, H264Enc->Param.u32Quant, H264Enc->Param.bitstream_size , 1 );
	}
	
	H264Enc->Param.u32Quant = H264Enc->Ratec.rtn_quant;

	//printf(" favc_quant = %d\n",favc_quant);

	//H264RateControlUpdate(&h264_ratec,enc_param.bitstream_size,enc_param.frame_cost);
#endif

	//video_setting->intra = H264Enc->Param.keyframe;

	return H264Enc->Param.bitstream_size;
}



#if 0
pthread_t task_H264_enc;
pthread_t task_UDP_Send;
OS_Q H264_encQ;
unsigned char data_enc[ENC_SIZE*10];

void vtk_TaskProcessEvent_H264_enc( void )
{
	int  size;
	video_profile   video_setting;
	AVFrame pict;
	unsigned int u_image_size, v_image_size;

	uint32_t PicPhyAdr;       

	ENCODE_OBJ_t H264Obj;
	ENCODE_OBJ_t V4lObj;

	

	//set the default value
	video_setting.qmax 	= 51;
	video_setting.qmin 	= 10;
#ifdef RATE_CTL    
	video_setting.quant 	= 14;               // Init Quant
#else
	video_setting.quant 	= FIX_QUANT;
#endif    
	video_setting.bit_rate 	= 1800000;//512000;
	video_setting.width 	= ENC_IMG_WIDTH;
	video_setting.height 	= ENC_IMG_HEIGHT;
	video_setting.framerate= 25;	
	video_setting.gop_size 	= IPInterval;
	video_setting.frame_rate_base = 1;

	u_image_size = video_setting.width * video_setting.height;
	v_image_size = u_image_size + ( u_image_size >> 2 );


	InitV4l( &V4lObj, video_setting.width, video_setting.height );

	InitH264Enc( &H264Obj, &video_setting );

	while( 1 )
	{
		if( !ReadV4LPicture( &V4lObj, &PicPhyAdr ) )
			TriggerV4LNextFrame( &V4lObj );
		
		pict.data[0] = (unsigned char *)PicPhyAdr;
		pict.data[1] = (unsigned char *)(PicPhyAdr + u_image_size);
		pict.data[2] = (unsigned char *)(PicPhyAdr + v_image_size);  	    

		size = EncodeH264( &H264Obj,  (unsigned char *)Encbitsteam,(void *)&pict );

		if( OS_Q_Put( &H264_encQ, Encbitsteam, size ) )
			printf( " API_H264_enc is full \n");
	}

	CloseV4LDevice( &V4lObj );
	CloseH264Enc( &H264Obj );
	
}

void vtk_TaskProcessEvent_UDP_Send( void )
{
	unsigned char* msgH264_enc;
	unsigned char data[ENC_SIZE] = {0};
	int size = 0;

	OS_Q_Create( &H264_encQ, &data_enc, sizeof(data_enc) );
	
	struct sockaddr_in addr;
	int sockfd = -1;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	bzero( &addr, sizeof(addr) );
	
	addr.sin_family 		= AF_INET;
	addr.sin_addr.s_addr 	= inet_addr("192.168.10.151");;
	addr.sin_port 			= htons(25050);

	while( 1 )
    	{	    					
    		size = OS_Q_GetPtr( &H264_encQ,  (void*)&msgH264_enc );	//Get message
		
		memcpy( data, msgH264_enc, size );
		
		if( sendto(sockfd, data, size, 0, (struct sockaddr*)&addr,sizeof(addr))   == -1 )
		{
			printf("can not send data from socket! errno:%d,means:%s\n",errno,strerror(errno));
		}

		OS_Q_Purge( &H264_encQ );
    	}
}


void vtk_TaskInit_H264_enc( unsigned char priority )
{
	//create task
	if( pthread_create(&task_H264_enc,NULL,(void*)vtk_TaskProcessEvent_H264_enc,NULL) != 0 )
	{
		printf( "Create pthread error! \n" );
		exit(1);
	}
	
	if( pthread_create(&task_UDP_Send,NULL,(void*)vtk_TaskProcessEvent_UDP_Send,NULL) != 0 )
	{
		printf( "Create pthread error!n" );
		exit(1);
	}

	pthread_join( task_H264_enc, NULL );
}

#endif

