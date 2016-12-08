

#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <stdlib.h>

#include "g722.h"
#include "alsa.h"


//#define THREADED_VERSION

/*in case of troubles with a particular driver, try incrementing ALSA_PERIOD_SIZE
to 512, 1024, 2048, 4096...
then try incrementing the number of periods*/
#define ALSA_PERIODS 		8
#define ALSA_PERIOD_SIZE 		1024

/*uncomment the following line if you have problems with an alsa driver
having sound quality trouble:*/
//#define EPIPE_BUGFIX 1





void scale_down( int16_t* samples, int count )
{
	int i;
	for( i = 0; i < count; ++i )
		samples[i] = samples[i] >> 1;
}

void scale_up( int16_t* samples, int count )
{
	int i;
	for( i = 0; i < count; ++i )
		samples[i] = samples[i] << 1;
}

#ifdef EPIPE_BUGFIX
static void alsa_fill_w( snd_pcm_t* pcm_handle )
{
	snd_pcm_hw_params_t *hwparams = NULL;
	int channels;
        snd_pcm_uframes_t buffer_size;
	int buffer_size_bytes;
	void *buffer;

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca( &hwparams );
	snd_pcm_hw_params_current( pcm_handle, hwparams );

	/* get channels */
	snd_pcm_hw_params_get_channels( hwparams, &channels );

	/* get buffer size */
	snd_pcm_hw_params_get_buffer_size( hwparams, &buffer_size );

	/* fill half */
	buffer_size /= 2;

	/* allocate buffer assuming 2 bytes per sample */
	buffer_size_bytes = buffer_size * channels * 2;
	buffer = alloca( buffer_size_bytes );
	memset( buffer, 0, buffer_size_bytes );

	/* write data */
	snd_pcm_writei( pcm_handle, buffer, buffer_size );
}
#endif


static snd_mixer_t* alsa_mixer_open( const char* mixdev )
{
	snd_mixer_t* mixer = NULL;
	int err;
		
	if( ( err = snd_mixer_open( &mixer, 0 ) ) < 0 )
	{
		ms_warning("Could not open alsa mixer: %s\n",snd_strerror(err));
		return NULL;
	}
	
	if( ( err = snd_mixer_attach( mixer, mixdev ) ) < 0 )
	{
		ms_warning( "Could not attach mixer to card: %s\n",snd_strerror(err) );
		snd_mixer_close( mixer );
		return NULL;
	}
	
	if( ( err = snd_mixer_selem_register( mixer, NULL, NULL ) ) < 0 )
	{
		ms_warning( "snd_mixer_selem_register: %s\n",snd_strerror(err) );
		snd_mixer_close( mixer );
		return NULL;
	}
	
	if( ( err = snd_mixer_load( mixer ) ) < 0 )
	{
		ms_warning( "snd_mixer_load: %s\n",snd_strerror(err) );
		snd_mixer_close( mixer );
		return NULL;
	}
	
	return mixer;
}

static void alsa_mixer_close( snd_mixer_t* mix )
{
	snd_mixer_close( mix );
}

typedef enum 
{
	CAPTURE, 
	PLAYBACK, 
	CAPTURE_SWITCH, 
	PLAYBACK_SWITCH
} MixerAction;

