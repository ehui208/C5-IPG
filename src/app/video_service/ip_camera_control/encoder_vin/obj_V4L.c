/* V4L.c
 *
 *
 * Copyright (c)2008 Nuvoton technology corporation
 * http://www.nuvoton.com
 *
 * video for linux function
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
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

#include "obj_V4L.h"
#include "obj_h264e.h"

#define VID_DEVICE "/dev/video0"

#define VIDIOCGCAPTIME					_IOR('v',30,struct v4l2_buffer)      /*Get Capture time */
#define VIDIOCSPREVIEW                  			_IOR('v',43, int)

#define VIDEO_START						0
#define VIDEO_STOP						1

#define VIDEO_PALETTE_YUV420P_MACRO		50		/* YUV 420 Planar Macro */


typedef struct _V4lInfo_
{
	int32_t FD;
	int8_t* Name;
	int        initV4L;
	int32_t FrameFmt; /* see VIDEO_PALETTE_xxx */
	int32_t UseMMap;
	int32_t FrameSize;
	int32_t Frame; //gb_frame;	
	uint8_t *pu8VidBuf;

	uint64_t TimeStamp;
	//uint32_t PicPhyAdr;       
	
	struct video_window Win;
	struct video_mbuf MBufs; // gb_buffers;
	struct video_mmap MMap; //gb_buf;

	struct video_capability Capab;
	struct video_picture Pict;

} V4lInfo;
V4lInfo V4l;

static uint64_t s_u64PrevTS = 0;


