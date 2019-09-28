#include "AD7192.h"
#include "stdio.h"
#include "stm32f10x.h"
#include "ad7192_driver.h"
#include "delay.h"
#include "ADG1409.h"
#include "modbus.h"

//使能奇偶校验功能时，状态寄存器的内容必须与各 24 位转换结果一同传回

/*-------------------------------------------外部引用-------------------------------------------------------------------*/
extern BoolFlagTypedef   SalveTimeOutCheckFlag;
extern unsigned char   ReceiveArrayData[256];//接收缓存区

/*-------------------------------------------全局变量声明----------------------------------------------------------------------*/
//AD7192Registers数组为什么定义成无符号32位整型呢？又为什么定义8个元素呢？
//因为数据寄存器的位数最多(数据寄存器)可以达到32位，以32位为准。除了通讯寄存器，可操作的寄存器为8个，所以定义8个元素，一个寄存器对应AD7192Registers数组的一个元素，互不干扰。
//通信寄存器 在写操作期间为通信寄存器 读操作期间为状态寄存器
unsigned long int AD7192Registers[8]={0,0,0,0,0,0,0,0};////要读或写AD7192Registers[8]
unsigned long int AD7192Data = 0;

BoolFlagTypedef     WhetherParseTheData=True;  
ADG1409ModeTypedef  ADG1409ModeChoose=ADG1409ModeOne;
/*----------------------函数声明--------------------------------------------------------------------------------------*/

unsigned char BitCount(unsigned int n);//计算数据中 1 的总数
void OuputCorrectDataFunction(unsigned long int OuputCorrectData,unsigned char WhetherParseTheData);

/*-------------------------------------------------------------------AD7192初始化---------------------------------------------------------------------------------------------------------------*/
void AD7192Initialization()
{
	AD7192SoftwareReset();//软件复位
	delay_us(500);//复位后，用户必须等待500us再访问串行接口
  AD7192InternalZeroScaleCalibration();  //内部零电压校准
	AD7192InternalFullScaleCalibration();//内部满量程校准
}
/*-------------------------------------------------------------------AD7192复位---------------------------------------------------------------------------------------------------------------*/
//执行复位需要40个连续1，这样将逻辑复位
void AD7192SoftwareReset()
{
	unsigned char WriteBuf[1];
	unsigned char ReadBuf[1];
	unsigned char i;
	
	CS_L;
	WriteBuf[0]	= 0xFF;	 
	for(i=0; i<5; i++)
	{
		SpiOperation(WriteBuf, ReadBuf,1);
	}
	CS_H;
}

