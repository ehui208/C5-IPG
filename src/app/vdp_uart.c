

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include "uart.h"
#include "vdp_uart.h"
#include "slipframe.h"
#include "utility.h"		//czn_20160526

#include "./task_survey/task_survey.h"

Serial_Task_Info 		ttyS0_sti;
sem_t					ttyS0_wait_ack_sem;
pthread_t 				vdp_uart_process_pid;
Loop_vdp_common_buffer	vdp_uart_sending_queue;

int vdp_uart_send_data_once(char *data, int len);
void* vdp_uart_sending_queue_task( void* arg );
void vdp_uart_sending_data_process(char* msg_data,int len);
	
#if  1 //PACKAGE_TYPE == PACKAGE_TYPE_SLIP

Slip_Frame_Info	slip_frame;

#endif

/*******************************************************************************************
 * @fn:		ttyS0_rcv_handle
 *
 * @brief:	串口接收数据包处理
 *
 * @param:  	*data	- 数据区指针
 * @param:  	len		- 数据包长度
 *
 * @return: 	none
*******************************************************************************************/
void ttyS0_rcv_handle(char *data, int len)
{
	int ret;
	// 接收到一个数据帧
	#if  PACKAGE_TYPE == PACKAGE_TYPE_SLIP		//slip格式处理
	
	slip_push_fifo(&slip_frame,data,len);
	
	while(1)
	{
		ret = SlipRecFrameProcess(&slip_frame);
		if( ret < 0 )
			break;
		if( ret > 0 )
		{
			if( slip_frame.rx_out_cnt )
			{
				vdp_uart_recv_data(slip_frame.rx_out_buf, slip_frame.rx_out_cnt);
			}
		}
	}	
	
	#else		

	slip_push_fifo(&slip_frame,data,len);
	
	while(1)
	{
		ret = SlipRecFrameProcess(&slip_frame);
		if( ret < 0 )
			break;
		if( ret > 0 )
		{
			if( slip_frame.rx_out_cnt )
			{
				//vdp_uart_recv_data(slip_frame.rx_out_buf, slip_frame.rx_out_cnt);
				api_uart_recv_callback(slip_frame.rx_out_buf, slip_frame.rx_out_cnt);				
			}
		}
	}		
	#endif
}

/*******************************************************************************************
 * @fn:		Init_vdp_uart
 *
 * @brief:	初始化uart端口
 *
 * @param:  	none
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int Init_vdp_uart(void)
{		
	//if( !start_serial_task(&ttyS0_sti, "/dev/ttyS0", 115200, 8, 'N', 1, &ttyS0_rcv_handle) )
	if( !start_serial_task(&ttyS0_sti, "/dev/ttyS0", 9600, 8, 'N', 1, &ttyS0_rcv_handle) )
	{
		init_vdp_common_queue(&vdp_uart_sending_queue,500,(msg_process)vdp_uart_sending_data_process,NULL); 
		// 初始化等待ack应答的发送线程
		if( pthread_create(&vdp_uart_process_pid, 0, vdp_uart_sending_queue_task, &vdp_uart_sending_queue) )
		{
			eprintf("initial init_vdp_common_queue failed!\n");
			return -1;
		}
		// 初始化等待ack应答信号量	
		if( sem_init(&ttyS0_wait_ack_sem,0,0) == -1 )
		{
			eprintf("initial semaphore failed!\n");
			return -1;
		}
#if  1	//PACKAGE_TYPE == PACKAGE_TYPE_SLIP
		slip_frame_process_init(&slip_frame);
#endif	
	 	dprintf("ttyS0 open  Success!\n");
	}
	else
	{
		eprintf("start_serial_task failure!\n");
	}	
	return 0;
}

void* vdp_uart_sending_queue_task( void* arg )
{
	p_Loop_vdp_common_buffer	 pqueue = (p_Loop_vdp_common_buffer)arg;
	p_vdp_common_buffer 		pdb 	= 0;

	while( 1 )
	{
		int size;
		size = pop_vdp_common_queue( pqueue,&pdb,100 );
		if( size > 0 )
		{
			(*pqueue->process)(pdb,size);		
			purge_vdp_common_queue( pqueue );
		}
	}
	return 0;
}

void vdp_uart_sending_data_process(char* msg_data,int len)
{
	int ret;

	bprintf("uart send one pack!!\n");		

	sem_init(&ttyS0_wait_ack_sem,0,0);
	
	vdp_uart_send_data_once(msg_data,len);

	// 20161021_lzh_s
	/*  
	PrintCurrentTime(0);

	ret = sem_wait_ex2(&ttyS0_wait_ack_sem, 1000);
	if( ret != 0 ) 	//timeout
	{
		PrintCurrentTime(10);
		bprintf("uart send one pack again, ret = %d...\n",ret);		
		
		sem_init(&ttyS0_wait_ack_sem,0,0);
		
		vdp_uart_send_data_once(msg_data,len);
		
		sem_wait_ex2(&ttyS0_wait_ack_sem, 1000);
	}*/
	// 20161021_lzh_e
}

