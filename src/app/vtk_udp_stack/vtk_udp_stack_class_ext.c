

#include "vtk_udp_stack_class_ext.h"

extern int send_one_udp_data( int sock_fd, struct sockaddr_in sock_target_addr, char *data, int length);
extern int push_udp_common_queue( p_loop_udp_common_buffer pobj, p_udp_common_buffer data, int length);
extern int pop_udp_common_queue(p_loop_udp_common_buffer pobj, p_udp_common_buffer* pdb, int timeout);
extern int purge_udp_common_queue(p_loop_udp_common_buffer pobj);

/*
 功能：
	初始化一个udp实例: ID号，类型，端口号，目标地址
 参数：
	pins		- udp 收发实例指针
	id			- udp 实例id
	type			- udp 类型：0/点播，1/组播，2/广播
	rport		- udp 接收绑定的端口号
	tport		- udp 发送的端口号
	target_pstr	- udp 发送目标地址
*/
int init_one_udp_comm_rt_type_ext( udp_comm_rt* pins,  char* pname, int type, unsigned short rport, unsigned short tport, char* target_pstr)
{
	// 得到本机地址
	pins->local_addr 	= GetLocalIp();
	// global para initial
	pins->pname			= pname;
	pins->rport			= rport;
	pins->tport			= tport;
	pins->type			= type;
	pins->target_pstr	= target_pstr;
	
	// trs initial
	pins->sock_trs_fd = create_trs_udp_socket();
	if( pins->sock_trs_fd == -1 )
	{
		EXT_printf("create_trs_udp_socket Failure,%s\n", strerror(errno));
		return -1;		
	}
	
	if( pins->type == UDP_RT_TYPE_BROADCAST	)	// 允许套接口传送广播信息
	{
    	int so_broadcast = 1;	
	    if( setsockopt( pins->sock_trs_fd, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof(so_broadcast) ) == -1 )
		{
			EXT_printf("setsockopt failure!msg:%s\n", strerror(errno));
			return -1;		
		}
	}
	
	pins->tmsg_run_flag	= 0;
	if( pins->tmsg_buf.pQBuf != NULL )
	{
		if( pthread_create(&pins->tmsg_pid, 0, udp_trs_data_task_thread_ext, pins) )
		{
			EXT_printf("Create udp_trs_data_task_thread Failure,%s\n", strerror(errno));
			return -1;
		}
	}	
	// rcv initial
	if( (pins->sock_rcv_fd = create_rcv_udp_socket(NET_DEVICE_NAME,pins->rport,0))  == -1 )
	{
		EXT_printf("create_rcv_udp_socket failure!msg:%s\n", strerror(errno));
		return -1;
	}

	if( pins->type == UDP_RT_TYPE_MULTICAST && pins->target_pstr != NULL )
	{
		if( join_multicast_group(NET_DEVICE_NAME, pins->sock_rcv_fd, inet_addr(pins->target_pstr) ) == -1 )
		{
			EXT_printf("add multicast group failure!msg:%s\n", strerror(errno));
			return -1;
		}		
	}
	pins->trs_tar_addr.sin_family 		= AF_INET;  
	pins->trs_tar_addr.sin_addr.s_addr 	= inet_addr(pins->target_pstr);
	pins->trs_tar_addr.sin_port 		= htons( pins->tport );		

	pins->rcv_run_flag 	= 0;
	if( pthread_create(&pins->rcv_pid, 0, udp_rcv_data_task_thread_ext, pins) )
	{
		EXT_printf("Create udp_rcv_data_task_thread Failure,%s\n", strerror(errno));
		return -1;
	}
	
	pins->rmsg_run_flag	= 0;
	if( pins->rmsg_buf.pQBuf != NULL )
	{
		if( pthread_create(&pins->rmsg_pid, 0, udp_rcv_data_process_task_thread_ext, pins) )
		{
			EXT_printf("Create udp_rcv_data_process_task_thread Failure,%s\n", strerror(errno));
			return -1;
		}		
	}	

	// 初始化重发队列
	init_one_send_array(&pins->resend_array);	
	
	return 0;
}

// checksum的范围: 不包括 head，target.ip, 从target.cmd开始的累加和，checksum为最后一个字节，长度为包括checksum内的所有字节
int create_one_send_pack_ext( udp_comm_rt* pins, int target_ip, char* pbuf, int len )
{
	struct sockaddr_in trs_tar_addr;
	trs_tar_addr.sin_family 		= AF_INET;
	trs_tar_addr.sin_port			= pins->trs_tar_addr.sin_port;
	trs_tar_addr.sin_addr.s_addr	= target_ip;

	EXT_printf("send ip=%08x, len=%d\n",target_ip,len );
	
	send_one_udp_data( pins->sock_trs_fd, trs_tar_addr, pbuf,len);

	return len;
}

void *udp_trs_data_task_thread_ext(void *arg)
{
	int size;
	udp_comm_rt* pins = (udp_comm_rt*)arg;

	p_udp_common_buffer pdb;
	
	pins->tmsg_run_flag = 1;
			
	EXT_printf("udp_trs_data_task_thread = %s\n",pins->pname );
		
	do
	{
		size = pop_udp_common_queue( &pins->tmsg_buf, &pdb, 0);
		if( size > 0 )
		{
			// send process
			ext_pack_buf* ppack_buf = (ext_pack_buf*)pdb;
			create_one_send_pack_ext( pins, ppack_buf->ip, ppack_buf->dat, ppack_buf->len );
			// clear 
			purge_udp_common_queue( &pins->tmsg_buf );
		}	
	}
	while(pins->tmsg_run_flag);

	EXT_printf("udp_trs_data_task_thread exit...!\n"); 

	return (void *)0;
}

