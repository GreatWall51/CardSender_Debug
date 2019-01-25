/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 lib_iso14443Apcd.h
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
#ifndef _LIB_ISO14443APCD_H_
#define _LIB_ISO14443APCD_H_


/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>

/*----------------------------- Global Defines ------------------------------*/
#define ATQA_LEN 2

#define READ_AUTHORITY                0XF1
#define WRITE_AUTHORITY               0XF2
#define MF_CREATE_AUTHORITY           0XFA
#define MF_DELETE_AUTHORITY           0XFF
#define ADD_KEY_AUTHORITY             0XFB 
#define MF_KEY_USE_AUTHORITY          0xF0 //��Ŀ¼��Կʹ��Ȩ��
#define MF_USER_AUTHEH_AFTER          0XAA //�û���Կ��֤��״̬
#define MF_USER_KEY_MODIFY_AUTHORITY  0XFA //�û���Կ�޸�Ȩ��
#define MF_ADMIN_AUTHEH_AFTER         0XFF //����Ա��Կ��֤��״̬
#define MF_ADMIN_KEY_MODIFY_AUTHORITY 0XFF //����Ա��Կ�޸�Ȩ��
#define DF_KEY_USE_AUTHORITY          0xF0 //DF��Կʹ��Ȩ��
#define DF_USER_AUTHEH_AFTER          0X55 //�û���Կ��֤��״̬
#define DF_KEY_MODIFY_AUTHORITY       0XF5 //DF��Կ�޸�Ȩ��
#define DF_ADMIN_AUTHEH_AFTER         0XFF //����Ա��Կ�޸�Ȩ��

/*----------------------------- Global Typedefs -----------------------------*/
typedef enum {
    ISO14443A_FALSE = 0,
    ISO14443A_TRUE = 1,
} ISO14443A_JUDGE_E;
typedef enum {
    ISO14443A_SUCCESS = 0,
    ISO14443A_FAILURE = 1,
} ISO14443A_RESULT_E;

typedef enum {
		ISO14443A_CMD_AUTHEN_1     = 0x0c,//��֤1
	  ISO14443A_CMD_AUTHEN_2     = 0x14,//��֤2
    ISO14443A_CMD_AUTH_A       = 0x60,//��֤A��Կ
    ISO14443A_CMD_AUTH_B       = 0x61,//��֤B��Կ
    ISO14443A_CMD_READ         = 0x30,//����
    ISO14443A_CMD_WRITE        = 0xA0,//д��
    ISO14443A_CMD_REQUEST_IDL  = 0x26,//Ѱ��������δ��������״̬��
    ISO14443A_CMD_REQUEST_ALL  = 0x52,//Ѱ��������ȫ����
    ISO14443A_CMD_ANTICOL1     = 0x93,//1������ͻ
    ISO14443A_CMD_ANTICOL2     = 0x95,//2������ͻ
    ISO14443A_CMD_ANTICOL3     = 0x97,//3������ͻ
    ISO14443A_CMD_ANTICOL_NVB  = 0x20,
    ISO14443A_CMD_SELECT1      = 0X93,//1������ͻ��ѡ�п�
    ISO14443A_CMD_SELECT2      = 0X95,//2������ͻ��ѡ�п�
    ISO14443A_CMD_SELECT3      = 0X97,//3������ͻ��ѡ�п�
    ISO14443A_CMD_SELECT_NVB   = 0X70,
    ISO14443A_CMD_HALT         = 0x50,//����
    ISO14443A_CMD_RATS         = 0xE0,//����PUDPָ��ģʽ
    ISO14443A_CMD_TRANSFER     = 0xB0,//���滺����������
    ISO14443A_CMD_DECREMENT    = 0xC0,//�ۿ�
    ISO14443A_CMD_INCREMENT    = 0xC1,//��ֵ
    ISO14443A_CMD_RESTORE      = 0xC2,//�������ݵ�������
} ISO14443A_CMD_CODE_E;

typedef enum {
    ISO14443A_REQUEST,
    ISO14443A_ANTICOL,
    ISO14443A_SELECT,
    ISO14443A_HALT,
	
	  ISO14443A_CMD_MAX,
} ISO14443A_CMD_E;

