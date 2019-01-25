/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 fm17x2_drv.c
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/05/24
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/05/24, Liuwq create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "hal_fm17x2.h"
#include "fm17x2_drv.h"
#include <string.h>
#include "debug.h"
/*------------------- Global Definitions and Declarations -------------------*/
#define SFZ_MODCONDUCTANCE 0X0E //身份证调制深度控制值

static uint16_t delay_time_max;
/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
void delayNms ( unsigned int x )
{
	uint16_t y = 0;
	while ( x-- )
	{
		y = 4000;
		while ( y-- )
		{
			;
		}
	}
}

void reg_write ( uint8_t reg, uint8_t val )
{
	SPI_CS_EN();
	reg <<= 1;
	reg &= 0x7f;
	SPI_READ_WRITE ( reg );
	SPI_READ_WRITE ( val );
	SPI_CS_DIS();
}


uint8_t reg_read ( uint8_t reg )
{
	uint8_t val;
	SPI_CS_EN();
	reg <<= 1;
	reg |= 0x80;
	SPI_READ_WRITE ( reg );
	val = SPI_READ_WRITE ( 0x00 );
	SPI_CS_DIS();
	return val;
}

static uint8_t Clear_FIFO ( void )
{
	uint8_t	temp;
	uint16_t	i;
	temp = reg_read ( Control );			/* 清空FIFO */
	temp = ( temp | 0x01 );
	reg_write ( Control, temp );
	for ( i = 0; i < RF_TimeOut; i++ ) /* 检查FIFO是否被清空 */
	{
		temp = reg_read ( FIFO_Length );
		if ( temp == 0 )
		{
			return TRUE;
		}
	}
	return FALSE;
}

static void Write_FIFO ( uint8_t count, uint8_t* buff )
{
	uint8_t	i;

	for ( i = 0; i < count; i++ )
	{
		reg_write ( FIFO, * ( buff + i ) );
	}
}

uint8_t Read_FIFO ( uint8_t* buff )
{
	uint8_t	len;
	uint8_t	i;

	len = reg_read ( FIFO_Length );
	if ( len == 0 )
	{
		return 0;
	}

	if ( len >= FIFO_MAX_BUF )
	{

		len = FIFO_MAX_BUF;
	}

	for ( i = 0; i < len; i++ )
	{
		* ( buff + i ) = reg_read ( FIFO );
	}

	return len;
}

void PcdInit ( uint8_t mode )
{
	uint16_t i;

	FM1702_RST_EN(); //reset;
	delayNms ( 2 );
	FM1702_RST_DIS(); //reset;
	delayNms ( 1 );

//	while(reg_read(Command) != 0);		        /* 等待Command = 0,FM1702复位成功 */

	for ( i = 0; i < 0x800; i++ ) /* 延时 */
	{
		if ( reg_read ( Command ) == 0x00 )
		{
			break;
		}
	}

	reg_write ( Page_Sel, 0x80 );

	for ( i = 0; i < 0x80; i++ ) /* 延时 */
	{
		if ( reg_read ( Command ) == 0x00 )
		{
			reg_write ( Page_Sel, 0x00 );
			break;
		}
	}
	reg_write ( TimerClock, 0x0b );		 //address 2AH    /* 定时器周期设置寄存器 */
	reg_write ( TimerControl, 0x02 );	 //address 2BH	/* 定时器控制寄存器 */
	reg_write ( TimerReload, 0x42 );	   //address 2CH    /* 定时器初值寄存器 */
	reg_write ( InterruptEn, 0x7f );		 //address 06H    /* 中断使能/禁止寄存器 */
	reg_write ( Int_Req, 0x7f );        //address 07H    /* 中断请求标识寄存器 */
	reg_write ( MFOUTSelect, 0x02 );		 //address 26H    /* mf OUT 选择配置寄存器 */

	//设置调制器的输入源为内部编码器, 并且设置TX1和TX2
	reg_write ( TxControl, 0x5b );		 //address 11H    /* 发送控制寄存器 */
	reg_write ( RxControl2, 0x01 );
	reg_write ( RxWait, 0x05 );
	reg_write ( TypeSH, 0x00 );

	reg_write ( CWConductance, 0x3f );
	reg_write ( CoderControl, 0x19 );
	reg_write ( ModWidth, 0x13 );
	reg_write ( ModConductance, 0x3f );
//	reg_write(TypeBFraming,0x00);
	reg_write ( RxControl1, 0x73 );
	reg_write ( DecoderControl, 0x08 );
	reg_write ( BitPhase, 0xad );
	reg_write ( RxThreshold, 0xff );
	reg_write ( ChannelRedundancy, 0x03 );
//    reg_write(BPSKDemControl,0x00);
	reg_write ( ClockQControl, 0x00 );
	reg_write ( CRCPresetLSB, 0x63 );
	reg_write ( CRCPresetMSB, 0x63 );
	reg_write ( FIFOLevel, 0x08 );
	reg_write ( IRQPinConfig, 0x02 );

	if ( mode == TYPEB_MODE ) //TYPEB模式
	{
		reg_write ( BPSKDemControl, 0x1e );
		reg_write ( CoderControl, 0x20 );
		reg_write ( TypeBFraming, 0x05 );
		reg_write ( DecoderControl, 0x19 );
		reg_write ( ChannelRedundancy, 0x24 );
		reg_write ( TxControl, 0x4b );
//		reg_write(ModConductance,0xaf);
		reg_write ( ModConductance, SFZ_MODCONDUCTANCE );
	}
}

