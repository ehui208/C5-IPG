
#include "../task_io_server/task_IoServer.h"
#include "vtk_udp_stack_io_server.h"

udp_io_server_instance_t		one_udp_io_server_ins;

Loop_vdp_common_buffer			udp_io_server_msg_queue;
pthread_t						udp_io_server_process_pid;
void* 							udp_io_server_msg_process( void* arg );
int 							udp_io_server_inner_recv_anaylasis( char* buf, int len );

extern int send_one_udp_data( int sock_fd, struct sockaddr_in sock_target_addr, char *data, int length);
extern int push_udp_common_queue( p_loop_udp_common_buffer pobj, p_udp_common_buffer data, unsigned char length);
extern int pop_udp_common_queue(p_loop_udp_common_buffer pobj, p_udp_common_buffer* pdb, int timeout);
extern int purge_udp_common_queue(p_loop_udp_common_buffer pobj);

int udp_io_server_udp_recv_anaylasis( char* buf, int len );

// udp通信初始化
int init_udp_io_server_instance( void )
{
	
	init_one_udp_comm_rt_buff( 	&one_udp_io_server_ins.udp, 500,	udp_io_server_udp_recv_anaylasis, 500, NULL );
	init_one_udp_comm_rt_type( 	&one_udp_io_server_ins.udp, "udp_io_server", UDP_RT_TYPE_UNICAST,UDP_IO_SERVER_UDP_PORT, UDP_IO_SERVER_UDP_PORT,NULL );

	// init business rsp wait array
	init_one_send_array(&one_udp_io_server_ins.waitrsp_array);
	
 	init_vdp_common_queue(&udp_io_server_msg_queue,100,(msg_process)udp_io_server_inner_recv_anaylasis,NULL);	
 	if( pthread_create(&udp_io_server_process_pid, 0, udp_io_server_msg_process, &udp_io_server_msg_queue) )
	{
		udp_io_server_printf("Create task thread Failure,%s\n", strerror(errno));
	}
	
	udp_io_server_printf("init_one_device_update ok!\n");	

	return 0;
}

#define UDP_IO_SERVER_POLLING_MS		200		// lzh_20161022  //100
void* udp_io_server_msg_process(void* arg )
{
	p_Loop_vdp_common_buffer	pmsgqueue 	= (p_Loop_vdp_common_buffer)arg;
	p_vdp_common_buffer pdb 	= 0;

	while( 1 )
	{	
		int size;
		size = pop_vdp_common_queue( pmsgqueue,&pdb,UDP_IO_SERVER_POLLING_MS );
		if( size > 0 )
		{
			(*pmsgqueue->process)(pdb,size);
			purge_vdp_common_queue( pmsgqueue );
		}
		// 100ms定时查询
		poll_all_business_recv_array( &one_udp_io_server_ins.waitrsp_array, UDP_IO_SERVER_POLLING_MS );
	}	
	return 0;	
}

// inner接收命令业务分析
int udp_io_server_inner_recv_anaylasis( char* buf, int len )
{
	return 0;
}

// udp接收命令
int udp_io_server_udp_recv_anaylasis( char* buf, int len )
{	
	target_head	*ptarget = (target_head*)buf;
	
	// 1、匹配到业务应答的数据包通过触发信号同步到任务的等待队列中
	// 2、未匹配到业务应答的数据包发送给系统survey处理
	if( trig_one_business_recv_array( &one_udp_io_server_ins.waitrsp_array, ptarget->id, ptarget->cmd,buf+sizeof(target_head), len-sizeof(target_head) ) < 0)
	{
		api_udp_io_serverc_recv_callback( ptarget->ip, ptarget->cmd, ptarget->id, buf+sizeof(target_head), len-sizeof(target_head) );
	}

	return 0;
}

// 通过udp:25008端口发送数据，不等待业务应答
int api_udp_io_server_send_data( int target_ip, int cmd, const char* pbuf, unsigned int len )
{
	// 发送数据
	sem_t *presend_sem = one_udp_comm_trs_api( &one_udp_io_server_ins.udp, target_ip, cmd, ++one_udp_io_server_ins.send_cmd_sn, 1, (char*)pbuf, len);
	// 等待通信应答
	if(presend_sem > 0)
	{
		if( sem_wait_ex2(presend_sem, ACK_RESPONSE_TIMEOUT) != 0 )
		{
			return -1;
		}
	}
	return 0;
}

// 通过udp:25008端口发送数据包后，并等待业务应答，得到业务应答的数据
int api_udp_io_server_send_req(int target_ip, int cmd, char* psbuf, int slen , char *prbuf, unsigned int *prlen)
{	
	int send_cmd_sn = ++one_udp_io_server_ins.send_cmd_sn;
	
	// 加入等待业务应答队列	 - 等待接收的命令和序列号需要和发送的命令和序列号先联系
	sem_t *pwaitrsp_sem = join_one_business_recv_array( &one_udp_io_server_ins.waitrsp_array,send_cmd_sn, cmd+0x80,BUSINESS_RESEND_TIME, prbuf, prlen );
	
	// 加入数据发送队列，等待ack应答
	sem_t *presend_sem = one_udp_comm_trs_api( &one_udp_io_server_ins.udp, target_ip, cmd, send_cmd_sn, 1, psbuf, slen);

	if( pwaitrsp_sem > 0 && presend_sem > 0 )
	{
		// 等待服务器通信应答
		if( sem_wait_ex2(presend_sem, ACK_RESPONSE_TIMEOUT) != 0 )
		{
			dele_one_business_recv_array( &one_udp_io_server_ins.waitrsp_array, send_cmd_sn, cmd+0x80 );
			bprintf("S_UUDP:NO_COMMU_ACK\n");
			return -1;
		}
		// 等待binuess应答	
		if( sem_wait_ex2(pwaitrsp_sem, BUSINESS_WAIT_TIMEPUT+100) != 0 )
		{
			bprintf("S_UUDP:NO_BUSINESS_ACK\n");
			return -1;
		}
	}
	else
	{
		bprintf("S_UUDP:SEM_ERR\n");
		return -1;
	}
	if( prlen == NULL )
		bprintf("S_UUDP:HAVE_BUSINESS_ACK_PKT size %d\n" ,0);
	else
		bprintf("S_UUDP:HAVE_BUSINESS_ACK_PKT size %d\n" ,*prlen);	

	return 0;
}

// 接收到udp:25007端口数据包后给出的业务应答
int api_udp_io_server_send_rsp( int target_ip, int cmd, int id, const char* pbuf, unsigned int len )
{	
	// 发送数据
	sem_t *presend_sem = one_udp_comm_trs_api( &one_udp_io_server_ins.udp, target_ip, cmd, id, 1, (char*)pbuf, len);

	// 等待通信应答
	if(presend_sem > 0)
	{
		if( sem_wait_ex2(presend_sem, ACK_RESPONSE_TIMEOUT) != 0 )
		{
			return -1;
		}
	}
	return 0;
}


// 接收到udp:25000端口数据包的回调函数 - 发送到survey处理
int api_udp_io_serverc_recv_callback( int target_ip, int cmd, int sn, char* pbuf, unsigned int len )
{
	return -1;
}

