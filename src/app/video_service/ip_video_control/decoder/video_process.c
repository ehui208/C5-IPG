
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
#include <pthread.h>
#include<net/if.h>

#include "favc_avcodec.h"
#include "favc_version.h"
#include "obj_h264d.h"
#include "obj_vpost.h"
#include "obj_vpe.h"


#include "video_displaye_controller.h"
#include "video_process.h" 

//#include "../IP_Video_Recording/h264_to_mp4.h"

// lzh_20161013_s
#include "../../ip_camera_control/encoder_vin/udp_fragment_opt.h"
#include "../../../utility.h"
// lzh_20161013_e

pthread_t task_H264_dec;
pthread_attr_t attr_H264_dec;

u_int8_t data_dec[DEC_SIZE*20];
H264_RECEIVE_t Receive;
struct sockaddr_in addr;


int SetH264DecVideo( H264_DEC_VIDEO_TYPE type )
{	
	// IO ....
	return 0;
}

H264_DEC_VIDEO_PARA GetH264DecVideo( void )
{
	H264_DEC_VIDEO_PARA para;
	H264_DEC_VIDEO_TYPE type = VGA_DEC; //QVGA_DEC;
	
	// IO ....

	switch( type )
	{
		case QVGA_DEC:
			para.width = 320;
			para.height= 240;
			break;

		case VGA_DEC:
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

/*------------------------------------------------------------------------
								加入组播 (吕智辉的软件, 不了解)
入口:  
		网卡名称, 文件句柄, 组播地址

处理:
	   加入组播	

出口:
		无
------------------------------------------------------------------------*/
static int join_multicast_group2( char* net_dev_name, int32_t socket_fd, int32_t mcg_addr )
{
	struct ifreq ifr;
	struct ip_mreq mreq;
	
	//Get Loacl IP Addr
	memset(ifr.ifr_name, 0, IFNAMSIZ);	
	sprintf(ifr.ifr_name,"%s", net_dev_name);
	if(ioctl(socket_fd, SIOCGIFADDR, &ifr) == -1)//使用 SIOCGIFADDR 获取接口地址
	{
		printf("ioctl call failure!msg:%s\n", strerror(errno));
		goto add_multicast_group_error;
	}
	
	//Added The Multicast Group
	memset(&mreq, 0, sizeof(struct ip_mreq));
	mreq.imr_multiaddr.s_addr = mcg_addr;
	mreq.imr_interface.s_addr = htonl(inet_network(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr)));	// 本机地址加入mcg_addr组播组
	if(setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq)) == -1)
	{
		printf("Setsockopt IPPROTO_IP::IP_ADD_MEMBERSHIP Failure!\n");
		goto add_multicast_group_error;
	}	

	return 0;
add_multicast_group_error:
	return -1;
}

/*------------------------------------------------------------------------
								退出组播 (吕智辉的软件, 不了解)
入口:  
		网卡名称, 文件句柄, 组播地址

处理:
	   加入组播	

出口:
		无
------------------------------------------------------------------------*/
static int leave_multicast_group2( char* net_dev_name, int32_t socket_fd, int32_t mcg_addr )
{
	struct ifreq ifr;
	struct ip_mreq mreq;

	//Get Loacl IP Addr
	sprintf(ifr.ifr_name,"%s", net_dev_name);
	if(ioctl(socket_fd, SIOCGIFADDR, &ifr) == -1)
	{
		printf("ioctl call failure!msg:%s\n", strerror(errno));
		goto leave_multicast_group_error;
	}
	//Leave The Multicast Group
	memset(&mreq, 0, sizeof(struct ip_mreq));
	mreq.imr_multiaddr.s_addr = mcg_addr;
	mreq.imr_interface.s_addr = htonl(inet_network(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr)));
	if(setsockopt(socket_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(struct ip_mreq)) == -1)		// 本机地址离开mcg_addr组播组
	{
		printf("Setsockopt IPPROTO_IP::IP_DROP_MEMBERSHIP Failure!\n");
		goto leave_multicast_group_error;
	}
	return 0;
	
leave_multicast_group_error:
	return -1;
}

DECODE_OBJ_t VpeObj;
static DECODE_OBJ_t VpostObj;
static DECODE_OBJ_t H264Obj;

//FILE*  fout;
//avi_t	 avi;

/*------------------------------------------------------------------------
							Data-process线程关闭时的回调函数
入口:  
		无

处理:
	   释放本线程创建的	

出口:
		无
------------------------------------------------------------------------*/
void H264DecCleanup( void *arg )
{	
	CloseH264Dec( &H264Obj );	
	CloseVpe( &VpeObj );
	CloseVpost( &VpostObj );	
	
//	avi_end( &avi );
//	fclose( fout );
//	VideoDisplayState = VIDEO_DISPLAY_IDLE;

	printf( "%s(%d)-<%s> \n", __FILE__, __LINE__, __FUNCTION__ ) ;
}

