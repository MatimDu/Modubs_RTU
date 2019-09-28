#include "Modbus.h"
#include "delay.h"
#include "stm32f10x.h"
#include "usart.h"
#include "AD7192.h"
#include <string.h>
#include "ADG1409.h"
//模式配置
//默认校验位是        偶校验位
//默认传输模式是      RTU
//TransferModeTypedef TransferMode = RTU_Mode;
//OddEvenCheckTypedef OddEvenCheckMode=OddCheck; 
BoolFlagTypedef     SalveTimeOutCheckFlag=False;
BoolFlagTypedef     ReceiveDataCompleteFlag =False;
BoolFlagTypedef     ReceiveDataCrcRightFlag =False;
unsigned char   ReceiveArrayData[256]={0};//接收缓存区
unsigned char * ReceiveArrayDataPoniter=ReceiveArrayData;//接收缓存区

#define ModbusSendOneByte                        UART4_Send_Byte
#define ReceiveArrayDataClean()                 {memset(ReceiveArrayData,0,256);\
                                                 memset(AD7192Registers,0,8*4);\
                                                 ReceiveDataCompleteFlag =False;\
                                                 SalveTimeOutCheckFlag =False;\
                                                 ReceiveArrayDataPoniter=ReceiveArrayData;} // 0 填充数组  初始化
#define ModbusInterframeSpaceDelay()             delay_ms(2); //报文帧由时长至少为 3.5个字符时间的空闲间隔区分
#define ModbusSlaveAdress                        0x01       //  0：广播地址  1~47：字节点单独地址 248~55：保留
#define SalveTotalRegisterNumber                 8  //可读写8个寄存器   八个寄存器宽度 8/24/32 
                                                 
                                                 
void ModbusResponseToHoldRegister(void);
void ModbusResponseToInputRegister(void);
void ModbusResponseWriteSingleRegister(void);
void ModbusResponseWriteMultiRegister(void);
                                                 
   

#if 0 
                                                 
#else 
/*********************************************************************************/ 
/*函数名称:  GetCRC16()                           
*输入参数：  puchMsg； 用于计算CRC的报文
             usDataLen：报文中的字节数
*返回值：    CRC值    
*功能介绍：   
        (1)CRC16校验； 返回校验码；               
*/                                    
/*********************************************************************************/ 
unsigned short GetCRC16(unsigned char *puchMsg, unsigned short usDataLen)   
{   
    /* CRC 高位字节值表 */   
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
    //CRC寄存器初始化 
    unsigned char uchCRCHi = 0xFF ; /* 高CRC字节初始化 */   
    unsigned char uchCRCLo = 0xFF ; /* 低CRC 字节初始化 */   
    unsigned uIndex = 0; /* CRC循环中的索引 */   
     
    while (usDataLen--) /* 传输消息缓冲区 */   
    {   
        uIndex = uchCRCHi ^ *puchMsg++ ; /* 计算CRC */   
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;   
        uchCRCLo = auchCRCLo[uIndex] ;   
    }   
    return (unsigned short)((unsigned short)uchCRCHi << 8 | uchCRCLo) ;   
}
/*********************************************************************************/ 
/*函数名称:  SendOneByteToModbus()                             
 *输入参数： SendByteData：发送的数据           
 *返回值：    无      
 *功能介绍：   
        (1)发送一个字节到 ModbusSlave； 
*/
/*********************************************************************************/
void SendOneByteToModbus(unsigned char SendByteData)
{
    ModbusSendOneByte(SendByteData);
}