static int get_mixer_element( snd_mixer_t *mixer, const char *name, MixerAction action )
{
	const char *elemname;
	snd_mixer_elem_t *elem;
	int err;
	long sndMixerPMin	= 0;
	long sndMixerPMax 	= 0;
	long newvol 			= 0;
	long value			= 0;	
	
	elem = snd_mixer_first_elem( mixer );
	
	while( elem != NULL )
	{
		elemname = snd_mixer_selem_get_name( elem );
		//ms_message("Found alsa mixer element %s.",elemname);
		if( strcmp( elemname, name ) == 0 )
		{
			switch( action )
			{
				case CAPTURE:
					if( snd_mixer_selem_has_capture_volume(elem) )
					{
						snd_mixer_selem_get_capture_volume_range( elem, &sndMixerPMin, &sndMixerPMax );
						err = snd_mixer_selem_get_capture_volume( elem, SND_MIXER_SCHN_UNKNOWN, &newvol );
						newvol -= sndMixerPMin;
						value = ( 100 * newvol ) / ( sndMixerPMax - sndMixerPMin );
						if( err < 0 ) 
							ms_warning( "Could not get capture volume for %s:%s\n", name, snd_strerror(err) );
						//else ms_message("Successfully get capture level for %s.",elemname);
						break;
					}
					break;
					
				case PLAYBACK:
					if( snd_mixer_selem_has_playback_volume( elem ) )
					{
						snd_mixer_selem_get_playback_volume_range( elem, &sndMixerPMin, &sndMixerPMax );
						err = snd_mixer_selem_get_playback_volume( elem, SND_MIXER_SCHN_FRONT_LEFT, &newvol );
						newvol -= sndMixerPMin;
						value = ( 100 * newvol ) / ( sndMixerPMax - sndMixerPMin );
						if( err < 0 ) 
							ms_warning( "Could not get playback volume for %s:%s\n", name, snd_strerror(err) );
						//else ms_message("Successfully get playback level for %s.",elemname);
						break;
					}
					break;
					
				case CAPTURE_SWITCH:				
					break;
					
				case PLAYBACK_SWITCH:
					break;
			}
		}
		
		elem = snd_mixer_elem_next( elem );
	}
	
	return value;
}


static void set_mixer_element( snd_mixer_t *mixer, const char *name, int level, MixerAction action )
{
	const char *elemname;
	snd_mixer_elem_t *elem;
	long sndMixerPMin	= 0;
	long sndMixerPMax	= 0;
	long newvol			= 0;
	
	elem = snd_mixer_first_elem( mixer );
	
	while( elem != NULL )
	{
		elemname = snd_mixer_selem_get_name( elem );
		ms_message( "Found alsa mixer element %s.      is %s\n", elemname, name );
		if( strcmp(elemname, name) == 0 )
		{
			switch( action )
			{
				case CAPTURE:
					if( snd_mixer_selem_has_capture_volume( elem ) )
					{
						snd_mixer_selem_get_capture_volume_range( elem, &sndMixerPMin, &sndMixerPMax );
						newvol = ( ( ( sndMixerPMax - sndMixerPMin ) * level ) / 100 ) + sndMixerPMin;
						snd_mixer_selem_set_capture_volume_all( elem, newvol );
						ms_message("Successfully set capture level for %s. \n",elemname);
						return;
					}
					break;
					
				case PLAYBACK:
					if( snd_mixer_selem_has_playback_volume( elem ) )
					{
						snd_mixer_selem_get_playback_volume_range( elem, &sndMixerPMin, &sndMixerPMax );
						newvol = ( ( ( sndMixerPMax - sndMixerPMin ) * level ) / 100 ) + sndMixerPMin;
						snd_mixer_selem_set_playback_volume_all( elem, newvol );
						//ms_message("Successfully set playback level for %s.",elemname);
						return;
					}
					break;
					
				case CAPTURE_SWITCH:
					if( snd_mixer_selem_has_capture_switch(elem) )
					{
						snd_mixer_selem_set_capture_switch_all( elem, level );
						//ms_message("Successfully set capture switch for %s.",elemname);
					}
					break;
					
				case PLAYBACK_SWITCH:
					if( snd_mixer_selem_has_playback_switch(elem) )
					{
						snd_mixer_selem_set_playback_switch_all( elem, level );
						//ms_message("Successfully set capture switch for %s.",elemname);
					}
					break;

			}
		}
		elem = snd_mixer_elem_next( elem );
	}

	return ;
}

void alsa_card_set_level(  int level, CardMixerElem e )
{	
	snd_mixer_t *mixer = NULL;
	
	switch( e )//Master plughw:1 Capture PCM default
	{
		case CARD_CAPTURE:
			if( ( mixer = alsa_mixer_open( "hw:1" ) ) == NULL ) 
				return ;
			set_mixer_element( mixer, "PCM", level, CAPTURE );		
			break;

		case CARD_PLAYBACK:
			if( ( mixer = alsa_mixer_open( "hw:0" ) ) == NULL ) 
				return ;			
			set_mixer_element( mixer, "PCM", level, PLAYBACK );
			break;			

		default:
			ms_warning("alsa_card_set_level: unsupported command.");		
	}

	alsa_mixer_close( mixer );
}