/*-------------------------------------------------------------------AD7192调用基础函数----------------------------------------------------------------------------------------------------------------*/
//参数一：形参包括要写入寄存器的起始地址RegisterStartAddress(取值范围是从0x00——0x07)，
//参数二：写入寄存器的个数，
//参数三：指向要写入AD7192寄存器的数组的指针(DataBuffer才是要写出的值其他是中间变量)，
//参数四：const unsigned char OffsetInBuffer 虽然定义但没有用到，主要是与通过SPI向AD7192读操作函数对称
unsigned char WriteToAD7192ViaSPI(const unsigned char RegisterStartAddress, const unsigned char NumberOfRegistersToWrite, unsigned long int *DataBuffer, const unsigned char OffsetInBuffer)
{
	unsigned char WriteBuf[4];
	unsigned char ReadBuf[4];
	unsigned char i;
	CS_L;
	for(i=0; i<NumberOfRegistersToWrite; i++)
	{
		WriteBuf[0]	= WEN|RW_W|((RegisterStartAddress + i)<<3)|CREAD_DIS;//无一例外，首先写入通信寄存器;8位数据;下一个操作是对指定寄存器执行写操作。
		WriteBuf[1] = DataBuffer[RegisterStartAddress + i]>>16; //右移16位表示什么意思？DataBuffer是指向无符号长整型的数组指针，每个数组元素占4个字节(32位)，最高的8位无效，该语句是将16-23这8位二进制数赋给WriteBuf[1]。
		WriteBuf[2] = DataBuffer[RegisterStartAddress + i]>>8; //右移8位表示什么意思？同理该语句是将8-15这8位二进制数赋给WriteBuf[2]。
		WriteBuf[3]	= DataBuffer[RegisterStartAddress + i];//同理该语句是将0-7这8位二进制数赋给WriteBuf[3]。
		SpiOperation(WriteBuf, ReadBuf, 4);
	}
	CS_H;
	return 0;
}
//参数一：形参包括要读寄存器的起始地址RegisterStartAddress(取值范围是从0x00——0x07)，
//参数二：要读取寄存器的个数，
//参数三：指向将读取AD7192寄存器数据存入的数组的指针(DataBuffer才是要读入的值其他是中间变量)，一般指向AD7192Registers[8]，
//参数四：const unsigned char OffsetInBuffer，字面意思是缓存内偏移，是指AD7192Registers[8]数组内部偏移，注意是数组哦，之前我们说过AD7192Registers[8]之所以定义8个元素，一个寄存器对应AD7192Registers[8]数组的一个元素，互不干扰。
 unsigned char ReadFromAD7192ViaSPI(const unsigned char RegisterStartAddress, const unsigned char NumberOfRegistersToRead, unsigned long int *DataBuffer, const unsigned char OffsetInBuffer)
{
	unsigned char WriteBuf[4];
	unsigned char ReadBuf[4];
	unsigned char i;
	
	CS_L;
	for(i=0; i < NumberOfRegistersToRead; i++)
	{

		//写通信寄存器
		//禁止连续读取 写入使能
		WriteBuf[0] = WEN|RW_R|((RegisterStartAddress + i)<<3)|CREAD_DIS;	
		
		SpiOperation(WriteBuf,ReadBuf,1);//首先通过写入通信寄存器来选定下一步要读取的寄存器
                                                //然后再将WriteBuf清空
			
		WriteBuf[0] = NOP;
		WriteBuf[1]	= NOP;
		WriteBuf[2]	= NOP;
		WriteBuf[3]	= NOP;

		switch(RegisterStartAddress + i){

			case REG_ID		 :
			case REG_COM_STA : 
			case REG_GPOCON  :
				SpiOperation(WriteBuf, ReadBuf, 1); //此3种情况是读取一个字节   因为寄存器是8位
				DataBuffer[OffsetInBuffer + i ] = ReadBuf[0];
				break;
				
			case REG_MODE    : 
			case REG_CONF	 : 
			case REG_OFFSET  :
			case REG_FS		 : 
				SpiOperation(WriteBuf, ReadBuf, 3);	    //此4种情况是读取3个字节 因为寄存器是24位
				DataBuffer[OffsetInBuffer + i ] = ReadBuf[0];
				DataBuffer[OffsetInBuffer + i ] = (DataBuffer[OffsetInBuffer + i ]<<8) + ReadBuf[1];
				DataBuffer[OffsetInBuffer + i ] = (DataBuffer[OffsetInBuffer + i ]<<8) + ReadBuf[2];  
				break;
				
			case REG_DATA	 :	 //数据寄存器(0x03，24位或32位)   如果使能状态寄存器回传的话 就是32位了 后8位表示通道
				if (AD7192Registers[REG_MODE] & DAT_STA_EN)	{

					SpiOperation(WriteBuf, ReadBuf, 4);	  //多通道使能，将状态寄存器的内容附加到数据寄存器24位的数据上，所以是32位数据
					DataBuffer[OffsetInBuffer + i ] = ReadBuf[0];
					DataBuffer[OffsetInBuffer + i ] = (DataBuffer[OffsetInBuffer + i ]<<8) + ReadBuf[1];
					DataBuffer[OffsetInBuffer + i ] = (DataBuffer[OffsetInBuffer + i ]<<8) + ReadBuf[2];					
					DataBuffer[OffsetInBuffer + i ] = (DataBuffer[OffsetInBuffer + i ]<<8) + ReadBuf[3];
					break;
					}
				else {

					SpiOperation(WriteBuf, ReadBuf, 3);	   //do not transfer the status contents after read data register 
					DataBuffer[OffsetInBuffer + i ] = ReadBuf[0];
					DataBuffer[OffsetInBuffer + i ] = (DataBuffer[OffsetInBuffer + i ]<<8) + ReadBuf[1];
					DataBuffer[OffsetInBuffer + i ] = (DataBuffer[OffsetInBuffer + i ]<<8) + ReadBuf[2];
					break;
					}
	
			default			 : 
					break;
															
		}
	}
		CS_H;
	return 0;
}
/*-------------------------------------------------------------------AD7192启动连续读函数(注意是启动哦！)----------------------------------------------------------------------------------------------------------------*/
//连续读和执行一次读不一样，需要启动
void AD7192StartContinuousRead()
{
	unsigned char WriteBuf[1];
	unsigned char ReadBuf[1];
        //使用前配置
    AD7192Registers[REG_MODE] = MODE_CONT|DAT_STA_DIS|INCLK_MCLK2EN|SINC_4|ENPAR_DIS|CLK_DIV_DIS|SINGLECYCLE_DIS|REJ60_DIS|0x080;		//Output Rate =	MCLK/1024/128 without chop
    AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|AIN1_AIN2|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;	// Gain = 1
    WriteToAD7192ViaSPI(REG_MODE, 2, AD7192Registers, REG_MODE);
    
	CS_L;   
	WriteBuf[0] = WEN|RW_R|(REG_DATA<<3)|CREAD_EN;
	SpiOperation(WriteBuf, ReadBuf, 1);	

}
/*-------------------------------------------------------------------AD7192连续读函数----------------------------------------------------------------------------------------------------------------*/
unsigned long int AD7192ContinuousRead()
{
	unsigned char WriteBuf[4];
	unsigned char ReadBuf[4];
	unsigned long int DataBuffer;

	WriteBuf[0] = NOP;
	WriteBuf[1] = NOP;	
	WriteBuf[2] = NOP;
	WriteBuf[3]	= NOP;

	CS_L;
	//当DOUT/RDY变为低电平，提示转换已结束时
	//读取转换结果后，DOUT/RDY返回到高电平直到获得下一转换结果为止
	while(SDO_V  == 0){;}			
	while(SDO_V  == 1){;}			//	waiting the 1st RDY failling edge; 等待第一个数据准备完成的下降沿
		
	if ((AD7192Registers[REG_MODE] & DAT_STA_EN) == DAT_STA_EN)	
       {	//多通道使能，将状态寄存器的内容附加到数据寄存器24位的数据上，所以此情况是读4个字节。
            SpiOperation(WriteBuf, ReadBuf, 4);	  
            DataBuffer = ReadBuf[0];
            DataBuffer = (DataBuffer<<8) + ReadBuf[1];
            DataBuffer = (DataBuffer<<8) + ReadBuf[2];
            DataBuffer = (DataBuffer<<8) + ReadBuf[3];
		}
	else 
        {
            SpiOperation(WriteBuf, ReadBuf, 3);	   //do not transfer the status contents after read data register 
            DataBuffer = ReadBuf[0];
            DataBuffer = (DataBuffer<<8) + ReadBuf[1];
            DataBuffer = (DataBuffer<<8) + ReadBuf[2];
		}
	return DataBuffer;
}
/*-------------------------------------------------------------------AD7192读取转换数据----------------------------------------------------------------------------------------------------------------*/
unsigned long int AD7192ReadConvertingData()
{
	unsigned char WriteBuf[4];
	unsigned char ReadBuf[4];
	unsigned long int DataBuffer;

	CS_L;
	//读数据寄存器
	WriteBuf[0] = WEN|RW_R|((REG_DATA)<<3)|CREAD_DIS;	 
	SpiOperation(WriteBuf, ReadBuf, 1);

	WriteBuf[0] = NOP;
	WriteBuf[1] = NOP;	
	WriteBuf[2] = NOP;
	WriteBuf[3]	= NOP;
	
	//完成对数据寄存器的读操作后，DOUT/RDY 引脚 / 位置 1
	while(SDO_V == 0){;}		//从数据寄存器中读取数据字后，Dout变为高电平	
	while(SDO_V == 1){;}			//	waiting the 1st RDY failling edge;

		//如果使能了状态寄存器回传的话
		//数据的后三位就是状态寄存器的CHD[2:0]位
	if ((AD7192Registers[REG_MODE] & DAT_STA_EN) == DAT_STA_EN)	
		{
			SpiOperation(WriteBuf, ReadBuf, 4);	  
			DataBuffer = ReadBuf[0];
			DataBuffer = (DataBuffer<<8) + ReadBuf[1];
			DataBuffer = (DataBuffer<<8) + ReadBuf[2];
			DataBuffer = (DataBuffer<<8) + ReadBuf[3];
		}
	else 
		{
			SpiOperation(WriteBuf, ReadBuf, 3);	   //do not transfer the status contents after read data register 
			DataBuffer = ReadBuf[0];
			DataBuffer = (DataBuffer<<8) + ReadBuf[1];
			DataBuffer = (DataBuffer<<8) + ReadBuf[2];
		}

		return DataBuffer;
}
/*------------------------------------------------------------------AD7192退出连续读函数(和连续读启动函数、连续读操作函数配合使用)--------------------------------------------------------------------------------------------------------------*/
void AD7192ExitContinuousRead()
{

	unsigned char WriteBuf[1];
	unsigned char ReadBuf[1];

	while(SDO_V == 0){;}			
	while(SDO_V == 1){;}			//	waiting the 1st RDY failling edge;

	WriteBuf[0]	= WEN|RW_R|(REG_DATA<<3)|CREAD_DIS;

	SpiOperation(WriteBuf, ReadBuf, 1);		

	CS_H;
}
/*-------------------------------------------------------------------AD7192内部零电平校准----------------------------------------------------------------------------------------------------------------*/
void AD7192InternalZeroScaleCalibration()
{	
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;

	//斩波失能   基准电压1    失能基准电压检测  双极性
	AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|AIN1_AIN2|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;	// Gain = 1
	WriteToAD7192ViaSPI(REG_CONF, 1, AD7192Registers, REG_CONF);
	
	//内部零电平校准  返回状态寄存器数据
	AD7192Registers[REG_MODE] = MODE_INZCL|DAT_STA_EN|INCLK_MCLK2EN|SINC_4|ENPAR_EN|CLK_DIV_DIS|SINGLECYCLE_DIS|REJ60_DIS|0x080;		
	WriteToAD7192ViaSPI(REG_MODE, 1, AD7192Registers, REG_MODE);

	CS_L;
	while(SDO_V == 1){;}			//	wait until RDY = 0;  //校准完成引脚返回低电平
	CS_H;
}
/*-------------------------------------------------------------------AD7192内部满量程校准----------------------------------------------------------------------------------------------------------------*/
void AD7192InternalFullScaleCalibration()
{
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;
	AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|AIN1_AIN2|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;	// Gain = 1
	WriteToAD7192ViaSPI(REG_CONF, 1, AD7192Registers, REG_CONF);

	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_MODE] = MODE_INFCL|DAT_STA_EN|INCLK_MCLK2EN|SINC_4|ENPAR_EN|CLK_DIV_2|SINGLECYCLE_DIS|REJ60_DIS|0x080;		
	WriteToAD7192ViaSPI(REG_MODE, 1, AD7192Registers, REG_MODE);

	CS_L;
	while(SDO_V == 1){;}			//	wait until RDY = 0;
	CS_H;

}/*-------------------------------------------------------------------AD7192系统零电平校准----------------------------------------------------------------------------------------------------------------*/
void AD7192ExternalZeroScaleCalibration()
{
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;
	AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|AIN1_AIN2|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;	// Gain = 1
	WriteToAD7192ViaSPI(REG_CONF, 1, AD7192Registers, REG_CONF);

//	The user should connect the system zero-scale input to the channel input pins as selected by the CH7 to CH0 bits in the configuration register	
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_MODE] = MODE_SYSZCL|DAT_STA_EN|INCLK_MCLK2EN|SINC_4|ENPAR_EN|CLK_DIV_DIS|SINGLECYCLE_DIS|REJ60_DIS|0x080;		
	WriteToAD7192ViaSPI(REG_MODE, 1, AD7192Registers, REG_MODE);

	CS_L;
	while(SDO_V == 1){;}			//	wait until RDY = 0;
	CS_H;

}
/*-------------------------------------------------------------------AD7192系统满量程校准----------------------------------------------------------------------------------------------------------------*/
void AD7192ExternalFullScaleCalibration()
{
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;

	AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|AIN1_AIN2|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;	// Gain = 1
	WriteToAD7192ViaSPI(REG_CONF, 1, AD7192Registers, REG_CONF);
	
//	The user should connect the system full-scale input to the channel input pins as selected by the CH7 to CH0 bits in the configuration register
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_MODE] = MODE_SYSFCL|DAT_STA_EN|INCLK_MCLK2EN|SINC_4|ENPAR_EN|CLK_DIV_2|SINGLECYCLE_DIS|REJ60_DIS|0x080;		
	WriteToAD7192ViaSPI(REG_MODE, 1, AD7192Registers, REG_MODE);

	CS_L;
	while(SDO_V == 1){;}			//	wait until RDY = 0;
	CS_H;

}
/*-------------------------------------------------------------------AD7192调用基础函数----------------------------------------------------------------------------------------------------------------*/
//完成转换后，DOUT/RDY 变为低电平。从数据寄存器中读取数据字后，DOUT/RDY 变为高电平。如果 CS 为低电平，DOUT/RDY 将保持高电平，直到又启动并完成一次转换为止。
//如果需要，即使 DOUT/RDY 已变为高电平，也可以多次读取数据寄存器
void AD7192StartSingleConvertion(unsigned long int Channels)
{
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;
	AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|Channels|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;
	WriteToAD7192ViaSPI(REG_CONF, 1, AD7192Registers, REG_CONF);
	
	AD7192Registers[REG_MODE] = MODE_SING|DAT_STA_EN |INCLK_MCLK2EN|SINC_4|ENPAR_EN|CLK_DIV_DIS|SINGLECYCLE_DIS|REJ60_DIS|0x080;
	WriteToAD7192ViaSPI(REG_MODE, 1, AD7192Registers, REG_MODE);
}
/*-------------------------------------------------------------------AD7192调用基础函数----------------------------------------------------------------------------------------------------------------*/
//开始连续转换
void AD7192StartContinuousConvertion(unsigned long int Channels)
{
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;
	AD7192Registers[REG_MODE] = MODE_CONT|DAT_STA_EN|INCLK_MCLK2EN|SINC_4|ENPAR_EN|CLK_DIV_DIS|SINGLECYCLE_DIS|REJ60_DIS|0x080;		//Output Rate =	MCLK/1024/128 without chop
	AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|Channels|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;	// Gain = 1
	WriteToAD7192ViaSPI(REG_MODE, 2, AD7192Registers, REG_MODE);

}
/*-------------------------------------------------------------------AD7192读温度寄存器----------------------------------------------------------------------------------------------------------------*/
/*
AD7192 内置一个温度传感器。利用配置寄存器中的 CH2位可以选择温度传感器。如果 CH2 位设置为 1，就会使能
温度传感器。使用温度传感器并选择双极性模式时，如果温度为 0 K，器件应返回 0x800000 码。为使传感器发挥最
佳性能，需要执行单点校准。
*/
//读温度寄存器
unsigned long int AD7192ReadTemperature()
{

	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;
	//单次转换   内部时钟  Sinc 4 滤波器选择  使能奇偶校验位
	AD7192Registers[REG_MODE] = MODE_SING|DAT_STA_DIS|INCLK_MCLK2EN|SINC_4|ENPAR_EN|CLK_DIV_DIS|SINGLECYCLE_DIS|REJ60_DIS|0x080;
	WriteToAD7192ViaSPI(REG_MODE,1,AD7192Registers,REG_MODE);
	
	AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|TEMP|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;
	WriteToAD7192ViaSPI(REG_CONF, 1, AD7192Registers, REG_CONF);

	AD7192Data = AD7192ReadConvertingData();	

	return AD7192Data;

}