/*********************************************************************************/ 
/*函数名称:  SendMultiBytesToModbus()                             
 *输入参数： MultiBytes：数据指针
             MultiBytesLength:字节长度
 *返回值：    无      
 *功能介绍：   
        (1)发送多个字节到 Modbu Slave； 
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
/*函数名称:  SendRtuCmdToModBus()                             
 *输入参数： ModbusSlaveAddr：从机地址
             CmdData：指令的数组首地址
 *返回值：    无      
 *功能介绍：   
        (1)发送ModBus RTU 指令到 Modbus Slave,加上CRC校验码； 
*/
/*********************************************************************************/ 
void SendCmdToModbus(unsigned char ModbusSlaveAddr, unsigned char CmdData[])
{
    unsigned char CRC16=0;
    CRC16=GetCRC16(CmdData,1);
    ModbusInterframeSpaceDelay();
    SendOneByteToModbus(ModbusSlaveAddr); //地址位
    SendOneByteToModbus(*CmdData);  //功能代码
    SendOneByteToModbus((unsigned char)(CRC16  & 0x00FF));       /* send CRC16 low */ 
    SendOneByteToModbus((unsigned char)(CRC16 >> 8 & 0x00FF));    /* send CRC16 high */ 
    ModbusInterframeSpaceDelay();
}
/*********************************************************************************/ 
/*函数名称:  SendRtuDataToModbus()                             
 *输入参数： ModbusSlaveAddr：从机地址
             SendData：待发送数据的首地址
             SendDataLength：待发送数据的首地址的长度（字节数）
 *返回值：    无      
 *功能介绍：   
        (1)发送ModBus RTU 数据到 Modbus Slave,加上CRC校验码； 
*/
/*********************************************************************************/ 
void SendRtuDataToModbus(unsigned char ModbusSlaveAddr,unsigned char CmdData,unsigned char SendData[], unsigned short SendDataLength)
{
    unsigned char CRC16=0;
    CRC16=GetCRC16(SendData,SendDataLength);
    ModbusInterframeSpaceDelay();//帧间延时
    
    SendOneByteToModbus(ModbusSlaveAddr); //地址位
    SendOneByteToModbus(CmdData);  //功能代码
    SendOneByteToModbus((unsigned char)(CRC16  & 0x00FF));       /* send CRC16 low */ 
    SendOneByteToModbus((unsigned char)(CRC16 >> 8 & 0x00FF));    /* send CRC16 high */ 
}
/*********************************************************************************/ 
/*函数名称:  ReceiveRtuDataFromModbus()                             
 *输入参数： SendData：待发送数据的首地址            
             SendDataLength：待发送数据的首地址的长度（字节数）
 *返回值：    无      
 *功能介绍：   
       ； 
*/
/*********************************************************************************/ 
void  ReceiveRtuDataFromModbus(void)
{
    unsigned short ReceiveArrayDataCRC=0;
    unsigned short ArrayDataCRC=0;
    
   // ReceiveArrayDataCRC =*(ReceiveArrayDataPoniter-2)+ (*(ReceiveArrayDataPoniter-1)<<8);
   // ArrayDataCRC=GetCRC16(ReceiveArrayData,(ReceiveArrayDataPoniter-ReceiveArrayData-2));//算出来和发过来的高低位反了

    //地址判断
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
        
        case 0x03: //读保持寄存器
            ModbusResponseToHoldRegister();
        break;        
        case 0x04://读输入寄存器
            ModbusResponseToInputRegister();
        break;              
        case 0x06: //写单个寄存器
          ModbusResponseWriteSingleRegister();
        break;
        case 0x10://写多个寄存器
           ModbusResponseWriteMultiRegister();
        break;
        case 0x41://读一次AD转换值
            ModbusResponseReadChannelOnce();
        break;
        case 0x42://读多次AD值
            ModbusResponseReadChannelContinous();
        break;
        default:                        //异常响应
            ReceiveArrayData[1] |=0x80; //响应功能码
            ReceiveArrayData[2]  =0x01; //异常码
            SendMultiBytesToModbus(ReceiveArrayData,3);
        break;
    }
    ReceiveArrayDataClean();
}

