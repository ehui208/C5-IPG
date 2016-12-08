/**
  ******************************************************************************
  * @file    main.c
  * @author  lvzhihui
  * @version V1.0.0
  * @date    2016.04.15
  * @brief   This file is the beginner of all tasks
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2016 V-Tec</center></h2>
  ******************************************************************************
  */

#include "../os/RTOS.h"
#include "../os/mul_timer.h"
#include "vdp_uart.h"
#include "task_survey/obj_multi_timer.h"
#include "video_service/ip_video_cs_control.h"
#include "vtk_udp_stack/vtk_udp_stack_c5_ipc_cmd.h"
// lzh_20160901_s
#include "vtk_udp_stack/vtk_udp_stack_c5_ipc_cmd2.h"
// lzh_20160901_e

// lzh_20160806_s
#include "vtk_udp_stack/vtk_udp_stack_device_update.h"
// lzh_20160806_e
// lzh_20160830_s
#include "vtk_udp_stack/vtk_udp_stack_io_server.h"
// lzh_20160830_e

#include "task_survey/task_survey.h"
#include "task_debug_sbu/task_debug_sbu.h"
#include "task_io_server/task_IoServer.h"
#include "./task_survey/obj_CallServer_Virtual/obj_CallServer_Virtual.h"	//czn_20160526

int main( void )
{
		//czn_20160629_s
#ifndef SYNC_PROCESS
	struct sigaction myAction;
	myAction.sa_sigaction = DebugBacktrace;
	sigemptyset(&myAction.sa_mask);
	myAction.sa_flags = SA_RESTART | SA_SIGINFO;
	sigaction(SIGSEGV, &myAction, NULL);
	sigaction(SIGUSR1, &myAction, NULL);
	sigaction(SIGFPE, &myAction, NULL);
	sigaction(SIGILL, &myAction, NULL);
	sigaction(SIGBUS, &myAction, NULL);
	sigaction(SIGABRT, &myAction, NULL);
	sigaction(SIGSYS, &myAction, NULL);	
	sigaction(SIGTERM, &myAction, NULL);	
#else
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask,SIGALRM);
	sigprocmask(SIG_BLOCK,&sigmask,NULL);
#endif	
	//czn_20160629_e
	
	init_all_timer();	
	init_video_cs_service();
	init_c5_ipc_instance();
	// lzh_20160901_s
	init_c5_ipc2_instance();
	// lzh_20160901_e
	// lzh_20160806_s
	init_device_update_instance();
	// lzh_20160806_e
	
	// lzh_20160830_s
	init_udp_io_server_instance();
	// lzh_20160830_e
	
	vtk_TaskInit_io_server();
	vtk_TaskInit_debug_sbu();	
	vtk_TaskInit_survey();		

	//czn_20160526_s
	//init_vdp_becalled_task();			// lzh_20161022
	//init_vdp_caller_task();			// lzh_20161022
	//czn_20160526_e

	// 框架基本功能初始化	
	Init_vdp_uart();
	// 网络管理对象最后运行
	//vtk_TaskInit_net_manang();		// lzh_20161022s

	int reval=0,*repra = 0;

	//-----------test, must be deleted!!!------------
	
	#if 0
	int *test_ret = 0;

	*test_ret = 0;
	printf( "===============================================\n");
	printf( "===============================================\n");
	
	
	static int video_multicast_cnt = 0;
	while(1)
	{
		API_VIDEO_C_SERVICE_TURN_ON_MULTICAST( inet_addr("192.168.0.102"), 150 );
		API_talk_on_by_unicast(inet_addr("192.168.0.102"),AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);		
		sleep(3);
		API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST();		
		API_talk_off();
		sleep(3);
		printf( "================================video_multicast_cnt = %d================\n", ++video_multicast_cnt );
	}	
	#endif
	//-----------test, must be deleted!!!------------
	
	reval = pthread_join( task_control.task_pid, &repra );
	while(1)
	{
		usleep(2000000);
		//send_ip_link_receipt_to_uart();
		printf("pthread_join_task_control return reval=%d,repra=%d\n",reval,(int)repra);
	}
	return 0;
}

