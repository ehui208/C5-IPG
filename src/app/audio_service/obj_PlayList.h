/**
 *******************************************************************************
 * @file    obj_playlist.h
 * @author  LCC
 * @version V1.0.0
 * @date    2014.03.21
 * @brief
 *******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2012 V-Tec</center></h2>
 *******************************************************************************
 */

#ifndef __OBJ_PLAYLIST_H
#define __OBJ_PLAYLIST_H

#ifdef __cplusplus
extern "C"
{
#endif

#define PLAYLIST_OK                          		0
#define PLAYLIST_ERR_UNOPENING_FILE        -3
#define PLAYLIST_ERR_FS                     		-4
#define PLAYLIST_ERR_PARSE_CFG              	-5
#define PLAYLIST_ERR_PARSE_WAVE             	-6


#pragma pack(1)
typedef struct
{
    unsigned char   	ID;
    char 			FileName[40];
    char 			DispName[13];
    unsigned int  	Length;
} PlayList_Cfg_t;
#pragma pack()

/* Define Object Property-----------------------------------------------------*/

/* Define Object Function - Public--------------------------------------------*/
int PlayList_GetFileName(char *pFileName, unsigned char TuneID);
int PlayList_GetDispName(char *pDispName, unsigned char TuneID);
int PlayList_GetUsingLength_ms(unsigned char TuneID, unsigned long *time);
int PlayList_GetNumTunes(unsigned char *pTuneNum);
/* Define Object Function - Private-------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif

/* End Of File ---------------------------------------------------------------*/