typedef enum {
		ISO14443A_Rats,
		ISO14443A_GetRandom,
		ISO14443A_SelectFile,
		ISO14443A_ExAuthenticate,
		ISO14443A_CreatADF,
		ISO14443A_DeleteADF,
		ISO14443A_CreatKeyFile,
		ISO14443A_AddKey,
		ISO14443A_CreatEFFile,
		ISO14443A_WriteEF,
		ISO14443A_ReadEF,
		ISO14443A_InAuthenticate,
	
		ISO14443A_CPU_CMD_MAX,
} ISO14443A_CPU_CMD_E;
typedef enum {
    ISO14443A_SUCCESSCODE = 0,
    ISO14443A_DEFAULT_ERR,
    ISO14443A_PARAM_ERR,
		ISO14443A_SEND_CMD_ERROR,
    ISO14443A_REQUEST_ERR,
    ISO14443A_ANTICOL_ERR,
    ISO14443A_SELECT_ERR,
    ISO14443A_HALT_ERR,
    ISO14443A_AUTHENT_ERR,
    ISO14443A_READ_ERR,
    ISO14443A_WRITE_ERR,
    ISO14443A_RATS_ERR,
		ISO14443A_GetRandom_ERR,
	  ISO14443A_SelectFile_ERR,
	  ISO14443A_ExAuthenticate_ERR,
	  ISO14443A_InAuthenticate_ERR,
		ISO14443A_DeleteADF_ERR,
	  ISO14443A_CreatADF_ERR,
	  ISO14443A_CreatKeyFile_ERR,
	  ISO14443A_AddKey_ERR,
	  ISO14443A_CreatEFFile_ERR,
	  ISO14443A_WriteEF_ERR,
	  ISO14443A_ReadEF_ERR,
		ISO14443A_FILE_ALREADY_EXIST,
    ISO14443A_RECEIVE_TIMEOUT,
} ISO14443A_ERR_CODE_E;

//typedef enum {
//	
//}CPU_CARD_ERR_CODE_E;
typedef struct {
    /* ATQA answer to request of type A*/
    uint8_t 	ATQA[ATQA_LEN];                     //��REQA��0x26����WUPA��0x52������Ӧ����Ҫ�������б�UID�м�����ATQA����16λ������bit7��bit8���ж���1������2������3��UID��
    uint8_t 	CascadeLevel;                //��ʶ�����м���,��1��,2��,3��,���ų��ȶ�Ӧ4�ֽ�,7�ֽ�,10�ֽ�
    /* UID : unique Identification*/
    uint8_t 	UIDsize;                    //���ų���,��4byte,7byte,10byte
    uint8_t 	UID[11];                    //����,��������У���(1�ֽ�)
    /* SAK : Select acknowledge*/
    uint8_t 	SAK;                        //Select ask,ѡ��Ӧ��CPU��:0x20,IC��:0x08
    uint8_t 	ATSSupported;               //Answer To Select �Ƿ�ͨ��(1->CPU����0->ic��)
    uint8_t 	IsDetected;                 //RF���Ƿ��⵽TYPE_A��
    //RATSָ��ʱʹ��,RATSָ���ʽ:��ʼ�ֽ� + ����(FSDI��CID,1�ֽ�) + CRCA16...add by lwq
    uint8_t 	CID,   //RATSָ��Ĳ����ֶεĵ�4λ,�������ַ�˵�PICC���߼�����0��14��Χ��,ֵ15ΪRFU.
                //CID��PCD�涨�����Ҷ�ͬһʱ�̴���ACTIVE״̬�е�����PICC����Ӧ��Ψһ�ġ�
                //CID��PICC�������ڼ��ǹ̶��ģ�����PICCӦʹ��CID��Ϊ���߼���ʶ�����������ڽ��յ��ĵ�һ���޲���RATS��
                //PCD����CIDΪ0��ʾÿ�ν�֧�ֶ�һ��PICC���ж�λ��
                //CID��ֵһ��Ϊ1.
                FSDI,  //RATSָ��Ĳ����ֶεĵ�4λ,�����ڱ���FSD��FSD������PCD���յ���֡����󳤶ȡ� һ��Ϊ5,��ʾPCD���յ���֡����󳤶�Ϊ64�ֽ�.
                //��������,TARSָ��Ĳ����ֶ�Ϊ0x51,����RATSָ��Ϊ: 0xE0  0x51
                //PPSָ��ʱʹ��,
                DRI,   //��������,PSS1��b4b3λ
                DSI;   //��������,PSS1��b2b1λ
} ISO14443A_CARD;
/*----------------------------- External Variables --------------------------*/
#define FIFO_BUFF_MAX 128
/* CPU��������ָ��ṹ*/
typedef struct   CPU_CMD_S {
    uint8_t CLA;
    uint8_t INS;
    uint8_t P1;
    uint8_t P2;
    uint8_t LC;
    uint8_t data_buff[FIFO_BUFF_MAX+10];
    uint8_t LE;
} CPU_CMD_T;

