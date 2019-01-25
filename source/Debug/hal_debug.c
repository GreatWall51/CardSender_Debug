/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName:      hal_debug.c
* Desc:      �ô��ڴ�ӡ�����ֽ�����
*
*
* Author:    LiuWeiQiang
* Date:      2017/05/18
* Notes:
*
* -----------------------------------------------------------------------------
* Histroy: v1.0   2017/05/18, LiuWeiQiang create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "hal_debug.h"
#include "list.h"
#include "debug.h"
/*---------------------- Constant / Macro Definitions -----------------------*/
USART_TypeDef* DBG_USART    = USART2;
GPIO_TypeDef*  DBG_TX_PORT = GPIOA;
GPIO_TypeDef*  DBG_RX_PORT = GPIOA;

const uint32_t DBG_USART_CLK =RCC_APB1Periph_USART2;

const uint32_t DBG_TX_PORT_CLK = RCC_AHBPeriph_GPIOA;
const uint32_t DBG_RX_PORT_CLK = RCC_AHBPeriph_GPIOA;

const uint16_t DBG_TX_PIN = GPIO_Pin_2;
const uint16_t DBG_RX_PIN = GPIO_Pin_3;

const uint8_t  DBG_TX_PIN_SOURCE = GPIO_PinSource2;
const uint8_t  DBG_RX_PIN_SOURCE = GPIO_PinSource3;

const uint8_t  DBG_TX_AF = GPIO_AF_1;
const uint8_t  DBG_RX_AF = GPIO_AF_1;

/*������*/
const uint32_t DBG_BAUD = 115200;

#define DEBUG_CMD_DEFINE(cmd,dsc) {NULL,#cmd,dsc,_debug_cmd_fn_##cmd,0}
#define DEBUG_CMD_FN_DECLARE(cmd) extern void _debug_cmd_fn_##cmd(void)

/*----------------------- Type Declarations ---------------------------------*/
typedef struct
{
    uint8_t finish;
    uint32_t cmd_len;
    char cmd_buf[CMD_MAX_LEN];
} dbg_cmd_reveiver_t;//�������ݽ�����

typedef struct _DEBUG_CMD_STR
{
    struct _DEBUG_CMD_STR *next;
    char *cmdStr;//������
    char *cmdDescribe; //��������
    void (*doCMD)(void);//��Ӧ����
    uint32_t t_evt_mask; //��ʱ�¼�
} DEBUG_CMD_STR;//�������ݽ�����
/*------------------- Global Definitions and Declarations -------------------*/
//�ڴ˴��������ڵ��Եĺ���
DEBUG_CMD_FN_DECLARE(test);
DEBUG_CMD_FN_DECLARE(test_io);
DEBUG_CMD_FN_DECLARE(card_sfz);
DEBUG_CMD_FN_DECLARE(card_ic);
DEBUG_CMD_FN_DECLARE(card_app);
DEBUG_CMD_FN_DECLARE(card_cpu);


//�ڴ˴�ע�ᴮ�ڵ��Եĺ���
static DEBUG_CMD_STR debug_cmd_table[]=
{
	DEBUG_CMD_DEFINE(test, "test describ"),
	DEBUG_CMD_DEFINE(test_io, "test_io describ"),
	DEBUG_CMD_DEFINE(card_sfz,"test sfz card"),
	DEBUG_CMD_DEFINE(card_ic, "test ic card"),
	DEBUG_CMD_DEFINE(card_cpu,"test cpu card"),
	DEBUG_CMD_DEFINE(card_app,"test card_app"),
};

static dbg_cmd_reveiver_t dbg_cmd_reveiver;//�������ݽ�����
dbg_cmd_split_t dbg_cmd_split;        //�������������
LIST(debug_cmd_list);
/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
/******************************************************************************
* Name:      debug_cmd_exe
*
* Desc:      �������������������в���ע��ĺ�����ִ��
* InParam:
* OutParam:
* Return:    0->ʧ�ܣ�1->�ɹ�
* Note:      ��ֲʱ�����޸�
* Author:    LiuWeiQiang
* -----------------------------------------------------------------------------
* Log:   2018/03/31, Create this function by LiuWeiQiang
 ******************************************************************************/
