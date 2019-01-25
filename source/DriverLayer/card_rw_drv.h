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
///////////////////////////////////////IC卡相关////////////////////////////////
/*IC卡扇区数据类型*/
typedef enum
{
	DATA_USER,  //用户数据
	DATA_KEY,   //密钥
} CARD_DATA_TYPE;

/*卡操作辅助指令*/
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
/*读写卡配置*/
typedef struct CARD_RW_S
{
	uint8_t prj_num[16];      //项目号
} CARD_RW_CONFIG;

/* IC卡读和写块通信数据结构*/
typedef struct
{
	uint8_t cmd_aux;          //辅助指令  0x11:写，0x12:读
	uint8_t uid[4];           //卡号
	uint8_t block_cn;         //绝对块号   /4得扇区号
	uint8_t key_cn;           //密钥号:0x60 KEYA  0X61 KEYB
	uint8_t key[6];           //密钥value
	uint8_t block_buffer[16]; //block数据value
	uint8_t result;           //1成功，2失败
	uint8_t result_expand;    //错误码,见CARD_ERR0R_ENUM
} IC_BLOCK_RW_STR;

/* IC卡读和写块操作结果上传通信数据结构*/
typedef struct
{
	uint8_t cmd_aux;      //辅助指令0X15
	uint8_t uid[4];       //卡号
	uint8_t block_cn;     //绝对块号   /4得扇区号
	uint8_t result;       //IC卡操作结果，0x01成功 0x02失败
	uint8_t payload[16];  //数据，若为读操作，存放读到的数据，若为写操作，则无数据
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

/////////////////////////cpu 卡相关////////////////////////////////////////////////
#define CPU_RWBUF_MAX 250 

//基本文件类型
typedef enum
{
	EF_TYPE_BIN           = 0x28,
	EF_TYPE_FIXED_RECORD  = 0x2a,
	EF_TYPE_RANDOM_RECORD = 0x2c,
	EF_TYPE_LOOP          = 0x2e,
	EF_TYPE_KEY           = 0x3f,
	EF_TYPE_PBOC_ED_EP    = 0x2f,
} EF_TYPE_E;

/* CPU卡内部认证通信数据结构*/
typedef struct   CPU_IN_AUTH_S
{
	uint8_t cmd_aux;      //辅助指令0X20
	uint8_t uid[4];
	uint8_t adf[2];
	uint8_t key_cn;
	uint8_t key_len;
	uint8_t key[16];
	uint8_t result;
	uint8_t result_expand;
} CPU_IN_AUTH_T;

/* CPU卡读写记录通信数据结构*/
typedef struct   CPU_RW_S
{
	uint8_t cmd_aux;             //辅助指令0X21
	uint8_t uid[4];              //cpu卡卡号
	uint8_t adf[2];              //adf文件夹标识
	uint8_t ef[2];               //ef文件标识
	uint8_t key_cn;              //密钥号
	uint8_t key_len;             //密钥长度
	uint8_t key[16];             //密钥数据
	uint8_t ef_type[2];          //ef_type[0]:定长记录文件或二进制文件,ef_type[1]每条记录大小
	uint8_t result;              //读写结果
	uint8_t result_expand;       //结果说明
	uint8_t addr_start;          //开始写的位置,最小标号为1,不能是0   记录为记录号，二进制为偏移量
	uint8_t len;                 //读写数据的长度
	uint8_t data[CPU_RWBUF_MAX]; //最多支持200个字节写入
} CPU_RW_T;
//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////CPU发卡相关///////////////////////////////////
typedef enum
{
    ExAuthenticate_KEY_INDEX       = 0,   //ADF外部认证密钥
    InAuthenticate_KEY_INDEX       = 1,   //ADF内部认证密钥
    ReadEF_KEY_INDEX               = 2,   //ADF下EF读认证密钥
    WriteEF_KEY_INDEX              = 3,   //ADF下EF写认证密钥
	MF_ExAuthenticate_KEY_INDEX    = 4,   //MF外部认证密钥
	MF_InAuthenticate_KEY_INDEX    = 5,   //MF内部认证密钥
		CPU_KEY_INDEX_MAX,
} CPU_KEY_INDEX_E;

/* CPU卡发卡通信数据结构*/
typedef struct   CPU_SENDER_CARD_S
{
	uint8_t cmd_aux;             //辅助指 AUTO_SEND( 默认参数初始化)，PARAM_SEND(传入参数初始化)
	uint8_t uid[4];              //cpu卡卡号
	uint8_t mf[2];               //主目录(0x3f 0x00)
	uint8_t adf[2];              //adf文件夹标识
	uint8_t adf_size[2];         //adf大小
	uint8_t key_file[2];         //密钥文件标识
	uint8_t key_file_size[2];    //密钥文件大小
	uint8_t key_type[CPU_KEY_INDEX_MAX]; //密钥类型，分别为内部认证密钥，读EF文件密钥，写EF文件密钥(密钥类型，密钥号，密钥要相对应)
	uint8_t key_cnt[CPU_KEY_INDEX_MAX];  //密钥号，分别为内部认证密钥，读EF文件密钥，写EF文件密钥
	uint8_t key[16 * CPU_KEY_INDEX_MAX]; //密钥
	uint8_t ef[2];               //ef文件标识
	uint8_t ef_type[2];          //ef_type[0]:定长记录文件或二进制文件,ef_type[1]每条记录大小
	uint8_t ef_item_cnt;         //记录的总条数
	uint8_t ef_rw_key;           //密钥权限
	uint8_t result;              //发卡结果
} CPU_SENDER_CARD_T;

typedef enum
{
	CPU_SUCCESS              = 0,
	CARD_UID_ERROR           = 1,    //卡号不匹配
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
	CARD_SENDER_NONE   = 0,    //无
	CHECK_HAVE_CARD    = 1,    //检测是否有卡
	CHECK_CARD_TYPE    = 2,    //判断卡类型
	SELECT_MF          = 3,    //进入主目录
	DEFAULE_AUTH       = 4,    //厂家默认密钥认证
	IN_AUTH            = 5,    //内部认证
	DELETE_ALL         = 6,    //删除MF下所有文件
	CREAT_MF_KEY_FILE  = 7,    //创建主目录下密钥文件
	ADD_MF_KEY         = 8,    //添加主目录下的密钥
	CREAT_ADF_FILE     = 9,    //创建应用文件夹
	CREAT_ADF_KEY_FILE = 0x0a, //创建密钥文件
	ADD_ADF_KEY        = 0x0b, //添加adf下的密钥
	CREAT_EF           = 0x0c, //创建EF文件
	INIT_EF            = 0x0d, //初始化EF文件
	HAL_CARD           = 0x0e, //停卡
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
