

#ifndef _SLIPFRAME_H
#define _SLIPFRAME_H

#define SLIP_FRAME_MAX_FIFO    	2048
#define SLIP_FRAME_MAX_R_IN		500
#define SLIP_FRAME_MAX_R_OUT	500
#define SLIP_FRAME_MAX_T_IN		500
#define SLIP_FRAME_MAX_T_OUT	500

#define SLIP_FRAME_END			0xc0
#define SLIP_FRAME_ESC			0xdb

typedef struct Slip_Frame_Info_tag
{
	//rx fifo
	int 			rx_fifo_w;
	int	 		rx_fifo_r;
	char 		rx_fifo[SLIP_FRAME_MAX_FIFO];
	sem_t 		rx_in_sem;						// 输入信号量
	pthread_mutex_t 	lock;	
	//rx frame process
	int 			rx_in_cnt;							// 接收输入字节计数
	char 		rx_in_buf[SLIP_FRAME_MAX_R_IN];		//接收输入数据区
	int 			rx_out_cnt;							// 接收输出字节计数
	char 		rx_out_buf[SLIP_FRAME_MAX_R_OUT];	//接收输出数据区
	//tx frame process
	int	 		tx_in_cnt;							// 发送输入字节计数
	char 		tx_in_buf[SLIP_FRAME_MAX_T_IN];		//发送输入数据区
	int	 		tx_out_cnt;							// 发送输出字节计数
	char 		tx_out_buf[SLIP_FRAME_MAX_T_OUT];	//发送输出数据区
} Slip_Frame_Info, *p_Slip_Frame_Info;

void slip_push_fifo(p_Slip_Frame_Info pSlipFrame, char* pDat, int len);
int slip_pop_fifo(p_Slip_Frame_Info pSlipFrame, char* pCh);

int slip_frame_process_init( p_Slip_Frame_Info pSlipFrame );
int slip_frame_process_exit( p_Slip_Frame_Info pSlipFrame );


/*******************************************************************************************
 * @fn:		SlipTrsFrameProcess
 *
 * @brief:	slip发送数据帧处理
 *
 * @param:  	pBuffer - 带发送数据帧的指针, len - 发送的数据包长度
 *
 * @return: 	有效数据帧的长度
 *******************************************************************************************/
int SlipTrsFrameProcess( p_Slip_Frame_Info pSlipFrame );

/*******************************************************************************************
 * @fn:		SlipRecFrameProcess
 *
 * @brief:	slip接收数据帧处理
 *
 * @param:  	pSlipFrame - 处理对象
 *
 * @return: 	0/无数据，1/有数据
 *******************************************************************************************/
int SlipRecFrameProcess( p_Slip_Frame_Info pSlipFrame );

#endif