int alsa_card_get_level( CardMixerElem e )
{
	snd_mixer_t *mixer;
	char* mixdev = "default";
	int value 		= -1;
	
	if( ( mixer = alsa_mixer_open( mixdev ) ) == NULL ) 
		return 0;

	switch( e )
	{
		case CARD_CAPTURE:
			value = get_mixer_element( mixer, "Capture", CAPTURE );
			break;

		case CARD_PLAYBACK:
			value = get_mixer_element( mixer, "PCM", PLAYBACK );
		break;			

		default:
			ms_warning("alsa_card_set_level: unsupported command.");		
	}		
	
	alsa_mixer_close( mixer );
	
	return value;
}

struct AlsaCtrl* alsa_write_init( struct AlsaCtrl* obj,  int rate, int channels )
{
	if( obj == NULL )
	{
		if ( (obj = (struct AlsaCtrl*)malloc(sizeof(*obj))) == NULL )
			return NULL;
	}

	memset( obj, 0, sizeof(*obj) );

	obj->pcmdev		= "default:0";
	obj->handle		= NULL;
	obj->rate		= rate;
	obj->nchannels	= channels;

	return obj;
}

struct AlsaCtrl* alsa_ring_init( struct AlsaCtrl* obj,  int rate, int channels )
{
	if( obj == NULL )
	{
		if ( (obj = (struct AlsaCtrl*)malloc(sizeof(*obj))) == NULL )
			return NULL;
	}

	memset( obj, 0, sizeof(*obj) );

	obj->pcmdev		= "default";
	obj->handle		= NULL;
	obj->rate		= rate;
	obj->nchannels	= channels;

	return obj;
}

void alsa_write_uninit( struct AlsaCtrl* obj )
{
	if( obj->handle != NULL ) 
		snd_pcm_close( obj->handle );
	
	free( obj );
}


