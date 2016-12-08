
#ifndef _UART_H_
#define _UART_H_

#define SERIAL_MAX_BUFF_SIZE 	(300)
#define SERIAL_MAX_BUFF_NUM 	16

typedef struct Serial_Buffer_tag{
	int  	buf_len;
	char 	padding[SERIAL_MAX_BUFF_SIZE];
} Serial_Buffer, *p_Serial_Buffer;

typedef struct Loop_Serial_Buffer_tag{
	p_Serial_Buffer 	w_p;
	p_Serial_Buffer 	r_p;
	pthread_mutex_t 	lock;
	char            		is_full; /* 0:Not Full 1:Full */
	Serial_Buffer   	buf[SERIAL_MAX_BUFF_NUM];
} Loop_Serial_Buffer, *p_Loop_Serial_Buffer;

typedef struct Serial_Dev_Param_tag{
	int  baud_rate;
	int  data_bits;
	int  stop_bits;
	char parity;
	char reserve[3];
} Serial_Dev_Param, *p_Serial_Dev_Param;

typedef struct Serial_Task_Info_tag{
	char 			   	sdev_name[128];
	Serial_Dev_Param  	sdev_sdp;
	int  			   		sdev_fd;
	Loop_Serial_Buffer 	sdev_lsb;
	pthread_t 		   	sdev_task_tpid;
	int				   	sdev_task_run_flag;
	sem_t 			   	sdev_queue_sem;
	void (*data_handle_cbfun)(char *, int);
	char			   		sdev_task_recv_buff[SERIAL_MAX_BUFF_SIZE];
	char			  		sdev_task_send_buff[SERIAL_MAX_BUFF_SIZE];
} Serial_Task_Info, *p_Serial_Task_Info;

int start_serial_task(p_Serial_Task_Info psti, char *dev_name, int baud_rate, int data_bits, char parity, int stop_bits, void (*data_handle_cbfun)(char *, int));
int stop_serial_task(p_Serial_Task_Info);

int push_serial_data(p_Serial_Task_Info, char *, int);
int pop_serial_data(p_Serial_Task_Info, char *, int *);

int write_serial_data_direct(p_Serial_Task_Info psti, char *data, int len);


#endif
