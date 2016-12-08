/**
  ******************************************************************************
  * @file    hal_gpio_def.h
  * @author  jia
  * @version V1.0.0
  * @date    2012.06.01
  * @brief   This file contains the define of GPIO use.
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2012 V-Tec</center></h2>
  ******************************************************************************
  */ 

#ifndef _hal_gpio_def_H_
#define _hal_gpio_def_H_

#if 0	// G24
#define MIC_MUTE_SET					_IOW('G', 10, unsigned int)
#define SPK_MUTE_SET					_IOW('G', 11, unsigned int)
#define BACK_LIGHT_1_SET				_IOW('G', 12, unsigned int)
#define BACK_LIGHT_2_SET				_IOW('G', 13, unsigned int)
#define LED_1_SET						_IOW('G', 14, unsigned int)
#define LED_2_SET						_IOW('G', 15, unsigned int)
#define LED_3_SET						_IOW('G', 16, unsigned int)
#define RL_CON_SET						_IOW('G', 17, unsigned int)
#define DET_CH_SET						_IOW('G', 18, unsigned int)
#define PWN_OUT_SET					_IOW('G', 19, unsigned int)
#else	// IPG
#define SOS_LED_SET		 			_IOW('G', 10, unsigned int) 
#define TALK_LED_SET		 		_IOW('G', 11, unsigned int) 
#define BACK_LIGHT_SET		 		_IOW('G', 12, unsigned int) 
#define LDS6204_INIT		 		_IOW('G', 13, unsigned int) 
#define LDS6204_REST				_IOW('G', 14, unsigned int) 
#define CD4094_OUT		 			_IOW('G', 15, unsigned int) 	
#define PWN_OUT_SET		 			_IOW('G', 16, unsigned int) 
#define MIC_MUTE_OUT_SET			_IOW('G', 17, unsigned int)
#define SPK_MUTE_OUT_SET			_IOW('G', 18, unsigned int)
#endif

#endif