static int alsa_set_params( snd_pcm_t* pcm_handle, int rw, int bits, int stereo, int rate )
{
	snd_pcm_hw_params_t *hwparams	= NULL;
	snd_pcm_sw_params_t *swparams	= NULL;
	int dir;
	uint exact_uvalue;
	unsigned long exact_ulvalue;
	int channels;
	int periods	= ALSA_PERIODS;
	int periodsize	= ALSA_PERIOD_SIZE;
	snd_pcm_uframes_t buffersize;
	int err;
	int format;
	
	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca( &hwparams );
	
	/* Init hwparams with full configuration space */
	if( snd_pcm_hw_params_any( pcm_handle, hwparams) < 0 )  
	{
		ms_warning("alsa_set_params: Cannot configure this PCM device.\n");
		return -1;
	}

	if( snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0 ) 
	{
		ms_warning("alsa_set_params: Error setting access.\n");
		return -1;
	}
	
	/* Set sample format */
	format = SND_PCM_FORMAT_S16_LE;
	if( snd_pcm_hw_params_set_format(pcm_handle, hwparams, format) < 0 ) 
	{
		ms_warning("alsa_set_params: Error setting format.\n");
		return -1;
	}
	
	/* Set number of channels */
	if( stereo ) 
		channels = 2;
	else 
		channels = 1;
	
	if( snd_pcm_hw_params_set_channels( pcm_handle, hwparams, channels ) < 0 ) 
	{
		ms_warning("alsa_set_params: Error setting channels.\n");
		return -1;
	}
	
	/* Set sample rate. If the exact rate is not supported */
	/* by the hardware, use nearest possible rate.         */ 
	exact_uvalue = rate;
	dir = 0;
	if( (err = snd_pcm_hw_params_set_rate_near( pcm_handle, hwparams, &exact_uvalue, &dir )) < 0 )
	{
		ms_warning("alsa_set_params: Error setting rate to %i:%s\n",rate,snd_strerror(err));
		return -1;
	}
	
	if( dir != 0 ) 
	{
		ms_warning("alsa_set_params: The rate %d Hz is not supported by your hardware. "
		"==> Using %d Hz instead.\n", rate, exact_uvalue);
	}

	/* choose greater period size when rate is high */
	periodsize = periodsize * ( rate / 8000 );	
	
	/* Set buffer size (in frames). The resulting latency is given by */
	/* latency = periodsize * periods / (rate * bytes_per_frame)     */
	/* set period size */
	exact_ulvalue = periodsize;
	dir = 0;
	if( snd_pcm_hw_params_set_period_size_near(pcm_handle, hwparams, &exact_ulvalue, &dir) < 0 ) 
	{
		ms_warning("alsa_set_params: Error setting period size.\n");
		return -1;
	}
	
	if( dir != 0 ) 
	{
		ms_warning("alsa_set_params: The period size %d is not supported by your hardware. "
		"==> Using %d instead.\n", periodsize, (int)exact_ulvalue);
	}

	ms_warning("alsa_set_params: periodsize:%d Using %d\n", periodsize, (int)exact_ulvalue);
	periodsize = exact_ulvalue;
	/* Set number of periods. Periods used to be called fragments. */ 
	exact_uvalue = periods;
	dir = 0;
	if( snd_pcm_hw_params_set_periods_near(pcm_handle, hwparams, &exact_uvalue, &dir) < 0 ) 
	{
		ms_warning("alsa_set_params: Error setting periods.\n");
		return -1;
	}
	
	ms_warning("alsa_set_params: period:%d Using %d\n", periods, exact_uvalue);
	if( dir != 0 ) 
	{
		ms_warning("alsa_set_params: The number of periods %d is not supported by your hardware. "
		"==> Using %d instead.\n", periods, exact_uvalue);
	}

	periods = exact_ulvalue;

	if( snd_pcm_hw_params_get_buffer_size( hwparams, &buffersize ) < 0 )
	{
		buffersize=0;
		ms_warning("alsa_set_params: could not obtain hw buffer size.\n");
	}

	/* Apply HW parameter settings to */
	/* PCM device and prepare device  */
	if( (err = snd_pcm_hw_params( pcm_handle, hwparams )) < 0 ) 
	{
		ms_warning("alsa_set_params: Error setting HW params:%s\n",snd_strerror(err));
		return -1;
	}

	/*prepare sw params */
	if( rw )
	{
		snd_pcm_sw_params_alloca( &swparams );
		snd_pcm_sw_params_current( pcm_handle, swparams );
		
		//ms_message("periodsize=%i, buffersize=%i",(int) periodsize, (int)buffersize);
		if( ( err = snd_pcm_sw_params_set_start_threshold( pcm_handle, swparams, periodsize * 2 ) ) < 0 )
		{
			ms_warning( "alsa_set_params: Error setting start threshold:%s\n", snd_strerror(err) );
		}
		
		if( ( err = snd_pcm_sw_params_set_stop_threshold(pcm_handle, swparams, periodsize * periods ) ) < 0)
		{
			ms_warning("alsa_set_params: Error setting stop threshold:%s\n", snd_strerror(err) );
		}
		
		if( (err = snd_pcm_sw_params( pcm_handle, swparams ) ) < 0)
		{
			ms_warning( "alsa_set_params: Error setting SW params:%s\n", snd_strerror(err) );
			return -1;
		}
	}

	
	return 0;	
}


static void alsa_resume( snd_pcm_t *handle )
{
	int err;
	snd_pcm_status_t* status = NULL;

	snd_pcm_status_alloca( &status );
	
	if( (err = snd_pcm_status( handle, status ) ) != 0 )
	{
		ms_warning("snd_pcm_status() failed: %s\n",snd_strerror(err));
		return;
	}

	if( snd_pcm_status_get_state(status) == SND_PCM_STATE_SUSPENDED )
	{
		ms_warning("Maybe suspended, trying resume\n");
		if( (err = snd_pcm_resume(handle)) != 0 )
		{
			if( err != EWOULDBLOCK ) 
				ms_warning("snd_pcm_resume() failed: %s\n",snd_strerror(err));
		}
	}
}


snd_pcm_t* alsa_open_w( const char* pcmdev, int bits, int stereo, int rate )
{
	snd_pcm_t *pcm_handle;

	ms_message("alsa_open_w: opening %s at %iHz, bits=%i, stereo=%i\n",pcmdev,rate,bits,stereo);


	if( snd_pcm_open( &pcm_handle, pcmdev, SND_PCM_STREAM_PLAYBACK, 0 ) < 0 )
	{
		ms_warning( "alsa_open_w: Error opening PCM device %s\n", pcmdev );
		return NULL;
	}

	alsa_resume( pcm_handle );

	{
		struct timeval tv1;
		struct timeval tv2;
		struct timezone tz;
		int diff = 0;
		int err = gettimeofday( &tv1, &tz );

		while( 1 ) 
		{ 
			if( !(alsa_set_params( pcm_handle, 1, bits, stereo, rate ) < 0) )
			{
				ms_message("alsa_open_w: Audio params set\n");
				break;
			}

			if( !gettimeofday(&tv2, &tz) && !err )
				diff = ((tv2.tv_sec - tv1.tv_sec) * 1000000) + (tv2.tv_usec - tv1.tv_usec);
			else 
				diff = -1;
			
			if( (diff < 0) || (diff > 3000000) )  /* 3 secondes */
			{
				ms_error("alsa_open_w: Error setting params for more than 3 seconds\n");
				snd_pcm_close(pcm_handle);
				return NULL;
			}
			ms_warning("alsa_open_w: Error setting params (for %d micros)\n", diff);
			usleep(200000);
		}
	}

#ifdef EPIPE_BUGFIX
	alsa_fill_w( pcmdev );
#endif

	return pcm_handle;
}

