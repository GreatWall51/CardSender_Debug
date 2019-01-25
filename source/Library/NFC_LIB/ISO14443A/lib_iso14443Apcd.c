/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 lib_iso14443Apcd.c
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
#include "lib_iso14443Apcd.h"
#include "define.h"
#include "fm17x2_drv.h"
#include <string.h>
#include <stdio.h>
#include "timer_drv.h"
#if DEBUG_ISO14443A || DEBUG_CPU
#include "debug.h"
#endif
/*------------------- Global Definitions and Declarations -------------------*/
static ISO14443A_CARD iso14443a_card;

/*---------------------- Constant / Macro Definitions -----------------------*/
/* ATQ FLAG */
/* according to FSP ISO 11443-3 the b7&b8 bits of ATQA tag answer is UID size bit frame */
#define ISO14443A_UID_MASK				  0xC0
#define CASCADE_LVL_1					      1
#define CASCADE_LVL_2					      2
#define CASCADE_LVL_3					      3
#define ATQ_FLAG_UID_SINGLE_SIZE		0
#define	ATQ_FLAG_UID_DOUBLE_SIZE		1
#define ATQ_FLAG_UID_TRIPLE_SIZE		2
#define ISO14443A_UID_SINGLE_SIZE		4
#define	ISO14443A_UID_DOUBLE_SIZE		7
#define ISO14443A_UID_TRIPLE_SIZE	  10

/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
/******************************************************************************
* Name: 	 iso14443A_error_check
*
* Desc: 	 �ж�Ӧ���ź��Ƿ���ȷ
* Param:
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/05/25, Create this function by Liuwq
 ******************************************************************************/
