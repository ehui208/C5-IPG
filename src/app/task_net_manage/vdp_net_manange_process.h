
#ifndef _vdp_netmanage_process_H
#define _vdp_netmanage_process_H

#define MAC_PARAM				0
#define IP_PARAM					1
#define GW_PARAM				2
#define MASK_PARAM				3

#define TIME_OUT					0
#define GOOD						1
#define GENERAL 					2
#define BAD 						3

#define ROUTE_FILE				"/mnt/nand1-1/route.sh"
#define TEMP_FILE				"/mnt/nand1-1/1.tem"



#define DIP_SET_NET_WROK		1
#define NET_WROK_TEST			2
#define RESET_NET_WROK			3
#define SET_NET_WROK			4
#define DEFAULT_NET_WROK		5

typedef struct 
{
	unsigned char msg_type;
	unsigned char data;
	char macadd[18];
	char ipadd[18];	
	char maskadd[18];
	char gwadd[18];	
} NetWork;

unsigned char SetNetWork( char* mac, char* ip, char* mask, char* gw );
unsigned char ResetNetWork( void );


#if 0
#define API_NetWork_DipSet( data ) 			\
	    API_NetWork_Common( DIP_SET_NET_WROK, data, NULL, NULL, NULL, NULL )

#define API_NetWork_Test( mode ) 			\
	    API_NetWork_Common( NET_WROK_TEST, mode, NULL, NULL, NULL, NULL )

#define API_NetWork_Reset() 				\
	    API_NetWork_Common( RESET_NET_WROK, NULL, NULL, NULL, NULL, NULL )

#define API_NetWork_Set( mac, ip, mask, gw ) 	\
	    API_NetWork_Common( SET_NET_WROK, NULL, mac, ip, mask, gw )

#define API_NetWork_Default() 	\
	    API_NetWork_Common( DEFAULT_NET_WROK, NULL, NULL, NULL, NULL, NULL )
#endif


#endif

