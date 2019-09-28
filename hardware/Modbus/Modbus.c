#include "Modbus.h"
#include "delay.h"
#include "stm32f10x.h"
#include "usart.h"
#include "AD7192.h"
#include <string.h>
#include "ADG1409.h"
//ģʽ����
//Ĭ��У��λ��        żУ��λ
//Ĭ�ϴ���ģʽ��      RTU
//TransferModeTypedef TransferMode = RTU_Mode;
//OddEvenCheckTypedef OddEvenCheckMode=OddCheck; 
BoolFlagTypedef     SalveTimeOutCheckFlag=False;
BoolFlagTypedef     ReceiveDataCompleteFlag =False;
BoolFlagTypedef     ReceiveDataCrcRightFlag =False;
unsigned char   ReceiveArrayData[256]={0};//���ջ�����
unsigned char * ReceiveArrayDataPoniter=ReceiveArrayData;//���ջ�����

#define ModbusSendOneByte                        UART4_Send_Byte
#define ReceiveArrayDataClean()                 {memset(ReceiveArrayData,0,256);\
                                                 memset(AD7192Registers,0,8*4);\
                                                 ReceiveDataCompleteFlag =False;\
                                                 SalveTimeOutCheckFlag =False;\
                                                 ReceiveArrayDataPoniter=ReceiveArrayData;} // 0 �������  ��ʼ��
#define ModbusInterframeSpaceDelay()             delay_ms(2); //����֡��ʱ������Ϊ 3.5���ַ�ʱ��Ŀ��м������
#define ModbusSlaveAdress                        0x01       //  0���㲥��ַ  1~47���ֽڵ㵥����ַ 248~55������
#define SalveTotalRegisterNumber                 8  //�ɶ�д8���Ĵ���   �˸��Ĵ������ 8/24/32 
                                                 
                                                 
void ModbusResponseToHoldRegister(void);
void ModbusResponseToInputRegister(void);
void ModbusResponseWriteSingleRegister(void);
void ModbusResponseWriteMultiRegister(void);
                                                 
   

#if 0 
                                                 
#else 
/*********************************************************************************/ 
/*��������:  GetCRC16()                           
*���������  puchMsg�� ���ڼ���CRC�ı���
             usDataLen�������е��ֽ���
*����ֵ��    CRCֵ    
*���ܽ��ܣ�   
        (1)CRC16У�飻 ����У���룻               
*/                                    
/*********************************************************************************/ 
unsigned short GetCRC16(unsigned char *puchMsg, unsigned short usDataLen)   
{   
    /* CRC ��λ�ֽ�ֵ�� */   
    unsigned char auchCRCHi[256] = { 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,   
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,   
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,   
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,   
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,   
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,   
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,   
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,   
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,   
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,   
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,   
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,   
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,   
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,   
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,   
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,   
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,   
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,   
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,   
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,   
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,   
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,   
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,   
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,   
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,   
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40 
    };   
     
    unsigned char auchCRCLo[256] = { 
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,   
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,   
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,   
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,   
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,   
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,   
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,   
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,   
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,   
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,   
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,   
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,   
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,   
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,   
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,   
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,   
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,   
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,   
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,   
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,   
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,   
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,   
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,   
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,   
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,   
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40     
    }; 
    //CRC�Ĵ�����ʼ�� 
    unsigned char uchCRCHi = 0xFF ; /* ��CRC�ֽڳ�ʼ�� */   
    unsigned char uchCRCLo = 0xFF ; /* ��CRC �ֽڳ�ʼ�� */   
    unsigned uIndex = 0; /* CRCѭ���е����� */   
     
    while (usDataLen--) /* ������Ϣ������ */   
    {   
        uIndex = uchCRCHi ^ *puchMsg++ ; /* ����CRC */   
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;   
        uchCRCLo = auchCRCLo[uIndex] ;   
    }   
    return (unsigned short)((unsigned short)uchCRCHi << 8 | uchCRCLo) ;   
}
/*********************************************************************************/ 
/*��������:  SendOneByteToModbus()                             
 *��������� SendByteData�����͵�����           
 *����ֵ��    ��      
 *���ܽ��ܣ�   
        (1)����һ���ֽڵ� ModbusSlave�� 
*/
/*********************************************************************************/
void SendOneByteToModbus(unsigned char SendByteData)
{
    ModbusSendOneByte(SendByteData);
}