static uint8_t iso14443A_error_check ( ISO14443A_CMD_E cmd, uint8_t* ask, uint8_t ask_len )
{
    uint8_t result = ISO14443A_DEFAULT_ERR;
    if ( cmd >= ISO14443A_CMD_MAX || ask == NULL || ask_len < 1 )
    {
        return ISO14443A_PARAM_ERR;
    }
    switch ( cmd )
    {
    case ISO14443A_REQUEST:
        if ( ( ask_len == ATQA_LEN ) && ( ask[0] == 0x04 || ask[0] == 0x08 ) )
        {
            result = ISO14443A_SUCCESS;
        }
        else
        {
            result = ISO14443A_REQUEST_ERR;
        }
        break;

    case ISO14443A_ANTICOL:
        if ( ( ask_len == 5 ) && ( ask[4] == ( ask[0]^ask[1]^ask[2]^ask[3] ) ) )
        {
            result = ISO14443A_SUCCESS;
        }
        else
        {
            result = ISO14443A_REQUEST_ERR;
        }
        break;

    case ISO14443A_SELECT:
        if ( ( ask_len == 1 ) && ((ask[0] & 0x20 )== 0x20))
        {
            if ( (ask[0] & 0x20 )== 0x20)
            {
                iso14443a_card.ATSSupported = 1;//cpu��
            }
            else
            {
                iso14443a_card.ATSSupported = 0;//ic��
            }
            result = ISO14443A_SUCCESS;
        }
	else
	{
		result = ISO14443A_REQUEST_ERR;
	}
        break;

    default:
        result = ISO14443A_DEFAULT_ERR;
        break;
    }
    return result;
}
static void ISO14443A_CompleteStructure ( uint8_t* pATQA )
{
    /* according to FSP ISO 11443-3 the b7&b8 bits of ATQA tag answer is UID size bit frame */
    /* Recovering the UID size */
    switch ( ( iso14443a_card.ATQA[0] & ISO14443A_UID_MASK ) >> 6 )
    {
    case ATQ_FLAG_UID_SINGLE_SIZE:
        iso14443a_card.UIDsize 			= ISO14443A_UID_SINGLE_SIZE;
        iso14443a_card.CascadeLevel 	= CASCADE_LVL_1;
        break;
    case ATQ_FLAG_UID_DOUBLE_SIZE:
        iso14443a_card.UIDsize 			= ISO14443A_UID_DOUBLE_SIZE;
        iso14443a_card.CascadeLevel 	= CASCADE_LVL_2;
        break;
    case ATQ_FLAG_UID_TRIPLE_SIZE:
        iso14443a_card.UIDsize 			= ISO14443A_UID_TRIPLE_SIZE;
        iso14443a_card.CascadeLevel 	= CASCADE_LVL_3;
        break;
    }
}
/******************************************************************************
* Name: 	 ISO14443A_REQA
*
* Desc: 	 ��������ų������п�
* Param(in): 	 req_type:ISO14443A_CMD_REQUEST_ALL->WakeUpѰ��������ȫ����
                          ISO14443A_CMD_REQUEST_STD->RequestѰ��������δ��������״̬��
* Param(out): 	 pTagType:ATQA
                 ˵��:ATQA:00 04 IC��
                           00 08 CPU��
* Return:
* Global:
* Note: 	 �յ�2�ֽ�Ӧ��00 04 IC����00 08 CPU��
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/09/29, Create this function by liuwq
*******************************************************************************/
static int8_t ISO14443A_REQA ( void )
{
    uint8_t result, send_recv_buff[64];

    reg_write ( CRCPresetLSB, 0x63 );		//CRCPresetLSB address is 0x23
    reg_write ( CWConductance, 0x3f );
    reg_write ( Bit_Frame, 0x07 );
    reg_write ( ChannelRedundancy, 0x03 );
    result = reg_read ( Control );
    result = result & ( 0xf7 );
    reg_write ( Control, result );			//Control reset value is 00
    SetTime_delay ( 8 );
    send_recv_buff[0] = ISO14443A_CMD_REQUEST_ALL;		/* Requestģʽѡ�� */

#if DEBUG_ISO14443A
    debug ( "send ISO14443A_REQA cmd:\r\n" );
    debug_hex ( send_recv_buff, 1 );
#endif
    result = Command_Send ( 1, send_recv_buff, Transceive ); //Transceive=0x1E	/* ���ͽ������� */
    if ( result == TRUE )
    {
        result = Read_FIFO ( send_recv_buff );		/* ��FIFO�ж�ȡӦ����Ϣ��RevBuffer[]�� */
#if DEBUG_ISO14443A
        debug ( "get ISO14443A_REQA ask!\r\n" );
        debug_hex ( send_recv_buff, result);
#endif
        result = iso14443A_error_check ( ISO14443A_REQUEST, send_recv_buff, result);
        if(result == ISO14443A_SUCCESS)
        {
#if DEBUG_ISO14443A
            debug ( "ISO14443A_REQA success!\r\n" );
#endif
            memcpy ( iso14443a_card.ATQA, send_recv_buff, ATQA_LEN );
            ISO14443A_CompleteStructure ( iso14443a_card.ATQA );
            return ISO14443A_SUCCESS;
        }
        else
        {
#if DEBUG_ISO14443A
            debug ( "ISO14443A_REQA fail!\r\n" );
#endif
            return ISO14443A_REQUEST_ERR;
        }
    }
#if DEBUG_ISO14443A
    debug ( "send ISO14443A_REQA cmd ERROR!\r\n" );
#endif
    return ISO14443A_REQUEST_ERR;
}
/******************************************************************************
* Name: 	 ISO14443A_Anticollision
*
* Desc: 	 ���ͷų�ͻָ��
* Param(in):
* Param(out): 	 pTagUID:���Ŵ�ŵ�ַ��
* Return:
* Global:
* Note: 	 �յ�5�ֽ�Ӧ��ǰ4�ֽ�Ϊ���ţ����һ�ֽ�ΪУ���
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/10/10, Create this function by liuwq
*******************************************************************************/
static int8_t ISO14443A_Anticollision ( void )
{
    uint8_t	result;
    uint8_t	i;
    uint8_t	row, col;
    uint8_t	pre_row;
    uint8_t send_recv_buf[64] = {0};

    row = 0;
    col = 0;
    pre_row = 0;
    reg_write ( CRCPresetLSB, 0x63 );
    reg_write ( CWConductance, 0x3f );
//	reg_write(ModConductance,0x3f);
    send_recv_buf[0] = ISO14443A_CMD_ANTICOL1;
    send_recv_buf[1] = ISO14443A_CMD_ANTICOL_NVB;
    reg_write ( ChannelRedundancy, 0x03 );
#if DEBUG_ISO14443A
    debug ( "send ISO14443A_ANTICOLLISION cmd:\r\n" );
    debug_hex ( send_recv_buf, 2 );
#endif
    SetTime_delay ( 8 );
    result = Command_Send ( 2, send_recv_buf, Transceive );
    while ( 1 )
    {
        if ( result == FALSE )
        {
            return ( ISO14443A_ANTICOL_ERR );
        }
        result = reg_read ( FIFO_Length );
        if ( result == 0 )
        {
            return ISO14443A_ANTICOL_ERR;
        }

        Read_FIFO ( send_recv_buf );
        ISO14443A_Save_UID ( row, col, result, send_recv_buf, iso14443a_card.UID ); /* ���յ���UID����UID������ */
        result = reg_read ( ErrorFlag );				 /* �жϽӅ������Ƿ���� */
        result = result & 0x01;
        if ( result == 0x00 )
        {
            result = iso14443A_error_check ( ISO14443A_ANTICOL, iso14443a_card.UID, 5 ); /* У���յ���UID */
            if ( result == ISO14443A_REQUEST_ERR )
            {
#if DEBUG_ISO14443A
                debug ( "ISO14443A_Anticollision  fail!\r\n" );
#endif
                return ISO14443A_ANTICOL_ERR;
            }
#if DEBUG_ISO14443A
            debug ( "ISO14443A_Anticollision  success!\r\n" );
            debug ( "Get uid:" );
            debug_hex ( iso14443a_card.UID, 4 );
#endif
            return ISO14443A_SUCCESS;
        }
        else
        {
            result = reg_read ( CollPos );				/* ��ȡ��ͻ���Ĵ��� */
            row = result / 8;
            col = result % 8;
            send_recv_buf[0] = ISO14443A_CMD_ANTICOL1;
            ISO14443A_Set_BitFraming ( row + pre_row, col, send_recv_buf ); /* ���ô��������ݵ��ֽ��� */
            pre_row = pre_row + row;
            for ( i = 0; i < pre_row + 1; i++ )
            {
                send_recv_buf[i + 2] = iso14443a_card.UID[i];
            }
            if ( col != 0x00 )
            {
                row = pre_row + 1;
            }
            else
            {
                row = pre_row;
            }
            SetTime_delay ( 8 );
            result = Command_Send ( row + 2, send_recv_buf, Transceive );
        }
    }
}
/******************************************************************************
* Name: 	 ISO14443A_Select
*
* Desc: 	 ѡ�п�ָ��
* Param(in): 	 ��ѡ�еĿ��Ŀ���
* Param(out):
* Return:
* Global:
* Note: 	 �յ�1�ֽڵ�Ӧ��IC��Ӧ��0x08��CPU��Ӧ��0x20
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/10/10, Create this function by liuwq
*******************************************************************************/
static uint8_t ISO14443A_Select ( void )
{
    uint8_t	result, i, send_recv_buf[64] = {0};

    reg_write ( CRCPresetLSB, 0x63 );
    reg_write ( CWConductance, 0x3f );
    send_recv_buf[0] = ISO14443A_CMD_SELECT1;
    send_recv_buf[1] = ISO14443A_CMD_SELECT_NVB;
    for ( i = 0; i < 5; i++ )
    {
        send_recv_buf[i + 2] = iso14443a_card.UID[i];
    }
    reg_write ( ChannelRedundancy, 0x0f );
#if DEBUG_ISO14443A
    debug ( "send ISO14443A_Select cmd:\r\n" );
    debug_hex ( send_recv_buf, 7 );
#endif
    SetTime_delay ( 8 );
    result = Command_Send ( 7, send_recv_buf, Transceive );

    if ( result == FALSE )
    {
        return ISO14443A_SELECT_ERR;
    }
    else
    {
        result = reg_read ( ErrorFlag );
        if ( ( result & 0x02 ) == 0x02 )
        {
            return ( FM1702_PARITYERR );
        }
        if ( ( result & 0x04 ) == 0x04 )
        {
            return ( FM1702_FRAMINGERR );
        }
        if ( ( result & 0x08 ) == 0x08 )
        {
            return ( FM1702_CRCERR );
        }
        result = reg_read ( FIFO_Length );
        if ( result != 1 )
        {
            return ( FM1702_BYTECOUNTERR );
        }
        result = Read_FIFO ( send_recv_buf );	/* ��FIFO�ж�ȡӦ����Ϣ */
#if DEBUG_ISO14443A
        debug ( "get ISO14443A_Select ask:\r\n" );
        debug_hex ( send_recv_buf, result );
#endif
        result = iso14443A_error_check(ISO14443A_SELECT, send_recv_buf, result);
#if DEBUG_ISO14443A
        if(result == ISO14443A_SUCCESS)
        {
            debug ( "ISO14443A_Select success!\r\n" );
        }
        else
        {
            debug ( "ISO14443A_Select fail:\r\n" );
        }
#endif
//		if(temp == 0x08 || temp==0x88 || (temp>=0x17 && temp<=0x20||temp==0x28)||temp==0x2a || temp == 0x53)	/* �ж�Ӧ���ź��Ƿ���ȷ */
        if ( send_recv_buf[0] != 0x04 )	/* �ж�Ӧ���ź��Ƿ���ȷ 0x04Ϊ���Ų�������ʶ������������ֻ����д��ʱ�ٽ����ж� */
        {
            return ISO14443A_SUCCESS;
        }
        else
        {
            return ISO14443A_SELECT_ERR;
        }
    }
}

