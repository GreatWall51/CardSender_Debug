/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 lib_iso14443Bpcd.c
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


/*------------------------------- Includes ----------------------------------*/
#include "opt.h"
#include "lib_iso14443Bpcd.h"
#include "fm17x2_drv.h"
#include <string.h>
#include "define.h"

#if DEBUG_ISO14443B
#include "debug.h"
#endif
/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
static uint8_t iso14443B_error_check ( uint8_t cmd, uint8_t* ask, uint8_t ask_len )
{
	uint8_t result = ISO14443B_DEFAULT_ERR;
	if ( ask == NULL || ask_len < 1 )
	{
		return ISO14443B_PARAM_ERR;
	}
	switch ( cmd )
	{
		case ISO14443B_REQUEST:
			if ( ( ask_len == 12 ) && ( ask[0] == 0x50 ) )
			{
				result = ISO14443B_SUCCESS;
			}
			else
			{
				result = ISO14443B_REQUEST_ERR;
			}

			break;

		case ISO14443B_ANTICOL:
			if ( ( ask_len == 0x01 ) && ( ask[0] == 0x08 || ask[0] == 0x00 ) )
			{
				result = ISO14443B_SUCCESS;
			}
			else
			{
				result = ISO14443B_ANTICOL_ERR;
			}
			break;

		case ISO14443B_GETUID:
			if ( ( ask_len == 10 ) && ( ask[8] == 0x90 ) )
			{
				result = ISO14443B_SUCCESS;
			}
			else
			{
				result = ISO14443B_GETUID_ERR;
			}
			break;


		default:
			result = ISO14443B_DEFAULT_ERR;
			break;
	}
	return result;
}

/******************************************************************************
* Name: 	 ISO14443B_Request
*
* Desc:
* Param(in):
* Param(out): 	 PUPI
* Return:
* Global:
* Note:
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/10/11, Create this function by liuwq
*******************************************************************************/
static uint8_t ISO14443B_Request (void)
{	
	uint8_t 	result,send_recv_buf[64];

	reg_write(CRCPresetLSB,0xff);
	reg_write(CRCPresetMSB,0xff);
	reg_write(CWConductance,0x3f);
	reg_write(ModConductance,0x0E);//04

	reg_write(Bit_Frame,0x07);		/* 发送7bit */
	reg_write(ChannelRedundancy,0x2c);	/* 关闭CRC */
	result = reg_read(Control);
	result = result & (0xf7);
	reg_write(Control,result);			/* 屏蔽CRYPTO1位 */

	send_recv_buf[0] = 0x05;	
	send_recv_buf[1] = 0x00;		
	send_recv_buf[2] = 0x00;		/* Request模式选择 */
#if DEBUG_ISO14443B
	debug ( "SEND ISO14443B_Request CMD:\r\n" );
	debug_hex ( send_recv_buf, 3 );
#endif
	SetTime_delay(8);
	result = Command_Send(3, send_recv_buf, Transceive);
	if(result == FALSE)
	{
		return FM1702_NOTAGERR;  //无卡
	}
  result = Read_FIFO(send_recv_buf);		/* 从FIFO中读取应答信息 *///考虑判断返回的数据长度
#if DEBUG_ISO14443B
	debug ( "ISO14443B_Request ASK:\r\n" );
	debug_hex ( send_recv_buf,result);
#endif
	return iso14443B_error_check(ISO14443B_REQUEST,send_recv_buf,result); 
}


static uint8_t ISO14443B_Anticoll (void)
{
	uint8_t result,send_recv_buf[64]={0};

	reg_write(CWConductance,0x3f);
	reg_write(ModConductance,0x0E);
	reg_write(CRCPresetLSB,0xff);
	reg_write(CRCPresetMSB,0xff);
	reg_write(ChannelRedundancy,0x2c);

	send_recv_buf[0] = 0x1d;
	send_recv_buf[1] = 0x00;
	send_recv_buf[2] = 0x00;
	send_recv_buf[3] = 0x00;
	send_recv_buf[4] = 0x00;
	send_recv_buf[5] = 0x00;
	send_recv_buf[6] = 0x08;
	send_recv_buf[7] = 0x01;
	send_recv_buf[8] = 0x08;
#if DEBUG_ISO14443B
	debug ( "SEND ISO14443B_Anticoll CMD:\r\n" );
	debug_hex ( send_recv_buf, 9 );
#endif
	SetTime_delay(8);
	result = Command_Send(9, send_recv_buf, Transceive);
	if (result == FALSE)
	{
		return FM1702_NOTAGERR;
	}
	
	result = Read_FIFO(send_recv_buf);
	
#if DEBUG_ISO14443B
	debug ( "recv ISO14443B_Anticoll ask:\r\n" );
	debug_hex ( send_recv_buf,result);
#endif
	
	return iso14443B_error_check(ISO14443B_ANTICOL,send_recv_buf,result);
}
static uint8_t ISO14443B_ReadUID ( uint8_t* uid )
{
	uint8_t result,send_recv_buf[64]={0};

	reg_write(CWConductance,0x3f);
	reg_write(ModConductance,0x0E);
	reg_write(CRCPresetLSB,0xff);
	reg_write(CRCPresetMSB,0xff);
	reg_write(ChannelRedundancy,0x2c);

	send_recv_buf[0] = 0x00;
	send_recv_buf[1] = 0x36;
	send_recv_buf[2] = 0x00;
	send_recv_buf[3] = 0x00;
	send_recv_buf[4] = 0x08;
#if DEBUG_ISO14443B
	debug ( "send ISO14443B_ReadUID cmd:\r\n" );
	debug_hex ( send_recv_buf, 5);
#endif
  SetTime_delay(8);
	result = Command_Send(5, send_recv_buf, Transceive);
	if (result == FALSE)
	{
		return FM1702_NOTAGERR;
	}
	
	result = Read_FIFO(send_recv_buf);
#if DEBUG_ISO14443B
	debug ( "recv ISO14443B_ReadUID ask:\r\n" );
	debug_hex ( send_recv_buf,result);
#endif
	result = iso14443B_error_check(ISO14443B_GETUID,send_recv_buf,result);
	if(result == ISO14443B_SUCCESS)
	{
		memcpy(uid,send_recv_buf,8);
		return ISO14443B_SUCCESS;
	}
	return ISO14443B_FAILURE;
}

uint8_t scan_ISO14443B_card ( uint8_t* uid )
{
	uint8_t result = ISO14443B_DEFAULT_ERR;
	if ( uid == NULL ) return ISO14443B_PARAM_ERR;
	PcdInit ( TYPEB_MODE );
	memset(uid,0,8);
	result = ISO14443B_Request ();
	if ( result != ISO14443B_SUCCESS ) return ISO14443B_REQUEST_ERR;

	result = ISO14443B_Anticoll ();
	if ( result != ISO14443B_SUCCESS ) return ISO14443B_ANTICOL_ERR;


	result = ISO14443B_ReadUID ( uid );
	if ( result != ISO14443B_SUCCESS ) return ISO14443B_GETUID_ERR;
	return ISO14443B_SUCCESS;
}
/*---------------------------------------------------------------------------*/

