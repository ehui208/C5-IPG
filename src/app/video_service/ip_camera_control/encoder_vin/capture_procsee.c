
//#include "video_capture_controller.h"
#include "capture_procsee.h"
#include "obj_h264e.h"
#include "obj_V4L.h"
#include "obj_adjust.h"

// lzh_20161013_s
#include "udp_fragment_opt.h"
// lzh_20161013_e

//#include "../IP_Video_Recording/JpegEnc.h"

//char *pchSaveFolder = NULL;

//#define DEFAULT_SAVE_FOLDER "/mnt/nand1-2/software/n3296_h264/"



typedef struct
{
	int32_t Sd;
	int32_t addr;
	int32_t Port;
} H264_SEND_t;

#define ENC_SIZE	ENC_IMG_WIDTH*ENC_IMG_HEIGHT*3/2

pthread_t task_H264_enc;

pthread_attr_t attr_H264_enc;

static ENCODE_OBJ_t H264Obj;
ENCODE_OBJ_t V4lObj;	

static H264_SEND_t sockfd;



int SetH264EncVideo( H264_ENC_VIDEO_TYPE type )
{	
	// IO ....
	return 0;
}

H264_ENC_VIDEO_PARA GetH264EncVideo( void )
{
	H264_ENC_VIDEO_PARA para;
	H264_ENC_VIDEO_TYPE type = VGA_ENC; //QVGA_ENC;
	
	// IO ....

	switch( type )
	{
		case QVGA_ENC:
			para.width = 320;
			para.height= 240;
			break;

		case VGA_ENC:
			para.width = 640;
			para.height = 480;
			break;

		default:
			para.width = 320;
			para.height = 240;
			break;
	}

	return para;

}

int SetH264EncVideoPara( ENCODE_PROFILE_t para )
{
	// IO...
	return 0;
}

ENCODE_PROFILE_t GetH264EncVideoPara( void )
{
	ENCODE_PROFILE_t video_setting;
	H264_ENC_VIDEO_PARA para = GetH264EncVideo();

	// IO...
	
	//set the default value
	video_setting.qmax		= 51;
	video_setting.qmin		= 10;
#ifdef RATE_CTL    
	video_setting.quant 	= 14;	// Init Quant
#else
	video_setting.quant 	= FIX_QUANT;
#endif    
	video_setting.bit_rate	= 1800*1000;//512000;
	video_setting.width 	= para.width;
	video_setting.height	= para.height;
	video_setting.framerate = 20; //25;	// 25
	video_setting.gop_size	= IPInterval;
	video_setting.frame_rate_base = 1;
	video_setting.key_frame = 40;	//50;

	return video_setting;
}

/*------------------------------------------------------------------------
							关闭Data-process(Threads)线程时的回调函数
入口:  
	

处理:
	  	

出口:
		无
------------------------------------------------------------------------*/
void H264EncCleanup( void *arg )
{	
	CloseH264Enc( &H264Obj );	

	CloseV4LDevice( &V4lObj );	
	
	close( sockfd.Sd  );
	
//	CaputureState = CAPUTRE_IDLE;

	printf( "%s(%d)-<%s> \n", __FILE__, __LINE__, __FUNCTION__ ) ;
}

/*------------------------------------------------------------------------
							创建Data-process(Threads)并运行
入口:  
	

处理:
	  	

出口:
		无

		
------------------------------------------------------------------------*/
unsigned long GetTickCount(void);

