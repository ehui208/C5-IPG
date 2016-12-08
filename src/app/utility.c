

//czn_20160708_s
#include "./task_survey/task_survey.h"	
#include "./task_survey/sys_msg_process.h"
#include "./task_io_server/obj_TableProcess.h"
#include "task_debug_sbu/task_debug_sbu.h"
//czn_20160708_e
#include "./task_io_server/obj_ResourceTable.h"		//czn_20160827

#include "utility.h"

extern int sem_timedwait(sem_t *, const struct timespec *);

/****************************************************************************************************************************
 * @fn:		init_vdp_common_queue
 *
 * @brief:	初始化vdp通用数据队列
 *
 * @param:  	pobj 			- 队列对象
 * @param:  	qsize 			- 队列大小
 * @param:  	powner 			- 队列的拥有者
 * @param:	   	process		   	- 队列的数据处理函数
 				
 *
 * @return: 	0/ok, -1/err
****************************************************************************************************************************/
int init_vdp_common_queue(p_Loop_vdp_common_buffer pobj, int qsize, msg_process process, void* powner )
{
	// create embOS queue
	pobj->QSize		= qsize;
	pobj->pQBuf		= (unsigned char*)malloc(qsize);
	OS_Q_Create( &pobj->embosQ, pobj->pQBuf, qsize );

	pobj->process	= process;
	pobj->powner 	= powner;

	if( pobj->pQBuf == NULL ) printf("malloc err!\n"); //else printf("malloc addr = 0x%08x\n",pobj->pQBuf);
		
	return 0;
}

/****************************************************************************************************************************
 * @fn:		exit_vdp_common_queue
 *
 * @brief:	释放一个队列
 *
 * @param:  	pobj 			- 队列对象 
 *
 * @return: 	0/ok, -1/err
****************************************************************************************************************************/
int exit_vdp_common_queue( p_Loop_vdp_common_buffer pobj )
{
	free( pobj->pQBuf );
	return 0;
}

/****************************************************************************************************************************
 * @fn:		push_vdp_common_queue
 *
 * @brief:	向vdp通用队列中加入消息类型
 *
 * @param:  	pobj 	- 队列对象
 * @param:  	data 	- 数据指针
 * @length:  	length 	- 数据长度
 *
 * @return: 	0/full, !0/current data pointer
****************************************************************************************************************************/
int push_vdp_common_queue( p_Loop_vdp_common_buffer pobj, char *data, unsigned char length)
{
	return OS_Q_Put (&pobj->embosQ, data, length);	//0: ok, 1: full
}

/****************************************************************************************************************************
 * @fn:		pop_vdp_common_queue
 *
 * @brief:	从队列中得到数据指针
 *
 * @param:  	pobj 		- 队列对象
 * @param:  	pdb 		- 数据指针
 * @param:  	timeout 	- 超时时间，ms为单位
 *
 * @return: 	size of data
****************************************************************************************************************************/
int pop_vdp_common_queue(p_Loop_vdp_common_buffer pobj, p_vdp_common_buffer* pdb, int timeout)
{
	if( !timeout )
		return OS_Q_GetPtr( &pobj->embosQ,  (void*)pdb );
	else
		return OS_Q_GetPtrTimed( &pobj->embosQ, (void*)pdb, timeout );
}

/****************************************************************************************************************************
 * @fn:		purge_vdp_common_queue
 *
 * @brief:	清除最近的队列
 *
 * @param:  	pobj 		- 队列对象
 *
 * @return: 	size of data
****************************************************************************************************************************/
int purge_vdp_common_queue(p_Loop_vdp_common_buffer pobj)
{
	OS_Q_Purge( &pobj->embosQ );
	return 0;
}

/****************************************************************************************************************************
 * @fn:		init_vdp_common_task
 *
 * @brief:	初始化一个任务
 *
 * @param:  	ptask 			- 任务对象
 * @param:  	task_id			- 任务ID
 * @param:  	pthread			- 任务服务线程服务函数
 * @param:	   	msg_buf		   	- 任务的消息队列 - 负责处理异步消息
 * @param:	   	syc_buf		   	- 任务的同步队列 - 负责处理同步消息
 *
 * @return: 	0/ok, -1/err
****************************************************************************************************************************/
int init_vdp_common_task( vdp_task_t* ptask, int task_id, void* (*pthread)(void*), p_Loop_vdp_common_buffer msg_buf, p_Loop_vdp_common_buffer syc_buf )
{
	ptask->task_id 			= task_id;
	ptask->p_msg_buf		= msg_buf;
	ptask->p_syc_buf		= syc_buf;
	ptask->task_pid			= 0;
	ptask->task_run_flag 	= 1;
	if( pthread  != NULL )
	{
		if( pthread_create(&ptask->task_pid, 0, pthread, ptask) != 0 )
		{
			eprintf("Create task thread Failure,%s\n", strerror(errno));
		}
		else
		{
			dprintf("Create task thread pid = %lu\n", ptask->task_pid);	
		}
	}	
	return 0;
}