/*------------------------------------------------------------------------
									子初始化(V4L)
入口:  
	

处理:
	  	打开文件; 配置参数; 映射视频数据帧的地址指针

出口:
		无
------------------------------------------------------------------------*/
int32_t InitV4LDevice( ENCODE_OBJ_t* f, int32_t Width, int32_t Height, int initv4l )
{
	u_int32_t ret;
	int32_t val;
	V4lInfo *V4l = NULL;
	
	if( initv4l )
	{
		V4l  = EncodeNew0( V4lInfo, 1 );

		V4l->FD = open( VID_DEVICE, O_RDWR );
		if( V4l->FD < 0 ) 
		{
			printf( "Can not open video device \n" );
			goto fail;
		}

		ret = ioctl( V4l->FD, VIDIOCGCAP, &V4l->Capab );
		if( ret < 0) 
		{
			printf( " VIDIOCGCAP error \n" );
			goto fail;
		}

		if( !(V4l->Capab.type & VID_TYPE_CAPTURE) ) 
		{
			printf( "Fatal: grab device does not handle capture\n");
			goto fail;
		}

		ret = ioctl( V4l->FD, VIDIOCGPICT, &V4l->Pict );
		if( ret < 0) 
		{
			printf( " VIDIOCGPICT error \n" );
			goto fail;
		}

		V4l->Pict.palette	= VIDEO_PALETTE_YUV420P;
		V4l->Pict.depth	= 16;

		ret = ioctl( V4l->FD, VIDIOCSPICT, &V4l->Pict );
		if( ret < 0) 
		{
			printf( " VIDIOCSPICT error \n" );
			goto fail;
		}

		ret = ioctl( V4l->FD, VIDIOCGMBUF, &V4l->MBufs );
		if( ret < 0 ) 
		{
			/* try to use read based access */
			V4l->Win.width		= Width;
			V4l->Win.height 		= Height;
			V4l->Win.x 			= 0;
			V4l->Win.y 			= 0;
			V4l->Win.chromakey 	= -1;
			V4l->Win.flags 		= 0;

			ret = ioctl( V4l->FD, VIDIOCSWIN, &V4l->Win );
			if( ret < 0 ) 
			{
				printf( "VIDIOCSPICT error \n" );
				goto fail;
			}

			V4l->FrameFmt = VIDEO_PALETTE_YUV420P;

			val = 1;
			ret = ioctl( V4l->FD, VIDIOCCAPTURE, &val );
			if( ret < 0 ) 
			{
				printf( "VIDIOCCAPTURE error \n" );
				goto fail;
			}
			
			V4l->UseMMap = 0;
		} 
		else 
		{
			V4l->pu8VidBuf = mmap( 0, V4l->MBufs.size, PROT_READ|PROT_WRITE, MAP_SHARED, V4l->FD, 0 );
			if( (unsigned char*)-1 == V4l->pu8VidBuf ) 
			{
				V4l->pu8VidBuf = mmap( 0, V4l->MBufs.size, PROT_READ|PROT_WRITE, MAP_PRIVATE, V4l->FD, 0 );
				if( (unsigned char*)-1 == V4l->pu8VidBuf ) 
				{
					printf( " mmap error \n" );
					goto fail;
				}
			}
			
			V4l->Frame 			= 0;		// 一次只采集一帧

			/* start to grab the first frame */
			V4l->MMap.frame		= V4l->Frame % V4l->MBufs.frames;
			V4l->MMap.height 	= Height;
			V4l->MMap.width 		= Width;
			V4l->MMap.format	= VIDEO_PALETTE_YUV420P;

			ret = ioctl( V4l->FD, VIDIOCMCAPTURE, &V4l->MMap );		// 采集图像
			if( ret < 0 ) 
			{
				if( errno != EAGAIN ) 
					printf( "VIDIOCMCAPTURE error \n" );
				else 
					printf( "Fatal: grab device does not receive any video signal\n" ) ;

				goto fail;
			}
			V4l->FrameFmt	= V4l->MMap.format;
			V4l->UseMMap	= 1;
		}
		
		V4l->FrameSize = V4l->MMap.width  * V4l->MMap.height * 2;

		f->Data = V4l;		
	}
	else
	{
		V4l = (V4lInfo*)f->Data;

		if( V4l->UseMMap )
			munmap( V4l->pu8VidBuf, V4l->MBufs.size);

		ret = ioctl( V4l->FD, VIDIOCGMBUF, &V4l->MBufs );
		if( ret < 0 ) 
		{
			/* try to use read based access */
			V4l->Win.width		= Width;
			V4l->Win.height 		= Height;
			V4l->Win.x			= 0;
			V4l->Win.y			= 0;
			V4l->Win.chromakey	= -1;
			V4l->Win.flags		= 0;
		
			ret = ioctl( V4l->FD, VIDIOCSWIN, &V4l->Win );
			if( ret < 0 ) 
			{
				printf( "VIDIOCSPICT error \n" );
				goto fail;
			}
		
			V4l->FrameFmt = VIDEO_PALETTE_YUV420P;
		
			val = 1;
			ret = ioctl( V4l->FD, VIDIOCCAPTURE, &val );
			if( ret < 0 ) 
			{
				printf( "VIDIOCCAPTURE error \n" );
				goto fail;
			}
			
			V4l->UseMMap = 0;
		} 
		else 
		{
			V4l->pu8VidBuf = mmap( 0, V4l->MBufs.size, PROT_READ|PROT_WRITE, MAP_SHARED, V4l->FD, 0 );
			if( (unsigned char*)-1 == V4l->pu8VidBuf ) 
			{
				V4l->pu8VidBuf = mmap( 0, V4l->MBufs.size, PROT_READ|PROT_WRITE, MAP_PRIVATE, V4l->FD, 0 );
				if( (unsigned char*)-1 == V4l->pu8VidBuf ) 
				{
					printf( " mmap error \n" );
					goto fail;
				}
			}
			
			V4l->Frame			= 0;
		
			/* start to grab the first frame */
			V4l->MMap.frame 	= V4l->Frame % V4l->MBufs.frames;
			V4l->MMap.height	= Height;
			V4l->MMap.width 		= Width;
			V4l->MMap.format	= VIDEO_PALETTE_YUV420P;
		
			ret = ioctl( V4l->FD, VIDIOCMCAPTURE, &V4l->MMap );
			if( ret < 0 ) 
			{
				if( errno != EAGAIN ) 
					printf( "VIDIOCMCAPTURE error \n" );
				else 
					printf( "Fatal: grab device does not receive any video signal\n" ) ;
		
				goto fail;
			}
			V4l->FrameFmt	= V4l->MMap.format;
			V4l->UseMMap	= 1;
		}
		
		V4l->FrameSize = V4l->MMap.width  * V4l->MMap.height * 2;
		
		f->Data = V4l;		
		
	}

	return 0;
	
	fail:
	{
		CloseV4LDevice( f );
		return -1;
	}
}