/*********************************************************************************/ 
/*��������:  SendMultiBytesToModbus()                             
 *��������� MultiBytes������ָ��
             MultiBytesLength:�ֽڳ���
 *����ֵ��    ��      
 *���ܽ��ܣ�   
        (1)���Ͷ���ֽڵ� Modbu Slave�� 
*/
/*********************************************************************************/
void SendMultiBytesToModbus(unsigned char * MultiBytes,unsigned char MultiBytesLength)
{
    while(MultiBytesLength--)
    {
        ModbusSendOneByte(*MultiBytes++);
    }
}
/*********************************************************************************/ 
/*��������:  SendRtuCmdToModBus()                             
 *��������� ModbusSlaveAddr���ӻ���ַ
             CmdData��ָ��������׵�ַ
 *����ֵ��    ��      
 *���ܽ��ܣ�   
        (1)����ModBus RTU ָ� Modbus Slave,����CRCУ���룻 
*/
/*********************************************************************************/ 
void SendCmdToModbus(unsigned char ModbusSlaveAddr, unsigned char CmdData[])
{
    unsigned char CRC16=0;
    CRC16=GetCRC16(CmdData,1);
    ModbusInterframeSpaceDelay();
    SendOneByteToModbus(ModbusSlaveAddr); //��ַλ
    SendOneByteToModbus(*CmdData);  //���ܴ���
    SendOneByteToModbus((unsigned char)(CRC16  & 0x00FF));       /* send CRC16 low */ 
    SendOneByteToModbus((unsigned char)(CRC16 >> 8 & 0x00FF));    /* send CRC16 high */ 
    ModbusInterframeSpaceDelay();
}
/*********************************************************************************/ 
/*��������:  SendRtuDataToModbus()                             
 *��������� ModbusSlaveAddr���ӻ���ַ
             SendData�����������ݵ��׵�ַ
             SendDataLength�����������ݵ��׵�ַ�ĳ��ȣ��ֽ�����
 *����ֵ��    ��      
 *���ܽ��ܣ�   
        (1)����ModBus RTU ���ݵ� Modbus Slave,����CRCУ���룻 
*/
/*********************************************************************************/ 
void SendRtuDataToModbus(unsigned char ModbusSlaveAddr,unsigned char CmdData,unsigned char SendData[], unsigned short SendDataLength)
{
    unsigned char CRC16=0;
    CRC16=GetCRC16(SendData,SendDataLength);
    ModbusInterframeSpaceDelay();//֡����ʱ
    
    SendOneByteToModbus(ModbusSlaveAddr); //��ַλ
    SendOneByteToModbus(CmdData);  //���ܴ���
    SendOneByteToModbus((unsigned char)(CRC16  & 0x00FF));       /* send CRC16 low */ 
    SendOneByteToModbus((unsigned char)(CRC16 >> 8 & 0x00FF));    /* send CRC16 high */ 
}
/*********************************************************************************/ 
/*��������:  ReceiveRtuDataFromModbus()                             
 *��������� SendData�����������ݵ��׵�ַ            
             SendDataLength�����������ݵ��׵�ַ�ĳ��ȣ��ֽ�����
 *����ֵ��    ��      
 *���ܽ��ܣ�   
       �� 
*/
/*********************************************************************************/ 
void  ReceiveRtuDataFromModbus(void)
{
    unsigned short ReceiveArrayDataCRC=0;
    unsigned short ArrayDataCRC=0;
    
   // ReceiveArrayDataCRC =*(ReceiveArrayDataPoniter-2)+ (*(ReceiveArrayDataPoniter-1)<<8);
   // ArrayDataCRC=GetCRC16(ReceiveArrayData,(ReceiveArrayDataPoniter-ReceiveArrayData-2));//������ͷ������ĸߵ�λ����

    //��ַ�ж�
    if((0 != ReceiveArrayData[0]) && (ModbusSlaveAdress != ReceiveArrayData[0])) 
    {  
        ReceiveArrayDataClean();
        return ;
    }    
       
    if((ReceiveArrayDataPoniter-ReceiveArrayData-2) > 0)
    {
        ReceiveArrayDataCRC = (*(ReceiveArrayDataPoniter-1))+ (*(ReceiveArrayDataPoniter-2)<<8);
        ReceiveDataCrcRightFlag=(ReceiveArrayDataCRC != GetCRC16(ReceiveArrayData,(ReceiveArrayDataPoniter-ReceiveArrayData-2))) ? False : True;
    }
        
    if(False == ReceiveDataCrcRightFlag)
    {
        ReceiveArrayDataClean();
        return ;
    }
    switch(ReceiveArrayData[1])
    {
        
        case 0x03: //�����ּĴ���
            ModbusResponseToHoldRegister();
        break;        
        case 0x04://������Ĵ���
            ModbusResponseToInputRegister();
        break;              
        case 0x06: //д�����Ĵ���
          ModbusResponseWriteSingleRegister();
        break;
        case 0x10://д����Ĵ���
           ModbusResponseWriteMultiRegister();
        break;
        case 0x41://��һ��ADת��ֵ
            ModbusResponseReadChannelOnce();
        break;
        case 0x42://�����ADֵ
            ModbusResponseReadChannelContinous();
        break;
        default:                        //�쳣��Ӧ
            ReceiveArrayData[1] |=0x80; //��Ӧ������
            ReceiveArrayData[2]  =0x01; //�쳣��
            SendMultiBytesToModbus(ReceiveArrayData,3);
        break;
    }
    ReceiveArrayDataClean();
}

