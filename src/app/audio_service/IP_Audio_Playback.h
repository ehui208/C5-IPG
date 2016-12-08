
#ifndef _AUDIO_PLAYBACK_PROCESS_H_
#define _AUDIO_PLAYBACK_PROCESS_H_



void api_spk_volume_set( int vol );
int api_spk_volume_get( void );

int api_audio_playback_handle( unsigned int ip, unsigned short port );
int api_audio_playback_cancel( void );

///////////////////////////////////////////////////////////////////////
typedef enum
{
	 FROM_AUDIO_IDLE,
	 FROM_AUDIO_START,
	 FROM_AUDIO_RUN,
	 FROM_AUDIO_STOP,
}
FROM_AUDIO_STATE_t;

int audio_fm_process_state_init( void );
FROM_AUDIO_STATE_t audio_fm_process_state_get( void );
int audio_fm_process_state_set_idle( void );
int audio_fm_process_state_set_start( void );
int audio_fm_process_state_set_run( void );
int audio_fm_process_state_set_stop( void );


#endif

