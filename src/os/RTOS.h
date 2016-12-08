
#ifndef RTOS_H_INCLUDED     /* Avoid multiple inclusion             */
#define RTOS_H_INCLUDED

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <linux/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h> 

#include <time.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>



/*********************************************************************
*
*       Basic type defines
*
**********************************************************************
*/

#define OS_I8				signed char
#define OS_U8				unsigned char
#define OS_I16			signed short
#define OS_U16			unsigned short
#define OS_I32			long
#define OS_U32			unsigned OS_I32

/* Defines a true integer. This type is guaranteed
   a) to have at least 8 bits,
   b) to compile and execute best on the target CPU
   It will normally be the same as an int, but on most
   8-bit CPUs it will be replaced by a character because
   the CPU can deal with 8 bits more efficient than with
   16 bit integers.
   Use with care !
*/
#define OS_INT			int
#define OS_UINT			unsigned OS_INT
#define OS_TIME			int
#define OS_STAT			OS_U32 //OS_U8
#define OS_PRIO			OS_U32 //OS_U8
#define OS_BOOL			OS_U32//OS_U8

typedef signed   char		INT8;
typedef unsigned char   	UINT8;

typedef signed   short  	INT16;
typedef unsigned short  	UINT16;

typedef signed   int   		INT32;
typedef unsigned int   		UINT32;

typedef signed   char		int8;
typedef unsigned char   	uint8;

typedef signed   short  	int16;
typedef unsigned short  	uint16;

typedef signed   int   		int32;
typedef unsigned int   		uint32;

typedef unsigned char 		BOOL;
#define uint8				UINT8
#define TRUE    			1
#define FALSE   			0

//cao_20160429

#endif /* RTOS_H_INCLUDED */
