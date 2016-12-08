
/*****************************************************************************************

*****************************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>   
#include <termios.h> 
#include <time.h> 
#include <pthread.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "uart.h"
#include "utility.h"

void *serial_task_thread(void *);

int start_serial_task(p_Serial_Task_Info psti, char *dev_name, int baud_rate, int data_bits, char parity, int stop_bits, void (*data_handle_cbfun)(char *, int))
{
	if( !psti || !dev_name || !baud_rate || !data_handle_cbfun )
	{
		printf("Param Error!\n");
		return -1;
	}	
	memcpy(psti->sdev_name, dev_name, strlen(dev_name));
	psti->sdev_sdp.baud_rate = baud_rate;
	psti->sdev_sdp.data_bits = data_bits;
	psti->sdev_sdp.parity = parity;
	psti->sdev_sdp.stop_bits = stop_bits;
	psti->data_handle_cbfun = data_handle_cbfun;
	
	if( sem_init(&(psti->sdev_queue_sem), 0, 0) == -1 )
	{
		printf("sem_init call failure!msg:%s\n", strerror(errno));
		return -1;
	}
	
	psti->sdev_task_run_flag = 1;
	
	psti->sdev_lsb.w_p 	= psti->sdev_lsb.buf;
	psti->sdev_lsb.r_p 	= psti->sdev_lsb.buf;
	psti->sdev_lsb.is_full 	= 0;
	if(pthread_mutex_init(&(psti->sdev_lsb.lock), 0) == -1)
	{
		printf("pthread_mutex_init call failure!msg:%s\n", strerror(errno));
		sem_destroy(&(psti->sdev_queue_sem));
		return -1;
	}
	
	if( pthread_create(&(psti->sdev_task_tpid), 0, &serial_task_thread, (void *)psti) )
	{
		printf("Create serial_task_thread Failure,%s\n", strerror(errno));
		pthread_mutex_destroy(&(psti->sdev_lsb.lock));
		sem_destroy(&(psti->sdev_queue_sem));
		return -1;
	}	
	return 0;
}

int stop_serial_task(p_Serial_Task_Info psti)
{
	psti->sdev_task_run_flag = 0;
	
	sem_post(&(psti->sdev_queue_sem));
	
	pthread_join(psti->sdev_task_tpid, 0);
	
	pthread_mutex_destroy(&(psti->sdev_lsb.lock));
	
	sem_destroy(&(psti->sdev_queue_sem));
	
	return 0;
}

/*
VTIME和VMIN需配合使用，它们的组合关系如下：

    1、VTIME=0，VMIN=0：此时即使读取不到任何数据，函数read也会返回，返回值是0。

    2、VTIME=0，VMIN>0：read调用一直阻塞，直到读到VMIN个字符后立即返回。

    3、VTIME>0，VMIN=0：read调用读到数据则立即返回，否则将为每个字符最多等待 VTIME*100ms 时间。

    4、VTIME>0，VMIN>0：read调用将保持阻塞直到读取到第一个字符，读到了第一个字符之后开始计时，此后若时间到了 VTIME*100ms 或者时间未到但已读够了VMIN个字符则会返回。若在时间未到之前又读到了一个字符(但此时读到的总数仍不够VMIN)则计时重新开始(即每个字符都有VTIME*100ms的超时时间)。
*/

