#define _GNU_SOURCE		//czn_20160812 use_direct_io
#include <fcntl.h>

#include "../task_survey/task_survey.h"
#include "../task_debug_sbu/task_debug_sbu.h"
#include "../task_io_server/task_IoServer.h"
#include "../task_io_server/vdp_IoServer_Data.h"
#include "../task_io_server/vdp_IoServer_State.h"

#include "vdp_net_manange.h"

extern int sem_timedwait(sem_t *, const struct timespec *);


pthread_t NetWorkManage_id;
sem_t NetWorkSem;
int NetWorkManage;
unsigned char reply;



/*****************************************************************************************************
 * @fn      AssertParam
 *
 * @brief   judging the input address is correct
 *
 * @param   type-> address type 		param-> address data
 *			
 * @return  1--ok 0--fail
 *******************************************************************************************************/
unsigned char AssertParam( char type, char* param )
{
	int n[6];
	unsigned char c[6];
	unsigned char i;

	if( param == NULL )
		return 0;
	
	switch( type )
	{
		case MAC_PARAM:
			// format
			//if( 11== sscanf(param,"%d%c%d%c%d%c%d%c%d%c%d",&n[0],&c[0],&n[1],&c[1],&n[2],&c[2],&n[3],&c[3],&n[4],&c[4],&n[5]) )
			if( 11== sscanf(param,"%02x%c%02x%c%02x%c%02x%c%02x%c%02x",&n[0],&c[0],&n[1],&c[1],&n[2],&c[2],&n[3],&c[3],&n[4],&c[4],&n[5]) )//czn_20160812
			{
				c[5] = ':';

				// comparison address data
				for( i = 0; i < 6; i++ )
				{
					if( (c[i] != ':' ) || (n[i] > 255 || n[i] < 0) )
						return 0;
				}
				return 1;
			} 
			return 0;

		case IP_PARAM:
		case GW_PARAM:
		case MASK_PARAM:
			// format
			if( 7 == sscanf(param,"%d%c%d%c%d%c%d",&n[0],&c[0],&n[1],&c[1],&n[2],&c[2],&n[3]) )
			{
				c[3] = '.';

				// comparison address data
				for( i = 0; i < 4; i++ )
				{
					if( (c[i] != '.' ) || (n[i] > 255 || n[i] < 0) )
						return 0;
				}
				return 1;
			} 
			return 0;
	}

	return 0;
}

/*****************************************************************************************************
 * @fn      SetNetWork
 *
 * @brief   configuration network address
 *
 * @param   mac-> mac address  ip-> ip address  mask-> mask address  gw-> gw address
 *			
 * @return  1--ok 0--fail
 *******************************************************************************************************/