int alsa_write( snd_pcm_t *handle, unsigned char *buf, int nsamples )
{
	int err;
	
	if( (err = snd_pcm_writei(handle, buf, nsamples)) < 0 )
	{
		if( err == -EPIPE )
		{
			snd_pcm_prepare( handle );
#ifdef EPIPE_BUGFIX
			alsa_fill_w( handle );
#endif
			if( (err = snd_pcm_writei( handle, buf, nsamples )) < 0 ) 
				ms_warning("alsa_card_write: Error writing sound buffer (nsamples=%i):%s\n",nsamples,snd_strerror(err));
		}
		else if( err == -ESTRPIPE )
		{
			alsa_resume( handle );
		}
		else if( err != -EWOULDBLOCK )
		{
			ms_warning("alsa_card_write: snd_pcm_writei() failed:%s.\n",snd_strerror(err));
		}
	}
	else if( err != nsamples ) 
	{
		ms_debug("Only %i samples written instead of %i\n",err,nsamples);
	}
	
	return err;
}


#if 0
void alsa_write_process( MSFilter *obj, MSFilter *f )
{

	AlsaWriteData *ad 	= (AlsaWriteData*)obj->data;
	//PlayerData *d		= (PlayerData*)f->data;
	int size;
	int samples;
	int err;
	//snd_pcm_uframes_t frames;
	unsigned char* buffer;

	if( ad->handle == NULL && ad->pcmdev != NULL )
	{
		ad->handle = alsa_open_w( ad->pcmdev, 24, ad->nchannels, ad->rate );
#ifdef EPIPE_BUGFIX
		alsa_fill_w( ad->pcmdev );
#endif
	}

	alsa_card_set_level( obj, player_level );
	
	if( ad->handle == NULL ) 
		return;
	
	size = 256;

	buffer = (unsigned char *) malloc(size);

			
	while( 1 )
	{
		if( player_state != MSPlayerPlaying )
			break;
		
		err = read( d->fd, buffer, size );

		if( err == 0 )
			break;
		
		samples = size / ( 2 * ad->nchannels );
		err = alsa_write( ad->handle, buffer, samples );
 
		if( err > 0)
			;
		else
			break;
	}

	free( buffer );
	alsa_write_uninit( obj );

	player_state = MSPlayerClosed;
	
}
#endif






struct AlsaCtrl* alsa_read_init( struct AlsaCtrl* obj,  int rate, int channels )
{
	if( obj == NULL )
	{
		if ((obj = (struct AlsaCtrl*)malloc(sizeof(*obj))) == NULL)
			return NULL;
	}

	memset( obj, 0, sizeof(*obj) );

	obj->pcmdev		= "default:1";
	obj->handle		= NULL;
	obj->rate		= rate;
	obj->nchannels	= channels;

	return obj;
}

