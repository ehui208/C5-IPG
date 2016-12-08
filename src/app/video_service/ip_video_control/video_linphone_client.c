

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <error.h>

#include "../linphone_linker.h"
#include "../linphone_interface.h"
#include "../ip_camera_control/video_linphone_server.h"
#include "ip_video_control.h"
#include "video_linphone_client.h"

extern video_linphone_linker	one_linphone_linker;

FILE*	linphone_client_fd = NULL;
// linphonec client类型的开启或是关闭
int onoff_local_linphone_client( int is_on, int auto_talk )
{
	int rc;
	if( is_on )
	{
#ifdef LINPHONE_CLIENT_RUN_ENABLE	
		// 1	- 通过popen调用shell命令启动linphone "/mnt/videodoor/videodoor -D -a"
		char	cmd[256] = {'\0'};
		if( auto_talk )
		{
			snprintf( cmd, 256, "%s", "/mnt/videodoor/videodoor -D -a" );
		}
		else
		{
			snprintf( cmd, 256, "%s", "/mnt/videodoor/videodoor -D" );		
		}
		
		if( linphone_client_fd != NULL )
		{
			// 1 - 发送terminate命令
			API_linphonec_Close();
			// 2 - 发送quit命令到linphonec，关闭linphonc进程
			API_linphonec_quit();
			usleep(100*1000);			
			// 3、linphonec进程关闭
			rc = pclose(linphone_client_fd); 
			if( rc == -1 )
			{
				vd_printf("pclose fail!\n"); 	
			}
			else
			{
				vd_printf("sub pid close status = %d,cmd return vaule = %d!\n",rc, WEXITSTATUS(rc));				
			}
			linphone_client_fd = NULL;			
		}
		
		if( (linphone_client_fd = popen( cmd, "r")) == NULL )
		{
			vd_printf("linphone client popen fail!\n"); 	
		}
#endif
		// 2 - 延时一会
		usleep(1500*1000);	
		vd_printf("linphone client turn on ok!\n");		
	}
	else
	{
#ifdef LINPHONE_CLIENT_RUN_ENABLE	
		if( linphone_client_fd != NULL )
		{
			// 1 - 发送terminate命令
			API_linphonec_Close();
			// 2 - 发送quit命令到linphonec，关闭linphonc进程
			API_linphonec_quit();
			usleep(100*1000);			
			// 3、linphonec进程关闭
			rc = pclose(linphone_client_fd); 
			if( rc == -1 )
			{
				vd_printf("pclose fail!\n"); 	
			}
			else
			{
				vd_printf("sub pid close status = %d,cmd return vaule = %d!\n",rc, WEXITSTATUS(rc));				
			}
			linphone_client_fd = NULL;			
		}		
#endif		
		// 2 - 延时一会
		usleep(1500*1000);	
		vd_printf("linphone client turn off ok!\n");
	}
	return 0;
}

int api_linphone_client_to_call( int server_ip, int auto_talk )
{
	static int call_num = 0;
	
	vd_printf("api_linphone_client_to_call addr = 0x%08x, num = %d\n",server_ip,++call_num);

	// 已在客户端则无需再启动linphonec
	if( one_linphone_linker.state != VD_LINPHONE_CLIENT )
	{
		if( one_linphone_linker.state == VD_LINPHONE_SERVER )
		{
			// 关闭服务器端
			onoff_local_linphone_server(0,auto_talk);
			one_linphone_linker.state = VD_LINPHONE_IDLE;
		}
	}
	
	// 通过linphone_linker启动远程linphone服务器端
	if( api_one_linphone_linker_req( server_ip, 1, auto_talk ) == VD_LINPHONE_MSG_REQ_OK )
	{
		// 空闲状态，则启动为客户端
		if( one_linphone_linker.state == VD_LINPHONE_IDLE )
		{
			onoff_local_linphone_client(1,auto_talk);
			one_linphone_linker.state = VD_LINPHONE_CLIENT;
		}	
#ifdef LINPHONE_CLIENT_RUN_ENABLE		
		// 呼叫remote server		
		API_linphonec_Invite(server_ip);
#endif
		return 0;
	}
	else
		return -1;
}

int api_linphone_client_becalled( int server_ip, int auto_talk )
{
	vd_printf("api_linphone_client_becalled addr = 0x%08x\n",server_ip);

	if( api_video_client_linphone_apply_on(server_ip) < 0 )
		return -1;

	// 已在客户端则无需再启动linphonec
	if( one_linphone_linker.state != VD_LINPHONE_CLIENT )
	{
		// 空闲状态，则启动为客户端
		if( one_linphone_linker.state == VD_LINPHONE_IDLE )
		{
			onoff_local_linphone_client(1,auto_talk);
			one_linphone_linker.state = VD_LINPHONE_CLIENT;
		}
		else if( one_linphone_linker.state == VD_LINPHONE_SERVER )
		{
			// 关闭服务器端
			onoff_local_linphone_server(0,auto_talk);
			// 启动客户端
			onoff_local_linphone_client(1,auto_talk);	
			one_linphone_linker.state = VD_LINPHONE_CLIENT;
		}
	}	
	return 0;
}

int api_linphone_client_to_close( int server_ip )
{
	vd_printf("api_close_linphone_client addr = 0x%08x\n",server_ip);

	if( one_linphone_linker.state == VD_LINPHONE_CLIENT )
	{
		onoff_local_linphone_client(0,0);
		// 通过linphone_linker关闭远程linphone服务器端
		if( api_one_linphone_unlink_req( server_ip, 1 ) == VD_LINPHONE_MSG_CLOSE_OK )
		{
		}		
	}
		
	one_linphone_linker.state = VD_LINPHONE_IDLE;
	
	return 0;
}

int api_linphone_client_beclosed( int server_ip )
{
	vd_printf("api_close_linphone_client addr = 0x%08x\n",server_ip);

	if( api_video_client_linphone_apply_off(server_ip) < 0 )
		return -1;

	if( one_linphone_linker.state == VD_LINPHONE_CLIENT )
	{
		onoff_local_linphone_client(0,0);
	}
		
	one_linphone_linker.state = VD_LINPHONE_IDLE;
	
	return 0;
}