//实际温度获取
float RealTemperature(unsigned long int TemperatureCode)
{
	float temp = 0.0;
	temp = (TemperatureCode-0x800000)/2815.0-273;
	return temp;
}

/*-------------------------------------------------------------------AD7192开始连续读取数据----------------------------------------------------------------------------------------------------------------*/
void StartContinousRead(ADG1409ModeTypedef ADG1409ModeChoose,unsigned short NumberOfDataToContinuousRead)
{	
	unsigned short i=0;
    ADG1409ModeChooseFunction(ADG1409ModeChoose);    
	AD7192StartContinuousRead();
	
	for(i=0; i < NumberOfDataToContinuousRead; i++)
	{
        TIM2->CNT=0;
        TIM_Cmd(TIM2,ENABLE);
		AD7192Data = AD7192ContinuousRead();
        TIM_Cmd(TIM2,DISABLE);
        if(False == SalveTimeOutCheckFlag)            
        {	
            if ((AD7192Registers[REG_MODE] & DAT_STA_EN) == DAT_STA_EN)	//回传状态寄存器
            {
                ReceiveArrayData[2]=AD7192Data>>24;
                ReceiveArrayData[3]=AD7192Data>>16;
                ReceiveArrayData[4]=AD7192Data>>8;
                ReceiveArrayData[5]=AD7192Data;
                SendMultiBytesToModbus(ReceiveArrayData,6);
                continue;
            }         
            else
            {
                ReceiveArrayData[2]=AD7192Data>>16;
                ReceiveArrayData[3]=AD7192Data>>8;
                ReceiveArrayData[4]=AD7192Data;
                SendMultiBytesToModbus(ReceiveArrayData,5);
                continue;
            }
        }
        else
        {
           ReceiveArrayData[2]  =0x03; //异常码  操作超时    
        }
        
        AD7192ExitContinuousRead();
        ReceiveArrayData[1] |=0x80; //响应功能码
        SendMultiBytesToModbus(ReceiveArrayData,3);
        break;
	}
}
/*-------------------------------------------------------------------AD7192开始一次转换---------------------------------------------------------------------------------------------------------------*/
//单次转换模式下，AD7192在完成转换后处于关断模式。将模式寄存器中的MD2，MD1，和MD0分别设置为 0,0,1便可以启动单次转换，此时AD7192上电，执行单次转换然后返回关断模式。
//片内振荡器上电需要大约1ms
void StartSingleConvertion(unsigned long int Channels,ADG1409ModeTypedef ADG1409ModeChoose)
{	
    ADG1409ModeChooseFunction(ADG1409ModeChoose);    
	delay_ms(1);
    
    //等待超时检测
    TIM2->CNT=0;
    TIM_Cmd(TIM2,ENABLE);
	AD7192StartSingleConvertion(Channels);
	AD7192Data = AD7192ReadConvertingData();
    TIM_Cmd(TIM2,DISABLE);
    if(False == SalveTimeOutCheckFlag)//判断是否超时
    {
        if((AD7192Registers[REG_MODE] & ENPAR_EN) == ENPAR_EN)	//如果使能了偶校验   
        {
            if((BitCount(AD7192Data >> 4) % 2) == 0)//数据字中的1的总数为偶数。可用数据  右移是为了去掉数据的通道数据
            {
                if(True == WhetherParseTheData)//使能 发送解析后的数据
                {
                    //AdData=(AD7192Data>>8);//低八位是通道位 和必要的 1
//                    Channels=AD7192Data&0x07;//保留低三位  为通道
                    
//                    ReceiveArrayData[2]  =AD7192Data&0x07;  //通道号
                    
                    //最高八位是ADG的通道选择位
                    //其次是数据24位数据
                    ReceiveArrayData[2]  =ADG1409ModeChoose & 0x03;
                    ReceiveArrayData[3]  =AD7192Data>>24;  
                    ReceiveArrayData[4]  =AD7192Data>>16;
                    ReceiveArrayData[5]  =AD7192Data>>8; //低八位是通道位 和必要的 1  舍去低八位
                    SendMultiBytesToModbus(ReceiveArrayData,6); 
                    return;
					
                }
                else if(False == WhetherParseTheData) //发送原始数据   32位原始数据 带状态寄存器
                {
                     // printf("ReadData = 0x%08X \r\n", AD7192Data);
                    ReceiveArrayData[2]  =AD7192Data>>24;
                    ReceiveArrayData[3]  =AD7192Data>>16;
                    ReceiveArrayData[4]  =AD7192Data>>8;
                    ReceiveArrayData[5]  =AD7192Data;
                    SendMultiBytesToModbus(ReceiveArrayData,6); 
                    return;
                }
            }				
            else
            {
                 ReceiveArrayData[2]  =0x04;//异常码  数据受损
            }             
        }
        else
        {
//            printf("ReadData = 0x%08X \r\n", AD7192Data);  没有奇偶校验 24位
            ReceiveArrayData[2]  =AD7192Data>>24;
            ReceiveArrayData[3]  =AD7192Data>>16;
            ReceiveArrayData[4]  =AD7192Data>>8;
            SendMultiBytesToModbus(ReceiveArrayData,5); 
            return;
        }
    }
    else
    {
        ReceiveArrayData[2]  =0x03; //异常码  操作超时  
    }
      
     ReceiveArrayData[1] |=0x80; //响应功能码
     SendMultiBytesToModbus(ReceiveArrayData,3);
	//printf("\r\nAIN1_COM = 0x%08X \r\n", AD7192Data);
	
}

