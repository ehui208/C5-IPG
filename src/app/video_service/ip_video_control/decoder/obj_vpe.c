#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <linux/fb.h>
#include <unistd.h>
#include "obj_vpe.h"


#include "obj_h264d.h"
#include "obj_vpost.h"


/* VPE device */
char vpe_device[] = "/dev/vpe";

/*------------------------------------------------------------------------
									VPE运行	
入口:  
	 指针(Vpe使用的结构指针)
	 指针(Vpost的运行结构指针)
	 指针(Decode的运行结构指针) 


处理:
	 VPE运行函数	

出口:
		 无
------------------------------------------------------------------------*/
int FormatConversion( DECODE_OBJ_t* vpe, DECODE_OBJ_t* vpost, DECODE_OBJ_t* h264 )
{	
	VpeInfo *Vpe = (VpeInfo*)vpe->Data;
	VpostInfo *Vpost = (VpostInfo*)vpost->Data;
	H264DecInfo *H264Dec = (H264DecInfo*)h264->Data;	
	
	u_int32_t value;
	u_int32_t ptr_y, ptr_u, ptr_v;
	u_int32_t ret =0;
	u_int32_t pDstBuf;

	// 得到decoder的解碼后輸出數據指針
	ptr_y = (unsigned int)H264Dec->Profile.coded.data[0];
	ptr_u = (unsigned int)H264Dec->Profile.coded.data[1];
	ptr_v = (unsigned int)H264Dec->Profile.coded.data[2];

	#if 1
	while( ioctl (Vpe->FD, VPE_WAIT_INTERRUPT, &value) ) 
	{
		if( errno == EINTR ) 
		{
			continue;
		}
		else 
		{
			printf ("%s: VPE_WAIT_INTERRUPT failed: %s\n", __FUNCTION__, strerror (errno));
			goto fail;
		}
	}
	#endif

	if( Vpe->BuffSwitch )
	{
		Vpe->BuffSwitch		= 0;
		Vpost->Var.yoffset  	= Vpost->Var.yres;
		pDstBuf 			   	= Vpost->FBAddr;         
	}
	else
	{	
		Vpe->BuffSwitch		= 1;
		Vpost->Var.yoffset 	= 0;
		pDstBuf 				= Vpost->FBAddr + ( Vpost->BufferSize / 2 );  
	}
	
	if( ioctl( Vpost->FD, FBIOPAN_DISPLAY, &Vpost->Var) < 0 ) 
		printf( "ioctl FB_IOPAN_DISPLAY fail\n" );


	Vpe->Setting.src_addrPacY	= ptr_y;
	Vpe->Setting.src_addrU	= ptr_u;	
	Vpe->Setting.src_addrV	= ptr_v;				
	Vpe->Setting.dest_addrPac 	= (unsigned int)pDstBuf;	

		
	if( (ioctl(Vpe->FD, VPE_SET_FORMAT_TRANSFORM, &Vpe->Setting)) < 0 )		
	{
		printf("VPE_IO_GET fail\n");
		goto fail;
	}		
	

	if( (ioctl(Vpe->FD, VPE_TRIGGER, NULL)) < 0 )	
	{
		printf("VPE_TRIGGER info fail\n");
		goto fail;
	}		
	
	return ret;

	fail:
	{
		CloseVpe( vpe );

		return -1;
	}
}


/*------------------------------------------------------------------------
									关闭VPE	
入口:  
	 Vpe的运行结构指针

处理:
	关闭VPE(关闭文件, 释放内存)

出口:
		 无
------------------------------------------------------------------------*/
int CloseVpe( DECODE_OBJ_t* f  ) 
{
	VpeInfo *Vpe = (VpeInfo*)f->Data;
	
	if( Vpe->FD )
		close( Vpe->FD );
	Vpe->FD = 0;

	if( Vpe  )
		free( Vpe );
	
	return 0;
}

/*------------------------------------------------------------------------
									VPE初始化
入口:  
	  VPE自身的运行结构指针
	  Vpost的运行结构指针
	  源视频的分辨率
	  目标视频的分辨率

处理:
		创建VPE(申请内存, 打开文件, 参数配置等)

出口:
		 无
------------------------------------------------------------------------*/
int InitVpe( DECODE_OBJ_t* f, DECODE_OBJ_t* vpost, int Srcwidth, int Srcheight, int Tarwidth, int Tarheight  )
{
	int32_t value;
	int32_t ret;
	VpeInfo *Vpe  = DecodeNew0( VpeInfo, 1 );
	VpostInfo *Vpost = (VpostInfo*)vpost->Data;
	
	Vpe->FD= open( vpe_device, O_RDWR );		
	if( Vpe->FD < 0 )
	{
		printf( "open vpe device fail \n" );
		goto fail;	
	}	
	printf("%s----->%d\n",__func__,__LINE__);
	// Get Video Decoder IP Register size
	ret = ioctl( Vpe->FD, VPE_INIT, NULL ) ;
	if( ret < 0 )
	{
		printf( "VPE_INIT fail \n" );
		goto fail;
	}

	value = 0x5a;
	ret =  ioctl( Vpe->FD, VPE_IO_SET, &value ) ;
	if( ret < 0 )
	{
		printf( "VPE_IO_SET fail\n" );
		goto fail;
	}			
	
	value = 0;		
	ret =  ioctl( Vpe->FD, VPE_SET_MMU_MODE, &value ) ;
	if( ret < 0 )
	{
		printf( "VPE_SET_MMU_MODE fail\n" );
		goto fail;
	}

	Vpe->Setting.src_format	= VPE_SRC_PLANAR_YUV420;
	Vpe->Setting.src_width		= Srcwidth ;
	Vpe->Setting.src_height	= Srcheight;		
		
	Vpe->Setting.src_leftoffset 	= 0;
	Vpe->Setting.src_rightoffset	= 0;	
	Vpe->Setting.dest_width	= Vpost->Var.xres;
	Vpe->Setting.dest_height	= Vpost->Var.yres;
	Vpe->Setting.algorithm		= VPE_SCALE_DDA;
	Vpe->Setting.rotation 		= VPE_OP_NORMAL;

	if( (Vpost->Var.bits_per_pixel/8) == 4 )
		Vpe->Setting.dest_format 	= VPE_DST_PACKET_RGB888;
	else
		Vpe->Setting.dest_format 	= VPE_DST_PACKET_RGB565;
	

	f->Data = Vpe;
	
	return 0;

	fail:
	{
		CloseVpe( f );

		return -1;
	}
			
}

