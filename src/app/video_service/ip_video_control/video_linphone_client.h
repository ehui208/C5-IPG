

#ifndef _VIDEO_LINPHONE_CLIENT_H
#define _VIDEO_LINPHONE_CLIENT_H

#include "../linphone_linker.h"

#define LINPHONE_CLIENT_RUN_ENABLE

int onoff_local_linphone_client( int is_on, int auto_talk );

int api_linphone_client_to_call( int server_ip, int auto_talk );
int api_linphone_client_becalled( int server_ip, int auto_talk );
int api_linphone_client_to_close( int server_ip );
int api_linphone_client_beclosed( int server_ip );


#endif