/*------------------------------------------------------------------------
									(V4L)组织一帧视频数据 __ 对外
入口:  
	

处理:
			组织一帧数据,提供帧数据指针	  	

出口:
		无
------------------------------------------------------------------------*/
int32_t ReadV4LPicture( ENCODE_OBJ_t* f, u_int32_t *pu32BufPhyAddr )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	int32_t i32TryCnt = 0;
	unsigned char *pchVidInBuf;
	struct v4l2_buffer sV4L2Buff;

	
	while( ioctl( V4l->FD, VIDIOCSYNC, &V4l->Frame ) < 0 && ( errno == EAGAIN || errno == EINTR ) )
	{
		usleep( 10000 );
		i32TryCnt ++;
		if( i32TryCnt >= 100 )
		{
			printf( " V4L fail\n" );
			return -1;
		}
	}		

	sV4L2Buff.index = V4l->Frame;

	ioctl( V4l->FD, VIDIOCGCAPTIME, &sV4L2Buff );
#if 1
	V4l->TimeStamp	= (u_int64_t)sV4L2Buff.timestamp.tv_sec * 1000000 + (u_int64_t)sV4L2Buff.timestamp.tv_usec;
	if( s_u64PrevTS != 0 )
	{
		if( (V4l->TimeStamp - s_u64PrevTS) > 300000 )
			printf( "V4L get raw picture over %"PRId64" us\n", (V4l->TimeStamp - s_u64PrevTS) );
	}
	s_u64PrevTS = V4l->TimeStamp;		
#endif

	*pu32BufPhyAddr = sV4L2Buff.m.userptr;	/* encode physical address */
	// 得到该帧相对于mmap的映射地址的偏移量
	pchVidInBuf = V4l->pu8VidBuf + V4l->MBufs.offsets[V4l->Frame];
	
#if defined (STATISTIC)

	if(s_u32VidInFrames == 0){
		s_u32VidInFirstTS = s_u64PrevTS;
	}

	s_u32VidInFrames ++;
	
	if(s_u32VidInFrames == 300)
	{
		printf( "Vin frame rate %"PRId64" fps \n", s_u32VidInFrames/((s_u64PrevTS - s_u32VidInFirstTS)/1000000) );
		s_u32VidInFrames = 0;
	}
#endif

	return 0;
}

/*------------------------------------------------------------------------
								(V4L)触发_准备下一帧数据 _ 对外
入口:  
	

处理:
			

出口:
		无
------------------------------------------------------------------------*/
int32_t TriggerV4LNextFrame( ENCODE_OBJ_t* f )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	int32_t i32TryCnt = 0;
	int32_t ret;

	while( ioctl( V4l->FD, VIDIOCSYNC, &V4l->Frame ) < 0 && ( errno == EAGAIN || errno == EINTR ) )
	{
		usleep( 10000 );

		if( ++i32TryCnt >= 100 )
		{
			printf( "TriggerV4LNextFrame: V4L fail\n" );
			return -1;
		}
	}
	
	V4l->MMap.frame = V4l->Frame;

	ret = ioctl( V4l->FD, VIDIOCMCAPTURE, &V4l->MMap );	// 触发捕获数据帧
	if( ret < 0 ) 
	{
		if( errno == EAGAIN )
			printf( "Cannot Sync\n" );
		else
			printf( "VIDIOCMCAPTURE\n" );
		return -1;
	}

	/* This is now the grabbing frame */
	V4l->Frame = (V4l->Frame + 1) % V4l->MBufs.frames;

	return 0;
}

