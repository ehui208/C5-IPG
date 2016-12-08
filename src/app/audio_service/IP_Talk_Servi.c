
#include "IP_Talk_Servi.h"




AUDIO_CODE_t audio_code = G711_ALAW;


void SetAudioCode( AUDIO_CODE_t code )
{
	audio_code = code;
}

AUDIO_CODE_t GetAudioCode( void )
{
	return audio_code;
}

void LoadAudioCode( void )
{
	SetAudioCode( G711_ALAW );
}

int api_talk_turn_on( void )
{
	AUDIO_DATA_t net;
	
	LoadAudioCode();

	GetAudioType( &net );
	
	api_audio_capture_handle( net.target_ip_address, net.target_port );

	api_audio_playback_handle( net.target_ip_address, net.source_port);

	return 0;
}

int api_talk_turn_off( void )
{
	printf("%s\n",__FUNCTION__);
	api_audio_capture_cancel();

	api_audio_playback_cancel();
	
	return 0;
}