typedef struct{
	uint8_t file_id[2];       //�ļ�ID
    uint8_t file_type;        //�ļ����� 0x38:adf��0x3F
    uint8_t file_size[2];     //�ļ��ռ��С
    uint8_t build_authority;  //����Ȩ��
    uint8_t delete_authority; //ɾ��Ȩ��
    uint8_t app_id;           //Ӧ��ID 0x95
    uint8_t REF[2];           //Ԥ��
    uint8_t file_name[16];    //�ļ����ƣ�5~16�ֽڣ�
	uint8_t file_name_len;    //�ļ����Ƴ���
} CPU_CREATE_DF_DATA_T;
typedef struct{
	uint8_t file_id[2];       //�ļ�ID
    uint8_t file_type;        //�ļ����� 0x3F:��Կ�ļ�
    uint8_t file_size[2];     //�ļ��ռ��С
    uint8_t app_id;           //DF���ı�ʶ��
    uint8_t add_authority;    //����Ȩ��
    uint8_t REF[2];           //Ԥ��
} CPU_CREATE_KEY_FILE_DATA_T;
typedef struct{
	uint8_t key_cnt;          //��Կ��
    uint8_t key_type;         //��Կ����
    uint8_t use_authority;    //ʹ��Ȩ
    uint8_t modify_authority; //����Ȩ
    uint8_t authority;        //����Ȩ��
    uint8_t err_cnt;          //�������
	uint8_t key[16];          //��Կ
	uint8_t key_len;          //��Կ����
} CPU_CREATE_KEY_DATA_T;
/*------------------------ Global Function Prototypes -----------------------*/
uint8_t scan_ISO14443A_card(uint8_t* uid);
uint8_t ISO14443A_Authen(uint8_t *uid, uint8_t sector, uint8_t *key, uint8_t key_type);
uint8_t ISO14443A_Read(uint8_t block, uint8_t *buff);
uint8_t ISO14443A_Write(uint8_t block, uint8_t *buff);
uint8_t ISO14443A_Halt(void);
uint8_t is_support_rats(void);

uint8_t Rats(void);
uint8_t GetRandom(uint8_t *rece_len, uint8_t *data);
uint8_t ExAuthenticate(uint8_t key_cn, uint8_t *key_buff);
uint8_t CreatADF(uint8_t *file_id, uint8_t *size);
uint8_t DeleteADF(void);
uint8_t SelectFile(uint8_t *file_id);
uint8_t CreatKeyFile(uint8_t *file_id, uint8_t *size);
uint8_t AddKey(uint8_t key_type, uint8_t key_cnt, uint8_t key_len, uint8_t *key);
uint8_t InAuthenticate(uint8_t *des_in, uint8_t key_cn, uint8_t *des_out);
uint8_t CreatEFFile(uint8_t *file_id, uint8_t tiem_cnt, uint8_t item_len, uint8_t rw_key);
uint8_t WriteEF(uint8_t item_id, uint8_t item_len, uint8_t *data);
uint8_t ReadEF(uint8_t item_id, uint8_t item_len, uint8_t *data);


uint8_t ISO14443A_CreatDF ( CPU_CREATE_DF_DATA_T *creat_cmd);
uint8_t ISO14443A_CreateKeyFile (CPU_CREATE_KEY_FILE_DATA_T *create_cmd);
uint8_t ISO14443A_Addkey ( CPU_CREATE_KEY_DATA_T *add_cmd );
#endif //_LIB_ISO14443APCD_H_
