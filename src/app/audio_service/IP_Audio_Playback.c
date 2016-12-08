


#include "alsa.h"
#include "g722.h"
#include "g711common.h"
#include "IP_Audio_Playback.h"
#include "IP_Talk_Servi.h"


#include "../../os/RTOS.h"
#include "../../os/OSQ.h"

// lzh_20160831_s
#include "audio_service.h"
// lzh_20160831_e

pthread_t task_Audio_dec;
pthread_t task_Audio_UDP_Receive;

pthread_attr_t attr_Audio_dec;
pthread_attr_t arrt_Audio_UDP_Receive;


typedef struct
{
	int32_t Sd;
	int32_t addr;
	int32_t Port;
}Audio_RECEIVE_t;
Audio_RECEIVE_t auReceive;

g722_decode_state_t* dec_state;
AlsaCtrl_t* dec_alsa;
static struct sockaddr_in rec_addr;


void api_spk_volume_set( int vol )
{
	vol = vol * 10;
	alsa_card_set_level( vol, CARD_PLAYBACK );
}

int api_spk_volume_get( void )
{
	int vol;
	
	vol = alsa_card_get_level( CARD_PLAYBACK );

	vol = vol / 10;
	
	return vol;
}

static void api_spk_power_on( void )
{
	//API_POWER_TALK_ON();
}

static void api_spk_power_off( void )
{
	//API_POWER_TALK_OFF();
}

static void LoadAudioPlaybackVol( void )
{
	api_spk_volume_set( 10 );

	api_spk_power_on();
}

static int AudioDecode( int16_t decode_data[], const uint8_t pcm_data[], int len )
{
	// lzh_20160831_s
#ifndef AUDIO_FOR_IP8210
	int i 	= 0;
	int dec_len = 0;

	if( GetAudioCode() == G711_ALAW )
	{
		for( i = 0; i < len; i++ )
			decode_data[i] = alaw_to_s16( pcm_data[i] ); 

		dec_len = len;
	}
	else if( GetAudioCode() == G711_ULAW )
	{
		for( i = 0; i < len; i++ )
			decode_data[i] = ulaw_to_s16( pcm_data[i] ); 

		dec_len = len;		
	}
	else if( GetAudioCode() == G722 )
	{
		dec_len = g722_decode(dec_state, decode_data, pcm_data, len );
	}
#else
	AUDIO_HEAD au_rec_head;

	int i	= 0;
	int dec_len = 0;

	// cmpare数据包头
	memcpy( (unsigned char*)&au_rec_head, pcm_data, AUDIO_PACK_HEAD_LEN); 
	if( au_rec_head.DataType != 0x0001 )
		return 0;

	/*
	if( GetAudioCode() == G711_ALAW )
	{
		for( i = 0; i < len; i++ )
			decode_data[i] = alaw_to_s16( pcm_data[i+AUDIO_PACK_HEAD_LEN] ); 

		dec_len = len-AUDIO_PACK_HEAD_LEN;
	}
	else if( GetAudioCode() == G711_ULAW )
	{
		for( i = 0; i < len; i++ )
			decode_data[i] = ulaw_to_s16( pcm_data[i+AUDIO_PACK_HEAD_LEN] ); 

		dec_len = len-AUDIO_PACK_HEAD_LEN;		
	}
	else if( GetAudioCode() == G722 )
	{
		dec_len = g722_decode(dec_state, decode_data, pcm_data+AUDIO_PACK_HEAD_LEN, len )-AUDIO_PACK_HEAD_LEN;
	}
	*/
	if( GetAudioCode() == G711_ALAW )
	{
		G711Decoder2(decode_data,pcm_data+AUDIO_PACK_HEAD_LEN,len,0);	
		dec_len = len-AUDIO_PACK_HEAD_LEN;
	}
	else if( GetAudioCode() == G711_ULAW )
	{
		G711Decoder2(decode_data,pcm_data+AUDIO_PACK_HEAD_LEN,len,1);
		dec_len = len-AUDIO_PACK_HEAD_LEN;		
	}
	else if( GetAudioCode() == G722 )
	{
		dec_len = g722_decode(dec_state, decode_data, pcm_data+AUDIO_PACK_HEAD_LEN, len )-AUDIO_PACK_HEAD_LEN;
	}	
#endif
	// lzh_20160831_e
	return dec_len;
}

