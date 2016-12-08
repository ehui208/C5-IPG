

#include "alsa.h"
#include "g722.h"
#include "g711common.h"
#include "IP_Audio_Capture.h"
#include "IP_Talk_Servi.h"

#include "../../os/RTOS.h"
#include "../../os/OSQ.h"

// lzh_20160831_s
#include "audio_service.h"
// lzh_20160831_e

typedef struct
{
	int32_t Sd;
	int32_t addr;
	int32_t Port;
} AUDIO_SEND_t;


pthread_t task_Audio_enc;
pthread_t task_Audio_UDP_Send;

pthread_attr_t attr_Audio_enc;
pthread_attr_t arrt_Audio_UDP_Send;


g722_encode_state_t* enc_state;
AlsaCtrl_t* enc_alsa;

static AUDIO_SEND_t SendFD;
static struct sockaddr_in SendAddr;


void api_mic_volume_set( int vol )
{
	vol = vol * 10;
	alsa_card_set_level( vol, CARD_CAPTURE );
}

int api_mic_volume_get( void )
{
	int vol;
	
	vol = alsa_card_get_level( CARD_CAPTURE );

	vol = vol / 10;
	
	return vol;
}

static void api_mic_power_on( void )
{
	//API_POWER_TALK_ON();
}

static void api_mic_power_off( void )
{
	//API_POWER_TALK_OFF();
}

static void LoadAudioCaptureVol( void )
{
	api_mic_volume_set( 7 );

	api_mic_power_on();
}

static int AudioEncode( uint8_t encode_data[], const int16_t pcm_data[], int len )
{
	// lzh_20160831_s
#ifndef AUDIO_FOR_IP8210
	int i 	= 0;
	int enc_len = 0;

	if( GetAudioCode() == G711_ALAW )
	{
		for( i = 0; i < len; i++ )
			encode_data[i] = s16_to_alaw( ((int16_t*)pcm_data)[i] ); 

		enc_len = len;		
	}
	else if( GetAudioCode() == G711_ULAW )
	{
		for( i = 0; i < len; i++ )
			encode_data[i] = s16_to_ulaw( ((int16_t*)pcm_data)[i] ); 

		enc_len = len;		
	}
	else if( GetAudioCode() == G722 )
	{
		enc_len = g722_encode(enc_state, encode_data, (int16_t*)pcm_data, len );
	}
#else
	extern AUDIO_HEAD au_trs_head;
	int i	= 0;
	int enc_len = 0;

	// copy数据包头
	memcpy( encode_data,(unsigned char*)&au_trs_head, AUDIO_PACK_HEAD_LEN); 

	/*
	if( GetAudioCode() == G711_ALAW )
	{
		for( i = 0; i < len; i++ )
			encode_data[i+AUDIO_PACK_HEAD_LEN] = s16_to_alaw( ((int16_t*)pcm_data)[i] ); 

		enc_len = len+AUDIO_PACK_HEAD_LEN;		
	}
	else if( GetAudioCode() == G711_ULAW )
	{
		for( i = 0; i < len; i++ )
			encode_data[i+AUDIO_PACK_HEAD_LEN] = s16_to_ulaw( ((int16_t*)pcm_data)[i] ); 

		enc_len = len+AUDIO_PACK_HEAD_LEN;		
	}
	else if( GetAudioCode() == G722 )
	{
		enc_len = g722_encode(enc_state, encode_data+AUDIO_PACK_HEAD_LEN, (int16_t*)pcm_data, len )+AUDIO_PACK_HEAD_LEN;
	}
	*/
	if( GetAudioCode() == G711_ALAW )
	{
		G711Encoder2(pcm_data,encode_data+AUDIO_PACK_HEAD_LEN,len,0);
		enc_len = len+AUDIO_PACK_HEAD_LEN;		
	}
	else if( GetAudioCode() == G711_ULAW )
	{
		G711Encoder2(pcm_data,encode_data+AUDIO_PACK_HEAD_LEN,len,1);
		enc_len = len+AUDIO_PACK_HEAD_LEN;		
	}
	else if( GetAudioCode() == G722 )
	{
		enc_len = g722_encode(enc_state, encode_data+AUDIO_PACK_HEAD_LEN, (int16_t*)pcm_data, len )+AUDIO_PACK_HEAD_LEN;
	}
	
	au_trs_head.FrameNo++;
	
#endif
	// lzh_20160831_e
	return enc_len;
}

static void AudioEncCleanup( void* arg )
{	
	printf("%s\n",__FUNCTION__);

	alsa_read_uninit( enc_alsa );

	if( GetAudioCode() == G722 )
		g722_encode_release( enc_state );
}

