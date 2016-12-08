
//#include "multicast_data_process.h"
#include "video_displaye_controller.h"




typedef enum
{
	FROM_AVI_FILE,
	FROM_JPG_FILE,
	FROM_SENSOR,
	FROM_MULTICAST,
	FROM_UNICAST,
}
VIDEO_SOURCE_t;


pthread_t task_VideoDisplayerController;

OS_Q Q_VideoDisplayerController;

uint8 Q_VideoDisplayerController_Buffer[100] = {0};
int videodisplay_reset_cnt = 0;
void ResetVideoDisplayerQueue(void)
{
	videodisplay_reset_cnt++;
 	OS_Q_Delete(&Q_VideoDisplayerController);	
	OS_Q_Create( &Q_VideoDisplayerController, Q_VideoDisplayerController_Buffer, sizeof(Q_VideoDisplayerController_Buffer) );
}

/*------------------------------------------------------------------------
								任务: Video-Displaye-Controller
入口:  
	 

处理:

出口:
		 无
------------------------------------------------------------------------*/
void  VideoDisplayerController( void  )
{
	uint8* pMsg;
	VIDEO_SOURCE_t type;
	uint8 temp;

	int size;
	MsgVideoDisplayerController_t msg_buf;
	
	printf( " VideoDisplayerController %d,\n",  getpid() );
	OS_Q_Create( &Q_VideoDisplayerController, Q_VideoDisplayerController_Buffer, sizeof(Q_VideoDisplayerController_Buffer) );

	while( 1 )
	{
		#if 0
		OS_Q_GetPtr( &Q_VideoDisplayerController, (void*)&pMsg );
		type =  ((MsgVideoDisplayerController_t*)pMsg)->Source;		
		switch( type )
		{
			case FROM_AVI_FILE:				
				break;
			
			case FROM_JPG_FILE:
				break;
			
			case FROM_SENSOR:
				break;
			
			case FROM_MULTICAST:
			case FROM_UNICAST:
				if( type == FROM_MULTICAST )
					temp = 1;
				else
					temp = 0;
				
				if( ((MsgVideoDisplayerController_t*)pMsg)->VideoDisplay.Cmd == VIDEO_DISPLAY_JOIN )
				{
					EnableVideoDisplay(  temp, ((MsgVideoDisplayerController_t*)pMsg)->VideoDisplay.Port, 
											  ((MsgVideoDisplayerController_t*)pMsg)->VideoDisplay.Ip );
				}
				else
				{
					DisableVideoDisplay(  temp, ((MsgVideoDisplayerController_t*)pMsg)->VideoDisplay.Port, 
											  ((MsgVideoDisplayerController_t*)pMsg)->VideoDisplay.Ip );
				}
				break;
		}
		
		OS_Q_Purge( &Q_VideoDisplayerController );
		
		#else
		
		size = OS_Q_GetPtr( &Q_VideoDisplayerController, (void*)&pMsg );		
		memcpy( (unsigned char*)&msg_buf, pMsg, (size<=sizeof(MsgVideoDisplayerController_t))?size:sizeof(MsgVideoDisplayerController_t));
		OS_Q_Purge( &Q_VideoDisplayerController );

		type =  msg_buf.Source;		
		
		switch( type )
		{
			case FROM_AVI_FILE:				
				break;
			
			case FROM_JPG_FILE:
				break;
			
			case FROM_SENSOR:
				break;
			
			case FROM_MULTICAST:
			case FROM_UNICAST:
				if( type == FROM_MULTICAST )
					temp = 1;
				else
					temp = 0;
				
				if( msg_buf.VideoDisplay.Cmd == VIDEO_DISPLAY_JOIN )
				{
					EnableVideoDisplay(  temp, msg_buf.VideoDisplay.Port, msg_buf.VideoDisplay.Ip );
				}
				else
				{
					DisableVideoDisplay(  temp, msg_buf.VideoDisplay.Port, msg_buf.VideoDisplay.Ip );
				}
				break;
		}
		#endif
	}
}

#if H264_DEC_TEST_MODE
pthread_t task_testController;
pthread_t task_testAdjust;

