/* V4L.h
 *
 *
 * Copyright (c)2008 Nuvoton technology corporation
 * http://www.nuvoton.com
 *
 * V4L header file
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#ifndef __V4L_H__
#define __V4L_H__

#include <stdint.h>
#include <sys/types.h>
#include "obj_h264e.h"


int32_t StopV4LCapture( ENCODE_OBJ_t* f );

int32_t StartV4LCapture( ENCODE_OBJ_t* f );


int32_t ReadV4LPicture( ENCODE_OBJ_t* f, u_int32_t *pu32BufPhyAddr );

int32_t TriggerV4LNextFrame( ENCODE_OBJ_t* f );

void CloseV4LDevice( ENCODE_OBJ_t* f );

int InitV4l( ENCODE_OBJ_t* f, int32_t Width, int32_t Height, int32_t format, int init_v4l );

int32_t QueryV4LZoomInfo(  ENCODE_OBJ_t* f, struct v4l2_cropcap *psVideoCropCap, struct v4l2_crop *psVideoCrop );

int32_t SetV4LEncode( ENCODE_OBJ_t* f, int32_t frame, int32_t width, int32_t height, int32_t palette );

int32_t SetSensorBrigness( ENCODE_OBJ_t* f, int32_t ctrl );
int32_t GetSensorBrigness( ENCODE_OBJ_t* f, int32_t* ctrl );

int32_t SetSensorContrast( ENCODE_OBJ_t* f, int32_t ctrl );
int32_t GetSensorContrast( ENCODE_OBJ_t* f, int32_t* ctrl );

int32_t SetSensorColor( ENCODE_OBJ_t* f, int32_t ctrl );
int32_t GetSensorColor( ENCODE_OBJ_t* f, int32_t* ctrl );

#endif

