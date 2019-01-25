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
#define MF_KEY_USE_AUTHORITY          0xF0 //主目录密钥使用权限
#define MF_USER_AUTHEH_AFTER          0XAA //用户密钥认证后状态
#define MF_USER_KEY_MODIFY_AUTHORITY  0XFA //用户密钥修改权限
#define MF_ADMIN_AUTHEH_AFTER         0XFF //管理员密钥认证后状态
#define MF_ADMIN_KEY_MODIFY_AUTHORITY 0XFF //管理员密钥修改权限
#define DF_KEY_USE_AUTHORITY          0xF0 //DF密钥使用权限
#define DF_USER_AUTHEH_AFTER          0X55 //用户密钥认证后状态
#define DF_KEY_MODIFY_AUTHORITY       0XF5 //DF密钥修改权限
#define DF_ADMIN_AUTHEH_AFTER         0XFF //管理员密钥修改权限

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
		ISO14443A_CMD_AUTHEN_1     = 0x0c,//认证1
	  ISO14443A_CMD_AUTHEN_2     = 0x14,//认证2
    ISO14443A_CMD_AUTH_A       = 0x60,//验证A密钥
    ISO14443A_CMD_AUTH_B       = 0x61,//验证B密钥
    ISO14443A_CMD_READ         = 0x30,//读块
    ISO14443A_CMD_WRITE        = 0xA0,//写块
    ISO14443A_CMD_REQUEST_IDL  = 0x26,//寻天线区内未进入休眠状态卡
    ISO14443A_CMD_REQUEST_ALL  = 0x52,//寻天线区内全部卡
    ISO14443A_CMD_ANTICOL1     = 0x93,//1级防冲突
    ISO14443A_CMD_ANTICOL2     = 0x95,//2级防冲突
    ISO14443A_CMD_ANTICOL3     = 0x97,//3级防冲突
    ISO14443A_CMD_ANTICOL_NVB  = 0x20,
    ISO14443A_CMD_SELECT1      = 0X93,//1级防冲突后选中卡
    ISO14443A_CMD_SELECT2      = 0X95,//2级防冲突后选中卡
    ISO14443A_CMD_SELECT3      = 0X97,//3级防冲突后选中卡
    ISO14443A_CMD_SELECT_NVB   = 0X70,
    ISO14443A_CMD_HALT         = 0x50,//休眠
    ISO14443A_CMD_RATS         = 0xE0,//进入PUDP指令模式
    ISO14443A_CMD_TRANSFER     = 0xB0,//保存缓冲区中数据
    ISO14443A_CMD_DECREMENT    = 0xC0,//扣款
    ISO14443A_CMD_INCREMENT    = 0xC1,//充值
    ISO14443A_CMD_RESTORE      = 0xC2,//调块数据到缓冲区
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
    uint8_t 	ATQA[ATQA_LEN];                     //对REQA（0x26）和WUPA（0x52）的响应，主要作用是判别UID有几级，ATQA共有16位，根据bit7和bit8来判断是1级还是2级还是3级UID；
    uint8_t 	CascadeLevel;                //标识卡号有几级,分1级,2级,3级,卡号长度对应4字节,7字节,10字节
    /* UID : unique Identification*/
    uint8_t 	UIDsize;                    //卡号长度,有4byte,7byte,10byte
    uint8_t 	UID[11];                    //卡号,包括卡号校验和(1字节)
    /* SAK : Select acknowledge*/
    uint8_t 	SAK;                        //Select ask,选择应答CPU卡:0x20,IC卡:0x08
    uint8_t 	ATSSupported;               //Answer To Select 是否通过(1->CPU卡，0->ic卡)
    uint8_t 	IsDetected;                 //RF场是否检测到TYPE_A卡
    //RATS指令时使用,RATS指令格式:开始字节 + 参数(FSDI和CID,1字节) + CRCA16...add by lwq
    uint8_t 	CID,   //RATS指令的参数字段的低4位,它定义编址了的PICC的逻辑号在0到14范围内,值15为RFU.
                //CID由PCD规定，并且对同一时刻处在ACTIVE状态中的所有PICC，它应是唯一的。
                //CID在PICC被激活期间是固定的，并且PICC应使用CID作为其逻辑标识符，它包含在接收到的第一个无差错的RATS。
                //PCD设置CID为0表示每次仅支持对一张PICC进行定位。
                //CID的值一般为1.
                FSDI,  //RATS指令的参数字段的低4位,它用于编码FSD。FSD定义了PCD能收到的帧的最大长度。 一般为5,表示PCD能收到的帧的最大长度为64字节.
                //综上所述,TARS指令的参数字段为0x51,所以RATS指令为: 0xE0  0x51
                //PPS指令时使用,
                DRI,   //发送因子,PSS1的b4b3位
                DSI;   //接收因子,PSS1的b2b1位
} ISO14443A_CARD;
/*----------------------------- External Variables --------------------------*/
#define FIFO_BUFF_MAX 128
/* CPU卡操作的指令结构*/
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
	uint8_t file_id[2];       //文件ID
    uint8_t file_type;        //文件类型 0x38:adf，0x3F
    uint8_t file_size[2];     //文件空间大小
    uint8_t build_authority;  //创建权限
    uint8_t delete_authority; //删除权限
    uint8_t app_id;           //应用ID 0x95
    uint8_t REF[2];           //预留
    uint8_t file_name[16];    //文件名称（5~16字节）
	uint8_t file_name_len;    //文件名称长度
} CPU_CREATE_DF_DATA_T;
typedef struct{
	uint8_t file_id[2];       //文件ID
    uint8_t file_type;        //文件类型 0x3F:密钥文件
    uint8_t file_size[2];     //文件空间大小
    uint8_t app_id;           //DF短文标识符
    uint8_t add_authority;    //增加权限
    uint8_t REF[2];           //预留
} CPU_CREATE_KEY_FILE_DATA_T;
typedef struct{
	uint8_t key_cnt;          //密钥号
    uint8_t key_type;         //密钥类型
    uint8_t use_authority;    //使用权
    uint8_t modify_authority; //更改权
    uint8_t authority;        //后续权限
    uint8_t err_cnt;          //错误计数
	uint8_t key[16];          //密钥
	uint8_t key_len;          //密钥长度
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
