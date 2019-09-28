/********************************************************************************
 Author : CAC (China Applications Support Team) 

 Date :   January, 2012

 File name :   AD7192.h

 Description :	 Use the GPIO to simulate the SPI communication of AD7192

 Hardware plateform : 	ADuC7026 and AD7190/92EBZ
********************************************************************************/

#ifndef AD7192_H
#define AD7192_H

#include "stm32f10x.h"
#include <stdio.h>
#include "usart.h"
#include "delay.h"
#include "adg1409.h"
#include "ad7192_driver.h"
#include "time.h"
#include "Modbus.h"

//Register Map and bit definition

//Communications Register
//ͨ�żĴ�����������һ������   	8λ
#define	WEN					0x00   //д��ʹ�ܣ�0ʹ��
#define WEN_DIS				0x80   //д��ʧ��
#define RW_W				0x00	 //��ʾ��һ������д����
#define	RW_R				0x40   //��ʾ��һ������������

#define REG_COM_STA	    	0x00	 //״̬�Ĵ���   8λ
#define	REG_MODE			0x01	 //ģʽ�Ĵ���   24λ
#define	REG_CONF			0x02	//���üĴ���			24λ
#define	REG_DATA			0x03	//���ݼĴ���			24λ
#define	REG_ID				0x04	//ID�Ĵ���
#define	REG_GPOCON		    0x05	//GPOCON �Ĵ���   8λ
#define	REG_OFFSET		    0x06	//ʧ���Ĵ���     24 λ
#define	REG_FS				0x07	//�����̼Ĵ���     24 λ

#define	CREAD_EN			0x04	//������ȡ����   
#define	CREAD_DIS			0x00	

//Status Register
#define	RDY_H				0x80  //ADC����
#define	RDY_L				0x00
#define	ERR_H				0x40	//ADC����
#define	ERR_L				0x00
#define	NOREF_H				0x20	//���ⲿ��׼��ѹλ
#define	NOREF_L				0x00
#define	PARITY_H			0x10	//���ݼĴ�������żУ��
#define	PARITY_L			0x00
//ָʾ�������ݼĴ�������ת�����ʱѡ������һͨ����
#define	CHDST_AIN1_AIN2	    0x00
#define	CHDST_AIN3_AIN4	    0x01
#define	CHDST_TEMP			0x02
#define	CHDST_AIN2_AIN2	    0x03
#define	CHDST_AIN1_COM	    0x04
#define	CHDST_AIN2_COM	    0x05
#define	CHDST_AIN3_COM	    0x06
#define	CHDST_AIN4_COM	    0x07

//Mode Register  	24λ�Ĵ���
#define	MODE_CONT				0x000000    //����ת��
#define	MODE_SING				0x200000	//����ת��
#define	MODE_IDLE				0x400000	//����ģʽ
#define	MODE_PD					0x600000	//�ض�ģʽ
#define	MODE_INZCL				0x800000	//�ڲ����ƽУ׼
#define	MODE_INFCL				0xA00000	//�ڲ�������У׼
#define	MODE_SYSZCL				0xC00000	//ϵͳ���ƽУ׼
#define	MODE_SYSFCL				0xE00000	//ϵͳ������У׼
#define	DAT_STA_EN				0x100000	//ʹ��ÿ�����ݼĴ���������֮����״̬�Ĵ�������
#define	DAT_STA_DIS				0x000000	  
#define	EXT_XTAL				0x000000	//�ⲿ����
#define	EXT_CLK					0x040000	//�ⲿʱ��
#define	INCLK_MCLK2TRI		    0x080000	//4.92 MHz �ڲ�ʱ�ӡ�MCLK2 ����Ϊ��̬��
#define	INCLK_MCLK2EN			0x0C0000	//4.92 MHz �ڲ�ʱ�ӡ��ڲ�ʱ�ӿɴ� MCLK2 ��á�
#define	SINC_4					0x000000	//Sinc 4 �˲���ѡ��
#define	SINC_3					0x008000	//sinc 3 �˲���
#define	ENPAR_EN				0x002000	//ʹ����żУ��λ
#define	ENPAR_DIS				0x000000
#define	CLK_DIV_2				0x001000	//ʱ�� 2 ��Ƶ
#define	CLK_DIV_DIS				0x000000
#define	SINGLECYCLE_EN		    0x000800	//������ת��ʹ��λ
#define	SINGLECYCLE_DIS		    0x000000
#define	REJ60_EN				0x000400
#define	REJ60_DIS				0x000000
										
