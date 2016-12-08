/**
 *******************************************************************************
 * @file    obj_PlayList.c
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

/* Includes ------------------------------------------------------------------*/
//#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include<stdio.h>
#include <stdlib.h>

#include "obj_PlayList.h"
//#include "task_PlayList.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MAX_TOKEN_SIZE      40
#define MAX_BUF_SIZE        40   /* Size of input buffer in bytes */


#define LEX_OUTSIZE         -3
#define LEX_ERROR           -2
#define LEX_EOF             -1
#define LEX_UNMATCHED       0
#define LEX_SPECIAL         1
#define LEX_DIGIT           2
#define LEX_STR             3


#define STATE_START         0
#define STATE_SPECIAL       1
#define STATE_DIGIT         2
#define STATE_STR           3
#define STATE_ILLEGAL       4

/* emFile¶ÔdirpathºÍfileName\u017dóÐ¡Ð\u017d²»Ãô\u017eÐ£¬µ«¶ÔdeviceName\u017dóÐ¡Ð\u017dÃô\u017eÐ£¬
   ±ØÐëÓë_XXX_GetDriverName()Ò»ÖÂ£¬±\u0178³ÌÐòµÄnorºÍmmc¶\u0152ÊÇÐ¡Ð\u017d */
#define RING_DEFAULT_CFG_FILE   "/mnt/nand1-2/rings/default.txt"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


char    text[MAX_TOKEN_SIZE];
int     leng;
FILE *input_file;
char    ch_buf[MAX_BUF_SIZE];	/* input buffer */
int     buf_pos;		        /* current position in input buffer */
int     num_chars;              /* Number of characters read into ch_buf */

/* Private function prototypes -----------------------------------------------*/
static int parseOneCfg(PlayList_Cfg_t *pCfg);

/* Private functions ---------------------------------------------------------*/
static int isspecial(int _C)
{
    return ((   _C == '<')
            || (_C == '>'));
}

static int input(char *ch)
{
    if (buf_pos < num_chars)
    {
        *ch = ch_buf[buf_pos];
        buf_pos++;
        return 1;
    }
    if (feof(input_file))
    {
        return LEX_EOF;
    }
    num_chars = fread(ch_buf, 1, MAX_BUF_SIZE, input_file);
    if ((num_chars == 0)
        && (ferror(input_file) != 0))
    {
        clearerr(input_file);
        return LEX_ERROR;
    }
    *ch = ch_buf[0];
    buf_pos = 1;
    return 1;
}

static void unput()
{
    buf_pos--;
    if (buf_pos < 0)
    {
        buf_pos = 0;
    }
}

/**
 * @brief ¶Á³öÖ\u017e¶\u0161IDºÅÅäÖÃ.
 * @param[out] pPlayList_Cfg Save the config retrieved from playlist file.
 * @param[in] CfgID Specified which cfg to read out, the first cfgID is 0.
 * @return Error code.
 * @retval ==0 Ok.
 * @retval <0  In case of any error.
 */