uint8_t debug_cmd_exe(char *cmd)
{
    DEBUG_CMD_STR *debug_cmd = NULL;

    if (cmd == NULL) return 0;
    debug_cmd = list_head(debug_cmd_list);
    if (debug_cmd == NULL) 
    {
        return 0; //����Ϊ��(û��ע���κ�����)
    }
    for (debug_cmd = list_head(debug_cmd_list);
            debug_cmd != NULL;
            debug_cmd = list_item_next(debug_cmd))
    {
        if (strcmp(cmd, debug_cmd->cmdStr) == 0)
        {
//            debug("find cmd in debug_cmd_list success\r\n");
            (*debug_cmd->doCMD)();
            return 1;
        }
    }
//    debug("find cmd fail\r\n");
    return 0;
}
/******************************************************************************
* Name: 	 debug_cmd_help 
*
* Desc: 	 �����������
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 ��ֲʱ�����޸�
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2018/03/31, Create this function by LiuWeiQiang
 ******************************************************************************/
void debug_cmd_help(void)
{
     DEBUG_CMD_STR *debug_cmd = NULL;
     for (debug_cmd = list_head(debug_cmd_list);
            debug_cmd != NULL;
            debug_cmd = list_item_next(debug_cmd))
    {
        debug("  %-6s - [%c] %s\r\n", debug_cmd->cmdStr, debug_cmd->t_evt_mask?'T':'0', debug_cmd->cmdDescribe);
    }
}
/******************************************************************************
* Name: 	 recrive_dbg_cmd_callback 
*
* Desc: 	 �������ݽ��ջص�����
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 ��ֲʱ����Ҫ�޸�
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2018/03/31, Create this function by LiuWeiQiang
 ******************************************************************************/
void recrive_dbg_cmd_callback(uint8_t data)
{
    if(dbg_cmd_reveiver.finish != 0) return;
    dbg_cmd_reveiver.cmd_buf[dbg_cmd_reveiver.cmd_len++] = data;
    dbg_cmd_reveiver.cmd_buf[dbg_cmd_reveiver.cmd_len] = '\0';
    switch (data)
    {
    case '\n'://ͬ��
    case '\r':
    {
        dbg_cmd_reveiver.finish = 1;
    }
    break;
    case 0x08: //�˸�
    case 0x7F:
        //case 0x53: //DEL
    {
        if (dbg_cmd_reveiver.cmd_len)
        {
            dbg_cmd_reveiver.cmd_len--;
            dbg_cmd_reveiver.cmd_buf[dbg_cmd_reveiver.cmd_len] = '\0';
        }
    }
    break;

    default:break;
    }
}
/******************************************************************************
* Name: 	 hal_debug_real_time 
*
* Desc: 	 �����������
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 ��ֲʱ����Ҫ�޸�
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2018/03/31, Create this function by LiuWeiQiang
 ******************************************************************************/
void hal_debug_real_time(void)
{
//    uint8_t i;
    if(dbg_cmd_reveiver.finish == 1)
    {
        char * ptr;
        if(dbg_cmd_reveiver.cmd_len == 0) return;

//        debug("%s\r\n",dbg_cmd_reveiver.cmd_buf);
    	/* �������������*/
    	  memset(&dbg_cmd_split,0,sizeof(dbg_cmd_split_t));
        dbg_cmd_reveiver.cmd_buf[dbg_cmd_reveiver.cmd_len] = '\0';
        ptr = strtok(dbg_cmd_reveiver.cmd_buf," ");
        while(ptr)
        {
            dbg_cmd_split.arg[dbg_cmd_split.arg_cnt++] = ptr;
            if(dbg_cmd_split.arg_cnt >= ARG_MAX_CNT+1)
            {
                break;
            }
            ptr = strtok(NULL," ");
        }
				//��һ������Ϊ������н���
        if(debug_cmd_exe(dbg_cmd_split.arg[0]))
        {
//            debug("cmd deal success!\r\n");
        }
				//���뻻�з�����# >���л���
				else if(memcmp(dbg_cmd_split.arg[0],"\r\n",2)==0 \
								|| memcmp(dbg_cmd_split.arg[0],"\n",1)==0 \
								|| memcmp(dbg_cmd_split.arg[0],"\r",1)==0)
				{
					debug("\r\n# >");
				}
        else
        {
            debug("Unknown cmd!\r\n");
            debug_cmd_help();
        }
				
//        for(i=0;i<dbg_cmd_resolver.arg_cnt;i++)
//        {
//            debug("%s\r\n",dbg_cmd_resolver.arg[i]);
//        }
        memset(&dbg_cmd_reveiver,0,sizeof(dbg_cmd_reveiver_t));
    }
}

/******************************************************************************
* Name: 	 get_dbg_cmd_resolver 
*
* Desc: 	 ��ȡ������Ϣ
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 ��ֲʱ�����޸�
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2018/03/31, Create this function by LiuWeiQiang
 ******************************************************************************/
dbg_cmd_split_t *get_dbg_cmd_split(void)
{
    return &dbg_cmd_split;
}

