



//#include "capture_procsee.h"
#include "video_capture_controller.h"

#include "obj_adjust.h"
#include "obj_h264e.h"
#include "obj_V4L.h"





typedef enum
{
	TO_AVI_FILE,
	TO_JPG_FILE,
	TO_SENSOR,
	TO_CAPTURE_START,
	TO_CAPTURE_STOP,
}
VIDEO_SOURCE_t;


pthread_t task_VideoCaptureController;



OS_Q Q_VideoCaptureController;


/*------------------------------------------------------------------------
							任务:  VideoCaptureController
入口:  
	

处理:
	  	

出口:
		无
------------------------------------------------------------------------*/
u_int8_t Q_VideoCaptureController_Buffer[100] = {0};

void  VideoCaptureController( void  )
{
	char* pMsg;
	
	int size;
	MsgVideoCaptureController_t msg_buf;
	
	OS_Q_Create( &Q_VideoCaptureController, Q_VideoCaptureController_Buffer, sizeof(Q_VideoCaptureController_Buffer) );

	while( 1 )
	{	
		size = OS_Q_GetPtr( &Q_VideoCaptureController, (void*)&pMsg );		
		memcpy( (unsigned char*)&msg_buf, pMsg, (size<=sizeof(MsgVideoCaptureController_t))?size:sizeof(MsgVideoCaptureController_t));
		OS_Q_Purge( &Q_VideoCaptureController );
			
		if( size )
		{		
			switch( msg_buf.Source )
			{
				case TO_AVI_FILE:				
					break;
				
				case TO_JPG_FILE:
					break;
				
				case TO_SENSOR:
					break;
				
				case TO_CAPTURE_START:
					EnableCapture( msg_buf.ToCapture );
					break;
					
				case TO_CAPTURE_STOP:
					DisableCapture();
					break;
					
				default:
					break;
			}
		}		
	}
}

#if H264_ENC_TEST_MODE
pthread_t task_testController;
pthread_t task_testAdjust;

/*------------------------------------------------------------------------

------------------------------------------------------------------------*/
static void testdController( void )
{
	extern int mode;
	char IP[30];
	int32_t PORT = 0;

	if( mode == 1 ) // -d  IPG
	{
		strcpy( (char*)IP, (char*)"192.168.10.152" );
		PORT = 25051;
	}else if( mode == 2 )// -c DEMO
	{
		strcpy( (char*)IP, (char*)"192.168.10.154" );
		PORT = 25050;
	}

	while(1)
	{		
		API_CaptureStart( PORT, IP  );
		//API_ToMulticastJoin( 25050,"192.168.10.151"  );
		sleep( 150 );
		API_CaptureStop();
		sleep( 3 );
	}
}

typedef struct
{
	int32_t Sd;
	int32_t addr;
	int32_t Port;
} TEST_T;

static TEST_T test_ip;

typedef enum
{
    ADJ_GET,
    ADJ_SET,
    ADJ_DEC,
    ADJ_INC,
}Dir_ype_t;

typedef struct
{
	Dir_ype_t		dir;
	AdjustType_t	type;
    ImagePara_s		data;
} UDP_Image_t;


