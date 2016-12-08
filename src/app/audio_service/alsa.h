
#ifndef _alsa_h
#define _alsa_h



 #include "../../os/RTOS.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <alsa/asoundlib.h>



#define ms_message	printf
#define ms_warning	printf
#define ms_debug		printf
#define ms_error		printf


struct _AlsaData
{
	char* pcmdev;
	char* mixdev;
};
typedef struct _AlsaData AlsaData;

enum CardMixerElem
{
	CARD_MASTER,
	CARD_PLAYBACK,
	CARD_CAPTURE,
	CARD_RING,	
};
typedef enum CardMixerElem CardMixerElem;


struct AlsaCtrl
{
	char* pcmdev;
	char* pcmdev_r;
	char* pcmdev_w;	
	char* mixdev;
	snd_pcm_t* handle;
	snd_pcm_t* handle_r;
	snd_pcm_t* handle_w;
	int rate;
	int nchannels;
	uint64_t read_samples;
};
typedef struct AlsaCtrl AlsaCtrl_t;

void scale_down( int16_t* samples, int count );
void scale_up( int16_t* samples, int count );

void alsa_card_set_level(  int level, CardMixerElem e );
int alsa_card_get_level( CardMixerElem e );

struct AlsaCtrl* alsa_ring_init( struct AlsaCtrl* obj,  int rate, int channels );
struct AlsaCtrl* alsa_write_init( struct AlsaCtrl* obj, int rate, int channels );
struct AlsaCtrl* alsa_read_init( struct AlsaCtrl* obj, int rate, int channels );

snd_pcm_t* alsa_open_r( const char* pcmdev, int bits, int stereo, int rate );
int alsa_can_read( snd_pcm_t* dev );
int alsa_read( snd_pcm_t* handle, unsigned char* buf, int nsamples );

snd_pcm_t* alsa_open_w( const char* pcmdev, int bits, int stereo, int rate );
int alsa_write( snd_pcm_t *handle, unsigned char *buf, int nsamples );


void alsa_write_uninit( struct AlsaCtrl* obj );
void alsa_read_uninit( struct AlsaCtrl* obj );

#endif