/******************************************************************************
* Name: 	 ISO14443A_Halt
*
* Desc: 	 ͣ��
* Param(in):
* Param(out):
* Return:
* Global:
* Note: 	 ��ָ��ֱ�ӷ��Ͳ���ҪӦ��
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/10/10, Create this function by liuwq
*******************************************************************************/
uint8_t ISO14443A_Halt ( void )
{
    uint8_t	result, send_recv_buf[64] = {0};

    reg_write ( CRCPresetLSB, 0x63 );
    reg_write ( CWConductance, 0x3f );
    reg_write ( ChannelRedundancy, 0x03 );
    send_recv_buf[0] = ISO14443A_CMD_HALT;
    send_recv_buf[1] = 0x00;
    SetTime_delay ( 8 );
    result = Command_Send ( 2, send_recv_buf, Transmit );
    if ( result == TRUE )
    {
#if DEBUG_ISO14443A
        debug ( "ISO14443A_Halt success!\r\n" );
#endif
        return ISO14443A_SUCCESS;
    }
    else
    {
#if DEBUG_ISO14443A
        debug ( "send ISO14443A_Halt cmd ERROR!\r\n" );
#endif
        result = reg_read ( ErrorFlag );
        if ( ( result & 0x02 ) == 0x02 )
        {
            return ( FM1702_PARITYERR );
        }

        if ( ( result & 0x04 ) == 0x04 )
        {
            return ( FM1702_FRAMINGERR );
        }

        return ( FM1702_NOTAGERR );
    }
}
/******************************************************************************
* Name: 	 ISO14443A_Authen
*
* Desc: 	 ��Կ��֤
* Param(in): uid:����,block:������(0~15),key:��Կ(6�ֽ�),key_type:��Կ����
* Param(out):
* Return:   0->�ɹ�������->ʧ��
* Global:
* Note: 	 �׿���Ҫ��֤a��Կ��������д��������
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/10/10, Create this function by liuwq
*******************************************************************************/
uint8_t ISO14443A_Authen ( uint8_t* uid, uint8_t block, uint8_t* key, uint8_t key_type )
{

    uint8_t i, result, send_recv_buf[64] = {0};
    if ( ( uid == NULL ) || ( ( key_type != ISO14443A_CMD_AUTH_A ) && ( key_type != ISO14443A_CMD_AUTH_B ) ) || ( block > 63 ) )
    {
        return ISO14443A_PARAM_ERR;
    }

    result = Load_key ( key );


    if ( result == 0 )
    {
#if DEBUG_ISO14443A
        debug ( "load key success!\r\n" );
#endif
    }
    else
    {
#if DEBUG_ISO14443A
        debug ( "load key error!\r\n" );
#endif
        return ISO14443A_AUTHENT_ERR;
    }


    reg_write ( CRCPresetLSB, 0x63 );
    reg_write ( CWConductance, 0x3f );       //ѡ�����TX1��TX2�������ߵ��迹
    reg_write ( ModConductance, 0x3f );

    result = reg_read ( Control );			 //��ȡ����ָ��
    result = result & 0xf7;
    reg_write ( Control, result );


    send_recv_buf[0] = key_type;
    send_recv_buf[1] = block;//( block << 2 ) + 3;			 //���������׵�ַ
    for ( i = 0; i < 4; i++ )
    {
        send_recv_buf[2 + i] = uid[i];
    }
    reg_write ( ChannelRedundancy, 0x0f );   //����CRC,��żУ��У��
    SetTime_delay ( 8 );
    result = Command_Send ( 6, send_recv_buf, ISO14443A_CMD_AUTHEN_1 );	 	//* ��֤������֤���̵�1�� */

    if ( result == FALSE )
    {
        return FM1702_NOTAGERR;
    }
    result = reg_read ( ErrorFlag ); //ErrorFlag address is 0x0A.

    if ( ( result & 0x02 ) == 0x02 )
    {
        return FM1702_PARITYERR;
    }
    if ( ( result & 0x04 ) == 0x04 )
    {
        return FM1702_FRAMINGERR;
    }
    if ( ( result & 0x08 ) == 0x08 )
    {
        return FM1702_CRCERR;
    }
    SetTime_delay ( 8 );
    result = Command_Send ( 0, send_recv_buf, ISO14443A_CMD_AUTHEN_2 ); //AUTHENT2

    if ( result == FALSE )
    {
        return FM1702_NOTAGERR;
    }

    result = reg_read ( ErrorFlag );

    if ( ( result & 0x02 ) == 0x02 )
    {
        return FM1702_PARITYERR;
    }
    if ( ( result & 0x04 ) == 0x04 )
    {
        return FM1702_FRAMINGERR;
    }
    if ( ( result & 0x08 ) == 0x08 )
    {
        return FM1702_CRCERR;
    }

    result = reg_read ( Control );
    result = result & 0x08;          //Crypto1on=1��֤ͨ��
    if ( result == 0x08 )
    {
#if DEBUG_ISO14443A
        debug ( "ISO14443A_Authen success!\r\n" );
#endif
        return ISO14443A_SUCCESS;
    }
#if DEBUG_ISO14443A
    debug ( "ISO14443A_Authen fail!\r\n" );
#endif
    return ISO14443A_AUTHENT_ERR;
}