/*********************************************************************************/ 
/*函数名称:  ModbusResponseToHoldRegister()                             
 *输入参数： 无 
 *返回值：   无      
 *功能介绍：   
        (1)；功能码读保持寄存器 
* 注意 请求的寄存器地址减一才为实际操作寄存器地址
*/
/*********************************************************************************/ 
void ModbusResponseToHoldRegister(void)
{
    unsigned short StartRegisterAddr=0;
    unsigned char  RegisterNumber=0;
    unsigned char  Temp=0;
    StartRegisterAddr=(ReceiveArrayData[2]<<8)+ReceiveArrayData[3]; 
    RegisterNumber=(ReceiveArrayData[4]<<8)+ReceiveArrayData[5];
    if((0x01 <= RegisterNumber) && (RegisterNumber <= SalveTotalRegisterNumber))//先判断访问的寄存器数量
    {
        if(((0x0001 <= StartRegisterAddr) && (StartRegisterAddr <=SalveTotalRegisterNumber)) && (StartRegisterAddr+RegisterNumber-1  <= SalveTotalRegisterNumber))//再判断访问寄存器的地址
        {
          //等待超时检测
          TIM2->CNT=0;
          TIM_Cmd(TIM2,ENABLE);
          ReadFromAD7192ViaSPI((StartRegisterAddr-1),RegisterNumber,AD7192Registers,0);//请求的寄存器地址减一才为实际操作寄存器地址
          TIM_Cmd(TIM2,DISABLE);
          if(False == SalveTimeOutCheckFlag)
          {
            //发送数据
            ReceiveArrayData[2]=4*RegisterNumber;//字节数  这里一个寄存器 32位
            for(Temp=0;Temp<RegisterNumber;Temp++)
            {
                ReceiveArrayData[4*Temp+3] =AD7192Registers[StartRegisterAddr-1+Temp]>>24;//高字节先发送
                ReceiveArrayData[4*Temp+4] =AD7192Registers[StartRegisterAddr-1+Temp]>>16;//低字节后发送                      
                ReceiveArrayData[4*Temp+5] =AD7192Registers[StartRegisterAddr-1+Temp]>>8;//高字节先发送
                ReceiveArrayData[4*Temp+6] =AD7192Registers[StartRegisterAddr-1+Temp];//低字节后发送
            }
            SendMultiBytesToModbus((ReceiveArrayData),(ReceiveArrayData[2]+3));
            return;            
          }
          else
          {
              ReceiveArrayData[2]  =0x04; //异常码 
          }              
        }
        //End if(((0x0001 <= StartRegisterAddr) && ....
        else
        {
            ReceiveArrayData[2]  =0x02; //异常码 
        }
    }
    //End if(0x01 <= RegisterNumber) && ....
    else
    {       
         ReceiveArrayData[2]  =0x03; //异常码  寄存器数量
    }
    
     ReceiveArrayData[1] |=0x80; //响应功能码
     SendMultiBytesToModbus(ReceiveArrayData,3);
}

