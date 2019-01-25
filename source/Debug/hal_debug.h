/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 hal_debug.h 
* Desc:
* 
* 
* Author: 	 LiuWeiQiang
* Date: 	 2017/05/18
* Notes: 
* 
* -----------------------------------------------------------------------------
* Histroy: v1.0   2017/05/18, LiuWeiQiang create this file
* 
******************************************************************************/
#ifndef _HAL_DEBUG_H_     
#define _HAL_DEBUG_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>
 
/*---------------------- Constant / Macro Definitions -----------------------*/
#define CMD_MAX_LEN 100 //串口命令允许输入的最大字符个数
#define ARG_MAX_CNT 10 //debug命令的最大参数个数

/*----------------------------- Global Typedefs -----------------------------*/

typedef struct
{
    uint8_t arg_cnt;
    char* arg[ARG_MAX_CNT+1];
}dbg_cmd_split_t;//串口命令分离器
 
/*----------------------------- Global Defines ------------------------------*/
#define DEBUG_CMD_FN(cmd) void _debug_cmd_fn_##cmd(void)
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
void putCh(uint8_t ch);
void debug_uart_disable(void);
void debug_uart_init(void);
//void cmd_resolver_init(void);
void hal_debug_real_time(void);
dbg_cmd_split_t *get_dbg_cmd_split(void);
#endif //_HAL_DEBUG_H_