/****************************************************************************************************************************
 * @fn:		exit_vdp_common_task
 *
 * @brief:	销毁一个任务
 *
 * @param:  	ptask 			- 任务对象
 *
 * @return: 	0/ok, -1/err
****************************************************************************************************************************/
int exit_vdp_common_task( vdp_task_t* ptask )
{
	ptask->task_run_flag = 0;
	pthread_join( ptask->task_pid, NULL );
	return 0;	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline int select_ex(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	int r;
	do
	{
		r = select(nfds, readfds, writefds, exceptfds, timeout);
	}while((-1 == r) && (EINTR == errno));
	return r;
}

inline int ioctlex(int fd, int request, void * arg)
{
	int r;
	do
	{
		r = ioctl(fd, request, arg);
	}while((-1 == r) && (EINTR == errno));
	return r;
}

int sem_wait_ex(sem_t *p_sem, int semto)
{
	struct timeval tv;
	struct timespec ts;
	int ret;
	
	if(semto > 0)
	{
		if(gettimeofday(&tv, 0) == -1)
		{
			printf("clock_gettime call failure!msg:%s\n", strerror(errno));
		}
		ts.tv_sec = (tv.tv_sec + (semto / 1000 ));
		ts.tv_nsec = (tv.tv_usec  + ((semto % 1000) * 1000)) * 1000;
		ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
		ts.tv_nsec %= 1000 * 1000 * 1000;
		
		while(((ret = sem_timedwait(p_sem, &ts)) != 0) && (errno == EINTR));
		if(errno == ETIMEDOUT)
		{
			ret = 0;
		}
	}
	else
	{
		while(((ret = sem_wait(p_sem)) != 0) && (errno == EINTR));
	}	
	return ret;
}

int sem_wait_ex2(sem_t *p_sem, int semto)
{
	struct timeval tv;
	struct timespec ts;
	int ret;
	
	if(semto > 0)
	{
		if(gettimeofday(&tv, 0) == -1)
		{
			printf("clock_gettime call failure!msg:%s\n", strerror(errno));
		}
		//printf("::sem_wait_ex2 %d=%d.%d\n\r",1000,tv.tv_sec,tv.tv_usec);	
		
		ts.tv_sec = (tv.tv_sec + (semto / 1000 ));
		ts.tv_nsec = (tv.tv_usec  + ((semto % 1000) * 1000)) * 1000;
		ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
		ts.tv_nsec %= 1000 * 1000 * 1000;
		
		ret = sem_timedwait(p_sem, &ts);
	}
	else
	{
		while(((ret = sem_wait(p_sem)) != 0) && (errno == EINTR));
	}	
	return ret;
}

int set_block_io(int fd, int is_block)
{
	int socket_flags;

	socket_flags = fcntl(fd, F_GETFL, 0);
	if(socket_flags < 0)
	{
		printf("Cant Not Get Socket Flags!\n");
		return -1;
	}
	if(is_block)
	{
		if(fcntl(fd, F_SETFL, (socket_flags & ~O_NONBLOCK)) < 0)
		{
			printf("Cant Not Set Socket Flags Block!\n");
			return -1;
		}
	}
	else
	{
		if(fcntl(fd, F_SETFL, (socket_flags | O_NONBLOCK)) < 0)
		{
			printf("Cant Not Set Socket Flags Non_Block!\n");
			return -1;
		}
	}
	return 0;
}

int get_format_time(char *tstr)
{
	int len;
	time_t t;
	t = time(NULL);	
	strcpy(tstr, ctime(&t));
	len = strlen(tstr);
	tstr[len-1] = '\0';
	return len;
}
      
#define MAC_BCAST_ADDR      (unsigned char *) "\xff\xff\xff\xff\xff\xff"   
#define ETH_INTERFACE       "eth0"   
  
struct arpMsg 
{  
	struct ethhdr ethhdr;       		/* Ethernet header */  
	unsigned short htype;              	/* hardware type (must be ARPHRD_ETHER) */  
	unsigned short ptype;              	/* protocol type (must be ETH_P_IP) */  
	unsigned char  hlen;               	/* hardware address length (must be 6) */  
	unsigned char  plen;               	/* protocol address length (must be 4) */  
	unsigned short operation;          /* ARP opcode */  
	unsigned char  sHaddr[6];          /* sender's hardware address */  
	unsigned char  sInaddr[4];         /* sender's IP address */  
	unsigned char  tHaddr[6];          	/* target's hardware address */  
	unsigned char  tInaddr[4];         /* target's IP address */  
	unsigned char  pad[18];            	/* pad for min. Ethernet payload (60 bytes) */  
};  
  
struct server_config_t 
{  
	unsigned int server;       		/* Our IP, in network order */  
	unsigned int start;        		/* Start address of leases, network order */  
	unsigned int end;          		/* End of leases, network order */  
	struct option_set *options; 	/* List of DHCP options loaded from the config file */  
	char *interface;       	 		/* The name of the interface to use */  
	int ifindex;            			/* Index number of the interface to use */  
	unsigned char arp[6];       		/* Our arp address */  
	unsigned long lease;        		/* lease time in seconds (host order) */  
	unsigned long max_leases;   	/* maximum number of leases (including reserved address) */  
	char remaining;         			/* should the lease file be interpreted as lease time remaining, or * as the time the lease expires */  
	unsigned long auto_time;    	/* how long should udhcpd wait before writing a config file.  * if this is zero, it will only write one on SIGUSR1 */  
	unsigned long decline_time;     	/* how long an address is reserved if a client returns a * decline message */  
	unsigned long conflict_time;    	/* how long an arp conflict offender is leased for */  
	unsigned long offer_time;   	/* how long an offered address is reserved */  
	unsigned long min_lease;    	/* minimum lease a client can request*/  
	char *lease_file;  
	char *pidfile;  
	char *notify_file;      			/* What to run whenever leases are written */  
	unsigned int siaddr;       		/* next server bootp option */  
	char *sname;            			/* bootp server name */  
	char *boot_file;        			/* bootp boot file option */  
};    
  
struct server_config_t server_config;
  
  
/*
interface 	- 网卡设备类型 接口
ifindex	- 检索索引 
addr		- 主机IP地址 
arp		- 主机arp地址
*/  
int read_interface(char *interface, int *ifindex, u_int32_t *addr, unsigned char *arp)  
{  
	int fd;  
	/*ifreq结构定义在/usr/include/net/if.h，用来配置ip地址，激活接口，配置MTU等接口信息的。 
	其中包含了一个接口的名字和具体内容——（是个共用体，有可能是IP地址，广播地址，子网掩码，MAC号，MTU或其他内容）。 
	ifreq包含在ifconf结构中。而ifconf结构通常是用来保存所有接口的信息的。 
	*/  
	struct ifreq ifr;  
	struct sockaddr_in *our_ip;  

	memset(&ifr, 0, sizeof(struct ifreq));  
	
	/*建立一个socket函数，SOCK_RAW是为了获取第三个参数的IP包数据，      IPPROTO_RAW提供应用程序自行指定IP头部的功能。    */  
	if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) 
	{  
		ifr.ifr_addr.sa_family = AF_INET;  
		
		/*将网卡类型赋值给ifr_name*/  
		strcpy(ifr.ifr_name, interface);  
  
		if (addr) 
		{  
			/*SIOCGIFADDR用于检索接口地址*/  
			if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) 
			{
				/*获取本机IP地址，addr是一个指向该地址的指针*/  
				our_ip = (struct sockaddr_in *) &ifr.ifr_addr;  
				*addr = our_ip->sin_addr.s_addr;  
				printf("%s (our ip) = %s\n", ifr.ifr_name, inet_ntoa(our_ip->sin_addr));  
			} 
			else 
			{  
				printf("SIOCGIFADDR failed, is the interface up and configured?: %s\n",  strerror(errno));  
				return -1;  
            		}  
		}
  
		/*SIOCGIFINDEX用于检索接口索引*/  
		if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) 
		{  
			printf("adapter index %d\n", ifr.ifr_ifindex);  
			/*指针ifindex 获取索引*/  
			*ifindex = ifr.ifr_ifindex;  
		} 
		else 
		{  
			printf("SIOCGIFINDEX failed!: %s\n", strerror(errno));  
			return -1;  
		}  
		
		/*SIOCGIFHWADDR用于检索硬件地址*/  
		if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) 
		{  
			/*所获取的硬件地址复制到结构server_config的数组arp[6]参数中*/  
			memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);  
			printf("adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x\n",  arp[0], arp[1], arp[2], arp[3], arp[4], arp[5]);  
		} 
		else 
		{  
			printf("SIOCGIFHWADDR failed!: %s\n", strerror(errno));  
			return -1;  
		}  
	}  
	else 
	{  
		printf("socket failed!: %s\n", strerror(errno));  
		return -1;  
	}  
	close(fd);  
	return 0;  
}  
  
  
/*
yiaddr 	- 目标IP地址
ip		- 本机IP地址
mac		- 本机mac地址
interface	- 网卡类型
*/  
int arpping(u_int32_t yiaddr, u_int32_t ip, unsigned char *mac, char *interface)  
{  
	int timeout 	= 2;  
	int optval 	= 1;  
	int s;                      		/* socket */  
	int rv 		= 0;                 		/* return value */  
	struct sockaddr 	addr; 	/* for interface name */  
	struct arpMsg 	arp;  
	fd_set 			fdset;  
	struct timeval 	tm;  
	time_t 			prevTime;
  
	/*socket发送一个arp包*/  
	if ((s = socket (PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP))) == -1) 
	{  
		eprintf("Could not open raw socket\n");  
		return -1;  
	}  
      
	/*设置套接口类型为广播，把这个arp包是广播到这个局域网*/  
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1) 
	{  
		eprintf("Could not setsocketopt on raw socket\n");  
		close(s);  
		return -1;  
	}
  
	/* 对arp设置，这里按照arp包的封装格式赋值即可，详见http://blog.csdn.net/wanxiao009/archive/2010/05/21/5613581.aspx */  
	memset(&arp, 0, sizeof(arp));  
	
	memcpy(arp.ethhdr.h_dest, MAC_BCAST_ADDR, 6);  	/* MAC DA */  
	memcpy(arp.ethhdr.h_source, mac, 6);        			/* MAC SA */  
	
	arp.ethhdr.h_proto 	= htons(ETH_P_ARP);     		/* protocol type (Ethernet) */  
	arp.htype 			= htons(ARPHRD_ETHER);        	/* hardware type */  
	arp.ptype 			= htons(ETH_P_IP);            		/* protocol type (ARP message) */
	arp.hlen 				= 6;                   				/* hardware address length */  
	arp.plen 				= 4;                   				/* protocol address length */  
	arp.operation 			= htons(ARPOP_REQUEST);       	/* ARP op code */  
	
	*((u_int *) arp.sInaddr) = ip;          					/* source IP address */  
	memcpy(arp.sHaddr, mac, 6);         					/* source hardware address */  
	*((u_int *) arp.tInaddr) = yiaddr;      					/* target IP address */  
  
	memset(&addr, 0, sizeof(addr));  
	strcpy(addr.sa_data, interface);  
	
	/*发送arp请求*/  
	if( sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0 ) 
	{
		eprintf("arp send error: %s\n", strerror(errno));  
		rv = 0;  
	}
	/* 利用select函数进行多路等待*/  
	tm.tv_usec = 0;
	time(&prevTime);
	while( timeout > 0 ) 
	{
		FD_ZERO(&fdset);
		FD_SET(s, &fdset);		
		tm.tv_sec = timeout;  
		
		if( select(s + 1, &fdset, (fd_set *) NULL, (fd_set *) NULL, &tm) < 0 ) 
		{  
			eprintf("Error on ARPING request: %s\n", strerror(errno));  
			if (errno != EINTR) rv = 0;  
		} 
		else if( FD_ISSET(s, &fdset) ) 
		{  
			if( recv(s, &arp, sizeof(arp), 0) < 0 )
			{
				dprintf("received one reply err.....\n");  
			}
			else			
			{
				dprintf("received one reply ok.....\n");  
			}
			/*如果条件 htons(ARPOP_REPLY) bcmp(arp.tHaddr, mac, 6) == 0 *((u_int *) arp.sInaddr) == yiaddr 三者都为真，则ARP应答有效,说明这个地址是已近存在的*/  
			if( (arp.operation == htons(ARPOP_REPLY)) &&  (bcmp(arp.tHaddr, mac, 6) == 0)  &&  (*((u_int *) arp.sInaddr) == yiaddr) ) 
			{  
				dprintf("Valid arp reply receved for this address\n");  
				rv = 1;  
				break;  
			}  
		}  
		timeout -= (time(NULL) - prevTime);
		time(&prevTime);  
	}  
	close(s);  
	return rv;  
}  
  
  
int check_ip(u_int32_t addr)  
{  
	struct in_addr temp;  
  
	if( arpping(addr, server_config.server, server_config.arp, ETH_INTERFACE) > 0 )  
	{  
		temp.s_addr = addr;  
		dprintf("%s belongs to someone, reserving it for %ld seconds\n",  inet_ntoa(temp), server_config.conflict_time);  
		return 1;  
	}  
	else  
	    return 0;  
}  
  
  
int check_ip_repeat( int ip )  
{         
	int ret = 0;

	/*读以太网接口函数，获取一些配置信息*/  
	if (read_interface(ETH_INTERFACE, &server_config.ifindex,  &server_config.server, server_config.arp) < 0)  
	{  
		exit(0);  
	}  
	  
	/*IP检测函数*/  
	ret = check_ip(ip);
	if( ret <= 0)  
	{  
		dprintf("IP:%08x can use\n", ip);   
	}  
	else  
	{  
		dprintf("IP:%08x conflict\n", ip);  
	} 	  
	return ret;  
}  

