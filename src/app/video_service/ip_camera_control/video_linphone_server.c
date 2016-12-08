

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <error.h>

#include "../linphone_linker.h"
#include "../linphone_interface.h"
#include "../ip_video_control/video_linphone_client.h"
#include "ip_camera_control.h"
#include "video_linphone_server.h"

extern video_linphone_linker	one_linphone_linker;

FILE* linphone_server_fd = NULL;

// linphonec server类型的开启或是关闭
int onoff_local_linphone_server( int is_on, int auto_talk )
{
	int rc;
	if( is_on )
	{
#ifdef LINPHONE_SERVER_RUN_ENABLE	
		char	cmd[256] = {'\0'};
		if( auto_talk )
		{
			snprintf( cmd, 256, "%s", "/mnt/videodoor/videodoor -C -a" );
		}
		else
		{
			snprintf( cmd, 256, "%s", "/mnt/videodoor/videodoor -C" );
		}
		
		if( linphone_server_fd != NULL )
		{
			// 1 - 发送terminate命令
			API_linphonec_Close();
			// 2 - 发送quit命令到linphonec，关闭linphonc进程
			API_linphonec_quit();
			usleep(100*1000);
			// 3、linphonec进程关闭
			rc = pclose(linphone_server_fd); 
			if( rc == -1 )
			{
				vd_printf("pclose fail!\n"); 	
			}
			else
			{
				vd_printf("sub pid close status = %d,cmd return vaule = %d!\n",rc, WEXITSTATUS(rc));				
			}
			linphone_server_fd = NULL;			
		}
		
		if( (linphone_server_fd = popen( cmd, "r")) == NULL )
		{
			vd_printf("linphone server popen fail!\n"); 	
		}
#endif		
		// 2 - 延时一会
		usleep(1200*1000);	
		vd_printf("linphone server turn on ok!\n");	
	}
	else
	{
#ifdef LINPHONE_SERVER_RUN_ENABLE		
		if( linphone_server_fd != NULL )
		{
			// 1 - 发送terminate命令
			API_linphonec_Close();
			// 2 - 发送quit命令到linphonec，关闭linphonc进程
			API_linphonec_quit();
			usleep(100*1000);			
			// 3、linphonec进程关闭
			rc = pclose(linphone_server_fd); 
			if( rc == -1 )
			{
				vd_printf("pclose fail!\n"); 	
			}
			else
			{
				vd_printf("sub pid close status = %d,cmd return vaule = %d!\n",rc, WEXITSTATUS(rc));				
			}
			linphone_server_fd = NULL;			
		}
#endif		
		// 2 - 延时一会
		usleep(1200*1000);			
		vd_printf("linphone server turn off ok!\n");
	}
	return 0;
}

int api_linphone_server_to_call( int client_ip, int auto_talk )
{
	vd_printf("api_linphone_server_to_call addr = 0x%08x\n",client_ip);

	// 已在服务器端则无需再启动linphonec
	if( one_linphone_linker.state != VD_LINPHONE_SERVER )
	{
		if( one_linphone_linker.state == VD_LINPHONE_CLIENT )
		{
			// 关闭客户端
			onoff_local_linphone_client(0,auto_talk);		
			one_linphone_linker.state = VD_LINPHONE_IDLE;
		}
	}

	// 通过linphone_linker启动远程linphone客户端
	if( api_one_linphone_linker_req( client_ip, 0, auto_talk) == VD_LINPHONE_MSG_REQ_OK )
	{
		// 空闲状态，则启动为服务器端
		if( one_linphone_linker.state == VD_LINPHONE_IDLE )
		{	
			// 启动服务器端
			onoff_local_linphone_server(1,auto_talk);
			one_linphone_linker.state = VD_LINPHONE_SERVER;
		}	
#ifdef LINPHONE_SERVER_RUN_ENABLE			
		// 呼叫remote client
		API_linphonec_Invite(client_ip);
#endif
	}	
	
	return 0;	
}

int api_linphone_server_becalled( int client_ip, int auto_talk )
{
	static int becalled_num = 0;

	vd_printf("api_linphone_server_becalled addr = 0x%08x, num = %d\n",client_ip,++becalled_num);

	// 客户端呼叫请求检查
	if( api_camera_server_linphone_apply_on(client_ip) < 0 )
		return -1;	

	// 已在服务器端则无需再启动linphonec
	if( one_linphone_linker.state != VD_LINPHONE_SERVER )
	{
		// 空闲状态，则启动为服务器端
		if( one_linphone_linker.state == VD_LINPHONE_IDLE )
		{	
			// 启动服务器端
			onoff_local_linphone_server(1,auto_talk);
			one_linphone_linker.state = VD_LINPHONE_SERVER;
		}
		else if( one_linphone_linker.state == VD_LINPHONE_CLIENT )
		{
			// 关闭客户端
			onoff_local_linphone_client(0,auto_talk);		
			// 启动服务器端
			onoff_local_linphone_server(1,auto_talk);
			one_linphone_linker.state = VD_LINPHONE_SERVER;
		}
	}
	
	return 0;
}

int api_linphone_server_to_close( int client_ip )
{
	vd_printf("api_close_linphone_server addr = 0x%08x\n",client_ip);

	if( one_linphone_linker.state == VD_LINPHONE_SERVER )
	{
		onoff_local_linphone_server(0,0);
		// 通过linphone_linker关闭远程linphone客户端
		if( api_one_linphone_unlink_req( client_ip, 0 ) == VD_LINPHONE_MSG_CLOSE_OK )
		{
		}		
	}
	
	one_linphone_linker.state = VD_LINPHONE_IDLE;
	
	return 0;
}

int api_linphone_server_beclosed( int client_ip )
{
	vd_printf("api_close_linphone_server addr = 0x%08x\n",client_ip);

	// 客户端关闭请求检查
	if( api_camera_server_linphone_apply_off(client_ip) < 0 )
		return -1;

	if( one_linphone_linker.state == VD_LINPHONE_SERVER )
	{
		onoff_local_linphone_server(0,0);
	}
	
	one_linphone_linker.state = VD_LINPHONE_IDLE;
	
	vd_printf("linphone server closed!\n"); 	
	
	return 0;
}