//#define THREADED_VERSION
snd_pcm_t* alsa_open_r( const char* pcmdev, int bits, int stereo, int rate )
{
	snd_pcm_t *pcm_handle;
	int err;	

	ms_message( "alsa_open_r: opening %s at %iHz, bits=%i, stereo=%i\n", pcmdev, rate, bits, stereo );
	
#if 1	
	if( snd_pcm_open( &pcm_handle, pcmdev, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK ) < 0 )
	{
		ms_warning( "alsa_open_r: Error opening PCM device %s\n", pcmdev );
		return NULL;
	}
#else
	/* want blocking mode for threaded version */
	if( snd_pcm_open( &pcm_handle, pcmdev, SND_PCM_STREAM_CAPTURE, 0 ) < 0 )
	{
		ms_warning( "alsa_open_r: Error opening PCM device %s\n", pcmdev );
		return NULL;
	}
#endif

	{
		struct timeval tv1;
		struct timeval tv2;
		struct timezone tz;
		
		int diff = 0;
		err = gettimeofday( &tv1, &tz );
		
		while( 1 ) 
		{ 
			if( !(alsa_set_params( pcm_handle, 0, bits, stereo, rate)<0) )
			{
				ms_message("alsa_open_r: Audio params set");
				break;
			}
			
			if( !gettimeofday(&tv2, &tz) && !err ) 
				diff = ((tv2.tv_sec - tv1.tv_sec) * 1000000) + (tv2.tv_usec - tv1.tv_usec);
			else 
				diff = -1;
			
			if ((diff < 0) || (diff > 3000000)) /* 3 secondes */
			{ 
				ms_error("alsa_open_r: Error setting params for more than 3 seconds");
				snd_pcm_close( pcm_handle );
				return NULL;
			}
			ms_warning("alsa_open_r: Error setting params (for %d micros)", diff);
			usleep(200000);
		}
	}

	err = snd_pcm_start( pcm_handle );
	if( err < 0 )
	{
		ms_warning("snd_pcm_start() failed: %s", snd_strerror(err));
	}

	return pcm_handle;
}

int alsa_can_read( snd_pcm_t* dev )
{
	snd_pcm_sframes_t avail;
	int err;

	alsa_resume( dev );
	avail = snd_pcm_avail_update( dev );

	//printf(" alsa_can_read	avail %d\n",avail);

	/* A buggy driver does not return an error while being in Xrun */
	if( avail >= 0 && snd_pcm_state(dev) == SND_PCM_STATE_XRUN ) 
		avail=-EPIPE;
	
	if( avail < 0 ) 
	{
		ms_error( "snd_pcm_avail_update: %s\n", snd_strerror(avail) );	// most probably -EPIPE
		/* overrun occured, snd_pcm_state() would return SND_PCM_STATE_XRUN
		 FIXME: handle other error conditions*/
		ms_error( "*** alsa_can_read fixup, trying to recover\n" );
		
		snd_pcm_drain( dev ); /* Ignore possible error, at least -EAGAIN.*/
		err = snd_pcm_recover( dev, avail, 0 );
		
		if( err )
		{ 
			ms_error( "snd_pcm_recover() failed with err %d: %s\n", err, snd_strerror(err) );
			return -1;
		}
		
		err = snd_pcm_start( dev );
		if( err )
		{ 
			ms_error( "snd_pcm_start() failed with err %d: %s\n", err, snd_strerror(err) ); 
			return -1; 
		}
		
		ms_message( "Recovery done\n" );
	}
	
	return avail;
}


int alsa_read( snd_pcm_t* handle, unsigned char* buf, int nsamples )
{
	int err;
	
	err = snd_pcm_readi( handle, buf, nsamples );
	if( err < 0 )
	{
		ms_warning( "alsa_read: snd_pcm_readi() returned %i \n", err );
		
		if( err == -EPIPE )
		{
			snd_pcm_prepare( handle );
			err = snd_pcm_readi( handle, buf, nsamples );			
			if( err < 0 ) 
				ms_warning("alsa_read: snd_pcm_readi() failed:%s.\n",snd_strerror(err));
		}
		else if( err == -ESTRPIPE )
		{
			alsa_resume( handle );
		}
		else if( err != -EWOULDBLOCK )
		{
			ms_warning("alsa_read: snd_pcm_readi() failed:%s.\n",snd_strerror(err));
		}
	}
	else if( err == 0 )
	{
		ms_warning("alsa_read: snd_pcm_readi() returned 0\n");
	}
	
	return err;
}

#if 0
void alsa_read_preprocess(MSFilter *obj)
{
#ifdef THREADED_VERSION
	AlsaReadData *ad=(AlsaReadData*)obj->data;
	alsa_start_r(ad);
#endif
}

void alsa_read_postprocess(MSFilter *obj)
{
	AlsaReadData *ad=(AlsaReadData*)obj->data;
#ifdef THREADED_VERSION
	alsa_stop_r(ad);
#endif
	ms_ticker_set_time_func(obj->ticker,NULL,NULL);
	if (ad->handle!=NULL) snd_pcm_close(ad->handle);
	ad->handle=NULL;
}
#endif