static int lex(void)
{
    char    hold_char = 0;
    int     currentState;

    memset(text, '\0', MAX_TOKEN_SIZE);
    leng = 0;
    currentState = STATE_START;

    while (1)
    {
        int result;
        result = input(&hold_char);
        if (result == LEX_ERROR)
        {
            return LEX_ERROR;
        }

        switch (currentState)
        {
        case STATE_START:
            if (result == LEX_EOF) // Input <1>
            {
                return LEX_EOF;
            }
            else if (isspace(hold_char)) // Input <2>
            {
                continue;
            }
            else if (isspecial(hold_char)) // Input <3>
            {
                text[0] = hold_char;
                leng = 1;
                currentState = STATE_SPECIAL;
                continue;
            }
            else if (isdigit(hold_char)) // Input <4>
            {
                text[0] = hold_char;
                leng = 1;
                currentState = STATE_DIGIT;
                continue;
            }
            else if (hold_char == '"') // Input <5>
            {
                leng = 0;
                currentState = STATE_STR;
                continue;
            }
            else if (isgraph(hold_char)) // Input <6> Leaving graph
            {
                currentState = STATE_ILLEGAL;
                continue;
            }
            else // Input <7> Leaving cntrl or ASCII > 127
            {
                continue;
            }
            break;
        case STATE_SPECIAL:
            if (result == LEX_EOF) // Input <1>
            {
                return LEX_SPECIAL;
            }
            else if (isspace(hold_char)) // Input <2>
            {
                unput();
                return LEX_SPECIAL;
            }
            else if (isspecial(hold_char)) // Input <3>
            {
                text[leng] = hold_char;
                leng++;
                continue;
            }
            else if (isdigit(hold_char)) // Input <4>
            {
                unput();
                return LEX_SPECIAL;
            }
            else if (hold_char == '"') // Input <5>
            {
                unput();
                return LEX_SPECIAL;
            }
            else if (isgraph(hold_char)) // Input <6> Leaving graph
            {
                unput();
                return LEX_SPECIAL;
            }
            else // Input <7> Leaving cntrl or ASCII > 127
            {
                unput();
                return LEX_SPECIAL;
            }
            break;
        case STATE_DIGIT:
            if (result == LEX_EOF) // Input <1>
            {
                return LEX_DIGIT;
            }
            else if (isspace(hold_char)) // Input <2>
            {
                unput();
                return LEX_DIGIT;
            }
            else if (isspecial(hold_char)) // Input <3>
            {
                unput();
                return LEX_DIGIT;
            }
            else if (isdigit(hold_char)) // Input <4>
            {
                text[leng] = hold_char;
                leng++;
                continue;
            }
            else if (hold_char == '"') // Input <5>
            {
                unput();
                return LEX_DIGIT;
            }
            else if (isgraph(hold_char)) // Input <6> Leaving graph
            {
                currentState = STATE_ILLEGAL;
                continue;
            }
            else // Input <7> Leaving cntrl or ASCII > 127
            {
                unput();
                return LEX_DIGIT;
            }
            break;
        case STATE_STR:
            if (result == LEX_EOF) // Input <1>
            {
                return LEX_UNMATCHED;
            }
            else if (isspace(hold_char)) // Input <2>
            {
                unput();
                return LEX_UNMATCHED;
            }
            else if (isspecial(hold_char)) // Input <3>
            {
                unput();
                return LEX_UNMATCHED;
            }
            else if (isdigit(hold_char)) // Input <4>
            {
                text[leng] = hold_char;
                leng++;
                continue;
            }
            else if (hold_char == '"') // Input <5>
            {
                return LEX_STR;
            }
            else if (isgraph(hold_char)) // Input <6> Leaving graph
            {
                text[leng] = hold_char;
                leng++;
                continue;
            }
            else // Input <7> Leaving cntrl or ASCII > 127
            {
                unput();
                return LEX_UNMATCHED;
            }
            break;
        case STATE_ILLEGAL:
            if (result == LEX_EOF) // Input <1>
            {
                return LEX_UNMATCHED;
            }
            else if (isspace(hold_char)) // Input <2>
            {
                unput();
                return LEX_UNMATCHED;
            }
            else if (isspecial(hold_char)) // Input <3>
            {
                unput();
                return LEX_UNMATCHED;
            }
            else if (isdigit(hold_char)) // Input <4>
            {
                continue;
            }
            else if (hold_char == '"') // Input <5>
            {
                unput();
                return LEX_UNMATCHED;
            }
            else if (isgraph(hold_char)) // Input <6> Leaving graph
            {
                continue;
            }
            else // Input <7> Leaving cntrl or ASCII > 127
            {
                unput();
                return LEX_UNMATCHED;
            }
            break;
        default:
            break;
        } // end of switch (currentState)
    } // end of while (1)
}

/**
 * @brief ¶Á³öÖ\u017e¶\u0161IDºÅÅäÖÃ.
 * @param[out] pPlayList_Cfg Save the config retrieved from playlist file.
 * @return Error code.
 * @retval ==0 Ok.
 * @retval <0  In case of any error.
 */