#define IP_STR_MAX_LENGTH	16
#define MAC_STR_MAX_LENGTH	18

int GetLocalIp(void)
{  
	char ip_addr[IP_STR_MAX_LENGTH];
	return GetLocalIpStr(ip_addr);
}  

int GetLocalIpStr( char* pIPStr )
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
	if( ioctl( sock_get_ip, SIOCGIFADDR, &ifr_ip) < 0 )     
	{     
		return -1;
	}       

	// 得到ip地址的字符串
	pIP = inet_ntoa(((struct sockaddr_in *)&ifr_ip.ifr_addr)->sin_addr);

	// 得到网络字节序IP 地址
	ip = htonl(inet_network(pIP) );

	memcpy( pIPStr, pIP, IP_STR_MAX_LENGTH);

	//printf("local ip str:%s \n",pIP);  

	close( sock_get_ip ); 
	
	return ip; 
}  

int GetLocalMacStr( char* pMacStr ) 
{  
	int sock_get_mac;        
	struct ifreq ifr_mac;  

	char mac_addr[MAC_STR_MAX_LENGTH];
      
	if( (sock_get_mac=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		eprintf("socket create failse...GetLocalMac!\n");  
		return -1;  
	}      
	memset(&ifr_mac,0,sizeof(ifr_mac));     
	strncpy( ifr_mac.ifr_name, "eth0", sizeof(ifr_mac.ifr_name)-1);   
	if( ioctl( sock_get_mac, SIOCGIFHWADDR, &ifr_mac) < 0) 
	{  
	    return -1;
	}  
      
	sprintf(mac_addr,"%02x:%02x:%02x:%02x:%02x:%02x",  
	        (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],  
	        (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],  
	        (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],  
	        (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],  
	        (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],  
	        (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);  

  	mac_addr[MAC_STR_MAX_LENGTH-1] = '\0';
	
	//printf("local mac str:%s \n",mac_addr);  
      
	close( sock_get_mac );  

	memcpy( pMacStr, mac_addr, MAC_STR_MAX_LENGTH);
	
	return 0;
}  

int ConvertIpStr2IpInt( const char* ipstr, int* ipint )
{
	struct in_addr s;  						// IPv4地址结构体
	inet_pton(AF_INET, ipstr, (void *)&s);
	*ipint = s.s_addr;
	return 0;
}

int ConvertIpInt2IpStr( int ipint, char* ipstr  )
{
	struct in_addr s;  						// IPv4地址结构体

	s.s_addr = ipint;

	inet_ntop(AF_INET, (void *)&s, ipstr, 16);
		
	return 0;
}

int create_trs_udp_socket(void)
{
	return socket(AF_INET, SOCK_DGRAM, 0);	
}


int create_rcv_udp_socket(char *net_dev_name, unsigned short prot, int block )
{
	int m_loop;
	int sockfd = -1;
	struct ifreq ifr;
	struct sockaddr_in self_addr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	m_loop = 1;
	if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &m_loop, sizeof(m_loop)) == -1 )
	{
		eprintf("Setsockopt SOL_SOCKET::SO_REUSEADDR Failure!\n");
		goto create_udp_socket_error;
	}
	
	memset(ifr.ifr_name, 0, IFNAMSIZ);
	sprintf(ifr.ifr_name, "%s", net_dev_name);
	if(setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(struct ifreq)))
	{
		eprintf("Setsockopt SOL_SOCKET::SO_BINDTODEVICE Failure!\n");
		goto create_udp_socket_error;
	}
	
	//Set Asynchronous IO
	if(set_block_io(sockfd, block))
	{
		eprintf("Set Asynchronous IO Failure!\n");
		goto create_udp_socket_error;
	}
	//Bind Self Address Port
	memset(&self_addr, 0, sizeof(struct sockaddr_in)); 
	self_addr.sin_family = AF_INET; 
	self_addr.sin_addr.s_addr = INADDR_ANY;
	self_addr.sin_port = htons(prot); 
	if(bind(sockfd, (struct sockaddr *)&self_addr, sizeof(struct sockaddr_in)) == -1)
	{     
		eprintf("bind call failure!msg:%s\n", strerror(errno));
		goto create_udp_socket_error;
	}
	
	//Dont Recv Loop Data
	#if 1
	m_loop = 1;
	if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &m_loop, sizeof(int)) == -1)
	{
		printf("Setsockopt IPPROTO_IP::IP_MULTICAST_LOOP Failure!\n");
		goto create_udp_socket_error;
	}
	#endif
	
 	return sockfd;
 	
