
#include "waveheader.h"
#include "alsa.h"
#include "obj_PlayList.h"
#include "../../os/RTOS.h"
#include "../../os/OSQ.h"

//#define ms_message(...)
//#define ms_warning(...)
//#define ms_debug(...)
//#define ms_error(...)


typedef enum
{
	 AUDIO_RING_IDLE,
	 AUDIO_RING_RUN,
}
AUDIO_RING_STATE_t;

typedef struct
{
	int fd;
	int rate;
	int nchannels;
	int hsize;
	int samplesize;
	BOOL is_raw;
} WAVE_DATA;


pthread_t task_Ring_dec;
pthread_attr_t attr_Ring_dec;

pthread_t T_task_Ring_dec;
pthread_attr_t T_attr_Ring_dec;


WAVE_DATA wave_data;
WAVE_DATA T_wave_data;

AlsaCtrl_t* ring_alsa;
AlsaCtrl_t* T_ring_alsa;

static unsigned char TuneID = 0;

AUDIO_RING_STATE_t  AudioRingState = AUDIO_RING_IDLE;

void api_ring_volume_set( int vol )
{
	vol = vol * 10;
	alsa_card_set_level( vol, CARD_PLAYBACK );
}

int api_ring_volume_get( void )
{
	int vol;
	
	vol = alsa_card_get_level( CARD_PLAYBACK );

	vol = vol / 10;
	
	return vol;
}

static void api_ring_power_on( void )
{

}

static void api_ring_power_off( void )
{

}

static void LoadAudioRingVol( void )
{
	api_ring_volume_set( 1 );

	api_ring_power_on();
}


static int ms_read_wav_header_from_fd( wave_header_t *header, int fd )
{
	int count;
	int skip;
	int hsize = 0;
	riff_t *riff_chunk 			=& header->riff_chunk;
	format_t *format_chunk	=& header->format_chunk;
	data_t *data_chunk		=& header->data_chunk;
	
	unsigned long len = 0;
	
	len = read( fd, (char*)riff_chunk, sizeof(riff_t) );
	
	if( len != sizeof(riff_t) )
	{
		goto not_a_wav;
	}
	
	if( 0 != strncmp(riff_chunk->riff, "RIFF", 4) || 0 != strncmp(riff_chunk->wave, "WAVE", 4) )
	{
		goto not_a_wav;
	}
	
	len = read( fd, (char*)format_chunk, sizeof(format_t) );
	
	if( len != sizeof(format_t) )
	{
		ms_warning("Wrong wav header: cannot read file");
		goto not_a_wav;
	}
	
	if( (skip = le_uint32(format_chunk->len) - 0x10) > 0 )
	{
		lseek( fd, skip, SEEK_CUR );
	}
	
	hsize = sizeof(wave_header_t) - 0x10 + le_uint32(format_chunk->len);
	
	count = 0;
	do{
		len = read( fd, data_chunk, sizeof(data_t) );

		if( len != sizeof(data_t) )
		{
			ms_warning("Wrong wav header: cannot read file\n");
			goto not_a_wav;
		}

		if( strncmp(data_chunk->data, "data", 4) != 0 )
		{
			ms_warning("skipping chunk=%s len=%i\n", data_chunk->data, data_chunk->len);
			lseek( fd, le_uint32(data_chunk->len), SEEK_CUR );
			count++;
			hsize += len + le_uint32(data_chunk->len);
		}
		else
		{
			hsize += len;
			break;
		}
	}while( count < 30 );
	
	return hsize;

	not_a_wav:
		/*rewind*/
		lseek( fd, 0, SEEK_SET );
		return -1;
}

static int read_wav_header( WAVE_DATA* data )
{
	wave_header_t header;
	format_t *format_chunk =& header.format_chunk;
	int ret = ms_read_wav_header_from_fd( &header, data->fd );
	
	if( ret == -1 ) 
		goto not_a_wav;
	
	data->rate		= le_uint32( format_chunk->rate );
	data->nchannels	= le_uint16( format_chunk->channel );
	
	if( data->nchannels == 0 ) 
		goto not_a_wav;
	
	data->samplesize	= le_uint16( format_chunk->blockalign ) / data->nchannels;
	data->hsize		= ret;
	
	#ifdef WORDS_BIGENDIAN
	if( le_uint16(format_chunk->blockalign) == le_uint16(format_chunk->channel) * 2 )
		data->swap=TRUE;
	#endif
	
	data->is_raw = FALSE;
	
	return 0;

	not_a_wav:
	{
		/*rewind*/
		lseek( data->fd, 0, SEEK_SET );
		data->hsize 	= 0;
		data->is_raw = TRUE;
		return -1;
	}
}


