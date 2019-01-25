/******************************************************************************
* Copyright 2010-2014 Renyucjs@163.com
* FileName: 	 TimerRun.c
* Desc:
*
*
* Author: 	 Kimson
* Date: 	 2014/03/06
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2014/03/06, Kimson create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "timer_drv.h"
#include "hal_timer.h"
/*------------------- Global Definitions and Declarations -------------------*/

/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/

/*----------------------- Variable Declarations -----------------------------*/
static uint32_t s_timeIntCnt = 0;//基础分频计数
uint16_t  g_SysTimerEvt = 0;
static uint32_t  s_SysTimerEvt = 0;
static uint32_t delay_timer = 0;
/** 填写程序需要的定时分频事件 **/
const static TIME_EVT_ENUM s_ReqEvt[] =
{
	TIME_EVT_1,
	TIME_EVT_2,
	TIME_EVT_4,
	TIME_EVT_8,
	TIME_EVT_16,
	TIME_EVT_32,
	TIME_EVT_64,
	TIME_EVT_128,
	TIME_EVT_256,
	TIME_EVT_512,
	TIME_EVT_1024,
	TIME_EVT_2048,
	TIME_EVT_4096,
	TIME_EVT_8192,
};
/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/


/******************************************************************************
* Name: 	 init_timer
*
* Desc:
* Param:
* Return:
* Global:
* Note:
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2014/03/14, Create this function by Kimson
 ******************************************************************************/
void init_timer ( void )
{
	g_SysTimerEvt = 0;
	s_SysTimerEvt = 0;
	timer_init();
}
/******************************************************************************
* Name: 	 timer_real_time
*
* Desc:
* Param:
* Return:
* Global:
* Note:
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2014/03/14, Create this function by Kimson
 ******************************************************************************/
void timer_real_time ( void )
{
	// Clear event
	g_SysTimerEvt = 0;
	// nothing happend
	if ( !s_SysTimerEvt )
	{
		return;
	}
	//Set Event
	g_SysTimerEvt = s_SysTimerEvt;
	s_SysTimerEvt = 0;///TODO
}

/******************************************************************************
* Name: 	 timer_irq
*
* Desc: 	 This function must be run in TIME_BASE period IRQ Handler
* Param:
* Return:
* Global:
* Note:
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2014/03/14, Create this function by Kimson
 ******************************************************************************/
void timer_irq_callback ( void )
{
	uint8_t i;
	s_timeIntCnt ++;
	if ( delay_timer > 0 )
	{
		delay_timer--;
	}
	for ( i = 0; i < sizeof ( s_ReqEvt ) /sizeof ( s_ReqEvt[0] ); i++ )
	{
		if ( ! ( s_timeIntCnt & GET_EVT_MASK ( s_ReqEvt[i] ) ) )
		{
			s_SysTimerEvt |= s_ReqEvt[i];
		}
	}

}
uint32_t get_sys_timer ( void )
{
	return s_timeIntCnt;
}
void delay_ms ( uint16_t nms )
{
	delay_timer = nms;
	while ( delay_timer );
	return ;
}

/*---------------------------------------------------------------------------*/


