

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>      /* basic system data types */
#include <error.h>

#include "video_source_map_tab.h"
#include "../video_multicast_common.h"

// 得到组播服务的地址和端口，或是代理的组播和服务(需要配置) - 本机服务器可以针对部分IP实行代理
// mcg_addr 	- 为本地视频服务器的组播地址 ，若本地无视频服务器，则该地址为代理服务器的ip地址
// pport		- 为本地视频服务器的端口号
// -1: 	err
// 0:	multicast service
int get_server_multicast_addr_port( int* mcg_addr, unsigned short *pport )
{
	char addr[200];
	char str[16];

	// 得到网络字节序的ip地址 ( 高字节对齐, 先发送高字节 ) 
	int my_ip = GetLocalIp();

	// 根据ip地址得到组播地址
	sprintf( addr,"224.0.2.%d", (my_ip>>24)&0xff );
	memset( str, 0, 16 );
	strcpy( str, addr );
	*mcg_addr	= inet_addr(str);

	// 根据ip地址得到组播端口号	(考虑到PC监视多个IPG的情况下，端口号不能搞重复，故采用可变的端口号)
	//*pport 		= VIDEO_SERVER_MULTICAST_PORT;	
	unsigned short my_port = (htonl(my_ip))&0x1ff;		// 保留512个地址
	*pport = (VIDEO_SERVER_MULTICAST_PORT + my_port);	
	
	printf("get server multicast addr = %s, port = %d\n",str,*pport);	
	
	return 0;
}


int get_server_multicast_addr_port2( int service_ip, int* mcg_addr, unsigned short *pport )
{
	char addr[200];
	char str[16];

	int my_ip = service_ip;

	// 根据ip地址得到组播地址
	sprintf( addr,"224.0.2.%d", (my_ip>>24)&0xff );
	memset( str, 0, 16 );
	strcpy( str, addr );
	*mcg_addr	= inet_addr(str);

	// 根据ip地址得到组播端口号	(考虑到PC监视多个IPG的情况下，端口号不能搞重复，故采用可变的端口号)
	//*pport 		= VIDEO_SERVER_MULTICAST_PORT;	
	unsigned short my_port = (htonl(my_ip))&0x1ff;		// 保留512个地址
	*pport = (VIDEO_SERVER_MULTICAST_PORT + my_port);	
	
	printf("get server multicast addr = %s, port = %d\n",str,*pport);	
	
	return 0;	
}

	