//Configuration Register			
#define CHOP_EN					0x800000		//ն��ʹ��λ
#define	CHOP_DIS				0x000000
#define	REF_IN1					0x000000		//��׼��ѹѡ��λ
#define	REF_IN2					0x100000
//ͨ��ѡ��λ
#define	AIN1_AIN2				0x000100
#define	AIN3_AIN4				0x000200
#define	TEMP					0x000400
#define	AIN2_AIN2				0x000800
#define	AIN1_COM				0x001000
#define	AIN2_COM				0x002000
#define	AIN3_COM				0x004000
#define	AIN4_COM				0x008000

#define	BURN_EN					0x000080//��λ�� 1 ��ʹ���ź�·���е� 500 nA ����Դ
#define	BURN_DIS				0x000000
#define	REFDET_EN				0x000040//ʹ�ܻ�׼��ѹ��⹦��
#define	REFDET_DIS		    	0x000000
#define	BUF_EN					0x000010//ʹ��ģ������˵Ļ�����
#define	BUF_DIS					0x000000
#define	UB_UNI					0x000008//����ѡ��λ����λ�� 1 ʱ��ѡ�񵥼��Թ���ģʽ����λ�� 0 ʱ��ѡ��˫���Թ���ģʽ��
#define UB_BI					0x000000

#define	GAIN_1					0x000000
#define	GAIN_8					0x000003
#define	GAIN_16					0x000004
#define	GAIN_32					0x000005
#define	GAIN_64					0x000006
#define	GAIN_128				0x000007

//GPOCON Register
#define BPDSW_CLOSE			    0x40//���ŹضϿ��ؿ���λ
#define	BPDSW_OPEN			    0x00
#define	GP32EN					0x20//������� P3 ��������� P2 ʹ��
#define	GP32DIS					0x00
#define	GP10EN					0x10//������� P1 ��������� P0 ʹ��
#define	GP10DIS					0x00
#define	P3DAT_H					0x08//������� P3
#define	P3DAT_L					0x00
#define	P2DAT_H					0x04
#define	P2DAT_L					0x00
#define	P1DAT_H					0x02
#define	P1DAT_L					0x00
#define	P0DAT_H					0x01
#define	P0DAT_L					0x00

//No Operation
#define	NOP							0x00									



extern unsigned long int AD7192Registers[8];
//extern unsigned long int AD7192Data;

unsigned char WriteToAD7192ViaSPI(const unsigned char RegisterStartAddress, const unsigned char NumberOfRegistersToWrite, unsigned long int *DataBuffer, const unsigned char OffsetInBuffer);
unsigned char ReadFromAD7192ViaSPI(const unsigned char RegisterStartAddress, const unsigned char NumberOfRegistersToRead, unsigned long int *DataBuffer, const unsigned char OffsetInBuffer);
void AD7192Initialization(void);
void AD7192SoftwareReset(void);
void AD7192InternalZeroScaleCalibration(void);
void AD7192InternalFullScaleCalibration(void);
void AD7192ExternalZeroScaleCalibration(void);
void AD7192ExternalFullScaleCalibration(void);
void AD7192StartContinuousRead(void);	 
unsigned long int AD7192ContinuousRead(void);
void AD7192ExitContinuousRead(void);
void AD7192StartSingleConvertion(unsigned long int Channels);
void AD7192StartContinuousConvertion(unsigned long int Channels);
unsigned long int AD7192ReadConvertingData(void);
unsigned long int AD7192ReadTemperature(void);
float RealTemperature(unsigned long int TemperatureCode);

void ReadTemperature(void);
void StartSingleConvertion(unsigned long int Channels,ADG1409ModeTypedef ADG1409ModeChoose);
void StartContinuousConvertion(unsigned long int Channels,unsigned short NumberOfDataToContinuousConvertion);
void StartContinousRead(ADG1409ModeTypedef ADG1409ModeChoose,unsigned short NumberOfDataToContinuousRead);

void ModbusResponseReadChannelOnce(void);
void ModbusResponseReadChannelContinous(void);
#endif

