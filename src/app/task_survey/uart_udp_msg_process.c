
#include "task_survey.h"
#include "sys_msg_process.h"
#include "../video_service/ip_video_cs_control.h"
#include "../vtk_udp_stack/vtk_udp_stack_c5_ipc_cmd.h"
#include "uart_udp_msg_process.h"

unsigned char UartCheckSum( char* pbuf, int len );
int vdp_uart_send_data_once(char *data, int len);

void send_ip_link_receipt_to_uart(void)
{
	C5_IPG_PACK send_pack;	

	send_pack.target.flag 	= IP8210_CMD_FLAG;
	send_pack.target.start	= IP8210_CMD_START;
	send_pack.target.type	= IP_LINK_RECEIPT;
	send_pack.target.ip		= GetLocalIp();
	send_pack.target.len	= sizeof(C5_IPG_HEAD)+1+1;
	send_pack.dat[0]		= 0xa2;
	send_pack.dat[1]		= 0xa2;
	vdp_uart_send_data_once( (char*)&send_pack,send_pack.target.len ); 
	//dprintf("response IP_LINK_RECEIPT to uart\n");	
}

void send_ip_link_receipt_to_udp( int target_ip )
{
	C5_IPG_PACK send_pack;	

	send_pack.target.flag 	= IP8210_CMD_FLAG;
	send_pack.target.start	= IP8210_CMD_START;
	send_pack.target.type	= IP_LINK_RECEIPT;
	send_pack.target.ip		= GetLocalIp();
	send_pack.target.len	= sizeof(C5_IPG_HEAD)+1+1;
	send_pack.dat[0]		= 0xa2;
	send_pack.dat[1]		= 0xa2;
	api_udp_transfer_send_data( target_ip, (char*)&send_pack,send_pack.target.len ); 
	api_udp_transfer2_send_data( target_ip, (char*)&send_pack,send_pack.target.len ); 
	//dprintf("response IP_LINK_RECEIPT to udp\n");	
}

void server_turn_on_vd( int target_ip )
{
	
}
	
