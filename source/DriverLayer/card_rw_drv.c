/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName:      card_rw_drv.c
* Desc:
*
*
* Author:    liuwq
* Date:      2017/10/11
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2017/10/11, liuwq create this file
*
******************************************************************************/


/*-------------------------------- Includes ----------------------------------*/
#include "opt.h"
#include "card_rw_drv.h"
#include "card_encrypt.h"
#include "lib_iso14443Apcd.h"
#include <string.h>
#include "debug.h"
#include "des.h"
/*----------------------- Constant / Macro Definitions -----------------------*/
#define STEP_MAX 16
#define RETRY_MAX_TIMES 2
//判断块是用户数据块还是密码块
#define block_judge(block_cn)     ((((block_cn+1)&0x03)==0)||(block_cn==0))?DATA_KEY:DATA_USER
#define EACH_ITEM_MAX_LEN 0x20
//立林科技默认的cpu卡参数/////////////////////////////////////////
uint8_t const  LEELEN_MF[2]            = {0x3F, 0x00}; //leelen主文件夹id
uint8_t const  LEELEN_ADF[2]           = {0x10, 0x05}; //leelen应用文件夹id
uint8_t const  LEELEN_ADF_SIZE[2]      = {0X04, 0x00}; //文件夹总大小1KB
uint8_t const  LEELEN_KEY_FILE[2]      = {0x00, 0x01}; //密钥文件id不能为0，不然选中的时候应答错误
uint8_t const  LEELEN_KEY_FILE_SIZE[2] = {0X00, 0xa0}; //密钥文件大小10个密钥，每个密钥长度为16
uint8_t const  LEELEN_EF[2]            = {0x00, 0x02}; //EF文件id
uint8_t const  LEELEN_EX_KEY_CN        = 0x03;         //外部密钥号
uint8_t const  LEELEN_IN_KEY_CN        = 0x05;         //内部密钥号
uint8_t const  LEELEN_EF_RKEY_CN       = 0x01;         //读EF密钥号
uint8_t const  LEELEN_EF_WEY_CN        = 0x02;         //写EF密钥号
uint8_t const  LEELEN_KEY_LEN          = 0x10;         //密钥长度
uint8_t const  LEELEN_READ_KEY_CN      = 0x01;
uint8_t const  LEELEN_WRITE_KEY_CN     = 0x02;
uint8_t const  LEELEN_EF_TYPE[2]       = {0x2A, EACH_ITEM_MAX_LEN}; //记录类型0x2A，长度EACH_ITEM_MAX_LEN即32字节
uint8_t const  LEELEN_EF_ITEM_CNT      = 0x0a;         //定长记录的总个数
uint8_t const  LEELEN_EF_RW_KEY        = 0x91;         //读写密钥号
uint8_t const  LEELEN_EF_DATA_LEN      = 250;          //可写的记录文件总大小

///////////////////////////////////////////////////////////////////
#define ExAuthenticate_KEY_TYPE 0x39 //外部认证密钥类型
#define	InAuthenticate_KEY_TYPE 0x30 //内部认证密钥类型
#define	ReadEF_KEY_TYPE         0x39 //读EF密钥类型
#define	WriteEF_KEY_TYPE        0x39 //写EF密钥类型


/*------------------------ Variable Declarations -----------------------------*/
static CARD_RW_CONFIG  card_rw_config;

/*------------------------ Function Prototype --------------------------------*/




