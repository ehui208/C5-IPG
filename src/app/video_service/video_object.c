

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>      /* basic system data types */
#include <error.h>

#include "video_object.h"

video_object	m_g_video_object;

/*
	video_src_name - 请求视频资源的名称
*/
int set_g_video_object_src_attr( VD_OBJECT_TRANS_TYPE trans_type, int ip, char* src_name, int trans_time, int auto_talk )
{
	m_g_video_object.vd_trans_type		= trans_type;
	m_g_video_object.ip					= ip;
	m_g_video_object.vd_trans_time		= trans_time;
	m_g_video_object.auto_talk			= auto_talk;
	if( src_name != NULL) 
	{
		strcpy( m_g_video_object.vd_src_name,src_name);
	}
}

int get_g_video_object_src_ip( void )
{
	return m_g_video_object.ip;
}

char* get_g_video_object_src_name( void )
{
	return m_g_video_object.vd_src_name;
}

int get_g_video_object_trans_time( void )
{
	return m_g_video_object.vd_trans_time;
}

int get_g_video_object_trans_type( void )
{
	return m_g_video_object.vd_trans_type;
}

int get_g_video_object_auto_talk( void )
{
	return m_g_video_object.auto_talk;
}

