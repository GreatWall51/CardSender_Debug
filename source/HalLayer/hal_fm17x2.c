/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 hal_fm17x2.c
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/05/24
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/05/24, Liuwq create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "hal_fm17x2.h"


/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/


/*---------------------------------------------------------------------------*/
void fm17x2_reset_pin_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);					  											 
	GPIO_InitStructure.GPIO_Pin = FM1702_RST_PIN_NUM;                 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(FM1702_RST_PIN_GROP, &GPIO_InitStructure);
  cbi(FM1702_RST_PIN_GROP,FM1702_RST_PIN_NUM);
	//注意同步修改hal_fm1722.h中的FM1702_RST_EN()和FM1702_RST_DIS()这两个宏
}
void hal_fm17x2_init(void)
{
	spi_drv_init();
	fm17x2_reset_pin_init();
}
