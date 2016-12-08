

#ifndef _VIDEO_LINPHONE_SERVER_H
#define _VIDEO_LINPHONE_SERVER_H

#include "../linphone_linker.h"

#define LINPHONE_SERVER_RUN_ENABLE

int onoff_local_linphone_server( int is_on, int auto_talk );

int api_linphone_server_to_call( int client_ip, int auto_talk );
int api_linphone_server_becalled( int client_ip, int auto_talk );
int api_linphone_server_to_close( int client_ip );
int api_linphone_server_beclosed( int client_ip );

#endif


