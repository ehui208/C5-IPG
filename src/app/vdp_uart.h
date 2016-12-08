
#ifndef VDP_UART_H
#define  VDP_UART_H

#define 	PACKAGE_TYPE_STRING		0		// 字符串数据包
#define	PACKAGE_TYPE_SLIP			1		// slip数据包格式

#define 	PACKAGE_TYPE				PACKAGE_TYPE_STRING
//#define 	PACKAGE_TYPE				PACKAGE_TYPE_SLIP


/*******************************************************************************************
 * @fn:		Init_vdp_uart
 *
 * @brief:	初始化uart端口
 *
 * @param:  	none
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int Init_vdp_uart(void);

/*******************************************************************************************
 * @fn:		vdp_uart_send_data
 *
 * @brief:	串口发送数据包处理
 *
 * @param:  	*data	- 数据区指针
 * @param:  	len		- 数据包长度
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int vdp_uart_send_data(char *data, int len);

/*******************************************************************************************
 * @fn:		vdp_uart_recv_data
 *
 * @brief:	串口接收数据包处理
 *
 * @param:  	buf - 数据指针
 * @param:  	len - 数据长度
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int vdp_uart_recv_data( char* buf, int len);

/*******************************************************************************************
 * @fn:		close_vdp_uart
 *
 * @brief:	关闭串口即相关资源
 *
 * @param:  	none
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int close_vdp_uart(void);

int api_uart_recv_callback( char* pbuf, unsigned int len );

int API_Send_BusinessRsp_ByUart(unsigned char  target_id,unsigned char  source_id,unsigned short cmd_type,unsigned char *buf,int len);	//czn_20160601
#endif