/******************************************************************************
* Name: 	 ISO14443A_Read
*
* Desc:      ��block���ݿ�
* Param(in): block:���Կ��
* Param(out):buff:����������(16�ֽ�)
* Return:
* Global:
* Note:
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/09/23, Create this function by liuwq
*******************************************************************************/
uint8_t ISO14443A_Read ( uint8_t block, uint8_t* buff )
{
    uint8_t	result;

    if ( block > 63 || buff == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
    reg_write ( CRCPresetLSB, 0x63 );
    reg_write ( CWConductance, 0x3f );
    reg_write ( ModConductance, 0x3f );
    reg_write ( ChannelRedundancy, 0x0f );

    buff[0] = ISO14443A_CMD_READ;
    buff[1] = block;
    SetTime_delay ( 8 );
    result = Command_Send ( 2, buff, Transceive );
    if ( result == 0 )
    {
        return FM1702_NOTAGERR;
    }

    result = reg_read ( ErrorFlag );
    if ( ( result & 0x02 ) == 0x02 )
    {
        return FM1702_PARITYERR;
    }
    if ( ( result & 0x04 ) == 0x04 )
    {
        return FM1702_FRAMINGERR;
    }
    if ( ( result & 0x08 ) == 0x08 )
    {
        return FM1702_CRCERR;
    }

    result = reg_read ( FIFO_Length );
    if ( result == 0x10 )
    {
        Read_FIFO ( buff );
#if DEBUG_ISO14443A
        debug ( "read block %d data:\r\n", block );
        debug_hex ( buff, 16 );
#endif
        return FM1702_OK;
    }
    else if ( result == 0x04 )
    {
        result = Read_FIFO ( buff );
    }
    else
    {
        return FM1702_BYTECOUNTERR;
    }
    return ISO14443A_FAILURE;
}
/******************************************************************************
* Name: 	 ISO14443A_Write
*
* Desc: 	 дblock���ݿ�
* Param(in):  block:���Կ�ţ�buff:��д����(16�ֽ�)
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/09/22, Create this function by liuwq
*******************************************************************************/
uint8_t ISO14443A_Write ( uint8_t block, uint8_t* buff )
{
    uint8_t	result, send_recv_buf[64] = {0};

    if ( block > 63 || buff == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
    reg_write ( CRCPresetLSB, 0x63 );
    reg_write ( CWConductance, 0x3f );
    reg_write ( ChannelRedundancy, 0x07 );
    send_recv_buf[0] = ISO14443A_CMD_WRITE;
    send_recv_buf[1] = block;
    SetTime_delay ( 8 );
    result = Command_Send ( 2, send_recv_buf, Transceive );
    if ( result == FALSE )
    {
        return ( FM1702_NOTAGERR );
    }

    result = reg_read ( FIFO_Length );
    if ( result == 0 )
    {
        return ( FM1702_BYTECOUNTERR );
    }

    Read_FIFO ( send_recv_buf );
    result = send_recv_buf[0];

    switch ( result )
    {
    case 0x00:
        return ( FM1702_NOTAUTHERR );
    case 0x04:
        return ( FM1702_EMPTY );
    case 0x0a:
        break;
    case 0x01:
        return ( FM1702_CRCERR );
    case 0x05:
        return ( FM1702_PARITYERR );
    default:
        return ( FM1702_WRITEERR );
    }
    SetTime_delay ( 8 );
    result = Command_Send ( 16, buff, Transceive );

    if ( result == TRUE )
    {
#if DEBUG_ISO14443A
        debug ( "ISO14443A_Write SUCCESS!\r\n" );
#endif
        return ISO14443A_SUCCESS;
    }
    else
    {
#if DEBUG_ISO14443A
        debug ( "ISO14443A_Write ERROR!\r\n" );
#endif
        result = reg_read ( ErrorFlag );
        if ( ( result & 0x02 ) == 0x02 )
        {
            return ( FM1702_PARITYERR );
        }
        else if ( ( result & 0x04 ) == 0x04 )
        {
            return ( FM1702_FRAMINGERR );
        }
        else if ( ( result & 0x08 ) == 0x08 )
        {
            return ( FM1702_CRCERR );
        }
        else
        {
            return ISO14443A_WRITE_ERR;
        }
    }
}
/******************************************************************************
* Name: 	 scan_ISO14443A_card
*
* Desc:      ɨ��A�࿨
* Param(in):
* Param(out):uid���Ŵ�ŵ�ַ
* Return:   0->�ɹ�������->ʧ��
* Global:
* Note:
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/10/11, Create this function by liuwq
*******************************************************************************/
uint8_t scan_ISO14443A_card ( uint8_t* uid )
{
    uint8_t result = ISO14443A_DEFAULT_ERR;
    if ( uid == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
    PcdInit ( TYPEA_MODE );

    memset ( &iso14443a_card, 0, sizeof ( ISO14443A_CARD ) );
    result = ISO14443A_REQA();
    if ( result != ISO14443A_SUCCESS )
    {
        return ISO14443A_REQUEST_ERR;
    }

    result = ISO14443A_Anticollision();
    if ( result != ISO14443A_SUCCESS )
    {
        return ISO14443A_ANTICOL_ERR;
    }

    result = ISO14443A_Select();
    if ( result != ISO14443A_SUCCESS )
    {
        return ISO14443A_REQUEST_ERR;
    }
    memcpy ( uid, iso14443a_card.UID, iso14443a_card.UIDsize );

    return ISO14443A_SUCCESSCODE;
}
/******************************************************************************
* Name: 	 is_support_rats
*
* Desc: 	 �ж��Ƿ�ΪCPU��
* Param(in):
* Param(out):
* Return: 	 1->CPU��,0->IC��
* Global:
* Note:
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/10/10, Create this function by liuwq
*******************************************************************************/
uint8_t is_support_rats ( void )
{
    return iso14443a_card.ATSSupported;
}


//////////////////////////////////////////////CPU������/////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
#include "des.h"
uint8_t CPU_PCB(void)
{
	static uint8_t PCB = 0;
	switch(PCB)
	{
		case 0x00:
		PCB=0x0a;
		break;
		case 0x0a:
		PCB=0x0b;
		break;
		case 0x0b:
		PCB=0x0a;
		break;
		default:
		PCB=0x0a;
		break; 
	}
	return PCB;
}

static uint8_t iso14443A_cpu_ask_check ( uint8_t cmd, uint8_t* ask, uint8_t ask_len )
{
    uint8_t result = ISO14443A_DEFAULT_ERR;
    if ( ask == NULL || ask_len < 1 )
    {
        return ISO14443A_PARAM_ERR;
    }
    switch ( cmd )
    {
    case ISO14443A_Rats:
        if ( ( ask_len == 16 ) && ( ask[6] == 0x90 ) && (( ask[7] == 0x00 )))
        {
            result = ISO14443A_SUCCESS;
        }
        else
        {
            result = ISO14443A_REQUEST_ERR;
        }
        break;
				
		case ISO14443A_GetRandom:
				if ( (ask_len == 12)&&(ask[ask_len - 2] == 0x90 )&&( ask[ask_len - 1] == 0x00 ))
				{
            result = ISO14443A_SUCCESS;
        }
        else
        {
            result = ISO14443A_GetRandom_ERR;
        }
        break;

    case ISO14443A_SelectFile:
		case ISO14443A_ExAuthenticate:
		case ISO14443A_InAuthenticate:
		case ISO14443A_DeleteADF:
		case ISO14443A_CreatADF:
		case ISO14443A_CreatKeyFile:
		case ISO14443A_AddKey:
		case ISO14443A_CreatEFFile:
		case ISO14443A_ReadEF:
		case ISO14443A_WriteEF:
				if ( (ask[ask_len - 2] == 0x90 )&&( ask[ask_len - 1] == 0x00 ))
				{
            result = ISO14443A_SUCCESS;
        }
        else
        {
            result = ISO14443A_SelectFile_ERR;
        }
        break;


    default:
        result = ISO14443A_DEFAULT_ERR;
        break;
    }
    return result;
}
/******************************************************************************
* Name: 	 ISO14443A_Rats
*
* Desc: 	 ����RATSָ��
* Param(in):
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/10/10, Create this function by liuwq
*******************************************************************************/
uint8_t Rats ( void )
{
    uint8_t result,r_len,send_recv_buf[FIFO_BUFF_MAX];

    send_recv_buf[0] = ISO14443A_CMD_RATS;
    send_recv_buf[1] = 0x51;
		#if DEBUG_CPU
		debug ( "send ISO14443A_Rats cmd:\r\n" );
		debug_hex ( send_recv_buf, 2 );
		#endif
		SetTime_delay ( 8 );
    result = cmd_send_recv(2,send_recv_buf,&r_len,send_recv_buf);
    if ( result == ISO14443A_SUCCESS )
    {
				#if DEBUG_CPU
				debug ( "get ISO14443A_Rats ask(len = %d):\r\n",r_len);
				debug_hex ( send_recv_buf,r_len);
				#endif
			
        if ( iso14443A_cpu_ask_check ( ISO14443A_Rats, send_recv_buf, r_len ) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "ISO14443A_Rats SUCCESS!\r\n" );
					#endif
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_Rats ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,r_len);
				  #endif
					return ISO14443A_RATS_ERR;
				}
    }
		else
		{
			#if DEBUG_CPU
			debug ( "send ISO14443A_Rats cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}

/******************************************************************************
* Name: 	 GetRandom
*
* Desc: 	 ��ȡ�����
* Param(in):
* Param(out):
* Return: 	 0->success
* Global:
* Note:
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/07, Create this function by liuwq
*******************************************************************************/

uint8_t GetRandom ( uint8_t* rece_len, uint8_t* data )
{
//#undef DEBUG_CPU
//#define DEBUG_CPU 0
    uint8_t	result,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* get_random;

    if ( rece_len == NULL || data == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
		
		send_recv_buf[0] = CPU_PCB();
		send_recv_buf[1] = 0x01;
		get_random = ( CPU_CMD_T* ) &send_recv_buf[2];
    get_random->CLA = 0x00;
    get_random->INS = 0x84;
    get_random->P1 = 0x00;
    get_random->P2 = 0x00;
    get_random->LC = 0x08;
#if DEBUG_CPU
		debug ( "send GetRandom cmd:\r\n" );
		debug_hex(send_recv_buf,7);
#endif
		SetTime_delay(20);
    result = cmd_send_recv (7, send_recv_buf, rece_len, send_recv_buf);
		if(*rece_len < 2) return ISO14443A_GetRandom_ERR;
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_GetRandom,send_recv_buf,*rece_len) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "GetRandom ask (len = %d):\r\n", *rece_len );
					debug_hex ( send_recv_buf, *rece_len );
					debug ( "GetRandom success!\r\n" );
					#endif
					memcpy ( data, &send_recv_buf[2], *rece_len - 4 );
					*rece_len=8;
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_Rats ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,*rece_len);
				  #endif
					return ISO14443A_GetRandom_ERR;
				}
		}
		else
		{
				#if DEBUG_CPU
								debug ( "send GetRandom cmd error!\r\n" );
				#endif
		}
//#undef DEBUG_CPU
//#define DEBUG_CPU 1
		return ISO14443A_SEND_CMD_ERROR;
}

