
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>      /* basic system data types */
#include <error.h>

#include "linphone_interface.h"

unix_socket_t	linphonec_server_socket;

void linphonec_server_socket_recv_data(char* pbuf, int len);

int init_linphone_if_service( void )
{
	init_unix_socket(&linphonec_server_socket,1,LOCAL_PORT_FLAG,linphonec_server_socket_recv_data);		// 服务器端
	create_unix_socket_create(&linphonec_server_socket);
	usleep(100*1000);
	vd_printf(" init_linphone_if_service ok \n");
	return 0;
}

int deinit_linphone_if_service(void)
{
	deinit_unix_socket(&linphonec_server_socket);
	usleep(10*1000);
	vd_printf(" deinit_linphone_if_service ok \n");
	return 0;
}

void linphonec_server_socket_recv_data(char* pbuf, int len)
{
	vdp_linphonec_if_buffer_head* ptrLinphonecif_buffer = (vdp_linphonec_if_buffer_head*)pbuf;

	// 解析呼叫指令
	if( ptrLinphonecif_buffer->cmd == EXOSIP_CALL_INVITE )
	{
		// 得到呼叫启动事件
		vd_printf(" .............................LINPHONEC Start Invite................... \n");
	}
	else if( (ptrLinphonecif_buffer->cmd == EXOSIP_CALL_RELEASED) || (ptrLinphonecif_buffer->cmd == EXOSIP_CALL_CLOSED) || (ptrLinphonecif_buffer->cmd == EXOSIP_CALL_MESSAGE_NEW) )
	{
		// 得到呼叫结束事件
		vd_printf(" .............................LINPHONEC Stop Invite................... \n");
	}
}


int API_linphonec_Invite( int ip )
{
	unsigned char ip1,ip2,ip3,ip4;
	
	char strLinphonecCmd[25];		//"call sip:xxx.xxx.xxx.xxx"
	
	memset(strLinphonecCmd, 0, sizeof(strLinphonecCmd) );

	ip1 = ip;
	ip2 = ip>>8;
	ip3 = ip>>16;
	ip4 = ip>>24;
	// 得到呼叫启动命令
	sprintf(strLinphonecCmd, "call sip:%d.%d.%d.%d", ip1,ip2,ip3,ip4);

	// 发送数据到linphonec if
	unix_socket_send_data( &linphonec_server_socket, strLinphonecCmd, sizeof(strLinphonecCmd) );

	vd_printf("lingphenc send %s\n", strLinphonecCmd);

	return 0;
}

int API_linphonec_Answer( void )
{
	char strLinphonecCmd[25];		//"call sip:xxx.xxx.xxx.xxx"
	
	memset(strLinphonecCmd, 0, sizeof(strLinphonecCmd) );

	sprintf(strLinphonecCmd, "%s", "answer");

	// 发送数据到linphonec if
	unix_socket_send_data( &linphonec_server_socket, strLinphonecCmd, sizeof(strLinphonecCmd) );

	vd_printf("lingphenc send %s\n", strLinphonecCmd);

	return 0;
}

int API_linphonec_Close( void )
{
	char strLinphonecCmd[25];

	usleep(100000); 	//lzh_20151102	
	
	memset(strLinphonecCmd, 0, sizeof(strLinphonecCmd) );
	
	// 得到呼叫结束命令
	sprintf(strLinphonecCmd, "%s", "terminate");

	// 发送数据到linphonec if
	unix_socket_send_data( &linphonec_server_socket, strLinphonecCmd, sizeof(strLinphonecCmd) );

	vd_printf("lingphenc send %s\n", strLinphonecCmd);

	usleep(100000);	//wait linphnec close compeleted!
	
	return 0;		
}

int API_linphonec_quit( void )
{
	char strLinphonecCmd[25];

	memset(strLinphonecCmd, 0, sizeof(strLinphonecCmd) );
	
	// 得到呼叫结束命令
	sprintf(strLinphonecCmd, "%s", "quit");

	// 发送数据到linphonec if
	unix_socket_send_data( &linphonec_server_socket, strLinphonecCmd, sizeof(strLinphonecCmd) );

	vd_printf("lingphenc send %s\n", strLinphonecCmd);

	usleep(100000);	//wait linphnec close compeleted!
	
	return 0;		
}



