
#include "obj_vpost.h"

char fbdevice[] = "/dev/fb0";

#define IOCTL_LCD_GET_DMA_BASE     		_IOR('v', 32, unsigned int *)
#define VIDEO_FORMAT_CHANGE			_IOW('v', 50, unsigned int)	//frame buffer format change

/*------------------------------------------------------------------------
							关闭Vpost	
入口:  
	 指针(Vpost的运行结构指针) 

处理:
	1.  请屏
	2.  解除映射关系
	3.  关闭文件
	4.  释放内存    

出口:
		 无
------------------------------------------------------------------------*/
int CloseVpost( DECODE_OBJ_t* f )
{
	VpostInfo *Vpost = (VpostInfo*)f->Data;
	
	memset( Vpost->VideoBuffer, 0, Vpost->BufferSize );
	
	if( Vpost->VideoBuffer ) 
	{
		munmap( Vpost->VideoBuffer, Vpost->BufferSize);
		Vpost->VideoBuffer = NULL;
	}
	
	Vpost->FBAddr = 0;

	if( Vpost->FD )
		close( Vpost->FD  );
	
	Vpost->FD = 0;

	if( Vpost )
		free( Vpost );
	
	return 0;
}

/*------------------------------------------------------------------------
							Vpost初始化
入口:  
	 指针(Vpost的运行结构指针) 

处理:
	1.  申请内存(Vpost的运行结构)
	2.  打开文件: fb0
	3.  读取FB信息(显存大小, 分辨率)
	4.  映射: 显存指针    

出口:
		 无
------------------------------------------------------------------------*/
int InitVpost( DECODE_OBJ_t* f )
{
	VpostInfo *Vpost  = DecodeNew0( VpostInfo, 1 );
	
	int32_t ret = 0;
	
	Vpost->FD = open( fbdevice, O_RDWR );
	if( Vpost->FD == -1 ) 
	{
		printf( "open vpost device fail \n" );
		goto fail;
	}
	
	ret = ioctl( Vpost->FD, FBIOGET_VSCREENINFO, &Vpost->Var ) ;
	if( ret < 0 ) 
	{
		printf( "ioctl FBIOGET_VSCREENINFO fail \n" );
		goto fail;
	}

	printf( "FB ---> width =%d, height = %d, bpp=%d\n", Vpost->Var.xres, Vpost->Var.yres, (Vpost->Var.bits_per_pixel/8) );	
	
    	// Get FB Physical Buffer address  得到lcd驱动器的fifo缓存的dma目标物理地址，通过mmap映射到用户层
    	ret = ioctl( Vpost->FD, IOCTL_LCD_GET_DMA_BASE, &Vpost->FBAddr ) ;
	if( ret < 0 ) 
	{
		printf( "ioctl IOCTL_LCD_GET_DMA_BASE fail \n");
		goto fail;
	}

	Vpost->BufferSize = Vpost->Var.xres * Vpost->Var.yres * (Vpost->Var.bits_per_pixel / 8) * 2;

	// lcd驱动器的fifo缓存的映射地址，可以用户层读写操作
	Vpost->VideoBuffer = mmap( NULL, Vpost->BufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, Vpost->FD, 0 );

	if( Vpost->VideoBuffer == MAP_FAILED )
	{
		printf( "LCD Video Map Failed!\n" );
		goto fail;
	}

	memset( Vpost->VideoBuffer, 0, Vpost->BufferSize );

	f->Data = Vpost;

	// lzh_20160923_s
	// 1/pal,	0/ntsc
	ret = ioctl( Vpost->FD, VIDEO_TV_SYSTEM, 1 ) ;
	if( ret < 0 ) 
	{
		printf( "ioctl FBIOGET_VSCREENINFO fail \n" );
	}	
	// lzh_20160923_e
	
	return 0;

	fail:
	{
		CloseVpost( f );
		
		return -1;
	}
	
}