/*------------------------------------------------------------------------

------------------------------------------------------------------------*/
void sig_handler( int sig)
{
       if(sig == SIGINT)
	{
		//avi_end( &avi );
		//fclose( fout );
		printf( "%s(%d)-<%s> \n", __FILE__, __LINE__, __FUNCTION__ ) ;
		//exit(0);
       }
}

/*------------------------------------------------------------------------
								Data-process线程
入口:  
		无

处理:

出口:
		无
------------------------------------------------------------------------*/
void  vtk_TaskProcessEvent_H264_dec( void )
{
	int size = 0;
#ifndef SUPPORT_PACK_FRAGMENT	
	int result = 0;
	u_int8_t msgMulticast_Data[DEC_SIZE];
	u_int8_t IDR = 0;	
#endif

	u_int32_t addr_len = sizeof( addr );
	H264_DEC_VIDEO_PARA para;

	//gnal( SIGINT, sig_handler  );
	pthread_cleanup_push( H264DecCleanup, NULL  );

	para = GetH264DecVideo();
	
	if( InitVpost( &VpostObj ) )
	{
		// 进入空闲状态
		video_process_state_set_idle();
		return;
	}
	if ( InitVpe( &VpeObj, &VpostObj, para.width, para.height, 0, 0 ) )	
	{
		CloseVpost( &VpostObj );		
		// 进入空闲状态
		video_process_state_set_idle();
		return;
	}

	if( InitH264Dec( &H264Obj, para.width, para.height ) < 0 )
	{
		CloseVpe( &VpeObj );
		CloseVpost( &VpostObj );			
		// 进入空闲状态
		video_process_state_set_idle();
		return;
	}

	// 进入运行状态
	video_process_state_set_run();

	// lzh_20161013_s
	udp_fragment_t	recv_fragment;
	// lzh_20161013_e

	while( 1 )
	{	
		// lzh_20161013_s	
		// 分包接收
		#ifdef SUPPORT_PACK_FRAGMENT
		
		size = recvfrom( Receive.Sd, (unsigned char*)&recv_fragment, sizeof(udp_fragment_t), 0, (struct sockaddr*)&addr, &addr_len );		

		if( video_process_state_get() == VIDEO_DISPLAY_RUN )
		{
			CVideoPackProcessor_ProcPack((unsigned char*)&recv_fragment,size);
		}
		#else
		
		size = recvfrom( Receive.Sd, msgMulticast_Data, sizeof(msgMulticast_Data), 0, (struct sockaddr*)&addr, &addr_len );		
		
		if( IDR == 0 && msgMulticast_Data[0] == 0 && msgMulticast_Data[1] == 0 && msgMulticast_Data[2] == 0 && msgMulticast_Data[3] == 1 )
		{
			//if( ( (msgMulticast_Data[4]) & ((1<<5)-1) ) == 7 && ( (msgMulticast_Data[16]) & ((1<<5)-1) ) == 8 && ( (msgMulticast_Data[24]) & ((1<<5)-1) ) == 5 )
			if( ( (msgMulticast_Data[4]) & (0x1f) ) == 7)
			{
				IDR = 1;				

				#if 0
				fout = fopen( "/mnt/nand1-2/software/n3296_h264/IP_VIDEO/debug/bin/test.avi", "wb" );
				if( fout == NULL )
					printf( "cannot open output file\n" );
				else
					avi_init( &avi, fout, 25.0, "h264" );
				#endif
			}
		}
		
		if( IDR )
		{
			result = DecodeH264( &H264Obj, (unsigned char*)msgMulticast_Data, size );
			
			if( result < 0 )
				printf( "frame decode FAIL!\n" );
//			else				
//				avi_write( &avi, msgMulticast_Data, size );

			FormatConversion( &VpeObj, &VpostObj, &H264Obj );
		}
		#endif
		// lzh_20161013_e		
	}	

	// lzh_20161114_s
	//pthread_cleanup_pop( 1 );
	pthread_cleanup_pop( 0 );
	// lzh_20161114_e	
	
}

