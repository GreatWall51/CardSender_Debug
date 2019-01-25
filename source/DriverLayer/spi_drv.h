/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 spi_drv.h
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
#ifndef _SPI_DRV_H_
#define _SPI_DRV_H_


/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>

/*----------------------------- Global Defines ------------------------------*/
#define SPI_CS_PIN_GROP   GPIOA
#define SPI_CS_PIN_NUM    GPIO_Pin_4
#define SPI_CS_PIN_CLOCK	RCC_AHBPeriph_GPIOA

/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/


void spi_drv_init ( void );
uint8_t spi_read_write ( uint8_t data );
#endif //_SPI_DRV_H_