void alsa_read_uninit( struct AlsaCtrl* obj )
{
	if( obj->handle != NULL ) 
		snd_pcm_close( obj->handle );

	free( obj );
}


#if 0

void alsa_read_process( MSFilter *obj, MSFilter *f )
{
	AlsaReadData *ad	=	(AlsaReadData*)obj->data;
	//PlayerData *d		= 	(PlayerData*)f->data;
	
	int samples 		= 	(128*ad->rate)/8000;	
	unsigned char* buffer		=	NULL;
	int err;

	
	if( ad->handle == NULL && ad->pcmdev != NULL )
	{
		ad->handle = alsa_open_r( ad->pcmdev, 16, ad->nchannels == 2, ad->rate );
		if( ad->handle )
		{
			ad->read_samples = 0;
		}
	}
	
	if( ad->handle == NULL ) 
		return;

	int size = samples * 2 * ad->nchannels; 	

	buffer = (unsigned char *)malloc(size);
		
	while( alsa_can_read( ad->handle ) >= samples )
	{			
		if( ( err = alsa_read( ad->handle, buffer, samples ) ) <= 0 ) 
		{
			ms_warning( "Fail to read samples\n" );
			free( buffer );
			return;
		}		
	}
}





void testPlayer( void )
{
	int err;
	unsigned char* buffer;	
	snd_mixer_t *mixer;

	uint8_t test_1[1024];
	uint8_t test_2[1024];
	uint16_t test_3=0;
	uint16_t test_4=0;
	g722_encode_state_t* enc_state;
	g722_decode_state_t* dec_state;

	
	AlsaWriteData *ad	= 	ms_new0( AlsaWriteData, 1 );

	ad->pcmdev_r	= 	"default:1";
	ad->pcmdev_w	= 	"default";
	ad->mixdev		= 	"default";
	ad->handle_r		= 	NULL;
	ad->handle_w	= 	NULL;
	ad->rate			= 	8000;
	ad->nchannels	= 	1;

	int samples 		=	(128*ad->rate)/8000;	
	int size 			= 	samples * 2 * ad->nchannels;

	buffer 			= 	(unsigned char *)malloc(size);


	enc_state = g722_encode_init(NULL, 64000, 0);
	dec_state = g722_decode_init(NULL, 64000, 0);	

	//if( ad->handle == NULL && ad->pcmdev != NULL )
	{
		ad->handle_w = alsa_open_w( ad->pcmdev_w, 16, ad->nchannels == 2, ad->rate );
		ad->handle_r  = alsa_open_r( ad->pcmdev_r, 16, ad->nchannels == 2, ad->rate );
	}

	
	if( ad->handle_w == NULL || ad->handle_r == NULL ) 
		return;
	
	size = 256;

	buffer = (unsigned char *) malloc(size);
#if 0			
	if( (mixer = alsa_mixer_open(ad->mixdev)) == NULL ) 
		return ;

	set_mixer_element( mixer, "PCM", 50, PLAYBACK );
	set_mixer_element( mixer, "PCM", 50, CAPTURE );
	alsa_mixer_close( mixer );
#endif	
	while( 1 )
	{
	//printf("aaa\n");
		while( alsa_can_read( ad->handle_r ) >= samples  || 1)
		{		
			//printf( "222222222222222 %d\n", samples );
			if( ( err = alsa_read( ad->handle_r, buffer, samples ) ) <= 0 ) 
			{
				//printf( "Fail to read samples\n" );
				//free( buffer );
				//return;
			}		

			scale_down((int16_t *)buffer, samples/2);
			test_3=g722_encode(enc_state, test_1, (int16_t *)buffer, samples/2); 	
			
			test_4=g722_decode(dec_state,  (int16_t *)test_2, test_1, test_3);
			printf( "222222222222222 %d    %d\n", test_4,samples );
			scale_up((int16_t *)buffer,test_4);
			
			err = alsa_write( ad->handle_w, buffer, samples );
		}	

	}

	free( buffer );
	
	if( ad->handle_r != NULL ) 
		snd_pcm_close( ad->handle_r );

	if( ad->handle_w != NULL ) 
		snd_pcm_close( ad->handle_w );
	
	free( ad );

}
#endif

