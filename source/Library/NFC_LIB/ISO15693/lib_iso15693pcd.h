/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 lib_iso15693pcd.h 
* Desc:
* 
* 
* Author: 	 liuwq
* Date: 	 2017/09/22
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2017/09/22, liuwq create this file
* 
******************************************************************************/
#ifndef _LIB_ISO15693PCD_H_     
#define _LIB_ISO15693PCD_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include <stdint.h> 
 
/*----------------------------- Global Defines ------------------------------*/
#define ISO15693_SUCCESSCODE 0
#define ISO15693_ERRORCODE 1
 
/*----------------------------- Global Typedefs -----------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
uint8_t scan_ISO15693_card(uint8_t* uid);
 

#endif //_LIB_ISO15693PCD_H_
