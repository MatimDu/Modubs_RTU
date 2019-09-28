#include "AD7192.h"
#include "stdio.h"
#include "stm32f10x.h"
#include "ad7192_driver.h"
#include "delay.h"
#include "ADG1409.h"
#include "modbus.h"

//ʹ����żУ�鹦��ʱ��״̬�Ĵ��������ݱ������ 24 λת�����һͬ����

/*-------------------------------------------�ⲿ����-------------------------------------------------------------------*/
extern BoolFlagTypedef   SalveTimeOutCheckFlag;
extern unsigned char   ReceiveArrayData[256];//���ջ�����

/*-------------------------------------------ȫ�ֱ�������----------------------------------------------------------------------*/
//AD7192Registers����Ϊʲô������޷���32λ�����أ���Ϊʲô����8��Ԫ���أ�
//��Ϊ���ݼĴ�����λ�����(���ݼĴ���)���Դﵽ32λ����32λΪ׼������ͨѶ�Ĵ������ɲ����ļĴ���Ϊ8�������Զ���8��Ԫ�أ�һ���Ĵ�����ӦAD7192Registers�����һ��Ԫ�أ��������š�
//ͨ�żĴ��� ��д�����ڼ�Ϊͨ�żĴ��� �������ڼ�Ϊ״̬�Ĵ���
unsigned long int AD7192Registers[8]={0,0,0,0,0,0,0,0};////Ҫ����дAD7192Registers[8]
unsigned long int AD7192Data = 0;

BoolFlagTypedef     WhetherParseTheData=True;  
ADG1409ModeTypedef  ADG1409ModeChoose=ADG1409ModeOne;
/*----------------------��������--------------------------------------------------------------------------------------*/

unsigned char BitCount(unsigned int n);//���������� 1 ������
void OuputCorrectDataFunction(unsigned long int OuputCorrectData,unsigned char WhetherParseTheData);

/*-------------------------------------------------------------------AD7192��ʼ��---------------------------------------------------------------------------------------------------------------*/
void AD7192Initialization()
{
	AD7192SoftwareReset();//�����λ
	delay_us(500);//��λ���û�����ȴ�500us�ٷ��ʴ��нӿ�
  AD7192InternalZeroScaleCalibration();  //�ڲ����ѹУ׼
	AD7192InternalFullScaleCalibration();//�ڲ�������У׼
}
/*-------------------------------------------------------------------AD7192��λ---------------------------------------------------------------------------------------------------------------*/
//ִ�и�λ��Ҫ40������1���������߼���λ
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

