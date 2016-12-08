
#ifndef _AUDIO_CAPTURE_PROCESS_H_
#define _AUDIO_CAPTURE_PROCESS_H_

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <linux/videodev.h>
#include <errno.h>
#include <linux/fb.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <signal.h>
#include <dirent.h>



void api_mic_volume_set( int vol );
int api_mic_volume_get( void );

int api_audio_capture_handle( unsigned int ip, unsigned short port );
int  api_audio_capture_cancel( void );


typedef enum
{
	 TO_AUDIO_IDLE,
	 TO_AUDIO_START,
	 TO_AUDIO_RUN,
	 TO_AUDIO_STOP,
}
TO_AUDIO_STATE_t;

int audio_to_process_state_init( void );
TO_AUDIO_STATE_t audio_to_process_state_get( void );
int audio_to_process_state_set_idle( void );
int audio_to_process_state_set_start( void );
int audio_to_process_state_set_run( void );
int audio_to_process_state_set_stop( void );


#endif

