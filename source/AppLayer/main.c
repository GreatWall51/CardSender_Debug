/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 main.c
* Desc:
*
*
* Author: 	 liuwq
* Date: 	 2017/09/23
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2017/09/23, liuwq create this file
*
******************************************************************************/
/*------------------------------- Includes ----------------------------------*/
#include "opt.h"
#include <stm32f0xx.h>
#include "wdg_drv.h"
#include "timer_drv.h"
#include "beep_drv.h"
#include "debug.h"

#if CARD_READ
#include "fm17x2_drv.h"
#include "card_read_drv.h"
#include "card_rw_drv.h"
#endif

void drv_init ( void )
{
	init_timer();
	beep_drv_init();
#if (CFG_ENABLE_DEBUG&0X01)
	debug_init();
#else
#endif

#if CARD_READ
	card_read_drv_init();
	card_rw_drv_init();
#endif



#if ((!CFG_ENABLE_DEBUG)&0X01)
	wdg_drv_init();
#endif
}

void app_init ( void )
{

}

void drv_run_real_time ( void )
{
	timer_real_time();
	beep_real_time();
#if (CARD_READ ==1 )&&(CARD_DEBUG==0)
	card_read_drv_real_time();
#endif
}

void app_run_real_time ( void )
{
#if (CFG_ENABLE_DEBUG&0X01)
	debug_real_time();
#else
#endif
}

int main ( void )
{
	drv_init();
	app_init();
	while ( 1 )
	{
		drv_run_real_time();
		app_run_real_time();
		
		
//		{		//×Ô¶¯·¢¿¨
//				CARD_INFO_T card_data={0};
//				card_read_drv_real_time();
//				if(card_get_event(&card_data))
//				{
//					CPU_SENDER_CARD_T cpu_sender_card;
//					memcpy(cpu_sender_card.uid,card_data.card_uid,card_data.card_len);
//					cpu_sender_card.adf[0] = 0x0;
//					cpu_sender_card.adf[1] = 0x0;
//					cpu_card_sender(&cpu_sender_card);
//					debug("cpu_card_sender result: %x\r\n",cpu_sender_card.result);
//					if(cpu_sender_card.result == 0)
//					{
//						beep_set_mode(BEEP_OPEN_READ_MODE);
//					}
//					else
//					{
//						beep_set_mode(BEEP_CARD_2_MODE);
//					}
//				}
//				else
//				{
//					
//				}
//		}

#if ((!CFG_ENABLE_DEBUG)&0X01)
		wdg_feed_real_time();
#endif
	}
}
/*---------------------------------------------------------------------------*/