/*------------------------------------------------------------------------

------------------------------------------------------------------------*/
static void testController( void )
{
	extern int mode;
	printf( " testController %d,\n",  getpid() );	
	int8_t IP[30];
	int32_t PORT = 0;

	if( mode == 1 ) // -d  IPG
	{
		strcpy( (char*)IP, (char*)"192.168.10.154" );
		PORT = 25050;
	}else if( mode == 2 )// -c DEMO
	{
		strcpy( (char*)IP, (char*)"192.168.10.152" );
		PORT = 25051;
	}
	
	while(1)
	{		
		//API_FromMulticastJoin( 25050, (int8_t*)"224.0.2.1" ); 
		API_FromUnicastJoin( PORT, IP );
		sleep( 200 );
		//API_FromMulticastLeave( 25050, (int8_t*)"224.0.2.1" ); 
		API_FromUnicastLeave();
		sleep( 2 );	
		
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


typedef enum
{
    ADJ_CONTRAST,
    ADJ_BRIGHT,
    ADJ_COLOR,
    ADJ_ALL,
}AdjustType_t;

typedef struct
{
    	unsigned char   	logContrastCnt;
	unsigned char 	logBrightCnt;
	unsigned char 	logColorCnt;
}ImagePara_t;

typedef struct
{
	Dir_ype_t			dir;
	AdjustType_t		type;
    	ImagePara_t		data;
} UDP_Image_t;


static void testdAdjust( void )
{
	struct sockaddr_in tx_addr;
	struct sockaddr_in rx_addr;
	u_int32_t size, len;
	int temp;
	unsigned char Encbitsteam[1024];
	UDP_Image_t image;
	
	test_ip.Sd	= -1;
	test_ip.addr 	= inet_addr( "192.168.10.154" );
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
			
	//image.dir = ADJ_GET;
	//image.type = ADJ_ALL;
	
	while(1)
	{
		test_h:

		printf("\n************* adjust ************\n");
		printf("--- DIR: 0/get 1/set  2/dec 3/inc---\n");	
		printf("--- TYPE: 0/CONTRAST 1/BRIGHT 2/COLOR 3/ALL ---\n");	
		printf("--- DATA: 0~9 ---\n");	
		printf("*********************************\n");

		test_err:
			
		temp = (getchar() - 0x30);
		if( temp == 0 || temp == 1 || temp == 2 || temp == 3 )
			image.dir = (Dir_ype_t)temp;
		else if( temp == 'h' )
			goto test_h;
		else
			goto test_err;

		temp = (getchar() - 0x30);
		if( temp == 0 || temp == 1 || temp == 2 || temp == 3 )
			image.type = (AdjustType_t)temp;
		else
			goto test_err;		

		if( temp == 3 )
		{
			temp = (getchar() - 0x30);
			if( temp >= 0 && temp <=9 )
				image.data.logContrastCnt = temp;

			temp = (getchar() - 0x30);
			if( temp >= 0 && temp <=9 )
				image.data.logBrightCnt = temp;

			temp = (getchar() - 0x30);
			if( temp >= 0 && temp <=9 )
				image.data.logColorCnt = temp;
		}
		
		printf(" dir:%d	type:%d	logColorCnt:%d	\n", image.dir, image.type, image.data.logColorCnt );
		
		if( sendto(test_ip.Sd, &image, sizeof(image), 0, (struct sockaddr*)&tx_addr,sizeof(tx_addr)) == -1 )
		{
			printf("can not send data from socket! errno:%d,means:%s\n",errno,strerror(errno));
		}

		size = recvfrom( test_ip.Sd, &image, sizeof(image), 0, (struct sockaddr*)&rx_addr, &len );

		printf(" logContrastCnt:%d	logBrightCnt:%d	logColorCnt:%d	\n", image.data.logContrastCnt, image.data.logBrightCnt, image.data.logColorCnt );

	}
}
#endif


/*------------------------------------------------------------------------
								创建任务: Video-Displaye-Controller
入口:  
	 

处理:

出口:
		 无
------------------------------------------------------------------------*/
void vtk_VideoDisplayerController( void )
{	
	video_process_state_init();

	if( pthread_create(&task_VideoDisplayerController,NULL,(void*)VideoDisplayerController, NULL) != 0 )
	{
		printf( "Create task_VideoDisplayerController pthread error! \n" );
		exit(1);
	}	
#if H264_DEC_TEST_MODE
	if( pthread_create(&task_testController,NULL,(void*)testController, NULL) != 0 )
	{
		printf( "Create task_VideoDisplayerController pthread error! \n" );
		exit(1);
	}	

	if( pthread_create(&task_testAdjust,NULL,(void*)testdAdjust, NULL) != 0 )
	{
		printf( "Create task_VideoCaptureController pthread error! \n" );
		exit(1);
	}

	//pthread_join( task_VideoDisplayerController, NULL );
#else
	pthread_detach( task_VideoDisplayerController );
#endif
}

/*------------------------------------------------------------------------
								加入组播							
入口:  
	 端口号,组播地址

处理:

出口:
		 无
------------------------------------------------------------------------*/
void API_FromMulticastJoin( int16_t port, int32_t mcg_lowbyte )
{
#if 0
	MsgVideoDisplayerController_t msg;

	msg.Source 				= FROM_MULTICAST;
	msg.VideoDisplay.Cmd	= VIDEO_DISPLAY_JOIN;
	msg.VideoDisplay.Port	= port;
	msg.VideoDisplay.Ip		= mcg_lowbyte;

	// test
	//OS_Q_DisplayInfo(&Q_VideoDisplayerController);
	printf(" Q_VideoDisplayerController reset cnt = %d \n",videodisplay_reset_cnt);		
	// test

	if( OS_Q_Put( &Q_VideoDisplayerController, &msg, sizeof(msg) ) )
	{
		printf(" Q_VideoDisplayerController if full \n");
		ResetVideoDisplayerQueue();		
	}	
#else
	EnableVideoDisplay(   1, port, mcg_lowbyte  );
#endif
}

/*------------------------------------------------------------------------
								退出组播							
入口:  
	 端口号,组播地址

处理:

出口:
		 无
------------------------------------------------------------------------*/
void API_FromMulticastLeave( int16_t port, int32_t mcg_lowbyte )
{
#if 0
	MsgVideoDisplayerController_t msg;

	msg.Source 				= FROM_MULTICAST;
	msg.VideoDisplay.Cmd	= VIDEO_DISPLAY_LEAVE;
	msg.VideoDisplay.Port	= port;
	msg.VideoDisplay.Ip		= mcg_lowbyte;

	// test
	//OS_Q_DisplayInfo(&Q_VideoDisplayerController);
	printf(" Q_VideoDisplayerController reset cnt = %d \n",videodisplay_reset_cnt);		
	// test
		
	if( OS_Q_Put( &Q_VideoDisplayerController, &msg, sizeof(MsgVideoDisplayerController_t) ) )
	{
		printf(" Q_VideoDisplayerController if full \n");
		ResetVideoDisplayerQueue();
	}
#else
	DisableVideoDisplay(  1, port, mcg_lowbyte );
#endif
}

/*------------------------------------------------------------------------
								加入组播							
入口:  
	 端口号,组播地址

处理:

出口:
		 无
------------------------------------------------------------------------*/
void API_FromUnicastJoin( int16_t port, int32_t mcg_lowbyte )
{
	MsgVideoDisplayerController_t msg;

	msg.Source 				= FROM_UNICAST;
	msg.VideoDisplay.Cmd	= VIDEO_DISPLAY_JOIN;
	msg.VideoDisplay.Port	= port;
	msg.VideoDisplay.Ip		= mcg_lowbyte;

	// test
	//OS_Q_DisplayInfo(&Q_VideoDisplayerController);
	printf(" Q_VideoDisplayerController reset cnt = %d \n",videodisplay_reset_cnt);		
	// test

	if( OS_Q_Put( &Q_VideoDisplayerController, &msg, sizeof(msg) ) )
	{
		printf(" Q_VideoDisplayerController if full \n");
		ResetVideoDisplayerQueue();		
	}
}

/*------------------------------------------------------------------------
								退出组播							
入口:  
	 端口号,组播地址

处理:

出口:
		 无
------------------------------------------------------------------------*/
void API_FromUnicastLeave( void )
{
	MsgVideoDisplayerController_t msg;

	msg.Source 				= FROM_UNICAST;
	msg.VideoDisplay.Cmd	= VIDEO_DISPLAY_LEAVE;

	// test
	//OS_Q_DisplayInfo(&Q_VideoDisplayerController);
	printf(" Q_VideoDisplayerController reset cnt = %d \n",videodisplay_reset_cnt);		
	// test
		
	if( OS_Q_Put( &Q_VideoDisplayerController, &msg, sizeof(MsgVideoDisplayerController_t) ) )
	{		
		printf(" Q_VideoDisplayerController if full \n");
		ResetVideoDisplayerQueue();
	}
}

