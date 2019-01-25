/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 hal_fm17x2.h
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
#ifndef _HAL_FM17X2_H_
#define _HAL_FM17X2_H_


/*------------------------------- Includes ----------------------------------*/
#include "define.h"
#include "spi_drv.h"

/*----------------------------- Global Defines ------------------------------*/
#define FM1702_RST_PIN_GROP   GPIOB
#define FM1702_RST_PIN_NUM    GPIO_Pin_1
#define FM1702_RST_PIN_CLOCK	RCC_AHBPeriph_GPIOB


#define FM1702_RST_EN() do{sbi(FM1702_RST_PIN_GROP,FM1702_RST_PIN_NUM);}while(0) 
#define FM1702_RST_DIS() do{cbi(FM1702_RST_PIN_GROP,FM1702_RST_PIN_NUM);}while(0) 

#define SPI_CS_EN() do{cbi(SPI_CS_PIN_GROP,SPI_CS_PIN_NUM);}while(0) 
#define SPI_CS_DIS() do{sbi(SPI_CS_PIN_GROP,SPI_CS_PIN_NUM);}while(0)
#define SPI_READ_WRITE(data) spi_read_write(data)

/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/
void hal_fm17x2_init(void);


#endif //_HAL_FM17X2_H_