static int player_open( WAVE_DATA* data, void *arg )
{
	const char *file = (const char*)arg;
	
	if( (data->fd = open(file,O_RDONLY|O_BINARY))==-1 )
	{
		ms_warning("Failed to open %s\n",file);
		return -1;
	}
	
	if( read_wav_header( data ) != 0 && strstr(file,".wav") )
	{
		ms_warning("File %s has .wav extension but wav header could be found.\n",file);
	}
	
	//ms_message("%s opened: rate=%i,channel=%i\n",file,d->rate,d->nchannels);
	printf("%s opened: rate=%i,channel=%i,d->samplesize=%i\n",file, data->rate, data->nchannels, data->samplesize);
	return 0;
}


static void AudioRingCleanup( void *arg )
{	
	printf( " AudioRingCleanup \n ");
	
	alsa_write_uninit( ring_alsa );
	
//	AudioRingState = AUDIO_RING_IDLE;
}

static void  vtk_TaskProcessEvent_Ring( void )
{
	int size		= 0;
	int err 		= 0;
	int samples	= 0;	
	char buff[1024];	
	//char* buffer;

	PlayList_GetFileName( buff, TuneID );

	player_open( &wave_data, buff );

	ring_alsa = alsa_write_init( NULL, wave_data.rate, wave_data.nchannels );

	if( ring_alsa->handle == NULL && ring_alsa->pcmdev != NULL )
		ring_alsa->handle  = alsa_open_w( ring_alsa->pcmdev, 24, ring_alsa->nchannels == 2, ring_alsa->rate );

	if( ring_alsa->handle == NULL )
		return;


	pthread_cleanup_push( AudioRingCleanup, NULL );

	//LoadAudioRingVol();

	size = 512;

	while( 1 )
	{				
		err = read( wave_data.fd, buff, size );

		if( err == 0 )
			break;

		samples = size / ( 2 * wave_data.nchannels );

		err = alsa_write( ring_alsa->handle, (unsigned char*)buff, samples );
		
		if( err < 0)
			break;	
	}

	pthread_cleanup_pop( 1 );	
}

#if 0
void T_AudioRingCleanup( void *arg )
{	
	printf( " T_AudioRingCleanup \n ");
	
	alsa_write_uninit( T_ring_alsa );
}

void  T_vtk_TaskProcessEvent_Ring( void )
{
	int rate		= 16000;
	int channels	= 1;
	int size		= 0;
	int err 		= 0;
	int samples	= 0;	
	char buff[512];	

	player_open( &T_wave_data, "/mnt/nand1-2/rings/Popular.wav" );

	T_ring_alsa = alsa_ring_init( NULL, rate, channels );

	if( T_ring_alsa->handle == NULL && T_ring_alsa->pcmdev != NULL )
		T_ring_alsa->handle  = alsa_open_w( T_ring_alsa->pcmdev, 16, T_ring_alsa->nchannels == 2, T_ring_alsa->rate );

	if( T_ring_alsa->handle == NULL )
		return;

	pthread_cleanup_push( T_AudioRingCleanup, NULL );
	
	while( 1 )
	{				
		size = read( T_wave_data.fd, buff, 256 );

		if( size == 0 )
			break;

		//samples = size / ( 2 * T_wave_data.nchannels );
		err = alsa_write( T_ring_alsa->handle, buff, size*2 );

		if( err < 0)
			break;	
	}

	pthread_cleanup_pop( 1 );	
}
#endif

void api_RingPlay( int logicTune )
{
	if( AudioRingState == AUDIO_RING_IDLE )
	{
		AudioRingState = AUDIO_RING_RUN;

		TuneID = logicTune;
		
		pthread_attr_init( &attr_Ring_dec );
		pthread_attr_setdetachstate( &attr_Ring_dec, PTHREAD_CREATE_DETACHED );
		if( pthread_create(&task_Ring_dec, &attr_Ring_dec, (void*)vtk_TaskProcessEvent_Ring, NULL ) != 0 )
		{
			printf( "Create task_Audio_dec pthread error! \n" );
			exit(1);
		}
#if 0
		pthread_attr_init( &T_attr_Ring_dec );
		pthread_attr_setdetachstate( &T_attr_Ring_dec, PTHREAD_CREATE_DETACHED );
		if( pthread_create(&T_task_Ring_dec, &T_attr_Ring_dec, (void*)T_vtk_TaskProcessEvent_Ring, NULL ) != 0 )
		{
			printf( "Create task_Audio_dec pthread error! \n" );
			exit(1);
		}		
#endif		
	}
}

void api_WavePlayer_Stop( void )
{
	if( AudioRingState == AUDIO_RING_RUN )
	{
		//api_ring_power_off();
		
		pthread_cancel( task_Ring_dec ) ;
		pthread_join( task_Ring_dec, NULL );
		AudioRingState = AUDIO_RING_IDLE;		
#if 0
		pthread_cancel( T_task_Ring_dec ) ;
		pthread_join( T_task_Ring_dec, NULL );		
#endif
	}
}

