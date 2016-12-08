
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include "utility.h"
#include "slipframe.h"

void slip_push_fifo(p_Slip_Frame_Info pSlipFrame, char* pDat, int len)
{	
	int i;

	pthread_mutex_lock(&(pSlipFrame->lock));
	
	for( i = 0; i < len; i++ )
	{
		pSlipFrame->rx_fifo[pSlipFrame->rx_fifo_w++] = *pDat++;
		
		if ( pSlipFrame->rx_fifo_w >= SLIP_FRAME_MAX_FIFO) 
		{
		    	pSlipFrame->rx_fifo_w = 0;
		}
        }

	pthread_mutex_unlock(&(pSlipFrame->lock));
	
	sem_post(&(pSlipFrame->rx_in_sem));
}

int slip_pop_fifo(p_Slip_Frame_Info pSlipFrame, char* pCh)
{
	int ret;
	pthread_mutex_lock(&(pSlipFrame->lock));

	if (pSlipFrame->rx_fifo_r == pSlipFrame->rx_fifo_w) 
	{
		ret = 0;
	}
	else
	{
		*pCh = pSlipFrame->rx_fifo[pSlipFrame->rx_fifo_r++];
		
		if ( pSlipFrame->rx_fifo_r >= SLIP_FRAME_MAX_FIFO ) 
		{
			pSlipFrame->rx_fifo_r = 0;
		}
		ret = 1;
	}
	pthread_mutex_unlock(&(pSlipFrame->lock));	
	return ret;
}

/*******************************************************************************************
 * @fn:		SetOneSlipFrame
 *
 * @brief:	转换一个数据包到SLIP的数据格式
 *
 * @param:  	Dst - 目标数据区指针，Src - 源数据区指针，Size - 转换的数据大小
 *
 * @return: 	Dst数据区的有效数据长度
 *******************************************************************************************/
short SetOneSlipFrame(unsigned char *Dst, unsigned char *Src, short Size)
{
	short i,size_frame;
	
	size_frame = 0;

	if( Size <= 0 )
		return 0;
	
	Dst[size_frame++] = SLIP_FRAME_END;
	
	for( i = 0; i < Size; i++ )
	{
		if( Src[i] == SLIP_FRAME_END )
		{
			Dst[size_frame++] = SLIP_FRAME_ESC;
			Dst[size_frame++] = SLIP_FRAME_ESC+1;
		}
		else if( Src[i] == SLIP_FRAME_ESC )
		{
			Dst[size_frame++] = SLIP_FRAME_ESC;
			Dst[size_frame++] = SLIP_FRAME_ESC+2;
		}
		else
		{
			Dst[size_frame++] = Src[i];
		}
	}
	Dst[size_frame++] = SLIP_FRAME_END;
	return size_frame;
}

/*******************************************************************************************
 * @fn:		GetOneSlipFrame
 *
 * @brief:	从一个SLIP数据包还原到有效数据包
 *
 * @param:  Dst - 目标数据区指针，Src - 源数据区指针，size - 转换的数据大小
 *
 * @return: Dst数据区的有效数据长度
 *******************************************************************************************/
short GetOneSlipFrame(unsigned char *Dst, unsigned char *Src, short Size)
{
	short i,size_frame;
	
	size_frame = 0;
	for( i = 0; i < Size; i++ )
	{
		if( Src[i] == SLIP_FRAME_END )
		{
			break;
		}
		else if( Src[i] == SLIP_FRAME_ESC )
		{
			i++;
			if( Src[i] == (SLIP_FRAME_ESC+1) )
			{
				Dst[size_frame++] = SLIP_FRAME_END;
			}
			else if( Src[i] == (SLIP_FRAME_ESC+2) )
			{
				Dst[size_frame++] = SLIP_FRAME_ESC;
			}
			else
			{
				continue;	//break;
			}
		}
		else
		{
			Dst[size_frame++] = Src[i];
		}
	}
	return size_frame;
}

/*******************************************************************************************
 * @fn:		SlipTrsFrameProcess
 *
 * @brief:	slip发送数据帧处理
 *
 * @param:  	pBuffer - 带发送数据帧的指针, len - 发送的数据包长度
 *
 * @return: 	有效数据帧的长度
 *******************************************************************************************/
