/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 fm17x2_reg.h
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
#ifndef _FM17X2_REG_H_
#define _FM17X2_REG_H_


/*------------------------------- Includes ----------------------------------*/


/*----------------------------- Global Defines ------------------------------*/
/* FM1702������ */
#define Transceive	0x1E			/* ���ͽ������� */
#define Transmit	0x1a			/* �������� */
#define ReadE2		0x03			/* ��FM1702 EEPROM���� */
#define WriteE2		0x01			/* дFM1702 EEPROM���� */
#define LoadKeyE2	0x0b			/* ����Կ��EEPROM���Ƶ�KEY���� */
#define LoadKey		0x19			/* ����Կ��FIFO���渴�Ƶ�KEY���� */

#define RF_TimeOut_1MS 	0x36			/* ����������ʱʱ�� 1MS*/
#define RF_TimeOut	0x1f4		/* ����������ʱʱ��  0x1f4 == 8.5ms   0x24c == 10ms*/
#define RF_TimeOut_10ms 0x25c
#define RF_TimeOut_30ms 0x438
#define RF_TimeOut_100ms 0xe5b0

#define Req		0x01
#define Sel		0x02

/* ��Ƭ���Ͷ��嶨�� */
#define TYPEA_MODE	0			/* TypeAģʽ */
#define TYPEB_MODE	1			/* TypeBģʽ */

/* ����������붨�� ERR CODE  */
#define FM1702_OK		0		/* ��ȷ */
#define FM1702_NOTAGERR		1		/* �޿� */
#define FM1702_CRCERR		2		/* ��ƬCRCУ����� */
#define FM1702_EMPTY		3		/* ��ֵ������� */
#define FM1702_AUTHERR		4		/* ��֤���ɹ� */
#define FM1702_PARITYERR	5		/* ��Ƭ��żУ����� */
#define FM1702_CODEERR		6		/* ͨѶ����(BCCУ���) */
#define FM1702_SERNRERR		8		/* ��Ƭ���кŴ���(anti-collision ����) */
#define FM1702_SELECTERR	9		/* ��Ƭ���ݳ����ֽڴ���(SELECT����) */
#define FM1702_NOTAUTHERR	10		/* ��Ƭû��ͨ����֤ */
#define FM1702_BITCOUNTERR	11		/* �ӿ�Ƭ���յ���λ������ */
#define FM1702_BYTECOUNTERR	12		/* �ӿ�Ƭ���յ����ֽ����������������Ч */
#define FM1702_RESTERR		13		/* ����restore�������� */
#define FM1702_TRANSERR		14		/* ����transfer�������� */
#define FM1702_WRITEERR		15		/* ����write�������� */
#define FM1702_INCRERR		16		/* ����increment�������� */
#define FM1702_DECRERR		17		/* ����decrement�������� */
#define FM1702_READERR		18		/* ����read�������� */
#define FM1702_LOADKEYERR	19		/* ����LOADKEY�������� */
#define FM1702_FRAMINGERR	20		/* FM1702֡���� */
#define FM1702_REQERR		21		/* ����req�������� */
#define FM1702_SELERR		22		/* ����sel�������� */
#define FM1702_ANTICOLLERR	23		/* ����anticoll�������� */
#define FM1702_INTIVALERR	24		/* ���ó�ʼ���������� */
#define FM1702_READVALERR	25		/* ���ø߼�����ֵ�������� */
#define FM1702_DESELECTERR	26
#define FM1702_CMD_ERR		42		/* ������� */

/* FM1702�Ĵ������� */
#define Page_Sel		0x00	/* ҳд�Ĵ��� */
#define Command			0x01	/* ����Ĵ��� */
#define FIFO			0x02	/* 64�ֽ�FIFO�������������Ĵ��� */
#define PrimaryStatus		0x03	/* ��������������FIFO��״̬�Ĵ���1 */
#define FIFO_Length		0x04	/* ��ǰFIFO���ֽ����Ĵ��� */
#define SecondaryStatus		0x05	/* ����״̬�Ĵ���2 */
#define InterruptEn		0x06	/* �ж�ʹ��/��ֹ�Ĵ��� */
#define Int_Req			0x07	/* �ж������ʶ�Ĵ��� */
#define Control			0x09	/* ���ƼĴ��� */
#define ErrorFlag		0x0A	/* ����״̬�Ĵ��� */
#define CollPos			0x0B	/* ��ͻ���Ĵ��� */
#define TimerValue		0x0c	/* ��ʱ����ǰֵ */
#define Bit_Frame		0x0F	/* λ֡�����Ĵ��� */
#define TxControl		0x11	/* ���Ϳ��ƼĴ��� */
#define CWConductance		0x12	/* ѡ�����TX1��TX2�������ߵ��迹 */
#define ModConductance		0x13	/* ������������迹 */
#define CoderControl		0x14	/* �������ģʽ��ʱ��Ƶ�� */
#define ModWidth         0x15      /*ѡ���ز����ƿ�� */
#define TypeBFraming	0x17	/* ����ISO14443B֡��ʽ */
#define RxControl1		0x19	/*�����������*/
#define DecoderControl		0x1a	/* ������ƼĴ��� */
#define BitPhase		0x1b	/* �����������ͽ�����ʱ����� */
#define RxThreshold		0x1c	/* ѡ��bit �������ֵ */
#define BPSKDemControl		0x1d	/* BPSK ���������ƼĴ��� */
#define RxControl2		0x1e	/* ������Ƽ�ѡ�����Դ */
#define ClockQControl		0x1f	/* ʱ�Ӳ������ƼĴ��� */
#define RxWait			0x21	/* ѡ����ͽ���֮���ʱ���� */
#define ChannelRedundancy	0x22	/* RFͨ������ģʽ���üĴ��� */
#define CRCPresetLSB		0x23
#define CRCPresetMSB		0x24
#define MFOUTSelect		0x26	/* mf OUT ѡ�����üĴ��� */
#define FIFOLevel		0x29	/* ����FIFO ������� */
#define TimerClock		0x2a	/* ��ʱ���������üĴ��� */
#define TimerControl		0x2b	/* ��ʱ�����ƼĴ��� */
#define TimerReload		0x2c	/* ��ʱ����ֵ�Ĵ��� */
#define IRQPinConfig		0x2d	/* IRQ ������� */
#define TypeSH			0x31	/* �Ϻ���׼ѡ��Ĵ��� */
#define TestDigiSelect		0x3d	/* ���Թܽ����üĴ��� */


/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/

#endif //_FM17X2_REG_H_