/*-------------------------------------------------------------------AD7192开始连续转换--------------------------------------------------------------------------------------------------------*/
void  StartContinuousConvertion(unsigned long int Channels,unsigned short NumberOfDataToContinuousConvertion)
{	
	unsigned short i=0;
	AD7192StartContinuousConvertion(Channels);
	for(i=0; i < NumberOfDataToContinuousConvertion; i++)
	{
		AD7192Registers[REG_DATA] = 0;	
		AD7192Data = AD7192ReadConvertingData();
		OuputCorrectDataFunction(AD7192Data,True);
	//printf("ReadData = 0x%08X \r\n", AD7192Data);
	}
}

/*-------------------------------------------------------------------AD7192温度的读取--------------------------------------------------------------------------------------------------------*/
void ReadTemperature(void)
{
	float AD7192Temperature = 0.0;
	AD7192Data = AD7192ReadTemperature();
	AD7192Temperature = RealTemperature(AD7192Data);
	printf("\r\nTemperature = %2.1f C\r\n", AD7192Temperature);
	
}

/*-------------------------------------------------------------------AD7192数据正误校验--------------------------------------------------------------------------------------------------------*/
/*
使能奇偶校验
使能奇偶校验功能时，状态寄存器的内容必须与各24位转换结果一同传输。对于各转换结果读取数据，状态寄存器中的奇偶校验位经过编程，
使得24位数据字中的 1 的总数为偶数，如果不是偶数，则说明所接收的数据已受损。
*/
//去除状态寄存器的值 和 为了满足奇偶校验的值
//也就是去除最后8位（二进制）
//也可以在unsigned long int AD7192ReadConvertingData()  直接截取