static void vtk_TaskProcessEvent_Audio_enc( void )
{
	int err;
	int rate 		= 	8000; //16000;
	int channels	= 	1;
	// lzh_20160831_s
#ifndef AUDIO_FOR_IP8210	
	int samples 	=	(160 * rate)/8000;	
#else
	int samples 	=	AUDIO_G711_DATA_LEN;  // 读取样本(16bits)，也是编码后传输的长度
	InitialAudioPackHead(); 
#endif	
	// lzh_20160831_e
//	int size		=	samples * 2 * channels;

	
	u_int32_t addr_len = sizeof( SendAddr );

	unsigned char buffer[1024];
	unsigned char enc_data[1024];
	unsigned short enc_size;

	pthread_cleanup_push( AudioEncCleanup, NULL );		

	enc_alsa 	= alsa_read_init( NULL, rate, channels );

	if( GetAudioCode() == G722 )
		enc_state= g722_encode_init( NULL, 64000, 0 );		

	if( enc_alsa->handle == NULL && enc_alsa->pcmdev != NULL )
		enc_alsa->handle  = alsa_open_r( enc_alsa->pcmdev, 16, enc_alsa->nchannels == 2, enc_alsa->rate );

	if( enc_alsa->handle == NULL )
	{
		alsa_read_uninit( enc_alsa );
		
		if( GetAudioCode() == G722 )
			g722_encode_release( enc_state );
		
		// 进入空闲状态
		audio_to_process_state_set_idle();
		
		return;	
	}
	else
	{
		// 进入运行状态
		audio_to_process_state_set_run();
	}
	
	LoadAudioCaptureVol();
	while( 1 )
	{
		if( alsa_can_read( enc_alsa->handle ) >= samples )
		{		
			if( ( err = alsa_read( enc_alsa->handle, buffer, samples ) ) <= 0 ) 
			{
				printf( "Fail to read samples\n" );
				return;
			}		

			// lzh_20160831_s
			#ifndef AUDIO_FOR_IP8210
			
			enc_size = AudioEncode( enc_data, (int16_t*)buffer, samples );

			//printf("s\n");
			//printf("---snd=%d:%02x%02x%02x%02x%02x%02x---\n",enc_size,enc_data[0],enc_data[1],enc_data[6],enc_data[7],enc_data[8],enc_data[9]);

			if( sendto( SendFD.Sd, enc_data, enc_size, 0, (struct sockaddr*)&SendAddr, addr_len ) == -1 )
			{
				printf("can not send data from socket! errno:%d,means:%s\n",errno,strerror(errno));
			}

			#else

			SendAudioData( (int16_t*)buffer, samples, SendFD.Sd, (struct sockaddr*)&SendAddr, addr_len );
			
			#endif
			// lzh_20160831_e
			
		}
	}
	
	// lzh_20161114_s
	//pthread_cleanup_pop( 1 );
	pthread_cleanup_pop( 0 );
	// lzh_20161114_e	
}

int api_audio_capture_handle( unsigned int ip, unsigned short port )
{
	if( audio_to_process_state_get() == TO_AUDIO_IDLE )
	{
		// 进入启动状态
		audio_to_process_state_set_start();
	
		SendFD.addr 	= ip;
		SendFD.Port 	= port;
		SendFD.Sd 	= -1;
		
		if( ( SendFD.Sd = socket( PF_INET, SOCK_DGRAM, 0 ) ) == -1 )
		{
			printf("creating socket failed!");
			// 进入空闲状态
			audio_to_process_state_set_idle();
			return 0;
		}

		// lzh_20160812_s
		#if 0
		int m_loop = 1;
		if( setsockopt(SendFD.Sd, SOL_SOCKET, SO_REUSEADDR, &m_loop, sizeof(m_loop)) == -1 )
		{
			printf("Setsockopt SOL_SOCKET::SO_REUSEADDR Failure!\n");
			return 0;
		}	
		#endif
		// lzh_20160812_e

		bzero( &SendAddr, sizeof(SendAddr) );
		SendAddr.sin_family 		= AF_INET;
		SendAddr.sin_addr.s_addr	= SendFD.addr;
		SendAddr.sin_port		= SendFD.Port;

		pthread_attr_init( &attr_Audio_enc );		
		// lzh_20161114_s
		//pthread_attr_setdetachstate( &attr_Audio_enc, PTHREAD_CREATE_DETACHED );
		// lzh_20161114_e		
		if( pthread_create( &task_Audio_enc, &attr_Audio_enc, (void*)vtk_TaskProcessEvent_Audio_enc, NULL) != 0 )
		{
			printf( "Create task_H264_enc pthread error! \n" );
			// 进入空闲状态
			audio_to_process_state_set_idle();
		}
	}

	return 0;
}

int  api_audio_capture_cancel( void )
{
	if( audio_to_process_state_get() == TO_AUDIO_RUN )
	{	
		// 进入停止状态
		audio_to_process_state_set_stop();
	
		api_mic_power_off();
		
		pthread_cancel( task_Audio_enc ) ;
		pthread_join( task_Audio_enc, NULL );
	
		close( SendFD.Sd  );			
		
		// 进入空闲状态
		audio_to_process_state_set_idle();
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
TO_AUDIO_STATE_t 	ToAudioState = TO_AUDIO_IDLE;
pthread_mutex_t 	ToAudioState_lock;

int audio_to_process_state_init( void )
{
	pthread_mutex_init( &ToAudioState_lock, 0);	
	ToAudioState = TO_AUDIO_IDLE;
	return 0;
}

TO_AUDIO_STATE_t audio_to_process_state_get( void )
{
	TO_AUDIO_STATE_t ToAudioState_read;	
	pthread_mutex_lock( &ToAudioState_lock );		
	ToAudioState_read = ToAudioState;
	pthread_mutex_unlock( &ToAudioState_lock );
	return ToAudioState_read;
}

int audio_to_process_state_set_idle( void )
{
	pthread_mutex_lock( &ToAudioState_lock );		
	ToAudioState = TO_AUDIO_IDLE;		
	pthread_mutex_unlock( &ToAudioState_lock );		
	return 0;
}

int audio_to_process_state_set_start( void )
{
	pthread_mutex_lock( &ToAudioState_lock );		
	ToAudioState = TO_AUDIO_START;		
	pthread_mutex_unlock( &ToAudioState_lock );		
	return 0;
}

int audio_to_process_state_set_run( void )
{
	pthread_mutex_lock( &ToAudioState_lock );		
	ToAudioState = TO_AUDIO_RUN;		
	pthread_mutex_unlock( &ToAudioState_lock );		
	return 0;
}

int audio_to_process_state_set_stop( void )
{
	pthread_mutex_lock( &ToAudioState_lock );		
	ToAudioState = TO_AUDIO_STOP;		
	pthread_mutex_unlock( &ToAudioState_lock );		
	return 0;
}



