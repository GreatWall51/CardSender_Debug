/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : debug.c
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年7月15日
  最近修改   :
  功能描述   :
  函数列表   :
  修改历史   :
  1.日    期   : 2016年7月15日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include <stm32f0xx.h>
#include "timer_drv.h"
#include "hal_timer.h"
#include "define.h"
#include "debug.h"
#include "hal_debug.h"

/*----------------------- Constant / Macro Definitions -----------------------*/


/*------------------------ Variable Define/Declarations ----------------------*/
uint8_t g_fm23_init_flg;

int g_dbg_last_line = 0;
int g_dbg_pre_line = 0;
char* g_dbg_last_file = NULL;
char* g_dbg_pre_file = NULL;
uint32_t g_dbg_chk_timeout = 0;
size_t g_dbg_chk_val = 0;

/*----------------------------- External Variables ---------------------------*/
extern void putCh ( uint8_t ch );

//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;
	/* Whatever you require here. If the only file you are using is */
	/* standard output using printf() for debugging, no file handling */
	/* is required. */
};
/* FILE is typedef’ d in stdio.h. */
FILE __stdout;
int _sys_exit ( int x )
{
	x = x;
	return 0;
}
void _ttywrch ( int ch )
{
	ch = ch;
}

//重定义fputc函数
int fputc ( int ch, FILE* f )
{
	putCh ( ch );
	return ch;
}

/*------------------------ Function Prototype --------------------------------*/


/*------------------------ Variable Define/Declarations ----------------------*/


/*------------------------ Function Implement --------------------------------*/


/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
void debug_init()
{
	debug_uart_init();
	debug ( "..........DEBUG_ENABLE..........\r\n" );
}
void debug_real_time ( void )
{
	hal_debug_real_time();
}