unsigned char SetNetWork( char* mac, char* ip, char* mask, char* gw )
{
	FILE* file = NULL;
	FILE* file_tem = NULL;
	char* ptr  = NULL;
	char line[100] = {'0'};
	unsigned char ret = 0;
	unsigned char len = sizeof( line );


	char* _mac   = "/sbin/ifconfig eth0 hw ether";
	char* _ip       = "/sbin/ifconfig eth0";
	char* _mask = "/sbin/route add -net 224.0.0.0 netmask";
	char* _gw     = "/sbin/route add default gw";
	//char* _down = "/sbin/ifconfig eth0 down";


	if( mac == NULL && ip == NULL && mask == NULL && gw == NULL )
		goto err;

	// read-only file   	// write file
	if( ((file=fopen(ROUTE_FILE,"r"))==NULL)  || ((file_tem=fopen(TEMP_FILE,"w+"))==NULL) )
	{
		eprintf( "SetNetWork error:%s\n",strerror(errno) );
		goto err;
	}

	//read line
	while( fgets(line,len,file) != NULL )
	{	
		// Invalid item ->delete
		if( strchr(line,'/') == NULL )
			continue;					

		// find set item
		if( (ptr = strchr( line, '#' )) == NULL )
		{
			// copy file
			fprintf( file_tem, "%s", line );
			continue;
		}
		
		// find mac item
		if( memcmp(ptr,"#mac",4) == 0 )
		{		
			// check parameter
			if( AssertParam(MAC_PARAM,mac) )
			{	
				//format
				snprintf( line, len, "%s %s %s\n", _mac, mac,"#mac" );
				//API_io_server_InnerWrite(MSG_ID_NetManage,MACAdderss, (unsigned char*)mac );
			}
		}
		// find ip item
		else if( memcmp(ptr,"#ip",3)  == 0 )
		{	
			// check parameter
			if( AssertParam(IP_PARAM,ip) )
			{			
				//format			
				snprintf( line, len, "%s %s %s\n", _ip, ip, "up #ip");
				//API_io_server_InnerWrite(MSG_ID_NetManage,IPAdderss, (unsigned char*)ip);

				dprintf("io server inner write ip_address string = %s ------------\n",ip);
				
			}
		}
		// find mask item
		else if( memcmp(ptr,"#mask",5) == 0 )
		{		
			// check parameter		
			if( AssertParam(MASK_PARAM,mask) )
			{
				//format			
				snprintf( line, len,  "%s %s %s\n", _mask, mask, "dev eth0 #mask" );
				//API_io_server_InnerWrite(MSG_ID_NetManage,Mask, (unsigned char*)mask);				
			}
		}
		// find gateway item
		else if( memcmp(ptr,"#gateway",8) == 0 )
		{		
			// check parameter		
			if( AssertParam(GW_PARAM,gw) )
			{
				//format			
				snprintf( line, len, "%s %s %s\n", _gw, gw, "dev eth0 #gateway" );
				//API_io_server_InnerWrite(MSG_ID_NetManage,GWAdderss, (unsigned char*)gw);				
			}
		}

		// copy file
		fprintf( file_tem, "%s", line );
	}
	fclose( file );		
	file = NULL;

	#if 0
	// open file and empty file
	if( (file = fopen( ROUTE_FILE, "w" )) == NULL )
	{
		dprintf( "SetNetWork error:%s\n",strerror(errno) );
		goto err;
	}

	//move top
	fseek( file_tem, 0, SEEK_SET);
	
	//read line
	while( fgets(line,len,file_tem) != NULL )
	{
		// write file
		fprintf( file, "%s", line );
	}
	#else		//czn_20160812 use_direct_io
	int ffd = open(ROUTE_FILE,O_WRONLY|O_DIRECT);
	char *pdata = NULL;
	int pagelen;
	if(ffd > 0)
	{
		/*bprintf("direct open is ok\n");
		if(file = fdopen(ffd,"w") == NULL)
		{
			dprintf( "SetNetWork error:%s\n",strerror(errno) );
			goto err;
		}
		*/
		//move top
		fseek( file_tem, 0, SEEK_SET);
		pagelen = getpagesize();
		if(posix_memalign((void**)&pdata, pagelen, pagelen) != 0)
		{
			bprintf( "SetNetWork error:%s\n",strerror(errno) );
			close(ffd);
			return 0;
		}
		memset(pdata,0,pagelen);
		while(fread(pdata,1,pagelen,file_tem)>0)
		{
			write(ffd,pdata,pagelen);
			memset(pdata,0,pagelen);
		}
		free(pdata);
		close(ffd);
	}
	else
	{
		bprintf("direct open is fail\n");
	}
	#endif
	ret = 1;

	err:
	{
		if( file != NULL)
			fclose( file );

		if( file_tem != NULL )
			fclose( file_tem );

		file 	     = NULL; 
		file_tem = NULL; 
	}

	
	return ret;
}

/*****************************************************************************************************
 * @fn      ResetNetWork
 *
 * @brief   none
 *
 * @param   none
 *			
 * @return  1--ok 0--fail
 *******************************************************************************************************/
unsigned char ResetNetWork( void )
{
	FILE* fd   = NULL;

	// run route.sh 
	if( (fd = popen( ROUTE_FILE, "r" )) == NULL )
	{
		eprintf( "SetNetWork error:%s\n",strerror(errno) );
		return 0;
	}

	pclose( fd );
	fd = NULL;
	return 1;

}


