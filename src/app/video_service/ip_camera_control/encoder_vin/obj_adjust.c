
#include "obj_V4L.h"
#include "obj_adjust.h"

#include "../../../task_io_server/task_IoServer.h"
#include "../../../task_io_server/vdp_IoServer_Data.h"

static ImagePara_s	imagePara;
extern ENCODE_OBJ_t V4lObj;

#define IMAGE_BRIGHT_DEFAULT		5
#define IMAGE_COLOR_DEFAULT			5
#define IMAGE_CONTRAST_DEFAULT		5

int ReadImagePara( ImagePara_s* para )
{
    unsigned char   logContrastCnt;
	unsigned char 	logBrightCnt;
	unsigned char 	logColorCnt;

	int InitImagePara_OK = 1;

	if( API_io_server_read_local( MSG_ID_DEBUG_SBU, IO_CameraBright, &logBrightCnt ) != 0 )
		InitImagePara_OK = 0;
	if( API_io_server_read_local( MSG_ID_DEBUG_SBU, IO_CameraColor, &logColorCnt ) != 0 )
		InitImagePara_OK = 0;
	if( API_io_server_read_local( MSG_ID_DEBUG_SBU, IO_CameraContrast, &logContrastCnt ) != 0 )
		InitImagePara_OK = 0;

	if( InitImagePara_OK && (para != NULL) )
	{
		para->logBrightCnt 		= logBrightCnt;
		para->logColorCnt		= logColorCnt;
		para->logContrastCnt 	= logContrastCnt;
	}
	return InitImagePara_OK;
}

int WriteImagePara( ImagePara_s* para )
{
	unsigned char	logContrastCnt;
	unsigned char	logBrightCnt;
	unsigned char	logColorCnt;
	int 			InitImagePara_OK = 1;

	if( para != NULL )
	{
		logBrightCnt			= para->logBrightCnt;
		logColorCnt				= para->logColorCnt;
		logContrastCnt			= para->logContrastCnt;
	}
	else
	{
		logBrightCnt			= IMAGE_BRIGHT_DEFAULT;
		logColorCnt				= IMAGE_COLOR_DEFAULT;
		logContrastCnt			= IMAGE_CONTRAST_DEFAULT;		
	}

	if( API_io_server_write_local( MSG_ID_DEBUG_SBU, IO_CameraBright, &logBrightCnt ) != 0 )
		InitImagePara_OK = 0;
	if( API_io_server_write_local( MSG_ID_DEBUG_SBU, IO_CameraColor, &logColorCnt ) != 0 )
		InitImagePara_OK = 0;
	if( API_io_server_write_local( MSG_ID_DEBUG_SBU, IO_CameraContrast, &logContrastCnt ) != 0 )
		InitImagePara_OK = 0;

	return InitImagePara_OK;
	
}

const unsigned char BIRGHTNESS_STEP_TAB[10]=
{
// 	0, 	 1,    2,    3,    4,    5,   6,		7,	 8,	  9
	0x80,0x90,0xa0,0xb0,0xc0,0xdc,0xe0,	0xe8,0xf0,0xf6
};
const unsigned char CONTRAST_STEP_TAB[10]=
{
// 	0, 	 1,    2,    3,    4,    5,   6,		7,	 8,	  9
	0x10,0x20,0x30,0x40,0x50,0x62,0x70,	0x80,0x90,0xA0
};
const unsigned char COLOR_STEP_TAB_U[10]=
{
// 	0, 	 1,    2,    3,    4,    5,   6,		7,	 8,	  9
	0x01,0x10,0x15,0x20,0x25,0x35,0x45,	0x60,0x70,0x80
};
const unsigned char COLOR_STEP_TAB_V[10]=
{
// 	0, 	 1,    2,    3,    4,    5,   6,		7,	 8,	  9
	0x80,0x82,0x84,0x88,0x90,0x95,0xA0,	0xB0,0xE0,0xF0
};

