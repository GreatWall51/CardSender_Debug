/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 lib_iso14443Bpcd.h 
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
#ifndef _LIB_ISO14443BPCD_H_     
#define _LIB_ISO14443BPCD_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
 #include <stdint.h>
 
/*----------------------------- Global Defines ------------------------------*/

 
/*----------------------------- Global Typedefs -----------------------------*/
typedef enum{
	ISO14443B_SUCCESS = 0,
	ISO14443B_FAILURE = 1,
}ISO14443B_RESULT_E;

typedef enum{
	ISO14443B_REQUEST,
	ISO14443B_ANTICOL,
	ISO14443B_GETUID,
}ISO14443B_CMD_E;
typedef enum{
	ISO14443B_SUCCESSCODE = 0,
	ISO14443B_DEFAULT_ERR,
	ISO14443B_PARAM_ERR,
	ISO14443B_REQUEST_ERR,
	ISO14443B_ANTICOL_ERR,
	ISO14443B_GETUID_ERR,
	ISO14443B_SEND_CMD_ERR,
}ISO14443B_ERR_CODE_E;
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
uint8_t scan_ISO14443B_card(uint8_t* uid);
 

#endif //_LIB_ISO14443BPCD_H_