extern int GetLocalIp(void);
extern int ConvertIpStr2IpInt( const char* ipstr, int* ipint );
	
#if 0
/*****************************************************************************************************
 * @fn      vtk_TaskProcessNetWorkManage
 *
 * @brief   none
 *
 * @param   none
 *			
 * @return  none
 *******************************************************************************************************/
void vtk_TaskProcessNetWorkManage( void )
{	
	NetWork* Msg_NetWork;
	char Msg[sizeof(NetWork)];
	
	while( 1 )
	{		
		dprintf( "vtk_TaskProcessNetWorkManage\n" );
		memset( Msg, 0, sizeof(Msg_NetWork) );
		msqueue_receive( NetWorkManage, Msg, sizeof(NetWork) );

		Msg_NetWork = (NetWork*)Msg;

		reply = 0;
		
		switch( Msg_NetWork->msg_type )
		{
			case DIP_SET_NET_WROK:
				reply = DipSetNetWork( Msg_NetWork->data );
				break;
				
			case NET_WROK_TEST:
				break;

			case RESET_NET_WROK:
				reply = ResetNetWork();
				break;

			case SET_NET_WROK:
				reply = SetNetWork( Msg_NetWork->macadd, Msg_NetWork->ipadd, Msg_NetWork->maskadd, Msg_NetWork->gwadd );
				break;

			case DEFAULT_NET_WROK:
				reply = SetNetWorkDefault();
				break;
		
			default:
				break;
		}

		sem_post( &NetWorkSem );
	}
}

/*****************************************************************************************************
 * @fn      vtk_TaskInit_NetWorkManage
 *
 * @brief   none
 *
 * @param   none
 *			
 * @return  none
 *******************************************************************************************************/
void vtk_TaskInit_NetWorkManage( void )
{		
	NetWorkManage = msqueue_create( "net work manage", MAXMESSAGE, sizeof(NetWork) );

	int ret = pthread_create( &NetWorkManage_id, NULL, (void *) vtk_TaskProcessNetWorkManage, NULL );

	if( ret != 0 || NetWorkManage == -1  )
	{
		dprintf( "Create pthread error!\n" );
		exit( 1 );
	}

	if (sem_init(&NetWorkSem, 0, 0) == -1)
	{
		dprintf( "Create pthread error!\n" );
		exit( 1 );
	}
}

#endif

#if 0
/*****************************************************************************************************
 * @fn      API_NetWork_Common
 *
 * @brief   none
 *
 * @param   none
 *			
 * @return  none
 *******************************************************************************************************/
unsigned char API_NetWork_Common( unsigned char msg_type, unsigned char data, unsigned char* mac, unsigned char* ip, unsigned char* mask, unsigned char* gw )
{
	NetWork Msg_NetWork;
	struct timeval tv;
	struct timespec ts;
	int wait_ms = 2000;


	
	Msg_NetWork.msg_type = msg_type;
	Msg_NetWork.data	  = data;

	if( mac != NULL )
		memcpy( Msg_NetWork.macadd, mac, sizeof(Msg_NetWork.macadd) );
	
	if( ip != NULL )
		memcpy( Msg_NetWork.ipadd, ip, sizeof(Msg_NetWork.ipadd)  );

	if( mask != NULL )
		memcpy( Msg_NetWork.maskadd, mask, sizeof(Msg_NetWork.maskadd)  );

	if( gw != NULL )	
		memcpy( Msg_NetWork.gwadd, gw, sizeof(Msg_NetWork.gwadd)  );

	msqueue_send( NetWorkManage, &Msg_NetWork, sizeof(NetWork) );

	if( gettimeofday(&tv, 0) == -1 )
	{
		dprintf("clock_gettime call failure!msg:%s\n", strerror(errno));
	}
	
	ts.tv_sec = (tv.tv_sec + (wait_ms / 1000 ));
	ts.tv_nsec = (tv.tv_usec  + ((wait_ms % 1000) * 1000)) * 1000;
	ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
	ts.tv_nsec %= 1000 * 1000 * 1000;
	
	sem_timedwait( &NetWorkSem, &ts );

	return reply;
}

#endif

