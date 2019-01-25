/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 wdg_drv.c 
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
#include "stm32f0xx.h"
#include "timer_drv.h"
#include "wdg_drv.h" 
 
/*------------------- Global Definitions and Declarations -------------------*/
uint32_t LsiFreq = 40000;
 
/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------- Type Declarations ---------------------------------*/
 
 
/*----------------------- Variable Declarations -----------------------------*/
 
 
/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/

void wdg_drv_init(void)
{
  if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
  {
    /* IWDGRST flag set */
    /* Turn on LED1 */

    /* Clear reset flags */
    RCC_ClearFlag();

  }
  else
	{
		
	}
    /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: LSI/32 */
  IWDG_SetPrescaler(IWDG_Prescaler_32);

  /* Set counter reload value to obtain 250ms IWDG TimeOut.
     Counter Reload Value = 250ms/IWDG counter clock period
                          = 250ms / (LSI/32)
                          = 0.25s / (LsiFreq/32)
                          = LsiFreq/(32 * 4)
                          = LsiFreq/128
   */
  IWDG_SetReload(LsiFreq/2);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();

}


void wdg_feed_real_time(void)
{
     if(IS_TIMER_EVT(TIME_EVT_16))
     {
         IWDG_ReloadCounter();  
     }
}

