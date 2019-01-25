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
//�жϿ����û����ݿ黹�������
#define block_judge(block_cn)     ((((block_cn+1)&0x03)==0)||(block_cn==0))?DATA_KEY:DATA_USER
#define EACH_ITEM_MAX_LEN 0x20
//���ֿƼ�Ĭ�ϵ�cpu������/////////////////////////////////////////
uint8_t const  LEELEN_MF[2]            = {0x3F, 0x00}; //leelen���ļ���id
uint8_t const  LEELEN_ADF[2]           = {0x10, 0x05}; //leelenӦ���ļ���id
uint8_t const  LEELEN_ADF_SIZE[2]      = {0X04, 0x00}; //�ļ����ܴ�С1KB
uint8_t const  LEELEN_KEY_FILE[2]      = {0x00, 0x01}; //��Կ�ļ�id����Ϊ0����Ȼѡ�е�ʱ��Ӧ�����
uint8_t const  LEELEN_KEY_FILE_SIZE[2] = {0X00, 0xa0}; //��Կ�ļ���С10����Կ��ÿ����Կ����Ϊ16
uint8_t const  LEELEN_EF[2]            = {0x00, 0x02}; //EF�ļ�id
uint8_t const  LEELEN_EX_KEY_CN        = 0x03;         //�ⲿ��Կ��
uint8_t const  LEELEN_IN_KEY_CN        = 0x05;         //�ڲ���Կ��
uint8_t const  LEELEN_EF_RKEY_CN       = 0x01;         //��EF��Կ��
uint8_t const  LEELEN_EF_WEY_CN        = 0x02;         //дEF��Կ��
uint8_t const  LEELEN_KEY_LEN          = 0x10;         //��Կ����
uint8_t const  LEELEN_READ_KEY_CN      = 0x01;
uint8_t const  LEELEN_WRITE_KEY_CN     = 0x02;
uint8_t const  LEELEN_EF_TYPE[2]       = {0x2A, EACH_ITEM_MAX_LEN}; //��¼����0x2A������EACH_ITEM_MAX_LEN��32�ֽ�
uint8_t const  LEELEN_EF_ITEM_CNT      = 0x0a;         //������¼���ܸ���
uint8_t const  LEELEN_EF_RW_KEY        = 0x91;         //��д��Կ��
uint8_t const  LEELEN_EF_DATA_LEN      = 250;          //��д�ļ�¼�ļ��ܴ�С

///////////////////////////////////////////////////////////////////
#define ExAuthenticate_KEY_TYPE 0x39 //�ⲿ��֤��Կ����
#define	InAuthenticate_KEY_TYPE 0x30 //�ڲ���֤��Կ����
#define	ReadEF_KEY_TYPE         0x39 //��EF��Կ����
#define	WriteEF_KEY_TYPE        0x39 //дEF��Կ����


/*------------------------ Variable Declarations -----------------------------*/
static CARD_RW_CONFIG  card_rw_config;

/*------------------------ Function Prototype --------------------------------*/