static void AudioDecCleanup( void* arg )
{	
	printf("%s\n",__FUNCTION__);
	alsa_write_uninit( dec_alsa );
	
	if( GetAudioCode() == G722 )
		g722_decode_release( dec_state );
}

static void  vtk_TaskProcessEvent_Audio_dec( void )
{
	int err;
	int rate		=	8000; //16000;
	int channels	=	1;
//	int i			= 	0;
	
	u_int32_t addr_len = sizeof( rec_addr );
	unsigned char udpbuffer[1024];

	int16_t dec_data[1024];
	unsigned short dec_size;
	unsigned short data_size;

	dec_alsa	= alsa_write_init( NULL, rate, channels );
	
	if( GetAudioCode() == G722 )
		dec_state = g722_decode_init( NULL, 64000, 0 );	

	pthread_cleanup_push( AudioDecCleanup, NULL );

	if( dec_alsa->handle == NULL && dec_alsa->pcmdev != NULL )
		dec_alsa->handle  = alsa_open_w( dec_alsa->pcmdev, 16, dec_alsa->nchannels == 2, dec_alsa->rate );

	if( dec_alsa->handle == NULL )
	{
		alsa_write_uninit( dec_alsa );		
		if( GetAudioCode() == G722 )
			g722_decode_release( dec_state );	
		// 进入空闲状态
		audio_fm_process_state_set_idle();
		return;
	}
	else
	{
		// 进入运行状态
		audio_fm_process_state_set_run();
	}
	LoadAudioPlaybackVol();

	// lzh_20160816_s
	// 判断接收数据包地址是否与本机地址相同
	int local_ip = GetLocalIp();
	// lzh_20160816_s
	
	while( 1 )
	{
		data_size = recvfrom( auReceive.Sd, udpbuffer, sizeof(udpbuffer), 0, (struct sockaddr*)&rec_addr, &addr_len );
		//printf( "AudioEncode %d \n", data_size );
		//printf("...%08x-%08x...\n",local_ip,rec_addr.sin_addr.s_addr);

		// lzh_20160816_s
		if( local_ip == rec_addr.sin_addr.s_addr )
		{
			printf("...rec local pack...\n");
			continue;
		}
		// lzh_20160816_e

		// lzh_20160831_s
		// AUDIO_G711_DATA_LEN;  // 比较编码传输的长度, 也是解码后写入样本长度(16bits)，
		// lzh_20160831_e

		//printf("---rec=%d:%02x%02x%02x%02x%02x%02x---\n",data_size,udpbuffer[0],udpbuffer[1],udpbuffer[6],udpbuffer[7],udpbuffer[8],udpbuffer[9]);

		// lzh_20160831_s
		#ifndef AUDIO_FOR_IP8210

		printf("r%d\n",data_size);
		
		dec_size = AudioDecode( dec_data, udpbuffer, data_size );
		err = alsa_write( dec_alsa->handle, (unsigned char*)dec_data, dec_size );

		#else

		ReceiveAudioData(udpbuffer, data_size, dec_alsa->handle);

		#endif
		// lzh_20160831_e		
	}

	// lzh_20161114_s
	//pthread_cleanup_pop( 1 );
	pthread_cleanup_pop( 0 );
	// lzh_20161114_e	
}