static int parseOneCfg(PlayList_Cfg_t *pCfg)
{
    int             step;

    memset(pCfg, '\0', sizeof(PlayList_Cfg_t));
    step = 0;
    do
    {
        int result;
        result = lex();
        if (result < 0)
        {
            return PLAYLIST_ERR_PARSE_CFG;
        }

        switch (step)
        {
        case 0:// <
            if ((result == LEX_SPECIAL)
                && (strcmp(text, "<") == 0))
            {
                step = 1;
            }
            break;
        case 1: // ID
            if (result == LEX_DIGIT)
            {
                pCfg->ID = atoi(text);
                step = 2;
            }
            else
            {
                return PLAYLIST_ERR_PARSE_CFG;
            }
            break;
        case 2: // FileName
            if (result == LEX_STR)
            {
                strcpy(pCfg->FileName, text);
                step = 3;
            }
            else
            {
                return PLAYLIST_ERR_PARSE_CFG;
            }
            break;
        case 3: // DispName
            if (result == LEX_STR)
            {
                strcpy(pCfg->DispName, text);
                step = 4;
            }
            else
            {
                return PLAYLIST_ERR_PARSE_CFG;
            }
            break;
        case 4: // Length
            if (result == LEX_DIGIT)
            {
                pCfg->Length = atoi(text);
                step = 5;
            }
            else
            {
                return PLAYLIST_ERR_PARSE_CFG;
            }
            break;
        case 5: // >
            if ((result == LEX_SPECIAL)
                && (strcmp(text, ">") == 0))
            {
                return PLAYLIST_OK;
            }
            else
            {
                return PLAYLIST_ERR_PARSE_CFG;
            }
            break;
        default:
            break;
        }
    } while (1);
}

/* Public functions ----------------------------------------------------------*/

/**
 * @brief ¶Á³öÖ\u017e¶\u0161IDºÅÇúÄ¿µÄÎÄ\u0152þÃû³Æ.
 * @param[out] pDispName Save the file name retrieved from playlist file.
 * @param[in] TuneID Specified which tune to read out, the first tune
 *            ID is 1.
 * @return Error code.
 * @retval ==0 Ok.
 * @retval <0  In case of any error.
 */
int PlayList_GetFileName(char *pFileName, unsigned char TuneID)
{
	PlayList_Cfg_t  tmp_cfg;
	int             errCode;

	input_file = fopen(RING_DEFAULT_CFG_FILE, "r");
	if (input_file == 0)
	{
	    return PLAYLIST_ERR_UNOPENING_FILE;
	}
	fseek(input_file, 0, SEEK_SET);
	buf_pos = 0;
	num_chars = 0;

	errCode = PLAYLIST_ERR_PARSE_CFG;
	while (1)
	{
	    int result;
	    result = parseOneCfg(&tmp_cfg);
	    if (result != PLAYLIST_OK)
	    {
	        break;
	    }
	    if (tmp_cfg.ID == TuneID)
	    {
	        strcpy(pFileName, tmp_cfg.FileName);
	        errCode = PLAYLIST_OK;
	        break;
	    }
	}

	fclose(input_file);
	return errCode;
}

/**
 * @brief ¶Á³öÖ\u017e¶\u0161IDºÅÇúÄ¿µÄÏÔÊ\u0178Ãû³Æ.
 * @param[out] pDispName Save the display name retrieved from playlist file.
 * @param[in] TuneID Specified which tune to read out, the first tune
 *            ID is 1.
 * @return Error code.
 * @retval ==0 Ok.
 * @retval <0  In case of any error.
 */
int PlayList_GetDispName(char *pDispName, unsigned char TuneID)
{
    PlayList_Cfg_t  tmp_cfg;
    int             errCode;

    input_file = fopen(RING_DEFAULT_CFG_FILE, "r");
    if (input_file == 0)
    {
        return PLAYLIST_ERR_UNOPENING_FILE;
    }
    fseek(input_file, 0, SEEK_SET);
    buf_pos = 0;
    num_chars = 0;

    errCode = PLAYLIST_ERR_PARSE_CFG;
    while (1)
    {
        int result;
        result = parseOneCfg(&tmp_cfg);
        if (result != PLAYLIST_OK)
        {
            break;
        }
        if (tmp_cfg.ID == TuneID)
        {
            strcpy(pDispName, tmp_cfg.DispName);
            errCode = PLAYLIST_OK;
            break;
        }
    }

    fclose(input_file);
    return errCode;
}

