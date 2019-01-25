/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 fm17x2_drv.h
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
#ifndef _FM17X2_DRV_H_
#define _FM17X2_DRV_H_


/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>
#include "fm17x2_reg.h"
/*----------------------------- Global Defines ------------------------------*/
#define  FIFO_MAX_BUF 128

/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/
void fm17x2_drv_init ( void );

void reg_write ( uint8_t reg, uint8_t val );
uint8_t reg_read ( uint8_t reg );
uint8_t Read_FIFO ( uint8_t* buff );
void ISO14443A_Save_UID ( uint8_t row, uint8_t col, uint8_t length,uint8_t *recv_buf,uint8_t *uid);
void ISO14443A_Set_BitFraming ( uint8_t row, uint8_t col,uint8_t *recv_buf);
void SetTime_delay ( unsigned long delaytime );
uint8_t Load_key ( uint8_t* uncoded_keys );
void PcdInit ( uint8_t mode );
uint8_t Command_Send ( uint8_t count, uint8_t* buff, uint8_t Comm_Set );
uint8_t cmd_send_recv(uint8_t s_len,uint8_t *s_buf,uint8_t *r_len,uint8_t *r_buf);

#endif //_FM17X2_DRV_H_
