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
//通信寄存器，决定下一个操作   	8位
#define	WEN					0x00   //写入使能，0使能
#define WEN_DIS				0x80   //写入失能
#define RW_W				0x00	 //表示下一个操作写操作
#define	RW_R				0x40   //表示下一个操作读操作

#define REG_COM_STA	    	0x00	 //状态寄存器   8位
#define	REG_MODE			0x01	 //模式寄存器   24位
#define	REG_CONF			0x02	//配置寄存器			24位
#define	REG_DATA			0x03	//数据寄存器			24位
#define	REG_ID				0x04	//ID寄存器
#define	REG_GPOCON		    0x05	//GPOCON 寄存器   8位
#define	REG_OFFSET		    0x06	//失调寄存器     24 位
#define	REG_FS				0x07	//满量程寄存器     24 位

#define	CREAD_EN			0x04	//连续读取数据   
#define	CREAD_DIS			0x00	

//Status Register
#define	RDY_H				0x80  //ADC就绪
#define	RDY_L				0x00
#define	ERR_H				0x40	//ADC错误
#define	ERR_L				0x00
#define	NOREF_H				0x20	//无外部基准电压位
#define	NOREF_L				0x00
#define	PARITY_H			0x10	//数据寄存器的奇偶校验
#define	PARITY_L			0x00
//指示产生数据寄存器所含转换结果时选定了哪一通道。
#define	CHDST_AIN1_AIN2	    0x00
#define	CHDST_AIN3_AIN4	    0x01
#define	CHDST_TEMP			0x02
#define	CHDST_AIN2_AIN2	    0x03
#define	CHDST_AIN1_COM	    0x04
#define	CHDST_AIN2_COM	    0x05
#define	CHDST_AIN3_COM	    0x06
#define	CHDST_AIN4_COM	    0x07

//Mode Register  	24位寄存器
#define	MODE_CONT				0x000000    //连续转换
#define	MODE_SING				0x200000	//单次转换
#define	MODE_IDLE				0x400000	//空闲模式
#define	MODE_PD					0x600000	//关断模式
#define	MODE_INZCL				0x800000	//内部零电平校准
#define	MODE_INFCL				0xA00000	//内部满量程校准
#define	MODE_SYSZCL				0xC00000	//系统零电平校准
#define	MODE_SYSFCL				0xE00000	//系统满量程校准
#define	DAT_STA_EN				0x100000	//使能每次数据寄存器读操作之后传输状态寄存器内容
#define	DAT_STA_DIS				0x000000	  
#define	EXT_XTAL				0x000000	//外部晶振
#define	EXT_CLK					0x040000	//外部时钟
#define	INCLK_MCLK2TRI		    0x080000	//4.92 MHz 内部时钟。MCLK2 引脚为三态。
#define	INCLK_MCLK2EN			0x0C0000	//4.92 MHz 内部时钟。内部时钟可从 MCLK2 获得。
#define	SINC_4					0x000000	//Sinc 4 滤波器选择
#define	SINC_3					0x008000	//sinc 3 滤波器
#define	ENPAR_EN				0x002000	//使能奇偶校验位
#define	ENPAR_DIS				0x000000
#define	CLK_DIV_2				0x001000	//时钟 2 分频
#define	CLK_DIV_DIS				0x000000
#define	SINGLECYCLE_EN		    0x000800	//单周期转换使能位
#define	SINGLECYCLE_DIS		    0x000000
#define	REJ60_EN				0x000400
#define	REJ60_DIS				0x000000
										
//Configuration Register			
#define CHOP_EN					0x800000		//斩波使能位
#define	CHOP_DIS				0x000000
#define	REF_IN1					0x000000		//基准电压选择位
#define	REF_IN2					0x100000
//通道选择位
#define	AIN1_AIN2				0x000100
#define	AIN3_AIN4				0x000200
#define	TEMP					0x000400
#define	AIN2_AIN2				0x000800
#define	AIN1_COM				0x001000
#define	AIN2_COM				0x002000
#define	AIN3_COM				0x004000
#define	AIN4_COM				0x008000

#define	BURN_EN					0x000080//此位置 1 将使能信号路径中的 500 nA 电流源
#define	BURN_DIS				0x000000
#define	REFDET_EN				0x000040//使能基准电压检测功能
#define	REFDET_DIS		    	0x000000
#define	BUF_EN					0x000010//使能模拟输入端的缓冲器
#define	BUF_DIS					0x000000
#define	UB_UNI					0x000008//极性选择位。此位置 1 时，选择单极性工作模式。此位清 0 时，选择双极性工作模式。
#define UB_BI					0x000000

#define	GAIN_1					0x000000
#define	GAIN_8					0x000003
#define	GAIN_16					0x000004
#define	GAIN_32					0x000005
#define	GAIN_64					0x000006
#define	GAIN_128				0x000007

//GPOCON Register
#define BPDSW_CLOSE			    0x40//电桥关断开关控制位
#define	BPDSW_OPEN			    0x00
#define	GP32EN					0x20//数字输出 P3 和数字输出 P2 使能
#define	GP32DIS					0x00
#define	GP10EN					0x10//数字输出 P1 和数字输出 P0 使能
#define	GP10DIS					0x00
#define	P3DAT_H					0x08//数字输出 P3
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

