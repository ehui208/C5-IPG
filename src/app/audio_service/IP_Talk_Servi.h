

#ifndef _IP_TALK_SERVI_H_
#define _IP_TALK_SERVI_H_

#include "IP_Audio_Capture.h"
#include "IP_Audio_Playback.h"
#include "IP_Audio_Control.h"
#include "IP_Audio_Ring.h"


typedef enum
{
	 G711_ALAW,
	 G711_ULAW,
	 G722,
}
AUDIO_CODE_t;


void SetAudioCode( AUDIO_CODE_t code );
AUDIO_CODE_t GetAudioCode( void );
void LoadAudioCode( void );
int api_talk_turn_on(void );
int api_talk_turn_off( void );



#endif