/******************************************************************************
* Name: 	 cmd_resolver_init 
*
* Desc: 	 �����������ʼ������
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 ��ֲʱ�����޸�
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2018/03/31, Create this function by LiuWeiQiang
 ******************************************************************************/
void cmd_resolver_init(void)
{
    uint16_t index;
    
    list_init(debug_cmd_list);
    for (index = 0; index < (sizeof(debug_cmd_table) / sizeof(debug_cmd_table[0])); index++)
    {
        list_add(debug_cmd_list,&debug_cmd_table[index]);
    }
}
///////////////////////////////////////////////////////////////////////////////
//////////////////////���º�����ֲʱ��Ҫ�޸�///////////////////////////////////
/******************************************************************************
* Name: 	 putCh 
*
* Desc: 	 �������һ���ֽ�
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 ��ֲʱ��Ҫ�޸�
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2018/03/31, Create this function by LiuWeiQiang
 ******************************************************************************/
void putCh ( uint8_t ch )
{
	while ( ! ( ( DBG_USART->ISR ) & ( 1<<7 ) ) );
	DBG_USART->TDR=ch;
}
/******************************************************************************
* Name: 	 debug_uart_disable 
*
* Desc: 	 ���ô����ж�
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 ��ֲʱ��Ҫ�޸�
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2018/03/31, Create this function by LiuWeiQiang
 ******************************************************************************/
void debug_uart_disable(void)
{
    NVIC_InitTypeDef    NVIC_InitStructure;
	
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
		NVIC_Init ( &NVIC_InitStructure );

    USART_ITConfig(DBG_USART, USART_IT_TC, DISABLE);
    USART_ITConfig(DBG_USART, USART_IT_RXNE, DISABLE);
}

/******************************************************************************
* Name: 	 debug_uart_init 
*
* Desc: 	 ���ڳ�ʼ��
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 ��ֲ��Ҫ�޸�
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2018/03/31, Create this function by LiuWeiQiang
 ******************************************************************************/
void debug_uart_init(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef 	NVIC_InitStructure;

	
	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd ( DBG_TX_PORT_CLK | DBG_RX_PORT_CLK, ENABLE );

	/* Enable USART clock */
	if ( DBG_USART == USART1 )
	{
		RCC_APB2PeriphClockCmd ( DBG_USART_CLK, ENABLE );
	}
	else if ( DBG_USART == USART2 )
	{
		RCC_APB1PeriphClockCmd ( DBG_USART_CLK, ENABLE );
	}
	/* Connect PXx to USARTx_Tx */
	GPIO_PinAFConfig ( DBG_TX_PORT, DBG_TX_PIN_SOURCE, DBG_TX_AF );

	/* Connect PXx to USARTx_Rx */
	GPIO_PinAFConfig ( DBG_RX_PORT, DBG_RX_PIN_SOURCE, DBG_RX_AF );

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = DBG_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init ( DBG_TX_PORT, &GPIO_InitStructure );

	/* Configure USART Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = DBG_RX_PIN;
	GPIO_Init ( DBG_RX_PORT, &GPIO_InitStructure );

	USART_InitStructure.USART_BaudRate = DBG_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	/* USART configuration */
	USART_Init ( DBG_USART, &USART_InitStructure );

	USART_ClearITPendingBit ( DBG_USART, USART_IT_TC );
	USART_ITConfig ( DBG_USART,USART_IT_TC,ENABLE );
	USART_ITConfig ( DBG_USART,USART_IT_RXNE,ENABLE );
	/* Enable USART */
	USART_Cmd ( DBG_USART, ENABLE );

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init ( &NVIC_InitStructure );
	cmd_resolver_init();
}

/******************************************************************************
* Name: 	 USART1_IRQHandler 
*
* Desc: 	 �����жϴ�����
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 ��ֲʱ��Ҫ�޸�
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2018/03/31, Create this function by LiuWeiQiang
 ******************************************************************************/
void USART2_IRQHandler ( void )
{
	if ( USART_GetFlagStatus ( USART2, USART_FLAG_ORE ) != RESET )
	{
		USART_ClearITPendingBit ( USART2, USART_IT_ORE );
	}
	if ( USART_GetITStatus ( USART2, USART_IT_TC ) != RESET )
	{
		USART_ClearITPendingBit ( USART2, USART_IT_TC );
	}
	if ( USART_GetITStatus ( USART2, USART_IT_RXNE ) != RESET )
	{

//				uint8_t data;
//				data = USART_ReceiveData(USART2);
//				USART_SendData(USART2, data);
		recrive_dbg_cmd_callback ( USART_ReceiveData ( USART2 ) );
	}
}
/*---------------------------------------------------------------------------*/