/*------------------------ Function Implement --------------------------------*/
/******************************************************************************
* Name:      cpu_card_default_init
*
* Desc:      发CPU卡默认参数初始化（必须传入uid）
* Param(in):
* Param(out):
* Return:
* Global:
* Note:      需要传入卡号
* Author:    liuwq
*------------------------------------------------------------------------------
* Log:   2017/11/13, Create this function by liuwq
*******************************************************************************/
uint8_t cpu_card_default_init ( CPU_SENDER_CARD_T* def_param )
{
    uint8_t index = 0;
    CPU_SENDER_CARD_T* cpu_para = NULL;

    if ( def_param == NULL )
    {
        return 1;
    }
    cpu_para = ( CPU_SENDER_CARD_T* ) def_param;
//	  scan_ISO14443A_card ( cpu_para->uid );由检测到卡事件时传入
    cpu_para->mf[0] = LEELEN_MF[0];
    cpu_para->mf[1] = LEELEN_MF[1];
    cpu_para->adf[0] = LEELEN_ADF[0];
    cpu_para->adf[1] = LEELEN_ADF[1];
    cpu_para->adf_size[0] = LEELEN_ADF_SIZE[0];
    cpu_para->adf_size[1] = LEELEN_ADF_SIZE[1];
    cpu_para->key_file[0] = LEELEN_KEY_FILE[0];
    cpu_para->key_file[1] = LEELEN_KEY_FILE[1];
    cpu_para->key_file_size[0] = LEELEN_KEY_FILE_SIZE[0];
    cpu_para->key_file_size[1] = LEELEN_KEY_FILE_SIZE[1];
		
    cpu_para->key_type[ExAuthenticate_KEY_INDEX] = ExAuthenticate_KEY_TYPE;
    cpu_para->key_type[InAuthenticate_KEY_INDEX] = InAuthenticate_KEY_TYPE;
    cpu_para->key_type[ReadEF_KEY_INDEX] = ReadEF_KEY_TYPE;
    cpu_para->key_type[WriteEF_KEY_INDEX] = WriteEF_KEY_TYPE;
		cpu_para->key_type[MF_ExAuthenticate_KEY_INDEX] = ExAuthenticate_KEY_TYPE;
		cpu_para->key_type[MF_InAuthenticate_KEY_INDEX] = InAuthenticate_KEY_TYPE;
		
    cpu_para->key_cnt[ExAuthenticate_KEY_INDEX] = LEELEN_EX_KEY_CN;
    cpu_para->key_cnt[InAuthenticate_KEY_INDEX] = LEELEN_IN_KEY_CN;
    cpu_para->key_cnt[ReadEF_KEY_INDEX] = LEELEN_EF_RKEY_CN;
    cpu_para->key_cnt[WriteEF_KEY_INDEX] = LEELEN_EF_WEY_CN;
	cpu_para->key_cnt[MF_ExAuthenticate_KEY_INDEX] = LEELEN_EX_KEY_CN;
    cpu_para->key_cnt[MF_InAuthenticate_KEY_INDEX] = LEELEN_IN_KEY_CN;
    for ( index = 0; index < CPU_KEY_INDEX_MAX; index++ ) //获取密钥
    {
		if(index == MF_ExAuthenticate_KEY_INDEX || index == MF_InAuthenticate_KEY_INDEX)
		{
			gen_key_cpu ( cpu_para->uid, cpu_para->mf, cpu_para->key_cnt[index], card_rw_config.prj_num, &cpu_para->key[16 * index] );
		}
		else
		{
			gen_key_cpu ( cpu_para->uid, cpu_para->adf, cpu_para->key_cnt[index], card_rw_config.prj_num, &cpu_para->key[16 * index] );
		}
    }
    cpu_para->ef[0] = LEELEN_EF[0];
    cpu_para->ef[1] = LEELEN_EF[1];
    cpu_para->ef_type[0] = LEELEN_EF_TYPE[0];
    cpu_para->ef_type[1] = LEELEN_EF_TYPE[1];
    cpu_para->ef_item_cnt = LEELEN_EF_ITEM_CNT;
    cpu_para->ef_rw_key   = LEELEN_EF_RW_KEY;
    return 0;
}
//测试用
#if DEBUG_CPU_SENDER
void print_cpu_card_info ( CPU_SENDER_CARD_T* card_sender )
{
    uint8_t index;
	
    debug ( "uid:" );
    debug_hex ( card_sender->uid, 4 );
    debug ( "adf:" );
    debug_hex ( card_sender->adf, 2 );
    debug ( "adf size:" );
    debug_hex ( card_sender->adf_size, 2 );
    debug ( "key_file:" );
    debug_hex ( card_sender->key_file, 2 );
    debug ( "key_file_size:" );
    debug_hex ( card_sender->key_file_size, 2 );
    debug ( "ef:" );
    debug_hex ( card_sender->ef, 2 );
    debug ( "ef_type:" );
    debug_hex ( card_sender->ef_type, 2 );
    debug ( "ef_item_cnt:%x\r\n", card_sender->ef_item_cnt );
    debug ( "ef_rw_key:%x\r\n", card_sender->ef_rw_key );
		
    for ( index = 0; index < CPU_KEY_INDEX_MAX; index++ )
    {
			debug ("NO.%d key info:\r\n",index);
			debug ( "key_type(%x),",card_sender->key_type[index] );
			debug ( "key_cnt(%x)\r\n", card_sender->key_cnt[index] );
			debug_hex ( &card_sender->key[16 * index], 16 );
    }
}
#endif
/******************************************************************************
* Name:      card_ic_select
*
* Desc:      判断所读到的卡是否为期望选择的卡
* Param(in):     期望选择的卡的uid
* Param(out):
* Return:    0->成功
* Global:
* Note:
* Author:    liuwq
*------------------------------------------------------------------------------
* Log:   2017/10/10, Create this function by liuwq
*******************************************************************************/
static uint8_t card_ic_select ( uint8_t* card_uid )
{
    uint8_t status;
    uint8_t s_card_uid[11];//用来保存pcd扫描得到的卡号

    status = scan_ISO14443A_card ( s_card_uid );
    if ( status != ISO14443A_SUCCESSCODE )
    {
        status = scan_ISO14443A_card ( s_card_uid );
    }
    if ( status != ISO14443A_SUCCESSCODE )
    {
        return CARD_NONE_ERR;
    }
    //将pcd扫描得到的卡号与期望选择的卡号对比
    if ( memcmp ( s_card_uid, card_uid, 4 ) )
    {
        return CARD_ID_ERR;
    }
    return ISO14443A_SUCCESSCODE;
}
/******************************************************************************
* Name:      card_ic_rw_block
*
* Desc:      IC卡块读写
* Param(in):     date_buff
* Param(out):
* Return:    1->成功，2->失败
* Global:
* Note:      扇区读写权限:0x78,0x77,0x88,0x69,keyA只能读,keyB可读写
* Author:    liuwq
*------------------------------------------------------------------------------
* Log:   2017/10/10, Create this function by liuwq
*******************************************************************************/
uint8_t card_ic_rw_block ( IC_BLOCK_RW_STR* date_buff, CARD_DATA_TYPE date_type )
{
    uint8_t step = 1;
    IC_BLOCK_RW_STR* ic_block_info;

    ic_block_info = ( IC_BLOCK_RW_STR* ) date_buff;
    ic_block_info->result = IC_SUCCESS;
    ic_block_info->result_expand = 0;
    for ( step = 1; step < STEP_MAX; step++ )
    {
        switch ( step )
        {
        case 1 :/* 选择IC卡  */
            ic_block_info->result_expand = card_ic_select ( ic_block_info->uid );
            if ( ic_block_info->result_expand != ISO14443A_SUCCESSCODE )
            {
                step = STEP_MAX;
                ic_block_info->result = IC_FAIL;
            }
            break;
        case 2 :/* 获取密钥 */
            if ( ( ic_block_info->key_cn != ISO14443A_CMD_AUTH_A ) && ( ic_block_info->key_cn != ISO14443A_CMD_AUTH_B ) ) //无密钥号，使用立林默认密钥
            {
                if ( ic_block_info->cmd_aux == IC_BLOCK_READ )
                {
                    ic_block_info->key_cn = 0x00;  //读操作，立林默认为KEYA认证
                    gen_key_ic ( ic_block_info->key_cn, ic_block_info->uid, ic_block_info->block_cn >> 2, card_rw_config.prj_num, ic_block_info->key ); //计算卡的扇区密钥
                    ic_block_info->key_cn = ISO14443A_CMD_AUTH_A;
                }
                else if ( ic_block_info->cmd_aux == IC_BLOCK_WRITE )
                {
                    ic_block_info->key_cn = 0x01;  //写操作，立林默认为KEYB认证
                    gen_key_ic ( ic_block_info->key_cn, ic_block_info->uid, ic_block_info->block_cn >> 2, card_rw_config.prj_num, ic_block_info->key ); //计算卡的扇区密钥
                    ic_block_info->key_cn = ISO14443A_CMD_AUTH_B;
                }
            }
            break;
        case 3 :/* 密钥认证  */
            if ( ISO14443A_Authen ( ic_block_info->uid, ic_block_info->block_cn, ic_block_info->key, ic_block_info->key_cn ) )
            {
                step = STEP_MAX;
                ic_block_info->result = IC_FAIL;
                ic_block_info->result_expand = IC_AUTH_ERR;
            }
            break;
        case 4 :
            if ( ic_block_info->cmd_aux == IC_BLOCK_READ ) /* 读块步骤  */
            {
                memset ( ic_block_info->block_buffer, 0, 16 );
                if ( ISO14443A_Read ( ic_block_info->block_cn, ic_block_info->block_buffer ) )
                {
                    step = STEP_MAX;
                    ic_block_info->result = IC_FAIL;
                    ic_block_info->result_expand = BLOCK_ERR;
                }
            }
            else if ( ic_block_info->cmd_aux == IC_BLOCK_WRITE ) /* 写块步骤  */
            {
                if ( date_type != block_judge ( ic_block_info->block_cn ) )
                {
                    ic_block_info->result = IC_FAIL;
                    ic_block_info->result_expand = BLOCK_ERR;
                    break;
                }
                if ( ISO14443A_Write ( ic_block_info->block_cn, ic_block_info->block_buffer ) )
                {
                    step = STEP_MAX;
                    ic_block_info->result = IC_FAIL;
                    ic_block_info->result_expand = BLOCK_ERR;
                }
            }
            break;
        case 5 : /* halt卡  */
            //ISO14443A_Halt();
            break;
        default:
            break;
        }
    }
    return ic_block_info->result;
}
/******************************************************************************
* Name:      cpu_internal_auth
*
* Desc:      CPU卡内部认证
* Param(in):
* Param(out):
* Return:    IC_SUCCESS->成功
* Global:
* Note:      可使用默认参数进行验证，需要传入卡号，ADF置为0
* Author:    liuwq
*------------------------------------------------------------------------------
* Log:   2017/11/08, Create this function by liuwq
*******************************************************************************/
uint8_t cpu_internal_auth ( CPU_IN_AUTH_T* date_buff )
{
    CPU_IN_AUTH_T* in_auth;
    uint8_t mf[2] = {0x3f, 0x00};
    uint8_t step, rlen, des_in[8], des_out[8], random[8]; //因为使用des会改变des_in的值，所以定义临时数组random

    in_auth = ( CPU_IN_AUTH_T* ) date_buff;
    for ( step = 1; step <= ( STEP_MAX + 1 ); step++ )
    {
        in_auth->result = IC_FAIL;
        switch ( step )
        {
        case 1://选中卡
            in_auth->result = card_ic_select ( in_auth->uid );
            if ( in_auth->result )
            {
                step = STEP_MAX;
                in_auth->result = IC_FAIL;
                in_auth->result_expand = CARD_ID_ERR;
            }
            break;

        case 2://进入MF文件夹
            Rats();
            in_auth->result = SelectFile ( mf );
            if ( in_auth->result )
            {
                step = STEP_MAX;
                in_auth->result = IC_FAIL;
                in_auth->result_expand = MF_ERR;
            }
            break;

        case 3://进入ADF文件夹
            /* ADF==00 00 所有参数使用立林的参数*/
            if ( ( in_auth->adf[0] == 0 ) && ( in_auth->adf[1] == 0 ) )
            {
                in_auth->adf[0]  = LEELEN_ADF[0];
                in_auth->adf[1]  = LEELEN_ADF[1];
                in_auth->key_cn  = LEELEN_IN_KEY_CN;
                in_auth->key_len = LEELEN_KEY_LEN;
                gen_key_cpu ( in_auth->uid, in_auth->adf, in_auth->key_cn, card_rw_config.prj_num, in_auth->key );
            }

            in_auth->result = SelectFile ( in_auth->adf );
            if ( in_auth->result )
            {
                step = STEP_MAX;
                in_auth->result = IC_FAIL;
                in_auth->result_expand = ADF_ERR;
            }
            break;

        case 4://内部认证
            if ( in_auth->key_len == 0x10 )
            {
                in_auth->result = GetRandom ( &rlen, random );
                if ( in_auth->result )
                {
                    step = STEP_MAX;
                    in_auth->result = IC_FAIL;
                    in_auth->result_expand = IN_AUTH_ERR;
                    break;
                }
                memcpy ( des_in, random, 8 );
                TDES ( random, des_out, in_auth->key, ENCRY ); //本地进行加密计算
                in_auth->result = InAuthenticate ( des_in, in_auth->key_cn, des_out ); //内部认证
                if ( in_auth->result )
                {
                    step = STEP_MAX;
                    in_auth->result = IC_FAIL;
                    in_auth->result_expand = IN_AUTH_ERR;
                }
            }
            else if ( in_auth->key_len == 0x08 )
            {
                in_auth->result = GetRandom ( &rlen, random );
                if ( in_auth->result )
                {
                    step = STEP_MAX;
                    in_auth->result = IC_FAIL;
                    in_auth->result_expand = IN_AUTH_ERR;
                    break;
                }
                memcpy ( des_in, random, 8 );
                DES ( random, des_out, in_auth->key, ENCRY );
                in_auth->result = InAuthenticate ( des_in, in_auth->key_cn, des_out ); //内部认证
                if ( in_auth->result )
                {
                    step = STEP_MAX;
                    in_auth->result = IC_FAIL;
                    in_auth->result_expand = IN_AUTH_ERR;
                }
            }
            else {}
            break;

        case 5://停卡
                in_auth->result = ISO14443A_Halt();
                if(in_auth->result)
                {
                    step = STEP_MAX;
                }
                else//到此，内部认证成功
								{
										step = STEP_MAX + 1;
										in_auth->result = IC_SUCCESS;
										in_auth->result_expand = 0;
								}
        break;

        default:
            ISO14443A_Halt();
            break;
        }
    }
    return in_auth->result;
}
/******************************************************************************
* Name:      card_cpu_rw_ef
*
* Desc:      读写CPU卡记录文件，最多支持200字节
* Param(in):
* Param(out):
* Return:
* Global:		若使用leelen默认参数，只需传入卡号和adf：0x00，0x00
* Note:      条目编号从1开始
* Author:    liuwq
*------------------------------------------------------------------------------
* Log:   2017/11/10, Create this function by liuwq
*******************************************************************************/
uint8_t card_cpu_rw_ef ( CPU_RW_T* data_buf )
{
    uint8_t mf[2] = {0x3f, 0x00};
    uint8_t i, temp, step;
    CPU_RW_T* cpu_rw_info;

    cpu_rw_info = ( CPU_RW_T* ) data_buf;
    if ( ( cpu_rw_info == NULL ) || ( cpu_rw_info->len > CPU_RWBUF_MAX ) )
    {
        cpu_rw_info->result = IC_FAIL;
        cpu_rw_info->result_expand = CARD_ID_ERR;
        return IC_FAIL;
    }
    for ( step = 1; step <= ( STEP_MAX + 1 ); step++ )
    {
        cpu_rw_info->result = IC_FAIL;
        switch ( step )
        {
        case 1 :/*选择IC卡*/
            cpu_rw_info->result = card_ic_select ( cpu_rw_info->uid );
            if ( cpu_rw_info->result )
            {
                step = STEP_MAX;
                cpu_rw_info->result = IC_FAIL;
                cpu_rw_info->result_expand = CARD_ID_ERR;
            }
            break;

        case 2://选择MF文件夹
            cpu_rw_info->result = Rats();
            if ( cpu_rw_info->result )
            {
                step = STEP_MAX;
                cpu_rw_info->result = IC_FAIL;
                cpu_rw_info->result_expand = MF_ERR;
            }
            cpu_rw_info->result = SelectFile ( mf );
            if ( cpu_rw_info->result )
            {
								cpu_rw_info->result = SelectFile ( mf );
								if ( cpu_rw_info->result )
								{
									step = STEP_MAX;
									cpu_rw_info->result = IC_FAIL;
									cpu_rw_info->result_expand = MF_ERR;
								}
            }
            break;

        case 3://选择adf
            /* ADF==00 00 所有参数使用立林的参数*/
            if ( ( cpu_rw_info->adf[0] == 0 ) && ( cpu_rw_info->adf[1] == 0 ) )
            {
                cpu_rw_info->adf[0]  = LEELEN_ADF[0];
                cpu_rw_info->adf[1]  = LEELEN_ADF[1];
                cpu_rw_info->ef[0] = LEELEN_EF[0];
                cpu_rw_info->ef[1] = LEELEN_EF[1];
                cpu_rw_info->ef_type[0] = LEELEN_EF_TYPE[0];
                cpu_rw_info->ef_type[1] = LEELEN_EF_TYPE[1];
                cpu_rw_info->addr_start = 1;
                if ( cpu_rw_info->cmd_aux == CPU_EF_WRITE )
                {
                    cpu_rw_info->key_cn  = LEELEN_EF_WEY_CN;
                }
                else if ( cpu_rw_info->cmd_aux == CPU_EF_READ )
                {
                    cpu_rw_info->key_cn  = LEELEN_EF_RKEY_CN;
                }
                else
                {
                    cpu_rw_info->key_cn  = LEELEN_EF_RKEY_CN;
                }
                cpu_rw_info->key_len = LEELEN_KEY_LEN;
                gen_key_cpu ( cpu_rw_info->uid, cpu_rw_info->adf, cpu_rw_info->key_cn, card_rw_config.prj_num, cpu_rw_info->key );
            }
            cpu_rw_info->result = SelectFile ( cpu_rw_info->adf );
            if ( cpu_rw_info->result )
            {
                step = STEP_MAX;
                cpu_rw_info->result = IC_FAIL;
                cpu_rw_info->result_expand = ADF_ERR;
            }
            break;

        case 4://外部认证读/写密码
            cpu_rw_info->result = ExAuthenticate ( cpu_rw_info->key_cn, cpu_rw_info->key );
            if ( cpu_rw_info->result )
            {
                step = STEP_MAX;
                cpu_rw_info->result = IC_FAIL;
                cpu_rw_info->result_expand = EX_AUTH_ERR;
            }
            break;

        case 5://选择EF文件
            cpu_rw_info->result = SelectFile ( cpu_rw_info->ef );
            if ( cpu_rw_info->result )
            {
                step = STEP_MAX;
                cpu_rw_info->result = IC_FAIL;
                cpu_rw_info->result_expand = EF_ERR;
            }
            break;

        case 6://读/写EF文件
            if ( cpu_rw_info->ef_type[0] == EF_TYPE_FIXED_RECORD ) //定长记录文件
            {
                if ( cpu_rw_info->ef_type[1] < 1 ) //每条记录的长度小于1
                {
                    step = STEP_MAX;
                    cpu_rw_info->result = IC_FAIL;
                    cpu_rw_info->result_expand = EF_OPERATE_ERR;
                    break;
                }
                temp = cpu_rw_info->len / cpu_rw_info->ef_type[1];
                if ( temp ) //读/写的长度超过单条记录的长度，分多条读/写
                {
                    for ( i = 0; i < temp; i++ )//完整读出整条条目数据
                    {
                        if ( cpu_rw_info->cmd_aux == CPU_EF_WRITE )
                        {
                            cpu_rw_info->result = WriteEF ( i + cpu_rw_info->addr_start, cpu_rw_info->ef_type[1], &cpu_rw_info->data[cpu_rw_info->ef_type[1] * ( i )] );
                        }
                        else if ( cpu_rw_info->cmd_aux == CPU_EF_READ )
                        {
                            cpu_rw_info->result = ReadEF ( i + cpu_rw_info->addr_start, cpu_rw_info->ef_type[1], &cpu_rw_info->data[cpu_rw_info->ef_type[1] * ( i )] );
                        }
                        if ( cpu_rw_info->result )
                        {
                            step = STEP_MAX;
                            cpu_rw_info->result = IC_FAIL;
                            cpu_rw_info->result_expand = EF_OPERATE_ERR;
                            break;
                        }
                    }
                    temp = cpu_rw_info->len % cpu_rw_info->ef_type[1]; //判断剩余不足单条记录的字节数
                    if ( temp == 0 )
                    {
                        break;
                    }
                    if ( cpu_rw_info->cmd_aux == CPU_EF_WRITE )
                    {
//                            debug("leave %d data,item no.%d,data:",temp,i+cpu_rw_info->addr_start);
//                            debug_print_format(&cpu_rw_info->data[cpu_rw_info->ef_type[1]*(i)],temp,10);
                        cpu_rw_info->result = WriteEF ( i + cpu_rw_info->addr_start, temp, &cpu_rw_info->data[cpu_rw_info->ef_type[1] * ( i )] );
//                            cpu_rw_info->result = WriteEF(i+cpu_rw_info->addr_start,cpu_rw_info->ef_type[1],&cpu_rw_info->data[cpu_rw_info->ef_type[1]*(i)]);
                    }
                    else if ( cpu_rw_info->cmd_aux == CPU_EF_READ )
                    {
                        cpu_rw_info->result = ReadEF ( i + cpu_rw_info->addr_start, temp, &cpu_rw_info->data[cpu_rw_info->ef_type[1] * ( i )] );
                    }
                    if ( cpu_rw_info->result )
                    {
                        step = STEP_MAX;
                        cpu_rw_info->result = IC_FAIL;
                        cpu_rw_info->result_expand = EF_OPERATE_ERR;
                        break;
                    }
                }
                else//读/写的长度小于单条记录的长度,直接读/写
                {
                    if ( cpu_rw_info->cmd_aux == CPU_EF_WRITE )
                    {
                        cpu_rw_info->result = WriteEF ( cpu_rw_info->addr_start, cpu_rw_info->len, cpu_rw_info->data );
                    }
                    else if ( cpu_rw_info->cmd_aux == CPU_EF_READ )
                    {
                        cpu_rw_info->result = ReadEF ( cpu_rw_info->addr_start, cpu_rw_info->len, cpu_rw_info->data );
                    }
                    if ( cpu_rw_info->result )
                    {
                        step = STEP_MAX;
                        cpu_rw_info->result = IC_FAIL;
                        cpu_rw_info->result_expand = EF_OPERATE_ERR;
                    }
                }
            }
            else if ( cpu_rw_info->ef_type[0] == EF_TYPE_BIN ) //二进制文件，暂不支持
            {
                step = STEP_MAX;
                cpu_rw_info->result = IC_FAIL;
                cpu_rw_info->result_expand = EF_OPERATE_ERR;
            }
            break;

        case 7://停卡
            cpu_rw_info->result = ISO14443A_Halt();
            if ( cpu_rw_info->result )
            {
                step = STEP_MAX;
                cpu_rw_info->result = IC_FAIL;
                cpu_rw_info->result_expand = EF_OPERATE_ERR;
            }
            else
            {
                step = STEP_MAX+1;
                cpu_rw_info->result = IC_SUCCESS;
                cpu_rw_info->result_expand = 0;
            }
            break;

        default:
            ISO14443A_Halt();
            break;
        }
    }
    return cpu_rw_info->result;
}

