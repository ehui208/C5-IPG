/* DrvVPE.h
 *
 * Copyright (c) 2009 Nuvoton technology corporation
 * All rights reserved.
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __ASM_ARM_W55FA92_DRVVPE_H
#define __ASM_ARM_W55FA92_DRVVPE_H

///#include <linux/config.h>
#include <linux/videodev.h>
#include <asm/io.h>
#include "obj_h264d.h"
/* VPE source format */
typedef enum
{
	VPE_SRC_PLANAR_YONLY  	= 0,	 
	VPE_SRC_PLANAR_YUV420 	= 1,
	VPE_SRC_PLANAR_YUV411 	= 2,
	VPE_SRC_PLANAR_YUV422 	= 3,		
	VPE_SRC_PLANAR_YUV422T	= 5,	
	VPE_SRC_PLANAR_YUV444 	= 9,
	
	VPE_SRC_PACKET_YUV422 	= 12,
	VPE_SRC_PACKET_RGB555 	= 13,
	VPE_SRC_PACKET_RGB565 	= 14,
	VPE_SRC_PACKET_RGB888 	= 15
}E_VPE_SRC_FMT;
/* VPE destination format */
typedef enum 
{
	VPE_DST_PACKET_YUV422	= 0,
	VPE_DST_PACKET_RGB555	= 1,
	VPE_DST_PACKET_RGB565	= 2,
	VPE_DST_PACKET_RGB888	= 3
}E_VPE_DST_FMT;

/* operation */
typedef enum 
{
	VPE_OP_NORMAL	= 0x0,
	VPE_OP_RIGHT		= 0x1,
	VPE_OP_LEFT,
	VPE_OP_UPSIDEDOWN,
	VPE_OP_FLIP,
	VPE_OP_FLOP
	//VPE_DDA_SCALE	// Not support now
}E_VPE_CMD_OP;

/* scale algorithm */
typedef enum
{
	VPE_SCALE_DDA = 0,				/* 3x3 and Bilinear are disabled */
	//VPE_SCALE_3X3 = 1,				/* Only enable 3x3, Not support now. It has to be approached by 2 steps*/
	VPE_SCALE_BILINEAR = 2,			/* Only enable Bilinear */
	//VPE_SCALE_3X3_BILINEAR = 3		/* Both downscale are enabled, Not support now */
}E_VPE_SCALE_MODE;

/* frame mode or on the fly mode */
typedef enum
{
	VPE_HOST_FRAME  =0,	 		// Block base (8x8 or 16x16)
	VPE_HOST_VDEC_LINE =1,		// Line base. for H264, H.263 annex-j. (4x4 block) 
	//VPE_HOST_JPEG =2,			// Not support now
	//VPE_HOST_VDEC_SW =3		// Software, Block base, Not support now
	VPE_HOST_FRAME_TURBO =3		// Block base, Turbo mode
}E_VPE_HOST;

typedef struct vpe_transform
{
	u_int32_t	src_addrPacY;
	u_int32_t	src_addrU;
	u_int32_t	src_addrV;
	u_int32_t	src_format;
	u_int32_t	src_width;
	u_int32_t	src_height;	
	u_int32_t	src_leftoffset;
	u_int32_t	src_rightoffset;	

	u_int32_t	dest_addrPac;
	u_int32_t	dest_format;
	u_int32_t	dest_width;
	u_int32_t	dest_height;
	u_int32_t	dest_leftoffset;
	u_int32_t	dest_rightoffset;

	u_int32_t	algorithm;
	
	E_VPE_CMD_OP rotation;
} vpe_transform_t;

typedef struct _VpeInfo_
{
	int32_t FD;
	int8_t* Name;
	int32_t BuffSwitch;
	
	vpe_transform_t Setting;
}VpeInfo;


#define VPE_INIT							_IO ('k', 130)
#define VPE_IO_SET						_IOW('k', 131, unsigned int)
#define VPE_IO_GET						_IOR('k', 132, vpe_transform_t *)
//#define VPE_IO_GET					_IOR('k', 132, unsigned int)
#define VPE_GET_MMU_MODE				_IOR('k', 133, unsigned int)
#define VPE_SET_FORMAT_TRANSFORM		_IOW('k', 134, vpe_transform_t *)
#define VPE_WAIT_INTERRUPT				_IO ('k', 135)
#define VPE_TRIGGER						_IO ('k', 136)
#define VPE_STOP							_IO ('k', 137)
#define VPE_POLLING_INTERRUPT			_IOR('k', 138, unsigned int)
#define VPE_SET_MMU_MODE				_IOW('k', 139, unsigned int)

int FormatConversion( DECODE_OBJ_t* vpe, DECODE_OBJ_t* vpost, DECODE_OBJ_t* h264 );


int CloseVpe( DECODE_OBJ_t* f );
int InitVpe( DECODE_OBJ_t* f, DECODE_OBJ_t* vpost, int Srcwidth, int Srcheight, int Tarwidth, int Tarheight  );





#endif//__ASM_ARM_W55FA92_DRVVPE_H