/*********************************************************************************/ 
/*��������:  ModbusResponseToHoldRegister()                             
 *��������� �� 
 *����ֵ��   ��      
 *���ܽ��ܣ�   
        (1)������������ּĴ��� 
* ע�� ����ļĴ�����ַ��һ��Ϊʵ�ʲ����Ĵ�����ַ
*/
/*********************************************************************************/ 
void ModbusResponseToHoldRegister(void)
{
    unsigned short StartRegisterAddr=0;
    unsigned char  RegisterNumber=0;
    unsigned char  Temp=0;
    StartRegisterAddr=(ReceiveArrayData[2]<<8)+ReceiveArrayData[3]; 
    RegisterNumber=(ReceiveArrayData[4]<<8)+ReceiveArrayData[5];
    if((0x01 <= RegisterNumber) && (RegisterNumber <= SalveTotalRegisterNumber))//���жϷ��ʵļĴ�������
    {
        if(((0x0001 <= StartRegisterAddr) && (StartRegisterAddr <=SalveTotalRegisterNumber)) && (StartRegisterAddr+RegisterNumber-1  <= SalveTotalRegisterNumber))//���жϷ��ʼĴ����ĵ�ַ
        {
          //�ȴ���ʱ���
          TIM2->CNT=0;
          TIM_Cmd(TIM2,ENABLE);
          ReadFromAD7192ViaSPI((StartRegisterAddr-1),RegisterNumber,AD7192Registers,0);//����ļĴ�����ַ��һ��Ϊʵ�ʲ����Ĵ�����ַ
          TIM_Cmd(TIM2,DISABLE);
          if(False == SalveTimeOutCheckFlag)
          {
            //��������
            ReceiveArrayData[2]=4*RegisterNumber;//�ֽ���  ����һ���Ĵ��� 32λ
            for(Temp=0;Temp<RegisterNumber;Temp++)
            {
                ReceiveArrayData[4*Temp+3] =AD7192Registers[StartRegisterAddr-1+Temp]>>24;//���ֽ��ȷ���
                ReceiveArrayData[4*Temp+4] =AD7192Registers[StartRegisterAddr-1+Temp]>>16;//���ֽں���                      
                ReceiveArrayData[4*Temp+5] =AD7192Registers[StartRegisterAddr-1+Temp]>>8;//���ֽ��ȷ���
                ReceiveArrayData[4*Temp+6] =AD7192Registers[StartRegisterAddr-1+Temp];//���ֽں���
            }
            SendMultiBytesToModbus((ReceiveArrayData),(ReceiveArrayData[2]+3));
            return;            
          }
          else
          {
              ReceiveArrayData[2]  =0x04; //�쳣�� 
          }              
        }
        //End if(((0x0001 <= StartRegisterAddr) && ....
        else
        {
            ReceiveArrayData[2]  =0x02; //�쳣�� 
        }
    }
    //End if(0x01 <= RegisterNumber) && ....
    else
    {       
         ReceiveArrayData[2]  =0x03; //�쳣��  �Ĵ�������
    }
    
     ReceiveArrayData[1] |=0x80; //��Ӧ������
     SendMultiBytesToModbus(ReceiveArrayData,3);
}

