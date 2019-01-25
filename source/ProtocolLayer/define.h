/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 define.h 
* Desc:
* 
* 
* Author: 	 liuwq
* Date: 	 2017/09/23
* Notes: 
* 
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/09/23, liuwq create this file
* 
******************************************************************************/
#ifndef _DEFINE_H_     
#define _DEFINE_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#define ERROR		1
#define	OK			0
#define TRUE        1
#define FALSE       0
/*----------------------------- Global Typedefs -----------------------------*/
#define sbi(ADDRESS,BIT) 	((ADDRESS->BSRR = BIT))	// Set bit
#define cbi(ADDRESS,BIT) 	((ADDRESS->BRR = BIT))	// Clear bit
#define	bis(ADDRESS,BIT)	(GPIO_ReadInputDataBit(ADDRESS,BIT))	// Is bit set?
#define	bic(ADDRESS,BIT)	(!GPIO_ReadInputDataBit(ADDRESS,BIT))	// Is bit clear
#define	toggle(ADDRESS,BIT)	(GPIO_WriteBit(ADDRESS,BIT,(BitAction)((1-GPIO_ReadOutputDataBit(ADDRESS,BIT)))))	// Toggle bit
 
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
 
 

#endif //_DEFINE_H_
