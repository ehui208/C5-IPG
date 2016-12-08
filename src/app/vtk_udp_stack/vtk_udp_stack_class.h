
#ifndef _VTK_UDP_STACK_CLASS_H
#define _VTK_UDP_STACK_CLASS_H

#include "../task_survey/task_survey.h"

#define NET_DEVICE_NAME		"eth0"

// video客户端命令接收端口号
#define VIDEO_CLIENT_CMD_RECV_PORT		28000
// video服务器端命令接收端口
#define VIDEO_SERVER_CMD_RECV_PORT		28001

// linphone call接收端口号
#define LINPHONE_STARTER_RECV_PORT		28002

// video组播传输端口号
#define VIDEO_SERVER_MULTICAST_PORT		28100	// 28003 更改为 28100-29100 , 共1000个端口，每个ip 2个端口

// audio客户端传输端口号
#define AUDIO_CLIENT_UNICAST_PORT		25003
// audio服务器端传输端口
#define AUDIO_SERVER_UNICAST_PORT		25003

//#define	VDP_PRINTF_UDP

#ifdef	VDP_PRINTF_UDP
#define	udp_printf(fmt,...)	printf("[UDP]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)
#else
#define	udp_printf(fmt,...)
#endif

#define IP8210_CMD_START 		0xA1	// 起始头
#define IP8210_CMD_FLAG			0x5A	// 标志

// c5-ipg额外加入的命令类型
#define IP8210_CMD_TYPE			0xF1	// 需要解析数据包

// ip8210 命令类型
#define DATA_SINGLE				0x01	/*数据转发*/
#define AUDIO_RUN				0x02	/*启动音频*/
#define DATA_AUDIO_RUN			0x03	/*启动音频+数据转发*/									 
#define AUDIO_STOP				0x04	/*停止音频*/
#define DATA_AUDIO_STOP			0x05	/*停止音频+数据转发*/
#define VIDEO_REC_RUN			0x08	/*启动视频接收(解码)*/
#define DATA_VIDEO_REC_RUN		0x09	/*启动视频接收(解码)+数据转发*/
#define VIDEO_REC_STOP			0x10	/*停止视频接收(解码)*/
#define DATA_VIDEO_REC_STOP		0x11	/*停止视频接收(解码)+数据转发*/
#define VIDEO_TRA_RUN			0x20	/*启动视频发送(编码)*/
#define DATA_VIDEO_TRA_RUN		0x21	/*启动视频发送(编码)+数据转发*/
#define VIDEO_TRA_STOP			0x40	/*停止视频发送(编码)*/
#define DATA_VIDEO_TRA_STOP		0x41	/*停止视频发送(编码)+数据转发*/
#define ARUN_VREC_RUN			0x0a 	/*启动音频+启动视频接收(解码)*/
#define DATA_ARUN_VREC_RUN		0x0b 	/*启动音频+启动视频接收(解码)+数据转发*/
#define ASTP_VREC_STP			0x14	/*停止音频+停止视频接收(解码)*/
#define DATA_ASTP_VREC_STP		0x15	/*停止音频+停止视频接收(解码)+数据转发*/
#define ARUN_VTRA_RUN			0x22	/*启动音频+启动视频发送(编码)*/
#define DATA_ARUN_VTRA_RUN		0x23	/*启动音频+启动视频发送(编码)+数据转发*/
#define ASTP_VTRA_STP			0x44	/*停止音频+停止视频发送(编码)*/
#define DATA_ASTP_VTRA_STP		0x45	/*停止音频+停止视频发送(编码)+数据转发*/
#define IP_LINK					0x96	/*IP在线测试*/
#define IP_LINK_RECEIPT			0x97	/*IP在线测试之应答*/

#define IP_ADDR_R				0x98	/*读取IP地址*/
#define IP_ADDR_R_RECEIPT		0x99	/*读取IP地址应答*/
#define IP_ADDR_W				0x9A	/*写入IP地址*/
#define IP_ADDR_W_RECEIPT		0x9B	/*写入IP地址应答*/