/*********************************************************************************/ 
/*��������:  ModbusResponseToInputRegister()                             
 *��������� �� 
 *����ֵ��   ��      
 *���ܽ��ܣ�   
        (1)�������������Ĵ��� 
* ע�� ����ļĴ�����ַ��һ��Ϊʵ�ʲ����Ĵ�����ַ
*/
/*********************************************************************************/ 
void ModbusResponseToInputRegister(void)
{
    unsigned short StartRegisterAddr=0;
    unsigned char  RegisterNumber=0;
    unsigned char  Temp=0;
    StartRegisterAddr=(ReceiveArrayData[2]<<8)+ReceiveArrayData[3]; 
    RegisterNumber=(ReceiveArrayData[4]<<8)+ReceiveArrayData[5];
    if((0x01 <= RegisterNumber) && (RegisterNumber <= SalveTotalRegisterNumber)) //���жϷ��ʵļĴ�������
    {
        if(((0x0001 <= StartRegisterAddr) && (StartRegisterAddr <=SalveTotalRegisterNumber)) && (StartRegisterAddr+RegisterNumber-1  <= SalveTotalRegisterNumber))//���жϷ��ʼĴ����ĵ�ַ
        {
          //�ȴ���ʱ���
          TIM2->CNT=0;
          TIM_Cmd(TIM2,ENABLE);
          ReadFromAD7192ViaSPI((StartRegisterAddr-1),RegisterNumber,AD7192Registers,0);//����ļĴ�����ַ��һ��Ϊʵ�ʲ����Ĵ�����ַ
          TIM_Cmd(TIM2,DISABLE);
          if(False == SalveTimeOutCheckFlag)//����û�г�ʱ
          {
            //��������
            ReceiveArrayData[2]=4*RegisterNumber;//�ֽ���
            for(Temp=0;Temp<RegisterNumber;Temp++)
            {
                ReceiveArrayData[4*Temp+3] =AD7192Registers[StartRegisterAddr-1+Temp]>>24;//���ֽ��ȷ���
                ReceiveArrayData[4*Temp+4] =AD7192Registers[StartRegisterAddr-1+Temp]>>16;//���ֽں���                      
                ReceiveArrayData[4*Temp+5] =AD7192Registers[StartRegisterAddr-1+Temp]>>8;//���ֽ��ȷ���
                ReceiveArrayData[4*Temp+6] =AD7192Registers[StartRegisterAddr-1+Temp];//���ֽں���
            }
            SendMultiBytesToModbus((ReceiveArrayData),(ReceiveArrayData[2]+3));
            return;            
          }
          else
          {
              ReceiveArrayData[2]  =0x04; //�쳣�� 
          }              
        }
        //End if(((0x0001 <= StartRegisterAddr) &&...
        else
        {
            ReceiveArrayData[2]  =0x02; //�쳣�� 
        }
    }
    //End if((0x01 <= RegisterNumber) && ...
    else
    {       
         ReceiveArrayData[2]  =0x03; //�쳣��  �Ĵ�������
    }
     
     ReceiveArrayData[1] |=0x80; //��Ӧ������
     SendMultiBytesToModbus(ReceiveArrayData,3);
    
}

