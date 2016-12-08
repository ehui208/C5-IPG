
#ifndef _UART_UDP_MSG_PROCESS_H
#define _UART_UDP_MSG_PROCESS_H

#include "../vtk_udp_stack/vtk_udp_stack_class.h"

#pragma pack(1)

typedef struct
{
	char 	start;
	char 	type;
	char 	len;
	char 	flag;
	int		ip;
} C5_IPG_HEAD;

typedef struct
{
	C5_IPG_HEAD	target;						//send target		目标头
	char		dat[IP8210_BUFFER_MAX]; 	//send data 		转发数据
} C5_IPG_PACK;

#pragma pack()

void uart_message_process( char* pbuf, int len );
void udp_message_process( char* pbuf, int len );


#endif