void uart_message_process( char* pbuf, int len )
{
	C5_IPG_PACK* pPack = (C5_IPG_PACK*)pbuf;

	// 确认数据包长度
	if( len < sizeof(C5_IPG_HEAD) || len != pPack->target.len )
		return;

	// 确认数据检验和
	if( UartCheckSum(pbuf+sizeof(C5_IPG_HEAD),len-sizeof(C5_IPG_HEAD)-1) != pbuf[len-1] )
		return;
	
	//dprintf("rec uart data: target_ip=0x%08x,len=%d,type=0x%02x\n",pPack->target.ip,len,pPack->target.type);

	if( pPack->target.start == IP8210_CMD_START && pPack->target.flag == IP8210_CMD_FLAG )
	{
		switch( pPack->target.type )
		{
			case IP_LINK:				// IP在线测试
				send_ip_link_receipt_to_uart();
				break;
						
			case DATA_SINGLE:			// 数据转发到udp
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				break;
			
			case AUDIO_RUN:				// 启动音频
				API_talk_on_by_unicast(pPack->target.ip,AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);
				break;
				
			case DATA_AUDIO_RUN:		// 启动音频+数据转发
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
				API_talk_on_by_unicast(pPack->target.ip,AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);
				break;
				
			case AUDIO_STOP:			// 停止音频
				API_talk_off();			
				break;
				
			case DATA_AUDIO_STOP:		// 停止音频+数据转发
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
				API_talk_off();			
				break;
				
			case VIDEO_REC_RUN:			// 启动视频接收(解码)
				API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_VIDEO_REC_RUN:	// 启动视频接收(解码)+数据转发
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
				API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);				
				break;
				
			case VIDEO_REC_STOP:		// 停止视频接收(解码)
				API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_VIDEO_REC_STOP:	// 停止视频接收(解码)+数据转发
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
				API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case VIDEO_TRA_RUN:			// 启动视频发送(编码)
				API_VIDEO_S_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_VIDEO_TRA_RUN:	// 启动视频发送(编码)+数据转发
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
				API_VIDEO_S_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				break;
				
			case VIDEO_TRA_STOP:		// 停止视频发送(编码)
				API_VIDEO_S_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_VIDEO_TRA_STOP:	// 停止视频发送(编码)+数据转发
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
				API_VIDEO_S_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case ARUN_VREC_RUN:			// 启动音频+启动视频接收(解码)
				API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				API_talk_on_by_unicast(pPack->target.ip,AUDIO_SERVER_UNICAST_PORT, AUDIO_CLIENT_UNICAST_PORT);							
				break;
				
			case DATA_ARUN_VREC_RUN:	// 启动音频+启动视频接收(解码)+数据转发
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
				API_talk_on_by_unicast(pPack->target.ip,AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);
				API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				break;
				
			case ASTP_VREC_STP:			// 停止音频+停止视频接收(解码)
				API_talk_off();
				API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_ASTP_VREC_STP:	// 停止音频+停止视频接收(解码)+数据转发
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
				API_talk_off();
				API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case ARUN_VTRA_RUN:			// 启动音频+启动视频发送(编码)
				API_talk_on_by_unicast(pPack->target.ip,AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);
				API_VIDEO_S_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_ARUN_VTRA_RUN:	// 启动音频+启动视频发送(编码)+数据转发
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
				API_talk_on_by_unicast(pPack->target.ip,AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);
				API_VIDEO_S_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				break;
				
			case ASTP_VTRA_STP:			// 停止音频+停止视频发送(编码)
				API_talk_off();
				API_VIDEO_S_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_ASTP_VTRA_STP:	// 停止音频+停止视频发送(编码)+数据转发
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
				API_talk_off();
				API_VIDEO_S_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
															
			case IP_ADDR_R:				// 读取IP地址
				break;
				
			case IP_ADDR_R_RECEIPT:		// 读取IP地址应答
				break;
				
			case IP_ADDR_W:				// 写入IP地址
				break;
				
			case IP_ADDR_W_RECEIPT:		// 写入IP地址应答
				break;
											
			case DATA_ASTP_VTRA_RUN:	// 数据转发，同时音频停止、视频接收
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
				API_talk_off();
				API_VIDEO_S_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_ASTP_VALL_STP:	// 数据转发，同时音频停止、视频接收发送停止
				api_udp_transfer_send_data(pPack->target.ip,pbuf,len);
				api_udp_transfer2_send_data(pPack->target.ip,pbuf,len);
				API_talk_off();
				API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);												
				API_VIDEO_S_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;

			case IP_LINK_RECEIPT:		// IP在线测试之应答
				break;				

			// lzh_20161025_s
			case CAMERA_BRIGHT_ADJUST:
				printf("---CAMERA_BRIGHT_ADJUST inc1---\n");
				api_video_s_service_adjust_bright(1);
				break;
			case CAMERA_COLOR_ADJUST:
				printf("---CAMERA_COLOR_ADJUST inc1---\n");
				api_video_s_service_adjust_color(1);
				break;
			// lzh_20161025_e
		}	
	}
}

