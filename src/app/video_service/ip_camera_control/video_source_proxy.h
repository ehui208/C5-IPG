
#ifndef _VIDEO_SOURCE_PROXY_H
#define _VIDEO_SOURCE_PROXY_H

typedef enum
{
	PROXY_NONE,
	PROXY_UNICAST,
	PROXY_MULTICAST,	
	PROXY_LINPHONE,
	PROXY_ONVIF,
}VIDEO_PROXY_TYPE;

typedef struct
{
	int					proxy_en;		// 服务器有效标志 0/ disable， 1/eanble ， 2/ proxy
	VIDEO_PROXY_TYPE	proxy_type;		// 代理的服务类型
	int 				proxy_ip;		// 代理服务器的ip
}VIDEO_PROXY;

int disable_video_server( void );
int enable_video_server( void );
int enable_video_server_proxy( void );

int is_video_server_invailable( void );
int is_video_server_available( void );
int is_video_server_proxy_enable( void );

/*******************************************************************************************
 * @fn:		set_video_server_proxy
 *
 * @brief:	设置代理服务器
 *
 * @param:  	pproxy_server  - 代理服务器的服务类型指针
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int set_video_server_proxy( const VIDEO_PROXY* pproxy_server );


/*******************************************************************************************
 * @fn:		get_video_server_proxy
 *
 * @brief:	得到代理服务器
 *
 * @param:  	pproxy_server  - 代理服务器的服务类型指针
 *
 * @return: 	0/ok, -1/err
*******************************************************************************************/
int get_video_server_proxy( VIDEO_PROXY* pproxy_server );

/*******************************************************************************************
 * @fn:		get_video_server_proxy_type
 *
 * @brief:	得到代理服务器的IP地址和服务类型
 *
 * @param:  	pproxy_server  - 代理服务器的服务类型指针
 *
 * @return: 	VIDEO_PROXY_TYPE
*******************************************************************************************/
VIDEO_PROXY_TYPE get_video_server_proxy_type( void );

/*******************************************************************************************
 * @fn:		get_video_server_proxy_type
 *
 * @brief:	得到代理服务器的IP地址
 *
 * @param:  	pproxy_server  - 代理服务器的服务类型指针
 *
 * @return: 	jip
*******************************************************************************************/
int get_video_server_proxy_ip( void );

#endif

