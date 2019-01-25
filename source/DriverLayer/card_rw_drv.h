/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 card_rw_drv.h
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/05/28
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/05/28, Liuwq create this file
*
******************************************************************************/
#ifndef _CARD_RW_DRV_H_
#define _CARD_RW_DRV_H_


/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>

/*----------------------------- Global Defines ------------------------------*/
#define  IC_SUCCESS  0x01
#define  IC_FAIL     0x02

/*----------------------------- Global Typedefs -----------------------------*/
///////////////////////////////////////IC�����////////////////////////////////
/*IC��������������*/
typedef enum
{
	DATA_USER,  //�û�����
	DATA_KEY,   //��Կ
} CARD_DATA_TYPE;

/*����������ָ��*/
enum CMD_AUX_E
{
	PARA_SET       = 0x01,
	PARA_REQ       = 0x02,
	IC_BLOCK_WRITE = 0x11,
	IC_BLOCK_READ  = 0x12,
	CPU_IN_AUTH    = 0x20,
	CPU_EF_WRITE   = 0x21,
	CPU_EF_READ    = 0x22,
};
/*��д������*/
typedef struct CARD_RW_S
{
	uint8_t prj_num[16];      //��Ŀ��
} CARD_RW_CONFIG;

/* IC������д��ͨ�����ݽṹ*/
typedef struct
{
	uint8_t cmd_aux;          //����ָ��  0x11:д��0x12:��
	uint8_t uid[4];           //����
	uint8_t block_cn;         //���Կ��   /4��������
	uint8_t key_cn;           //��Կ��:0x60 KEYA  0X61 KEYB
	uint8_t key[6];           //��Կvalue
	uint8_t block_buffer[16]; //block����value
	uint8_t result;           //1�ɹ���2ʧ��
	uint8_t result_expand;    //������,��CARD_ERR0R_ENUM
} IC_BLOCK_RW_STR;

/* IC������д���������ϴ�ͨ�����ݽṹ*/
typedef struct
{
	uint8_t cmd_aux;      //����ָ��0X15
	uint8_t uid[4];       //����
	uint8_t block_cn;     //���Կ��   /4��������
	uint8_t result;       //IC�����������0x01�ɹ� 0x02ʧ��
	uint8_t payload[16];  //���ݣ���Ϊ����������Ŷ��������ݣ���Ϊд��������������
} IC_RESULT_STR;

typedef enum
{
	NONE = 0X00,
	CARD_NONE_ERR       = 0x01,
	CARD_ID_ERR         = 0x02,
	IC_AUTH_ERR         = 0x03,
	BLOCK_ERR           = 0x04,
	MF_ERR              = 0X05,
	ADF_ERR             = 0x06,
	EX_AUTH_ERR         = 0x07,
	IN_AUTH_ERR         = 0x08,
	EF_ERR              = 0x09,
	EF_OPERATE_ERR      = 0x0A,
	IC_WRITE_SUCCESS    = 0x0B,
	IC_READ_SUCCESS     = 0x0C,
	CPU_OPERATE_SUCCESS = 0x0D,
} CARD_ERR0R_ENUM;
///////////////////////////////////////////////////////////////////////////////////

/////////////////////////cpu �����////////////////////////////////////////////////
#define CPU_RWBUF_MAX 250 

//�����ļ�����
typedef enum
{
	EF_TYPE_BIN           = 0x28,
	EF_TYPE_FIXED_RECORD  = 0x2a,
	EF_TYPE_RANDOM_RECORD = 0x2c,
	EF_TYPE_LOOP          = 0x2e,
	EF_TYPE_KEY           = 0x3f,
	EF_TYPE_PBOC_ED_EP    = 0x2f,
} EF_TYPE_E;

/* CPU���ڲ���֤ͨ�����ݽṹ*/
typedef struct   CPU_IN_AUTH_S
{
	uint8_t cmd_aux;      //����ָ��0X20
	uint8_t uid[4];
	uint8_t adf[2];
	uint8_t key_cn;
	uint8_t key_len;
	uint8_t key[16];
	uint8_t result;
	uint8_t result_expand;
} CPU_IN_AUTH_T;

/* CPU����д��¼ͨ�����ݽṹ*/
typedef struct   CPU_RW_S
{
	uint8_t cmd_aux;             //����ָ��0X21
	uint8_t uid[4];              //cpu������
	uint8_t adf[2];              //adf�ļ��б�ʶ
	uint8_t ef[2];               //ef�ļ���ʶ
	uint8_t key_cn;              //��Կ��
	uint8_t key_len;             //��Կ����
	uint8_t key[16];             //��Կ����
	uint8_t ef_type[2];          //ef_type[0]:������¼�ļ���������ļ�,ef_type[1]ÿ����¼��С
	uint8_t result;              //��д���
	uint8_t result_expand;       //���˵��
	uint8_t addr_start;          //��ʼд��λ��,��С���Ϊ1,������0   ��¼Ϊ��¼�ţ�������Ϊƫ����
	uint8_t len;                 //��д���ݵĳ���
	uint8_t data[CPU_RWBUF_MAX]; //���֧��200���ֽ�д��
} CPU_RW_T;
//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////CPU�������///////////////////////////////////
typedef enum
{
    ExAuthenticate_KEY_INDEX       = 0,   //ADF�ⲿ��֤��Կ
    InAuthenticate_KEY_INDEX       = 1,   //ADF�ڲ���֤��Կ
    ReadEF_KEY_INDEX               = 2,   //ADF��EF����֤��Կ
    WriteEF_KEY_INDEX              = 3,   //ADF��EFд��֤��Կ
	MF_ExAuthenticate_KEY_INDEX    = 4,   //MF�ⲿ��֤��Կ
	MF_InAuthenticate_KEY_INDEX    = 5,   //MF�ڲ���֤��Կ
		CPU_KEY_INDEX_MAX,
} CPU_KEY_INDEX_E;