create_udp_socket_error:
	if(sockfd != -1)
	{
		close(sockfd);
	}
	return -1;
}

int join_multicast_group(char *net_dev_name, int socket_fd, int mcg_addr)
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
	mreq.imr_multiaddr.s_addr = mcg_addr; //inet_addr(mcg_addr);
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

int join_multicast_group_ip(int socket_fd, int mcg_addr, int addr)
{
	struct ip_mreq mreq;

	//Added The Multicast Group
	memset(&mreq, 0, sizeof(struct ip_mreq));
	mreq.imr_multiaddr.s_addr = mcg_addr; 	//inet_addr(mcg_addr);
	mreq.imr_interface.s_addr = addr;		// inet_addr(addr);
	if(setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq)) == -1)
	{
		printf("Setsockopt IPPROTO_IP::IP_ADD_MEMBERSHIP Failure!\n");
		return -1;
	}	
	return 0;
}

int leave_multicast_group(char *net_dev_name, int socket_fd, int mcg_addr)
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
	mreq.imr_multiaddr.s_addr = mcg_addr; //inet_addr(mcg_addr);
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

int send_comm_udp_data( int sock_fd, struct sockaddr_in sock_target_addr, char *data, int length)
{
	int ret;

	ret = sendto(sock_fd, data, length, 0, (struct sockaddr*)&sock_target_addr,sizeof(struct sockaddr_in));
	
	if( ret == -1 )
	{
		eprintf("can not send data from socket! errno:%d,means:%s\n",errno,strerror(errno));
		return -1;
	}
	
	printf("send one package: sock = %d, ip = 0x%x, port = %d....\n",sock_fd,htonl(sock_target_addr.sin_addr.s_addr),htons(sock_target_addr.sin_port));
	
	return ret;
}

int PrintCurrentTime(int num)
{	
	struct timeval tv;	
	if( gettimeofday(&tv, 0) == -1 )		
		printf("clock_gettime call failure!msg:%s\n\r", strerror(errno));	
	else		
		//printf("::cur time %d=%d.%d\n\r",num,tv.tv_sec,tv.tv_usec);	
		printf("time=%d.%d\n\r",tv.tv_sec,tv.tv_usec); 
	return 0;
}


#if 0
/* Obtain a backtrace and print it to @code{stdout}. */
void print_trace (void)
{
	void *array[10];
	size_t size;
	char **strings;
	size_t i;
	size = backtrace(array,10);
	strings = backtrace_symbols(array, size);
	if( NULL == strings )
	{
		printf("backtrace_symbols");
	}
	else
	{
		printf( "Obtained %zd stack frames.\n", size);

		for (i = 0; i < size; i++)
			printf ("%s\n", strings[i]);

		free (strings);
		strings = NULL;
	}
}