void SetTime_delay ( unsigned long delaytime )
{
	unsigned long temp, TimeLoad;
	uint8_t Prescalar;
	temp = 1;
	Prescalar = 0;
	TimeLoad = 0;
	while ( 1 )
	{
		temp = temp * 2;
		TimeLoad = ( delaytime * 13560 ) / temp ;
		Prescalar++;
		if ( TimeLoad < 256 )
		{
			break;
		}
		if ( Prescalar > 21 )
		{
			break;
		}

	}
	if ( Prescalar < 21 )
	{
		reg_write ( TimerClock, Prescalar );
		reg_write ( TimerReload, TimeLoad );
	}
	else
	{
		reg_write ( TimerClock, 21 );	//默认值为最大延时时间
		reg_write ( TimerReload, 255 );		//默认值为最大延时时间
	}

	/* 延时时间10ms  30ms   100ms 3档*/
	if ( delaytime < 11 )
	{
		delay_time_max = RF_TimeOut_10ms;
	}
	else if ( delaytime < 31 )
	{
		delay_time_max = RF_TimeOut_30ms;
	}
	else
	{
		delay_time_max = RF_TimeOut_100ms;
	}

}

uint8_t Command_Send ( uint8_t count, uint8_t* buff, uint8_t Comm_Set )
{

	uint16_t	i;
	uint8_t temp;

	reg_write ( Int_Req, 0X7f );
	reg_write ( InterruptEn, 0Xbf );
	reg_write ( TimerControl, 0X02 ); //数据发送结束后，定时器开始计时

	reg_write ( Command, 0x00 );
	Clear_FIFO();
	Write_FIFO ( count, buff );
//	temp = reg_read(MFOUTSelect);
	reg_write ( Command, Comm_Set );		/* 命令执行 */

	for ( i = 0; i < delay_time_max; i++ ) /* 检查命令执行否 */
	{
//      temp = reg_read(MFOUTSelect);	//??
		temp = reg_read ( Command );
		if ( temp == 0x00 )
		{
			return TRUE;
		}
		/* 中断超时 ...added by yeqizhi, 2016-12-16 */
		temp = reg_read ( Int_Req );
		if ( temp & 0x04 ) //IdelIRq
		{
			return TRUE;
		}
		if ( temp & 0x20 ) //TimerIRq  定时器时间用尽
		{
			return FALSE;
		}
	}
	return FALSE;
}
/******************************************************************************
* Name: 	 cmd_send_recv 
*
* Desc: 	 命令发送并接收数据
* Param: 	 
* Return: 	 1->失败，0->成功
* Global: 	 
* Note: 	 
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/05/25, Create this function by Liuwq
 ******************************************************************************/
uint8_t cmd_send_recv(uint8_t s_len,uint8_t *s_buf,uint8_t *r_len,uint8_t *r_buf)
{
    uint8_t result;
    if(s_len<1 || s_buf==NULL || r_len==NULL || r_buf==NULL) return FALSE;
    reg_write(ChannelRedundancy,0x0f);
    result = Command_Send(s_len, s_buf, Transceive);
	
    if(result == TRUE)
    {
			result = 0;
//			debug("Command_Send success!\r\n");
			*r_len = reg_read(FIFO_Length);
//			debug("get ask len = %d\r\n",*r_len);
			if(*r_len < FIFO_MAX_BUF)
			{
					Read_FIFO(r_buf);
			}
			else
			{
					result = 1;
			}
    }
		else
		{
//			debug("Command_Send fail!\r\n");
			result = 1;
		}
    return result;
}

