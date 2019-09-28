#ifndef _modbus_h
#define _modbus_h

typedef enum 
{
    OddCheck=0, //奇校验
    EvenCheck=1,//偶校验
    NoneCheck=2//无校验
}
OddEvenCheckTypedef;  //奇偶校验 使用无奇偶校验要求两个停止位

typedef enum 
{
    ASCII_Mode=0, //ASCII_Mode
    RTU_Mode=!ASCII_Mode,//RTU_Mode
}
TransferModeTypedef;  //传输模式

typedef enum 
{
    False=0,
    True=!False,
}
BoolFlagTypedef; 

void UsartInit(void);
void ReceiveRtuDataFromModbus(void);
void SendMultiBytesToModbus(unsigned char * MultiBytes,unsigned char MultiBytesLength);











#endif
