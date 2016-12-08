

#include "unix_socket.h"

/*******************************************************************************************
 * @fn:		Init_Interactive_Buffer
 *
 * @brief:	初始化队列
 *
 * @param:  	pobj - 队列对象
 *
 * @return: 	-1 - 初始化错误，0 - 初始化正确
 *******************************************************************************************/
int Init_Interactive_Buffer(p_Loop_Interactive_Buffer pobj)
{
	pobj->w_p 	= pobj->buf;
	pobj->r_p  	= pobj->buf;
	pobj->is_full	= 0;

	if( pthread_mutex_init(&pobj->lock, 0) == -1 )
	{
		printf("pthread_mutex_init call failure!msg:%s\n", strerror(errno));
		return -1;
	}
	if( sem_init(&pobj->sem, 0, 0) == -1 )
	{
		printf("sem_init call failure!msg:%s\n", strerror(errno));
		return -1;
	}	
	return 0;
}

/*******************************************************************************************
 * @fn:		DeInit_Interactive_Buffer
 *
 * @brief:	反初始化队列
 *
 * @param:  	pobj - 队列对象
 *
 * @return: 	-1 - 初始化错误，0 - 初始化正确
 *******************************************************************************************/
int DeInit_Interactive_Buffer(p_Loop_Interactive_Buffer pobj)
{
	pthread_mutex_destroy(&(pobj->lock));	
	sem_destroy(&(pobj->sem));
	return 0;
}

/*******************************************************************************************
 * @fn:		push_Interactive_Buffer
 *
 * @brief:	将数据加入到通用队列
 *
 * @param:  	pobj - 队列对象，data - 数据指针，length - 数据长度
 *
 * @return: 	0 - 队列已满，!0 - 写入成功后当前的数据指针
 *******************************************************************************************/
p_Interactive_Buffer push_Interactive_Buffer(p_Loop_Interactive_Buffer pobj,char *data, int length)
{
	p_Interactive_Buffer pllb = 0;

	pthread_mutex_lock(&pobj->lock);
	if( pobj->is_full)
	{
	    pthread_mutex_unlock(&pobj->lock);
	    return pllb;
	}

	pllb = pobj->w_p;
	pllb->len	= length;
	memcpy(pllb->padding, data, length);

	if(++pobj->w_p == (pobj->buf + INTERACTIVE_BUFFER_NUM))
	{
		pobj->w_p = pobj->buf;
	}
	if( pobj->w_p == pobj->r_p)
	{
	    pobj->is_full = 1;
	} 
	pthread_mutex_unlock(&pobj->lock);

	sem_post(&pobj->sem);

	return pllb;
}

/*******************************************************************************************
 * @fn:		pop_Interactive_Buffer
 *
 * @brief:	从队列中取出数据
 *
 * @param:  	pobj - 队列对象，is_to_next - 0/不调整数据指针，1/调整数据指针
 *
 * @return: 	0 - 队列已空，!0 - 队列非空，返回为当前数据区指针
 *******************************************************************************************/ 
p_Interactive_Buffer pop_Interactive_Buffer(p_Loop_Interactive_Buffer pobj, int is_to_next)
{
	p_Interactive_Buffer pllb = 0;

	pthread_mutex_lock(&pobj->lock);
	if( (pobj->w_p == pobj->r_p) && (!pobj->is_full) )
	{
		pthread_mutex_unlock(&pobj->lock);
		return pllb;
	}

	pllb = pobj->r_p;
	
	if(is_to_next)
	{
		if(++pobj->r_p == (pobj->buf + INTERACTIVE_BUFFER_NUM))
		{
			pobj->r_p = pobj->buf;
		}
		pobj->is_full = 0;

		if(pobj->r_p == pobj->w_p)
		{
			pllb = 0;
		}
		else
		{
			pllb = (p_Interactive_Buffer)1;
		}
	}
	pthread_mutex_unlock(&pobj->lock);
	return pllb;
}

void *unix_socket_pthread(void *arg);
void *unix_socket_sender_pthread(void *arg);

