
#ifndef _UNIX_SOCKET_H
#define _UNIX_SOCKET_H

#include "utility.h"

#define LOCAL_PORT_FLAG 	"/tmp/server_socket"

#define INTERACTIVE_BUFFER_SIZE	200
#define INTERACTIVE_BUFFER_NUM	10
typedef struct Interactive_Buffer_tag
{
	char padding[INTERACTIVE_BUFFER_SIZE];
	int	len;
} Interactive_Buffer, *p_Interactive_Buffer;

typedef struct Loop_Interactive_Buffer_tag
{
	p_Interactive_Buffer	w_p;
	p_Interactive_Buffer	r_p;
	pthread_mutex_t			lock;
	sem_t 					sem;	
	char					is_full; 	// 0:Not Full 1:Full
	Interactive_Buffer     	buf[INTERACTIVE_BUFFER_NUM];
} Loop_Interactive_Buffer, *p_Loop_Interactive_Buffer;

typedef void (*f_unix_socket_rcv)(char* pbuf, int len);

typedef struct unix_socket_tag
{
	int					is_server;				//unix socket type 0/client, 1/server
	//common attribute
	char*				p_unix_socket_path;		//unix socket path
	pthread_mutex_t 	task_rs_lock;  			//unix socket rcv&trs protect
	
	//server or client
	int					socket_fd;				//unix socket server fd
	int					client_fd;				//unix socket client fd
	struct sockaddr_un 	sockaddr;				//unix sockaddr server 
	struct sockaddr_un 	client_sockaddr;		//unix sockaddr client
	
	//receiver thread
	int					unix_socket_rcv_run;	//unix socket thread run flag
	pthread_t			p_uinx_socket_rcv_id;	//unix socket thread id
	int 				r_len;					//rec length
	char 				r_buf[INTERACTIVE_BUFFER_SIZE];
	
	//sender thread
	pthread_t			p_uinx_socket_trs_id;	//unix socket thread id
	//sender queue
	Loop_Interactive_Buffer sendqueue;			//unix socket send queue	
	
	f_unix_socket_rcv	callback_rcv;			//unix socket receive callback
} unix_socket_t, *p_unix_socket;


int init_unix_socket(p_unix_socket pUnixSocket,int is_server, char* pPath, f_unix_socket_rcv func_rcv);

int create_unix_socket_create(p_unix_socket pUnixSocket);
int deinit_unix_socket(p_unix_socket pUnixSocket);


// API for send data
int unix_socket_send_data(p_unix_socket pUnixSocket, char* pbuf, int len);

#endif