static int backtrace_arm(void **BUFFER, int SIZE)
{
	volatile int n = 0;
	volatile int *p;
	volatile int *q;
	volatile int ebp1;
	volatile int eip1;
	volatile int i = 0;
	p = &n;
	ebp1 = p[4];
	eip1 = p[6];
	fprintf(stderr, "backtrace_arm addr: 0x%0x, param1: 0x%0x, param2: 0x%0x\n", backtrace_arm, &BUFFER, &SIZE);
	fprintf(stderr, "n addr is 0x%0x\n", &n);
	fprintf(stderr, "p addr is 0x%0x\n", &p);
	for (i = 0; i < SIZE; i++)
	{
		fprintf(stderr, "ebp1 is 0x%0x, eip1 is 0x%0x\n", ebp1, eip1);
		BUFFER[i] = (void *)eip1;
		p = (int*)ebp1;
		q = p - 5;
		eip1 = q[5];
		ebp1 = q[2];
		if (ebp1 == 0 || eip1 == 0)
			break;
	}
	fprintf(stderr, "total level: %d\n", i);
	return i;
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
#include <ucontext.h>
#include <dlfcn.h>

typedef struct
{
	const char *dli_fname;  /* File name of defining object.  */
	void *dli_fbase;        /* Load address of that object.  */
	const char *dli_sname;  /* Name of nearest symbol.比如函数名*/
	void *dli_saddr;        /* Exact value of nearest symbol.比如函数的起始地址*/
} Dl_info;

struct ucontext_ce123 
{
	unsigned long     	uc_flags;
	struct ucontext  	*uc_link;
	stack_t       		uc_stack;
	struct sigcontext 	uc_mcontext;
	sigset_t      		uc_sigmask;   /* mask last for extensibility */
}ucontext_ce123_;


struct sigframe_ce123 
{
	struct sigcontext 	sc;				//保存一组寄存器上下文
	unsigned long 		extramask[1];
	unsigned long 		retcode;//保存返回地址
	//struct aux_sigframe aux __attribute__((aligned(8)));
}sigframe_ce123;


int backtrace_ce123 (void **array, int size)
{
	if (size <= 0)
		return 0;
	
	int *fp = 0, *next_fp = 0;
	int cnt = 0;
	int ret = 0;
	
	__asm__(
	
		"mov %0, fp\n"
		: "=r"(fp)
	);
	array[cnt++] = (void *)(*(fp-1));
	next_fp = (int *)(*(fp-3));
	while((cnt <= size) && (next_fp != 0))
	{
		array[cnt++] = (void *)*(next_fp - 1);
		next_fp = (int *)(*(next_fp-3));
	}
	ret = ((cnt <= size)?cnt:size);
	printf("Backstrace (%d deep)\n", ret);
	return ret;	
}



char ** backtrace_symbols_ce123 (void *const *array, int size)
{
	#define WORD_WIDTH 8
	Dl_info info[size];
	int status[size];
	int cnt;
	size_t total = 0;
	char **result;

	/* Fill in the information we can get from `dladdr'.  */
	for (cnt = 0; cnt < size; ++cnt)
	{
		//status[cnt] = _dl_addr(array[cnt], &info[cnt]);
		status[cnt] = dladdr(array[cnt], &info[cnt]);
		if( status[cnt] && info[cnt].dli_fname && info[cnt].dli_fname[0] != '\0' )
			/* We have some info, compute the length of the string which will be"<file-name>(<sym-name>) [+offset].  */
			total += (strlen (info[cnt].dli_fname ?: "")
				+ (info[cnt].dli_sname ? strlen (info[cnt].dli_sname) + 3 + WORD_WIDTH + 3 : 1)
				+ WORD_WIDTH + 5);
		else
			total += 5 + WORD_WIDTH;
	}
	/* Allocate memory for the result.  */
	result = (char **) malloc (size * sizeof (char *) + total);
	if (result != NULL)
	{
		char *last = (char *) (result + size);
		for (cnt = 0; cnt < size; ++cnt)
		{
			result[cnt] = last;
			if (status[cnt] && info[cnt].dli_fname && info[cnt].dli_fname[0] != '\0')
			{
				char buf[20];
				if (array[cnt] >= (void *) info[cnt].dli_saddr)
					sprintf (buf, "+%#lx", \
						(unsigned long)(array[cnt] - info[cnt].dli_saddr));
				else
					sprintf (buf, "-%#lx", \
						(unsigned long)(info[cnt].dli_saddr - array[cnt]));
				
				last += 1 + sprintf (last, "%s%s%s%s%s[%p]",
					info[cnt].dli_fname ?: "",
					info[cnt].dli_sname ? "(" : "",
					info[cnt].dli_sname ?: "",
					info[cnt].dli_sname ? buf : "",
					info[cnt].dli_sname ? ") " : " ",
					array[cnt]);
			}
			else
				last += 1 + sprintf (last, "[%p]", array[cnt]);
		}
		assert (last <= (char *) result + size * sizeof (char *) + total);
	}
	
	return result;
}




/* SIGSEGV信号的处理函数，回溯栈，打印函数的调用关系*/
void DebugBacktrace(unsigned int sn , siginfo_t  *si , void *ptr)
{
	if(NULL != ptr)
	{
		printf("\n\nunhandled page fault (%d) at: 0x%08x\n", si->si_signo,(unsigned int)si->si_addr);
		
		struct ucontext_ce123 *ucontext = (struct ucontext_ce123 *)ptr;
		int pc = ucontext->uc_mcontext.arm_pc;
		
		void *pc_array[1]; 
		pc_array[0] = pc;
		char **pc_name = backtrace_symbols_ce123(pc_array, 1);
		printf("%d: %s\n", 0, *pc_name);
		
		#define SIZE 100
		void *array[SIZE];
		int size, i;
		char **strings;
		size = backtrace_ce123(array, SIZE);
		strings = backtrace_symbols_ce123(array, size); 

		if( strings != NULL )
		{
			for( i=0; i<size; i++) 
			{
				printf("==%s==\n",strings[i]);
			}
			free(strings);
			strings = NULL;
		}
	}
}

//lzh_20160704_s
char tftp_local_file[100];
char tftp_ip_str[20];
int start_tftp_download( int ip_addr, char* filename )
{
	FILE* fd   = NULL;
	char tftp_str[200]={'\0'};

	memset( tftp_local_file, 0, 200);
	memset( tftp_ip_str, 0, 20);
	
	ConvertIpInt2IpStr( ip_addr,tftp_ip_str );
	snprintf(tftp_local_file,100,"/mnt/nand1-2/tftp/%s",filename);
	
	snprintf(tftp_str,200,"tftp -g -r %s -l %s -b 512 %s",filename,tftp_local_file,tftp_ip_str);
	printf("start_tftp_download -- %s\n",tftp_str);

	if( (fd = popen( tftp_str, "r" )) == NULL )
	{
		printf( "start_tftp_download error:%s\n",strerror(errno) );
		return -1;
	}
	pclose( fd );
	printf("start_tftp_download -- over\n");

	return 0;
}

int get_tftp_file_checksum( int* pfile_len, int* pfile_cks )
{
	int len;
	int cks = 0;
	int i,j,blocks,remain,bytes;
	unsigned char buffer[512];
	// 计算checksum
	FILE *file;
	file = fopen(tftp_local_file, "rb");
	if( file == NULL )
	{
		return -1;
	}	
	fseek(file, 0, SEEK_END);
	len = ftell(file);

	blocks = len/512;
	remain = len%512;

	fseek(file, 0, SEEK_SET);
	
	for( i = 0; i < blocks+1; i++ )
	{
		bytes = ((i==blocks)?remain:512);
		fread(buffer, 1, bytes, file);
		for( j = 0; j < bytes; j++ )
		{
			cks += buffer[j];
		}
	}
	fclose(file);
	*pfile_len = len;
	*pfile_cks = cks;
	return 0;
}

//lzh_20160704_e

//czn_20160827
int start_updatefile_and_reboot(int ip_addr,char* tarfilename)
{

	char cmd_line[251];
	char filepath[81] = {0};

	int filenamelen,i;

	filenamelen = strlen(tarfilename);
	
	for(i = 0; i<filenamelen;i++)
	{
		if(tarfilename[filenamelen-1 - i] == '/')
			break;
	}
	
	if(strcmp(&tarfilename[filenamelen - 3],".gz") == 0)
	{
		snprintf(cmd_line,250,"tar xzvf %s -C /mnt/nand1-2/tftp/",tarfilename);
		memcpy(filepath,&tarfilename[filenamelen - i ],i-3);
	}
	else if(strcmp(&tarfilename[filenamelen - 4],".zip") == 0)
	{
		snprintf(cmd_line,250,"unzip -o %s -d /mnt/nand1-2/tftp/",tarfilename);
		memcpy(filepath,&tarfilename[filenamelen - i ],i-4);
	}
	else
	{
		remove(tarfilename);
	
		return -1;
	}

		
	if(system(cmd_line) != 0)
	{

		remove(tarfilename);
	
		return -1;
	}

	UpgradeIni_Parser_Stru ini_parser;
	snprintf(cmd_line,250,"/mnt/nand1-2/tftp/%s/upgrade.ini",filepath);
	if(UpgradeIni_Parser(&ini_parser,cmd_line) != 0)
	{

		remove(tarfilename);
	
		return -1;
	}
	
	if(ini_parser.n329fw_cnt > 0)
	{
		for(i = 0;i < ini_parser.n329fw_cnt; i++)
		{
			snprintf(cmd_line,250,"cp /mnt/nand1-2/tftp/%s/firmware/%s   %s",filepath,ini_parser.n329fw_Items[i].fwname,ini_parser.n329fw_Items[i].update_path);
			if(system(cmd_line) != 0)
			{
				UpgradeIni_Free(&ini_parser);

				remove(tarfilename);
				
				return -1;
			}
		}
	}
	if(ini_parser.reset_enble)
	{
		FILE *pfile = fopen("/mnt/nand1-2/tftp/HaveUpdated","wb");
		if(pfile == NULL)
		{
			UpgradeIni_Free(&ini_parser);

			remove(tarfilename);
			
			bprintf("creat update flag fail\n");
			return -1;
		}
		bprintf("serverip = %08x\n",ip_addr);
		fwrite(&ip_addr,4,1,pfile);
		fclose(pfile);
	}
	
	/*if(ini_parser.have_stm32fw && ini_parser.stm32fw_update_enble)
	{
		snprintf(cmd_line,250,"/mnt/nand1-2/tftp/%s/firmware/%s",filepath,ini_parser.stm32fwname);
		FILE *pstmbin = fopen(cmd_line,"rb");
		
		if(pstmbin != NULL)
		{
			if(Stm32FwUpdate(pstmbin) == -1)
			{
				fclose(pstmbin);
				//system("rm -r /mnt/nand1-2/tftp/DT-IPG.bin");
				UpgradeIni_Free(&ini_parser);
				return -1;
			}
			//system("rm -r /mnt/nand1-2/tftp/DT-IPG.bin");
			
			fclose(pstmbin);
			//exit(0);//czn_test
		}
	}
	else*/
	{
		if(ini_parser.reset_enble)
		{
			remove(tarfilename);
			usleep(50000);	
			if(system("reboot") != 0)
			{
				UpgradeIni_Free(&ini_parser);
				bprintf("reboot fail\n");
				return -1;
			}
		}
		else
		{
			UpgradeIni_Free(&ini_parser);
			remove(tarfilename);
			return 1; //reset_disable 
		}
	
	}
	
	UpgradeIni_Free(&ini_parser);
	
	remove(tarfilename);
	
	return 0;
}

extern Updater_Run_Stru 	Updater_Run; 

typedef enum
{
	Configfile_Unkown	= 0,
	Configfile_GatewayTb,
	Configfile_RoomTb,
	Configfile_CdsNlTb,
	Configfile_DsNlTb,
	Configfile_ImNlTb,
	Configfile_ImGlMapTb,
	Configfile_Ds1NlTb,
	Configfile_Ds2NlTb,
	Configfile_Ds3NlTb,
	Configfile_Ds4NlTb,
}Configfig_Type;

Configfig_Type Get_Configfig_Type(char *filename)
{

	if(strcmp(filename,"ip_map_table.csv") == 0)
	{
		return Configfile_GatewayTb;
	}
	if(strcmp(filename,"device_map_table.csv") == 0)
	{
		return Configfile_RoomTb;
	}
	if(strcmp(filename,"cds_namelist_table.csv") == 0)
	{
		return Configfile_CdsNlTb;
	}
	if(strcmp(filename,"ds_general_namelist_table.csv") == 0)
	{
		return Configfile_DsNlTb;
	}
	if(strcmp(filename,"im_namelist_table.csv") == 0)
	{
		return Configfile_ImNlTb;
	}
	if(strcmp(filename,"imglmap_table.csv") == 0)
	{
		return Configfile_ImGlMapTb;
	}
	if(strcmp(filename,"ds1_namelist_table.csv") == 0)
	{
		return Configfile_Ds1NlTb;
	}
	if(strcmp(filename,"ds2_namelist_table.csv") == 0)
	{
		return Configfile_Ds2NlTb;
	}
	if(strcmp(filename,"ds3_namelist_table.csv") == 0)
	{
		return Configfile_Ds3NlTb;
	}
	if(strcmp(filename,"ds4_namelist_table.csv") == 0)
	{
		return Configfile_Ds4NlTb;
	}
	return Configfile_Unkown;
}
#if 0
int Update_Configfile(void)
{
	int i,alltbupdate;
	char cmd_line[251];
	Configfig_Type cftype;
	one_vtk_table *ptable;
	
	for(i = 0;i < Updater_Run.configfile_cnt; i++)
	{
		snprintf(cmd_line,250,"cp /mnt/nand1-2/tftp/%s   /mnt/nand1-2/%s",Updater_Run.configfile[i],Updater_Run.configfile[i]);
		
		if(system(cmd_line) != 0)
		{
			return -1;
		}
	}
	alltbupdate = 0;
	for(i = 0;i < Updater_Run.configfile_cnt; i++)
	{
		cftype = Get_Configfig_Type(Updater_Run.configfile[i]);
		switch(cftype)
		{
			case Configfile_GatewayTb:
			case Configfile_RoomTb:
				if(alltbupdate == 0)
				{
					free_ip_device_table();
					load_ip_device_table();
					alltbupdate = 1;
				}
				break;
				
			case Configfile_CdsNlTb:
				
				break;
				
			case Configfile_DsNlTb:
				
				break;

			case Configfile_ImNlTb:
				
				break;
				
			case Configfile_ImGlMapTb:
				
				break;
			case Configfile_Ds1NlTb:
				
				break;
			case Configfile_Ds2NlTb:
				
				break;
			case Configfile_Ds3NlTb:
				
				break;
			case Configfile_Ds4NlTb:
				
				break;	
		}
	}
	return 0;
}
#endif

int IsHaveUpdate(int *ip_addr)
{
	FILE *pfile = fopen("/mnt/nand1-2/tftp/HaveUpdated","rb");

	if(pfile == NULL)
	{
		return -1;
	}

	fread(ip_addr,4,1,pfile);

	fclose(pfile);

	int err;
	usleep(50000);
	err = system("rm -r /mnt/nand1-2/tftp");
	bprintf("rm -r tftp result=%d\n",err);
	usleep(50000);
	err = system("mkdir /mnt/nand1-2/tftp");
	bprintf("mkdir tftp result=%d\n",err);
	return 0;
}




UpgradeIniKey_Type Judge_UpgradeIniKeyType(char *buff)
{
	if(strstr( buff, "[DEVICE TYPE]" ) != NULL)
	{
		return UpgradeIniKey_DeviceType;
	}

	if(strstr( buff, "[FIRMWARE VER]" ) != NULL || strstr( buff, "[FIREWARE VER]" ) != NULL)
	{
		return UpgradeIniKey_FwVer;
	}

	if(strstr( buff, "[FIRMWARE PARSER]" ) != NULL || strstr( buff, "[FIREWARE PARSER]" ) != NULL)
	{
		return UpgradeIniKey_FwParser;
	}

	if(strstr( buff, "[FIRMWARE RESET]" ) != NULL || strstr( buff, "[FIREWARE RESET]" ) != NULL)
	{
		return UpgradeIniKey_FwReset;
	}

	if(strstr( buff, "[STM32]" ) != NULL)
	{
		return UpgradeIniKey_Stm32;
	}
	
	if(strstr( buff, "[END]" ) != NULL)
	{
		return UpgradeIniKey_End;
	}
	
	return UpgradeIniKey_Unkown;
}

int Judge_IsSpaceLine(char *buff)
{
	while(*buff != 0)
	{
		if(*buff != ' ' && *buff != '\r'&&*buff != '\n')
		{
			return 0;
		}
		buff ++;
	}

	return 1;
}

int Delete_Linebreak(char *buff,int str_len)
{
	//bprintf("%s len=%d\n",buff,str_len);
	while(str_len > 0)
	{
		if(buff[str_len -1] == '\r' || buff[str_len -1] == '\n')
		{
			buff[str_len -1] = 0;
			str_len --;
			//bprintf("delete happen\n");
		}
		else
		{
			break;
		}
	}
	//bprintf("delete return len=%d\n",str_len);
	return str_len;
}

int Load_FwParserItems(UpgradeIni_Parser_Stru *ini_parser,char *buff,int str_len)
{
	int i,cpy_len;
	void	*p;
	
	//str_len = strlen(buff);
	for(i = 0;i < (str_len -1);i ++)
	{
		if(buff[i] == '=')
			break;
	}
	if(i < (str_len -1))
	{
		if(ini_parser->n329fw_cnt == 0)
		{
			p = malloc(sizeof(N329fw_Stru));
			if(p == NULL)
			{
				return -1;
			}
		}
		else
		{
			p = realloc(ini_parser->n329fw_Items,sizeof(N329fw_Stru)*(ini_parser->n329fw_cnt+1));

			if(p == NULL)
			{
				ini_parser->n329fw_cnt = 0;
				free(ini_parser->n329fw_Items);
				return -1;
			}
		}
		ini_parser->n329fw_Items = (N329fw_Stru *)p;
		memset(&ini_parser->n329fw_Items[ini_parser->n329fw_cnt],0,sizeof(N329fw_Stru));
		
		cpy_len = (i > (SERVER_FILE_NAME-1) ? (SERVER_FILE_NAME-1):i);
		memcpy(ini_parser->n329fw_Items[ini_parser->n329fw_cnt].fwname,buff,cpy_len);
		ini_parser->n329fw_Items[ini_parser->n329fw_cnt].fwname[cpy_len] = 0;
		strncpy(ini_parser->n329fw_Items[ini_parser->n329fw_cnt].update_path,&buff[i+1], (SERVER_FILE_NAME*2-1));

		for(i = 0;i < strlen(ini_parser->n329fw_Items[ini_parser->n329fw_cnt].update_path);i++)
		{
			if(ini_parser->n329fw_Items[ini_parser->n329fw_cnt].update_path[i] == '\\')
			{
				ini_parser->n329fw_Items[ini_parser->n329fw_cnt].update_path[i] = '/';
			}
		}
		
		ini_parser->n329fw_cnt ++;	
	}

	return 0;
}
int Load_Stm32UpdateItems(UpgradeIni_Parser_Stru *ini_parser,char *buff,int str_len)
{
	int i,item_type = 0;
	void	*p;
	
	if(strstr(buff,"fw")!=NULL)
	{
		item_type = 1;
	}
	else if(strstr(buff,"update")!=NULL)
	{
		item_type = 2;
	}
	if(item_type != 0)
	{
		for(i = 0;i < (str_len -1);i ++)
		{
			if(buff[i] == '=')
				break;
		}

		if(i < (str_len -1))
		{
			if(item_type == 1)
			{
				ini_parser->have_stm32fw = 1;
				strncpy(ini_parser->stm32fwname,&buff[i+1], SERVER_FILE_NAME-1);
			}
			else
			{
				if(strstr(&buff[i+1],"enable")!=NULL)
				{
					ini_parser->stm32fw_update_enble = 1;
				}
				else
				{
					ini_parser->stm32fw_update_enble = 0;
				}
			}
		}
	}

	return 0;
	
}

int UpgradeIni_Parser(UpgradeIni_Parser_Stru *ini_parser, char* inifilename)
{
	FILE  *pfile = NULL;
	char buff[101];
	int	str_len;

	if( (pfile=fopen(inifilename,"r")) == NULL )
	{
		return -1;
	}


	memset(ini_parser,0,sizeof(UpgradeIni_Parser_Stru));
	//UpgradeIniKey_DeviceType
	fseek( pfile, 0, SEEK_SET);
	for( memset(buff,0,101);fgets(buff,100,pfile) != NULL;memset(buff,0,101))
	{
		str_len = strlen(buff);
		if(str_len >= 100)
		{
			fclose(pfile);
			return -1;
		}
		
		if(str_len > 0)
		{
			if(Judge_IsSpaceLine(buff) != 0)
			{
				continue;
			}
			if(Judge_UpgradeIniKeyType(buff) == UpgradeIniKey_DeviceType)
			{
				for( memset(buff,0,101);fgets(buff,100,pfile) != NULL;memset(buff,0,101))
				{
					str_len = strlen(buff);
					
					if(str_len >= 100)
					{
						fclose(pfile);
						return -1;
					}
					
					if(str_len > 0)
					{
						str_len = Delete_Linebreak(buff,str_len);
						if(Judge_IsSpaceLine(buff) == 0)
						{
							if(Judge_UpgradeIniKeyType(buff) == UpgradeIniKey_Unkown)
							{
								strncpy(ini_parser->device_type,buff,DEVICE_TYPE_MAX_LEN-1);
							}
							break;
						}
					}
				}
				break;
			}
		}
	}
	//UpgradeIniKey_FwVer
	fseek( pfile, 0, SEEK_SET);
	for( memset(buff,0,101);fgets(buff,100,pfile) != NULL;memset(buff,0,101))
	{
		str_len = strlen(buff);
		if(str_len >= 100)
		{
			fclose(pfile);
			return -1;
		}
		
		if(str_len > 0)
		{
			if(Judge_IsSpaceLine(buff) != 0)
			{
				continue;
			}
			if(Judge_UpgradeIniKeyType(buff) == UpgradeIniKey_FwVer)
			{
				for( memset(buff,0,101);fgets(buff,100,pfile) != NULL;memset(buff,0,101))
				{
					str_len = strlen(buff);
					if(str_len >= 100)
					{
						fclose(pfile);
						return -1;
					}
					
					if(str_len > 0)
					{
						str_len = Delete_Linebreak(buff,str_len);
						if(Judge_IsSpaceLine(buff) == 0)
						{
							if(Judge_UpgradeIniKeyType(buff) == UpgradeIniKey_Unkown)
							{
								strncpy(ini_parser->fw_ver,buff,FW_VERSION_MAX_LEN-1);
							}
							break;
						}
					}
				}
				break;
			}
		}
	}

	//UpgradeIniKey_FwParser
	fseek( pfile, 0, SEEK_SET);
	for( memset(buff,0,101);fgets(buff,100,pfile) != NULL;memset(buff,0,101))
	{
		str_len = strlen(buff);
		if(str_len >= 100)
		{
			fclose(pfile);
			return -1;
		}
		
		if(str_len > 0)
		{
			if(Judge_IsSpaceLine(buff) != 0)
			{
				continue;
			}
			if(Judge_UpgradeIniKeyType(buff) == UpgradeIniKey_FwParser)
			{
				for( memset(buff,0,101);fgets(buff,100,pfile) != NULL;memset(buff,0,101))
				{
					str_len = strlen(buff);
					if(str_len >= 100)
					{
						fclose(pfile);
						return -1;
					}
					//str_len = Delete_Linebreak(buff,str_len);
					if(str_len > 0)
					{
						str_len = Delete_Linebreak(buff,str_len);
						if(Judge_IsSpaceLine(buff) == 0)
						{
							if(Judge_UpgradeIniKeyType(buff) != UpgradeIniKey_Unkown)
							{
								break;
							}
							if(Load_FwParserItems(ini_parser,buff,str_len) == -1)
							{
								fclose(pfile);
								return -1;
							}
						}
					}
				}
				break;
			}
		}
	}

	//UpgradeIniKey_FwReset
	fseek( pfile, 0, SEEK_SET);
	for( memset(buff,0,101);fgets(buff,100,pfile) != NULL;memset(buff,0,101))
	{
		str_len = strlen(buff);
		if(str_len >= 100)
		{
			fclose(pfile);
			return -1;
		}
		
		if(str_len > 0)
		{
			if(Judge_IsSpaceLine(buff) != 0)
			{
				continue;
			}
			if(Judge_UpgradeIniKeyType(buff) == UpgradeIniKey_FwReset)
			{
				for( memset(buff,0,101);fgets(buff,100,pfile) != NULL;memset(buff,0,101))
				{
					str_len = strlen(buff);
					if(str_len >= 100)
					{
						fclose(pfile);
						return -1;
					}
					//str_len = Delete_Linebreak(buff,str_len);
					if(str_len > 0)
					{
						str_len = Delete_Linebreak(buff,str_len);
						if(Judge_IsSpaceLine(buff) == 0)
						{
							if(Judge_UpgradeIniKeyType(buff) != UpgradeIniKey_Unkown)
							{
								break;
							}
							if(strstr(buff,"ENABLE") != NULL)
							{
								if(strstr(buff,"1") != NULL)
								{
									ini_parser->reset_enble = 1;
								}
								else
								{
									ini_parser->reset_enble = 0;
								}
								break;
							}
							
						}
					}
				}
				break;
			}
		}
	}

	//UpgradeIniKey_stm32
	fseek( pfile, 0, SEEK_SET);
	for( memset(buff,0,101);fgets(buff,100,pfile) != NULL;memset(buff,0,101))
	{
		str_len = strlen(buff);
		if(str_len >= 100)
		{
			fclose(pfile);
			return -1;
		}
		
		if(str_len > 0)
		{
			if(Judge_IsSpaceLine(buff) != 0)
			{
				continue;
			}
			if(Judge_UpgradeIniKeyType(buff) == UpgradeIniKey_Stm32)
			{
				for( memset(buff,0,101);fgets(buff,100,pfile) != NULL;memset(buff,0,101))
				{
					str_len = strlen(buff);
					if(str_len >= 100)
					{
						fclose(pfile);
						return -1;
					}
					//str_len = Delete_Linebreak(buff,str_len);
					if(str_len > 0)
					{
						str_len = Delete_Linebreak(buff,str_len);
						if(Judge_IsSpaceLine(buff) == 0)
						{
							if(Judge_UpgradeIniKeyType(buff) != UpgradeIniKey_Unkown)
							{
								break;
							}
							Load_Stm32UpdateItems(ini_parser,buff,str_len);
						}
					}
				}
				break;
			}
		}
	}

	fclose(pfile);
	
	return 0;	
}

int UpgradeIni_Free(UpgradeIni_Parser_Stru *ini_parser)
{
	if(ini_parser->n329fw_cnt>0 && ini_parser->n329fw_Items != NULL)
	{
		free((void*)(ini_parser->n329fw_Items));
		ini_parser->n329fw_Items = NULL;
	}

	return 0;
}

void Printf_UpgradeIni_Parser_Stru(UpgradeIni_Parser_Stru *ini_parser)
{
	int i = 0;
	printf("DEVICE TYPE = %s\n",ini_parser->device_type);
	for(i = 0; ini_parser->device_type[i] != 0;i++)
	{
		printf("%02x ",ini_parser->device_type[i]);
	}
	printf("strlen= %d\n",strlen(ini_parser->device_type));
	printf("FIREWARE VER = %s\n",ini_parser->fw_ver);
	printf("FIREWARE PARSER cnt = %d\n",ini_parser->n329fw_cnt);
	
	for(i = 0;i <ini_parser->n329fw_cnt;i ++)
	{
		printf("%s = %s\n",ini_parser->n329fw_Items[i].fwname,ini_parser->n329fw_Items[i].update_path);
	}
	
	printf("FIREWARE RESET enble = %d\n",ini_parser->reset_enble);
	if(ini_parser ->have_stm32fw == 1)
	{
		printf("stm32fw name = %s,update en = %d\n",ini_parser->stm32fwname,ini_parser->stm32fw_update_enble);
	}
	else
	{
		printf("have no stm32fw\n");
	}
}

//czn_20160827_s
int UpdateResourceTable(void)
{
	int i;
	char filepath[RESOURCE_FILE_LEN+1]={0},prefilepath[RESOURCE_FILE_LEN+1]={0},cmd_line[250]={0};
	
	for(i = 0;i < Updater_Run.configfile_cnt;i ++)
	{
		memset(prefilepath,0,sizeof(prefilepath));
		
		snprintf(filepath,RESOURCE_FILE_LEN,"/mnt/nand1-2/%s",Updater_Run.resource_record[i].filename);
		
		if(API_ResourceTb_AddOneRecord(Updater_Run.resource_record[i].rid,0, filepath, prefilepath) != 0)
		{
			return -1;
		}
		
		if(strlen(prefilepath) > 0)
		{
			remove(prefilepath);
		}
		
		snprintf(cmd_line,250,"mv /mnt/nand1-2/tftp/%s   %s",Updater_Run.resource_record[i].filename,filepath);
	
		if(system(cmd_line) != 0)
		{
			return -1;
		}
	}
	
	API_ResourceTb_Flush_Immediate();
	
	return 0;
}

int UpdateFwAndResource(void)
{
	int havefwupdtae = 0,alltbupdate = 0,update_result,i;
	ResourceTb_OneRecord_Stru onerecord;
	one_vtk_table *ptable;
	
	for(i = 0;i < Updater_Run.configfile_cnt;i ++)
	{
		if(Updater_Run.resource_record[i].rid == RID0000_FIRMWIRE)
		{
			havefwupdtae = 1;
		}
		else
		{
			if(API_ResourceTb_GetOneRecord(Updater_Run.resource_record[i].rid, &onerecord) != 0)
			{
				return -1;
			}
			
			switch(Updater_Run.resource_record[i].rid)
			{
				case RID1000_IO_PARAMETER:
					break;
				case RID1001_TB_ROOM:
				case RID1002_TB_GATEWAY:
					if(alltbupdate == 0)
					{
						alltbupdate = 1;
					}
					break;	
			}
		}
	}
	
	API_ResourceTb_Flush_Immediate();
	
	if(havefwupdtae)
	{
		if(API_ResourceTb_GetOneRecord(RID0000_FIRMWIRE, &onerecord) != 0)
		{
			return -1;
		}
		return start_updatefile_and_reboot(Updater_Run.server_ip_addr,onerecord.filename);
	}
	
	return 1;
}

//czn_20160827_e
//czn_20160708_e