unsigned char UartCheckSum( char* pbuf, int len )
{
	unsigned char checksum = 0;
	int i;
	for( i = 0; i < len; i++ )
	{
		checksum += pbuf[i];
	}
	return checksum;
}


int vdp_uart_send_data_once(char *data, int len)
{		
	#if  PACKAGE_TYPE == PACKAGE_TYPE_SLIP		//slip格式处理	

	int i;
	unsigned char checksum;

	slip_frame.tx_in_cnt = len;	
	memcpy( slip_frame.tx_in_buf, data, len);

#ifdef VDP_PRINTF_DEBUG	
		//-----------------debug--------------
		printf("slip presend len = %d: ",slip_frame.tx_in_cnt);
		for( i = 0; i < slip_frame.tx_in_cnt; i++ )
		{
			printf(" %d ",slip_frame.tx_in_buf[i]);
		}
		printf("\n\r"); 
#endif	
	
	//checksum
	checksum = UartCheckSum(slip_frame.tx_in_buf,slip_frame.tx_in_cnt);
	slip_frame.tx_in_buf[slip_frame.tx_in_cnt] = checksum;
	slip_frame.tx_in_cnt++;
	//checksum

	SlipTrsFrameProcess( &slip_frame );

#ifdef VDP_PRINTF_DEBUG	
	//-----------------debug--------------
	printf("slip send len = %d: ",slip_frame.tx_out_cnt);
	for( i = 0; i < slip_frame.tx_out_cnt; i++ )
	{
		printf(" %d ",slip_frame.tx_out_buf[i]);
	}
	printf("\n\r"); 
#endif	
	//-----------------debug--------------	

	push_serial_data(&ttyS0_sti, slip_frame.tx_out_buf, slip_frame.tx_out_cnt);
	//write_serial_data_direct( &ttyS0_sti, slip_frame.tx_out_buf, slip_frame.tx_out_cnt);
	
	#else
	push_serial_data(&ttyS0_sti, data, len);
	#endif
	
	return 0;
}

int SendUartAck( void )
{
	char buf[10];
	
	buf[0] = 0xc0;
	buf[1] = MSG_ID_ACK;
	buf[2] = MSG_ID_ACK;
	buf[3] = buf[1]+buf[2];
	buf[4] = 0xc0;
	
	return write_serial_data_direct( &ttyS0_sti, buf, 5);
}

