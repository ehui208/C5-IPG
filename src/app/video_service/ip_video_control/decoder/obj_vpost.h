
#ifndef _VPOST_H_
#define _VPOST_H_

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
#include "obj_h264d.h"

typedef struct _VpostInfo_
{
	struct fb_var_screeninfo Var;
	int32_t FBAddr;
	int32_t FD;
	int8_t* Name;
	u_int8_t* VideoBuffer; 
	int32_t BufferSize;
}VpostInfo;


int CloseVpost( DECODE_OBJ_t* f );
int InitVpost( DECODE_OBJ_t* f );


#define VIDEO_TV_SYSTEM					_IOW('v', 51, unsigned int)

#endif