//计算数据中 1 的总数
unsigned char BitCount(unsigned int n)
{
    unsigned char c =0 ; // 计数器
    for (c =0; n; n >>=1) // 循环移位
		c += n &1 ; // 如果当前位是1，则计数器加1
    return c ;
}
//如果寄存器的数据 1 为 奇数 则奇偶检验使能位为 1
//如果寄存器的数据 1 为 偶数 则奇偶检验使能位为 0
//判断接收的数据是否受损
void OuputCorrectDataFunction(unsigned long int OuputCorrectData,unsigned char WhetherParseTheData)
{
	unsigned long int  AdData=0;
	unsigned char Channels=0;
	float temp = 0.0;
	if((AD7192Registers[REG_MODE] & ENPAR_EN) == ENPAR_EN)	//如果使能了奇偶校验
	{
		if((BitCount(OuputCorrectData >> 4) % 2) == 0)//数据字中的1的总数为偶数。可用数据  右移是为了去掉数据的通道数据
		{
				if(True == WhetherParseTheData)//使能 发送解析后的数据
				{
					AdData=(OuputCorrectData>>8);//低八位是通道位 和必要的 1
					Channels=OuputCorrectData&0x07;//保留低三位  为通道
					printf("Channels = 0x%02X",Channels);
					printf("----------------");
					printf("AdData = 0x%06X ",AdData);//24位数据位			
					printf("----------------");
					temp=AdData;
					temp=(10.25*(temp/0x800000 -1));
					printf("AdVoltage = %2.2f \r\n",temp);//差分AD电压
								
				}
				else if(False == WhetherParseTheData)
				printf("ReadData = 0x%08X \r\n", OuputCorrectData);
		}				
		else
				printf("数据受损\r\n");	
	}
	else
	{
		printf("ReadData = 0x%08X \r\n", OuputCorrectData);
	}
}
/*-------------------------------------------------------------------ModbusResponseReadChannels--------------------------------------------------------------------------------------------------------*/
/*********************************************************************************/ 
/*函数名称:  ModbusResponseReadChannelOnce()                             
 *输入参数： 无 
 *返回值：   无      
 *功能介绍：   
        (1)；读一次AD转换结果 
*/
/*********************************************************************************/ 
void ModbusResponseReadChannelOnce(void)
{   
    ADG1409ModeChoose = (ADG1409ModeTypedef)ReceiveArrayData[2];
    if(ADG1409ModeChoose <= 3)
    {    
        switch(ADG1409ModeChoose)
        {
            case ADG1409ModeOne:
                StartSingleConvertion(AIN1_AIN2,ADG1409ModeOne);//回传状态寄存器数据;
            break;
            case ADG1409ModeTwo:
                StartSingleConvertion(AIN1_AIN2,ADG1409ModeTwo);//回传状态寄存器数据;
            break;
            case ADG1409ModeThree:
                StartSingleConvertion(AIN1_AIN2,ADG1409ModeThree);//回传状态寄存器数据;
            break;
            case ADG1409ModeFour:
                StartSingleConvertion(AIN1_AIN2,ADG1409ModeFour);//回传状态寄存器数据;
            break;
            case ADG1409ModeOff:
                 ADG1409ModeChooseFunction(ADG1409ModeOff);//回传状态寄存器数据;
                 ReceiveArrayData[2]=0x00;
                 SendMultiBytesToModbus(ReceiveArrayData,3);
            break;
        }
        return;
        
    }
    else
    {
         ReceiveArrayData[2] =0x02;//设置模式 异常码
    }
     ReceiveArrayData[1] |=0x80; //响应功能码
     SendMultiBytesToModbus(ReceiveArrayData,3);	
}  
/*********************************************************************************/ 
/*函数名称:  ModbusResponseReadChannelContinous()                             
 *输入参数： 无 
 *返回值：   无      
 *功能介绍：   
        (1)；读多次AD转换结果 
*/
/*********************************************************************************/ 
void ModbusResponseReadChannelContinous(void)
{
    unsigned char ReadChannelContinousNumbers=0;
    ADG1409ModeChoose = (ADG1409ModeTypedef)ReceiveArrayData[2]; 
    ReadChannelContinousNumbers =  ReceiveArrayData[3]; 
    if(ADG1409ModeChoose <= 4)
    {   
        if(0 < ReadChannelContinousNumbers)
        {
            switch(ADG1409ModeChoose)
            {
                case ADG1409ModeOne:
                    StartContinousRead(ADG1409ModeChoose,ReadChannelContinousNumbers);//回传状态寄存器数据;
                break;
                case ADG1409ModeTwo:
                    StartContinousRead(ADG1409ModeChoose,ReadChannelContinousNumbers);//回传状态寄存器数据;
                break;
                case ADG1409ModeThree:
                    StartContinousRead(ADG1409ModeChoose,ReadChannelContinousNumbers);//回传状态寄存器数据;
                break;
                case ADG1409ModeFour:
                    StartContinousRead(ADG1409ModeChoose,ReadChannelContinousNumbers);//回传状态寄存器数据;
                break;
                case ADG1409ModeOff:
                     ADG1409ModeChooseFunction(ADG1409ModeOff);//回传状态寄存器数据;
                     ReceiveArrayData[2]=0x00;           //关闭通道
                     SendMultiBytesToModbus(ReceiveArrayData,3);
                break;
            }
        return;           
        }
        else
        {
           ReceiveArrayData[2] =0x04;//读取次数 异常码 
        }              
    }
    else
    {
         ReceiveArrayData[2] =0x02;//通道选择 异常码
    }
     ReceiveArrayData[1] |=0x80; //响应功能码
     SendMultiBytesToModbus(ReceiveArrayData,3);
}