/*********************************************************************************/ 
/*��������:  ModbusResponseWriteSingleRegister()                             
 *��������� �� 
 *����ֵ��   ��      
 *���ܽ��ܣ�   
        (1)��������д�����Ĵ��� 
* ע�� ����ļĴ�����ַ��һ��Ϊʵ�ʲ����Ĵ�����ַ
*/
/*********************************************************************************/ 
void ModbusResponseWriteSingleRegister(void)
{
    unsigned short StartRegisterAddr=0;
    unsigned long int WriteToAD7192Register=0;
    StartRegisterAddr=(ReceiveArrayData[2]<<8)+ReceiveArrayData[3]; 
    AD7192Registers[StartRegisterAddr-1]=(ReceiveArrayData[4]<<24)+(ReceiveArrayData[5]<<16)+(ReceiveArrayData[6]<<8)+ReceiveArrayData[7];//д���Ĵ�����ֵ 32λ
    if(WriteToAD7192Register <= 0x00ffffff)  //д�Ĵ������24λ
    {
        if((0x0001 <= StartRegisterAddr) && (StartRegisterAddr <=SalveTotalRegisterNumber)) //�жϷ��ʵ��Ǹ��Ĵ���
        {
          //�ȴ���ʱ���
          TIM2->CNT=0;
          TIM_Cmd(TIM2,ENABLE);
          WriteToAD7192ViaSPI((StartRegisterAddr-1),1,AD7192Registers,0);//����ļĴ�����ַ��һ��Ϊʵ�ʲ����Ĵ�����ַ 
          AD7192Registers[StartRegisterAddr-1] = 0;  //��������
          ReadFromAD7192ViaSPI((StartRegisterAddr-1),1,&AD7192Registers[StartRegisterAddr-1],0);//����ļĴ�����ַ��һ��Ϊʵ�ʲ����Ĵ�����ַ  
          TIM_Cmd(TIM2,DISABLE);
          if(False == SalveTimeOutCheckFlag)//����û�г�ʱ
          {
            //��������            
            ReceiveArrayData[2]=StartRegisterAddr>>8;//�Ĵ�����ַ��λ�ֽ���
            ReceiveArrayData[3]=StartRegisterAddr;                  
            ReceiveArrayData[4] =AD7192Registers[StartRegisterAddr-1]>>24;//���ֽ��ȷ���
            ReceiveArrayData[5] =AD7192Registers[StartRegisterAddr-1]>>16;//���ֽں���                      
            ReceiveArrayData[6] =AD7192Registers[StartRegisterAddr-1]>>8;//���ֽ��ȷ���
            ReceiveArrayData[7] =AD7192Registers[StartRegisterAddr-1];//���ֽں���
            SendMultiBytesToModbus((ReceiveArrayData),8);
            return;            
          }
          else
          {
              ReceiveArrayData[2]  =0x04; //д�����쳣�� 
          }              
        }
        //End if(((0x0001 <= StartRegisterAddr) &&...
        else
        {
            ReceiveArrayData[2]  =0x02; //�Ĵ�����ַ�쳣�� 
        }
    }
    else
    {
        ReceiveArrayData[2]  =0x02; //�Ĵ�����ֵ�쳣�� 
    }     
     ReceiveArrayData[1] |=0x80; //��Ӧ������
     SendMultiBytesToModbus(ReceiveArrayData,3);    
}
/*********************************************************************************/ 
/*��������:  ModbusResponseWriteMultiRegister()                             
 *��������� �� 
 *����ֵ��   ��      
 *���ܽ��ܣ�   
        (1)��������д����Ĵ��� 
* ע�� ����ļĴ�����ַ��һ��Ϊʵ�ʲ����Ĵ�����ַ
*/
/*********************************************************************************/ 
void ModbusResponseWriteMultiRegister(void)
{
    unsigned short StartRegisterAddr=0;
    unsigned char  RegisterNumber=0;
    unsigned char  Temp=0;
    unsigned char  TotalByteNumber=0;//�ֽ���
    
    StartRegisterAddr=(ReceiveArrayData[2]<<8)+ReceiveArrayData[3]; 
    RegisterNumber=(ReceiveArrayData[4]<<8)+ReceiveArrayData[5];
    TotalByteNumber=ReceiveArrayData[6];
    for(Temp=0;Temp<RegisterNumber;Temp++)
    {
        AD7192Registers[StartRegisterAddr-1+Temp]=(ReceiveArrayData[7+Temp*4]<<24)+(ReceiveArrayData[8+Temp*4]<<16)+(ReceiveArrayData[9+Temp*4]<<8)+ReceiveArrayData[10+Temp*4];
    }
    if((0x01 <= RegisterNumber) && (RegisterNumber <= SalveTotalRegisterNumber) && (TotalByteNumber == RegisterNumber*4) )//���жϷ��ʵļĴ�������  �ж��ֽ������������ļĴ���
    {
        if(((0x0001 <= StartRegisterAddr) && (StartRegisterAddr <=SalveTotalRegisterNumber)) && (StartRegisterAddr+RegisterNumber-1  <= SalveTotalRegisterNumber))//���жϷ��ʼĴ����ĵ�ַ
        {
          //�ȴ���ʱ���
          TIM2->CNT=0;
          TIM_Cmd(TIM2,ENABLE);
          
          WriteToAD7192ViaSPI((StartRegisterAddr-1),RegisterNumber,AD7192Registers,0);//����ļĴ�����ַ��һ��Ϊʵ�ʲ����Ĵ�����ַ.
            
            
          TIM_Cmd(TIM2,DISABLE);
          if(False == SalveTimeOutCheckFlag)
          {
            //��������
            ReceiveArrayData[2] =StartRegisterAddr>>8;//���ֽ��ȷ���
            ReceiveArrayData[3] =StartRegisterAddr;//���ֽں���                      
            ReceiveArrayData[4] =RegisterNumber>>8;//���ֽ��ȷ���   
            ReceiveArrayData[5] =RegisterNumber;//���ֽں���
            SendMultiBytesToModbus((ReceiveArrayData),6);
            return;            
          }
          else
          {
              ReceiveArrayData[2]  =0x04; //�쳣�� 
          }              
        }
        //End if(((0x0001 <= StartRegisterAddr) && ....
        else
        {
            ReceiveArrayData[2]  =0x02; //�쳣�� 
        }
    }
    //End if(0x01 <= RegisterNumber) && ....
    else
    {       
         ReceiveArrayData[2]  =0x03; //�쳣��  �Ĵ�������
    }
  
     ReceiveArrayData[1] |=0x80; //��Ӧ������
     SendMultiBytesToModbus(ReceiveArrayData,3);
}

#endif
