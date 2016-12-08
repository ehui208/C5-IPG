
#include "vtk_udp_stack_device_update.h"

device_update_instance_t		one_device_update_ins;

Loop_vdp_common_buffer			device_update_msg_queue;
pthread_t						device_update_process_pid;
void* 							device_update_msg_process( void* arg );
int 							device_update_inner_recv_anaylasis( char* buf, int len );

extern int send_one_udp_data( int sock_fd, struct sockaddr_in sock_target_addr, char *data, int length);
extern int push_udp_common_queue( p_loop_udp_common_buffer pobj, p_udp_common_buffer data, unsigned char length);
extern int pop_udp_common_queue(p_loop_udp_common_buffer pobj, p_udp_common_buffer* pdb, int timeout);
extern int purge_udp_common_queue(p_loop_udp_common_buffer pobj);

int device_update_udp_recv_anaylasis( char* buf, int len );

// udp通信初始化
int init_device_update_instance( void )
{
	
	init_one_udp_comm_rt_buff( 		&one_device_update_ins.udp, 500,	device_update_udp_recv_anaylasis, 500, NULL );
	init_one_udp_comm_rt_type_ext( 	&one_device_update_ins.udp, "device_update", UDP_RT_TYPE_MULTICAST,DEVICE_SEARCH_UDP_BOARDCAST_PORT, DEVICE_SEARCH_UDP_BOARDCAST_PORT,DEVICE_SEARCH_MULTICAST_ADDR );

	// init business rsp wait array
	init_one_send_array(&one_device_update_ins.waitrsp_array);
	
 	init_vdp_common_queue(&device_update_msg_queue,100,(msg_process)device_update_inner_recv_anaylasis,NULL);	
 	if( pthread_create(&device_update_process_pid, 0, device_update_msg_process, &device_update_msg_queue) )
	{
		dev_update_printf("Create task thread Failure,%s\n", strerror(errno));
	}
	
	dev_update_printf("init_one_device_update ok!\n");	

	return 0;
}

#define DEVICE_UPDATE_POLLING_MS		200		// lzh_20161022  // 100
void* device_update_msg_process(void* arg )
{
	p_Loop_vdp_common_buffer	pmsgqueue 	= (p_Loop_vdp_common_buffer)arg;
	p_vdp_common_buffer pdb 	= 0;

	while( 1 )
	{	
		int size;
		size = pop_vdp_common_queue( pmsgqueue,&pdb,DEVICE_UPDATE_POLLING_MS );
		if( size > 0 )
		{
			(*pmsgqueue->process)(pdb,size);
			purge_vdp_common_queue( pmsgqueue );
		}
		// 100ms定时查询
		poll_all_business_recv_array( &one_device_update_ins.waitrsp_array, DEVICE_UPDATE_POLLING_MS );
	}	
	return 0;	
}

// inner接收命令业务分析
int device_update_inner_recv_anaylasis( char* buf, int len )
{
	return 0;
}

// udp接收命令
int device_update_udp_recv_anaylasis( char* buf, int len )
{	
	ext_pack_buf *pbuf = (ext_pack_buf*)buf;

	device_search_package	*package	= (device_search_package*)pbuf->dat;
	int						dat_len		= len - sizeof(device_search_head)+sizeof(device_search_cmd);

	if( (package->head.s == 'S')||(package->head.w == 'W')||(package->head.i == 'I')||(package->head.t == 'T')||(package->head.c == 'C')||(package->head.h == 'H') )
	{		
		dev_update_printf( "receive ip=%08x,dat_len=%d,cmd=%04x\n", pbuf->ip,dat_len,htons(package->cmd.cmd));
		api_uddevice_update_recv_callback(pbuf->ip,htons(package->cmd.cmd),package->buf.dat,dat_len);
	}		
	return 0;
}


// 通过udp:25000端口发送数据，不等待业务应答
int api_udp_device_update_send_data( int target_ip, unsigned short cmd, const char* pbuf, unsigned int len )
{
	ext_pack_buf			send_pack;
	device_search_package*	psend_pack = (device_search_package*)send_pack.dat;	
	// initial head
	psend_pack->head.s 	= 'S';
	psend_pack->head.w 	= 'W';
	psend_pack->head.i 	= 'I';
	psend_pack->head.t 	= 'T';
	psend_pack->head.c 	= 'C';
	psend_pack->head.h 	= 'H';
	// cmd
	psend_pack->cmd.cmd	= cmd;
	// dat
	memcpy( psend_pack->buf.dat, pbuf, len );
	// len
	send_pack.len	= sizeof(device_search_head) + sizeof(device_search_cmd) + len;
	// ip
	send_pack.ip	= target_ip;
	
	// 加入发送队列
	push_udp_common_queue( &one_device_update_ins.udp.tmsg_buf, (p_udp_common_buffer)&send_pack, sizeof(ext_pack_buf)-IP8210_BUFFER_MAX+send_pack.len );

	return 0;
}

