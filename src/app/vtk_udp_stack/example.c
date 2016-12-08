
#ifdef 	EXAMPLE_EANBLE


#include "example.h"

example_instance_t		one_example_ins;

Loop_vdp_common_buffer	example_msg_queue;
int 					example_process_pid;

void* 	example_msg_process( void* arg );
int 	example_udp_recv_anaylasis( char* buf, int len );
int 	example_inner_recv_anaylasis( char* buf, int len );

int init_example_instance( void )
{
	// init state
	one_example_ins.state			= exp_idle;
	one_example_ins.send_cmd_sn 	= 0;	
	
	// init udp
	init_one_udp_comm_rt_buff( &one_example_ins.udp, 500,		example_udp_recv_anaylasis, 500, NULL );
	init_one_udp_comm_rt_type( &one_example_ins.udp, "example", UDP_RT_TYPE_UNICAST,EXAMPLE_RCV_PORT, EXAMPLE_TRS_PORT,NULL );	
	
	// init business rsp wait array
	init_one_send_array(&one_example_ins.waitrsp_array);

	// init inner msg queue & process task
	init_vdp_common_queue(&one_example_ins,NULL,(msg_process)example_inner_recv_anaylasis,NULL);
	if( pthread_create(&example_process_pid, 0, example_msg_process, &example_msg_queue) )
	{
		ext_printf("Create task thread Failure,%s\n", strerror(errno));
	}
	ext_printf("init_one_example ok!\n");	

	return 0;
}

#define EXAMPLE_POLLING_MS		100
void* example_msg_process(void* arg )
{
	p_Loop_vdp_common_buffer	pmsgqueue 	= (p_Loop_vdp_common_buffer)arg;
	p_vdp_common_buffer pdb 	= 0;

	while( 1 )
	{	
		// 内部消息队列处理
		if( (pdb = pop_vdp_common_queue( pmsgqueue,0)) != 0  )
		{
			(*pmsgqueue->process)(pdb->msg_data,pdb->len);
			purge_vdp_common_queue( pmsgqueue );
		}
		if( sem_wait_ex(&pmsgqueue->sem, EXAMPLE_POLLING_MS) == -1 )
		{
			ext_printf("sem_wait_ex failure!msg:%s\n", strerror(errno));
		}
		// 100ms定时查询
		poll_all_business_recv_array( &one_example_ins.waitrsp_array, EXAMPLE_POLLING_MS );
	}	
	return 0;	
}

// inner接收命令业务分析
int example_inner_recv_anaylasis( char* buf, int len )
{
	return 0;
}

// udp接收命令业务分析
int example_udp_recv_anaylasis( char* buf, int len )
{
	return 0;
}


example_msg_type example_req( int server_ip, int para1, int para2 )
{
	example_req_pack	sender; 

	// 命令组包
	sender.target.ip		= server_ip;
	sender.target.cmd		= exp_cmd_req;
	sender.target.id		= ++one_example_ins.send_cmd_sn;
	sender.para1			= para1;
	sender.para2			= para2;

	one_example_ins.msg = exp_none;

	// 加入等待业务应答队列	
	sem_t *pwaitrsp_sem = join_one_send_array(&one_example_ins.waitrsp_array,sender.target.id, 
													sender.target.cmd+1,BUSINESS_RESEND_TIME, 0, NULL, 0 );
	// 发送数据
	sem_t *presend_sem = one_udp_comm_trs_api( &one_example_ins.udp, sender.target.ip, sender.target.cmd, sender.target.id, 
			1, (char*)&sender+sizeof(target_head), sizeof(example_req_pack)-sizeof(target_head));

	// 等待服务器通信应答
	if( sem_wait_ex2(presend_sem, ACK_RESPONSE_TIMEOUT) != 0 )
	{
		one_example_ins.msg	= exp_timeout;		
		return one_example_ins.msg;
	}
		
	// 等待服务器业务回复2s
	if( sem_wait_ex2(pwaitrsp_sem, BUSINESS_WAIT_TIMEPUT) != 0 )
	{
		one_example_ins.msg	= exp_no_rsp;		
		return one_example_ins.msg;
	}

	if( one_example_ins.msg == exp_ok )
		ext_printf("example_req ok!\n" );
	else
		ext_printf("example_req er!\n" );

	return one_example_ins.msg;
}

example_msg_type example_rsp( int client_ip, example_req_pack* plinkreq, int result)
{
	example_rsp_pack	sender; 

	// 命令组包
	sender.target.ip		= client_ip;
	sender.target.cmd		= exp_cmd_req;
	sender.target.id		= plinkreq->target.id;
	sender.result			= result;

	one_example_ins.msg = exp_none;

	// 发送数据
	sem_t *presend_sem = one_udp_comm_trs_api( &one_example_ins.udp, sender.target.ip, sender.target.cmd, sender.target.id, 
			1, (char*)&sender+sizeof(target_head), sizeof(example_rsp_pack)-sizeof(target_head));

	// 等待服务器通信应答
	if( sem_wait_ex2(presend_sem, 2000) != 0 )
	{
		one_example_ins.msg	= exp_timeout;		
		return one_example_ins.msg;
	}
		
	if( one_example_ins.msg == exp_ok )
		ext_printf("example_rsp ok!\n" );
	else
		ext_printf("example_rsp er!\n" );

	return one_example_ins.msg;
}

#endif