int UpdateImagePara( ImagePara_s* para )
{
	SetSensorBrigness( 	&V4lObj, (int)BIRGHTNESS_STEP_TAB[para->logBrightCnt] );
	SetSensorContrast( 	&V4lObj, (int)CONTRAST_STEP_TAB[para->logContrastCnt] );
	
	int color;
	color = COLOR_STEP_TAB_U[para->logColorCnt];
	color <<= 16;
	color |= COLOR_STEP_TAB_V[para->logColorCnt];
	
	SetSensorColor( 	&V4lObj, color );	
	printf("----------update image para,bri=%d,col=%d,con=%d\n",para->logBrightCnt,para->logColorCnt,para->logContrastCnt);
	return 0;
}

/*******************************************************************************
 * @fn      ScreenAdjust()
 *
 * @brief   adjust screen para, image and vol
 *
 * @param   adjustType - what kinds of adjust
 * @param   val - inc or dec or value
 *
 * @return  none
 ******************************************************************************/
ImagePara_s ScreenAdjust( AdjustType_t adjustType, unsigned char val1, unsigned char val2, unsigned char val3 )
{	
	if( adjustType == ADJ_CONTRAST )
	{
		if( val1 ) // increase
		{
			if( imagePara.logContrastCnt < 9 )
				imagePara.logContrastCnt++;
			else
				imagePara.logContrastCnt = 0;
		}
		else // decrease
		{
			if( imagePara.logContrastCnt )
				imagePara.logContrastCnt--;
			else
				imagePara.logContrastCnt = 9;
		}
	}
	else if( adjustType == ADJ_BRIGHT )
	{
		if( val1 ) // increase
		{
			if( imagePara.logBrightCnt < 9 )
				imagePara.logBrightCnt++;
			else
				imagePara.logBrightCnt = 0;				
		}
		else // decrease
		{
			if( imagePara.logBrightCnt )
				imagePara.logBrightCnt--;
			else
				imagePara.logBrightCnt = 9;
		}
	}
	else if( adjustType == ADJ_COLOR )
	{
		if( val1 ) // increase
		{
			if( imagePara.logColorCnt < 9 )
				imagePara.logColorCnt++;
			else
				imagePara.logColorCnt = 0;
		}
		else // decrease
		{
			if( imagePara.logColorCnt )
				imagePara.logColorCnt--;
			else
				imagePara.logColorCnt = 9;
		}
	}
	else if( adjustType == ADJ_ALL ) 
	{
		imagePara.logContrastCnt	= val1;
		imagePara.logBrightCnt 		= val2;
		imagePara.logColorCnt 		= val3;
	}

	UpdateImagePara(&imagePara);

	WriteImagePara( &imagePara );

	return imagePara;
  
}

/////////////////////////////////////
// API
////////////////////////////////////
ImagePara_s SetImageContrastDec( void )
{
	return ScreenAdjust( ADJ_CONTRAST, 0, 0, 0 );
}

ImagePara_s SetImageContrastInc( void )
{
	return ScreenAdjust( ADJ_CONTRAST, 1, 0, 0 );
}

ImagePara_s SetImageBrightDec( void )
{
	return ScreenAdjust( ADJ_BRIGHT, 0, 0, 0 );
}

ImagePara_s SetImageBrightInc( void )
{
	return ScreenAdjust( ADJ_BRIGHT, 1, 0, 0 );
}

ImagePara_s SetImageColorDec( void )
{
	return ScreenAdjust( ADJ_COLOR, 0, 0, 0 );
}

ImagePara_s SetImageColorInc( void )
{
	return ScreenAdjust( ADJ_COLOR, 1, 0, 0 );
}

ImagePara_s SetImageAllValue( unsigned char logContrast, unsigned char logBright, unsigned char logColor )
{
	return ScreenAdjust( ADJ_ALL, logContrast, logBright, logColor );
}

// 读取IO参数并初始化
void API_LoadImagePara( void )
{
	if( !ReadImagePara(&imagePara) )
	{
		imagePara.logBrightCnt 		= IMAGE_BRIGHT_DEFAULT;		// lzh_20161022  // 0
		imagePara.logColorCnt 		= IMAGE_COLOR_DEFAULT;		// lzh_20161022  // 6
		imagePara.logContrastCnt 	= IMAGE_CONTRAST_DEFAULT;	// lzh_20161022  // 6
	}
	UpdateImagePara(&imagePara);
}