// 通过udp:25007端口发送数据包后，并等待业务应答，得到业务应答的数据
int api_udp_device_update_send_req(int target_ip, unsigned short cmd, char* psbuf, int slen , char *prbuf, unsigned int *prlen)
{	
	return 0;
}

// 接收到udp:25007端口数据包后给出的业务应答
int api_udp_device_update_send_rsp( int target_ip, unsigned short cmd, int id, const char* pbuf, unsigned int len )
{	
	return 0;
}

const char local_device_sn[10] = {0x00,0x03,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00};

int GetLocalMac( char* pMac ) 
{  
	int sock_get_mac;        
	struct ifreq ifr_mac;
	if( (sock_get_mac=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		return -1;  
	}      
	memset(&ifr_mac,0,sizeof(ifr_mac));     
	strncpy( ifr_mac.ifr_name, "eth0", sizeof(ifr_mac.ifr_name)-1);   
	if( ioctl( sock_get_mac, SIOCGIFHWADDR, &ifr_mac) < 0) 
	{  
	    return -1;
	}  	 
	pMac[0] = (char)ifr_mac.ifr_hwaddr.sa_data[0];  
	pMac[1] = (char)ifr_mac.ifr_hwaddr.sa_data[1];  
	pMac[2] = (char)ifr_mac.ifr_hwaddr.sa_data[2];  
	pMac[3] = (char)ifr_mac.ifr_hwaddr.sa_data[3];  
	pMac[4] = (char)ifr_mac.ifr_hwaddr.sa_data[4];  
	pMac[5] = (char)ifr_mac.ifr_hwaddr.sa_data[5];  
	return 0;
}  

//czn_20160812_s
int GetLocalGateway(void)
{
	FILE *file;
	char line[100] = {0},gateway[20]={0};
	char* ptr  = NULL;
	int addr_temp[4],ip = -1;
	
	
	
	if( ((file=fopen("/mnt/nand1-1/route.sh","r"))==NULL) )
	{
		eprintf( "GetLocalGateway error:%s\n",strerror(errno) );
		return -1;
	}

	//read line
	while( fgets(line,100,file) != NULL )
	{	
		// Invalid item ->delete
		if( strchr(line,'/') == NULL )
			continue;					

		// find set item
		if( (ptr = strchr( line, '#' )) == NULL )
		{
			// copy file
			
			continue;
		}
		
		
		// find gateway item
		if( memcmp(ptr,"#gateway",8) == 0 )
		{	
			// /sbin/route add default gw 192.168.1.1 dev eth0 #gateway
			if(sscanf(line,"%*s%*s%*s%*s%s",gateway) == 1)
			{
				bprintf("cutgateway:%s\n",gateway);
				if(sscanf(gateway,"%d%*c%d%*c%d%*c%d",&addr_temp[0],&addr_temp[1],&addr_temp[2],&addr_temp[3]) == 4)
				{
					ip = (addr_temp[3]<<24)|(addr_temp[2]<<16)|(addr_temp[1]<<8)|addr_temp[0];
				}
				else
				{
					
					eprintf( "GetLocalGateway error:%s\n",strerror(errno) );
				}
			}
			else
			{
				eprintf( "GetLocalGateway error:%s\n",strerror(errno) );
			}

			break;
		}

		
	}
	fclose( file );	

	return ip;
}

int GetLocalMaskAddr(void)
{
	char* pIP;
	int sock_get_ip;  
	int ip;
	struct   ifreq ifr_ip;     	
	if ((sock_get_ip=socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{  
		eprintf("socket create failse...GetLocalIp!\n");  
		return -1;  
	}  
	memset(&ifr_ip, 0, sizeof(ifr_ip));
	strncpy(ifr_ip.ifr_name, "eth0", sizeof(ifr_ip.ifr_name) - 1);     
	if( ioctl( sock_get_ip, SIOCGIFNETMASK, &ifr_ip) < 0 )     
	{     
		return -1;
	}       

	// 得到ip地址的字符串
	pIP = inet_ntoa(((struct sockaddr_in *)&ifr_ip.ifr_addr)->sin_addr);

	// 得到网络字节序IP 地址
	ip = htonl(inet_network(pIP) );

	close( sock_get_ip ); 
	
	return ip; 
}
//czn_20160812_e

// 接收到udp:25007端口数据包的回调函数
int api_uddevice_update_recv_callback( int target_ip, unsigned short cmd, char* pbuf, unsigned int len )
{
	device_search_read_req		*premote_device_req;
	device_search_write_req		*premote_device_w_req;

	device_search_read_rsp		local_device_info;
	device_search_write_rsp		write_device_info_rsp;

	struct in_addr temp;		//czn_20160812
	char ip_str[16],gw_str[16],mask_str[16],mac_str[19];

	switch( cmd )
	{
		case DEVICE_SEARCH_CMD_READ_REQ:
			premote_device_req = (device_search_read_req*)pbuf;
						
			memcpy( local_device_info.req_source_zero, premote_device_req->req_source_zero, 6);
			memcpy( local_device_info.req_target_zero, premote_device_req->req_target_zero, 6);
			local_device_info.req_source_ip 		= premote_device_req->req_source_ip;
			//czn_20160812_s
			local_device_info.rsp_target_ip 		= GetLocalIp();
			local_device_info.rsp_target_gateway	= GetLocalGateway();
			local_device_info.rsp_target_mask	= GetLocalMaskAddr();
			//czn_20160812_e

			GetLocalMac(local_device_info.rsp_target_mac);
			memcpy( local_device_info.rsp_target_sn, local_device_sn, sizeof(local_device_sn) );	

			dev_update_printf( "send ip=%08x, dat_len=%d,cmd=%04x\n %s %s", target_ip,sizeof(device_search_read_rsp),htons(DEVICE_SEARCH_CMD_READ_RSP),__DATE__,__TIME__);
			
			api_udp_device_update_send_data( target_ip, htons(DEVICE_SEARCH_CMD_READ_RSP), (char*)&local_device_info, sizeof(device_search_read_rsp) );
			break;

		case DEVICE_SEARCH_CMD_WRITE_REQ:
				
			dev_update_printf( "get write req: ip=%08x, dat_len=%d,cmd=%04x\n", target_ip,sizeof(device_search_read_req),htons(DEVICE_SEARCH_CMD_WRITE_REQ));
			
			premote_device_w_req = (device_search_write_req*)pbuf;
			write_device_info_rsp.rsp_target_new_ip 		= premote_device_w_req->req_target_new_ip;
			write_device_info_rsp.rsp_target_new_gateway	= premote_device_w_req->req_target_new_gateway;
			write_device_info_rsp.rsp_target_new_mask		= premote_device_w_req->req_target_new_mask;
			memcpy( write_device_info_rsp.rsp_target_new_mac, premote_device_w_req->req_target_new_mac, 6 );
			
			api_udp_device_update_send_data( target_ip, htons(DEVICE_SEARCH_CMD_WRITE_RSP), (char*)&write_device_info_rsp, sizeof(device_search_write_rsp) );							

			//czn_20160827
			temp.s_addr = premote_device_w_req->req_target_new_ip;  
			strncpy(ip_str,inet_ntoa(temp),15);
			temp.s_addr = premote_device_w_req->req_target_new_gateway;  
			strncpy(gw_str,inet_ntoa(temp),15);
			temp.s_addr = premote_device_w_req->req_target_new_mask;  
			strncpy(mask_str,inet_ntoa(temp),15);
			snprintf(mac_str,18,"%02x:%02x:%02x:%02x:%02x:%02x",premote_device_w_req->req_target_new_mac[0],
				premote_device_w_req->req_target_new_mac[1],premote_device_w_req->req_target_new_mac[2],premote_device_w_req->req_target_new_mac[3],
				premote_device_w_req->req_target_new_mac[4],premote_device_w_req->req_target_new_mac[5]);
			SetNetWork(mac_str,ip_str,mask_str,gw_str);
			
			dev_update_printf( "send ip=%08x, dat_len=%d,cmd=%04x\n", target_ip,sizeof(device_search_write_rsp),htons(DEVICE_SEARCH_CMD_WRITE_RSP));
			
			
			usleep(100000);
			ResetNetWork();
			  
			
			break;
	}
	return 0;
}



