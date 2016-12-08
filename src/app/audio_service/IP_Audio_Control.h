

#ifndef _IP_AUDIO_CONTROL_H_
#define _IP_AUDIO_CONTROL_H_


typedef enum
{
	AUDIO_IDLE,
	AUDIO_UNICAST,
	AUDIO_MULTICAST,
}
AUDIO_TYPE_t;


typedef struct
{
	AUDIO_TYPE_t 	type;
	int32_t			source_ip_address;
	int32_t			target_ip_address;	
	int32_t			source_port;
	int32_t			target_port;	
} AUDIO_DATA_t;


int SetAudioType( AUDIO_DATA_t* data );
int GetAudioType( AUDIO_DATA_t* data );

int API_talk_on( void );
// lzh_20160422_s
int API_talk_on_by_unicast( int target_ip, short client_port, short server_port );
// lzh_20160422_s
int API_talk_off( void );
int API_WavePlayer_Play( int logicTune );
int API_WavePlayer_Stop( void );
int API_mic_volume_set( int vol );
int API_mic_volume_get( void );
int API_spk_volume_set( int vol );
int API_spk_volume_get( void );
int API_WavePlayer_SetVolume( int vol );
int API_WavePlayer_GetVolume( void );
int API_PlayList_GetUsingLength_ms( unsigned char TuneID, unsigned long * time );

int audio_to_process_state_init();
int audio_fm_process_state_init();;

#endif