static void testdAdjust( void )
{
	struct sockaddr_in tx_addr;
	struct sockaddr_in rx_addr;
	u_int32_t size, len;
	unsigned char Encbitsteam[1024];
	UDP_Image_t image;
	ImagePara_s data;
	
	test_ip.Sd	= -1;
	test_ip.addr 	= inet_addr( "192.168.10.152" );
	test_ip.Port 	= htons( 25052);
	
	if( ( test_ip.Sd = socket( PF_INET, SOCK_DGRAM, 0 )	) == -1 )
	{
		printf("creating socket failed!");
		return;
	}

	bzero( &rx_addr, sizeof(rx_addr) );
	rx_addr.sin_family 		= AF_INET;
	rx_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	rx_addr.sin_port			= test_ip.Port ;
	len = sizeof( rx_addr );
	
	if( bind(test_ip.Sd, (struct sockaddr*)&rx_addr, sizeof(rx_addr)) != 0 )
	{
		perror( "______________________________________________________________________bind" );
		return;
	}

	bzero( &tx_addr, sizeof(tx_addr) );	
	tx_addr.sin_family 			= AF_INET;
	tx_addr.sin_addr.s_addr	= test_ip.addr;
	tx_addr.sin_port			= test_ip.Port;
			

	while(1)
	{
		size = recvfrom( test_ip.Sd, &image, sizeof(image), 0, (struct sockaddr*)&rx_addr, &len );
		
		switch( image.type )
		{
			case ADJ_CONTRAST:
				if( image.dir == ADJ_DEC )
					image.data = SetImageContrastDec();
				else if( image.dir == ADJ_INC )
					image.data = SetImageContrastInc();
				break;

			case ADJ_BRIGHT:
				if( image.dir == ADJ_DEC )
					image.data = SetImageBrightDec();
				else if( image.dir == ADJ_INC )
					image.data = SetImageBrightInc();		
				break;

			case ADJ_COLOR:
				if( image.dir == ADJ_DEC )
					image.data = SetImageColorDec();
				else if( image.dir == ADJ_INC )
					image.data = SetImageColorInc();			
				break;

			case ADJ_ALL:
				printf(" ADJ_ALL  %d %d %d \n",image.data.logContrastCnt, image.data.logBrightCnt, image.data.logColorCnt);
				if( image.dir == ADJ_GET )
					ReadImagePara( &image.data );
				else if( image.dir == ADJ_SET )
					image.data = SetImageAllValue( image.data.logContrastCnt, image.data.logBrightCnt, image.data.logColorCnt );
				break;
		}
		
		if( sendto(test_ip.Sd, &image, sizeof(image), 0, (struct sockaddr*)&tx_addr,sizeof(tx_addr)) == -1 )
		{
			printf("can not send data from socket! errno:%d,means:%s\n",errno,strerror(errno));
		}

	}
}
#endif

/*------------------------------------------------------------------------
							创建任务:  VideoCaptureController
入口:  
	

处理:
	  	

出口:
		无
------------------------------------------------------------------------*/
void vtk_VideoCaptureController( void )
{	
	catpure_process_state_init();
	
	if( pthread_create(&task_VideoCaptureController,NULL,(void*)VideoCaptureController, NULL) != 0 )
	{
		printf( "Create task_VideoCaptureController pthread error! \n" );
		exit(1);
	}	
	//sleep( 1 );
#if H264_ENC_TEST_MODE
	if( pthread_create(&task_testController,NULL,(void*)testdController, NULL) != 0 )
	{
		printf( "Create task_VideoCaptureController pthread error! \n" );
		exit(1);
	}
	
	if( pthread_create(&task_testAdjust,NULL,(void*)testdAdjust, NULL) != 0 )
	{
		printf( "Create task_VideoCaptureController pthread error! \n" );
		exit(1);
	}

	//pthread_join( task_VideoCaptureController, NULL );
#else
	pthread_detach( task_VideoCaptureController );
#endif
}


/*------------------------------------------------------------------------
									加入组播
入口:  
	

处理:
	  	

出口:
		无
------------------------------------------------------------------------*/
void API_CaptureStart( int16_t port, int32_t mcg_lowbyte )
{
	MsgVideoCaptureController_t msg;

	msg.Source 			= TO_CAPTURE_START;
	msg.ToCapture.Port	= port;
	msg.ToCapture.Ip	= mcg_lowbyte;

	// test
	OS_Q_DisplayInfo(&Q_VideoCaptureController);
	// test

	if( OS_Q_Put( &Q_VideoCaptureController, &msg, sizeof(MsgVideoCaptureController_t) ) )
		printf(" Q_VideoCaptureController if full \n");
}


/*------------------------------------------------------------------------
									退出组播
入口:  
	

处理:
	  	

出口:
		无
------------------------------------------------------------------------*/
void API_CaptureStop( void )
{
	MsgVideoCaptureController_t msg;

	msg.Source			= TO_CAPTURE_STOP;

	// test
	OS_Q_DisplayInfo(&Q_VideoCaptureController);
	// test

	if( OS_Q_Put( &Q_VideoCaptureController, &msg, sizeof(MsgVideoCaptureController_t) ) )
		printf(" Q_VideoCaptureController if full \n");
}

void API_ToMulticastLeave( int16_t port, int32_t mcg_lowbyte )
{
	API_CaptureStop();
}

void API_ToMulticastJoin( int16_t port, int32_t mcg_lowbyte )
{
	API_CaptureStart( port, mcg_lowbyte );
}