int api_audio_playback_handle( unsigned int ip, unsigned short port )
{
	if( audio_fm_process_state_get() == FROM_AUDIO_IDLE )
	{
		// 进入启动状态
		audio_fm_process_state_set_start();
	
		auReceive.addr 	= ip;
		auReceive.Port 	= port;
		auReceive.Sd  	= -1; 
		
		if( ( auReceive.Sd = socket( PF_INET, SOCK_DGRAM, 0 )  ) == -1 )
		{
			printf("creating socket failed!");
			// 进入空闲状态
			audio_fm_process_state_set_idle();
			return -1;
		}
	
		// lzh_20160812_s
		#if 0
		int m_loop = 1;
		if( setsockopt(auReceive.Sd, SOL_SOCKET, SO_REUSEADDR, &m_loop, sizeof(m_loop)) == -1 )
		{
			printf("Setsockopt SOL_SOCKET::SO_REUSEADDR Failure!\n");
			return 0;
		}	
		#endif
		// lzh_20160812_e
		
		bzero( &rec_addr, sizeof(rec_addr) );
		rec_addr.sin_family 		= AF_INET;
		rec_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
		rec_addr.sin_port			= auReceive.Port;
		
		if( bind(auReceive.Sd, (struct sockaddr*)&rec_addr, sizeof(rec_addr)) != 0 )
		{
			perror( "______________________________________________________________________bind" );
			// 进入空闲状态
			audio_fm_process_state_set_idle();
			return -1;
		}
	
		pthread_attr_init( &attr_Audio_dec );
		// lzh_20161114_s		
		//pthread_attr_setdetachstate( &attr_Audio_dec, PTHREAD_CREATE_DETACHED );
		// lzh_20161114_e		
		if( pthread_create(&task_Audio_dec,&attr_Audio_dec,(void*)vtk_TaskProcessEvent_Audio_dec, NULL ) != 0 )
		{
			printf( "Create task_Audio_dec pthread error! \n" );
			// 进入空闲状态
			audio_fm_process_state_set_idle();
			return -1;
		}
	}
	
	return 0;		
}

int api_audio_playback_cancel( void )
{
	if( audio_fm_process_state_get() == FROM_AUDIO_RUN )
	{	
		// 进入停止状态
		audio_fm_process_state_set_stop();
		
		api_spk_power_off();
		pthread_cancel( task_Audio_dec ) ;
		pthread_join( task_Audio_dec, NULL );

		close( auReceive.Sd );		

		// 进入空闲状态
		audio_fm_process_state_set_idle();
		
		return 0;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
FROM_AUDIO_STATE_t  FromAudioState = FROM_AUDIO_IDLE;
pthread_mutex_t 	FromAudioState_lock;

int audio_fm_process_state_init( void )
{
	pthread_mutex_init( &FromAudioState_lock, 0);	
	FromAudioState = FROM_AUDIO_IDLE;
	return 0;
}

FROM_AUDIO_STATE_t audio_fm_process_state_get( void )
{
	FROM_AUDIO_STATE_t FromAudioState_read;	
	pthread_mutex_lock( &FromAudioState_lock );		
	FromAudioState_read = FromAudioState;
	pthread_mutex_unlock( &FromAudioState_lock );
	return FromAudioState_read;
}

int audio_fm_process_state_set_idle( void )
{
	pthread_mutex_lock( &FromAudioState_lock );		
	FromAudioState = FROM_AUDIO_IDLE;		
	pthread_mutex_unlock( &FromAudioState_lock );		
	return 0;
}

int audio_fm_process_state_set_start( void )
{
	pthread_mutex_lock( &FromAudioState_lock );		
	FromAudioState = FROM_AUDIO_START;		
	pthread_mutex_unlock( &FromAudioState_lock );		
	return 0;
}

int audio_fm_process_state_set_run( void )
{
	pthread_mutex_lock( &FromAudioState_lock );		
	FromAudioState = FROM_AUDIO_RUN;		
	pthread_mutex_unlock( &FromAudioState_lock );		
	return 0;
}

int audio_fm_process_state_set_stop( void )
{
	pthread_mutex_lock( &FromAudioState_lock );		
	FromAudioState = FROM_AUDIO_STOP;		
	pthread_mutex_unlock( &FromAudioState_lock );		
	return 0;
}

