
#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/socket.h>   
#include <sys/un.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <error.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h> 
#include <netinet/in.h>    
#include <netinet/if_ether.h>   
#include <net/if.h>   
#include <net/if_arp.h>   
#include <arpa/inet.h>     

#include <assert.h>

//#include <execinfo.h>


#include "../os/RTOS.h"
#include "../os/OSQ.h"
#include "../os/OSTIME.h"

#define VDP_PRINTF_ERROR
//#define	VDP_PRINTF_DEBUG

#define VDP_PRINTF_BUSINESS
//#define VDP_PRINTF_LOGFILE

#ifdef	VDP_PRINTF_ERROR
#define	eprintf(fmt,...)	printf("[E]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)
#else
#define	eprintf(fmt,...)
#endif

#ifdef	VDP_PRINTF_DEBUG
#define	dprintf(fmt,...)	printf("[D]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)
#else
#define	dprintf(fmt,...)
#endif

#ifdef	VDP_PRINTF_BUSINESS
#ifdef 	VDP_PRINTF_LOGFILE
#define	bprintf(fmt,...)	printf("[B]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)  //PushBusinessMessage("[B]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)
#else
#define	bprintf(fmt,...)	printf("[B]-["__FILE__"]-[%04d]-"fmt"",__LINE__,##__VA_ARGS__)
#endif
#else
#define	bprintf(fmt,...)
#endif

#define CREATE_IP_ADDR(IP)					( ((int)IP_ADD0<<24)|((int)IP_ADD1<<16)|((int)IP_ADD2<<8)|IP )
#define COMBINING_IP_ADDR(IP1,IP2,IP3,IP4)	( ((int)IP1<<24)|((int)IP2<<16)|((int)IP3<<8)|IP4 )

//czn_20160827_s
#define RID0000_FIRMWIRE					0

#define RID1000_IO_PARAMETER				1000
#define RID1001_TB_ROOM					1001
#define RID1002_TB_GATEWAY				1002
#define RID1003_DS1_BTNMAP				1003
#define RID1004_DS2_BTNMAP				1004
#define RID1005_DS3_BTNMAP				1005
#define RID1006_DS4_BTNMAP				1006
#define RID1007_DS1_NAMELIST				1007
#define RID1008_DS2_NAMELIST				1008
#define RID1009_DS3_NAMELIST				1009
#define RID1010_DS4_NAMELIST				1010
#define RID1011_DSA_NAMELIST				1011
#define RID1012_IM_NAMELIST				1012
#define RID1013_IM_GL_MAP					1013

//czn_20160827_e

//czn_20160708_s

typedef enum
{
	UpgradeIniKey_Unkown = 0,
	UpgradeIniKey_DeviceType,
	UpgradeIniKey_FwVer,
	UpgradeIniKey_FwParser,
	UpgradeIniKey_FwReset,
	UpgradeIniKey_Stm32,
	UpgradeIniKey_End,
}UpgradeIniKey_Type;

typedef struct
{
	char		fwname[40];
	char		update_path[40*2];
}N329fw_Stru;

typedef struct
{
	char			device_type[20];
	char			fw_ver[20];
	int			n329fw_cnt;
	N329fw_Stru	*n329fw_Items;
	int			reset_enble;
	int			have_stm32fw;
	char			stm32fwname[40];
	int			stm32fw_update_enble;
}UpgradeIni_Parser_Stru;

//czn_20160708_e

/*******************************************************************************************
通用队列数据结构
*******************************************************************************************/
typedef void (*msg_process)(void*,int);

typedef unsigned char* p_vdp_common_buffer;

#define COMMON_RESPONSE_BIT		(0x80)

typedef struct Loop_vdp_common_buffer_tag
{
	OS_Q 					embosQ;
	int 					QSize;
	unsigned char*			pQBuf;
	msg_process 			process;					// 数据处理函数
	void*					powner;
} Loop_vdp_common_buffer, *p_Loop_vdp_common_buffer;

int init_vdp_common_queue(p_Loop_vdp_common_buffer pobj, int qsize, msg_process process, void* powner );
int exit_vdp_common_queue( p_Loop_vdp_common_buffer pobj );
int push_vdp_common_queue( p_Loop_vdp_common_buffer pobj, char *data, unsigned char length);
int pop_vdp_common_queue(p_Loop_vdp_common_buffer pobj, p_vdp_common_buffer* pdb, int timeout);
int purge_vdp_common_queue(p_Loop_vdp_common_buffer pobj);

typedef struct
{
	int								task_id;
	char*							task_name;
	p_Loop_vdp_common_buffer		p_msg_buf;
	p_Loop_vdp_common_buffer		p_syc_buf;	
	int 							task_run_flag;		// 独立线程运行标志
	pthread_t						task_pid;			// 独立线程运行id	
} vdp_task_t;

int init_vdp_common_task( vdp_task_t* ptask, int task_id, void* (*pthread)(void*), p_Loop_vdp_common_buffer msg_buf, p_Loop_vdp_common_buffer syc_buf );
int exit_vdp_common_task( vdp_task_t* ptask );

/////////////////////////////////////////////////////////////////////////////////////////////////////////
inline int select_ex(int, fd_set *, fd_set *, fd_set *, struct timeval *);
inline int ioctlex(int, int, void *);
int sem_wait_ex(sem_t *p_sem, int semto);
int sem_wait_ex2(sem_t *p_sem, int semto);
int set_block_io(int fd, int is_block);

int PrintCurrentTime(int num);

// common 
int get_format_time(char *tstr);
int check_ip_repeat( int ip );
int GetLocalIp(void);
int GetLocalIpStr( char* pIPStr );
int GetLocalMacStr( char* pMacStr ); 
int ConvertIpStr2IpInt( const char* ipstr, int* ipint );
int ConvertIpInt2IpStr( int ipint, char* ipstr	);

// socket serial

int create_trs_udp_socket(void);
int create_rcv_udp_socket(char *net_dev_name, unsigned short prot, int block );

int join_multicast_group(char *net_dev_name, int socket_fd, int mcg_addr);
int join_multicast_group_ip(int socket_fd, int mcg_addr, int addr);
int leave_multicast_group(char *net_dev_name, int socket_fd, int mcg_addr);


int send_comm_udp_data( int sock_fd, struct sockaddr_in sock_target_addr, char *data, int length);

void DebugBacktrace(unsigned int sn , siginfo_t  *si , void *ptr);

//lzh_20160704_s
int start_tftp_download( int ip_addr, char* filename );
int get_tftp_file_checksum( int* pfile_len, int* pfile_cks );
//lzh_20160704_e

//czn_20160705_s
int start_updatefile_and_reboot(int ip_addr,char* tarfilename);
int IsHaveUpdate(int *ip_addr);
//czn_20160705_e

// lzh_20160715_s
int create_video_record_filename( char* pfilename );
// lzh_20160715_e

//czn_20160708_s
int Update_Configfile(void);
int Stm32FwUpdate(FILE *pfw);		
int UpgradeIni_Parser(UpgradeIni_Parser_Stru *ini_parser, char* inifilename);
int UpgradeIni_Free(UpgradeIni_Parser_Stru *ini_parser);
void Printf_UpgradeIni_Parser_Stru(UpgradeIni_Parser_Stru *ini_parser);
//czn_20160708_e
//czn_20160827_s
int UpdateResourceTable(void);
int UpdateFwAndResource(void);
//czn_20160827_e

#endif


