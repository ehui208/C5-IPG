
#include "IP_Talk_Servi.h"
#include "IP_Audio_Ring.h"
#include "obj_PlayList.h"


typedef enum
{
	AUDIO_CONTROL_IDLE,
	AUDIO_CONTROL_TALK,
	AUDIO_CONTROL_RING,
}
AUDIO_STATE_t;

AUDIO_STATE_t audio_state = AUDIO_CONTROL_IDLE;

AUDIO_DATA_t  test_data;

/*********************************************************************
*
*       SetAudioType
*
*  Function description:
*       设置Audio网络参数.
*
*  Parameter:
*    data:     
*
*  Return value:
*    0 : 成功
*/
int SetAudioType( AUDIO_DATA_t* data )
{
	test_data.type					= data->type;
	test_data.source_ip_address	= data->source_ip_address;
	test_data.target_ip_address		= data->target_ip_address;
	test_data.source_port			= data->source_port;
	test_data.target_port			= data->target_port;

	return 0;
}

/*********************************************************************
*
*       GetAudioType
*
*  Function description:
*       读取Audio网络参数.
*
*  Parameter:
*    data:     
*
*  Return value:
*    0 : 成功
*/
int GetAudioType( AUDIO_DATA_t* data )
{
	data->type 				= test_data.type;
	data->source_ip_address 	= test_data.source_ip_address;
	data->target_ip_address 	= test_data.target_ip_address;
	data->source_port 		= test_data.source_port;
	data->target_port 			= test_data.target_port;

	return 0;
}

/*********************************************************************
*
*       API_talk_on
*
*  Function description:
*       启动音频.
*
*  Parameter:
*   
*
*  Return value:
*    0 : 成功
*    1 : 失败
*/
int API_talk_on( void )
{
	//if( audio_state == AUDIO_CONTROL_IDLE )
	{
		audio_state = AUDIO_CONTROL_TALK;
		api_talk_turn_on();
		return 0;
	}
	return 1;
}

// lzh_20160422_s
int API_talk_on_by_unicast( int target_ip, short client_port, short server_port )
{
	AUDIO_DATA_t net;	

	//if( audio_state == AUDIO_CONTROL_IDLE )
	{
		audio_state = AUDIO_CONTROL_TALK;
		
		net.target_ip_address	= target_ip;
		net.source_port 		= htons( client_port );
		net.target_port 		= htons( server_port );
		SetAudioType( &net );

		api_talk_turn_on();
	}
	//return 0;
}
// lzh_20160422_s

/*********************************************************************
*
*       API_talk_off
*
*  Function description:
*       关闭音频.
*
*  Parameter:
*   
*
*  Return value:
*    0 : 成功
*    1 : 失败
*/
int API_talk_off( void )
{
	//if( audio_state == AUDIO_CONTROL_TALK )
	{
		audio_state = AUDIO_CONTROL_IDLE;
		api_talk_turn_off();
		return 0;
	}
	//return 1;
}

/*********************************************************************
*
*       API_WavePlayer_Play
*
*  Function description:
*       启动铃声播放.
*
*  Parameter:
*       逻辑曲号.
*
*  Return value:
*    0 : 成功
*    1 : 失败
*/
int API_WavePlayer_Play( int logicTune )
{
	if( audio_state == AUDIO_CONTROL_IDLE )
	{
		audio_state = AUDIO_CONTROL_RING;
		api_RingPlay( logicTune );
		return 0;
	}
	return 1;
}

/*********************************************************************
*
*       API_WavePlayer_Stop
*
*  Function description:
*       停止铃声播放.
*
*  Parameter:
*       
*
*  Return value:
*    0 : 成功
*    1 : 失败
*/
int API_WavePlayer_Stop( void )
{
	if( audio_state == AUDIO_CONTROL_RING )
	{
		audio_state = AUDIO_CONTROL_IDLE;
		api_WavePlayer_Stop();
		usleep(200000);
		return 0;
	}
	return 1;
}

/*********************************************************************
*
*       API_mic_volume_set
*
*  Function description:
*       设置咪头音量
*
*  Parameter:
*       音量 0 ～ 10
*
*  Return value:
*    0 : 成功
*
*/
int API_mic_volume_set( int vol )
{
	api_mic_volume_set( vol );

	return 0;
}

/*********************************************************************
*
*       API_mic_volume_gt
*
*  Function description:
*       读取咪头音量
*
*  Parameter:
*       
*
*  Return value:
*    音量 0 ～ 10
*
*/
int API_mic_volume_get( void )
{
	return api_mic_volume_get();
}

/*********************************************************************
*
*       API_spk_volume_set
*
*  Function description:
*       设置喇叭音量
*
*  Parameter:
*       音量 0 ～ 10
*
*  Return value:
*    0 : 成功
*
*/
int API_spk_volume_set( int vol )
{
	api_spk_volume_set( vol );

	return 0;
}

/*********************************************************************
*
*       API_spk_volume_gt
*
*  Function description:
*       读取喇叭音量
*
*  Parameter:
*       
*
*  Return value:
*    音量 0 ～ 10
*
*/
int API_spk_volume_get( void )
{
	return api_spk_volume_get();
}

/*********************************************************************
*
*       API_WavePlayer_SetVolume
*
*  Function description:
*       设置铃声音量
*
*  Parameter:
*       音量 0 ～ 10
*
*  Return value:
*    0 : 成功
*
*/
int API_WavePlayer_SetVolume( int vol )
{
	api_ring_volume_set( vol );
	usleep(100000);

	return 0;
}

/*********************************************************************
*
*       API_WavePlayer_GetVolume
*
*  Function description:
*       读取铃声音量
*
*  Parameter:
*       
*
*  Return value:
*    音量 0 ～ 10
*
*/
int API_WavePlayer_GetVolume( void )
{
	return api_ring_volume_get();
}

/*********************************************************************
*
*       API_PlayList_GetUsingLength_ms
*
*  Function description:
*       读取铃声长度
*
*  Parameter:
*       
*
*  Return value:
*    >= 0 铃声长度
*    < 0  错误
*/
int API_PlayList_GetUsingLength_ms( unsigned char TuneID, unsigned long * time )
{
	return PlayList_GetUsingLength_ms( TuneID,  time);
}

