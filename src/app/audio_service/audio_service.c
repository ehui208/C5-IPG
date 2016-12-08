

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>      /* basic system data types */
#include <error.h>
#include <time.h>   

#include "g711common.h"
#include "g711core.h"
#include "audio_service.h"

AUDIO_HEAD au_trs_head;

void InitialAudioPackHead(void)
{
	au_trs_head.DataType	= 0x0001;
	au_trs_head.FrameNo		= 0x0001;
	memcpy( au_trs_head.magic, "SWITCH", 6 );
}

#define	CUR_G711_LAW	U_LAW

// 接收到数据包后播放(接收到的数据包长度解码后为2倍长度)
int ReceiveAudioData(unsigned char* ptrDatBuf,int len, snd_pcm_t *alsa_handle)
{
	short pcmbuf[AUDIO_G711_DATA_LEN];

	printf("r%d\n",len);
	
	memset( (char*)pcmbuf, 0, AUDIO_G711_DATA_LEN*2);

	ptrDatBuf 	+= AUDIO_PACK_HEAD_LEN;
	len			-= AUDIO_PACK_HEAD_LEN;
	
	G711Decoder2(pcmbuf, ptrDatBuf, len, CUR_G711_LAW);	

	alsa_write( alsa_handle, (unsigned char*)pcmbuf, len );
	
	return 0;
}

unsigned int timeGetTime()  
{  
	unsigned int uptime = 0;  
	struct timespec on;  
	if(clock_gettime(CLOCK_MONOTONIC, &on) == 0)  
		uptime = on.tv_sec*1000 + on.tv_nsec/1000000;  
	return uptime;  
}  

// 通過read读取数据后打包发送(得到未编码的数据包，是网络数据包的2倍长度)
int SendAudioData(unsigned short* ptrDatBuf,int len, int socket_id, struct sockaddr* psendaddr, int send_len )
{	
	unsigned char pcmbuf[AUDIO_PACK_HEAD_LEN+AUDIO_G711_DATA_LEN];
	
	au_trs_head.CurrPackage = 1;
	au_trs_head.Datalen = AUDIO_G711_DATA_LEN; ///2;
	au_trs_head.DataType =  1;
	au_trs_head.Framelen = au_trs_head.Datalen;
	au_trs_head.PackLen = 1200;
	au_trs_head.TotalPackage = 1;
	au_trs_head.Timestamp = 0; //timeGetTime();
	// copy数据包头
	memcpy( pcmbuf,(unsigned char*)&au_trs_head, AUDIO_PACK_HEAD_LEN);
	au_trs_head.FrameNo++;	
	if( au_trs_head.FrameNo >= 65535 ) au_trs_head.FrameNo = 1;
	
	// 编码ptrDatBuf中的数据，保存到pcmbuf中，长度为网络传输的数据包长度
	G711Encoder2((short*)ptrDatBuf, pcmbuf+AUDIO_PACK_HEAD_LEN, len, CUR_G711_LAW);	

	int udp_len = AUDIO_PACK_HEAD_LEN+AUDIO_G711_DATA_LEN; ///2;
	// 发送数据包
	if( sendto( socket_id, pcmbuf, udp_len, 0, psendaddr, send_len ) == -1 )
	{
		printf("can not send data from socket! errno:%d,means:%s\n",errno,strerror(errno));
	}

	printf("s%d\n",udp_len);
		
	return 0;
}