void udp_message_process( char* pbuf, int len )
{
	C5_IPG_PACK* pPack = (C5_IPG_PACK*)pbuf;

	// 确认数据包长度
	if( len < sizeof(C5_IPG_HEAD) || len != pPack->target.len )
		return;

	// 确认数据检验和
	if( UartCheckSum(pbuf+sizeof(C5_IPG_HEAD),len-sizeof(C5_IPG_HEAD)-1) != pbuf[len-1] )
		return;

	//dprintf("rec udp data: target_ip=0x%08x,len=%d,type=0x%02x\n",pPack->target.ip,len,pPack->target.type);

	if( pPack->target.start == IP8210_CMD_START && pPack->target.flag == IP8210_CMD_FLAG )
	{
		switch( pPack->target.type )
		{
			case IP_LINK:				// IP在线测试
				send_ip_link_receipt_to_udp(pPack->target.ip);
				break;

			case DATA_SINGLE:			// 数据转发到uart
				vdp_uart_send_data_once( pbuf,len ); 
				break;

			case AUDIO_RUN: 			// 启动音频
				API_talk_on_by_unicast(pPack->target.ip,AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);
				break;
				
			case DATA_AUDIO_RUN:		// 启动音频+数据转发
				vdp_uart_send_data_once( pbuf,len ); 
				API_talk_on_by_unicast(pPack->target.ip,AUDIO_CLIENT_UNICAST_PORT, AUDIO_SERVER_UNICAST_PORT);
				break;
				
			case AUDIO_STOP:			// 停止音频
				API_talk_off(); 		
				break;
				
			case DATA_AUDIO_STOP:		// 停止音频+数据转发
				vdp_uart_send_data_once( pbuf,len ); 
				API_talk_off(); 		
				break;
				
			case VIDEO_REC_RUN: 		// 启动视频接收(解码)
				API_VIDEO_S_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_VIDEO_REC_RUN:	// 启动视频接收(解码)+数据转发
				vdp_uart_send_data_once( pbuf,len ); 
				API_VIDEO_S_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				break;
				
			case VIDEO_REC_STOP:		// 停止视频接收(解码)
				API_VIDEO_S_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_VIDEO_REC_STOP:	// 停止视频接收(解码)+数据转发
				vdp_uart_send_data_once( pbuf,len ); 
				API_VIDEO_S_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case VIDEO_TRA_RUN: 		// 启动视频发送(编码)
				API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_VIDEO_TRA_RUN:	// 启动视频发送(编码)+数据转发
				vdp_uart_send_data_once( pbuf,len ); 
				API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				break;
				
			case VIDEO_TRA_STOP:		// 停止视频发送(编码)
				API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_VIDEO_TRA_STOP:	// 停止视频发送(编码)+数据转发
				vdp_uart_send_data_once( pbuf,len ); 
				API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case ARUN_VREC_RUN: 		// 启动音频+启动视频接收(解码)
				API_talk_on_by_unicast(pPack->target.ip,AUDIO_SERVER_UNICAST_PORT, AUDIO_CLIENT_UNICAST_PORT);										
				API_VIDEO_S_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_ARUN_VREC_RUN:	// 启动音频+启动视频接收(解码)+数据转发
				vdp_uart_send_data_once( pbuf,len ); 
				API_talk_on_by_unicast(pPack->target.ip,AUDIO_SERVER_UNICAST_PORT, AUDIO_CLIENT_UNICAST_PORT);										
				API_VIDEO_S_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				break;
				
			case ASTP_VREC_STP: 		// 停止音频+停止视频接收(解码)
				API_talk_off(); 		
				API_VIDEO_S_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_ASTP_VREC_STP:	// 停止音频+停止视频接收(解码)+数据转发
				vdp_uart_send_data_once( pbuf,len ); 
				API_talk_off(); 		
				API_VIDEO_S_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case ARUN_VTRA_RUN: 		// 启动音频+启动视频发送(编码)
				API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				API_talk_on_by_unicast(pPack->target.ip,AUDIO_SERVER_UNICAST_PORT, AUDIO_CLIENT_UNICAST_PORT);				
				break;
				
			case DATA_ARUN_VTRA_RUN:	// 启动音频+启动视频发送(编码)+数据转发
				vdp_uart_send_data_once( pbuf,len ); 
				API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);
				API_talk_on_by_unicast(pPack->target.ip,AUDIO_SERVER_UNICAST_PORT, AUDIO_CLIENT_UNICAST_PORT);				
				break;
				
			case ASTP_VTRA_STP: 		// 停止音频+停止视频发送(编码)
				API_talk_off(); 		
				API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
				
			case DATA_ASTP_VTRA_STP:	// 停止音频+停止视频发送(编码)+数据转发
				vdp_uart_send_data_once( pbuf,len ); 
				API_talk_off(); 		
				API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
															
			case IP_ADDR_R: 			// 读取IP地址
				break;
				
			case IP_ADDR_R_RECEIPT: 	// 读取IP地址应答
				break;
				
			case IP_ADDR_W: 			// 写入IP地址
				break;
				
			case IP_ADDR_W_RECEIPT: 	// 写入IP地址应答
				break;
											
			case DATA_ASTP_VTRA_RUN:	// 数据转发，同时音频停止、视频接收
				vdp_uart_send_data_once( pbuf,len ); 
				API_talk_off();
				API_VIDEO_C_SERVICE_TURN_ON_MULTICAST(pPack->target.ip);				
				break;
				
			case DATA_ASTP_VALL_STP:	// 数据转发，同时音频停止、视频接收发送停止
				vdp_uart_send_data_once( pbuf,len ); 
				API_talk_off();
				API_VIDEO_C_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				API_VIDEO_S_SERVICE_TURN_OFF_MULTICAST(pPack->target.ip);
				break;
			
			case IP_LINK_RECEIPT:		// IP在线测试之应答
				break;				
		}
	}
}