int set_serial_opt(int fd, int baud_rate, int data_bits, char parity, int stop_bits)
{ 
	struct termios tio;

	memset(&tio, 0, sizeof(struct termios));
	if(tcgetattr(fd, &tio) != 0)
	{
		printf("tcgetattr call error,msg:%s!\n", strerror(errno));
		return -1; 
	}
	tcflush(fd, TCIOFLUSH);
	switch(baud_rate)
	{
	    case 2400:
	        cfsetispeed(&tio, B2400);
	        cfsetospeed(&tio, B2400);
	        break;
	    case 4800:
	        cfsetispeed(&tio, B4800);
	        cfsetospeed(&tio, B4800);
	        break;
	    case 9600:
	        cfsetispeed(&tio, B9600);
	        cfsetospeed(&tio, B9600);
	        break;
	    case 19200:
	        cfsetispeed(&tio, B19200);
	        cfsetospeed(&tio, B19200);
	        break;
	    case 38400:
	        cfsetispeed(&tio, B38400);
	        cfsetospeed(&tio, B38400);
	        break;
	    case 57600:
	        cfsetispeed(&tio, B57600);
	        cfsetospeed(&tio, B57600);
	        break;
	    case 115200:
	        cfsetispeed(&tio, B115200);
	        cfsetospeed(&tio, B115200);
	        break;
	    default:
	        cfsetispeed(&tio, B9600);
	        cfsetospeed(&tio, B9600);
	        break;
	}  
	if( (tcsetattr(fd, TCSANOW, &tio)) != 0 )
	{
	    printf("tcsetattr call error,msg:%s!\n", strerror(errno));
	    return -1;
	}
	tcflush(fd, TCIOFLUSH);

	memset(&tio, 0, sizeof(struct termios));
	
	if(tcgetattr(fd, &tio) != 0)
	{
		printf("tcgetattr call error,msg:%s!\n", strerror(errno));
		return -1; 
	}
	tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tio.c_cflag &= ~CSIZE;
    
	switch(data_bits)
	{
		case 7:
		    tio.c_cflag |= CS7;
		    break;
		case 8:
		    tio.c_cflag |= CS8;
		    break;
		default:
			break;
	}
    
	switch(parity)
	{
		case 'O':
			tio.c_cflag |= (PARODD | PARENB);
			tio.c_iflag |= INPCK;
			break;
		case 'E':
			tio.c_cflag |= PARENB;
			tio.c_cflag &= ~PARODD;
			tio.c_iflag |= INPCK;
			break;
		case 'N':
			tio.c_cflag &= ~PARENB;
			tio.c_iflag &= ~INPCK;
			break;
		case 'S':
			tio.c_cflag &= ~PARENB;
			tio.c_cflag &= ~CSTOPB;
			break;
		default:
			break;
	}

	if(stop_bits == 1)
	{
		tio.c_cflag &= ~CSTOPB;
	}
	else if(stop_bits == 2)
	{
		tio.c_cflag |= CSTOPB;
	}
    
	if(parity != 'N')
	{
		tio.c_iflag |= INPCK;
	}
    
	tio.c_cflag |= CLOCAL | CREAD;
	tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tio.c_oflag &= ~OPOST;
	tio.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

	tcflush(fd, TCIFLUSH);
	// lzh_20160811_s
	tio.c_cc[VTIME] = 10;
	tio.c_cc[VMIN] = 1;
	//tio.c_cc[VTIME] = 1;
	//tio.c_cc[VMIN] = 250;
	// lzh_20160811_s
	if(tcsetattr(fd, TCSANOW, &tio) == -1)
	{
		printf("tcsetattr call error,msg:%s!\n", strerror(errno));
		return -1;
	}
	tcflush(fd, TCIFLUSH);
	return 0;
}