/* CPU������ͨ�����ݽṹ*/
typedef struct   CPU_SENDER_CARD_S
{
	uint8_t cmd_aux;             //����ָ AUTO_SEND( Ĭ�ϲ�����ʼ��)��PARAM_SEND(���������ʼ��)
	uint8_t uid[4];              //cpu������
	uint8_t mf[2];               //��Ŀ¼(0x3f 0x00)
	uint8_t adf[2];              //adf�ļ��б�ʶ
	uint8_t adf_size[2];         //adf��С
	uint8_t key_file[2];         //��Կ�ļ���ʶ
	uint8_t key_file_size[2];    //��Կ�ļ���С
	uint8_t key_type[CPU_KEY_INDEX_MAX]; //��Կ���ͣ��ֱ�Ϊ�ڲ���֤��Կ����EF�ļ���Կ��дEF�ļ���Կ(��Կ���ͣ���Կ�ţ���ԿҪ���Ӧ)
	uint8_t key_cnt[CPU_KEY_INDEX_MAX];  //��Կ�ţ��ֱ�Ϊ�ڲ���֤��Կ����EF�ļ���Կ��дEF�ļ���Կ
	uint8_t key[16 * CPU_KEY_INDEX_MAX]; //��Կ
	uint8_t ef[2];               //ef�ļ���ʶ
	uint8_t ef_type[2];          //ef_type[0]:������¼�ļ���������ļ�,ef_type[1]ÿ����¼��С
	uint8_t ef_item_cnt;         //��¼��������
	uint8_t ef_rw_key;           //��ԿȨ��
	uint8_t result;              //�������
} CPU_SENDER_CARD_T;

typedef enum
{
	CPU_SUCCESS              = 0,
	CARD_UID_ERROR           = 1,    //���Ų�ƥ��
	CARD_TYPE_ERROR          = 2,
	SELECT_MF_ERROR          = 3,
	IN_AUTH_ERROR            = 4,
	DEFAULE_AUTH_ERROR       = 5,
	DELETE_ALL_ERROR         = 6,
	CREAT_MF_KEY_FILE_ERROR  = 7,
	ADD_MF_KEY_ERROR         = 8,
	CREAT_ADF_ERROR          = 9,
	CREAT_ADF_KEY_FILE_ERROR = 0x0a,
	ADD_ADF_KEY_ERROR        = 0x0b,
	CREAT_EF_ERROR           = 0x0c,
	INIT_EF_ERROR            = 0x0d,
	HAL_CARD_ERROR           = 0x0e,
} CPU_CARD_RESULT_E;
typedef enum
{
	CARD_SENDER_NONE   = 0,    //��
	CHECK_HAVE_CARD    = 1,    //����Ƿ��п�
	CHECK_CARD_TYPE    = 2,    //�жϿ�����
	SELECT_MF          = 3,    //������Ŀ¼
	DEFAULE_AUTH       = 4,    //����Ĭ����Կ��֤
	IN_AUTH            = 5,    //�ڲ���֤
	DELETE_ALL         = 6,    //ɾ��MF�������ļ�
	CREAT_MF_KEY_FILE  = 7,    //������Ŀ¼����Կ�ļ�
	ADD_MF_KEY         = 8,    //�����Ŀ¼�µ���Կ
	CREAT_ADF_FILE     = 9,    //����Ӧ���ļ���
	CREAT_ADF_KEY_FILE = 0x0a, //������Կ�ļ�
	ADD_ADF_KEY        = 0x0b, //���adf�µ���Կ
	CREAT_EF           = 0x0c, //����EF�ļ�
	INIT_EF            = 0x0d, //��ʼ��EF�ļ�
	HAL_CARD           = 0x0e, //ͣ��
} CARD_SENDER_CMD_E;
//////////////////////////////////////////////////////////////

/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/
void card_rw_drv_init ( void );
uint8_t card_ic_rw_block ( IC_BLOCK_RW_STR* date_buff, CARD_DATA_TYPE date_type );
uint8_t cpu_card_sender ( CPU_SENDER_CARD_T* card_info );
uint8_t cpu_internal_auth ( CPU_IN_AUTH_T*          date_buff );
uint8_t card_cpu_rw_ef ( CPU_RW_T* data_buf );
#endif //_CARD_RW_DRV_H_