/*------------------------------------------------------------------------
								(V4L)启动帧数据组织
入口:  
	

处理:
			

出口:
		无
------------------------------------------------------------------------*/
int32_t StartV4LCapture( ENCODE_OBJ_t* f )
{	
	V4lInfo* V4l = (V4lInfo*)f->Data;
	int32_t val = VIDEO_START;
	int32_t ret;

	ret = ioctl( V4l->FD, VIDIOCCAPTURE, &val );
	if( ret < 0 )
		return -1;
	
	V4l->Frame = 0;
	return 0;
}

/*------------------------------------------------------------------------
								(V4L)关闭帧数据组织
入口:  
	

处理:
			

出口:
		无
------------------------------------------------------------------------*/
int32_t StopV4LCapture( ENCODE_OBJ_t* f )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	int32_t val = VIDEO_STOP;
	int32_t ret;

	ret = ioctl( V4l->FD, VIDIOCCAPTURE, &val );
	if( ret < 0 ) 
		return -1;
	
	return 0;
}

/*------------------------------------------------------------------------
								(V4L)关闭__对外
入口:  
	

处理:
			

出口:
		无
------------------------------------------------------------------------*/
void CloseV4LDevice( ENCODE_OBJ_t* f )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;

//	StopV4LCapture( f );
//	StopV4LCapture( f );	
	
	if( V4l->UseMMap )
		munmap( V4l->pu8VidBuf, V4l->MBufs.size);

	if( V4l->FD )
		close( V4l->FD );	
	V4l->FD = 0;

	if( V4l )
		free( V4l );
}

/*------------------------------------------------------------------------
								(V4L)初始化及启动__对外
入口:  
	

处理:
			

出口:
		无
------------------------------------------------------------------------*/
int InitV4l( ENCODE_OBJ_t* f, int32_t Width, int32_t Height, int32_t format, int init_v4l )
{
//	int32_t i;
//	int32_t ret;
	//u_int8_t* pu8PicPtr;
	//u_int64_t u64TS;
//	u_int32_t u32PicPhyAdr;

	if( InitV4LDevice( f, Width, Height,  init_v4l ) < 0 )
		return -1;

	//StartPreview(  f );

//	StopV4LCapture(  f );

	if( format )
		SetV4LEncode( f, 0, Width, Height, VIDEO_PALETTE_YUV420P );
	else
		SetV4LEncode( f, 0, Width, Height, VIDEO_PALETTE_YUV420P_MACRO );

	//SetV4LViewWindow(  f, Width, Height, Width, Height );

	StartV4LCapture( f );

#if 0
	// 10->OK, 4-> xOK, 3-> Fail
	for( i = 0; i < 2; i++ )    
	{
		ret = ReadV4LPicture( f, &u32PicPhyAdr );
		if( !ret )
			TriggerV4LNextFrame( f );	
		else
		    printf( "V4L read fail\n" );
	}
#endif
	return 0;
}

/*------------------------------------------------------------------------

------------------------------------------------------------------------*/
int32_t StartPreview( ENCODE_OBJ_t* f  )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	int32_t ret;
	int32_t val = 1;

	ret = ioctl( V4l->FD, VIDIOCSPREVIEW, &val );
	if( ret < 0 ) 
		return -1;
	
	return 0;
}

/*------------------------------------------------------------------------

------------------------------------------------------------------------*/
int32_t StopPreview( ENCODE_OBJ_t* f )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	int32_t ret;
	int32_t val = 0;

	ret = ioctl( V4l->FD, VIDIOCSPREVIEW, &val );
	if( ret < 0 ) 
		return -1;

	return 0;
}