/**
 * @brief »ñÈ¡ÇúÄ¿µÄÊ¹ÓÃ³\u20ac¶È£¬ÈôÅäÖÃlength ×ÓÏîÄÚÈÝ²»Îª0£¬ÔòÖ±\u0153Ó·µ»Ø\u017eÃ×ÓÏîµÄÖµ£»
 *        ÈôÅäÖÃlength ×ÓÏîÄÚÈÝÎª0£¬Ôò·µ»ØÇúÄ¿ÎïÀí³\u20ac¶È\u0152ÆËãÖµ¡£
 * @param[in] TuneID Specified which tune to read out, the first tune
 *            ID is 1.
 * @retval >=0 The using length of tunes in ms.
 * @retval <0  In case of any error.
 */
int PlayList_GetUsingLength_ms(unsigned char TuneID, unsigned long *time)
{
	PlayList_Cfg_t  tmp_cfg;
	int             leng_ms;
	char            tmp_Buf[70] = {0};

	input_file = fopen(RING_DEFAULT_CFG_FILE, "r");
	if (input_file == 0)
	{
	    return PLAYLIST_ERR_UNOPENING_FILE;
	}
	fseek(input_file, 0, SEEK_SET);
	buf_pos = 0;
	num_chars = 0;

	leng_ms = PLAYLIST_ERR_PARSE_CFG;
	while (1)
	{
	    int result;
	    result = parseOneCfg(&tmp_cfg);
	    if (result != PLAYLIST_OK)
	    {
	        break;
	    }
	    if (tmp_cfg.ID == TuneID)
	    {
	        leng_ms = tmp_cfg.Length;
	        break;
	    }
	}

	fclose(input_file);

	if (leng_ms == 0)
	{
	    FILE         *pFile;
	    pFile = fopen(tmp_cfg.FileName, "r");
	    if (pFile == 0)
	    {
	        return PLAYLIST_ERR_UNOPENING_FILE;
	    }
	    memset(tmp_Buf, '\0', sizeof(tmp_Buf));
	    fread(tmp_Buf, 1, sizeof(tmp_Buf), pFile);// read .wav file's format head
	    if (ferror(pFile) != 0)
	    {
	        clearerr(pFile);
	        leng_ms = PLAYLIST_ERR_FS;
	    }
	    else
	    {
	    	 fseek(pFile, 0, SEEK_END);

		 leng_ms = ftell(pFile);

		 leng_ms = (1000*leng_ms)/8000;
	    }
	    fclose(pFile);
	}
	*time = leng_ms;
	return leng_ms;
}

/**
 * @brief »ñÈ¡ÇúÄ¿ÊýÁ¿. // 20140714 ¹æ¶\u0161²¥·ÅÁÐ±íÀïÓÐÐ§ÊýÄ¿Ð¡ÓÚ100£¬\u017dóÓÚ100µÄÓÃÓÚÆäËûÓÃÍ\u0178
 * @param None
 * @retval >=0 The number of tunes.
 * @retval <0  In case of any error.
 */
int PlayList_GetNumTunes(unsigned char *pTuneNum)
{
	PlayList_Cfg_t  tmp_cfg;
	int             num;

	input_file = fopen(RING_DEFAULT_CFG_FILE, "r");
	if (input_file == 0)
	{
	    return PLAYLIST_ERR_UNOPENING_FILE;
	}
	fseek(input_file, 0, SEEK_SET);
	buf_pos = 0;
	num_chars = 0;

	num = 0;
	while (1)
	{
	    int result;
	    result = parseOneCfg(&tmp_cfg);
	    if (result != PLAYLIST_OK)
	    {
	        break;
	    }
	    // 20150714
	    if( tmp_cfg.ID < 100 )
	    {
	        num++;
	    }
	}

	fclose(input_file);
	*pTuneNum = num;
	return num;
}
/* End Of File ---------------------------------------------------------------*/