/*******************************************************************************************
 * @fn:		vdp_uart_send_data
 *
 * @brief:	串口发送数据包处理
 *
 * @param:  	*data	- 数据区指针
 * @param:  	len		- 数据包长度
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int vdp_uart_send_data(char *data, int len)
{
#ifdef VDP_PRINTF_DEBUG	
	//-----------------debug--------------
	int i;
	unsigned char* ptrDat;;
	ptrDat =(unsigned char*) data;
	printf("uart send len = %d: ",len);
	for( i = 0; i < len; i++ )
	{
		printf(" %d ",ptrDat[i]);	
	}
	printf("\n\r");	
#endif	
	//-----------------debug--------------	
	push_vdp_common_queue(&vdp_uart_sending_queue,data,len);
	return 0;
}

// 得到时间间隔
// struct timeval *plast_time - 系统时间变量
// int start_calc - 0/登记当前的系统时间，1/比较当前的系统时间
// return - us为单位的时间间隔
unsigned int get_sys_time_delay( struct timeval *plast_time, int start_calc )
{
	if( !start_calc )
	{
		gettimeofday(plast_time,NULL);
		return 0;
	}
	else
	{
		struct timeval cur_time;
		unsigned int time_delay;
		gettimeofday(plast_time,NULL);
		time_delay = 1000000*(cur_time.tv_sec - plast_time->tv_sec) + (cur_time.tv_usec - plast_time->tv_usec);
		return time_delay;
	}
}
/*******************************************************************************************
 * @fn:		vdp_uart_recv_data
 *
 * @brief:	串口接收数据包处理
 *
 * @param:  	buf - 数据指针
 * @param:  	len - 数据长度
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int vdp_uart_recv_data( char* buf, int len)
{
	//checksum
	unsigned char checksum;
	checksum = UartCheckSum(buf,len-1);
	
	bprintf("uart recv one pack,len=%d,cks=%d\n",len,checksum);	
	
	if( buf[len-1] != checksum )
	{
		eprintf("----checksum err = %d, ok = %d-----\n", checksum,buf[len-1]);
		return -1;
	}
	//checksum ok

	bprintf("uart recv one pack,cks ok!\n");	
	PrintCurrentTime(10);

	api_uart_recv_callback(buf, len);

	return 0;
}

/*******************************************************************************************
 * @fn:		close_vdp_uart
 *
 * @brief:	关闭串口即相关资源
 *
 * @param:  	none
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int close_vdp_uart(void)
{
	stop_serial_task(&ttyS0_sti);
	return 0;
}

// 接收到串口数据包的回调函数 - 发送到survey处理
int api_uart_recv_callback( char* pbuf, unsigned int len )
{
	UART_MSG_TYPE msg;

	msg.head.msg_target_id	= MSG_ID_survey;
	msg.head.msg_source_id 	= MSG_ID_UART;
	msg.head.msg_type 		= 0;
	msg.head.msg_sub_type 	= 0;
	msg.len 				= len;
	memcpy( msg.pbuf, pbuf, len );

	bprintf("uart recv one cmd,len=%d!\n",len);	

	return push_vdp_common_queue(task_control.p_msg_buf, (char*)&msg, sizeof(VDP_MSG_HEAD)+sizeof(int)+len );	
}

//czn_20160601
/*******************************************************************************************
 * @fn:		API_Send_BusinessRsp_ByUart
 *
 * @brief:	发送业务应答
 *
 * @param:  	
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int API_Send_BusinessRsp_ByUart(unsigned char  target_id,unsigned char  source_id,unsigned short cmd_type,unsigned char *buf,int len)
{
	unsigned char *msg_buf = (uint8*)malloc(len+4);
	
	if(msg_buf == NULL)
	{
		return -1;
	}
	msg_buf[0] = target_id;
	msg_buf[1] = source_id;
	msg_buf[2] = cmd_type >> 8;
	msg_buf[3] = cmd_type;
	
	if(len > 0 && buf != NULL)
	{
		memcpy(msg_buf+4,buf,len);
	}
	else
	{
		len = 0;
	}
	
	vdp_uart_send_data((char*)msg_buf,len + 4);
	free(msg_buf);
	return 0;
}