/*------------------------------------------------------------------------

------------------------------------------------------------------------*/
int32_t SetV4LViewWindow( ENCODE_OBJ_t* f, int32_t lcmwidth, int32_t lcmheight, int32_t prewidth, int32_t preheight)
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	int32_t ret;
	
	V4l->Win.width 		= prewidth;
	V4l->Win.height 		= preheight;
	V4l->Win.x 			= (lcmwidth-prewidth)/2;		//Depend on the VPOST 
	V4l->Win.y 			= (lcmheight-preheight)/2;		
	V4l->Win.chromakey 	= -1;
	V4l->Win.flags 		= 0;
	
	printf( "User Packet View Win Width , Height = %d, %d\n", V4l->Win.width, V4l->Win.height );
	printf( "User Packet View Win Position X , Y = %d, %d\n", V4l->Win.x, V4l->Win.y );

	ret = ioctl( V4l->FD, VIDIOCSWIN, &V4l->Win );
	if( ret < 0) 
	{
		printf( "User VIDIOCSWIN error\n" );
		goto fail;
	}
	return 0;
	
	fail:
	{
		CloseV4LDevice( f );
		return -1;
	}
}

/*------------------------------------------------------------------------

------------------------------------------------------------------------*/
int32_t SetV4LEncode( ENCODE_OBJ_t* f, int32_t frame, int32_t width, int32_t height, int32_t palette )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	int32_t ret;

	V4l->MMap.frame		= frame;
	V4l->MMap.format	= palette;
	V4l->MMap.height 	= height;
	V4l->MMap.width 		= width;
	V4l->FrameSize		= V4l->MMap.width  * V4l->MMap.height * 2;

	ret = ioctl( V4l->FD, VIDIOCMCAPTURE, &V4l->MMap );
	if( ret < 0 ) 
	{
		if (errno == EAGAIN)
			printf( " Cannot Sync\n" );
		else
			printf(" VIDIOCMCAPTURE \n" );
		return -1;
	}
	return 0;
}

/*------------------------------------------------------------------------

------------------------------------------------------------------------*/
int32_t QueryV4LZoomInfo(  ENCODE_OBJ_t* f, struct v4l2_cropcap *psVideoCropCap, struct v4l2_crop *psVideoCrop )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	int32_t err = 0;

	if (0 != ioctl( V4l->FD, VIDIOC_CROPCAP, psVideoCropCap)) {//Get cropping window capacity 
		printf("VIDIOC_CROPCAP fail\n");
		return -1; 
	}
	
	if (0 != ioctl( V4l->FD, VIDIOC_G_CROP, psVideoCrop)) {//Get current cropping window
			printf("VIDIOC_G_CROP fail\n");
			return -1; 
	}
	return err;
}

#define BRIGNESS	1
#define CONTRAST	2
#define COLOR		3

int32_t SetSensorBrigness( ENCODE_OBJ_t* f, int32_t ctrl )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	struct v4l2_control control;
	
	control.id = BRIGNESS;
	//control.value = ctrl * 12;
	control.value = ctrl;
	
	if( 0 != ioctl( V4l->FD, VIDIOC_S_CTRL, &control)) 
	{
		printf("VIDIOC_S_CTRL fail\n");
		return -1; 
	}

	return 1;	
}

int32_t GetSensorBrigness( ENCODE_OBJ_t* f, int32_t* ctrl )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	struct v4l2_control control;
	
	control.id = BRIGNESS;
	
	if( 0 != ioctl( V4l->FD, VIDIOC_G_CTRL, &control)) 
	{
		printf("VIDIOC_G_CTRL fail\n");
		return -1; 
	}

	//*ctrl = (control.value/12);
	*ctrl = control.value;
	
	return 1;	
}

int32_t SetSensorContrast( ENCODE_OBJ_t* f, int32_t ctrl )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	struct v4l2_control control;
	
	control.id = CONTRAST;
	//control.value = ctrl * 12;
	control.value = ctrl;
	
	if( 0 != ioctl( V4l->FD, VIDIOC_S_CTRL, &control)) 
	{
		printf("VIDIOC_S_CTRL fail\n");
		return -1; 
	}

	return 1;	
}