/*********************************************************************************/ 
/*函数名称:  ModbusResponseToInputRegister()                             
 *输入参数： 无 
 *返回值：   无      
 *功能介绍：   
        (1)；功能码读输入寄存器 
* 注意 请求的寄存器地址减一才为实际操作寄存器地址
*/
/*********************************************************************************/ 
void ModbusResponseToInputRegister(void)
{
    unsigned short StartRegisterAddr=0;
    unsigned char  RegisterNumber=0;
    unsigned char  Temp=0;
    StartRegisterAddr=(ReceiveArrayData[2]<<8)+ReceiveArrayData[3]; 
    RegisterNumber=(ReceiveArrayData[4]<<8)+ReceiveArrayData[5];
    if((0x01 <= RegisterNumber) && (RegisterNumber <= SalveTotalRegisterNumber)) //先判断访问的寄存器数量
    {
        if(((0x0001 <= StartRegisterAddr) && (StartRegisterAddr <=SalveTotalRegisterNumber)) && (StartRegisterAddr+RegisterNumber-1  <= SalveTotalRegisterNumber))//再判断访问寄存器的地址
        {
          //等待超时检测
          TIM2->CNT=0;
          TIM_Cmd(TIM2,ENABLE);
          ReadFromAD7192ViaSPI((StartRegisterAddr-1),RegisterNumber,AD7192Registers,0);//请求的寄存器地址减一才为实际操作寄存器地址
          TIM_Cmd(TIM2,DISABLE);
          if(False == SalveTimeOutCheckFlag)//处理没有超时
          {
            //发送数据
            ReceiveArrayData[2]=4*RegisterNumber;//字节数
            for(Temp=0;Temp<RegisterNumber;Temp++)
            {
                ReceiveArrayData[4*Temp+3] =AD7192Registers[StartRegisterAddr-1+Temp]>>24;//高字节先发送
                ReceiveArrayData[4*Temp+4] =AD7192Registers[StartRegisterAddr-1+Temp]>>16;//低字节后发送                      
                ReceiveArrayData[4*Temp+5] =AD7192Registers[StartRegisterAddr-1+Temp]>>8;//高字节先发送
                ReceiveArrayData[4*Temp+6] =AD7192Registers[StartRegisterAddr-1+Temp];//低字节后发送
            }
            SendMultiBytesToModbus((ReceiveArrayData),(ReceiveArrayData[2]+3));
            return;            
          }
          else
          {
              ReceiveArrayData[2]  =0x04; //异常码 
          }              
        }
        //End if(((0x0001 <= StartRegisterAddr) &&...
        else
        {
            ReceiveArrayData[2]  =0x02; //异常码 
        }
    }
    //End if((0x01 <= RegisterNumber) && ...
    else
    {       
         ReceiveArrayData[2]  =0x03; //异常码  寄存器数量
    }
     
     ReceiveArrayData[1] |=0x80; //响应功能码
     SendMultiBytesToModbus(ReceiveArrayData,3);
    
}

