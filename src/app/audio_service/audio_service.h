
#ifndef _AUDIO_SERVICE_H
#define _AUDIO_SERVICE_H

#include <sys/socket.h>

#include "alsa.h"

#include "g711core.h"

#pragma pack(2)

typedef struct _AUDIO_HEAD_
{
	char 			magic[6];
	//
	unsigned short DataType;          //数据类型
	unsigned short FrameNo;           //帧序号
	unsigned int   Timestamp;         //时间戳
	unsigned int   Framelen;          //帧数据长度
	unsigned short TotalPackage;      //总包数
	unsigned short CurrPackage;       //当前包数
	unsigned short Datalen;           //数据长度
	unsigned short PackLen;           //数据包大小
} AUDIO_HEAD;

#pragma pack()

// 数据序列- SWITCH + 0100 + SN(0100.0200...) + DATA
// datlen=90   
//53,57,49,54,43,48, 01,00,46,00, 44,02,00,00,40,00   00,00,01,00
/*
//53,57,49,54,43,48, 01,00, af,0b, 84,5d,00,00, 40,00,00,00, 01,00, 01,00, 40,00, b0,04, 
ff,08,9f,ff,8f,0a,08,02,ff,b6,8f,8d,07,ff,96,06,0a,0d,81,13,03,08
89,01,ff,7f,ff,8c,86,b0,9f,06,a4,25,22,90,86,ff
*/

#define 	AUDIO_PACK_HEAD_LEN		sizeof(AUDIO_HEAD)
#define 	AUDIO_G711_DATA_LEN		64 //128

#define AUDIO_FOR_IP8210

void InitialAudioPackHead(void);

int SendAudioData(unsigned short* ptrDatBuf,int len, int socket_id, struct sockaddr* psendaddr, int send_len );
int ReceiveAudioData(unsigned char* ptrDatBuf,int len, snd_pcm_t *alsa_handle);


#endif