int SlipTrsFrameProcess( p_Slip_Frame_Info pSlipFrame )
{
	pSlipFrame->tx_out_cnt = SetOneSlipFrame(pSlipFrame->tx_out_buf, pSlipFrame->tx_in_buf, pSlipFrame->tx_in_cnt );
	return 1;
}

/*******************************************************************************************
 * @fn:		SlipRecFrameProcess
 *
 * @brief:	slip接收数据帧处理
 *
 * @param:  	pSlipFrame - 处理对象
 *
 * @return: 	-1/无数据，0/有数据，1/有效数据包
 *******************************************************************************************/
int SlipRecFrameProcess( p_Slip_Frame_Info pSlipFrame )
{
	char ch;
	
	if( !slip_pop_fifo(pSlipFrame,&ch) )
	{
		return -1;
	}
	do
	{
		//
		if( pSlipFrame->rx_in_cnt == 0 )
		{
			if( ch == 0xA1 )
			{
				pSlipFrame->rx_in_buf[pSlipFrame->rx_in_cnt++] = ch;
			}
		}
		else if( pSlipFrame->rx_in_cnt == 1 )
		{
			pSlipFrame->rx_in_buf[pSlipFrame->rx_in_cnt++] = ch;		
		}
		else if( pSlipFrame->rx_in_cnt == 2 )
		{
			// 得到长度
			pSlipFrame->rx_in_buf[pSlipFrame->rx_in_cnt++] = ch;
		}
		else if( pSlipFrame->rx_in_cnt >= 3 )
		{
			pSlipFrame->rx_in_buf[pSlipFrame->rx_in_cnt++] = ch;
			if( pSlipFrame->rx_in_cnt >= pSlipFrame->rx_in_buf[2] )
			{
				memcpy( pSlipFrame->rx_out_buf, pSlipFrame->rx_in_buf, pSlipFrame->rx_in_cnt );				
				pSlipFrame->rx_out_cnt 	= pSlipFrame->rx_in_cnt;
				pSlipFrame->rx_in_cnt	= 0;				
				//printf("----------------get one uart pack,len=%d---------------\n",pSlipFrame->rx_out_cnt);
				return 1;
			}
		}
		/*
		// 识别到有结束标志处理
		if( ch == SLIP_FRAME_END )
		{		
			// 若缓冲中有数据则转换格式，并清除掉接收缓冲中的内容
			if( pSlipFrame->rx_in_cnt )				
			{
				pSlipFrame->rx_out_cnt = GetOneSlipFrame(pSlipFrame->rx_out_buf, pSlipFrame->rx_in_buf,pSlipFrame->rx_in_cnt);
				pSlipFrame->rx_in_cnt = 0;
				return 1;
			}
			// 若缓冲中无数据则为空结束符，不予理睬
			else
			{
				return 0;
			}
		}
		// 无结束标志则将数据添加到待处理缓冲
		else
		{
			pSlipFrame->rx_in_buf[pSlipFrame->rx_in_cnt++] = ch;
			if( pSlipFrame->rx_in_cnt >= SLIP_FRAME_MAX_R_IN )
			{
				pSlipFrame->rx_in_cnt = 0;
			}			
		}
		*/
	}while( slip_pop_fifo(pSlipFrame,&ch) );
	
	return 0;
}

/*******************************************************************************************
 *******************************************************************************************/
int slip_frame_process_init( p_Slip_Frame_Info pSlipFrame )
{
	pSlipFrame->rx_fifo_r 	= 0;
	pSlipFrame->rx_fifo_w 	= 0;
	pSlipFrame->rx_in_cnt 	= 0;
	pSlipFrame->rx_out_cnt 	= 0;
	pSlipFrame->tx_in_cnt 	= 0;
	pSlipFrame->tx_out_cnt 	= 0;
		
	if( sem_init(&(pSlipFrame->rx_in_sem), 0, 0) == -1 )
	{
		printf("slip sem init failure!msg:%s\n", strerror(errno));
		return -1;
	}	

	if(pthread_mutex_init(&(pSlipFrame->lock), 0) == -1)
	{
		printf("slip pthread_mutex_init failure!msg:%s\n", strerror(errno));
		sem_destroy(&(pSlipFrame->rx_in_sem));
		return -1;
	}
	return 0;
}

int slip_frame_process_exit( p_Slip_Frame_Info pSlipFrame )
{
	pthread_mutex_destroy(&(pSlipFrame->lock));	
	sem_destroy(&(pSlipFrame->rx_in_sem));
	return 0;
}