int32_t GetSensorContrast( ENCODE_OBJ_t* f, int32_t* ctrl )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	struct v4l2_control control;
	
	control.id = CONTRAST;
	
	if( 0 != ioctl( V4l->FD, VIDIOC_G_CTRL, &control)) 
	{
		printf("VIDIOC_G_CTRL fail\n");
		return -1; 
	}

	//*ctrl = (control.value/12);
	*ctrl = control.value;
	
	return 1;	
}

int32_t SetSensorColor( ENCODE_OBJ_t* f, int32_t ctrl )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	struct v4l2_control control;
	
	control.id = COLOR;
	//control.value = ctrl * 25;
	control.value = ctrl;

	if( 0 != ioctl( V4l->FD, VIDIOC_S_CTRL, &control)) 
	{
		printf("VIDIOC_S_CTRL fail\n");
		return -1; 
	}

	return 1;	
}

int32_t GetSensorColor( ENCODE_OBJ_t* f, int32_t* ctrl )
{
	V4lInfo* V4l = (V4lInfo*)f->Data;
	struct v4l2_control control;
	
	control.id = COLOR;
	
	if( 0 != ioctl( V4l->FD, VIDIOC_G_CTRL, &control)) 
	{
		printf("VIDIOC_G_CTRL fail\n");
		return -1; 
	}

	//*ctrl = (control.value/25);
	*ctrl = control.value;
	
	return 1;	
}

#if 0


#define MAX_USER_CTRL 	16
static unsigned int ctrl_id = 0;
struct v4l2_queryctrl PrivateUserCtrl[MAX_USER_CTRL];

ERRCODE
QueryV4LUserControl(void)
{
	struct v4l2_queryctrl queryctrl;
	ERRCODE err = ERR_V4L_SUCCESS;
	unsigned int id;

	memset (&queryctrl, 0, sizeof (queryctrl));
	printf ("QueryV4LUserControl standard start \n");
	for (id = V4L2_CID_BASE;
		id < V4L2_CID_LASTP1;
		id=id+1) {
			queryctrl.id =id;
			printf ("QueryV4LUserControl %d \n", queryctrl.id);
			if (0 == ioctl (s_sVidData.i32VidFD, VIDIOC_QUERYCTRL, &queryctrl)) {
				if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED){
					printf ("V4L2_CTRL_FLAG_DISABLED %d \n", queryctrl.id);
					continue;
				}
			}else {
				if (errno == EINVAL){
					printf ("EINVAL %d \n", queryctrl.id);
					continue;
				}	
				printf ("VIDIOC_QUERYCTRL");
				err = ERR_V4L_VID_QUERY_UID;	
			}
	}
	printf ("QueryV4LUserControl private start \n");
	for (id = V4L2_CID_PRIVATE_BASE; ;id=id+1) {	
			queryctrl.id =id;
			if (0 == ioctl (s_sVidData.i32VidFD, VIDIOC_QUERYCTRL, &queryctrl)) {
				if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
					continue;	
				if(ctrl_id>MAX_USER_CTRL){
					err = ERR_V4L_VID_QUERY_UID;	
					break;
				}					
				memcpy(&PrivateUserCtrl[ctrl_id], &queryctrl, sizeof(queryctrl)); 
				printf ("********************************************\n");
				printf ("id %d\n", PrivateUserCtrl[ctrl_id].id);
				printf ("name %s\n", PrivateUserCtrl[ctrl_id].name);
				printf ("minimum %d\n", PrivateUserCtrl[ctrl_id].minimum);
				printf ("maximum %d\n", PrivateUserCtrl[ctrl_id].maximum);	
				printf ("step %d\n", PrivateUserCtrl[ctrl_id].step);	
				ctrl_id = ctrl_id +1;
				
			} else {
				if (errno == EINVAL)
					break;
				printf("VIDIOC_QUERYCTRL");
				err = ERR_V4L_VID_QUERY_UID;	
			}
	}
	return err;
}
/*
	OV7725 Sensor as example 
	Contrast/Brightness Enable : 0xA6[2]  = REG_SDE_CTRL
	Brightness control register : 0x9B[7:0] = REG_BRIGHT
*/
static  struct v4l2_queryctrl* ctrl_by_name(char* name)
{
	unsigned int i;

