
#ifndef _OBJ_ADJUST_H_
#define _OBJ_ADJUST_H_


typedef enum
{
    ADJ_CONTRAST,
    ADJ_BRIGHT,
    ADJ_COLOR,
    ADJ_ALL,
}AdjustType_t;

#pragma pack(1)

typedef struct
{
    unsigned char   logContrastCnt;
	unsigned char 	logBrightCnt;
	unsigned char 	logColorCnt;
} ImagePara_s;

#pragma pack()

ImagePara_s SetImageContrastDec( void );
ImagePara_s SetImageContrastInc( void );
ImagePara_s SetImageBrightDec( void );
ImagePara_s SetImageBrightInc( void );
ImagePara_s SetImageColorDec( void );
ImagePara_s SetImageColorInc( void );
ImagePara_s SetImageAllValue( unsigned char logContrast, unsigned char logBright, unsigned char logColor );

void API_LoadImagePara( void );

#endif