uint8_t Load_key ( uint8_t* uncoded_keys )
{
	uint8_t  status;
	uint8_t  coded_keys[12];
	uint8_t  cnt = 0;
	uint8_t  ln  = 0;
	uint8_t  hn  = 0;

	for ( cnt = 0; cnt < 6; cnt++ )        // 转换密钥格式
	{
		ln = uncoded_keys[cnt] & 0x0F;
		hn = uncoded_keys[cnt] >> 4;
		coded_keys[cnt * 2 + 1] = ( ~ln << 4 ) | ln;
		coded_keys[cnt * 2 ] = ( ~hn << 4 ) | hn;
	}
	SetTime_delay ( 8 );
	status = Command_Send ( 12, coded_keys, LoadKey ); //* 将密钥从EEPROM复制到KEY缓存
	status = reg_read ( ErrorFlag ) & 0x40;

	if ( status == 0x40 )
	{
		return status;
	}
	return 0;
}

void ISO14443A_Save_UID ( uint8_t row, uint8_t col, uint8_t length,uint8_t *recv_buf,uint8_t *uid)
{
	uint8_t	i;
	uint8_t	temp;
	uint8_t	temp1;

	if ( ( row == 0x00 ) && ( col == 0x00 ) )
	{
		for ( i = 0; i < length; i++ )
		{
			uid[i] = recv_buf[i];
		}
	}
	else
	{
		temp = recv_buf[0];
		temp1 = uid[row - 1];
		switch ( col )
		{
			case 0:
				temp1 = 0x00;
				row = row + 1;
				break;
			case 1:
				temp = temp & 0xFE;
				temp1 = temp1 & 0x01;
				break;
			case 2:
				temp = temp & 0xFC;
				temp1 = temp1 & 0x03;
				break;
			case 3:
				temp = temp & 0xF8;
				temp1 = temp1 & 0x07;
				break;
			case 4:
				temp = temp & 0xF0;
				temp1 = temp1 & 0x0F;
				break;
			case 5:
				temp = temp & 0xE0;
				temp1 = temp1 & 0x1F;
				break;
			case 6:
				temp = temp & 0xC0;
				temp1 = temp1 & 0x3F;
				break;
			case 7:
				temp = temp & 0x80;
				temp1 = temp1 & 0x7F;
				break;
			default:
				break;
		}

		recv_buf[0] = temp;
		uid[row - 1] = temp1 | temp;
		for ( i = 1; i < length; i++ )
		{
			uid[row - 1 + i] = recv_buf[i];
		}
	}
}

void ISO14443A_Set_BitFraming ( uint8_t row, uint8_t col,uint8_t *recv_buf)
{
	switch ( row )
	{
		case 0:
			recv_buf[1] = 0x20;
			break;
		case 1:
			recv_buf[1] = 0x30;
			break;
		case 2:
			recv_buf[1] = 0x40;
			break;
		case 3:
			recv_buf[1] = 0x50;
			break;
		case 4:
			recv_buf[1] = 0x60;
			break;
		default:
			break;
	}

	switch ( col )
	{
		case 0:
			reg_write ( Bit_Frame, 0x00 );
			break;
		case 1:
			reg_write ( Bit_Frame, 0x11 );
			recv_buf[1] = ( recv_buf[1] | 0x01 );
			break;
		case 2:
			reg_write ( Bit_Frame, 0x22 );
			recv_buf[1] = ( recv_buf[1] | 0x02 );
			break;
		case 3:
			reg_write ( Bit_Frame, 0x33 );
			recv_buf[1] = ( recv_buf[1] | 0x03 );
			break;
		case 4:
			reg_write ( Bit_Frame, 0x44 );
			recv_buf[1] = ( recv_buf[1] | 0x04 );
			break;
		case 5:
			reg_write ( Bit_Frame, 0x55 );
			recv_buf[1] = ( recv_buf[1] | 0x05 );
			break;
		case 6:
			reg_write ( Bit_Frame, 0x66 );
			recv_buf[1] = ( recv_buf[1] | 0x06 );
			break;
		case 7:
			reg_write ( Bit_Frame, 0x77 );
			recv_buf[1] = ( recv_buf[1] | 0x07 );
			break;
		default:
			break;
	}
}
void fm17x2_drv_init ( void )
{
	hal_fm17x2_init();
	PcdInit ( TYPEA_MODE );
}
/*---------------------------------------------------------------------------*/