/*------------------------ Function Implement --------------------------------*/
/******************************************************************************
* Name:      cpu_card_default_init
*
* Desc:      ��CPU��Ĭ�ϲ�����ʼ�������봫��uid��
* Param(in):
* Param(out):
* Return:
* Global:
* Note:      ��Ҫ���뿨��
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
//	  scan_ISO14443A_card ( cpu_para->uid );�ɼ�⵽���¼�ʱ����
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
    for ( index = 0; index < CPU_KEY_INDEX_MAX; index++ ) //��ȡ��Կ
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
//������
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
* Desc:      �ж��������Ŀ��Ƿ�Ϊ����ѡ��Ŀ�
* Param(in):     ����ѡ��Ŀ���uid
* Param(out):
* Return:    0->�ɹ�
* Global:
* Note:
* Author:    liuwq
*------------------------------------------------------------------------------
* Log:   2017/10/10, Create this function by liuwq
*******************************************************************************/
static uint8_t card_ic_select ( uint8_t* card_uid )
{
    uint8_t status;
    uint8_t s_card_uid[11];//��������pcdɨ��õ��Ŀ���

    status = scan_ISO14443A_card ( s_card_uid );
    if ( status != ISO14443A_SUCCESSCODE )
    {
        status = scan_ISO14443A_card ( s_card_uid );
    }
    if ( status != ISO14443A_SUCCESSCODE )
    {
        return CARD_NONE_ERR;
    }
    //��pcdɨ��õ��Ŀ���������ѡ��Ŀ��ŶԱ�
    if ( memcmp ( s_card_uid, card_uid, 4 ) )
    {
        return CARD_ID_ERR;
    }
    return ISO14443A_SUCCESSCODE;
}
/******************************************************************************
* Name:      card_ic_rw_block
*
* Desc:      IC�����д
* Param(in):     date_buff
* Param(out):
* Return:    1->�ɹ���2->ʧ��
* Global:
* Note:      ������дȨ��:0x78,0x77,0x88,0x69,keyAֻ�ܶ�,keyB�ɶ�д
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
        case 1 :/* ѡ��IC��  */
            ic_block_info->result_expand = card_ic_select ( ic_block_info->uid );
            if ( ic_block_info->result_expand != ISO14443A_SUCCESSCODE )
            {
                step = STEP_MAX;
                ic_block_info->result = IC_FAIL;
            }
            break;
        case 2 :/* ��ȡ��Կ */
            if ( ( ic_block_info->key_cn != ISO14443A_CMD_AUTH_A ) && ( ic_block_info->key_cn != ISO14443A_CMD_AUTH_B ) ) //����Կ�ţ�ʹ������Ĭ����Կ
            {
                if ( ic_block_info->cmd_aux == IC_BLOCK_READ )
                {
                    ic_block_info->key_cn = 0x00;  //������������Ĭ��ΪKEYA��֤
                    gen_key_ic ( ic_block_info->key_cn, ic_block_info->uid, ic_block_info->block_cn >> 2, card_rw_config.prj_num, ic_block_info->key ); //���㿨��������Կ
                    ic_block_info->key_cn = ISO14443A_CMD_AUTH_A;
                }
                else if ( ic_block_info->cmd_aux == IC_BLOCK_WRITE )
                {
                    ic_block_info->key_cn = 0x01;  //д����������Ĭ��ΪKEYB��֤
                    gen_key_ic ( ic_block_info->key_cn, ic_block_info->uid, ic_block_info->block_cn >> 2, card_rw_config.prj_num, ic_block_info->key ); //���㿨��������Կ
                    ic_block_info->key_cn = ISO14443A_CMD_AUTH_B;
                }
            }
            break;
        case 3 :/* ��Կ��֤  */
            if ( ISO14443A_Authen ( ic_block_info->uid, ic_block_info->block_cn, ic_block_info->key, ic_block_info->key_cn ) )
            {
                step = STEP_MAX;
                ic_block_info->result = IC_FAIL;
                ic_block_info->result_expand = IC_AUTH_ERR;
            }
            break;
        case 4 :
            if ( ic_block_info->cmd_aux == IC_BLOCK_READ ) /* ���鲽��  */
            {
                memset ( ic_block_info->block_buffer, 0, 16 );
                if ( ISO14443A_Read ( ic_block_info->block_cn, ic_block_info->block_buffer ) )
                {
                    step = STEP_MAX;
                    ic_block_info->result = IC_FAIL;
                    ic_block_info->result_expand = BLOCK_ERR;
                }
            }
            else if ( ic_block_info->cmd_aux == IC_BLOCK_WRITE ) /* д�鲽��  */
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
        case 5 : /* halt��  */
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
* Desc:      CPU���ڲ���֤
* Param(in):
* Param(out):
* Return:    IC_SUCCESS->�ɹ�
* Global:
* Note:      ��ʹ��Ĭ�ϲ���������֤����Ҫ���뿨�ţ�ADF��Ϊ0
* Author:    liuwq
*------------------------------------------------------------------------------
* Log:   2017/11/08, Create this function by liuwq
*******************************************************************************/
uint8_t cpu_internal_auth ( CPU_IN_AUTH_T* date_buff )
{
    CPU_IN_AUTH_T* in_auth;
    uint8_t mf[2] = {0x3f, 0x00};
    uint8_t step, rlen, des_in[8], des_out[8], random[8]; //��Ϊʹ��des��ı�des_in��ֵ�����Զ�����ʱ����random

    in_auth = ( CPU_IN_AUTH_T* ) date_buff;
    for ( step = 1; step <= ( STEP_MAX + 1 ); step++ )
    {
        in_auth->result = IC_FAIL;
        switch ( step )
        {
        case 1://ѡ�п�
            in_auth->result = card_ic_select ( in_auth->uid );
            if ( in_auth->result )
            {
                step = STEP_MAX;
                in_auth->result = IC_FAIL;
                in_auth->result_expand = CARD_ID_ERR;
            }
            break;

        case 2://����MF�ļ���
            Rats();
            in_auth->result = SelectFile ( mf );
            if ( in_auth->result )
            {
                step = STEP_MAX;
                in_auth->result = IC_FAIL;
                in_auth->result_expand = MF_ERR;
            }
            break;

        case 3://����ADF�ļ���
            /* ADF==00 00 ���в���ʹ�����ֵĲ���*/
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

        case 4://�ڲ���֤
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
                TDES ( random, des_out, in_auth->key, ENCRY ); //���ؽ��м��ܼ���
                in_auth->result = InAuthenticate ( des_in, in_auth->key_cn, des_out ); //�ڲ���֤
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
                in_auth->result = InAuthenticate ( des_in, in_auth->key_cn, des_out ); //�ڲ���֤
                if ( in_auth->result )
                {
                    step = STEP_MAX;
                    in_auth->result = IC_FAIL;
                    in_auth->result_expand = IN_AUTH_ERR;
                }
            }
            else {}
            break;

        case 5://ͣ��
                in_auth->result = ISO14443A_Halt();
                if(in_auth->result)
                {
                    step = STEP_MAX;
                }
                else//���ˣ��ڲ���֤�ɹ�
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
* Desc:      ��дCPU����¼�ļ������֧��200�ֽ�
* Param(in):
* Param(out):
* Return:
* Global:		��ʹ��leelenĬ�ϲ�����ֻ�贫�뿨�ź�adf��0x00��0x00
* Note:      ��Ŀ��Ŵ�1��ʼ
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
        case 1 :/*ѡ��IC��*/
            cpu_rw_info->result = card_ic_select ( cpu_rw_info->uid );
            if ( cpu_rw_info->result )
            {
                step = STEP_MAX;
                cpu_rw_info->result = IC_FAIL;
                cpu_rw_info->result_expand = CARD_ID_ERR;
            }
            break;

        case 2://ѡ��MF�ļ���
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

        case 3://ѡ��adf
            /* ADF==00 00 ���в���ʹ�����ֵĲ���*/
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

        case 4://�ⲿ��֤��/д����
            cpu_rw_info->result = ExAuthenticate ( cpu_rw_info->key_cn, cpu_rw_info->key );
            if ( cpu_rw_info->result )
            {
                step = STEP_MAX;
                cpu_rw_info->result = IC_FAIL;
                cpu_rw_info->result_expand = EX_AUTH_ERR;
            }
            break;

        case 5://ѡ��EF�ļ�
            cpu_rw_info->result = SelectFile ( cpu_rw_info->ef );
            if ( cpu_rw_info->result )
            {
                step = STEP_MAX;
                cpu_rw_info->result = IC_FAIL;
                cpu_rw_info->result_expand = EF_ERR;
            }
            break;

        case 6://��/дEF�ļ�
            if ( cpu_rw_info->ef_type[0] == EF_TYPE_FIXED_RECORD ) //������¼�ļ�
            {
                if ( cpu_rw_info->ef_type[1] < 1 ) //ÿ����¼�ĳ���С��1
                {
                    step = STEP_MAX;
                    cpu_rw_info->result = IC_FAIL;
                    cpu_rw_info->result_expand = EF_OPERATE_ERR;
                    break;
                }
                temp = cpu_rw_info->len / cpu_rw_info->ef_type[1];
                if ( temp ) //��/д�ĳ��ȳ���������¼�ĳ��ȣ��ֶ�����/д
                {
                    for ( i = 0; i < temp; i++ )//��������������Ŀ����
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
                    temp = cpu_rw_info->len % cpu_rw_info->ef_type[1]; //�ж�ʣ�಻�㵥����¼���ֽ���
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
                else//��/д�ĳ���С�ڵ�����¼�ĳ���,ֱ�Ӷ�/д
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
            else if ( cpu_rw_info->ef_type[0] == EF_TYPE_BIN ) //�������ļ����ݲ�֧��
            {
                step = STEP_MAX;
                cpu_rw_info->result = IC_FAIL;
                cpu_rw_info->result_expand = EF_OPERATE_ERR;
            }
            break;

        case 7://ͣ��
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
* Desc:      CPU����(��Ĭ�ϲ���������ֻ�贫��uid��adf: 00 00)
* Param(in):
* Param(out):
* Return:		0->�ɹ�,>0Ϊ�����룬���CPU_CARD_RESULT_E
* Global:
* Note:      ��Ŀ¼�µ���Կ��ADF�µ���Կ�Ƿ�ֱ���
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
        case CHECK_HAVE_CARD://�жϿ����Ƿ���ȷ
				{
            //ɨ�迨������ISO14443A����ģʽ���Զ��е�ISO14443Aģʽ
            card_sender->result = card_ic_select ( card_sender->uid );
            if ( card_sender->result )
            {
                step = CHECK_HAVE_CARD - 1;//����
                if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //ʧ�ܴ����ﵽ���
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

        case CHECK_CARD_TYPE://�жϿ����ͣ�������APDUָ��ģʽ
				{
            card_sender->result = Rats();
            if ( card_sender->result )
            {
                step = CHECK_CARD_TYPE - 1;//����
                if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //ʧ�ܴ����ﵽ���
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

        case SELECT_MF://������Ŀ¼
				{
            card_sender->result = SelectFile ( card_sender->mf );
            if ( card_sender->result )
            {
                step = SELECT_MF - 1;//����
                if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //ʧ�ܴ����ﵽ���
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
				
				case DEFAULE_AUTH://�ó���Ĭ����Կ������֤
        {
            uint8_t key_cnt = 0;
            uint8_t key_buf[16] = {0};


						gen_key_cpu ( card_sender->uid, card_sender->mf, 0x03, card_rw_config.prj_num, key_buf );
						card_sender->result = ExAuthenticate ( 3, key_buf );
						
//						memset ( key_buf, 0xff, 16 );
//            card_sender->result = ExAuthenticate ( key_cnt, key_buf );
            if ( card_sender->result )
            {
                step = DEFAULE_AUTH - 1;//����
                if ( ( error_cnt++ ) == 1 ) //ʧ�ܴ����ﵽ���
                {
                    card_sender->result = DEFAULE_AUTH_ERROR;
                    step = IN_AUTH-1;//Ĭ����Կ��֤ʧ�ܣ������Ѿ������������Խ����ڲ���֤
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
							  step = DELETE_ALL-1;//�����ڲ���֤,ֱ��ɾ�������ļ�
            }
        }
        break;

        case IN_AUTH://�ڲ���֤���ж��Ƿ��Ѿ�������
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
            TDES ( des_temp, des_out, key_buf, ENCRY ); //���ؽ��м��ܼ���
            card_sender->result = InAuthenticate ( des_in, card_sender->key_cnt[InAuthenticate_KEY_INDEX], des_out ); //�ڲ���֤
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
                step = DELETE_ALL - 1;//����
                if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //ʧ�ܴ����ﵽ���
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

        case CREAT_MF_KEY_FILE://������Ŀ¼�µ���Կ�ļ�
				{
            card_sender->result = CreatKeyFile ( card_sender->key_file, card_sender->key_file_size );
            if ( card_sender->result )
            {
                if ( card_sender->result == ISO14443A_FILE_ALREADY_EXIST ) //�ļ��Ѿ�����
                {
                    error_cnt = 0;
                    #if DEBUG_CPU_SENDER
                    debug ( "MF_KEY_FILE already creat!!!!!\r\n" );
                    #endif
                }
                else
                {
                    step = CREAT_MF_KEY_FILE - 1;//����
                    if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //ʧ�ܴ����ﵽ���
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
								if(key_index == MF_ExAuthenticate_KEY_INDEX || key_index == MF_InAuthenticate_KEY_INDEX)//��Ŀ¼ֻ������ⲿ��֤��Կ
								{	
			  					#if DEBUG_CPU_SENDER
									debug ( "key type:%x\r\n", card_sender->key_type[key_index] );
									#endif
									for ( error_cnt = 0; error_cnt < RETRY_MAX_TIMES; error_cnt++ ) //д�ɹ���������дʧ�ܣ�������дERR_MAX_CNT��
									{
											card_sender->result = AddKey ( card_sender->key_type[key_index], card_sender->key_cnt[key_index],16, &card_sender->key[16 * key_index] );
											if ( card_sender->result )
											{
													if ( card_sender->result == ISO14443A_FILE_ALREADY_EXIST ) //�ļ��Ѿ�����
													{
															#if DEBUG_CPU_SENDER
															debug ( "key no.%x already creat!!!!!\r\n", key_index );
															#endif
															card_sender->result = ISO14443A_SUCCESS;
															break;//��������д������ѭ��д��һ����Կ
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
									if ( error_cnt == RETRY_MAX_TIMES ) //д����ֱ���˳�
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
                if ( card_sender->result == ISO14443A_FILE_ALREADY_EXIST ) //�ļ��Ѿ�����
                {
                    error_cnt = 0;
									  card_sender->result = ISO14443A_SUCCESS;
                    #if DEBUG_CPU_SENDER
                    debug ( "ADF_FILE already creat!!!!!\r\n" );
                    #endif
                }
                else
                {
                    step = CREAT_ADF_FILE - 1;//����
                    if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //ʧ�ܴ����ﵽ���
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
                if ( card_sender->result == ISO14443A_FILE_ALREADY_EXIST ) //�ļ��Ѿ�����
                {
                    error_cnt = 0;
                    #if DEBUG_CPU_SENDER
                    debug ( "ADF_KEY_FILE already creat!!!!!\r\n" );
                    #endif
									  card_sender->result = ISO14443A_SUCCESS;;
                }
                else
                {
                    step = CREAT_ADF_KEY_FILE - 1;//����
                    if ( ( error_cnt++ ) == 2 ) //ʧ�ܴ����ﵽ���
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
                for ( error_cnt = 0; error_cnt < RETRY_MAX_TIMES; error_cnt++ ) //д�ɹ���������дʧ�ܣ�������дERR_MAX_CNT��
                {
                    card_sender->result = AddKey ( card_sender->key_type[key_index], card_sender->key_cnt[key_index], 16, &card_sender->key[16 * key_index] );
                    if ( card_sender->result )
                    {
                        if ( card_sender->result == ISO14443A_FILE_ALREADY_EXIST ) //�ļ��Ѿ�����
                        {
                            #if DEBUG_CPU_SENDER
                            debug ( "key no.%x already creat!!!!!\r\n", key_index );
                            #endif
                            card_sender->result = ISO14443A_SUCCESS;;
                            break;//��������д������ѭ��д��һ����Կ
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
                if ( error_cnt == RETRY_MAX_TIMES ) //д����ֱ���˳�
                {
                    key_index = CPU_KEY_INDEX_MAX;
                    card_sender->result = ADD_ADF_KEY_ERROR;
                    step = STEP_MAX;
                }
            }
        }
        break;
#if 1//����EF�ļ�����ʼ��
        case CREAT_EF://��ѡ
				{
            card_sender->result = CreatEFFile ( card_sender->ef, card_sender->ef_item_cnt, card_sender->ef_type[1], card_sender->ef_rw_key );
            if ( card_sender->result )
            {
                if ( card_sender->result == ISO14443A_FILE_ALREADY_EXIST ) //�ļ��Ѿ�����
                {
                    error_cnt = 0;
                    #if DEBUG_CPU_SENDER
                    debug ( "ef file already creat!!\r\n" );
                    #endif
									  card_sender->result = ISO14443A_SUCCESS;
                }
                else
                {
                    step = CREAT_EF - 1;//����
                    if ( ( error_cnt++ ) == RETRY_MAX_TIMES ) //ʧ�ܴ����ﵽ���
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

        case INIT_EF://��ѡ��(����ʼ����д���buf�����һ���������item�������ᱨ���ȴ���)
        {
            uint8_t init_data[EACH_ITEM_MAX_LEN] = {0}, i = 0;
						uint8_t item_total_cnt = 0;
						
						//�ж�CPU_RWBUF_MAX�Ƿ�Ϊ������������������һ����ȡһ������
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
				case CREAT_EF://��ѡ
				case INIT_EF://��ѡ
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
* Desc:      �������Ĳ�������
* Param(in):
* Param(out):
* Return:
* Global:
* Note:      ���ڶ�дģ��ֻҪ���빤�̺ż���
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

