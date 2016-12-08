
#ifndef _VTK_UDP_STACK_CLASS_EXT_H
#define _VTK_UDP_STACK_CLASS_EXT_H

#include "vtk_udp_stack_class.h"

#define	VDP_PRINTF_EXT

#ifdef	VDP_PRINTF_EXT
#define	EXT_printf(fmt,...)	printf("[EXT]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)
#else
#define	EXT_printf(fmt,...)
#endif

typedef struct
{
	//ip
	int			ip;
	//len
	int			len;
	//data
	char		dat[IP8210_BUFFER_MAX]; 
} ext_pack_buf;

int init_one_udp_comm_rt_type_ext( udp_comm_rt* pins,  char* pname, int type, unsigned short rport, unsigned short tport, char* target_pstr);

void *udp_trs_data_task_thread_ext(void *arg);
void *udp_rcv_data_task_thread_ext(void *arg);
void *udp_rcv_data_process_task_thread_ext(void *arg);
int poll_all_send_array_ext( udp_comm_rt* pins, int time_gap );


#endif