/******************************************************************************
* Name:      cpu_card_sender
*
* Desc:      CPU发卡(用默认参数发卡，只需传入uid和adf: 00 00)
* Param(in):
* Param(out):
* Return:		0->成功,>0为错误码，详见CPU_CARD_RESULT_E
* Global:
* Note:      主目录下的密钥和ADF下的密钥是否分别存放
* Author:    liuwq
*------------------------------------------------------------------------------
* Log:   2017/11/11, Create this function by liuwq
*******************************************************************************/
uint8_t cpu_card_sender ( CPU_SENDER_CARD_T* card_info )
{
    static uint8_t error_cnt = 0;
    CPU_SENDER_CARD_T* card_sender = NULL;
    uint8_t step;

		
    if ( card_info == NULL )
    {
        return 1;
    }
		card_sender = ( CPU_SENDER_CARD_T* ) card_info;
		
		if ( card_sender->adf[0] == 0 && card_sender->adf[1] == 0 )
		{
			cpu_card_default_init(card_sender);
			#if DEBUG_CPU_SENDER
			print_cpu_card_info(card_sender);
			#endif
		}
    
    for ( step = CHECK_HAVE_CARD; step <= ( STEP_MAX + 1 ); step++ )
    {
        card_sender->result = IC_FAIL;
        switch ( step )
        {
        case CHECK_HAVE_CARD://判断卡号是否正确
				{
            //扫描卡，不是ISO14443A工作模式，自动切到ISO14443A模式
            card_sender->result = card_ic_select ( card_sender->uid );
            if ( card_sender->result )
            {
                step = CHECK_HAVE_CARD - 1;//重试
                if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //失败次数达到最大
                {
                    card_sender->result = CARD_UID_ERROR;
                    step = STEP_MAX;
                    #if DEBUG_CPU_SENDER
                    debug ( "NONE_CARD_ERROR!!!!!\r\n" );
                    #endif
                }
								break;
            }
            else
            {
                error_cnt = 0;
                #if DEBUG_CPU_SENDER
                debug ( "CHECK_HAVE_CARD success,get uid:" );
                debug_hex ( card_sender->uid, 4 );
                #endif
            }
				}
        break;

        case CHECK_CARD_TYPE://判断卡类型，并进入APDU指令模式
				{
            card_sender->result = Rats();
            if ( card_sender->result )
            {
                step = CHECK_CARD_TYPE - 1;//重试
                if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //失败次数达到最大
                {
                    card_sender->result = CARD_TYPE_ERROR;
                    step = STEP_MAX;
                    #if DEBUG_CPU_SENDER
                    debug ( "CARD_TYPE_ERROR!!!!!\r\n" );
                    #endif
                }
								break;
            }
            else
            {
                error_cnt = 0;
                #if DEBUG_CPU_SENDER
                debug ( "CHECK_CARD_TYPE success!\r\n" );
                #endif
            }
				}
        break;

        case SELECT_MF://进入主目录
				{
            card_sender->result = SelectFile ( card_sender->mf );
            if ( card_sender->result )
            {
                step = SELECT_MF - 1;//重试
                if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //失败次数达到最大
                {
                    card_sender->result = SELECT_MF_ERROR;
                    step = STEP_MAX;
                    #if DEBUG_CPU_SENDER
                    debug ( "SELECT_MF_ERROR!!!!!\r\n" );
                    #endif
                }
								break;
            }
            else
            {
                error_cnt = 0;
                #if DEBUG_CPU_SENDER
                debug ( "SelectMF success!\r\n" );
                #endif
            }
				}
        break;
				
				case DEFAULE_AUTH://用厂家默认密钥进行认证
        {
            uint8_t key_cnt = 0;
            uint8_t key_buf[16] = {0};


						gen_key_cpu ( card_sender->uid, card_sender->mf, 0x03, card_rw_config.prj_num, key_buf );
						card_sender->result = ExAuthenticate ( 3, key_buf );
						
//						memset ( key_buf, 0xff, 16 );
//            card_sender->result = ExAuthenticate ( key_cnt, key_buf );
            if ( card_sender->result )
            {
                step = DEFAULE_AUTH - 1;//重试
                if ( ( error_cnt++ ) == 1 ) //失败次数达到最大
                {
                    card_sender->result = DEFAULE_AUTH_ERROR;
                    step = IN_AUTH-1;//默认密钥认证失败，可能已经发过卡，所以进行内部认证
                    #if DEBUG_CPU_SENDER
                    debug ( "DEFAULE_AUTH_ERROR!!!!!\r\n" );
                    #endif
                }
								break;
            }
            else
            {
                error_cnt = 0;
                #if DEBUG_CPU_SENDER
                debug ( "DEFAULE_AUTH success!\r\n" );
							  debug ( "start send CPU card!\r\n" );
                #endif
							  step = DELETE_ALL-1;//跳过内部认证,直接删除所有文件
            }
        }
        break;

        case IN_AUTH://内部认证，判断是否已经发过卡
        {
            uint8_t rlen = 0,des_temp[8] = {0}, des_in[8] = {0}, des_out[8] = {0};
            uint8_t key_buf[16] = {0};

            gen_key_cpu ( card_sender->uid, card_sender->mf, card_sender->key_cnt[InAuthenticate_KEY_INDEX], card_rw_config.prj_num, key_buf );
            card_sender->result = GetRandom ( &rlen, des_temp );
            if ( card_sender->result )
            {
                step = STEP_MAX;
                card_sender->result = IC_FAIL;
                break;
            }
            memcpy ( des_in,des_temp, rlen );
            TDES ( des_temp, des_out, key_buf, ENCRY ); //本地进行加密计算
            card_sender->result = InAuthenticate ( des_in, card_sender->key_cnt[InAuthenticate_KEY_INDEX], des_out ); //内部认证
            if ( card_sender->result )
            {
                #if DEBUG_CPU_SENDER
                debug ( "MF_IN_AUTH_ERROR!!!!!\r\n" );
								debug ("card init fail\r\n");
                #endif
							  step = STEP_MAX + 1;
								card_sender->result = IN_AUTH_ERROR;
            }
            else
            {
                step = HAL_CARD - 1;
                card_sender->result = IC_SUCCESS;
                #if DEBUG_CPU_SENDER
                debug ( "CARD ALREADY INIT!!!!!\r\n" );
                #endif
            }
        }
        break;

        case DELETE_ALL:
				{
            card_sender->result = DeleteADF();
            if ( card_sender->result )
            {
                step = DELETE_ALL - 1;//重试
                if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //失败次数达到最大
                {
                    card_sender->result = DELETE_ALL_ERROR;
                    step = STEP_MAX;
                    #if DEBUG_CPU_SENDER
                    debug ( "DELETE_ALL_ERROR!!!!!\r\n" );
                    #endif
                }
								break;
            }
            else
            {
                error_cnt = 0;
                #if DEBUG_CPU_SENDER
                debug ( "DELETE_ALL success!\r\n" );
                #endif
            }
				}
        break;

        case CREAT_MF_KEY_FILE://创建主目录下的密钥文件
				{
            card_sender->result = CreatKeyFile ( card_sender->key_file, card_sender->key_file_size );
            if ( card_sender->result )
            {
                if ( card_sender->result == ISO14443A_FILE_ALREADY_EXIST ) //文件已经存在
                {
                    error_cnt = 0;
                    #if DEBUG_CPU_SENDER
                    debug ( "MF_KEY_FILE already creat!!!!!\r\n" );
                    #endif
                }
                else
                {
                    step = CREAT_MF_KEY_FILE - 1;//重试
                    if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //失败次数达到最大
                    {
                        #if DEBUG_CPU_SENDER
                        debug ( "CREAT_MF_KEY_FILE_ERROR!!!!!\r\n" );
                        #endif
                        card_sender->result = CREAT_MF_KEY_FILE_ERROR;
                        step = STEP_MAX;
                    }
										break;
                }
            }
            else
            {
                #if DEBUG_CPU_SENDER
                debug ( "CREAT_MF_KEY_FILE SUCCESS!!!!!\r\n" );
                #endif
                error_cnt = 0;
            }
//            card_sender->result = SelectFile ( card_sender->key_file );
//            if ( card_sender->result )
//            {
//                card_sender->result = SelectFile ( card_sender->key_file );
//                if ( card_sender->result )
//                {
//                    card_sender->result = CREAT_MF_KEY_FILE_ERROR;
//                    step = STEP_MAX;
//                    #if DEBUG_CPU_SENDER
//                    debug ( "SEL_KEY_FILE FAIL!!!!!\r\n" );
//                    #endif
//                }
//                else
//                {
//                    error_cnt = 0;
//                }
//            }
//            else
//            {
//								error_cnt = 0;
//                #if DEBUG_CPU_SENDER
//                debug ( "SEL_KEY_FILE success!\r\n" );
//                #endif
//            }
			  }
        break;

        case ADD_MF_KEY:
        {
            uint8_t key_index = 0;
            for ( key_index = 0; key_index < CPU_KEY_INDEX_MAX; key_index++ )
            {
								if(key_index == MF_ExAuthenticate_KEY_INDEX || key_index == MF_InAuthenticate_KEY_INDEX)//主目录只添加内外部认证密钥
								{	
			  					#if DEBUG_CPU_SENDER
									debug ( "key type:%x\r\n", card_sender->key_type[key_index] );
									#endif
									for ( error_cnt = 0; error_cnt < RETRY_MAX_TIMES; error_cnt++ ) //写成功，跳出，写失败，再重试写ERR_MAX_CNT次
									{
											card_sender->result = AddKey ( card_sender->key_type[key_index], card_sender->key_cnt[key_index],16, &card_sender->key[16 * key_index] );
											if ( card_sender->result )
											{
													if ( card_sender->result == ISO14443A_FILE_ALREADY_EXIST ) //文件已经存在
													{
															#if DEBUG_CPU_SENDER
															debug ( "key no.%x already creat!!!!!\r\n", key_index );
															#endif
															card_sender->result = ISO14443A_SUCCESS;
															break;//跳出本次写，回主循环写下一个密钥
													}
													else
													{
															#if DEBUG_CPU_SENDER
															debug ( "write key no.%x fail,retury %d times!\r\n", key_index ,error_cnt);
															#endif
													}
											}
											else
											{
													#if DEBUG_CPU_SENDER
													debug ( "write key no.%x success!\r\n", key_index );
													#endif
													break;
											}
									}
									if ( error_cnt == RETRY_MAX_TIMES ) //写错误，直接退出
									{
											#if DEBUG_CPU_SENDER
											debug ( "write key no.%x fail!\r\n",key_index);
											#endif
											key_index = CPU_KEY_INDEX_MAX;
											card_sender->result = ADD_MF_KEY_ERROR;
											step = STEP_MAX;
									}
								}
								
            }
        }
        break;

        case CREAT_ADF_FILE:
				{
            card_sender->result = CreatADF ( card_sender->adf, card_sender->adf_size );
            if ( card_sender->result )
            {
                if ( card_sender->result == ISO14443A_FILE_ALREADY_EXIST ) //文件已经存在
                {
                    error_cnt = 0;
									  card_sender->result = ISO14443A_SUCCESS;
                    #if DEBUG_CPU_SENDER
                    debug ( "ADF_FILE already creat!!!!!\r\n" );
                    #endif
                }
                else
                {
                    step = CREAT_ADF_FILE - 1;//重试
                    if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //失败次数达到最大
                    {
                        #if DEBUG_CPU_SENDER
                        debug ( "CREAT_ADF_ERROR!!!!!\r\n" );
                        #endif
                        card_sender->result = CREAT_ADF_ERROR;
                        step = STEP_MAX;
                    }
										break;
                }
            }
            else
            {
                #if DEBUG_CPU_SENDER
                debug ( "CREAT_ADF SUCCESS!!!!!\r\n" );
                #endif
                error_cnt = 0;
            }
            card_sender->result = SelectFile ( card_sender->adf );
            if ( card_sender->result )
            {
                card_sender->result = SelectFile ( card_sender->adf );
                if ( card_sender->result )
                {
                    card_sender->result = CREAT_ADF_ERROR;
                    step = STEP_MAX;
                    #if DEBUG_CPU_SENDER
                    debug ( "SELECT_ADF_ERROR!!!!!\r\n" );
                    #endif
                }
                else
                {
                    error_cnt = 0;
                }
            }
            else
            {
                #if DEBUG_CPU_SENDER
                debug ( "SELECT_ADF success!\r\n" );
                #endif
                error_cnt = 0;
            }
				}
        break;

        case CREAT_ADF_KEY_FILE:
				{
            card_sender->result = CreatKeyFile ( card_sender->key_file, card_sender->key_file_size );
            if ( card_sender->result )
            {
                if ( card_sender->result == ISO14443A_FILE_ALREADY_EXIST ) //文件已经存在
                {
                    error_cnt = 0;
                    #if DEBUG_CPU_SENDER
                    debug ( "ADF_KEY_FILE already creat!!!!!\r\n" );
                    #endif
									  card_sender->result = ISO14443A_SUCCESS;;
                }
                else
                {
                    step = CREAT_ADF_KEY_FILE - 1;//重试
                    if ( ( error_cnt++ ) == 2 ) //失败次数达到最大
                    {
                        #if DEBUG_CPU_SENDER
                        debug ( "CREAT_ADF_KEY_FILE_ERROR!!!!!\r\n" );
                        #endif
                        card_sender->result = CREAT_ADF_KEY_FILE_ERROR;
                        step = STEP_MAX;
                    }
										break;
                }
            }
            else
            {
                #if DEBUG_CPU_SENDER
                debug ( "CREAT_ADF_KEY_FILE SUCCESS!!!!!\r\n" );
                #endif
                error_cnt = 0;
            }
            card_sender->result = SelectFile ( card_sender->key_file );
            if ( card_sender->result )
            {
                card_sender->result = SelectFile ( card_sender->key_file );
                if ( card_sender->result )
                {
                    card_sender->result = CREAT_ADF_KEY_FILE;
                    step = STEP_MAX;
                    #if DEBUG_CPU_SENDER
                    debug ( "SELECT_ADF_KEY_FILE_ERROR!!!!!\r\n" );
                    #endif
                }
                else
                {
                    error_cnt = 0;
                }
            }
            else
            {
                #if DEBUG_CPU_SENDER
                debug ( "CREAT_ADF_KEY_FILE success!\r\n" );
                #endif
                error_cnt = 0;
            }
			  }
        break;

        case ADD_ADF_KEY:
        {
            uint8_t key_index = 0;
            for ( key_index = 0; key_index < CPU_KEY_INDEX_MAX; key_index++ )
            {
								#if DEBUG_CPU_SENDER
                debug ( "key type:%x\r\n", card_sender->key_type[key_index] );
                #endif
                for ( error_cnt = 0; error_cnt < RETRY_MAX_TIMES; error_cnt++ ) //写成功，跳出，写失败，再重试写ERR_MAX_CNT次
                {
                    card_sender->result = AddKey ( card_sender->key_type[key_index], card_sender->key_cnt[key_index], 16, &card_sender->key[16 * key_index] );
                    if ( card_sender->result )
                    {
                        if ( card_sender->result == ISO14443A_FILE_ALREADY_EXIST ) //文件已经存在
                        {
                            #if DEBUG_CPU_SENDER
                            debug ( "key no.%x already creat!!!!!\r\n", key_index );
                            #endif
                            card_sender->result = ISO14443A_SUCCESS;;
                            break;//跳出本次写，回主循环写下一个密钥
                        }
                        else
                        {
                            #if DEBUG_CPU_SENDER
                            debug ( "write key no.%x fail!!!!!\r\n", key_index );
                            #endif
                        }
                    }
                    else
                    {
                        #if DEBUG_CPU_SENDER
                        debug ( "write key no.%x success!\r\n", key_index );
                        #endif
                        break;
                    }
                }
                if ( error_cnt == RETRY_MAX_TIMES ) //写错误，直接退出
                {
                    key_index = CPU_KEY_INDEX_MAX;
                    card_sender->result = ADD_ADF_KEY_ERROR;
                    step = STEP_MAX;
                }
            }
        }
        break;
#if 1//创建EF文件并初始化
        case CREAT_EF://可选
				{
            card_sender->result = CreatEFFile ( card_sender->ef, card_sender->ef_item_cnt, card_sender->ef_type[1], card_sender->ef_rw_key );
            if ( card_sender->result )
            {
                if ( card_sender->result == ISO14443A_FILE_ALREADY_EXIST ) //文件已经存在
                {
                    error_cnt = 0;
                    #if DEBUG_CPU_SENDER
                    debug ( "ef file already creat!!\r\n" );
                    #endif
									  card_sender->result = ISO14443A_SUCCESS;
                }
                else
                {
                    step = CREAT_EF - 1;//重试
                    if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //失败次数达到最大
                    {
                        #if DEBUG_CPU_SENDER
                        debug ( "CeatEFFile fail!!!!!\r\n" );
                        #endif
                        card_sender->result = CREAT_EF_ERROR;
                        step = STEP_MAX;
                    }
										break;
                }
            }
            else
            {
                #if DEBUG_CPU_SENDER
                debug ( "CeatEFFile SUCCESS!!!!!\r\n" );
                #endif
                error_cnt = 0;
            }
            card_sender->result = SelectFile ( card_sender->ef );
            if ( card_sender->result )
            {
                card_sender->result = SelectFile ( card_sender->ef );
                if ( card_sender->result )
                {
                    card_sender->result = CREAT_EF_ERROR;
                    step = STEP_MAX;
                    #if DEBUG_CPU_SENDER
                    debug ( "SEL_EF fail!!!!!\r\n" );
                    #endif
                }
                else
                {
                    error_cnt = 0;
                }
            }
            else
            {
                error_cnt = 0;
                #if DEBUG_CPU_SENDER
                debug ( "SEL_EF success!!!!!\r\n" );
                #endif
            }
				}
        break;

        case INIT_EF://可选，(不初始化，写多个buf，最后一个如果不是item整数倍会报长度错误)
        {
            uint8_t init_data[EACH_ITEM_MAX_LEN] = {0}, i = 0;
						uint8_t item_total_cnt = 0;
						
						//判断CPU_RWBUF_MAX是否为定长的整数倍，不足一条的取一条长度
						item_total_cnt = (CPU_RWBUF_MAX % card_sender->ef_type[1])?(CPU_RWBUF_MAX / card_sender->ef_type[1] + 1):(CPU_RWBUF_MAX / card_sender->ef_type[1]);
            memset ( init_data, 0xff, EACH_ITEM_MAX_LEN );
            for ( i = 1; i <= ( item_total_cnt ); i++ )
            {
                card_sender->result = WriteEF ( i, card_sender->ef_type[1], init_data );
                if ( card_sender->result )
                {
                    #if DEBUG_CPU_SENDER
                    debug ( "INIT_EF fail,I = %d!!!!!\r\n", i );
                    #endif
                    card_sender->result = CREAT_EF_ERROR;
                    step = STEP_MAX;
                    break;
                }
            }
        }
        break;
#else
				case CREAT_EF://可选
				case INIT_EF://可选
				break;
#endif 
        case HAL_CARD:
				{
            card_sender->result = ISO14443A_Halt();
            if ( card_sender->result )
            {
                #if DEBUG_CPU_SENDER
                debug ( "sender card fail!!!!!!!!!!!!!!!!!\r\n" );
                debug ( "ISO14443A_Halt error!!!\r\n" );
                #endif
                card_sender->result = HAL_CARD_ERROR;
                step = STEP_MAX;
            }
            else
            {
                #if DEBUG_CPU_SENDER
                debug ( "sender card success!!!!!!!!!!!!!!!!!\r\n" );
                #endif
                card_sender->result = CPU_SUCCESS;
                step = STEP_MAX + 1;
            }
				}
        break;

        default:
				{
            ISO14443A_Halt();
            #if DEBUG_CPU_SENDER
            debug ( "sender card fail!!!!!!!!!!!!!!!!!\r\n" );
            #endif
				}
            break;
        }
    }
    return card_sender->result;
}
/******************************************************************************
* Name:      card_rw_drv_set_para
*
* Desc:      卡操作的参数设置
* Param(in):
* Param(out):
* Return:
* Global:
* Note:      对于读写模块只要传入工程号即可
* Author:    liuwq
*------------------------------------------------------------------------------
* Log:   2017/10/10, Create this function by liuwq
*******************************************************************************/
static void card_rw_drv_set_config ( CARD_RW_CONFIG* card_para )
{
    memcpy ( &card_rw_config, card_para, sizeof ( CARD_RW_CONFIG ) );
}

void card_rw_drv_init ( void )
{
    CARD_RW_CONFIG config;

    memset ( config.prj_num, 0xff, 16 );
    card_rw_drv_set_config ( &config );
}

/*---------------------------------------------------------------------------*/