/******************************************************************************
* Name: 	 SelectFile
*
* Desc: 	 ѡ���ļ�
* Param(in):
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/08, Create this function by liuwq
*******************************************************************************/
uint8_t SelectFile ( uint8_t* file_id )
{
    uint8_t	result, rece_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* select_file;

    if ( file_id == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
		send_recv_buf[0] = CPU_PCB();
		send_recv_buf[1] = 0x01;
    select_file = ( CPU_CMD_T* ) &send_recv_buf[2];
    select_file->CLA = 0x00;
    select_file->INS = 0xa4;
    select_file->P1 = 0x00;
    select_file->P2 = 0x00;
    select_file->LC = 0x02;
    select_file->data_buff[0] = *file_id;
    select_file->data_buff[1] = * ( file_id + 1 );
		#if DEBUG_CPU
		debug ( "send select file cmd:\r\n" );
		debug_hex (send_recv_buf,9 );
		#endif
		SetTime_delay(100);
    result = cmd_send_recv ( 9, send_recv_buf, &rece_len, send_recv_buf );
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_SelectFile,send_recv_buf,rece_len) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "get SelectFile(id:%x %x) ask (len = %d):\r\n", *file_id, * ( file_id + 1 ), rece_len );
					debug_hex ( send_recv_buf, rece_len );
					debug("SelectFile success!\r\n");
					#endif
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_SelectFile ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,rece_len);
				  #endif
					return ISO14443A_SelectFile_ERR;
				}
		}
		else
		{
			#if DEBUG_CPU
			debug ( "send SelectFile cmd error!\r\n" );
			#endif
		}
		return ISO14443A_SEND_CMD_ERROR;
}