void *serial_task_thread(void *arg)
{
	int temp_cnt = 0;
	int i;
	
	int ret, incsid, hsd;
	
	p_Serial_Task_Info psti = (p_Serial_Task_Info)arg;
	
	fd_set rfds, wfds;
	
	struct timeval tv;
	unsigned int send_len, been_send_len;
	
	printf("Start Serial %s Task!\n", psti->sdev_name);
	
	re_open:
	// 打开设备文件
	if( (psti->sdev_fd = open(psti->sdev_name, O_RDWR)) == -1 )
	{
		printf("Open Serial Device Failure!\n");
		if(psti->sdev_task_run_flag)
		{
			goto serial_task_thread_stop0;
		}
		else
		{
			goto serial_task_thread_exit0;
		}
	}
	else
	{
		printf("Open Serial Device Success!\n");
	}
	// 设置参数
	if( set_serial_opt(psti->sdev_fd, psti->sdev_sdp.baud_rate, psti->sdev_sdp.data_bits, psti->sdev_sdp.parity, psti->sdev_sdp.stop_bits) == -1)
	{
		printf("Set Serial Param Failure!\n");
		if(psti->sdev_task_run_flag)
		{
			goto serial_task_thread_stop1;
		}
		else
		{
			goto serial_task_thread_exit1;
		}
	}
	else
	{
		printf("Set Serial Param Success!\n");
	}
	
	while( psti->sdev_task_run_flag )
	{
		incsid = 0;
		been_send_len = 0;

		// 检测是否需要发送的数据包
		if((hsd = pop_serial_data(psti, psti->sdev_task_send_buff, &send_len)))
		{
    			incsid = 1;	// 表示还有数据待发送
	    }	
		send_incomplete:
		if(!psti->sdev_task_run_flag)
		{
			goto serial_task_thread_exit1;
		}
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		tv.tv_sec = 0;
		tv.tv_usec = 1000 * 20;
		if(hsd)
		{
			FD_SET(psti->sdev_fd, &rfds);
			FD_SET(psti->sdev_fd, &wfds);
			ret = select_ex(psti->sdev_fd + 1, &rfds, &wfds, 0, &tv);
		}
		else
		{
			FD_SET(psti->sdev_fd, &rfds);
			ret = select_ex(psti->sdev_fd + 1, &rfds, 0, 0, &tv);
		}
		if(ret == -1)
		{
		    printf("select call failure,msg:%s!\n", strerror(errno));
		    goto serial_task_thread_stop1;
		}
		// 读取接收数据缓冲
		if(FD_ISSET(psti->sdev_fd, &rfds))
		{
			ret = read(psti->sdev_fd, psti->sdev_task_recv_buff, SERIAL_MAX_BUFF_SIZE);
			if(ret == -1)
			{
				if(errno == EINTR)
				{
					printf("data not to recv...!\n");
					goto no_data_recv;
				}
				printf("read call failure!msg:%s\n", strerror(errno));
				if(psti->sdev_task_run_flag){
					goto serial_task_thread_stop1;
				}else{
					goto serial_task_thread_exit1;
				}
			}
			psti->sdev_task_recv_buff[ret] = 0;
			if(ret != 0)
			{
				#if 0
				printf("recv len = %d: ",ret);
				for( i = 0; i < ret; i++ )
				{
					printf("0x%02x,",psti->sdev_task_recv_buff[i]);
				}
				printf("\n\r");
				#endif
				
				psti->data_handle_cbfun(psti->sdev_task_recv_buff, ret);   //执行串口接收数据解析函数
			}
		}
		no_data_recv:
		
		if(hsd)
		{
			// 写入待发送数据
			if(FD_ISSET(psti->sdev_fd, &wfds))
			{
#if 0			
				//-----------------debug--------------
				printf("send len = %d: ",send_len - been_send_len);
				for( i = 0; i < send_len - been_send_len; i++ )
				{
					printf("0x%02x,",*((psti->sdev_task_send_buff + been_send_len)+i));
				}
				printf("\n\r"); 				
				//-----------------debug--------------
#endif				
				ret = write(psti->sdev_fd, (psti->sdev_task_send_buff + been_send_len), (send_len - been_send_len));
 				if(ret == -1)
				{
					if(errno == EINTR)
					{
						printf("data not to send...!\n");
						ret = 0;	
					}
					else
					{
						printf("write call failure!msg:%s\n", strerror(errno));
						if(psti->sdev_task_run_flag)
						{
							goto serial_task_thread_stop1;
						}
						else
						{
							goto serial_task_thread_exit1;
						}
					}
 				}
				//tcdrain() 等待直到所有写入 fd 引用的对象的输出都被传输。
 				if(tcdrain(psti->sdev_fd))
				{
 					printf("tcdrain call failure!msg:%s\n", strerror(errno));
 					goto serial_task_thread_stop1;
 				}
 				
				if(ret != (send_len - been_send_len))
				{
					been_send_len += ret;
				}
				else
				{
					incsid = 0;
				}
			}
			if(incsid)
			{
				goto send_incomplete;
			}
		}		
		if(sem_wait_ex(&(psti->sdev_queue_sem), 20) == -1)
		{
			printf("sem_wait_ex call failure!msg:%s\n", strerror(errno));
			goto serial_task_thread_stop1;
		}
		// test
		if( ++temp_cnt > 100 )
		{
			temp_cnt = 0;
			//printf("...\n");
		}
		// test
	}
serial_task_thread_exit1:
	close(psti->sdev_fd);
	
	printf("Stop Serial %s Task!\n", psti->sdev_name);

serial_task_thread_exit0:
	return (void *)0;
serial_task_thread_stop1:
	close(psti->sdev_fd);
serial_task_thread_stop0:
	sleep(1);
	printf("serial task err exit...\n");
	goto re_open;
}