	for (i = 0; i < MAX_USER_CTRL; i++)
		if( strcmp((char*)PrivateUserCtrl[i].name, name)==0 )
			return PrivateUserCtrl+i;
	return NULL;
}

ERRCODE
ChangeV4LUserControl_Brigness(int32_t ctrl)
{
	ERRCODE err = ERR_V4L_SUCCESS;
	struct v4l2_control control;
	struct v4l2_queryctrl* 		i2c_set_addr_ctrl;
	struct v4l2_queryctrl* 		i2c_write;
	struct v4l2_queryctrl* 		i2c_read;
	printf("!!!Remmember that the function only works for OV7725 Sesnor\n");
	
	i2c_set_addr_ctrl = ctrl_by_name("i2c_set_addr");
	i2c_write = ctrl_by_name("i2c_write");
	i2c_read= ctrl_by_name("i2c_read");
	
	/* Enable Brightness Adjustment */
	control.id = i2c_set_addr_ctrl->id;
	control.value = REG_SDE_CTRL;
	if (0 != ioctl (s_sVidData.i32VidFD, VIDIOC_S_CTRL, &control)) {//Set I2C Reg Addr
		printf("VIDIOC_G_CTRL fail\n");
		return -1; 
	}
	control.id = i2c_read->id;
	if (0 != ioctl (s_sVidData.i32VidFD, VIDIOC_G_CTRL, &control)) {//Read back 
		printf("VIDIOC_G_CTRL fail\n");
		return -1; 
	}
	control.id = i2c_write->id;
	control.value = control.value | 0x04;
	if (0 != ioctl (s_sVidData.i32VidFD, VIDIOC_S_CTRL, &control)) {//Enable brightness/constrast fail
		printf("VIDIOC_S_CTRL fail\n");
		return -1; 
	}

	/*  Brightness Ajustment */
	control.id = i2c_set_addr_ctrl->id;
	control.value =  REG_BRIGHT;
	if (0 != ioctl (s_sVidData.i32VidFD, VIDIOC_S_CTRL, &control)) {//Set I2C Reg Addr
		printf("VIDIOC_G_CTRL fail\n");
		return -1; 
	}
	control.id = i2c_read->id;
	if (0 != ioctl (s_sVidData.i32VidFD, VIDIOC_G_CTRL, &control)) {
		printf("VIDIOC_G_CTRL fail\n");
		return -1; 
	}
	control.id = i2c_write->id;	
	if(ctrl>=0){	
		control.value = control.value+1;
		if (0 != ioctl (s_sVidData.i32VidFD, VIDIOC_S_CTRL, &control)) {//Enable brightness/constrast fail 
			printf("VIDIOC_S_CTRL fail\n");
			return -1; 
		}
	}else{
		control.value = control.value-1;
		if (0 != ioctl (s_sVidData.i32VidFD, VIDIOC_S_CTRL, &control)) {//Enable brightness/constrast fail 
			printf("VIDIOC_S_CTRL fail\n");
			return -1; 
		}
	}	
	return err;	
}



ERRCODE
QueryV4LZoomInfo(struct v4l2_cropcap *psVideoCropCap, 
			struct v4l2_crop *psVideoCrop)
{
	ERRCODE err = ERR_V4L_SUCCESS;

	if (0 != ioctl (s_sVidData.i32VidFD, VIDIOC_CROPCAP, psVideoCropCap)) {//Get cropping window capacity 
		printf("VIDIOC_CROPCAP fail\n");
		return -1; 
	}
	
	if (0 != ioctl (s_sVidData.i32VidFD, VIDIOC_G_CROP, psVideoCrop)) {//Get current cropping window
			printf("VIDIOC_G_CROP fail\n");
			return -1; 
	}
	return err;
}