int init_unix_socket(p_unix_socket pUnixSocket,int is_server, char* pPath, f_unix_socket_rcv func_rcv)
{
	pUnixSocket->is_server			= is_server;
	pUnixSocket->p_unix_socket_path = pPath;	
	pUnixSocket->callback_rcv		= func_rcv;	
	
	Init_Interactive_Buffer(&pUnixSocket->sendqueue);

	if( pthread_mutex_init(&pUnixSocket->task_rs_lock, 0) == -1)
	{
		printf("pthread_mutex_init call failure!msg:%s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int create_unix_socket_create(p_unix_socket pUnixSocket)
{
	pUnixSocket->unix_socket_rcv_run = 1;
	return pthread_create(&pUnixSocket->p_uinx_socket_rcv_id, NULL, unix_socket_pthread, pUnixSocket);
}

int deinit_unix_socket(p_unix_socket pUnixSocket)
{
	pUnixSocket->unix_socket_rcv_run = 0;

	close( pUnixSocket->socket_fd );	// 触发select
	
	pthread_join(pUnixSocket->p_uinx_socket_rcv_id, NULL);

	DeInit_Interactive_Buffer(&pUnixSocket->sendqueue);
		
	return 0;
}


void *unix_socket_pthread(void *arg)
{
	int con_rtn;
	int b_reuse = 1;
	unsigned int socklen;
	fd_set fds;	

	p_unix_socket pUnixSocket = (p_unix_socket)arg;

	pUnixSocket->socket_fd = -1;		// server( linsten );  				client( connect, send, recv ) 
	pUnixSocket->client_fd = -1;		// server( accept, send, recv ); 	client( no use )
		

	if( pUnixSocket->is_server )
	{
		printf("start uart unix socket server thread....\n");
	}
	else
	{
		printf("start uart unix socket client thread....\n");
	}

	if( (pUnixSocket->socket_fd = socket(AF_UNIX,SOCK_STREAM,0)) == -1 )
	{
		printf("socket open failure,msg:%s\n",strerror(errno));
		goto error_exit;
	}

	struct linger so_linger;
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	// SO_LINGER选项用来设置延迟关闭的时间，等待套接字发送缓冲区中的数据发送完成		
	if( setsockopt(pUnixSocket->socket_fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(struct linger)) == -1 )
	{
		printf("setsockopt SOL_SOCKET::SO_LINGER call failure!msg:%s\n", strerror(errno));
		goto error_exit;
	}
	// 设置地址重复使用
	if( setsockopt(pUnixSocket->socket_fd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof(int)) == -1 )
	{
		printf("setsockopt SOL_SOCKET::SO_REUSEADDR call failure!msg:%s\n", strerror(errno));
		goto error_exit;
	}

	int socket_flags = fcntl(pUnixSocket->socket_fd, F_GETFL, 0);
	#if 1
	if( fcntl(pUnixSocket->socket_fd, F_SETFL, (socket_flags & ~O_NONBLOCK)) < 0 )
	{
		printf("Cant Not Set Socket Flags Block!\n");
		goto error_exit;
	}
    #else
	if( fcntl(pUnixSocket->socket_fd, F_SETFL, (socket_flags | O_NONBLOCK)) < 0 )
	{
		printf("Cant Not Set Socket Flags Non_Block!\n");
		goto error_exit;
	}
	#endif

	if( pUnixSocket->is_server )
	{
		//unlink(pUnixSocket->p_unix_socket_path); 
		pUnixSocket->sockaddr.sun_family = AF_UNIX;
		strcpy(pUnixSocket->sockaddr.sun_path, pUnixSocket->p_unix_socket_path);
		if( bind(pUnixSocket->socket_fd, (struct sockaddr *)&pUnixSocket->sockaddr, sizeof(struct sockaddr_un)) )
		{
			printf("bind call failure!msg:%s\n", strerror(errno));
			goto error_exit;
		}	
		printf("uart server listenning...!\n");
		if( listen( pUnixSocket->socket_fd, 1) )
		{
			printf("listen call failure!msg:%s\n", strerror(errno));
			goto error_exit;
		}
	}
	else
	{
		pUnixSocket->sockaddr.sun_family = AF_UNIX;
		strcpy(pUnixSocket->sockaddr.sun_path, pUnixSocket->p_unix_socket_path);	
		if( (con_rtn = connect(pUnixSocket->socket_fd, (struct sockaddr *)&pUnixSocket->sockaddr,sizeof(struct sockaddr_un)) ) < 0 )
		{
			printf("connect ERROR: con_rtn = %d, errno = %d %s\n", con_rtn, errno, strerror(errno));
			goto error_exit;
		}
	}
	
	while( pUnixSocket->unix_socket_rcv_run )
	{
		if( pUnixSocket->is_server )
		{       
			FD_ZERO(&fds);
			FD_SET(pUnixSocket->socket_fd, &fds); 
			// tv.tv_sec  = 20;
			// tv.tv_usec = 0;			
			// if(select_ex(pUnixSocket->socket_fd + 1, &fds, 0, 0, &tv) == -1)
			if(select_ex(pUnixSocket->socket_fd + 1, &fds, 0, 0, NULL) == -1)  // 20160107  等待有链接为止
			{
				printf("select call failure!msg:%s\n", strerror(errno));
				goto error_exit;
			}
			if( FD_ISSET(pUnixSocket->socket_fd, &fds) )
			{
				socklen = sizeof(struct sockaddr_un);
				if( (pUnixSocket->client_fd = accept(pUnixSocket->socket_fd, (struct sockaddr *)&(pUnixSocket->client_sockaddr), &socklen)) == -1 )
				{
					printf("accept one client failure!msg:%s\n", strerror(errno));
					goto error_exit;
				}
				else
				{
					printf("accept one client ok!\n");
				}
			}
		}
		
		fd_set rfds;
		struct timeval crtv;

		if( pUnixSocket->is_server )
		{
			printf("server interactive task working!\n");
		}
		else
		{
			printf("client Interactive task working!\n");	
		}
		
		if( pthread_create( &pUnixSocket->p_uinx_socket_trs_id, 0, &unix_socket_sender_pthread, arg) )
		{
			printf("Create interactive_sender_task_thread Failure,%s\n", strerror(errno));
			goto error_exit;
		}
		
		while( pUnixSocket->unix_socket_rcv_run )
		{
			int ret;
			
			FD_ZERO(&rfds);
			crtv.tv_sec = 0;			//1;	// 2
			crtv.tv_usec = 500*1000;	//0;

			if( pUnixSocket->is_server )
			{   
				// 查看是否有客户端连接
				if( pUnixSocket->client_fd != -1 )
				{
					FD_SET(pUnixSocket->client_fd, &rfds);
					ret = select_ex(pUnixSocket->client_fd + 1, &rfds, 0, 0, &crtv);
					if( ret == -1)
					{
						printf("select call failure!msg:%s\n", strerror(errno));
						goto error_exit;
					}	
					else if( ret != 0 )
					{
						if( FD_ISSET(pUnixSocket->client_fd, &rfds) )
						{
							pthread_mutex_lock(&pUnixSocket->task_rs_lock);				
							pUnixSocket->r_len = recv(pUnixSocket->client_fd, pUnixSocket->r_buf, INTERACTIVE_BUFFER_SIZE, 0);
							pthread_mutex_unlock(&pUnixSocket->task_rs_lock);					

							if( pUnixSocket->r_len < 0)
							{
								if((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR))
									goto error_exit;
								printf("recv failure!msg:%s\n", strerror(errno));
								goto error_exit;
							}							
							else if( pUnixSocket->r_len == 0 )
							{
								printf("client is active disconnected!\n");
								pUnixSocket->client_fd = -1;
								//goto error_exit;
							}
							//data process
							else if( pUnixSocket->r_len > 0)
							{
								(*pUnixSocket->callback_rcv)(pUnixSocket->r_buf,pUnixSocket->r_len);
							}															
						}
					}
					//printf("server is rcv polling per %d s!\n",crtv.tv_sec);				
				}
				else
				{
					FD_ZERO(&fds);
					FD_SET(pUnixSocket->socket_fd, &fds); 
					if(select_ex(pUnixSocket->socket_fd + 1, &fds, 0, 0, NULL) == -1)  // 20160107	等待有链接为止
					{
						printf("select call failure!msg:%s\n", strerror(errno));
						goto error_exit;
					}
					if( FD_ISSET(pUnixSocket->socket_fd, &fds) )
					{
						socklen = sizeof(struct sockaddr_un);
						if( (pUnixSocket->client_fd = accept(pUnixSocket->socket_fd, (struct sockaddr *)&(pUnixSocket->client_sockaddr), &socklen)) == -1 )
						{
							printf("accept one client failure!msg:%s\n", strerror(errno));
							goto error_exit;
						}
						else
						{
							printf("accept one client ok!\n");
						}
					}
				}
			}
			else
			{
				FD_SET(pUnixSocket->socket_fd, &rfds);
				ret = select_ex(pUnixSocket->socket_fd + 1, &rfds, 0, 0, &crtv);
				if( ret == -1)
				{
					printf("select call failure!msg:%s\n", strerror(errno));
					goto error_exit;
				}	
				else if( ret != 0 )
				{
					if( FD_ISSET(pUnixSocket->socket_fd, &rfds) )
					{
						pthread_mutex_lock(&pUnixSocket->task_rs_lock);				
						pUnixSocket->r_len = recv(pUnixSocket->socket_fd, pUnixSocket->r_buf, INTERACTIVE_BUFFER_SIZE, 0);
						pthread_mutex_unlock(&pUnixSocket->task_rs_lock);					

						if( pUnixSocket->r_len < 0)
						{
							if((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR))
								goto error_exit;
							printf("recv failure!msg:%s\n", strerror(errno));
							goto error_exit;
						}							
						if( pUnixSocket->r_len == 0 )
						{
							printf("server is active disconnected!\n");
							// goto error_exit;
							// lzh_20160129_s
							pUnixSocket->sockaddr.sun_family = AF_UNIX;
							strcpy(pUnixSocket->sockaddr.sun_path, pUnixSocket->p_unix_socket_path);	
							if( (con_rtn = connect(pUnixSocket->socket_fd, (struct sockaddr *)&pUnixSocket->sockaddr,sizeof(struct sockaddr_un)) ) < 0 )
							{
								printf("connect ERROR: con_rtn = %d, errno = %d %s\n", con_rtn, errno, strerror(errno));
								goto error_exit;
							}
							// lzh_20160129_e							
						}
						//data process
						if( pUnixSocket->r_len > 0)
						{
							(*pUnixSocket->callback_rcv)(pUnixSocket->r_buf,pUnixSocket->r_len);
						}									
					}				
				}
				//printf("client is rcv polling per %d s!\n",crtv.tv_sec);
			}
		}
	}
	error_exit:
	pUnixSocket->unix_socket_rcv_run = 0;
	pthread_join(pUnixSocket->p_uinx_socket_trs_id, NULL);
	close( pUnixSocket->socket_fd );
	close( pUnixSocket->client_fd );
	return NULL;
}

void *unix_socket_sender_pthread(void *arg)
{
	int ret;
	fd_set wfds;
	struct timeval cwtv;
	//unsigned int send_len, been_send_len;
	p_Interactive_Buffer psendbuff;
	int	send_fd;

	int	time_polling_gap = 5;	//5s
		
	p_unix_socket pUnixSocket = (p_unix_socket)arg;
	if( pUnixSocket->is_server )
	{       						
		printf("server sub sender task working!\n");
	}
	else
	{
		printf("client sub sender task working!\n");
	}

   	while(pUnixSocket->unix_socket_rcv_run)
	{
   		have_msg_data:
			
		psendbuff = pop_Interactive_Buffer(&pUnixSocket->sendqueue,0);
		
		if( psendbuff )
		{
			// 及时更新send_fd
			pthread_mutex_lock(&pUnixSocket->task_rs_lock);
			if( pUnixSocket->is_server )
				send_fd = pUnixSocket->client_fd;
			else
				send_fd = pUnixSocket->socket_fd;	
			pthread_mutex_unlock(&pUnixSocket->task_rs_lock);
		
			if( send_fd != -1 )
			{
				FD_ZERO(&wfds);
				cwtv.tv_sec = 0;			//1;  // 2 //20;
				cwtv.tv_usec = 500*1000;	//0;

				FD_SET(send_fd, &wfds);
				if( select_ex(send_fd + 1, 0, &wfds, 0, &cwtv) == -1 )
				{
					printf("select call failure!msg:%s\n", strerror(errno));
					goto interactive_sender_task_thread_error;
				}
				if( FD_ISSET(send_fd, &wfds) )
				{			
					pthread_mutex_lock(&pUnixSocket->task_rs_lock);

					ret = send(send_fd, psendbuff->padding, psendbuff->len, 0);
					if( ret < 0 )
					{
						printf("send failure!msg:%s\n", strerror(errno));
						pthread_mutex_unlock(&pUnixSocket->task_rs_lock);
						pop_Interactive_Buffer(&pUnixSocket->sendqueue,1);
						goto interactive_sender_task_thread_error;
					}
					
					pthread_mutex_unlock(&pUnixSocket->task_rs_lock);

					if( pop_Interactive_Buffer(&pUnixSocket->sendqueue,1) )
					{
						goto have_msg_data;
					}
				}
			}
		}
		if(sem_wait_ex(&pUnixSocket->sendqueue.sem, time_polling_gap*1000) == -1)
		{
			printf("sem_wait_ex call failure!msg:%s\n", strerror(errno));
			goto interactive_sender_task_thread_error;
		}
   	}   	
	interactive_sender_task_thread_error:
	pUnixSocket->unix_socket_rcv_run = 0;
	return (void *)0;
}

int unix_socket_send_data(p_unix_socket pUnixSocket, char* pbuf, int len)
{
	return (int)push_Interactive_Buffer( &pUnixSocket->sendqueue, pbuf, len );
}