#define DATA_ASTP_VTRA_RUN 		0x25	/*数据转发，同时音频停止、视频接收*/
#define DATA_ASTP_VALL_STP		0x55	/*数据转发，同时音频停止、视频接收发送停止*/

#define CAMERA_BRIGHT_ADJUST 	0xA0	/*串口通知调节摄像头亮度*/
#define CAMERA_COLOR_ADJUST		0xA1	/*串口通知调节摄像头色度*/


#define IP8210_BUFFER_MAX		240

// communication ack
#define	ACK						0xB5A1	// 兼容C5-IPC ([0]:B5, [1]:A1)


#define	ACK_RESPONSE_TIMEOUT	2000	// 时间需要长一点，上电第一次接收ack会延时1.02s才来??
#define BUSINESS_RESEND_TIME	5000
#define BUSINESS_WAIT_TIMEPUT	5000

//ip head struct define
typedef struct _baony_head_ 
{
	char 	start;
	char 	type;
	char 	len;
	char 	flag;
} baony_head;

//cmd head struct define
typedef struct _target_head_ 
{
	int				ip;
	unsigned short	cmd;
	unsigned short	id;
} target_head;

#define CHECKSUM_OFFSET_LEN	( sizeof(baony_head) + sizeof(int) )

//one normal package struct define
typedef struct _pack_
{
	//send head
	baony_head	head;
	//send target
	target_head	target;
	//send data
	char		dat[IP8210_BUFFER_MAX]; 
} pack;

typedef struct _pack_buf_
{
	//send target
	target_head	target;
	//len
	int			len;
	//send data
	char		dat[IP8210_BUFFER_MAX]; 
} pack_buf;

#define UDP_RT_TYPE_UNICAST		0
#define UDP_RT_TYPE_MULTICAST	1
#define UDP_RT_TYPE_BROADCAST	2

typedef int (*udp_msg_process)(char*,int);
typedef char* p_udp_common_buffer;

typedef struct loop_udp_common_buffer_tag
{
	OS_Q 					embosQ;
	int 					QSize;
	unsigned char*			pQBuf;
	udp_msg_process			process;			// 数据处理函数
} loop_udp_common_buffer, *p_loop_udp_common_buffer;

typedef struct _send_sem_id_t
{
	int				enable;
	sem_t			trig_sem;
	unsigned short	send_id;
	unsigned short	send_cmd;
	int				send_timeout;
	int				send_timeout_cnt;
	int				resend_times;
	char			*pbuffer;			// 动态建立的重发数据区指针
	int				len;				// 动态建立的重发数据区长度
	char			*prcvbuffer;		// 接收到数据保存到用户数据区的指针
	unsigned int	*prcvlen;			// 接收到数据保存到用户数据长度指针
} send_sem_id;

#define MAXf_SEND_SEM_ID_CNT		6
typedef struct _send_sem_id_array_t
{
	send_sem_id			dat[MAXf_SEND_SEM_ID_CNT];
    pthread_mutex_t 	lock;
} send_sem_id_array;

typedef struct _udp_comm_rt_t
{
	// comm
	char*						pname;				// 收发名称
	int							type;				// 类型：0/点播类型，1/组播类型，2/广播类型
	unsigned short				tport;				// 发送端口号
	unsigned short				rport;				// 接收端口号
	int							local_addr;			// 本机地址  网络字节序
	char*						target_pstr;		// 目标地址
	// trs
	int							sock_trs_fd;		// 发送socket句柄
	struct sockaddr_in 			trs_tar_addr;		// 发送方目标地址
	// trs buffer
	int 						tmsg_run_flag;		// 发送缓冲队列线程运行标志
	pthread_t					tmsg_pid;			// 发送缓冲队列线程运行id
	loop_udp_common_buffer		tmsg_buf;			// 发送缓冲消息队列
	// resend array
	send_sem_id_array			resend_array;		// 通信应答重发队列
	// rcv
	int							sock_rcv_fd;		// 接收socket句柄
	struct sockaddr_in 			rcv_tar_addr;		// 接收方源地址
	int 						rcv_run_flag;		// 接收线程运行标志
	pthread_t					rcv_pid;			// 接收线程运行id	
	// rcv buffer
	int 						rmsg_run_flag;		// 接收缓冲队列线程运行标志
	pthread_t					rmsg_pid;			// 接收缓冲队列处理线程运行id	
	loop_udp_common_buffer		rmsg_buf;			// 接收缓冲数据队列
} udp_comm_rt;