void vtk_TaskProcessEvent_H264_enc( void )
{
	//int test = 0;
	unsigned int size;
	ENCODE_PROFILE_t video_setting;
	ENCODE_FRAME_t pict;
	unsigned int u_image_size, v_image_size;
	unsigned char Encbitsteam[ENC_SIZE];
	
	uint32_t PicPhyAdr;
	//ImagePara_s imagePara;
	struct sockaddr_in addr;	

	video_setting = GetH264EncVideoPara();
	
	u_image_size = video_setting.width * video_setting.height;
	v_image_size = u_image_size + ( u_image_size >> 2 );

	printf( " video_setting.width %d      video_setting.height %d \n", video_setting.width, video_setting.height );

	pthread_cleanup_push( H264EncCleanup, NULL  );

	if( ( sockfd.Sd = socket( PF_INET, SOCK_DGRAM, 0 )  ) == -1 )
	{
		printf("creating socket failed!");
		// 进入空闲状态
		catpure_process_state_set_idle();
		return;
	}
	
#if 0
	int  on = 1;
	setsockopt( sockfd.Sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
#endif
	
	bzero( &addr, sizeof(addr) );
	
	addr.sin_family 		= AF_INET;
	addr.sin_addr.s_addr 	= sockfd.addr;
	addr.sin_port 			= sockfd.Port;


	if( InitV4l( &V4lObj, video_setting.width, video_setting.height, 0, 1 ) < 0 )
	{		
		printf("-----------open v4l device failed\n---------");
		close( sockfd.Sd  );		
		// 进入空闲状态
		catpure_process_state_set_idle();
		return;
	}
	
	if( InitH264Enc( &H264Obj, &video_setting ) < 0 )
	{
		printf("-----------open v4l device failed\n---------");
		close( sockfd.Sd  );		
		CloseV4LDevice( &V4lObj );	
		// 进入空闲状态
		catpure_process_state_set_idle();
		return;		
	}
	
	API_LoadImagePara();

//	imagePara = GetmageAllValue();

//	printf(" SetSensorBrigness %d \n", imagePara.logBrightCnt);
//	printf(" SetSensorContrast %d \n", imagePara.logContrastCnt);
//	printf(" SetSensorColor %d \n", imagePara.logColorCnt);

//	imagePara = SetImageAllValue( 7, 1, 7 );

//	printf(" SetSensorBrigness %d \n", imagePara.logBrightCnt);
//	printf(" SetSensorContrast %d \n", imagePara.logContrastCnt);
//	printf(" SetSensorColor %d \n", imagePara.logColorCnt);

	
	// lzh_20161013_s
	udp_fragment_t	send_fragment;
	int				send_fragment_len;
	init_send_fragment_head(&send_fragment);	

	unsigned int last_Timestamp	= GetTickCount();	

	// 进入运行状态
	catpure_process_state_set_run();
	
	// lzh_20161013_e	
	while( 1 )
	{		
#if 0	
		if( ++test > 0xffffffff )
		{		
			test = 0;

			InitV4l( &V4lObj, 1280, 720, 1, 0 );

			TriggerV4LNextFrame( &V4lObj );
			TriggerV4LNextFrame( &V4lObj );

			if( !ReadV4LPicture( &V4lObj, &PicPhyAdr ) )
			{			
				CaptureImage( &V4lObj, 1280, 720, NULL, PicPhyAdr );				
			}

			InitV4l( &V4lObj, video_setting.width, video_setting.height, 0 , 0 );

			TriggerV4LNextFrame( &V4lObj );
			TriggerV4LNextFrame( &V4lObj );
		}
#endif
		if( !ReadV4LPicture( &V4lObj, &PicPhyAdr ) )
		{
			pict.data[0] = (unsigned char *)PicPhyAdr;
			pict.data[1] = (unsigned char *)(PicPhyAdr + u_image_size);
			pict.data[2] = (unsigned char *)(PicPhyAdr + v_image_size);  	    

			size = EncodeH264( &H264Obj,  (unsigned char *)Encbitsteam, (void *)&pict );

			//size = 40*1200;
				
			// lzh_20161013_s			
			// 分包发送
			#if 1
			
			// -----------此段保证发送的时间间隔-------
#if 1
			while( 1 )
			{
				if( abs(GetTickCount()-last_Timestamp) < 50 )					
					usleep(1000);
				else
					break;
			}
			last_Timestamp	= GetTickCount();
#endif
			start_send_fragment_head(&send_fragment,size);
			// -----------此段保证发送的时间间隔-------
			
			printf("s:frame_sn=%d,len=%d,t=%d\n",send_fragment.m_head.FrameNo,send_fragment.m_head.Framelen,send_fragment.m_head.Timestamp);
			//PrintCurrentTime(0);

			pull_data_for_send:
				
			send_fragment_len = pull_send_fragment_data(&send_fragment,Encbitsteam);
			send_fragment_len += PACK_MARK_LEN;
			send_fragment_len += sizeof(AVPackHead);

			if( sendto(sockfd.Sd, (unsigned char*)&send_fragment, send_fragment_len, 0, (struct sockaddr*)&addr,sizeof(addr))   == -1 )
			{
				printf("can not send data from socket! errno:%d,means:%s\n",errno,strerror(errno));
			}
			if( prepare_for_next_fragment(&send_fragment) == 0 )
			{
				goto pull_data_for_send;
			}
			#else
			if( sendto(sockfd.Sd, Encbitsteam, size, 0, (struct sockaddr*)&addr,sizeof(addr))   == -1 )
			{
				printf("can not send data from socket! errno:%d,means:%s\n",errno,strerror(errno));
			}
			#endif
			// lzh_20161013_e				
			
			TriggerV4LNextFrame( &V4lObj );		
		}	
	}
	// lzh_20161114_s
	//pthread_cleanup_pop( 1 );
	pthread_cleanup_pop( 0 );
	// lzh_20161114_e	
}

/*------------------------------------------------------------------------
									启动:  CaptureMulticast
入口:  
	

处理:
	  	

出口:
		无
------------------------------------------------------------------------*/
void  EnableCapture( SUB_MSG_TO_CAPTURE_t Msg )
{			
	printf( " capture eanble: state=%d, port=%d, ip=0x%08x  \n", catpure_process_state_get(), Msg.Port, Msg.Ip);

	if( catpure_process_state_get() == CAPUTRE_IDLE )
	{
		sockfd.addr = Msg.Ip;
		sockfd.Port = htons( Msg.Port );
	
		// 进入启动状态
		catpure_process_state_set_start();
			
		pthread_attr_init( &attr_H264_enc );

		#if 0
		struct sched_param param;								// lzh
		pthread_attr_setschedpolicy(&attr_H264_enc, SCHED_RR);	// lzh
		param.sched_priority = 50;								// lzh
		pthread_attr_setschedparam(&attr_H264_enc, &param);		// lzh
		#endif
		// lzh_20161114_s
		//pthread_attr_setdetachstate( &attr_H264_enc, PTHREAD_CREATE_DETACHED );
		// lzh_20161114_e
		if( pthread_create(&task_H264_enc,&attr_H264_enc,(void*)vtk_TaskProcessEvent_H264_enc, NULL) != 0 )
		{
			printf( "Create task_H264_enc pthread error! \n" );
			// 进入空闲状态
			catpure_process_state_set_idle();
		}
		
		//pthread_detach( task_H264_enc );
		printf( "%s(%d)-<%s> \n", __FILE__, __LINE__, __FUNCTION__ ) ;

	}
}

/*------------------------------------------------------------------------
									关闭:  CaptureMulticast
入口:  
	

处理:
	  	

出口:
		无
------------------------------------------------------------------------*/
void  DisableCapture( void )
{
	printf( " capture disable: state=%d\n", catpure_process_state_get());

	if( catpure_process_state_get() == CAPUTRE_RUN )
	{
		usleep( 100*1000);
		// 进入停止状态
		catpure_process_state_set_stop();
	
		pthread_cancel( task_H264_enc ) ;
		pthread_join( task_H264_enc, NULL );
		usleep( 10*100 );
		printf( "%s(%d)-<%s> \n", __FILE__, __LINE__, __FUNCTION__ ) ;
		
		// 进入空闲状态
		catpure_process_state_set_idle();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAPUTRE_STATE_t 	CaputureState = CAPUTRE_IDLE;
pthread_mutex_t 	CaputureState_lock;

int catpure_process_state_init( void )
{
	pthread_mutex_init( &CaputureState_lock, 0);	
	CaputureState = CAPUTRE_IDLE;
	return 0;
}

CAPUTRE_STATE_t catpure_process_state_get( void )
{
	CAPUTRE_STATE_t CaputureState_read;	
	pthread_mutex_lock( &CaputureState_lock );		
	CaputureState_read = CaputureState;
	pthread_mutex_unlock( &CaputureState_lock );
	return CaputureState_read;
}

int catpure_process_state_set_idle( void )
{
	pthread_mutex_lock( &CaputureState_lock );		
	CaputureState = CAPUTRE_IDLE;		
	//printf("capture_process_state_set=%d\n",CaputureState);
	pthread_mutex_unlock( &CaputureState_lock );		
	return 0;
}

int catpure_process_state_set_start( void )
{
	pthread_mutex_lock( &CaputureState_lock );		
	CaputureState = CAPUTRE_START;		
	//printf("capture_process_state_set=%d\n",CaputureState);
	pthread_mutex_unlock( &CaputureState_lock );		
	return 0;
}

int catpure_process_state_set_run( void )
{
	pthread_mutex_lock( &CaputureState_lock );		
	CaputureState = CAPUTRE_RUN;		
	//printf("capture_process_state_set=%d\n",CaputureState);
	pthread_mutex_unlock( &CaputureState_lock );		
	return 0;
}

int catpure_process_state_set_stop( void )
{
	pthread_mutex_lock( &CaputureState_lock );		
	CaputureState = CAPUTRE_STOP;		
	//printf("capture_process_state_set=%d\n",CaputureState);
	pthread_mutex_unlock( &CaputureState_lock );		
	return 0;
}