/*
 * Return 0 -> Buff Is Full
 * Return 1 -> Push Data Success
 */
int push_serial_data(p_Serial_Task_Info psti, char *data, int len)
{
	p_Serial_Buffer psb = 0;

	pthread_mutex_lock(&(psti->sdev_lsb.lock));
	if(psti->sdev_lsb.is_full)
	{
	    pthread_mutex_unlock(&(psti->sdev_lsb.lock));
	    return (int)psb;
	}
	// test
	//printf("---push serial data ---- 111\n");
	// test
	
	psb = psti->sdev_lsb.w_p;
	psb->buf_len = len;
	memcpy(psb->padding, data, psb->buf_len);
    
	if(++psti->sdev_lsb.w_p == (psti->sdev_lsb.buf + SERIAL_MAX_BUFF_NUM))
	{
		psti->sdev_lsb.w_p = psti->sdev_lsb.buf;
	}
	if(psti->sdev_lsb.w_p == psti->sdev_lsb.r_p)
	{
		psti->sdev_lsb.is_full = 1;
	} 

	pthread_mutex_unlock(&(psti->sdev_lsb.lock));

	// test
	//printf("---push serial data ---- 222\n");
	// test

	sem_post(&psti->sdev_queue_sem);

	// test
	//printf("---push serial data ---- 333\n");
	// test
	
	return (int)psb;
}

/*
 * Return 0 -> Buff Empty
 * Return 1 -> Pop Data Success And Buff Empty To Next Get
 * Return 2 -> Pop Data Success And Buff Not Empty
 */
int pop_serial_data(p_Serial_Task_Info psti, char *data, int *p_len)
{
	p_Serial_Buffer psb = 0;

	pthread_mutex_lock(&(psti->sdev_lsb.lock));
	if((psti->sdev_lsb.w_p == psti->sdev_lsb.r_p) && (!psti->sdev_lsb.is_full))
	{
		pthread_mutex_unlock(&(psti->sdev_lsb.lock));
		return (int)psb;
	}

	psb = psti->sdev_lsb.r_p;
	memcpy(data, psb->padding, psb->buf_len);
	*p_len = psb->buf_len;

	if(++psti->sdev_lsb.r_p == (psti->sdev_lsb.buf + SERIAL_MAX_BUFF_NUM))
	{
		psti->sdev_lsb.r_p = psti->sdev_lsb.buf;
	}
	psti->sdev_lsb.is_full = 0;

	if(psti->sdev_lsb.r_p == psti->sdev_lsb.w_p)
	{
		psb = (p_Serial_Buffer)1;
	}
	else
	{
		psb = (p_Serial_Buffer)2;
	}
	pthread_mutex_unlock(&(psti->sdev_lsb.lock));

	return (int)psb;
}

int write_serial_data_direct(p_Serial_Task_Info psti, char *data, int len)
{
	int ret;
	ret = write(psti->sdev_fd,  data, len );
	if(ret == -1)
	{
		if(errno == EINTR)
			printf("data not to send...!\n");
		else
			printf("write call failure!msg:%s\n", strerror(errno));
		return -1;
	}
	//等待直到所有写入 fd 引用的对象的输出都被传输。
	if( tcdrain(psti->sdev_fd) )
	{
		printf("tcdrain call failure!msg:%s\n", strerror(errno));
		return -1;
	}
	// test
	//PrintCurrentTime(888);
	// test
	return 0;
}