int init_one_send_array( send_sem_id_array* psendarray );

// lzh_20160811_s
int one_udp_comm_trs_direct( udp_comm_rt* pins, int target_ip, char* pbuf, int len );
// lzh_20160811_e

sem_t* join_one_send_array( send_sem_id_array* psendarray, unsigned short send_id, unsigned short send_cmd, int send_timeout, int resend_times, char *pbuffer, int len );
int trig_one_send_array( send_sem_id_array* psendarray, unsigned short recv_id, unsigned short recv_cmd );
int poll_all_send_array( udp_comm_rt* pins, int time_gap );

// 定时查询业务应答队列的超时
int poll_all_business_recv_array( send_sem_id_array* psendarray, int time_gap );

/*
 功能：
	初始化一个udp实例的缓冲队列大小和消息处理回调函数，若大小为0，则无缓冲
 参数：
	pins				- udp 收发实例指针
	rmsg_qsize		- udp 接收数据缓冲大小
	rmsg_process			- udp 接收数据处理函数指针
	tmsg_qsize		- udp 发送数据缓冲大小
	tmsg_process			- udp 发送数据处理函数指针
*/
int init_one_udp_comm_rt_buff( udp_comm_rt* pins, int rmsg_qsize, udp_msg_process rmsg_process,int tmsg_qsize, udp_msg_process tmsg_process );

/*
 功能：
	初始化一个udp实例: ID号，类型，端口号，目标地址
 参数：
	pins		- udp 收发实例指针
	id			- udp 实例id
	type			- udp 类型：0/点播，1/组播，2/广播
	rport		- udp 接收绑定的端口号
	tport		- udp 发送的端口号
	target_pstr	- udp 发送目标地址
*/
int init_one_udp_comm_rt_type( udp_comm_rt* pins,  char* pname, int type, unsigned short rport, unsigned short tport, char* target_pstr);

/*
 功能：
	反初始化一个udp实例
 参数：
 	pins 		- udp 收发实例指针
*/
int deinit_one_udp_comm_rt( udp_comm_rt* pins);

/*
 功能：
	udp实例的数据接发送数据API函数
 参数：
 	pins 		- udp 收发实例指针
 	target_ip 	- 目标ip
 	cmd	 		- 命令
 	id	 		- 命令id
 	ack	 		- 应答次数(0无需通信应答)
 	ptrs_buf		- udp 发送数据指针
 	ptrs_len		- udp 发送数据长度
 返回：
	-1/err，0/ok
*/
sem_t* one_udp_comm_trs_api( udp_comm_rt* pins, int target_ip, int cmd, int id, int ack, char* ptrs_buf,  int ptrs_len );

// 加入一个发送相关联的命令和序列号到业务应答队列，并等待接收数据
sem_t* join_one_business_recv_array( send_sem_id_array* psendarray, unsigned short send_id, unsigned short send_cmd, int send_timeout, char *prcvbuffer, unsigned int *prcvlen );

// 一个接收的命令和序列号与业务对立中有效的发送命令和序列号匹配，匹配ok则触发等待的信号量
int trig_one_business_recv_array( send_sem_id_array* psendarray, unsigned short recv_id, unsigned short recv_cmd, char* buf, unsigned int len );

// 无通信应答时删除业务应答的数据
int dele_one_business_recv_array( send_sem_id_array* psendarray, unsigned short recv_id, unsigned short recv_cmd );

#endif