/*-------------------------------------------------------------------AD7192���û�������----------------------------------------------------------------------------------------------------------------*/
//����һ���βΰ���Ҫд��Ĵ�������ʼ��ַRegisterStartAddress(ȡֵ��Χ�Ǵ�0x00����0x07)��
//��������д��Ĵ����ĸ�����
//��������ָ��Ҫд��AD7192�Ĵ����������ָ��(DataBuffer����Ҫд����ֵ�������м����)��
//�����ģ�const unsigned char OffsetInBuffer ��Ȼ���嵫û���õ�����Ҫ����ͨ��SPI��AD7192�����������Գ�
unsigned char WriteToAD7192ViaSPI(const unsigned char RegisterStartAddress, const unsigned char NumberOfRegistersToWrite, unsigned long int *DataBuffer, const unsigned char OffsetInBuffer)
{
	unsigned char WriteBuf[4];
	unsigned char ReadBuf[4];
	unsigned char i;
	CS_L;
	for(i=0; i<NumberOfRegistersToWrite; i++)
	{
		WriteBuf[0]	= WEN|RW_W|((RegisterStartAddress + i)<<3)|CREAD_DIS;//��һ���⣬����д��ͨ�żĴ���;8λ����;��һ�������Ƕ�ָ���Ĵ���ִ��д������
		WriteBuf[1] = DataBuffer[RegisterStartAddress + i]>>16; //����16λ��ʾʲô��˼��DataBuffer��ָ���޷��ų����͵�����ָ�룬ÿ������Ԫ��ռ4���ֽ�(32λ)����ߵ�8λ��Ч��������ǽ�16-23��8λ������������WriteBuf[1]��
		WriteBuf[2] = DataBuffer[RegisterStartAddress + i]>>8; //����8λ��ʾʲô��˼��ͬ�������ǽ�8-15��8λ������������WriteBuf[2]��
		WriteBuf[3]	= DataBuffer[RegisterStartAddress + i];//ͬ�������ǽ�0-7��8λ������������WriteBuf[3]��
		SpiOperation(WriteBuf, ReadBuf, 4);
	}
	CS_H;
	return 0;
}
//����һ���βΰ���Ҫ���Ĵ�������ʼ��ַRegisterStartAddress(ȡֵ��Χ�Ǵ�0x00����0x07)��
//��������Ҫ��ȡ�Ĵ����ĸ�����
//��������ָ�򽫶�ȡAD7192�Ĵ������ݴ���������ָ��(DataBuffer����Ҫ�����ֵ�������м����)��һ��ָ��AD7192Registers[8]��
//�����ģ�const unsigned char OffsetInBuffer��������˼�ǻ�����ƫ�ƣ���ָAD7192Registers[8]�����ڲ�ƫ�ƣ�ע��������Ŷ��֮ǰ����˵��AD7192Registers[8]֮���Զ���8��Ԫ�أ�һ���Ĵ�����ӦAD7192Registers[8]�����һ��Ԫ�أ��������š�
 unsigned char ReadFromAD7192ViaSPI(const unsigned char RegisterStartAddress, const unsigned char NumberOfRegistersToRead, unsigned long int *DataBuffer, const unsigned char OffsetInBuffer)
{
	unsigned char WriteBuf[4];
	unsigned char ReadBuf[4];
	unsigned char i;
	
	CS_L;
	for(i=0; i < NumberOfRegistersToRead; i++)
	{

		//дͨ�żĴ���
		//��ֹ������ȡ д��ʹ��
		WriteBuf[0] = WEN|RW_R|((RegisterStartAddress + i)<<3)|CREAD_DIS;	
		
		SpiOperation(WriteBuf,ReadBuf,1);//����ͨ��д��ͨ�żĴ�����ѡ����һ��Ҫ��ȡ�ļĴ���
                                                //Ȼ���ٽ�WriteBuf���
			
		WriteBuf[0] = NOP;
		WriteBuf[1]	= NOP;
		WriteBuf[2]	= NOP;
		WriteBuf[3]	= NOP;

		switch(RegisterStartAddress + i){

			case REG_ID		 :
			case REG_COM_STA : 
			case REG_GPOCON  :
				SpiOperation(WriteBuf, ReadBuf, 1); //��3������Ƕ�ȡһ���ֽ�   ��Ϊ�Ĵ�����8λ
				DataBuffer[OffsetInBuffer + i ] = ReadBuf[0];
				break;
				
			case REG_MODE    : 
			case REG_CONF	 : 
			case REG_OFFSET  :
			case REG_FS		 : 
				SpiOperation(WriteBuf, ReadBuf, 3);	    //��4������Ƕ�ȡ3���ֽ� ��Ϊ�Ĵ�����24λ
				DataBuffer[OffsetInBuffer + i ] = ReadBuf[0];
				DataBuffer[OffsetInBuffer + i ] = (DataBuffer[OffsetInBuffer + i ]<<8) + ReadBuf[1];
				DataBuffer[OffsetInBuffer + i ] = (DataBuffer[OffsetInBuffer + i ]<<8) + ReadBuf[2];  
				break;
				
			case REG_DATA	 :	 //���ݼĴ���(0x03��24λ��32λ)   ���ʹ��״̬�Ĵ����ش��Ļ� ����32λ�� ��8λ��ʾͨ��
				if (AD7192Registers[REG_MODE] & DAT_STA_EN)	{

					SpiOperation(WriteBuf, ReadBuf, 4);	  //��ͨ��ʹ�ܣ���״̬�Ĵ��������ݸ��ӵ����ݼĴ���24λ�������ϣ�������32λ����
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
/*-------------------------------------------------------------------AD7192��������������(ע��������Ŷ��)----------------------------------------------------------------------------------------------------------------*/
//��������ִ��һ�ζ���һ������Ҫ����
void AD7192StartContinuousRead()
{
	unsigned char WriteBuf[1];
	unsigned char ReadBuf[1];
        //ʹ��ǰ����
    AD7192Registers[REG_MODE] = MODE_CONT|DAT_STA_DIS|INCLK_MCLK2EN|SINC_4|ENPAR_DIS|CLK_DIV_DIS|SINGLECYCLE_DIS|REJ60_DIS|0x080;		//Output Rate =	MCLK/1024/128 without chop
    AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|AIN1_AIN2|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;	// Gain = 1
    WriteToAD7192ViaSPI(REG_MODE, 2, AD7192Registers, REG_MODE);
    
	CS_L;   
	WriteBuf[0] = WEN|RW_R|(REG_DATA<<3)|CREAD_EN;
	SpiOperation(WriteBuf, ReadBuf, 1);	

}
/*-------------------------------------------------------------------AD7192����������----------------------------------------------------------------------------------------------------------------*/
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
	//��DOUT/RDY��Ϊ�͵�ƽ����ʾת���ѽ���ʱ
	//��ȡת�������DOUT/RDY���ص��ߵ�ƽֱ�������һת�����Ϊֹ
	while(SDO_V  == 0){;}			
	while(SDO_V  == 1){;}			//	waiting the 1st RDY failling edge; �ȴ���һ������׼����ɵ��½���
		
	if ((AD7192Registers[REG_MODE] & DAT_STA_EN) == DAT_STA_EN)	
       {	//��ͨ��ʹ�ܣ���״̬�Ĵ��������ݸ��ӵ����ݼĴ���24λ�������ϣ����Դ�����Ƕ�4���ֽڡ�
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
/*-------------------------------------------------------------------AD7192��ȡת������----------------------------------------------------------------------------------------------------------------*/
unsigned long int AD7192ReadConvertingData()
{
	unsigned char WriteBuf[4];
	unsigned char ReadBuf[4];
	unsigned long int DataBuffer;

	CS_L;
	//�����ݼĴ���
	WriteBuf[0] = WEN|RW_R|((REG_DATA)<<3)|CREAD_DIS;	 
	SpiOperation(WriteBuf, ReadBuf, 1);

	WriteBuf[0] = NOP;
	WriteBuf[1] = NOP;	
	WriteBuf[2] = NOP;
	WriteBuf[3]	= NOP;
	
	//��ɶ����ݼĴ����Ķ�������DOUT/RDY ���� / λ�� 1
	while(SDO_V == 0){;}		//�����ݼĴ����ж�ȡ�����ֺ�Dout��Ϊ�ߵ�ƽ	
	while(SDO_V == 1){;}			//	waiting the 1st RDY failling edge;

		//���ʹ����״̬�Ĵ����ش��Ļ�
		//���ݵĺ���λ����״̬�Ĵ�����CHD[2:0]λ
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
/*------------------------------------------------------------------AD7192�˳�����������(�����������������������������������ʹ��)--------------------------------------------------------------------------------------------------------------*/
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
/*-------------------------------------------------------------------AD7192�ڲ����ƽУ׼----------------------------------------------------------------------------------------------------------------*/
void AD7192InternalZeroScaleCalibration()
{	
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;

	//ն��ʧ��   ��׼��ѹ1    ʧ�ܻ�׼��ѹ���  ˫����
	AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|AIN1_AIN2|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;	// Gain = 1
	WriteToAD7192ViaSPI(REG_CONF, 1, AD7192Registers, REG_CONF);
	
	//�ڲ����ƽУ׼  ����״̬�Ĵ�������
	AD7192Registers[REG_MODE] = MODE_INZCL|DAT_STA_EN|INCLK_MCLK2EN|SINC_4|ENPAR_EN|CLK_DIV_DIS|SINGLECYCLE_DIS|REJ60_DIS|0x080;		
	WriteToAD7192ViaSPI(REG_MODE, 1, AD7192Registers, REG_MODE);

	CS_L;
	while(SDO_V == 1){;}			//	wait until RDY = 0;  //У׼������ŷ��ص͵�ƽ
	CS_H;
}
/*-------------------------------------------------------------------AD7192�ڲ�������У׼----------------------------------------------------------------------------------------------------------------*/
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

}/*-------------------------------------------------------------------AD7192ϵͳ���ƽУ׼----------------------------------------------------------------------------------------------------------------*/
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
/*-------------------------------------------------------------------AD7192ϵͳ������У׼----------------------------------------------------------------------------------------------------------------*/
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
/*-------------------------------------------------------------------AD7192���û�������----------------------------------------------------------------------------------------------------------------*/
//���ת����DOUT/RDY ��Ϊ�͵�ƽ�������ݼĴ����ж�ȡ�����ֺ�DOUT/RDY ��Ϊ�ߵ�ƽ����� CS Ϊ�͵�ƽ��DOUT/RDY �����ָߵ�ƽ��ֱ�������������һ��ת��Ϊֹ��
//�����Ҫ����ʹ DOUT/RDY �ѱ�Ϊ�ߵ�ƽ��Ҳ���Զ�ζ�ȡ���ݼĴ���
void AD7192StartSingleConvertion(unsigned long int Channels)
{
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;
	AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|Channels|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;
	WriteToAD7192ViaSPI(REG_CONF, 1, AD7192Registers, REG_CONF);
	
	AD7192Registers[REG_MODE] = MODE_SING|DAT_STA_EN |INCLK_MCLK2EN|SINC_4|ENPAR_EN|CLK_DIV_DIS|SINGLECYCLE_DIS|REJ60_DIS|0x080;
	WriteToAD7192ViaSPI(REG_MODE, 1, AD7192Registers, REG_MODE);
}
/*-------------------------------------------------------------------AD7192���û�������----------------------------------------------------------------------------------------------------------------*/
//��ʼ����ת��
void AD7192StartContinuousConvertion(unsigned long int Channels)
{
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;
	AD7192Registers[REG_MODE] = MODE_CONT|DAT_STA_EN|INCLK_MCLK2EN|SINC_4|ENPAR_EN|CLK_DIV_DIS|SINGLECYCLE_DIS|REJ60_DIS|0x080;		//Output Rate =	MCLK/1024/128 without chop
	AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|Channels|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;	// Gain = 1
	WriteToAD7192ViaSPI(REG_MODE, 2, AD7192Registers, REG_MODE);

}
/*-------------------------------------------------------------------AD7192���¶ȼĴ���----------------------------------------------------------------------------------------------------------------*/
/*
AD7192 ����һ���¶ȴ��������������üĴ����е� CH2λ����ѡ���¶ȴ���������� CH2 λ����Ϊ 1���ͻ�ʹ��
�¶ȴ�������ʹ���¶ȴ�������ѡ��˫����ģʽʱ������¶�Ϊ 0 K������Ӧ���� 0x800000 �롣Ϊʹ������������
�����ܣ���Ҫִ�е���У׼��
*/
//���¶ȼĴ���
unsigned long int AD7192ReadTemperature()
{

	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;
	//����ת��   �ڲ�ʱ��  Sinc 4 �˲���ѡ��  ʹ����żУ��λ
	AD7192Registers[REG_MODE] = MODE_SING|DAT_STA_DIS|INCLK_MCLK2EN|SINC_4|ENPAR_EN|CLK_DIV_DIS|SINGLECYCLE_DIS|REJ60_DIS|0x080;
	WriteToAD7192ViaSPI(REG_MODE,1,AD7192Registers,REG_MODE);
	
	AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN1|TEMP|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;
	WriteToAD7192ViaSPI(REG_CONF, 1, AD7192Registers, REG_CONF);

	AD7192Data = AD7192ReadConvertingData();	

	return AD7192Data;

}

//ʵ���¶Ȼ�ȡ
float RealTemperature(unsigned long int TemperatureCode)
{
	float temp = 0.0;
	temp = (TemperatureCode-0x800000)/2815.0-273;
	return temp;
}

/*-------------------------------------------------------------------AD7192��ʼ������ȡ����----------------------------------------------------------------------------------------------------------------*/
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
            if ((AD7192Registers[REG_MODE] & DAT_STA_EN) == DAT_STA_EN)	//�ش�״̬�Ĵ���
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
           ReceiveArrayData[2]  =0x03; //�쳣��  ������ʱ    
        }
        
        AD7192ExitContinuousRead();
        ReceiveArrayData[1] |=0x80; //��Ӧ������
        SendMultiBytesToModbus(ReceiveArrayData,3);
        break;
	}
}
/*-------------------------------------------------------------------AD7192��ʼһ��ת��---------------------------------------------------------------------------------------------------------------*/
//����ת��ģʽ�£�AD7192�����ת�����ڹض�ģʽ����ģʽ�Ĵ����е�MD2��MD1����MD0�ֱ�����Ϊ 0,0,1�������������ת������ʱAD7192�ϵ磬ִ�е���ת��Ȼ�󷵻عض�ģʽ��
//Ƭ�������ϵ���Ҫ��Լ1ms
void StartSingleConvertion(unsigned long int Channels,ADG1409ModeTypedef ADG1409ModeChoose)
{	
    ADG1409ModeChooseFunction(ADG1409ModeChoose);    
	delay_ms(1);
    
    //�ȴ���ʱ���
    TIM2->CNT=0;
    TIM_Cmd(TIM2,ENABLE);
	AD7192StartSingleConvertion(Channels);
	AD7192Data = AD7192ReadConvertingData();
    TIM_Cmd(TIM2,DISABLE);
    if(False == SalveTimeOutCheckFlag)//�ж��Ƿ�ʱ
    {
        if((AD7192Registers[REG_MODE] & ENPAR_EN) == ENPAR_EN)	//���ʹ����żУ��   
        {
            if((BitCount(AD7192Data >> 4) % 2) == 0)//�������е�1������Ϊż������������  ������Ϊ��ȥ�����ݵ�ͨ������
            {
                if(True == WhetherParseTheData)//ʹ�� ���ͽ����������
                {
                    //AdData=(AD7192Data>>8);//�Ͱ�λ��ͨ��λ �ͱ�Ҫ�� 1
//                    Channels=AD7192Data&0x07;//��������λ  Ϊͨ��
                    
//                    ReceiveArrayData[2]  =AD7192Data&0x07;  //ͨ����
                    
                    //��߰�λ��ADG��ͨ��ѡ��λ
                    //���������24λ����
                    ReceiveArrayData[2]  =ADG1409ModeChoose & 0x03;
                    ReceiveArrayData[3]  =AD7192Data>>24;  
                    ReceiveArrayData[4]  =AD7192Data>>16;
                    ReceiveArrayData[5]  =AD7192Data>>8; //�Ͱ�λ��ͨ��λ �ͱ�Ҫ�� 1  ��ȥ�Ͱ�λ
                    SendMultiBytesToModbus(ReceiveArrayData,6); 
                    return;
					
                }
                else if(False == WhetherParseTheData) //����ԭʼ����   32λԭʼ���� ��״̬�Ĵ���
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
                 ReceiveArrayData[2]  =0x04;//�쳣��  ��������
            }             
        }
        else
        {
//            printf("ReadData = 0x%08X \r\n", AD7192Data);  û����żУ�� 24λ
            ReceiveArrayData[2]  =AD7192Data>>24;
            ReceiveArrayData[3]  =AD7192Data>>16;
            ReceiveArrayData[4]  =AD7192Data>>8;
            SendMultiBytesToModbus(ReceiveArrayData,5); 
            return;
        }
    }
    else
    {
        ReceiveArrayData[2]  =0x03; //�쳣��  ������ʱ  
    }
      
     ReceiveArrayData[1] |=0x80; //��Ӧ������
     SendMultiBytesToModbus(ReceiveArrayData,3);
	//printf("\r\nAIN1_COM = 0x%08X \r\n", AD7192Data);
	
}

/*-------------------------------------------------------------------AD7192��ʼ����ת��--------------------------------------------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------AD7192�¶ȵĶ�ȡ--------------------------------------------------------------------------------------------------------*/
void ReadTemperature(void)
{
	float AD7192Temperature = 0.0;
	AD7192Data = AD7192ReadTemperature();
	AD7192Temperature = RealTemperature(AD7192Data);
	printf("\r\nTemperature = %2.1f C\r\n", AD7192Temperature);
	
}

/*-------------------------------------------------------------------AD7192��������У��--------------------------------------------------------------------------------------------------------*/
/*
ʹ����żУ��
ʹ����żУ�鹦��ʱ��״̬�Ĵ��������ݱ������24λת�����һͬ���䡣���ڸ�ת�������ȡ���ݣ�״̬�Ĵ����е���żУ��λ������̣�
ʹ��24λ�������е� 1 ������Ϊż�����������ż������˵�������յ�����������
*/
//ȥ��״̬�Ĵ�����ֵ �� Ϊ��������żУ���ֵ
//Ҳ����ȥ�����8λ�������ƣ�
//Ҳ������unsigned long int AD7192ReadConvertingData()  ֱ�ӽ�ȡ

//���������� 1 ������
unsigned char BitCount(unsigned int n)
{
    unsigned char c =0 ; // ������
    for (c =0; n; n >>=1) // ѭ����λ
		c += n &1 ; // �����ǰλ��1�����������1
    return c ;
}
//����Ĵ��������� 1 Ϊ ���� ����ż����ʹ��λΪ 1
//����Ĵ��������� 1 Ϊ ż�� ����ż����ʹ��λΪ 0
//�жϽ��յ������Ƿ�����
void OuputCorrectDataFunction(unsigned long int OuputCorrectData,unsigned char WhetherParseTheData)
{
	unsigned long int  AdData=0;
	unsigned char Channels=0;
	float temp = 0.0;
	if((AD7192Registers[REG_MODE] & ENPAR_EN) == ENPAR_EN)	//���ʹ������żУ��
	{
		if((BitCount(OuputCorrectData >> 4) % 2) == 0)//�������е�1������Ϊż������������  ������Ϊ��ȥ�����ݵ�ͨ������
		{
				if(True == WhetherParseTheData)//ʹ�� ���ͽ����������
				{
					AdData=(OuputCorrectData>>8);//�Ͱ�λ��ͨ��λ �ͱ�Ҫ�� 1
					Channels=OuputCorrectData&0x07;//��������λ  Ϊͨ��
					printf("Channels = 0x%02X",Channels);
					printf("----------------");
					printf("AdData = 0x%06X ",AdData);//24λ����λ			
					printf("----------------");
					temp=AdData;
					temp=(10.25*(temp/0x800000 -1));
					printf("AdVoltage = %2.2f \r\n",temp);//���AD��ѹ
								
				}
				else if(False == WhetherParseTheData)
				printf("ReadData = 0x%08X \r\n", OuputCorrectData);
		}				
		else
				printf("��������\r\n");	
	}
	else
	{
		printf("ReadData = 0x%08X \r\n", OuputCorrectData);
	}
}
/*-------------------------------------------------------------------ModbusResponseReadChannels--------------------------------------------------------------------------------------------------------*/
/*********************************************************************************/ 
/*��������:  ModbusResponseReadChannelOnce()                             
 *��������� �� 
 *����ֵ��   ��      
 *���ܽ��ܣ�   
        (1)����һ��ADת����� 
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
                StartSingleConvertion(AIN1_AIN2,ADG1409ModeOne);//�ش�״̬�Ĵ�������;
            break;
            case ADG1409ModeTwo:
                StartSingleConvertion(AIN1_AIN2,ADG1409ModeTwo);//�ش�״̬�Ĵ�������;
            break;
            case ADG1409ModeThree:
                StartSingleConvertion(AIN1_AIN2,ADG1409ModeThree);//�ش�״̬�Ĵ�������;
            break;
            case ADG1409ModeFour:
                StartSingleConvertion(AIN1_AIN2,ADG1409ModeFour);//�ش�״̬�Ĵ�������;
            break;
            case ADG1409ModeOff:
                 ADG1409ModeChooseFunction(ADG1409ModeOff);//�ش�״̬�Ĵ�������;
                 ReceiveArrayData[2]=0x00;
                 SendMultiBytesToModbus(ReceiveArrayData,3);
            break;
        }
        return;
        
    }
    else
    {
         ReceiveArrayData[2] =0x02;//����ģʽ �쳣��
    }
     ReceiveArrayData[1] |=0x80; //��Ӧ������
     SendMultiBytesToModbus(ReceiveArrayData,3);	
}  
/*********************************************************************************/ 
/*��������:  ModbusResponseReadChannelContinous()                             
 *��������� �� 
 *����ֵ��   ��      
 *���ܽ��ܣ�   
        (1)�������ADת����� 
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
                    StartContinousRead(ADG1409ModeChoose,ReadChannelContinousNumbers);//�ش�״̬�Ĵ�������;
                break;
                case ADG1409ModeTwo:
                    StartContinousRead(ADG1409ModeChoose,ReadChannelContinousNumbers);//�ش�״̬�Ĵ�������;
                break;
                case ADG1409ModeThree:
                    StartContinousRead(ADG1409ModeChoose,ReadChannelContinousNumbers);//�ش�״̬�Ĵ�������;
                break;
                case ADG1409ModeFour:
                    StartContinousRead(ADG1409ModeChoose,ReadChannelContinousNumbers);//�ش�״̬�Ĵ�������;
                break;
                case ADG1409ModeOff:
                     ADG1409ModeChooseFunction(ADG1409ModeOff);//�ش�״̬�Ĵ�������;
                     ReceiveArrayData[2]=0x00;           //�ر�ͨ��
                     SendMultiBytesToModbus(ReceiveArrayData,3);
                break;
            }
        return;           
        }
        else
        {
           ReceiveArrayData[2] =0x04;//��ȡ���� �쳣�� 
        }              
    }
    else
    {
         ReceiveArrayData[2] =0x02;//ͨ��ѡ�� �쳣��
    }
     ReceiveArrayData[1] |=0x80; //��Ӧ������
     SendMultiBytesToModbus(ReceiveArrayData,3);
}