ERRCODE
Zooming(struct v4l2_cropcap *psVideoCropCap, 
	struct v4l2_crop *psVideoCrop,
	int32_t ctrl)
{
	ERRCODE err = ERR_V4L_SUCCESS;

	if (QueryV4LZoomInfo(psVideoCropCap, psVideoCrop)!=ERR_V4L_SUCCESS)
	{
		printf("Query zoom info fail\n");
			return -1; 	
	}

	if(ctrl==1)
	{//zoom in 				
		if( ((psVideoCrop->c.width - psVideoCropCap->pixelaspect.denominator) >= psVideoCropCap->defrect.width/8) &&
		     ((psVideoCrop->c.height - psVideoCropCap->pixelaspect.numerator) >= psVideoCropCap->defrect.height/8) )
		{//Limits the JPEG upscale to 8X.
			printf("Zooming in\n");	
			psVideoCrop->c.width = psVideoCrop->c.width - psVideoCropCap->pixelaspect.denominator;
			psVideoCrop->c.height = psVideoCrop->c.height - psVideoCropCap->pixelaspect.numerator;
			psVideoCrop->c.left = psVideoCrop->c.left + psVideoCropCap->pixelaspect.denominator/2;
			psVideoCrop->c.top = psVideoCrop->c.top + psVideoCropCap->pixelaspect.numerator/2;
			if (0 != ioctl (s_sVidData.i32VidFD, VIDIOC_S_CROP, psVideoCrop)) {//Set current cropping window
				printf("VIDIOC_S_CROP fail\n");
				return -1; 
			}	
		}		
	}
	else 
	{//zoom out
		if( ((psVideoCrop->c.width + psVideoCropCap->pixelaspect.denominator) <= psVideoCropCap->defrect.width) &&
		     ((psVideoCrop->c.height + psVideoCropCap->pixelaspect.numerator) <= psVideoCropCap->defrect.height) ) 
		{//cropping dimension must be less or equal defrect. 
			printf("Zooming out\n");
			psVideoCrop->c.width = psVideoCrop->c.width + psVideoCropCap->pixelaspect.denominator;
			psVideoCrop->c.height = psVideoCrop->c.height + psVideoCropCap->pixelaspect.numerator;
			psVideoCrop->c.left = psVideoCrop->c.left - psVideoCropCap->pixelaspect.denominator/2;
			psVideoCrop->c.top = psVideoCrop->c.top - psVideoCropCap->pixelaspect.numerator/2;
			if (0 != ioctl (s_sVidData.i32VidFD, VIDIOC_S_CROP, psVideoCrop)) {//Set current cropping window
				printf("VIDIOC_S_CROP fail\n");
				return -1; 
			}	
		}		
	}
	printf("cropcap.defrect.left =%d\n",psVideoCropCap->defrect.left);			/* default window information */
	printf("cropcap.defrect.top =%d\n",psVideoCropCap->defrect.top);
	printf("cropcap.defrect.width =%d\n",psVideoCropCap->defrect.width);
	printf("cropcap.defrect.height =%d\n",psVideoCropCap->defrect.height);
	printf("cropcap.pixelaspect.numerator =%d\n",psVideoCropCap->pixelaspect.numerator);	/* constraint for step in y direct */
	printf("cropcap.pixelaspect.denominator =%d\n",psVideoCropCap->pixelaspect.denominator);/* constraint for step in x direct */

	printf("crop.c.left =%d\n",psVideoCrop->c.left);
	printf("crop.c.top =%d\n",psVideoCrop->c.top);
	printf("crop.c.width =%d\n",psVideoCrop->c.width);
	printf("crop.c.height =%d\n",psVideoCrop->c.height);
	
	return err;
}
#endif

#if 0
void vin_exit(void)
{
	FinializeV4LDevice();
	munmap(pu8FBBufAddr, u32FBBufSize);

	ioctl(i32FBFd, VIDEO_FORMAT_CHANGE, DISPLAY_MODE_RGB565);		
	close(i32FBFd);
	close(i32KpdFd);
}
#endif