/*------------------------------------------------------------------------
									组播加入
入口:  
		端口号,组播地址

处理:
	   创建Data-process线程; 创建组播接收线程;	

出口:
		无
------------------------------------------------------------------------*/
int EnableVideoDisplay( int32_t type, int16_t port, int32_t mcg_lowbyte )
{
	if( video_process_state_get() == VIDEO_DISPLAY_IDLE )
	{
		// 进入启动状态
		video_process_state_set_start();

		// lzh_20161015_s
		#ifdef SUPPORT_PACK_FRAGMENT
		start_udp_recv_task();
		#endif
		// lzh_20161015_e		

		printf( " vtk_TaskInit_H264_dec%d,,,,,,,,,,,,,,,,0x%08x\n", port, mcg_lowbyte );
		
		Receive.Sd  = -1;	
		if( ( Receive.Sd = socket( PF_INET, SOCK_DGRAM, 0 )  ) == -1 )
		{
			printf("creating socket failed!");
			video_process_state_set_idle();
			return -1;
		}
#if 0
		int on = 1;
		setsockopt( Receive.Sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
#endif
		bzero( &addr, sizeof(addr) );
		addr.sin_family 		= AF_INET;
		addr.sin_addr.s_addr 	= htonl(INADDR_ANY);
		addr.sin_port 		= htons( port );
		
		if( bind(Receive.Sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
		{
			perror( "______________________________________________________________________bind" );
			video_process_state_set_idle();
			return -1;
		}

		if( type )
			join_multicast_group2( "eth0", Receive.Sd, mcg_lowbyte );

		pthread_attr_init( &attr_H264_dec );

		#if 0
		struct sched_param param;								// lzh
		pthread_attr_setschedpolicy(&attr_H264_dec, SCHED_RR);	// lzh
		param.sched_priority = 50;								// lzh
		pthread_attr_setschedparam(&attr_H264_dec, &param);		// lzh
		#endif

		// lzh_20161114_s		
		//pthread_attr_setdetachstate( &attr_H264_dec, PTHREAD_CREATE_DETACHED );
		// lzh_20161114_e		
		if( pthread_create(&task_H264_dec,&attr_H264_dec,(void*)vtk_TaskProcessEvent_H264_dec, NULL ) != 0 )
		{
			printf( "Create task_H264_dec pthread error! \n" );
			
			video_process_state_set_idle();
			
			return -1;
		}
		//usleep( 500*1000 );

		//pthread_detach( task_UDP_Receive );	
		//pthread_detach( task_H264_dec );		
	}
	
	return 0;		
}

/*------------------------------------------------------------------------
									退出组播
入口:  
		端口号,组播地址

处理:
	   关闭Data-process线程; 关闭组播接收线程;	

出口:
		无
------------------------------------------------------------------------*/
int DisableVideoDisplay( int32_t type, int16_t port, int32_t mcg_lowbyte )
{
	if( video_process_state_get() == VIDEO_DISPLAY_RUN )
	{		
		// 进入停止状态
		video_process_state_set_stop();

		// 需要延时一下，保证接收线程读取到停止状态，不再进行组播解码显示
		usleep(100*1000);

#ifdef SUPPORT_PACK_FRAGMENT
		close_udp_recv_task();
#endif
			
		pthread_cancel( task_H264_dec ) ;
		pthread_join( task_H264_dec, NULL );		

		if( type )
			leave_multicast_group2( "eth0", Receive.Sd, mcg_lowbyte );		
		
		close( Receive.Sd );
		
		
		printf( "%s(%d)-<%s> \n", __FILE__, __LINE__, __FUNCTION__ ) ;

		// 进入空闲状态
		video_process_state_set_idle();
	}

	return 0;
}


// lzh_20161015_s
#ifdef SUPPORT_PACK_FRAGMENT

//////////////////////////////////////////////////////////////////////////////////////////////////
// udp组包线程处理
//////////////////////////////////////////////////////////////////////////////////////////////////
pthread_t 		task_udp_recv;
pthread_attr_t 	attr_udp_recv;

void cleanup_udp_recv_task( void *arg )
{	
	CVideoPackProcessor_ClearNode();
	printf( "%s(%d)-<%s> \n", __FILE__, __LINE__, __FUNCTION__ ) ;
}

extern unsigned long GetTickCount(void);

void  process_udp_recv_task( void )
{
	int result = 0;
	int size = 0;
	u_int8_t msgMulticast_Data[DEC_SIZE];
	u_int8_t IDR = 0;	

	pthread_cleanup_push( cleanup_udp_recv_task, NULL  );

	CVideoPackProcessor_InitNode();
	
	while( 1 )
	{	
		ReleaseSemaphore();

		//printf("r:one_frame_sem\n");
		
		// 解码是否停止
		// 解码并显示
		AVPackNode *Node;		

		CVideoPackProcessor_LockNode();
		
		Node = CVideoPackProcessor_PickPack();
		if( Node != NULL )
		{
			memcpy(msgMulticast_Data,Node->Content.Buffer,Node->Content.Len);
			
			printf("r:frame_sn=%d,len=%d,t=%d,%d\n",Node->Content.FrameNo,Node->Content.Len,Node->Content.Timestamp,(unsigned int)GetTickCount());
			
			size = Node->Content.Len;		
			CVideoPackProcessor_ReturnNode(Node);
			// 若无解码和显示则数据包接收正常，开启解码和显示，则解码的时间超过了40ms(25帧的情况下)，导致缓冲数据满而丢包
			#if 1
			//------------------------------------------------------------------------------------------
			// decode 处理
			if( IDR == 0 && msgMulticast_Data[0] == 0 && msgMulticast_Data[1] == 0 && msgMulticast_Data[2] == 0 && msgMulticast_Data[3] == 1 )
			{
				//if( ( (msgMulticast_Data[4]) & ((1<<5)-1) ) == 7 && ( (msgMulticast_Data[16]) & ((1<<5)-1) ) == 8 && ( (msgMulticast_Data[24]) & ((1<<5)-1) ) == 5 )
				if( ( (msgMulticast_Data[4]) & (0x1f) ) == 7)
				{
					IDR = 1;				
				}
			}
			if( IDR )
			{
				result = DecodeH264( &H264Obj, (unsigned char*)msgMulticast_Data, size );
				
				if( result < 0 )
					printf( "frame decode FAIL!\n" );

				FormatConversion( &VpeObj, &VpostObj, &H264Obj );
			}
			//------------------------------------------------------------------------------------------		
			#endif
		}
		
		CVideoPackProcessor_UnLockNode();
	}	

	// lzh_20161114_s
	//pthread_cleanup_pop( 1 );
	pthread_cleanup_pop( 0 );
	// lzh_20161114_e	
}

int start_udp_recv_task( void )
{
	pthread_attr_init( &attr_udp_recv );

#if 0
	struct sched_param param;								// lzh
	pthread_attr_setschedpolicy(&attr_udp_recv, SCHED_RR);	// lzh
	param.sched_priority = 50;								// lzh
	pthread_attr_setschedparam(&attr_udp_recv, &param); 	// lzh
#endif

	// lzh_20161114_s
	//pthread_attr_setdetachstate( &attr_udp_recv, PTHREAD_CREATE_DETACHED );
	// lzh_20161114_e
	if( pthread_create(&task_udp_recv,&attr_udp_recv,(void*)process_udp_recv_task, NULL ) != 0 )
	{
		printf( "Create task_H264_recv pthread error! \n" );
		return -1;
	}	
	return 0;		
}

int close_udp_recv_task( void )
{
	//TriggerSemaphore();
	//usleep( 10*1000 );	// 
	pthread_cancel( task_udp_recv ) ;
	pthread_join( task_udp_recv, NULL );		
	usleep( 1000 );	// 
		
	printf( "%s(%d)-<%s> \n", __FILE__, __LINE__, __FUNCTION__ ) ;
	return 0;
}

#endif


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
VIDEO_DISPLAY_STATE_t  	VideoDisplayState = VIDEO_DISPLAY_IDLE;
pthread_mutex_t 		VideoDisplayState_lock;

int video_process_state_init( void )
{
	pthread_mutex_init( &VideoDisplayState_lock, 0);	
	VideoDisplayState = VIDEO_DISPLAY_IDLE;
	return 0;
}

VIDEO_DISPLAY_STATE_t video_process_state_get( void )
{
	VIDEO_DISPLAY_STATE_t VideoDisplayState_read;	
	pthread_mutex_lock( &VideoDisplayState_lock );		
	VideoDisplayState_read = VideoDisplayState;
	pthread_mutex_unlock( &VideoDisplayState_lock );
	return VideoDisplayState_read;
}

int video_process_state_set_idle( void )
{
	pthread_mutex_lock( &VideoDisplayState_lock );		
	VideoDisplayState = VIDEO_DISPLAY_IDLE;		
	pthread_mutex_unlock( &VideoDisplayState_lock );		
	return 0;
}

int video_process_state_set_start( void )
{
	pthread_mutex_lock( &VideoDisplayState_lock );		
	VideoDisplayState = VIDEO_DISPLAY_START;		
	pthread_mutex_unlock( &VideoDisplayState_lock );		
	return 0;
}

int video_process_state_set_run( void )
{
	pthread_mutex_lock( &VideoDisplayState_lock );		
	VideoDisplayState = VIDEO_DISPLAY_RUN;		
	pthread_mutex_unlock( &VideoDisplayState_lock );		
	return 0;
}

int video_process_state_set_stop( void )
{
	pthread_mutex_lock( &VideoDisplayState_lock );		
	VideoDisplayState = VIDEO_DISPLAY_STOP;		
	pthread_mutex_unlock( &VideoDisplayState_lock );		
	return 0;
}