#define RCV_BUFFER_MAX	1000
#define RCV_TIMEOUT_GAP	100		//ms
void *udp_rcv_data_task_thread_ext(void *arg)
{
	int recvcnt;
	udp_comm_rt* pins = (udp_comm_rt*)arg;

	unsigned int addr_len = sizeof(struct sockaddr_in);
	char buffer[RCV_BUFFER_MAX];

	fd_set fds;
	struct timeval tv={1,0};
	int ret;
	
	pins->rcv_run_flag = 1;
	
	EXT_printf("udp_rcv_data_task_thread = %s, socket = %d\n",pins->pname, pins->sock_rcv_fd );

	do
	{
		FD_ZERO(&fds);
		FD_SET(pins->sock_rcv_fd,&fds);
		
		tv.tv_sec = 0;
		tv.tv_usec = RCV_TIMEOUT_GAP*1000;	// 100ms
		ret = select( pins->sock_rcv_fd + 1, &fds, NULL, NULL, &tv );

		switch( ret )
		{
			case -1:
				udp_printf("socket err!\n");
				goto udp_rcv_data_task_thread_error2;
			case 0:
				//timeout
				break;
			default:
				if( FD_ISSET( pins->sock_rcv_fd, &fds ) )
				{
					recvcnt = recvfrom(pins->sock_rcv_fd, buffer, RCV_BUFFER_MAX,0, (struct sockaddr*)&pins->rcv_tar_addr,&addr_len);
					
					if( recvcnt == -1 )
					{
						EXT_printf("can not rec data from socket! errno:%d,means:%s\n",errno,strerror(errno));
					}
					else
					{
						// 判断是否自己发送的数据
						if( (pins->rcv_tar_addr.sin_addr.s_addr != pins->local_addr) )
						{
							// 业务处理
							if( pins->rmsg_buf.pQBuf != NULL )
							{
								ext_pack_buf target_buf;
								target_buf.ip	= pins->rcv_tar_addr.sin_addr.s_addr;
								target_buf.len	= recvcnt;
								memcpy( target_buf.dat, buffer, recvcnt );						
								push_udp_common_queue( &pins->rmsg_buf, (p_udp_common_buffer)&target_buf,sizeof(ext_pack_buf)-IP8210_BUFFER_MAX+recvcnt );
							}
							else
							{
								EXT_printf("push_udp_common_queue er,len=%d\n",recvcnt);									
							}
						}
					}					
				}
				break;
		}
		// polling resend array
		poll_all_send_array( pins, RCV_TIMEOUT_GAP );
	}
	while(pins->rcv_run_flag);

	EXT_printf("udp_rcv_data_task_thread exit...!\n");	

	return (void *)0;
	
	udp_rcv_data_task_thread_error2:

	return (void *)-1;
}

void *udp_rcv_data_process_task_thread_ext(void *arg)
{
	int size;
	udp_comm_rt* pins = (udp_comm_rt*)arg;

	p_udp_common_buffer pdb;
	
	pins->rmsg_run_flag = 1;
			
	EXT_printf("udp_rcv_data_process_task_thread = %s\n",pins->pname );
		
	do
	{
		size = pop_udp_common_queue( &pins->rmsg_buf, &pdb, RCV_TIMEOUT_GAP);
		if( size > 0 )
		{
			(*pins->rmsg_buf.process)(pdb,size);
			purge_udp_common_queue( &pins->rmsg_buf );
		}	
	}
	while(pins->rmsg_run_flag);

	EXT_printf("udp_rcv_data_process_task_thread exit...!\n"); 

	return (void *)0;
}


int poll_all_send_array_ext( udp_comm_rt* pins, int time_gap )
{
	int i;
	pthread_mutex_lock( &pins->resend_array.lock );
	for( i = 0; i < MAXf_SEND_SEM_ID_CNT; i++ )
	{
		if( pins->resend_array.dat[i].enable )
		{			
			// 时间递增
			pins->resend_array.dat[i].send_timeout_cnt += time_gap;
			// 超时移除等待发送队列
			if( pins->resend_array.dat[i].send_timeout_cnt >= pins->resend_array.dat[i].send_timeout )
			{
				pins->resend_array.dat[i].send_timeout_cnt = 0;
				
				if( pins->resend_array.dat[i].resend_times == 0 )
				{
					udp_printf("sem_post = %d,cmd=0x%04x,id= %d, timeout...\n",(int)&pins->resend_array.dat[i].trig_sem,pins->resend_array.dat[i].send_cmd, pins->resend_array.dat[i].send_id);
					
					pins->resend_array.dat[i].enable		= 0;
					pins->resend_array.dat[i].send_id		= 0;		
					pins->resend_array.dat[i].send_cmd 		= 0;		
					pins->resend_array.dat[i].send_timeout	= 0;	
					pins->resend_array.dat[i].len			= 0;					
					if( pins->resend_array.dat[i].pbuffer != NULL )
					{
						free(pins->resend_array.dat[i].pbuffer);
						pins->resend_array.dat[i].pbuffer = NULL;
					}
					//sem_post(&pins->resend_array.dat[i].trig_sem);			// 无需触发，调用者已挂起在信号量上，超时退出
				}
				else
				{
					pins->resend_array.dat[i].resend_times--;
					// resend process
					pack_buf*	ppack_buf = (pack_buf*)pins->resend_array.dat[i].pbuffer;
					create_one_send_pack_ext( pins, ppack_buf->target.ip, ppack_buf->dat, ppack_buf->len );
				}
			}
		}
	}
	pthread_mutex_unlock( &pins->resend_array.lock );
	return -1;	
}