/*********************************************************************************/ 
/*函数名称:  ModbusResponseWriteSingleRegister()                             
 *输入参数： 无 
 *返回值：   无      
 *功能介绍：   
        (1)；功能码写单个寄存器 
* 注意 请求的寄存器地址减一才为实际操作寄存器地址
*/
/*********************************************************************************/ 
void ModbusResponseWriteSingleRegister(void)
{
    unsigned short StartRegisterAddr=0;
    unsigned long int WriteToAD7192Register=0;
    StartRegisterAddr=(ReceiveArrayData[2]<<8)+ReceiveArrayData[3]; 
    AD7192Registers[StartRegisterAddr-1]=(ReceiveArrayData[4]<<24)+(ReceiveArrayData[5]<<16)+(ReceiveArrayData[6]<<8)+ReceiveArrayData[7];//写到寄存器的值 32位
    if(WriteToAD7192Register <= 0x00ffffff)  //写寄存器最多24位
    {
        if((0x0001 <= StartRegisterAddr) && (StartRegisterAddr <=SalveTotalRegisterNumber)) //判断访问的那个寄存器
        {
          //等待超时检测
          TIM2->CNT=0;
          TIM_Cmd(TIM2,ENABLE);
          WriteToAD7192ViaSPI((StartRegisterAddr-1),1,AD7192Registers,0);//请求的寄存器地址减一才为实际操作寄存器地址 
          AD7192Registers[StartRegisterAddr-1] = 0;  //必须清零
          ReadFromAD7192ViaSPI((StartRegisterAddr-1),1,&AD7192Registers[StartRegisterAddr-1],0);//请求的寄存器地址减一才为实际操作寄存器地址  
          TIM_Cmd(TIM2,DISABLE);
          if(False == SalveTimeOutCheckFlag)//处理没有超时
          {
            //发送数据            
            ReceiveArrayData[2]=StartRegisterAddr>>8;//寄存器地址高位字节数
            ReceiveArrayData[3]=StartRegisterAddr;                  
            ReceiveArrayData[4] =AD7192Registers[StartRegisterAddr-1]>>24;//高字节先发送
            ReceiveArrayData[5] =AD7192Registers[StartRegisterAddr-1]>>16;//低字节后发送                      
            ReceiveArrayData[6] =AD7192Registers[StartRegisterAddr-1]>>8;//高字节先发送
            ReceiveArrayData[7] =AD7192Registers[StartRegisterAddr-1];//低字节后发送
            SendMultiBytesToModbus((ReceiveArrayData),8);
            return;            
          }
          else
          {
              ReceiveArrayData[2]  =0x04; //写操作异常码 
          }              
        }
        //End if(((0x0001 <= StartRegisterAddr) &&...
        else
        {
            ReceiveArrayData[2]  =0x02; //寄存器地址异常码 
        }
    }
    else
    {
        ReceiveArrayData[2]  =0x02; //寄存器数值异常码 
    }     
     ReceiveArrayData[1] |=0x80; //响应功能码
     SendMultiBytesToModbus(ReceiveArrayData,3);    
}
/*********************************************************************************/ 
/*函数名称:  ModbusResponseWriteMultiRegister()                             
 *输入参数： 无 
 *返回值：   无      
 *功能介绍：   
        (1)；功能码写多个寄存器 
* 注意 请求的寄存器地址减一才为实际操作寄存器地址
*/
/*********************************************************************************/ 
void ModbusResponseWriteMultiRegister(void)
{
    unsigned short StartRegisterAddr=0;
    unsigned char  RegisterNumber=0;
    unsigned char  Temp=0;
    unsigned char  TotalByteNumber=0;//字节数
    
    StartRegisterAddr=(ReceiveArrayData[2]<<8)+ReceiveArrayData[3]; 
    RegisterNumber=(ReceiveArrayData[4]<<8)+ReceiveArrayData[5];
    TotalByteNumber=ReceiveArrayData[6];
    for(Temp=0;Temp<RegisterNumber;Temp++)
    {
        AD7192Registers[StartRegisterAddr-1+Temp]=(ReceiveArrayData[7+Temp*4]<<24)+(ReceiveArrayData[8+Temp*4]<<16)+(ReceiveArrayData[9+Temp*4]<<8)+ReceiveArrayData[10+Temp*4];
    }
    if((0x01 <= RegisterNumber) && (RegisterNumber <= SalveTotalRegisterNumber) && (TotalByteNumber == RegisterNumber*4) )//先判断访问的寄存器数量  判断字节数等于两倍的寄存器
    {
        if(((0x0001 <= StartRegisterAddr) && (StartRegisterAddr <=SalveTotalRegisterNumber)) && (StartRegisterAddr+RegisterNumber-1  <= SalveTotalRegisterNumber))//再判断访问寄存器的地址
        {
          //等待超时检测
          TIM2->CNT=0;
          TIM_Cmd(TIM2,ENABLE);
          
          WriteToAD7192ViaSPI((StartRegisterAddr-1),RegisterNumber,AD7192Registers,0);//请求的寄存器地址减一才为实际操作寄存器地址.
            
            
          TIM_Cmd(TIM2,DISABLE);
          if(False == SalveTimeOutCheckFlag)
          {
            //发送数据
            ReceiveArrayData[2] =StartRegisterAddr>>8;//高字节先发送
            ReceiveArrayData[3] =StartRegisterAddr;//低字节后发送                      
            ReceiveArrayData[4] =RegisterNumber>>8;//高字节先发送   
            ReceiveArrayData[5] =RegisterNumber;//低字节后发送
            SendMultiBytesToModbus((ReceiveArrayData),6);
            return;            
          }
          else
          {
              ReceiveArrayData[2]  =0x04; //异常码 
          }              
        }
        //End if(((0x0001 <= StartRegisterAddr) && ....
        else
        {
            ReceiveArrayData[2]  =0x02; //异常码 
        }
    }
    //End if(0x01 <= RegisterNumber) && ....
    else
    {       
         ReceiveArrayData[2]  =0x03; //异常码  寄存器数量
    }
  
     ReceiveArrayData[1] |=0x80; //响应功能码
     SendMultiBytesToModbus(ReceiveArrayData,3);
}

#endif
