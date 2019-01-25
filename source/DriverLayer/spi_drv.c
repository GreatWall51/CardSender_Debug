/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 spi_drv.c
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
#include "stm32f0xx.h"
#include "spi_drv.h"
/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
void spi_drv_init ( void )
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* GPIOA Periph clock enable */
	RCC_AHBPeriphClockCmd ( SPI_CS_PIN_CLOCK, ENABLE );
	/* Configure PA4 in output pushpull mode as SPI_CS*/
	GPIO_InitStructure.GPIO_Pin = SPI_CS_PIN_NUM;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init ( SPI_CS_PIN_GROP, &GPIO_InitStructure );

	/* Enable SPI1 and GPIOA clocks */
	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_SPI1, ENABLE );

	/* Configure SPI1 pins: NSS, SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init ( GPIOA, &GPIO_InitStructure );

	/* Connect PXx to SD_SPI_SCK */
	GPIO_PinAFConfig ( GPIOA, GPIO_PinSource5, GPIO_AF_0 );

	/* Connect PXx to SD_SPI_MISO */
	GPIO_PinAFConfig ( GPIOA, GPIO_PinSource6, GPIO_AF_0 );

	/* Connect PXx to SD_SPI_MOSI */
	GPIO_PinAFConfig ( GPIOA, GPIO_PinSource7, GPIO_AF_0 );

	SPI_I2S_DeInit ( SPI1 );
//   SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);
	SPI_RxFIFOThresholdConfig ( SPI1, SPI_RxFIFOThreshold_QF );
	/* SPI1 configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init ( SPI1, &SPI_InitStructure );

	/* Enable SPI1  */
	SPI_Cmd ( SPI1, ENABLE );
}
uint8_t spi_read_write ( uint8_t data )
{
	while ( ( SPI1->SR & SPI_I2S_FLAG_TXE ) == 0 );
	* ( uint8_t* ) &SPI1->DR = data;

	while ( ( SPI1->SR & SPI_I2S_FLAG_RXNE ) == 0 );

	return SPI1->DR;
}

/*---------------------------------------------------------------------------*/