/******************************************************************************
* Name: 	 ExAuthenticate
*
* Desc: 	 �ⲿ��֤
* Param(in):
* Param(out):
* Return: 0->success
* Global:
* Note:		�ⲿ��֤��Ϊ�˶��豸�ĺϷ��Խ����ж�(�ж��Ƿ�Ϊ���ǵ��豸)��
					ֻ���ⲿ��֤ͨ�����豸���ܶԿ����в�����
					Ҫ�����ⲿ��֤������Ҫ��֤����Կ�ļ������ⲿ��֤��Կ��
				  �豸��cpu������һ���������Ȼ����������
					CPU�����ݴ��������des_in���������ȴ�������Կ���м������㣬��
					�����Ľ���봫���des_out�Աȣ���һ�£�����֤�ɹ�
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/07, Create this function by liuwq
*******************************************************************************/
uint8_t ExAuthenticate ( uint8_t key_cn, uint8_t* key_buff )
{
    uint8_t	result, r_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    uint8_t des_in[8], des_out[8];
    CPU_CMD_T* ex_auth;

    if ( key_buff == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
    result = GetRandom ( &r_len, des_in );	 //��ȡ�����
		if(result!= ISO14443A_SUCCESS) 
			return ISO14443A_GetRandom_ERR;
    TDES ( des_in, des_out, key_buff, ENCRY ); //8�ֽ��������TDES��4�ֽ��������DES
		send_recv_buf[0] = CPU_PCB();
		send_recv_buf[1] = 0x01;
    ex_auth = ( CPU_CMD_T* ) &send_recv_buf[2];
    ex_auth->CLA = 0x00;
    ex_auth->INS = 0x82;
    ex_auth->P1  = 0x00;
    ex_auth->P2  = key_cn;
    ex_auth->LC  = 0x08;
    memcpy (ex_auth->data_buff, des_out, ex_auth->LC );
		#if DEBUG_CPU
		debug ( "send ExAuthenticate cmd:\r\n" );
		debug_hex (send_recv_buf,15);
		#endif
		SetTime_delay(50);
    result = cmd_send_recv ( 15, send_recv_buf, &r_len, send_recv_buf );
		#if DEBUG_CPU
			debug ( "ExAuthenticate ask (len = %d):", r_len );
			debug_hex ( send_recv_buf, r_len );
			#endif
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_ExAuthenticate,send_recv_buf,r_len) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "get ExAuthenticate ask (len = %d):\r\n", r_len);
					debug_hex ( send_recv_buf, r_len );
					debug("ExAuthenticate success!\r\n");
					#endif
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_ExAuthenticate ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,r_len);
				  #endif
					return ISO14443A_ExAuthenticate_ERR;
				}
		}
		else
		{
			#if DEBUG_CPU
			debug ( "send ExAuthenticate cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}
/******************************************************************************
* Name: 	 InAuthenticate
*
* Desc: 	 CPU���ڲ���֤
* Param(in): 	 des_in:��֤������ des_out:��֤�Ľ��
* Param(out):
* Return:
* Global:
* Note: 	�ڲ���֤��Ϊ�˶Կ��ĺϷ��Խ����ж�(�ж��Ƿ�Ϊ���Ƿ��Ŀ�)
					Ҫ�����ڲ���֤������Ҫ��֤����Կ�ļ�������Կ
					CPU�����ݴ��������des_in���������ȴ�������Կ���м������㣬��
					�����Ľ���봫���des_out�Աȣ���һ�£�����֤�ɹ�
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/08, Create this function by liuwq
*******************************************************************************/
uint8_t InAuthenticate ( uint8_t* des_in, uint8_t key_cn, uint8_t* des_out )
{
    uint8_t	result, r_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* in_authen;

    if ( des_out == NULL || des_in == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
		
		send_recv_buf[0] = CPU_PCB();
		send_recv_buf[1] = 0x01;
    in_authen = ( CPU_CMD_T* ) &send_recv_buf[2];
    in_authen->CLA = 0x00;
    in_authen->INS = 0x88;
    in_authen->P1  = 0x00;
    in_authen->P2  = key_cn;
    in_authen->LC  = 0x08;
    memcpy ( in_authen->data_buff, des_in, in_authen->LC );
		#if DEBUG_CPU
    debug ( "des_in:" );
    debug_hex ( des_in, 8 );
    debug ( "hope result:" );
    debug_hex ( des_out, 8 );
		debug("send InAuthenticate cmd:\r\n");
		debug_hex(send_recv_buf,15);
		#endif
		SetTime_delay(100);
    result = cmd_send_recv ( 15, send_recv_buf, &r_len, send_recv_buf );
		
		if ( result == ISO14443A_SUCCESS )
    {			
        if ( iso14443A_cpu_ask_check ( ISO14443A_InAuthenticate, send_recv_buf, r_len ) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "get InAuthenticate ask(len = %d):\r\n",r_len);
					debug_hex ( send_recv_buf,r_len);
					#endif
					if ( memcmp ( des_out, &send_recv_buf[2], r_len-4 ) == 0 ) //�ڲ���Կ��ȷ
					{
						#if DEBUG_CPU
						debug ( "InAuthenticate success!\r\n" );
						#endif
						return ISO14443A_SUCCESS;
					}
					else
					{
						#if DEBUG_CPU
						debug ( "InAuthenticate error,key unsame!\r\n" );
						#endif
						return ISO14443A_InAuthenticate_ERR;
					}
					
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_InAuthenticate ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,r_len);
				  #endif
					return ISO14443A_InAuthenticate_ERR;
				}
    }
		else
		{
			#if DEBUG_CPU
			debug ( "send ISO14443A_InAuthenticate cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}
/******************************************************************************
* Name: 	 DeleteADF
*
* Desc: 	 ������ǰ DF �������ļ�
* Param(in):
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/08, Create this function by liuwq
*******************************************************************************/
uint8_t DeleteADF ( void )
{
    uint8_t	result, r_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* del_adf;
	
		send_recv_buf[0] = CPU_PCB();
		send_recv_buf[1] = 0x01;
    del_adf = ( CPU_CMD_T* ) &send_recv_buf[2];
    del_adf->CLA = 0x80;
    del_adf->INS = 0x0e;
    del_adf->P1 = 0;
    del_adf->P2 = 0;
    del_adf->LC = 0;
		#if DEBUG_CPU
		debug ( "send DeleteADF cmd:" );
		debug_hex ( send_recv_buf, 7 );
		#endif
	  SetTime_delay(100);
    result = cmd_send_recv ( 7, send_recv_buf, &r_len, send_recv_buf );
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_DeleteADF,send_recv_buf,r_len) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "get DeleteADF ask (len = %d):\r\n", r_len);
					debug_hex ( send_recv_buf, r_len );
					debug("DeleteADF success!\r\n");
					#endif
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_DeleteADF ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,r_len);
					#endif
					return ISO14443A_DeleteADF_ERR;
				}
		}
		else
		{
			#if DEBUG_CPU
			debug ( "send ISO14443A_DeleteADF cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}
/******************************************************************************
* Name: 	 CreatADF
*
* Desc: 	 �����ļ�
* Param(in):
* Param(out):
* Return: 	 0->�ɹ�,1->ADF�Ѿ����ڣ�2->ʧ��
* Global:
* Note: 	 Ŀ¼�ļ�����MF�⣩�������ܱ��Զ�ѡ��
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/08, Create this function by liuwq
*******************************************************************************/
uint8_t CreatADF ( uint8_t* file_id, uint8_t* size )
{
    uint8_t	result, r_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* creat_adf;

    if ( file_id == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
		send_recv_buf[0] = CPU_PCB();
		send_recv_buf[1] = 0x01;
    creat_adf = ( CPU_CMD_T* ) &send_recv_buf[2];
    creat_adf->CLA = 0x80;
    creat_adf->INS = 0xe0;
    creat_adf->P1 = *file_id;
    creat_adf->P2 = * ( file_id + 1 );
    creat_adf->LC = 0x0d;
    creat_adf->data_buff[0] = 0x38;   //����
    creat_adf->data_buff[1] = *size;  //�ļ��ռ�
    creat_adf->data_buff[2] = * ( size + 1 );
    creat_adf->data_buff[3] = 0xf0;   //����Ȩ��
    creat_adf->data_buff[4] = 0xf0;   //����Ȩ��
    creat_adf->data_buff[5] = 0x95;   //Ӧ���ļ�ID
    creat_adf->data_buff[6] = 0xff;   //����
    creat_adf->data_buff[7] = 0xff;   //����
    creat_adf->data_buff[8] = 0x44;   //DF����6-16�ֽ�
    creat_adf->data_buff[9] = 0x44;
    creat_adf->data_buff[10] = 0x46;
    creat_adf->data_buff[11] = 0x30;
    creat_adf->data_buff[12] = 0x31;
		#if DEBUG_CPU
		debug ( "send CreatADF cmd:" );
		debug_hex ( send_recv_buf, 20 );
		#endif
		SetTime_delay(100);
    result = cmd_send_recv ( 20, send_recv_buf, &r_len, send_recv_buf );
		
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_CreatADF,send_recv_buf,r_len) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "get CreatADF ask (len = %d):\r\n", r_len);
					debug_hex ( send_recv_buf, r_len );
					debug("CreatADF success!\r\n");
					#endif
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_CreatADF ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,r_len);
				  #endif
					if(send_recv_buf[2] == 0x6a && send_recv_buf[3] == 0x86)
					{
						return ISO14443A_FILE_ALREADY_EXIST;//ADF�ļ��Ѿ�����
					}
					else
					{
						return ISO14443A_CreatADF_ERR;
					}
				}
		}
		else
		{
			#if DEBUG_CPU
			debug ( "send CreatADF cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}

uint8_t ISO14443A_CreatDF ( CPU_CREATE_DF_DATA_T *creat_cmd)
{
    uint8_t	result, r_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* creat_adf;

    if ( creat_cmd == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
	send_recv_buf[0] = CPU_PCB();
	send_recv_buf[1] = 0x01;
    creat_adf = ( CPU_CMD_T* ) &send_recv_buf[2];
    creat_adf->CLA = 0x80;
    creat_adf->INS = 0xe0;
    creat_adf->P1 = creat_cmd->file_id[0];
    creat_adf->P2 = creat_cmd->file_id[1];
    creat_adf->LC = 8+creat_cmd->file_name_len;
	memcpy(creat_adf->data_buff,&creat_cmd->file_type,creat_adf->LC);
//	creat_df_data = (CPU_CREATE_DF_DATA_T*)creat_adf->data_buff;
//	creat_df_data->file_type = 0x38;
//    creat_df_data->file_size[0]=*size;
//	creat_df_data->file_size[1]=* ( size + 1 );
//	creat_df_data->build_authority = build_authority;
//	creat_df_data->delete_authority = delete_authority;
//	
//	creat_df_data->APP_ID = 
//	
//    creat_adf->data_buff[1] = *size;  //�ļ��ռ�
//    creat_adf->data_buff[2] = * ( size + 1 );
//    creat_adf->data_buff[3] = 0xf0;   //����Ȩ��
//    creat_adf->data_buff[4] = 0xf0;   //����Ȩ��
//    creat_adf->data_buff[5] = 0x95;   //Ӧ���ļ�ID
//    creat_adf->data_buff[6] = 0xff;   //����
//    creat_adf->data_buff[7] = 0xff;   //����
//    creat_adf->data_buff[8] = 0x44;   //DF����5-16�ֽ�
//    creat_adf->data_buff[9] = 0x44;
//    creat_adf->data_buff[10] = 0x46;
//    creat_adf->data_buff[11] = 0x30;
//    creat_adf->data_buff[12] = 0x31;
		#if DEBUG_CPU
		debug ( "send CreatADF cmd:" );
		debug_hex ( send_recv_buf, creat_adf->LC+7 );
		#endif
		SetTime_delay(100);
    result = cmd_send_recv ( creat_adf->LC+7, send_recv_buf, &r_len, send_recv_buf );
		
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_CreatADF,send_recv_buf,r_len) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "get CreatADF ask (len = %d):\r\n", r_len);
					debug_hex ( send_recv_buf, r_len );
					debug("CreatADF success!\r\n");
					#endif
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_CreatADF ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,r_len);
				  #endif
					if(send_recv_buf[2] == 0x6a && send_recv_buf[3] == 0x86)
					{
						return ISO14443A_FILE_ALREADY_EXIST;//ADF�ļ��Ѿ�����
					}
					else
					{
						return ISO14443A_CreatADF_ERR;
					}
				}
		}
		else
		{
			#if DEBUG_CPU
			debug ( "send CreatADF cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}
/******************************************************************************
* Name: 	 CreatKeyFile
*
* Desc: 	 ������Կ�ļ�
* Param(in):
* Param(out):
* Return: 	 0->�ɹ�,1->��Կ�ļ��Ѿ����ڣ�2->ʧ��
* Global:
* Note:
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/09, Create this function by liuwq
*******************************************************************************/
uint8_t CreatKeyFile ( uint8_t* file_id, uint8_t* size )
{
    uint8_t	result, r_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* carea_key_file;

    if ( file_id == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
		
		send_recv_buf[0] = CPU_PCB();
		send_recv_buf[1] = 0x01;
    carea_key_file = ( CPU_CMD_T* ) &send_recv_buf[2];
    carea_key_file->CLA = 0x80;
    carea_key_file->INS = 0xe0;
    carea_key_file->P1 = *file_id;
    carea_key_file->P2 = * ( file_id + 1 );
    carea_key_file->LC = 0x07;
    carea_key_file->data_buff[0] = 0x3f;     //�ļ�����
    carea_key_file->data_buff[1] = *size;    //�ļ��ռ�
    carea_key_file->data_buff[2] = * ( size + 1 );
    carea_key_file->data_buff[3] = 0x95;     //�̱�ʶ��
    carea_key_file->data_buff[4] = 0xf0;     //������ԿȨ�ޣ��������Ϊ����������
    carea_key_file->data_buff[5] = 0xff;
    carea_key_file->data_buff[6] = 0xff;
		#if DEBUG_CPU
		debug ( "send CreatKeyFile cmd:" );
		debug_hex ( send_recv_buf, 14 );
		#endif
		SetTime_delay(50);
    result = cmd_send_recv ( 14, send_recv_buf, &r_len, send_recv_buf );
		
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_CreatKeyFile,send_recv_buf,r_len) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "get CreatKeyFile ask (len = %d):\r\n", r_len);
					debug_hex ( send_recv_buf, r_len );
					debug("CreatKeyFile success!\r\n");
					#endif
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_CreatKeyFile ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,r_len);
				  #endif
					if(send_recv_buf[2] == 0x6a && send_recv_buf[3] == 0x86)
					{
						return ISO14443A_FILE_ALREADY_EXIST;//�ļ��Ѿ�����
					}
					else
					{
						return ISO14443A_CreatKeyFile_ERR;
					}
				}
		}
		else
		{
			#if DEBUG_CPU
			debug ( "send ISO14443A_CreatKeyFile cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}


uint8_t ISO14443A_CreateKeyFile (CPU_CREATE_KEY_FILE_DATA_T *create_cmd)
{
    uint8_t	result, r_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* carea_key_file;

    if ( create_cmd == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
		
	send_recv_buf[0] = CPU_PCB();
	send_recv_buf[1] = 0x01;
    carea_key_file = ( CPU_CMD_T* ) &send_recv_buf[2];
    carea_key_file->CLA = 0x80;
    carea_key_file->INS = 0xe0;
    carea_key_file->P1 = create_cmd->file_id[0];
    carea_key_file->P2 = create_cmd->file_id[1];
    carea_key_file->LC = 0x07;
	memcpy(carea_key_file->data_buff,&create_cmd->file_type,carea_key_file->LC);
	
//    carea_key_file->data_buff[0] = 0x3f;     //�ļ�����
//    carea_key_file->data_buff[1] = *size;    //�ļ��ռ�
//    carea_key_file->data_buff[2] = * ( size + 1 );
//    carea_key_file->data_buff[3] = 0x95;     //�̱�ʶ��
//    carea_key_file->data_buff[4] = 0xf0;     //������ԿȨ�ޣ��������Ϊ����������
//    carea_key_file->data_buff[5] = 0xff;
//    carea_key_file->data_buff[6] = 0xff;
	#if DEBUG_CPU
	debug ( "send CreatKeyFile cmd:" );
	debug_hex ( send_recv_buf, carea_key_file->LC+7 );
	#endif
		SetTime_delay(50);
    result = cmd_send_recv ( carea_key_file->LC+7, send_recv_buf, &r_len, send_recv_buf );
		
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_CreatKeyFile,send_recv_buf,r_len) == ISO14443A_SUCCESS )
			{
				#if DEBUG_CPU
				debug ( "get CreatKeyFile ask (len = %d):\r\n", r_len);
				debug_hex ( send_recv_buf, r_len );
				debug("CreatKeyFile success!\r\n");
				#endif
				return ISO14443A_SUCCESS;
			}
			else
			{
				#if DEBUG_CPU
				debug ( "ISO14443A_CreatKeyFile ask error,error code:!\r\n" );
				debug_hex(send_recv_buf,r_len);
			    #endif
				if(send_recv_buf[2] == 0x6a && send_recv_buf[3] == 0x86)
				{
					return ISO14443A_FILE_ALREADY_EXIST;//�ļ��Ѿ�����
				}
				else
				{
					return ISO14443A_CreatKeyFile_ERR;
				}
			}
		}
		else
		{
			#if DEBUG_CPU
			debug ( "send ISO14443A_CreatKeyFile cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}
/******************************************************************************
* Name: 	 AddKey
*
* Desc: 	 ����Կ�ļ������Կ
* Param(in):  key_type����Կ���ͣ�key_cnt����Կ�ţ�key_len����Կ���ȣ�key����Կ
* Param(out):
* Return:
* Global:  ��Կ�ļ�Ҫ���´�������Ȼ�ᵼ��д��Կʧ�ܡ�
* Note:			�ⲿ��֤��ef����Կ��erд��Կ��key_typeΪ0x39��
						�ڲ���֤��key_typeΪ0x30.
						
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/09, Create this function by liuwq
*******************************************************************************/
uint8_t AddKey ( uint8_t key_type, uint8_t key_cnt, uint8_t key_len, uint8_t* key )
{
    uint8_t	result, r_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* add_key;

    if ( key == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
		send_recv_buf[0] = CPU_PCB();
		send_recv_buf[1] = 0x01;
    add_key = ( CPU_CMD_T* ) &send_recv_buf[2];
    add_key->CLA = 0x80;
    add_key->INS = 0xd4;
    add_key->P1 = 0x01;
    add_key->P2 = key_cnt;
    add_key->LC = 0x15;
    add_key->data_buff[0] = key_type;
    add_key->data_buff[1] = 0xf0;
    add_key->data_buff[2] = 0xf0;
    add_key->data_buff[3] = 0xaa;
    add_key->data_buff[4] = 0xee;
    memcpy ( &add_key->data_buff[5], key, 16 );
		#if DEBUG_CPU
		debug ( "send AddKey cmd:" );
		debug_hex ( send_recv_buf, 28 );
		#endif
		SetTime_delay(50);
    result = cmd_send_recv ( 28, send_recv_buf, &r_len, send_recv_buf );
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_AddKey,send_recv_buf,r_len) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "get AddKey ask (len = %d):\r\n", r_len);
					debug_hex ( send_recv_buf, r_len );
					debug("AddKey success!\r\n");
					#endif
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_AddKey ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,r_len);
				  #endif
					if(send_recv_buf[2] == 0x6a && send_recv_buf[3] == 0x86)
					{
						return ISO14443A_FILE_ALREADY_EXIST;//��Կ�Ѿ�����
					}
					else
					{
					  return ISO14443A_AddKey_ERR;
					}
				}
		}
		else
		{
			#if DEBUG_CPU
			debug ( "send ISO14443A_AddKey cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}

uint8_t ISO14443A_Addkey ( CPU_CREATE_KEY_DATA_T *add_cmd )
{
	uint8_t	result, r_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* add_key;

    if ( add_cmd == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
		send_recv_buf[0] = CPU_PCB();
		send_recv_buf[1] = 0x01;
    add_key = ( CPU_CMD_T* ) &send_recv_buf[2];
    add_key->CLA = 0x80;
    add_key->INS = 0xd4;
    add_key->P1 = 0x01;
    add_key->P2 = add_cmd->key_cnt;
    add_key->LC = add_cmd->key_len+5;
	memcpy(add_key->data_buff,&add_cmd->key_type,add_key->LC);
//    add_key->data_buff[0] = key_type;
//    add_key->data_buff[1] = 0xf0;
//    add_key->data_buff[2] = 0xf0;
//    add_key->data_buff[3] = 0xaa;
//    add_key->data_buff[4] = 0xee;
//    memcpy ( &add_key->data_buff[5], key, 16 );
		#if DEBUG_CPU
		debug ( "send AddKey cmd:" );
		debug_hex ( send_recv_buf, add_key->LC +7 );
		#endif
		SetTime_delay(50);
    result = cmd_send_recv ( add_key->LC +7, send_recv_buf, &r_len, send_recv_buf );
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_AddKey,send_recv_buf,r_len) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "get AddKey ask (len = %d):\r\n", r_len);
					debug_hex ( send_recv_buf, r_len );
					debug("AddKey success!\r\n");
					#endif
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_AddKey ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,r_len);
				  #endif
					if(send_recv_buf[2] == 0x6a && send_recv_buf[3] == 0x86)
					{
						return ISO14443A_FILE_ALREADY_EXIST;//��Կ�Ѿ�����
					}
					else
					{
					  return ISO14443A_AddKey_ERR;
					}
				}
		}
		else
		{
			#if DEBUG_CPU
			debug ( "send ISO14443A_AddKey cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}
/******************************************************************************
* Name: 	 CreatEFFile
*
* Desc: 	 ����ef�ļ�
* Param(in):file_id���ļ�id��tiem_cnt����Ŀ�ܸ�����item_len��ÿ����Ŀ���ȣ�rw_key����ԿȨ������
* Param(out):
* Return:
* Global:
* Note:				
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/09, Create this function by liuwq
*******************************************************************************/
uint8_t CreatEFFile ( uint8_t* file_id, uint8_t tiem_cnt, uint8_t item_len, uint8_t rw_key )
{
    uint8_t	result, r_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* creat_ef_file;

    if ( file_id == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
		send_recv_buf[0] = CPU_PCB();
		send_recv_buf[1] = 0x01;
    creat_ef_file = ( CPU_CMD_T* ) &send_recv_buf[2];
    creat_ef_file->CLA = 0x80;
    creat_ef_file->INS = 0xe0;
    creat_ef_file->P1 = *file_id;
    creat_ef_file->P2 = * ( file_id + 1 );
    creat_ef_file->LC = 0x07;
    creat_ef_file->data_buff[0] = 0x2a;    //�ļ�����
    creat_ef_file->data_buff[1] = tiem_cnt;//��¼���ܸ���
    creat_ef_file->data_buff[2] = item_len;//ÿ����¼�ĳ���
    creat_ef_file->data_buff[3] = 0xf1;    //��Ȩ��
    creat_ef_file->data_buff[4] = 0xf1;    //дȨ��
    creat_ef_file->data_buff[5] = 0xff;    //����
    creat_ef_file->data_buff[6] = rw_key;  //��д��Կ�ţ���0x91��ʾ����Կ01��д��Կ02
		
		#if DEBUG_CPU
		debug ( "send CreatEFFile cmd:" );
		debug_hex ( send_recv_buf, 14 );
		#endif
		SetTime_delay(100);
    result = cmd_send_recv ( 14, send_recv_buf, &r_len, send_recv_buf );
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_CreatEFFile,send_recv_buf,r_len) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "get CreatEFFile ask (len = %d):\r\n", r_len);
					debug_hex ( send_recv_buf, r_len );
					debug("CreatEFFile success!\r\n");
					#endif
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_CreatEFFile ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,r_len);
				  #endif
					if(send_recv_buf[2] == 0x6a && send_recv_buf[3] == 0x86)
					{
						return ISO14443A_FILE_ALREADY_EXIST;//�Ѿ�����
					}
					else
					{
					  return ISO14443A_CreatEFFile_ERR;
					}
				}
		}
		else
		{
			#if DEBUG_CPU
			debug ( "send ISO14443A_CreatEFFile cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}

/******************************************************************************
* Name: 	 WriteEF
*
* Desc: 	 д��¼�ļ�(����)
* Param(in):  item_id:�ڼ�����¼��item_len����¼�ĳ��ȣ�data��д�������
* Param(out):
* Return:
* Global:
* Note: 	 ��Կ�ļ���Ҫ����д��дef��Կ��Ȼ�����дef��Կ�ⲿ��֤
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/09, Create this function by liuwq
*******************************************************************************/
uint8_t WriteEF ( uint8_t item_id, uint8_t item_len, uint8_t* data )
{
    uint8_t	result, r_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* write_ef;

    if ( data == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
		send_recv_buf[0] = CPU_PCB();
		send_recv_buf[1] = 0x01;
    write_ef = ( CPU_CMD_T* ) &send_recv_buf[2];
    write_ef->CLA = 0x00;
    write_ef->INS = 0xdc;
    write_ef->P1 = item_id;
    write_ef->P2 = 04;//ָ��ʹ��P1�еļ�¼��
    write_ef->LC = item_len;
    memcpy ( write_ef->data_buff, data, item_len );
		#if DEBUG_CPU
		debug ( "send WriteEF cmd:" );
		debug_hex ( send_recv_buf, 7 + item_len );
		#endif
		SetTime_delay(100);
    result = cmd_send_recv ( 7 + item_len, send_recv_buf, &r_len, send_recv_buf );
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_WriteEF,send_recv_buf,r_len) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "get WriteEF ask (len = %d):\r\n", r_len);
					debug_hex ( send_recv_buf, r_len );
					debug("WriteEF(id = %x) success!\r\n",item_id);
					#endif
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_WriteEF ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,r_len);
				  #endif
					return ISO14443A_WriteEF_ERR;
				}
		}
		else
		{
			#if DEBUG_CPU
			debug ( "send ISO14443A_WriteEF cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}

/******************************************************************************
* Name: 	 ReadEF
*
* Desc: 	 ����¼�ļ�(����)
* Param(in): 	 item_id:�ڼ�����¼��item_len����¼�ĳ���
* Param(out):   data������������
* Return:
* Global:
* Note: 	 ��Կ�ļ���Ҫ����д���ef��Կ��Ȼ����ж�ef��Կ�ⲿ��֤
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/09, Create this function by liuwq
*******************************************************************************/
uint8_t ReadEF ( uint8_t item_id, uint8_t item_len, uint8_t* data )
{
    uint8_t	result, r_len = 0,send_recv_buf[FIFO_BUFF_MAX];
    CPU_CMD_T* read_ef;

    if ( data == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
		send_recv_buf[0] = CPU_PCB();
		send_recv_buf[1] = 0x01;
    read_ef = ( CPU_CMD_T* ) &send_recv_buf[2];
    read_ef->CLA = 0x00;
    read_ef->INS = 0xb2;
    read_ef->P1 = item_id;
    read_ef->P2 = 04;
    read_ef->LC = item_len;
		#if DEBUG_CPU
		debug ( "send ReadEF cmd:" );
		debug_hex ( send_recv_buf, 7 );
		#endif
		SetTime_delay(50);
    result = cmd_send_recv ( 7, send_recv_buf, &r_len, send_recv_buf );
		if(result == ISO14443A_SUCCESS)
		{
			if ( iso14443A_cpu_ask_check(ISO14443A_ReadEF,send_recv_buf,r_len) == ISO14443A_SUCCESS )
        {
					#if DEBUG_CPU
					debug ( "get ReadEF ask (len = %d):\r\n", r_len);
					debug_hex ( send_recv_buf, r_len );
					debug("ReadEF(id = %x) success!\r\n",item_id);
					#endif
					memcpy ( data, &send_recv_buf[2], r_len - 4 );
					return ISO14443A_SUCCESS;
        }
				else
				{
					#if DEBUG_CPU
					debug ( "ISO14443A_ReadEF ask error,error code:!\r\n" );
					debug_hex(send_recv_buf,r_len);
				  #endif
					return ISO14443A_WriteEF_ERR;
				}
		}
		else
		{
			#if DEBUG_CPU
			debug ( "send ISO14443A_ReadEF cmd error!\r\n" );
			#endif
		}
    return ISO14443A_SEND_CMD_ERROR;
}
/*---------------------------------------------------------------------------*/
